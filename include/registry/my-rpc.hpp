/*
 * (C) 2015 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */

#ifndef __MY_RPC
#define __MY_RPC

#include <margo.h>

/* visible API for example RPC operation */

DECLARE_MARGO_RPC_HANDLER(rpc_srv_registry_request)

DECLARE_MARGO_RPC_HANDLER(my_rpc_shutdown_ult)

#endif /* __MY_RPC */
