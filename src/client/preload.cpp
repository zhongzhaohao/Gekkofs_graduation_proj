/*
  Copyright 2018-2022, Barcelona Supercomputing Center (BSC), Spain
  Copyright 2015-2022, Johannes Gutenberg Universitaet Mainz, Germany

  This software was partially supported by the
  EC H2020 funded project NEXTGenIO (Project ID: 671951, www.nextgenio.eu).

  This software was partially supported by the
  ADA-FS project under the SPPEXA project funded by the DFG.

  This file is part of GekkoFS' POSIX interface.

  GekkoFS' POSIX interface is free software: you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public License as
  published by the Free Software Foundation, either version 3 of the License,
  or (at your option) any later version.

  GekkoFS' POSIX interface is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public License
  along with GekkoFS' POSIX interface.  If not, see
  <https://www.gnu.org/licenses/>.

  SPDX-License-Identifier: LGPL-3.0-or-later
*/

#include <client/preload.hpp>
#include <client/path.hpp>
#include <client/logging.hpp>
#include <client/rpc/forward_management.hpp>
#include <client/preload_util.hpp>
#include <client/intercept.hpp>

#include <common/rpc/distributor.hpp>
#include <common/common_defs.hpp>

#include <fstream>

#include <hermes.hpp>


using namespace std;

std::unique_ptr<hermes::async_engine> ld_network_service; // extern variable

namespace {

#ifdef GKFS_ENABLE_FORWARDING
pthread_t mapper;
bool forwarding_running;

pthread_mutex_t remap_mutex;
pthread_cond_t remap_signal;
#endif

inline void
exit_error_msg(int errcode, const string& msg) {

    LOG_ERROR("{}", msg);
    gkfs::log::logger::log_message(stderr, "{}\n", msg);

    // if we don't disable interception before calling ::exit()
    // syscall hooks may find an inconsistent in shared state
    // (e.g. the logger) and thus, crash
    gkfs::preload::stop_interception();
    CTX->disable_interception();
    ::exit(errcode);
}

/**
 * Initializes the Hermes client for a given transport prefix
 * @return true if successfully initialized; false otherwise
 */
bool
init_hermes_client() {

    try {

        hermes::engine_options opts{};

        if(CTX->auto_sm())
            opts |= hermes::use_auto_sm;
        if(gkfs::rpc::protocol::ofi_psm2 == CTX->rpc_protocol()) {
            opts |= hermes::force_no_block_progress;
        }

        opts |= hermes::process_may_fork;

        ld_network_service = std::make_unique<hermes::async_engine>(
                hermes::get_transport_type(CTX->rpc_protocol()), opts);
        ld_network_service->run();
    } catch(const std::exception& ex) {
        fmt::print(stderr, "Failed to initialize Hermes RPC client {}\n",
                   ex.what());
        return false;
    }
    return true;
}

#ifdef GKFS_ENABLE_FORWARDING
void*
forwarding_mapper(void* p) {
    struct timespec timeout;
    clock_gettime(CLOCK_REALTIME, &timeout);
    timeout.tv_sec += 10; // 10 seconds

    int previous = -1;

    while(forwarding_running) {
        try {
            gkfs::utils::load_forwarding_map();

            if(previous != CTX->fwd_host_id()) {
                LOG(INFO, "{}() Forward to {}", __func__, CTX->fwd_host_id());

                previous = CTX->fwd_host_id();
            }
        } catch(std::exception& e) {
            exit_error_msg(EXIT_FAILURE,
                           fmt::format("Unable set the forwarding host '{}'",
                                       e.what()));
        }

        pthread_mutex_lock(&remap_mutex);
        pthread_cond_timedwait(&remap_signal, &remap_mutex, &timeout);
        pthread_mutex_unlock(&remap_mutex);
    }

    return nullptr;
}
#endif

#ifdef GKFS_ENABLE_FORWARDING
void
init_forwarding_mapper() {
    forwarding_running = true;

    pthread_create(&mapper, NULL, forwarding_mapper, NULL);
}
#endif

#ifdef GKFS_ENABLE_FORWARDING
void
destroy_forwarding_mapper() {
    forwarding_running = false;

    pthread_cond_signal(&remap_signal);

    pthread_join(mapper, NULL);
}
#endif

void
log_prog_name() {
    std::string line;
    std::ifstream cmdline("/proc/self/cmdline");
    if(!cmdline.is_open()) {
        LOG(ERROR, "Unable to open cmdline file");
        throw std::runtime_error("Unable to open cmdline file");
    }
    if(!getline(cmdline, line)) {
        throw std::runtime_error("Unable to read cmdline file");
    }
    std::replace(line.begin(), line.end(), '\0', ' ');
    line.erase(line.length() - 1, line.length());
    LOG(INFO, "Process cmdline: '{}'", line);
    cmdline.close();
}

} // namespace

