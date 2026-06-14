/*
 * Copyright (c) 2002-2025 the Network-Based Computing Laboratory
 * (NBCL), The Ohio State University.
 *
 * Contact: Dr. D. K. Panda (panda@cse.ohio-state.edu)
 *
 * For detailed copyright and licensing information, please refer to the
 * copyright file COPYRIGHT in the top level OMB directory.
 */
#include "osu_util_rccl_impl.h"

omb_xccl_int_t *omb_rccl_impl_inject()
{
    omb_xccl_int_t *omb_xccl_int = omb_xccl_interface_alloc();
    omb_xccl_int->xccl_stream = 0;
    omb_xccl_int->xccl_comm = NULL;
    omb_xccl_int->allocate_xccl_stream = _allocate_stream;
    omb_xccl_int->create_xccl_comm = _create_comm;
    omb_xccl_int->deallocate_xccl_stream = _deallocate_stream;
    omb_xccl_int->destroy_xccl_comm = _destroy_comm;
    omb_xccl_int->synchronize_xccl_stream = _synchronize_stream;
    omb_xccl_int->ncclAllToAll = ncclAllToAll;
    omb_xccl_int->ncclReduce = ncclReduce;
    omb_xccl_int->ncclBroadcast = ncclBroadcast;
    omb_xccl_int->ncclAllReduce = ncclAllReduce;
    omb_xccl_int->ncclReduceScatter = ncclReduceScatter;
    omb_xccl_int->ncclAllGather = ncclAllGather;
    omb_xccl_int->ncclSend = ncclSend;
    omb_xccl_int->ncclRecv = ncclRecv;
    omb_xccl_int->ncclGroupStart = ncclGroupStart;
    omb_xccl_int->ncclGroupEnd = ncclGroupEnd;
    return omb_xccl_int;
}

static void _create_comm(int num_ranks, int rank)
{
    ncclUniqueId nccl_comm_id;
    if (0 == rank) {
        /* Generates an Id to be used in ncclCommInitRank. */
        ncclGetUniqueId(&nccl_comm_id);
    }
    /*
     * ncclGetUniqueId should be called once when creating a
     * communicator and the Id should be distributed to all
     * ranks in the communicator before calling ncclCommInitRank
     */
    MPI_CHECK(MPI_Bcast((void *)&nccl_comm_id, sizeof(nccl_comm_id), MPI_BYTE,
                        0, MPI_COMM_WORLD));
    /* Create a new NCCL communicator */
    NCCL_CHECK(ncclCommInitRank(&nccl_comm, num_ranks, nccl_comm_id, rank));
}

static void _destroy_comm() { NCCL_CHECK(ncclCommDestroy(nccl_comm)); }

static void _allocate_stream()
{
    ROCM_CHECK(hipStreamCreateWithFlags(&nccl_stream, hipStreamNonBlocking));
}

static void _deallocate_stream() { ROCM_CHECK(hipStreamDestroy(nccl_stream)); }

static void _synchronize_stream()
{
    hipError_t err = hipErrorNotReady;
    int flag = 0;
    while (err == hipErrorNotReady) {
        err = hipStreamQuery(nccl_stream);
        MPI_CHECK(MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &flag,
                             MPI_STATUS_IGNORE));
    }
    ROCM_CHECK(err);
}
