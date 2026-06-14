
//@HEADER
// ***************************************************
//
// HPCG: High Performance Conjugate Gradient Benchmark
//
// Contact:
// Michael A. Heroux ( maherou@sandia.gov)
// Jack Dongarra     (dongarra@eecs.utk.edu)
// Piotr Luszczek    (luszczek@eecs.utk.edu)
//
// ***************************************************
//@HEADER

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

/*!
 @file hpcg.hpp

 HPCG data structures and functions
 */

/*
Hitory:
  *05.28.2023: HPC-Benchmark 23.5 release
*/

#ifndef HPCG_HPP
#define HPCG_HPP

#include "Geometry.hpp"
#include <fstream>

#ifndef USE_CUDA
#if defined(__x86_64__) || defined(__x86_64) || defined(_M_AMD64) || defined(__amd64__) || defined(__amd64)            \
    || defined(_M_X64)
#define USE_CUDA
#endif
#endif

#ifdef USE_CUDA
#include "Cuda.hpp"
#endif

#define XSTR(s) STR(s)
#define STR(s) #s

#define EMPTY_MACRO_ 1
#define CHECK_EMPTY_MACRO_(x) EMPTY_MACRO_##x
#define CHECK_EMPTY_MACRO(x) CHECK_EMPTY_MACRO_(x)

#ifndef make_HPCG_VER_MAJOR
#define HPCG_VER_MAJOR 24
#elif CHECK_EMPTY_MACRO(make_HPCG_VER_MAJOR) == 1
#define HPCG_VER_MAJOR 24
#else
#define HPCG_VER_MAJOR make_HPCG_VER_MAJOR
#endif

#ifndef make_HPCG_VER_MINOR
#define HPCG_VER_MINOR 09
#elif CHECK_EMPTY_MACRO(make_HPCG_VER_MINOR) == 1
#define HPCG_VER_MINOR 09
#else
#define HPCG_VER_MINOR make_HPCG_VER_MINOR
#endif

#ifndef make_HPCG_VER_PATCH
#define HPCG_VER_PATCH 0
#elif CHECK_EMPTY_MACRO(make_HPCG_VER_PATCH) == 1
#define HPCG_VER_PATCH 0
#else
#define HPCG_VER_PATCH make_HPCG_VER_PATCH
#endif

#ifndef make_HPCG_VER_BUILD
#define HPCG_VER_BUILD 0
#elif CHECK_EMPTY_MACRO(make_HPCG_VER_BUILD) == 1
#define HPCG_VER_BUILD 0
#else
#define HPCG_VER_BUILD make_HPCG_VER_BUILD
#endif

#define HPCG_VERSION (HPCG_VER_MAJOR * 1000 + HPCG_VER_MINOR * 100 + HPCG_VER_PATCH)

#define VER_HEADER \
    "HPCG-NVIDIA " XSTR(HPCG_VER_MAJOR)"." XSTR(HPCG_VER_MINOR) "." XSTR(HPCG_VER_PATCH) "  -- NVIDIA accelerated HPCG benchmark -- NVIDIA\n"

#define HPCG_LINE_MAX 256

extern std::ofstream HPCG_fout;

// Refer to src/init.cpp for possible user-defined values
struct HPCG_Params_STRUCT
{
    int comm_size;                   //!< Number of MPI processes in MPI_COMM_WORLD
    int comm_rank;                   //!< This process' MPI rank in the range [0 to comm_size - 1]
    int numThreads;                  //!< This process' number of threads
    local_int_t nx;                  //!< Number of processes in x-direction of 3D process grid
    local_int_t ny;                  //!< Number of processes in y-direction of 3D process grid
    local_int_t nz;                  //!< Number of processes in z-direction of 3D process grid
    int runningTime;                 //!< Number of seconds to run the timed portion of the benchmark
    int warmupTime;                  //!< Number of seconds to run the warmup portion of the benchmark
    int npx;                         //!< Number of x-direction grid points for each local subdomain
    int npy;                         //!< Number of y-direction grid points for each local subdomain
    int npz;                         //!< Number of z-direction grid points for each local subdomain
    int pz;                          //!< Partition in the z processor dimension, default is npz
    local_int_t zl;                  //!< nz for processors in the z dimension with value less than pz
    local_int_t zu;                  //!< nz for processors in the z dimension with value greater than pz
    bool benchmark_mode;             // !< Skips running reference code
    bool use_l2compression;          // !< Activates GPU L2 Compression
    bool use_hpcg_mem_reduction;     // !< Not passed as parameter. Set in main to true. Activates aggressive memory
                                     // reduction optimizations
    rank_type_t rank_type;           // !< Not passed as parameter. GPU or CPU
    p2p_comm_mode_t p2_mode;         // !< We have 4 methods to do p2p comm in MV and MG, refer to Geometry.hpp
    exec_mode_t exec_mode = GPUONLY; // !< Three modes supported: GPUONLY, CPUONLY, GPUCPU.
    int g2c;                         // !< Related to GPU/CPU local problem definition
    dim_3d_t diff_dim;               // !< Specifies the dim that is different for the CPU and GPU ranks
    local_problem_def_t local_problem_def; // !< Specifies how nx, ny, nz, and g2c are interpreted (4 possibilites)
    bool cpu_allowed_to_print; // !< Not passed as parameter. Specifies the CPU rank (opposite to GPU rank) that is
                               // allowed to print
    bool use_output_file;      // !< There is a global variable with the same name defined in src/init.cpp and used
                               // throughout the files
    local_int_t gpu_slice_size;
    local_int_t cpu_slice_size;
};
/*!
  HPCG_Params is a shorthand for HPCG_Params_STRUCT
 */
typedef HPCG_Params_STRUCT HPCG_Params;

extern void InitializeRanks(HPCG_Params& params);
extern int HPCG_Init(int* argc_p, char*** argv_p, HPCG_Params& params);
extern int HPCG_Finalize(void);

#endif // HPCG_HPP