namespace gkfs::preload {

/**
 * This function is only called in init_envrionment before reading hostfile and hostconfigfile 
 * and request registry to generate merge hostfile and hostconfigfile
 */
int request_registry(){
    string mergeflows,hostfile,hostconfigfile;
    if(!gkfs::utils::CheckMerge(mergeflows,hostfile,hostconfigfile))
        return 0;
    //to do request register to merge fs
    //std::cout<< " now is requesting registry "<<std::endl;
    auto err = gkfs::rpc::forward_request_registry(mergeflows,hostconfigfile,hostfile);
    if(err) {
        errno = err;
        return -1;
    }
    return 0;
}


/**
 * This function is only called in the preload constructor and initializes
 * the file system client，包括读取hostfile，设置守护进程通信方式，
 */
void
init_environment() {

    string registry_addr = "";
    try {
        LOG(INFO, "Loading registry address...");
        registry_addr = gkfs::utils::read_registry_file();// proto://url，此时已经把上下文rpc通信协议变量设置好了
    } catch(const std::exception& e) {
        exit_error_msg(EXIT_FAILURE,
                       "Failed to load hosts addresses: "s + e.what());
    }   
    // initialize Hermes interface to Mercury
    LOG(INFO, "Initializing RPC subsystem...");
    if(!init_hermes_client()) { //指定通讯方式，打开mercury异步事件处理引擎
        exit_error_msg(EXIT_FAILURE, "Unable to initialize RPC subsystem");
    }
    try {
        gkfs::utils::connect_to_registry(registry_addr);//find hosts addr and save them to ctx
    } catch(const std::exception& e) {
        exit_error_msg(EXIT_FAILURE,
                       "Failed to connect to hosts: "s + e.what());
    }
    //向registry请求融合后
    request_registry();

    vector<pair<string, string>> hosts{};
    pair<vector<unsigned int>,vector<unsigned int>> hosts_config{};
    try {
        LOG(INFO, "Loading peer addresses...");
        hosts = gkfs::utils::read_hosts_file();//name , proto://url，此时已经把上下文rpc通信协议变量设置好了
    } catch(const std::exception& e) {
        exit_error_msg(EXIT_FAILURE,
                       "Failed to load hosts addresses: "s + e.what());
    }   
    try {
        LOG(INFO, "Loading system config...");
        hosts_config = gkfs::utils::read_hosts_config_file();
    } catch(const std::exception& e) {
        exit_error_msg(EXIT_FAILURE,
                       "Failed to load system config: "s + e.what());
    }
    CTX->hostsconfig(hosts_config.first);
    //std::cout<< "here to print hc" <<std::endl;
    //for(auto hc : hosts_config.first){
    //     std::cout << hc << " " ;
    // }
    //std:: cout <<std::endl;
    CTX->fspriority(hosts_config.second);
    
    try {
        gkfs::utils::connect_to_hosts(hosts);//find hosts addr and save them to ctx
    } catch(const std::exception& e) {
        exit_error_msg(EXIT_FAILURE,
                       "Failed to connect to hosts: "s + e.what());
    }

    /* Setup distributor */
#ifdef GKFS_ENABLE_FORWARDING
    try {
        gkfs::utils::load_forwarding_map();

        LOG(INFO, "{}() Forward to {}", __func__, CTX->fwd_host_id());
    } catch(std::exception& e) {
        exit_error_msg(
                EXIT_FAILURE,
                fmt::format("Unable set the forwarding host '{}'", e.what()));
    }

    auto forwarder_dist = std::make_shared<gkfs::rpc::ForwarderDistributor>(
            CTX->fwd_host_id(), CTX->hosts().size());
    CTX->distributor(forwarder_dist);
#else
#ifdef GKFS_USE_GUIDED_DISTRIBUTION
    auto distributor = std::make_shared<gkfs::rpc::GuidedDistributor>(
            CTX->local_host_id(), CTX->hosts().size());
#else
    auto distributor = std::make_shared<gkfs::rpc::SimpleHashDistributor>(
            CTX->local_host_id(), CTX->hostsconfig(),&(CTX->pathfs()), CTX->local_fs_id());
#endif
    CTX->distributor(distributor);
#endif

    //printf("%ld",(unsigned int)(&(CTX->pathfs())));
    LOG(INFO, "Retrieving file system configuration...");

    if(!gkfs::rpc::forward_get_fs_config()) {
        exit_error_msg(
                EXIT_FAILURE,
                "Unable to fetch file system configurations from daemon process through RPC.");
    }

    LOG(INFO, "Environment initialization successful.");
}

int register_registry(){
    string workflow,hostfile,hostconfigfile;
    gkfs::utils::read_env(workflow,hostfile,hostconfigfile);
    //makring register request to registry
    auto err = gkfs::rpc::forward_register_registry(workflow,hostconfigfile,hostfile);
    if(err) {
        errno = err;
        return -1;
    }
    return 0;
}

} // namespace gkfs::preload

