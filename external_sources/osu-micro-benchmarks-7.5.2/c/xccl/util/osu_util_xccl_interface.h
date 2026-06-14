/*
 * Copyright (c) 2002-2025 the network-based computing laboratory
 * (NBCL), The Ohio State University.
 *
 * Contact: dr. D. K. Panda (panda@cse.ohio-state.edu)
 *
 * For detailed copyright and licensing information, please refer to the
 * copyright file copyright in the top level omb directory.
 */
#ifndef OSU_UTIL_NCCL_INTERFACE_H
#define OSU_UTIL_NCCL_INTERFACE_H

#if defined(_ENABLE_NCCL_) && defined(_ENABLE_RCCL_)
#error "Configure with either NCCL or RCCL"
#endif

#ifdef _ENABLE_NCCL_
#define OMB_XCCL_INCLUDE         <nccl.h>
#define OMB_ACC_STREAM           cudaStream_t
#define OMB_XCCL_TYPE_STR        "NCCL"
#define OMB_XCCL_TYPE_BENCH_NAME "nccl"
#define OMB_XCCL_ACC_TYPE        CUDA
#endif

#ifdef _ENABLE_RCCL_
#define OMB_XCCL_INCLUDE         <rccl.h>
#define OMB_ACC_STREAM           hipStream_t
#define OMB_XCCL_TYPE_STR        "RCCL"
#define OMB_XCCL_TYPE_BENCH_NAME "rccl"
#define OMB_XCCL_ACC_TYPE        ROCM
#endif

#include "osu_util_mpi.h"
#include OMB_XCCL_INCLUDE

#define OSU_NCCL_REDUCE_PARAM                                                  \
    const void *sendbuff, void *recvbuff, size_t count,                        \
        ncclDataType_t datatype, ncclRedOp_t op, int root, ncclComm_t comm,    \
        OMB_ACC_STREAM stream

#define OSU_NCCL_BROADCAST_PARAM                                               \
    const void *sendbuff, void *recvbuff, size_t count,                        \
        ncclDataType_t datatype, int root, ncclComm_t comm,                    \
        OMB_ACC_STREAM stream

#define OSU_NCCL_ALLREDUCE_PARAM                                               \
    const void *sendbuff, void *recvbuff, size_t count,                        \
        ncclDataType_t datatype, ncclRedOp_t op, ncclComm_t comm,              \
        OMB_ACC_STREAM stream

#define OSU_NCCL_REDUCESCATTER_PARAM                                           \
    const void *sendbuff, void *recvbuff, size_t recvcount,                    \
        ncclDataType_t datatype, ncclRedOp_t op, ncclComm_t comm,              \
        OMB_ACC_STREAM stream

#define OSU_NCCL_ALLGATHER_PARAM                                               \
    const void *sendbuff, void *recvbuff, size_t sendcount,                    \
        ncclDataType_t datatype, ncclComm_t comm, OMB_ACC_STREAM stream

#define OSU_NCCL_SEND_PARAM                                                    \
    const void *sendbuff, size_t count, ncclDataType_t datatype, int peer,     \
        ncclComm_t comm, OMB_ACC_STREAM stream

#define OSU_NCCL_RECV_PARAM                                                    \
    void *recvbuff, size_t count, ncclDataType_t datatype, int peer,           \
        ncclComm_t comm, OMB_ACC_STREAM stream

#define OSU_NCCL_ALLTOALL_PARAM                                                \
    const void *sendbuff, void *recvbuff, size_t count,                        \
        ncclDataType_t datatype, ncclComm_t comm, OMB_ACC_STREAM stream

/* ============================================================ */

#define NCCL_CHECK(cmd)                                                        \
    do {                                                                       \
        ncclResult_t r = cmd;                                                  \
        if (r != ncclSuccess) {                                                \
            fprintf(stderr,                                                    \
                    "Failed: " OMB_XCCL_TYPE_STR " Error %s: %d '%s'\n",       \
                    __FILE__, __LINE__, ncclGetErrorString(cmd));              \
            exit(EXIT_FAILURE);                                                \
        }                                                                      \
    } while (0)

/* ============================================================ */
/* NCCL Implementation struct */
typedef struct omb_xccl_int_t {
    OMB_ACC_STREAM xccl_stream;
    ncclComm_t xccl_comm;
    void (*allocate_xccl_stream)();
    void (*create_xccl_comm)(int r_ranks, int rank);
    void (*deallocate_xccl_stream)();
    void (*destroy_xccl_comm)();
    void (*synchronize_xccl_stream)();
    ncclResult_t (*ncclAllToAll)(OSU_NCCL_ALLTOALL_PARAM);
    ncclResult_t (*ncclReduce)(OSU_NCCL_REDUCE_PARAM);
    ncclResult_t (*ncclBroadcast)(OSU_NCCL_BROADCAST_PARAM);
    ncclResult_t (*ncclAllReduce)(OSU_NCCL_ALLREDUCE_PARAM);
    ncclResult_t (*ncclReduceScatter)(OSU_NCCL_REDUCESCATTER_PARAM);
    ncclResult_t (*ncclAllGather)(OSU_NCCL_ALLGATHER_PARAM);
    ncclResult_t (*ncclSend)(OSU_NCCL_SEND_PARAM);
    ncclResult_t (*ncclRecv)(OSU_NCCL_RECV_PARAM);
    ncclResult_t (*ncclGroupStart)();
    ncclResult_t (*ncclGroupEnd)();
} omb_xccl_int_t;

omb_xccl_int_t *omb_xccl_interface_inject();
omb_xccl_int_t *omb_xccl_interface_alloc();
void omb_xccl_interface_free(omb_xccl_int_t *i);
void omb_force_accelerator();

/* ============================================================ */
/* Core variables */
extern OMB_ACC_STREAM nccl_stream;
extern ncclComm_t nccl_comm;
#endif
