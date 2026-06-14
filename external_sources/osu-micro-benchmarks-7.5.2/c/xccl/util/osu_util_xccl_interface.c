/*
 * Copyright (c) 2002-2025 the network-based computing laboratory
 * (NBCL), The Ohio State University.
 *
 * Contact: dr. D. K. Panda (panda@cse.ohio-state.edu)
 *
 * For detailed copyright and licensing information, please refer to the
 * copyright file copyright in the top level omb directory.
 */
#include "osu_util_xccl_interface.h"
#include "rccl/osu_util_rccl_impl.h"
#include "nccl/osu_util_nccl_impl.h"

ncclComm_t nccl_comm;
OMB_ACC_STREAM nccl_stream;

omb_xccl_int_t *omb_xccl_interface_inject()
{
#if defined(_ENABLE_NCCL_)
    fprintf(stdout, "#Using NCCL\n");
    fflush(stdout);
    return omb_nccl_impl_inject();
#elif defined(_ENABLE_RCCL_)
    fprintf(stdout, "#Using RCCL\n");
    fflush(stdout);
    return omb_rccl_impl_inject();
#endif
}

void omb_force_accelerator()
{
#if defined(_ENABLE_NCCL_)
    options.accel = CUDA;
#elif defined(_ENABLE_RCCL_)
    options.accel = ROCM;
#endif
    if (PT2PT == options.bench) {
        options.src = 'D';
        options.dst = 'D';
    }
}

omb_xccl_int_t *omb_xccl_interface_alloc()
{
    omb_xccl_int_t *omb_xccl_int = malloc(sizeof(omb_xccl_int_t));
    OMB_CHECK_NULL_AND_EXIT(omb_xccl_int, "Unable to allocate memory");
    omb_xccl_int->xccl_stream = 0;
    omb_xccl_int->xccl_comm = NULL;
    omb_xccl_int->allocate_xccl_stream = NULL;
    omb_xccl_int->create_xccl_comm = NULL;
    omb_xccl_int->deallocate_xccl_stream = NULL;
    omb_xccl_int->destroy_xccl_comm = NULL;
    omb_xccl_int->synchronize_xccl_stream = NULL;
    omb_xccl_int->ncclReduce = NULL;
    omb_xccl_int->ncclBroadcast = NULL;
    omb_xccl_int->ncclAllReduce = NULL;
    omb_xccl_int->ncclAllToAll = NULL;
    omb_xccl_int->ncclReduceScatter = NULL;
    omb_xccl_int->ncclAllGather = NULL;
    omb_xccl_int->ncclSend = NULL;
    omb_xccl_int->ncclRecv = NULL;
    omb_xccl_int->ncclGroupStart = NULL;
    omb_xccl_int->ncclGroupEnd = NULL;
    return omb_xccl_int;
}

void omb_xccl_interface_free(omb_xccl_int_t *omb_xccl_int)
{
    free(omb_xccl_int);
}
