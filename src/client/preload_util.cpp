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

#include <client/preload_util.hpp>
#include <client/env.hpp>
#include <client/logging.hpp>
#include <client/rpc/forward_metadata.hpp>

#include <common/rpc/distributor.hpp>
#include <common/rpc/rpc_util.hpp>
#include <common/env_util.hpp>
#include <common/common_defs.hpp>

#include <hermes.hpp>

#include <fstream>
#include <sstream>
#include <regex>
#include <csignal>
#include <random>

extern "C" {
#include <sys/sysmacros.h>
}

using namespace std;

namespace {

/**
 * Looks up a host endpoint via Hermes
 * @param uri
 * @param max_retries
 * @return hermes endpoint, if successful
 * @throws std::runtime_error
 */
hermes::endpoint
lookup_endpoint(const std::string& uri, std::size_t max_retries = 3) {

    LOG(DEBUG, "Looking up address \"{}\"", uri);

    std::random_device rd; // obtain a random number from hardware
    std::size_t attempts = 0;
    std::string error_msg;

    do {
        try {
            std::cout<< "looking up " << uri << std::endl;
            return ld_network_service->lookup(uri);
        } catch(const exception& ex) {
            error_msg = ex.what();

            LOG(WARNING, "Failed to lookup address '{}'. Attempts [{}/{}]", uri,
                attempts + 1, max_retries);

            // Wait a random amount of time and try again
            std::mt19937 g(rd()); // seed the random generator
            std::uniform_int_distribution<> distr(
                    50, 50 * (attempts + 2)); // define the range
            std::this_thread::sleep_for(std::chrono::milliseconds(distr(g)));
            continue;
        }
    } while(++attempts < max_retries);

    throw std::runtime_error(
            fmt::format("Endpoint for address '{}' could not be found ({})",
                        uri, error_msg));
}

/**
 * extracts protocol from a given URI generated by the RPC server of the daemon
 * @param uri
 * @throws std::runtime_error
 */
void
extract_protocol(const string& uri) {
    if(uri.rfind("://") == string::npos) {
        // invalid format. kill client
        throw runtime_error(fmt::format("Invalid format for URI: '{}'", uri));
    }
    string protocol{};

    if(uri.find(gkfs::rpc::protocol::ofi_sockets) != string::npos) {
        protocol = gkfs::rpc::protocol::ofi_sockets;
    } else if(uri.find(gkfs::rpc::protocol::ofi_psm2) != string::npos) {
        protocol = gkfs::rpc::protocol::ofi_psm2;
    } else if(uri.find(gkfs::rpc::protocol::ofi_verbs) != string::npos) {
        protocol = gkfs::rpc::protocol::ofi_verbs;
    }
    // check for shared memory protocol. Can be plain shared memory or real ofi
    // protocol + auto_sm
    if(uri.find(gkfs::rpc::protocol::na_sm) != string::npos) {
        if(protocol.empty())
            protocol = gkfs::rpc::protocol::na_sm;
        else
            CTX->auto_sm(true);
    }
    if(protocol.empty()) {
        // unsupported protocol. kill client
        throw runtime_error(fmt::format(
                "Unsupported RPC protocol found in hosts file with URI: '{}'",
                uri));
    }
    LOG(INFO,
        "RPC protocol '{}' extracted from hosts file. Using auto_sm is '{}'",
        protocol, CTX->auto_sm());
    CTX->rpc_protocol(protocol);
}

/**
 * Reads the daemon generator hosts file by a given path, returning hosts and
 * URI addresses
 * @param path to hosts file
 * @return vector<pair<hosts, URI>>
 * @throws std::runtime_error
 */
vector<pair<string, string>>
load_hostfile(const std::string& path) {

    LOG(DEBUG, "Loading hosts file: \"{}\"", path);

    ifstream lf(path);
    if(!lf) {
        throw runtime_error(fmt::format("Failed to open hosts file '{}': {}",
                                        path, strerror(errno)));
    }
    vector<pair<string, string>> hosts;
    const regex line_re("^(\\S+)\\s+(\\S+)$",
                        regex::ECMAScript | regex::optimize);
    string line;
    string host;
    string uri;
    std::smatch match;
    while(getline(lf, line)) {
        if(!regex_match(line, match, line_re)) {

            LOG(ERROR, "Unrecognized line format: [path: '{}', line: '{}']",
                path, line);

            throw runtime_error(
                    fmt::format("unrecognized line format: '{}'", line));
        }
        host = match[1];//changqin...
        uri = match[2];//ofi+sockets://192.....
        hosts.emplace_back(host, uri);
    }
    if(hosts.empty()) {
        throw runtime_error(
                "Hosts file found but no suitable addresses could be extracted");
    }
    //extract_protocol(hosts[0].second);//protocol at 0 -- 
    // sort hosts so that data always hashes to the same place during restart
    //std::sort(hosts.begin(), hosts.end());
    // remove rootdir suffix from host after sorting as no longer required
    for(auto& h : hosts) {
        auto idx = h.first.rfind("#");
        if(idx != string::npos)
            h.first.erase(idx, h.first.length());
    }
    return hosts;
}

} // namespace

