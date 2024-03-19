/*
 * (C) 2015 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */

#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <register/my-rpc.hpp>

/* my-rpc:
 * This is an example RPC operation.  It includes a small bulk transfer,
 * driven by the server, that moves data from the client to the server.  The
 * server writes the data to a local file in /tmp.
 */

/* The rpc handler is defined as a single ULT in Argobots.  It uses
 * underlying asynchronous operations for the HG transfer, open, write, and
 * close.
 */

static void my_rpc_ult(hg_handle_t handle)
{
    hg_return_t           hret;
    my_rpc_out_t          out;
    my_rpc_in_t           in;
    hg_size_t             size;
    void*                 buffer;
    hg_bulk_t             bulk_handle;
    const struct hg_info* hgi;
    char*                 state_file_name;
    margo_instance_id mid;
    hret = margo_get_input(handle, &in);
    assert(hret == HG_SUCCESS);
    out.ret = 0;
    size = in.input_val;
    buffer = malloc(in.input_val);
    assert(buffer);

    /* get handle info and margo instance */
    hgi = margo_get_info(handle);
    assert(hgi);
    mid = margo_hg_info_get_instance(hgi);
    assert(mid != MARGO_INSTANCE_NULL);

    if (in.dump_state) {
        margo_state_dump(mid, "margo-example-server", 1, &state_file_name);
        printf("# Runtime state dumped to %s\n", state_file_name);
        free(state_file_name);
    }

    /* register local target buffer for bulk access */
    hret = margo_bulk_create(mid, 1, &buffer, &size, HG_BULK_WRITE_ONLY,
                             &bulk_handle);
    assert(hret == HG_SUCCESS);
    /* do bulk transfer from client to server */
    hret = margo_bulk_transfer(mid, HG_BULK_PULL, hgi->addr, in.bulk_handle, 0,
                               bulk_handle, 0, size);
    assert(hret == HG_SUCCESS);
    //printf("i got %s",(char*)buffer);
    char * fsave = (char*)malloc(100);
    strcpy(fsave,"/home/changqin/output/out.txt");
    int cnt =0;
    while(access(fsave, F_OK) != -1){
        //printf("now cnt :%d\n",cnt);
        char *filename = strrchr(fsave, '/');
        //printf("file %s\n",filename);
        char * tmp =  (char*)malloc(20);
        //sprintf(tmp, "out%d.txt\0", cnt);
        //printf("tmp %s \n",tmp);
        strcpy(filename + 1, tmp);
        //printf("now file %s\n",fsave);
        cnt++;
    }
    FILE * fd = fopen(fsave,"wb");
    //printf("buffer size %d",sizeof(*buffer));
    fwrite(buffer,1,size,fd);
    fclose(fd);
    /* write to a file; would be done with abt-io if we enabled it */

    margo_free_input(handle, &in);

    hret = margo_respond(handle, &out);
    assert(hret == HG_SUCCESS);

    margo_bulk_free(bulk_handle);
    margo_destroy(handle);
    free(buffer);

    return;
}
DEFINE_MARGO_RPC_HANDLER(my_rpc_ult)

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
