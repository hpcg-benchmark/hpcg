
/*
 * SPDX-FileCopyrightText: Copyright (c) 2024 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once
#ifdef USE_CUDA
#include "cublas_v2.h"
#include "cuda_runtime_api.h"
#include "cusparse.h"
#include <cuda.h>
#ifdef USE_NCCL
#include "nccl.h"
#define CHECK_NCCL(x)                                                                                                  \
    do                                                                                                                 \
    {                                                                                                                  \
        ncclResult_t ncclStatus = (x);                                                                                 \
        if (ncclStatus != ncclSuccess)                                                                                 \
        {                                                                                                              \
            fprintf(stderr, "NCCL: %s = %d (%s) at (%s:%d)\n", #x, ncclStatus, ncclGetErrorString(ncclStatus),         \
                __FILE__, __LINE__);                                                                                   \
            exit(1);                                                                                                   \
        }                                                                                                              \
    } while (0)
#endif
#ifdef USE_NVTX
#include <nvToolsExt.h>
#endif
#include <unistd.h>

extern cusparseHandle_t cusparsehandle;
extern cublasHandle_t cublashandle;
extern cudaStream_t stream;
extern cudaEvent_t copy_done;
extern cudaStream_t copy_stream;
extern int* ranktoId;   // DEV:Compress rank in MPI_WORLD to Neighbors
extern int* rankToId_h; // HOST:Compress rank in MPI_WORLD to Neighbors
extern int* idToRank_h;
extern bool Use_Compression;        /*USE CUDA L2 compression*/
extern bool Use_Hpcg_Mem_Reduction; /*USE HPCG aggresive memory reduction*/
#endif

#ifdef USE_CUDA
#define CHECK_CUDART(x)                                                                                                \
    do                                                                                                                 \
    {                                                                                                                  \
        cudaError_t res = (x);                                                                                         \
        if (res != cudaSuccess)                                                                                        \
        {                                                                                                              \
            char rank_name[1024];                                                                                      \
            gethostname(rank_name, 1024);                                                                              \
            fprintf(stderr, "CUDART: %s = %d (%s) on %s at (%s:%d)\n", #x, res, cudaGetErrorString(res), rank_name,    \
                __FILE__, __LINE__);                                                                                   \
            exit(1);                                                                                                   \
        }                                                                                                              \
    } while (0)

#define CHECK_CUBLAS(x)                                                                                                \
    do                                                                                                                 \
    {                                                                                                                  \
        cublasStatus_t cublasStatus = (x);                                                                             \
        if (cublasStatus != CUBLAS_STATUS_SUCCESS)                                                                     \
        {                                                                                                              \
            fprintf(stderr, "CUBLAS: %s = %d at (%s:%d)\n", #x, cublasStatus, __FILE__, __LINE__);                     \
            exit(1);                                                                                                   \
        }                                                                                                              \
    } while (0)

#define CHECK_CUSPARSE(x)                                                                                              \
    do                                                                                                                 \
    {                                                                                                                  \
        cusparseStatus_t cusparseStatus = (x);                                                                         \
        if (cusparseStatus != CUSPARSE_STATUS_SUCCESS)                                                                 \
        {                                                                                                              \
            fprintf(stderr, "CUSPARSE: %s = %d (%s) at (%s:%d)\n", #x, (int)cusparseStatus,                             \
                cusparseGetErrorString(cusparseStatus), __FILE__, __LINE__);                                           \
            exit(1);                                                                                                   \
        }                                                                                                              \
    } while (0)

// IF NVTX is needed for profiling, please define USE_NVTX
// Then, add PUSH_RANGE and POP_RANGE around the target code block
// See, https://developer.nvidia.com/blog/cuda-pro-tip-generate-custom-application-profile-timelines-nvtx/
// #define USE_NVTX
#ifdef USE_NVTX
const uint32_t colors[] = {0xff00ff00, 0xff0000ff, 0xffffff00, 0xffff00ff, 0xff00ffff, 0xffff0000, 0xffffffff};
const int num_colors = sizeof(colors) / sizeof(uint32_t);
#define PUSH_RANGE(name, cid)                                                                                          \
    {                                                                                                                  \
        int color_id = cid;                                                                                            \
        color_id = color_id % num_colors;                                                                              \
        nvtxEventAttributes_t eventAttrib = {0};                                                                       \
        eventAttrib.version = NVTX_VERSION;                                                                            \
        eventAttrib.size = NVTX_EVENT_ATTRIB_STRUCT_SIZE;                                                              \
        eventAttrib.colorType = NVTX_COLOR_ARGB;                                                                       \
        eventAttrib.color = colors[color_id];                                                                          \
        eventAttrib.messageType = NVTX_MESSAGE_TYPE_ASCII;                                                             \
        eventAttrib.message.ascii = name;                                                                              \
        nvtxRangePushEx(&eventAttrib);                                                                                 \
    }
#define POP_RANGE nvtxRangePop();
#else
#define PUSH_RANGE(name, cid)                                                                                          \
    {                                                                                                                  \
    }
#define POP_RANGE
#endif
#endif