namespace gkfs::utils {


/**
 * Retrieve metadata from daemon and return Metadata object
 * errno may be set
 * @param path
 * @param follow_links
 * @return Metadata
 */
optional<gkfs::metadata::Metadata>
get_metadata(const string& path, bool follow_links) {
    std::string attr;
    //std::cout<<"get_metatdata here"<<std::endl;
    auto err = gkfs::rpc::forward_stat(path, attr);
    if(err) {
        errno = err;
        return {};
    }
#ifdef HAS_SYMLINKS
    if(follow_links) {
        gkfs::metadata::Metadata md{attr};
        while(md.is_link()) {
            err = gkfs::rpc::forward_stat(md.target_path(), attr);
            if(err) {
                errno = err;
                return {};
            }
            md = gkfs::metadata::Metadata{attr};
        }
    }
#endif
    return gkfs::metadata::Metadata{attr};
}


/**
 * Converts the Metadata object into a stat struct, which is needed by Linux
 * @param path
 * @param md
 * @param attr
 * @return
 */
int
metadata_to_stat(const std::string& path, const gkfs::metadata::Metadata& md,
                 struct stat& attr) {

    /* Populate default values */
    attr.st_dev = makedev(0, 0);
    attr.st_ino = std::hash<std::string>{}(path);
    attr.st_nlink = 1;
    attr.st_uid = CTX->fs_conf()->uid;
    attr.st_gid = CTX->fs_conf()->gid;
    attr.st_rdev = 0;
    attr.st_blksize = gkfs::config::rpc::chunksize;
    attr.st_blocks = 0;

    memset(&attr.st_atim, 0, sizeof(timespec));
    memset(&attr.st_mtim, 0, sizeof(timespec));
    memset(&attr.st_ctim, 0, sizeof(timespec));

    attr.st_mode = md.mode();

#ifdef HAS_SYMLINKS
    if(md.is_link())
        attr.st_size = md.target_path().size() + CTX->mountdir().size();
    else
#endif
        attr.st_size = md.size();

