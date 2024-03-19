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

#include <client/preload_context.hpp>
#include <client/env.hpp>
#include <client/logging.hpp>
#include <client/open_file_map.hpp>
#include <client/open_dir.hpp>
#include <client/path.hpp>

#include <common/env_util.hpp>
#include <common/path_util.hpp>
#include <config.hpp>

#include <hermes.hpp>

#include <cassert>

extern "C" {
#include <libsyscall_intercept_hook_point.h>
#include <syscall.h>
}

namespace gkfs {

namespace preload {

decltype(PreloadContext::MIN_INTERNAL_FD) constexpr PreloadContext::
        MIN_INTERNAL_FD;
decltype(PreloadContext::MAX_USER_FDS) constexpr PreloadContext::MAX_USER_FDS;

PreloadContext::PreloadContext()
    : ofm_(std::make_shared<gkfs::filemap::OpenFileMap>()),
      fs_conf_(std::make_shared<FsConfig>()) {

    internal_fds_.set();
    internal_fds_must_relocate_ = true;

    char host[255];
    gethostname(host, 255);
    hostname = host;
}

void
PreloadContext::init_logging() {

    const std::string log_opts = gkfs::env::get_var(
            gkfs::env::LOG, gkfs::config::log::client_log_level);

    const std::string log_output = gkfs::env::get_var(
            gkfs::env::LOG_OUTPUT, gkfs::config::log::client_log_path);

#ifdef GKFS_DEBUG_BUILD
    // atoi returns 0 if no int conversion can be performed, which works
    // for us since if the user provides a non-numeric value we can just treat
    // it as zero
    const int log_verbosity = std::atoi(
            gkfs::env::get_var(gkfs::env::LOG_DEBUG_VERBOSITY, "0").c_str());

    const std::string log_filter =
            gkfs::env::get_var(gkfs::env::LOG_SYSCALL_FILTER, "");
#endif

    const std::string trunc_val =
            gkfs::env::get_var(gkfs::env::LOG_OUTPUT_TRUNC);

    const bool log_trunc = (!trunc_val.empty() && trunc_val[0] != '0');

    gkfs::log::create_global_logger(log_opts, log_output, log_trunc
#ifdef GKFS_DEBUG_BUILD
                                    ,
                                    log_filter, log_verbosity
#endif
    );
}

void
PreloadContext::mountdir(const std::string& path) {
    assert(gkfs::path::is_absolute(path));
    assert(!gkfs::path::has_trailing_slash(path));
    mountdir_components_ = gkfs::path::split_path(path);
    mountdir_ = path;
}

const std::string&
PreloadContext::mountdir() const {
    return mountdir_;
}

const std::vector<std::string>&
PreloadContext::mountdir_components() const {
    return mountdir_components_;
}

void
PreloadContext::cwd(const std::string& path) {
    cwd_ = path;
}

const std::string&
PreloadContext::cwd() const {
    return cwd_;
}

const std::vector<hermes::endpoint>&
PreloadContext::hosts() const {
    return hosts_;
}

void
PreloadContext::hosts(const std::vector<hermes::endpoint>& endpoints) {
    hosts_ = endpoints;
}

const std::vector<unsigned int>&
PreloadContext::hostsconfig() const {
    return hostsconfig_;
}

void
PreloadContext::hostsconfig(const std::vector<unsigned int>& hconfig) {
    hostsconfig_ = hconfig;
}

std::map<std::string, unsigned int>&
PreloadContext::pathfs() {
    return pathfs_;
}

void
PreloadContext::clear_hosts() {
    hosts_.clear();
}

uint64_t
PreloadContext::local_host_id() const {
    return local_host_id_;
}

void
PreloadContext::local_host_id(uint64_t id) {
    local_host_id_ = id;
}

uint64_t
PreloadContext::fwd_host_id() const {
    return fwd_host_id_;
}

void
PreloadContext::fwd_host_id(uint64_t id) {
    fwd_host_id_ = id;
}

const std::string&
PreloadContext::rpc_protocol() const {
    return rpc_protocol_;
}

void
PreloadContext::rpc_protocol(const std::string& rpc_protocol) {
    rpc_protocol_ = rpc_protocol;
}

bool
PreloadContext::auto_sm() const {
    return auto_sm_;
}

void
PreloadContext::auto_sm(bool auto_sm) {
    PreloadContext::auto_sm_ = auto_sm;
}

RelativizeStatus
PreloadContext::relativize_fd_path(int dirfd, const char* raw_path,
                                   std::string& relative_path, int flags,
                                   bool resolve_last_link) const {

    // Relativize path should be called only after the library constructor has
    // been executed
    assert(interception_enabled_);
    // If we run the constructor we also already setup the mountdir
    assert(!mountdir_.empty());

    // We assume raw path is valid
    assert(raw_path != nullptr);

    std::string path;

    if(raw_path != nullptr && raw_path[0] != gkfs::path::separator) {
        // path is relative
        if(dirfd == AT_FDCWD) {
            // path is relative to cwd
            path = gkfs::path::prepend_path(cwd_, raw_path);
        } else {
            if(!ofm_->exist(dirfd)) {
                return RelativizeStatus::fd_unknown;
            } else {
                // check if we have the AT_EMPTY_PATH flag
                // for fstatat.
                if(flags & AT_EMPTY_PATH) {
                    relative_path = ofm_->get(dirfd)->path();
                    return RelativizeStatus::internal;
                }
            }
            // path is relative to fd
            auto dir = ofm_->get_dir(dirfd);
            if(dir == nullptr) {
                return RelativizeStatus::fd_not_a_dir;
            }
            path = mountdir_;
            path.append(dir->path());
            path.push_back(gkfs::path::separator);
            path.append(raw_path);
        }
    } else {
        path = raw_path;
    }

    if(gkfs::path::resolve(path, relative_path, resolve_last_link)) {
        return RelativizeStatus::internal;
    }
    return RelativizeStatus::external;
}

bool
PreloadContext::relativize_path(const char* raw_path,
                                std::string& relative_path,
                                bool resolve_last_link) const {
    // Relativize path should be called only after the library constructor has
    // been executed
    assert(interception_enabled_);
    // If we run the constructor we also already setup the mountdir
    assert(!mountdir_.empty());

    // We assume raw path is valid
    assert(raw_path != nullptr);

    std::string path;

    if(raw_path != nullptr && raw_path[0] != gkfs::path::separator) {
        /* Path is not absolute, we need to prepend CWD;
         * First reserve enough space to minimize memory copy
         */
        path = gkfs::path::prepend_path(cwd_, raw_path);
    } else {
        path = raw_path;
    }

    return gkfs::path::resolve(path, relative_path, resolve_last_link);
}

const std::shared_ptr<gkfs::filemap::OpenFileMap>&
PreloadContext::file_map() const {
    return ofm_;
}

void
PreloadContext::distributor(std::shared_ptr<gkfs::rpc::Distributor> d) {
    distributor_ = d;
}

std::shared_ptr<gkfs::rpc::Distributor>
PreloadContext::distributor() const {
    return distributor_;
}

const std::shared_ptr<FsConfig>&
PreloadContext::fs_conf() const {
    return fs_conf_;
}

void
PreloadContext::enable_interception() {
    interception_enabled_ = true;
}

void
PreloadContext::disable_interception() {
    interception_enabled_ = false;
}

bool
PreloadContext::interception_enabled() const {
    return interception_enabled_;
}

int
PreloadContext::register_internal_fd(int fd) {

    assert(fd >= 0);

    if(!internal_fds_must_relocate_) {
        LOG(DEBUG, "registering fd {} as internal (no relocation needed)", fd);
        assert(fd >= MIN_INTERNAL_FD);
        internal_fds_.reset(fd - MIN_INTERNAL_FD);
        return fd;
    }

    LOG(DEBUG, "registering fd {} as internal (needs relocation)", fd);

    std::lock_guard<std::mutex> lock(internal_fds_mutex_);
    const int pos = internal_fds_._Find_first();

    if(static_cast<std::size_t>(pos) == internal_fds_.size()) {
        throw std::runtime_error(
                "Internal GekkoFS file descriptors exhausted, increase MAX_INTERNAL_FDS in "
                "CMake, rebuild GekkoFS and try again.");
    }
    internal_fds_.reset(pos);


#if defined(GKFS_ENABLE_LOGGING) && defined(GKFS_DEBUG_BUILD)
    long args[gkfs::syscall::MAX_ARGS]{fd, pos + MIN_INTERNAL_FD, O_CLOEXEC};
#endif

    LOG(SYSCALL,
        gkfs::syscall::from_internal_code | gkfs::syscall::to_kernel |
                gkfs::syscall::not_executed,
        SYS_dup3, args);

    const int ifd = ::syscall_no_intercept(SYS_dup3, fd, pos + MIN_INTERNAL_FD,
                                           O_CLOEXEC);

    LOG(SYSCALL,
        gkfs::syscall::from_internal_code | gkfs::syscall::to_kernel |
                gkfs::syscall::executed,
        SYS_dup3, args, ifd);

    assert(::syscall_error_code(ifd) == 0);

#if defined(GKFS_ENABLE_LOGGING) && defined(GKFS_DEBUG_BUILD)
    long args2[gkfs::syscall::MAX_ARGS]{fd};
#endif

    LOG(SYSCALL,
        gkfs::syscall::from_internal_code | gkfs::syscall::to_kernel |
                gkfs::syscall::not_executed,
        SYS_close, args2);

#if defined(GKFS_ENABLE_LOGGING) && defined(GKFS_DEBUG_BUILD)
    int rv = ::syscall_no_intercept(SYS_close, fd);
#else
    ::syscall_no_intercept(SYS_close, fd);
#endif

    LOG(SYSCALL,
        gkfs::syscall::from_internal_code | gkfs::syscall::to_kernel |
                gkfs::syscall::executed,
        SYS_close, args2, rv);

    LOG(DEBUG, "    (fd {} relocated to ifd {})", fd, ifd);

    return ifd;
}

void
PreloadContext::unregister_internal_fd(int fd) {

    LOG(DEBUG, "unregistering internal fd {}", fd);

    assert(fd >= MIN_INTERNAL_FD);

    const auto pos = fd - MIN_INTERNAL_FD;

    std::lock_guard<std::mutex> lock(internal_fds_mutex_);
    internal_fds_.set(pos);
}

bool
PreloadContext::is_internal_fd(int fd) const {

    if(fd < MIN_INTERNAL_FD) {
        return false;
    }

    const auto pos = fd - MIN_INTERNAL_FD;

    std::lock_guard<std::mutex> lock(internal_fds_mutex_);
    return !internal_fds_.test(pos);
}
//把所有0-max user fds中的文件描述符全部占用
void
PreloadContext::protect_user_fds() {

    LOG(DEBUG, "Protecting application fds [{}, {}]", 0, MAX_USER_FDS - 1);

    const int nullfd =
            ::syscall_no_intercept(SYS_openat, 0, "/dev/null", O_RDONLY);
    assert(::syscall_error_code(nullfd) == 0);
    protected_fds_.set(nullfd);

    const auto fd_is_open = [](int fd) -> bool {
        const int ret = ::syscall_no_intercept(SYS_fcntl, fd, F_GETFD);
        const int error = ::syscall_error_code(ret);
        return error == 0 || error != EBADF;
    };

    for(int fd = 0; fd < MAX_USER_FDS; ++fd) {
        if(fd_is_open(fd)) {
            LOG(DEBUG, "  fd {} was already in use, skipping", fd);
            continue;
        }

        const int ret = ::syscall_no_intercept(SYS_dup3, nullfd, fd, O_CLOEXEC);
        assert(::syscall_error_code(ret) == 0);
        protected_fds_.set(fd);
    }

    internal_fds_must_relocate_ = false;
}

void
PreloadContext::unprotect_user_fds() {

    for(std::size_t fd = 0; fd < protected_fds_.size(); ++fd) {
        if(!protected_fds_[fd]) {
            continue;
        }

        const int ret =
                ::syscall_error_code(::syscall_no_intercept(SYS_close, fd));

        if(ret != 0) {
            LOG(ERROR, "Failed to unprotect fd")
        }
    }

    internal_fds_must_relocate_ = true;
}


std::string
PreloadContext::get_hostname() {
    return hostname;
}

} // namespace preload
} // namespace gkfs
