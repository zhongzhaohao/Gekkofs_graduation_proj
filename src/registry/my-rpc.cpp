/*
 * (C) 2015 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */

#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <fstream>
#include <set>
#include <queue>
#include <registry/my-rpc.hpp>

#include <common/rpc/rpc_types.hpp>

static int merge_files(){
    return 0;

}

/**
 * @brief Responds with general file system meta information requested on client
 * startup.
 * @internal
 * Most notably this is where the client gets the information on which path
 * @endinterals
 * @param handle Mercury RPC handle
 * @return Mercury error code to Mercury
 */
hg_return_t
rpc_srv_registry_request(hg_handle_t handle)
{   
    std::cout<< "succeed in getting flows hfile hcfile\n" <<std::endl;
    rpc_registry_request_in_t in;
    rpc_registry_request_out_t out;
    std::vector<std::string> flow_arr = {};

    auto ret = margo_get_input(handle, &in);
    if(ret != HG_SUCCESS)
    assert(ret == HG_SUCCESS);
    auto flows = in.merge_flows;
    auto hfile = in.merge_hfile;
    auto hcfile = in.merge_hcfile;
    std::cout<< "succeed in getting flows hfile hcfile\n" << flows << "\n" 
                 << hfile << "\n" << hcfile << std::endl;
    try {
            std::stringstream ss(flows);
            std::string flow;
            while (std::getline(ss, flow, ';')) {
                flow_arr.push_back(flow);
            }
            std::priority_queue<fs_info> all_fs_info;
            std::set<std::string> all_daemons;
            for(int i = 0 ; i < flow_arr.size(); i ++){
                if(!job_flows.count(flow_arr[i])) {
                    std::cout<< "we can't find flow names " << flow_arr[i] <<std::endl;
                    continue;
                }
                std::ifstream hcf(job_flows[flow_arr[i]].first);
                std::ifstream hf(job_flows[flow_arr[i]].second);

                if (hcf.is_open() && hf.is_open()) {
                    std::string line;
                    while (std::getline(hcf, line)) {
                        struct fs_info fsinfo;
                        fsinfo.priority = i;

                        std::stringstream ss(line);
                        unsigned int fsdaemons,fspriority;
                        ss>> fsdaemons >> fspriority;
                        fsinfo.post_priority = fspriority;

                        for (int k = 0; k < fsdaemons; ++k) {
                            if (std::getline(hf, line)) {
                                if(all_daemons.count(line))
                                    continue;
                                all_daemons.insert(line);
                                fsinfo.daemon_addrs.push_back(line);
                            }
                                
                        }
                        if(fsinfo.daemon_addrs.size() > 0)
                            all_fs_info.push(fsinfo);
                        
                    }
                }
                hcf.close();
                hf.close();
            }

        std::ofstream hcf(hcfile);
        std::ofstream hf(hfile);
        unsigned int prior = 1;
        while(!all_fs_info.empty()){
            fs_info fsinfo = all_fs_info.top();
            all_fs_info.pop();
            hcf << fsinfo.daemon_addrs.size() << " " << prior <<std::endl;
            prior ++;
            for(auto addr : fsinfo.daemon_addrs){
                hf << addr << std::endl;
            }
        }
        hcf.close();
        hf.close();

    }   catch(const std::exception& e) {
            std::cout<< "Failed to respond my rpc ult\n" << e.what();
            out.err = -1;
    }

    std::cout<< "out err" << out.err <<std::endl;
    auto hret = margo_respond(handle, &out);
    if(hret != HG_SUCCESS) {
        std::cout<< "Failed to respond my rpc ult\n";
    }

    margo_free_input(handle, &in);
    margo_destroy(handle);
    return HG_SUCCESS;
}
DEFINE_MARGO_RPC_HANDLER(rpc_srv_registry_request)

hg_return_t
rpc_srv_registry_register(hg_handle_t handle)
{
    std::cout<< "succeed in getting flows hfile hcfile\n" <<std::endl;
    rpc_registry_register_in_t in;
    rpc_err_out_t out;

    auto ret = margo_get_input(handle, &in);
    if(ret != HG_SUCCESS)
    assert(ret == HG_SUCCESS);
    auto flow = in.work_flow;
    auto hfile = in.hfile;
    auto hcfile = in.hcfile;
    try {
        // create metadentry
        job_flows[flow] = {hcfile,hfile} ;
        std::cout<< "succeed in getting flows hcfile hfile\n" << flow << "\n" 
                 << job_flows[flow].first << "\n" << job_flows[flow].second << std::endl;
    } catch(const std::exception& e) {
        std::cout<< "Failed to respond my rpc ult\n" << e.what();
        out.err = -1;
    }

    std::cout<< "out err" << out.err <<std::endl;
    auto hret = margo_respond(handle, &out);
    if(hret != HG_SUCCESS) {
        std::cout<< "Failed to respond my rpc ult\n";
    }

    std::cout<< "here gives the contents of our map" <<std::endl;
     for (const auto& pair : job_flows) {
        std::cout << pair.first << " => " << pair.second.first << " "<< pair.second.second << std::endl;
    } 
    std::cout<< std::endl;
    margo_free_input(handle, &in);
    margo_destroy(handle);
    return HG_SUCCESS;
}
DEFINE_MARGO_RPC_HANDLER(rpc_srv_registry_register)