    if(CTX->fs_conf()->atime_state) {
        attr.st_atim.tv_sec = md.atime();
    }
    if(CTX->fs_conf()->mtime_state) {
        attr.st_mtim.tv_sec = md.mtime();
    }
    if(CTX->fs_conf()->ctime_state) {
        attr.st_ctim.tv_sec = md.ctime();
    }
    if(CTX->fs_conf()->link_cnt_state) {
        attr.st_nlink = md.link_count();
    }
    if(CTX->fs_conf()->blocks_state) { // last one will not encounter a
                                       // delimiter anymore
        attr.st_blocks = md.blocks();
    }
    return 0;
}

#ifdef GKFS_ENABLE_FORWARDING
map<string, uint64_t>
load_forwarding_map_file(const std::string& lfpath) {

    LOG(DEBUG, "Loading forwarding map file file: \"{}\"", lfpath);

    ifstream lf(lfpath);
    if(!lf) {
        throw runtime_error(
                fmt::format("Failed to open forwarding map file '{}': {}",
                            lfpath, strerror(errno)));
    }
    map<string, uint64_t> forwarding_map;
    const regex line_re("^(\\S+)\\s+(\\S+)$",
                        regex::ECMAScript | regex::optimize);
    string line;
    string host;
    uint64_t forwarder;
    std::smatch match;
    while(getline(lf, line)) {
        if(!regex_match(line, match, line_re)) {

            LOG(ERROR, "Unrecognized line format: [path: '{}', line: '{}']",
                lfpath, line);

            throw runtime_error(
                    fmt::format("unrecognized line format: '{}'", line));
        }
        host = match[1];
        forwarder = std::stoi(match[2].str());
        forwarding_map[host] = forwarder;
    }
    return forwarding_map;
}
#endif

#ifdef GKFS_ENABLE_FORWARDING
void
load_forwarding_map() {
    string forwarding_map_file;

    forwarding_map_file = gkfs::env::get_var(
            gkfs::env::FORWARDING_MAP_FILE, gkfs::config::forwarding_file_path);

    map<string, uint64_t> forwarding_map;

    while(forwarding_map.size() == 0) {
        try {
            forwarding_map = load_forwarding_map_file(forwarding_map_file);
        } catch(const exception& e) {
            auto emsg = fmt::format("Failed to load forwarding map file: {}",
                                    e.what());
            throw runtime_error(emsg);
        }
    }

    // if (forwarding_map.size() == 0) {
    //    throw runtime_error(fmt::format("Forwarding map file is empty: '{}'",
    //    forwarding_map_file));
    //}

    auto local_hostname = gkfs::rpc::get_my_hostname(true);

    if(forwarding_map.find(local_hostname) == forwarding_map.end()) {
        throw runtime_error(
                fmt::format("Unable to determine the forwarder for host: '{}'",
                            local_hostname));
    }
    LOG(INFO, "Forwarding map loaded for '{}' as '{}'", local_hostname,
        forwarding_map[local_hostname]);

    CTX->fwd_host_id(forwarding_map[local_hostname]);
}
#endif

bool CheckMerge(string &mergeflows,string &hostfile,string &hostconfigfile){
    auto merge = gkfs::env::get_var(gkfs::env::MERGE,
                                  gkfs::config::merge_default);
    std::transform(merge.begin(),merge.end(),merge.begin(),::tolower);
    hostfile = gkfs::env::get_var(gkfs::env::HOSTS_FILE,
                                  gkfs::config::hostfile_path);  
    hostconfigfile = gkfs::env::get_var(gkfs::env::HOSTS_CONFIG_FILE,
                                  gkfs::config::hostfile_config_path);
    mergeflows = gkfs::env::get_var(gkfs::env::MERGE_FLOWS,
                                  "");
    bool hfexist = !access(hostfile.c_str(),F_OK);
    bool hcexist = !access(hostconfigfile.c_str(),F_OK);
    if(merge == "on"){
        return true;
        if(hfexist && hcexist) return false;
        if(!hfexist && !hcexist) {
            if(!mergeflows.length())
                throw runtime_error("Trying to merge empty workflows");
            return true;
        }else{
            throw runtime_error("Please make sure hostfile or hostconfigfile not existing before first merge");
        }
    }
    return false;
}

string
read_registry_file() {
    string registryfile;
    //first para is env host file path ,second is ./hosts.txt, return a path
    registryfile = gkfs::env::get_var(gkfs::env::REGISTRY_FILE,
                                  gkfs::config::registryfile_path);
    
    ifstream lf(registryfile);
    string addr;
    getline(lf, addr);
    if(addr.empty()) {
        throw runtime_error(fmt::format("Registryfile empty: '{}'", registryfile));
    }
    extract_protocol(addr);
    LOG(INFO, "Getting registry addr: {}",addr);
    std::cout<<"Getting registry addr: "<<addr<<std::endl;
    return addr;
}

vector<pair<string, string>>
read_hosts_file() {
    string hostfile;
    //first para is env host file path ,second is ./hosts.txt, return a path
    hostfile = gkfs::env::get_var(gkfs::env::HOSTS_FILE,
                                  gkfs::config::hostfile_path);
    
    vector<pair<string, string>> hosts;// name , protoc://socket
    try {
        hosts = load_hostfile(hostfile);
    } catch(const exception& e) {
        auto emsg = fmt::format("Failed to load hosts file: {}", e.what());
        throw runtime_error(emsg);
    }

    if(hosts.empty()) {
        throw runtime_error(fmt::format("Hostfile empty: '{}'", hostfile));
    }

    LOG(INFO, "Hosts pool size: {}", hosts.size());
    return hosts;
}

pair<vector<unsigned int>,vector<unsigned int> >
read_hosts_config_file() {
    string hostconfigfile;
    //first para is env host file path ,second is ./hosts.txt, return a path
    hostconfigfile = gkfs::env::get_var(gkfs::env::HOSTS_CONFIG_FILE,
                                  gkfs::config::hostfile_config_path);
    ifstream lf(hostconfigfile);
    string line;
    vector<unsigned int> hcfile,fspriority;
    while (getline(lf, line)){
        std::istringstream iss(line);
        unsigned int x,y;
        if(!(iss >> x >> y)){
            throw runtime_error(fmt::format("Invalid file format: '{}'", hostconfigfile));
        }
        hcfile.push_back(x);
        fspriority.push_back(y);
    }
    
    if(hcfile.empty()) {
        throw runtime_error(fmt::format("HostConfigfile empty: '{}'", hostconfigfile));
    }

    LOG(INFO, "Hosts config pool size: {}", hcfile.size());
    //std::cout<<"succeed in reading "<<hostconfigfile<<" hostfonfig "<<hcfile.size()<<std::endl;
    return {hcfile,fspriority};
}
/**
 * Connects to daemons and lookup Mercury URI addresses via Hermes
 * @param hosts vector<pair<hostname, Mercury URI address>>
 * @throws std::runtime_error through lookup_endpoint()
 */
void
connect_to_hosts(const vector<pair<string, string>>& hosts) {
    auto local_hostname = gkfs::rpc::get_my_hostname(true);
    bool local_host_found = false;

    std::vector<hermes::endpoint> addrs;
    addrs.resize(hosts.size());

    vector<uint64_t> host_ids(hosts.size());
    // populate vector with [0, ..., host_size - 1]
    ::iota(::begin(host_ids), ::end(host_ids), 0);
    for(const auto& id : host_ids) {
        const auto& hostname = hosts.at(id).first;
        if(!local_host_found && hostname == local_hostname) {
            LOG(DEBUG, "Found local host: {}", hostname);
            CTX->local_host_id(id);
            local_host_found = true;
        }
    }
    /*
     * Shuffle hosts to balance addr lookups to all hosts
     * Too many concurrent lookups send to same host
     * could overwhelm the server,
     * returning error when addr lookup
     */
    ::random_device rd; // obtain a random number from hardware
    ::mt19937 g(rd());  // seed the random generator
    ::shuffle(host_ids.begin(), host_ids.end(), g); // Shuffle hosts vector
    // lookup addresses and put abstract server addresses into rpc_addresses

    for(const auto& id : host_ids) {
        const auto& hostname = hosts.at(id).first;
        const auto& uri = hosts.at(id).second;
       //std::cout<< "id:"<< id<<" hostname + uri" << hostname<<" " << uri<<std::endl;
        addrs[id] = lookup_endpoint(uri);
        LOG(DEBUG, "Found peer: {}", addrs[id].to_string());
    }

    if(!local_host_found) {
        LOG(WARNING, "Failed to find local host. Using host '0' as local host");
        CTX->local_host_id(0);
    }
    int id = CTX->local_host_id();
    CTX->local_fs_id(0);
    for(unsigned int fs = 0;fs < CTX->hostsconfig().size(); fs++){
        id -= CTX->hostsconfig()[fs];
        if(id < 0) {
            CTX->local_fs_id(fs);
            break;
        }
    }
    CTX->hosts(addrs);
}

/**
 * Connects to registry and lookup Mercury URI addresses via Hermes
 * @param hosts vector<pair<hostname, Mercury URI address>>
 * @throws std::runtime_error through lookup_endpoint()
 */
void
connect_to_registry(std::string addr) {
    hermes::endpoint endp = lookup_endpoint(addr);
    LOG(DEBUG, "Found registry: {}", endp.to_string());
    CTX->registry(endp);
    std::cout<<"now is endp of registry" << endp.to_string()<<endl;
}

} // namespace gkfs::utils
