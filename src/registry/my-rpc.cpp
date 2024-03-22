/*
 * (C) 2015 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */

#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <iostream>

#include <registry/my-rpc.hpp>

#include <common/rpc/rpc_types.hpp>
/**
 * @brief Responds with general file system meta information requested on client
 * startup.
 * @internal
 * Most notably this is where the client gets the information on which path
 * GekkoFS is accessible.
 * @endinteral
 * @param handle Mercury RPC handle
 * @return Mercury error code to Mercury
 */
hg_return_t
rpc_srv_registry_request(hg_handle_t handle)
{   
    std::cout<< "succeed in getting flows hfile hcfile\n" <<std::endl;
    rpc_registry_request_in_t in;
    rpc_registry_request_out_t out;

    auto ret = margo_get_input(handle, &in);
    if(ret != HG_SUCCESS)
    assert(ret == HG_SUCCESS);
    auto flows = in.merge_flows;
    auto hfile = in.merge_hfile;
    auto hcfile = in.merge_hcfile;
    try {
        // create metadentry
        std::cout<< "succeed in getting flows hfile hcfile\n" << flows << "\n" 
                 << hfile << "\n" << hcfile << std::endl;
    } catch(const std::exception& e) {
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

static void my_rpc_shutdown_ult(hg_handle_t handle)
{
    hg_return_t       hret;
    margo_instance_id mid;

    printf("Got RPC request to shutdown\n");

    /* get handle info and margo instance */
    mid = margo_hg_handle_get_instance(handle);
    assert(mid != MARGO_INSTANCE_NULL);

    hret = margo_respond(handle, NULL);
    assert(hret == HG_SUCCESS);

    margo_destroy(handle);
    //margo_finalize(mid);

    return;
}
DEFINE_MARGO_RPC_HANDLER(my_rpc_shutdown_ult)