/**
 * Called initially ONCE when preload library is used with the LD_PRELOAD
 * environment variable
 */
void
init_preload() {
    // The original errno value will be restored after initialization to not
    // leak internal error codes
    auto oerrno = errno;

    CTX->enable_interception();
    gkfs::preload::start_self_interception();

    CTX->init_logging();
    // from here ownwards it is safe to print messages
    LOG(DEBUG, "Logging subsystem initialized");

    // Kernel modules such as ib_uverbs may create fds in kernel space and pass
    // them to user-space processes using ioctl()-like interfaces. if this
    // happens during our internal initialization, there's no way for us to
    // control this creation and the fd will be created in the
    // [0, MAX_USER_FDS) range rather than in our private
    // [MAX_USER_FDS, MAX_OPEN_FDS) range. To prevent this for our internal
    // initialization code, we forcefully occupy the user fd range to force
    // such modules to create fds in our private range.
    CTX->protect_user_fds();//把所有0-max user fds中的文件描述符全部占用，这样新生成的文件描述符只能在私有范围内产生

    log_prog_name();
    gkfs::path::init_cwd();//修正上下文工作目录为当前进程的环境目录

    LOG(DEBUG, "Current working directory: '{}'", CTX->cwd());
    gkfs::preload::init_environment();//读了hostfile，并
    CTX->enable_interception();

    CTX->unprotect_user_fds();

#ifdef GKFS_ENABLE_FORWARDING
    init_forwarding_mapper();
#endif

    gkfs::preload::start_interception();
    errno = oerrno;

}
/**
 * Called last when preload library is used with the LD_PRELOAD environment
 * variable
 */
void
destroy_preload() {
#ifdef GKFS_ENABLE_FORWARDING
    destroy_forwarding_mapper();
#endif

    CTX->clear_hosts();
    LOG(DEBUG, "Peer information deleted");
    //register work flow to registry
    gkfs::preload::register_registry();

    ld_network_service.reset();
    LOG(DEBUG, "RPC subsystem shut down");

    gkfs::preload::stop_interception();
    CTX->disable_interception();
    LOG(DEBUG, "Syscall interception stopped");

    LOG(INFO, "All subsystems shut down. Client shutdown complete.");
}
