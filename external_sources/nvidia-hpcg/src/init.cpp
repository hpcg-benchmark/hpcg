
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

#ifndef HPCG_NO_MPI
#include <mpi.h>
#endif

#ifndef HPCG_NO_OPENMP
#include <omp.h>
#endif

#ifdef _WIN32
const char* NULLDEVICE = "nul";
#else
const char* NULLDEVICE = "/dev/null";
#endif

#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>

#include <fstream>
#include <iostream>

#include "hpcg.hpp"

#include "ReadHpcgDat.hpp"

int use_output_file = 0;
std::ofstream HPCG_fout; //!< output file stream for logging activities during HPCG run
#if defined(USE_CUDA) && defined(USE_NCCL)
ncclComm_t Nccl_Comm;
#endif

#ifndef HPCG_NO_MPI
char host_name[MPI_MAX_PROCESSOR_NAME];
char pro_name[MPI_MAX_PROCESSOR_NAME];
MPI_Comm proComm;
int global_rank = 0;
int global_total_ranks = 0;
int program_rank = 0;
int program_total_ranks = 0;
int* physical_rank_dims;
int* logical_rank_to_phys;
int* physical_rank_dims_d;
int* logical_rank_to_phys_d;
#else
char host_name[1000];
char pro_name[1000];
#endif

static int startswith(const char* s, const char* prefix)
{
    size_t n = strlen(prefix);
    if (strncmp(s, prefix, n))
        return 0;
    return 1;
}

int stringCmp(const void* a, const void* b)
{
    return strcmp((const char*) a, (const char*) b);
}

/*!
  Initializes an HPCG run by obtaining problem parameters (from a file or
  command line) and then broadcasts them to all nodes. It also initializes
  login I/O streams that are used throughout the HPCG run. Only MPI rank 0
  performs I/O operations.

  The function assumes that MPI has already been initialized for MPI runs.

  @param[in] argc_p the pointer to the "argc" parameter passed to the main() function
  @param[in] argv_p the pointer to the "argv" parameter passed to the main() function
  @param[out] params the reference to the data structures that is filled the basic parameters of the run

  @return returns 0 upon success and non-zero otherwise

  @see HPCG_Finalize
*/

void InitializeRanks(HPCG_Params& params)
{
    char(*host_names)[MPI_MAX_PROCESSOR_NAME];
    char(*program_names)[MPI_MAX_PROCESSOR_NAME];
    MPI_Comm nodeComm;
    int n, namelen, color, local_procs;
    size_t bytes;

    int deviceCount;
    int local_rank = 0;

    // 1) Find global
    MPI_Comm_rank(MPI_COMM_WORLD, &global_rank);        // GLobal rank for CPU and GPU
    MPI_Comm_size(MPI_COMM_WORLD, &global_total_ranks); // Global Number of ranks for CPU and GPU

    physical_rank_dims = new int[3 * global_total_ranks];
    logical_rank_to_phys = new int[global_total_ranks];

    bytes = global_total_ranks * sizeof(char[MPI_MAX_PROCESSOR_NAME]);

    // Color ranks by program name (if more than one binary executed, e.g., one for CPU and one for GPU)
    program_names = (char(*)[MPI_MAX_PROCESSOR_NAME]) malloc(bytes);
    strcpy(program_names[global_rank], __FILE__);
    for (n = 0; n < global_total_ranks; n++)
    {
        MPI_Bcast(&(program_names[n]), MPI_MAX_PROCESSOR_NAME, MPI_CHAR, n, MPI_COMM_WORLD);
    }
    qsort(program_names, global_total_ranks, sizeof(char[MPI_MAX_PROCESSOR_NAME]), stringCmp);

    color = 0;
    for (n = 0; n < global_total_ranks; n++)
    {
        if (n > 0 && strcmp(program_names[n - 1], program_names[n]))
            color++;
        if (strcmp(__FILE__, program_names[n]) == 0)
            break;
    }

    MPI_Comm_split(MPI_COMM_WORLD, color, 0, &proComm);
    MPI_Comm_rank(proComm, &program_rank);
    MPI_Comm_size(proComm, &program_total_ranks);
    free(program_names);

    MPI_Get_processor_name(host_name, &namelen); // Host name
    host_names = (char(*)[MPI_MAX_PROCESSOR_NAME]) malloc(bytes);
    strcpy(host_names[global_rank], host_name);

    for (n = 0; n < global_total_ranks; n++)
    {
        MPI_Bcast(&(host_names[n]), MPI_MAX_PROCESSOR_NAME, MPI_CHAR, n, MPI_COMM_WORLD);
    }

    qsort(host_names, global_total_ranks, sizeof(char[MPI_MAX_PROCESSOR_NAME]), stringCmp);

    color = 0;
    for (n = 0; n < global_total_ranks; n++)
    {
        if (n > 0 && strcmp(host_names[n - 1], host_names[n]))
            color++;
        if (strcmp(host_name, host_names[n]) == 0)
            break;
    }

    MPI_Comm_split(proComm, color, 0, &nodeComm);
    MPI_Comm_rank(nodeComm, &local_rank);
    MPI_Comm_size(nodeComm, &local_procs);

    free(host_names);
#ifdef USE_CUDA
    CHECK_CUDART(cudaGetDeviceCount(&deviceCount));
#endif

    // Figure out the rank type, based on execution mode (params.exec_mode)
    if (params.exec_mode == CPUONLY)
    {
        params.rank_type = CPU;
    }
    else if (params.exec_mode == GPUONLY)
    {
        params.rank_type = GPU;
#ifdef USE_CUDA
        CHECK_CUDART(cudaGetDeviceCount(&deviceCount));
        CHECK_CUDART(cudaSetDevice(local_rank % deviceCount));

        // Touch Pinned Memory
        double* t;
        CHECK_CUDART(cudaMallocHost((void**) (&(t)), sizeof(double)));
        CHECK_CUDART(cudaFreeHost(t));

        if (params.p2_mode == NCCL)
        {
#ifdef USE_NCCL
            ncclUniqueId id;
            if (global_rank == 0)
                CHECK_NCCL(ncclGetUniqueId(&id));
            MPI_Bcast(&id, sizeof(id), MPI_BYTE, 0, MPI_COMM_WORLD);
            CHECK_NCCL(ncclCommInitRank(&Nccl_Comm, global_total_ranks, id, global_rank));
#endif // USE_NCCL
        }

#endif // USE_CUDA
    }
    else /*CPUGPU*/
    {
        // Here we assume that a node has the same number of GPU and CPU ranks
        // This design is rigid but it is difficult to assign ranks automatically
        // to GPUs and CPUs otherwise
        params.cpu_allowed_to_print = false; // Enable printing for the first CPU rank only
        int ranks_for_numa = local_procs / deviceCount;
        if (ranks_for_numa == 1)
        {
            if (global_rank == 0)
                printf("Warning: All Ranks will be Assigned to GPUs, check the total number of ranks\n");
        }
        if (local_rank % ranks_for_numa == 0)
        {
            params.rank_type = GPU;
#ifdef USE_CUDA
            CHECK_CUDART(cudaSetDevice(local_rank / ranks_for_numa));
            // Touch Pinned Memory
            double* t;
            CHECK_CUDART(cudaMallocHost((void**) (&(t)), sizeof(double)));
            CHECK_CUDART(cudaFreeHost(t));
#endif
        }
        else
        {
            params.rank_type = CPU;
            if (local_rank == 1 && color == 0)
            {
                params.cpu_allowed_to_print = true;
            }
        }
    }

    MPI_Barrier(MPI_COMM_WORLD);
}

int HPCG_Init(int* argc_p, char*** argv_p, HPCG_Params& params)
{
    int argc = *argc_p;
    char** argv = *argv_p;
    char fname[80];
    int i, j, *iparams;
    char cparams[][9] = {"--nx=", "--ny=", "--nz=", "--rt=", "--npx=", "--npy=", "--npz=", "--b=", "--l2cmp=", "--mr=",
        "--exm=", "--g2c=", "--ddm=", "--lpm=", "--p2p=", "--of=", "--gss=", "--css=", "--wt="};
    time_t rawtime;
    tm* ptm;
    const int nparams = (sizeof cparams) / (sizeof cparams[0]);
    bool broadcastParams = false; // Make true if parameters read from file.

    const char* name = "HPCG_USE_OUTPUT_FILE";
    char* value;
    value = getenv(name);
    if (value != NULL)
    {
        use_output_file = atoi(value);
    }
    iparams = (int*) malloc(sizeof(int) * nparams);

    // Initialize iparams
    for (i = 0; i < nparams; ++i)
        iparams[i] = 0;

    /* for sequential and some MPI implementations it's OK to read first three args */
    for (i = 0; i < nparams; ++i)
        if (argc <= i + 1 || sscanf(argv[i + 1], "%d", iparams + i) != 1 || iparams[i] < 11)
            iparams[i] = 0;

    /* for some MPI environments, command line arguments may get complicated so we need a prefix */
    for (i = 1; i <= argc && argv[i]; ++i)
        for (j = 0; j < nparams; ++j)
            if (startswith(argv[i], cparams[j]))
                if (sscanf(argv[i] + strlen(cparams[j]), "%d", iparams + j) != 1)
                    iparams[j] = 0;

    // Check if --rt was specified on the command line
    int* rt = iparams + 3; // Assume runtime was not specified and will be read from the hpcg.dat file
    if (iparams[3])
        rt = 0; // If --rt was specified, we already have the runtime, so don't read it from file
    if (!iparams[0] && !iparams[1] && !iparams[2])
    { /* no geometry arguments on the command line */
        char HPCG_DAT_FILE[HPCG_LINE_MAX];
        if (argc > 1)
        {
            strcpy(HPCG_DAT_FILE, argv[1]);
        }
        else
        {
            strcpy(HPCG_DAT_FILE, "./hpcg.dat");
        }
        if (ReadHpcgDat(iparams, rt, iparams + 7, HPCG_DAT_FILE) == -1)
        {
            printf("No input data. Possible options:\n");
            fflush(0);
            printf("\t1) Specify path to input file: ./xhpcg <path to *.dat file>\n");
            printf("\t2) Copy hpcg.dat to the run directory\n");
            printf("\t3) Use command line parameters: ./xhpcg --nx <x> --ny <y> --nz <z> --rt <t>\n");
            exit(-1);
        }
        broadcastParams = true;
    }

    // Check for small or unspecified nx, ny, nz values
    // If any dimension is less than 16, make it the max over the other two dimensions, or 16, whichever is largest
    for (i = 0; i < 3; ++i)
    {
        if (iparams[i] < 16)
            for (j = 1; j <= 2; ++j)
                if (iparams[(i + j) % 3] > iparams[i])
                    iparams[i] = iparams[(i + j) % 3];
        if (iparams[i] < 16)
            iparams[i] = 16;
    }

#ifndef HPCG_NO_MPI
    MPI_Comm_rank(MPI_COMM_WORLD, &params.comm_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &params.comm_size);
#else
    params.comm_rank = 0;
    params.comm_size = 1;
#endif

// Broadcast values of iparams to all MPI processes
#ifndef HPCG_NO_MPI
    if (broadcastParams)
    {
        MPI_Bcast(iparams, nparams, MPI_INT, 0, MPI_COMM_WORLD);
    }
#endif

    params.nx = iparams[0];
    params.ny = iparams[1];
    params.nz = iparams[2];

    params.runningTime = iparams[3];
    params.warmupTime = iparams[18];

    params.npx = iparams[4];
    params.npy = iparams[5];
    params.npz = iparams[6];

    params.benchmark_mode = iparams[7] > 0;
    params.use_l2compression = iparams[8] > 0;
    params.use_hpcg_mem_reduction = iparams[9] > 0;

    /* 0: CPU only | 1: GPU only | 2: GPUCPU */
    params.exec_mode = iparams[10] == 2 ? GPUCPU : (iparams[10] == 1 ? CPUONLY : GPUONLY);
    params.g2c = iparams[11] == 0 ? 1 : iparams[11];

    /* 0: NONE | 1: X | 1: Y | 2: Z */
    params.diff_dim = iparams[12] == 3 ? Z : (iparams[12] == 2 ? Y : (iparams[12] == 1 ? X : NONE));

    // GPU_RATIO=0/*NX, NY, NZ are local to GPU and g2c is a ratio*/
    // GPU_ABS=1/*NX, NY, NZ are local to GPU and g2c is absolute dimension size*/,
    // GPU_CPU_RATIO=2/*NX, NY, NZ are local to GPU+CPU and g2c is ratio*/,
    // GPU_CPU_ABS=3/*NX, NY, NZ are local to GPU+CPU and g2c is absolute dimension size*/
    if (iparams[13] == 1)
        params.local_problem_def = GPU_ABS;
    else if (iparams[13] == 2)
        params.local_problem_def = GPU_CPU_RATIO;
    else if (iparams[13] == 3)
        params.local_problem_def = GPU_CPU_ABS;
    else
        params.local_problem_def = GPU_RATIO;

    // P2P Communication method
    if (iparams[14] == 1)
        params.p2_mode = MPI_CPU_All2allv;
    else if (iparams[14] == 2)
        params.p2_mode = MPI_CUDA_AWARE;
    else if (iparams[14] == 3)
        params.p2_mode = MPI_GPU_All2allv;
    else if (iparams[14] == 4)
        params.p2_mode = NCCL;
    else
        params.p2_mode = MPI_CPU;

    if (iparams[15] == 1)
    {
        params.use_output_file = 1;
        use_output_file = 1;
    }
    else
    {
        params.use_output_file = 0;
        use_output_file = 0;
    }
    
    // --gss
    params.gpu_slice_size = iparams[16] > 0 ? iparams[16] : 4096;

    // --css
    params.cpu_slice_size = iparams[17] > 0 ? iparams[17] : 8;

    if (params.comm_rank == 0)
    {
        printf("%s", VER_HEADER);
    }

#ifdef HPCG_NO_OPENMP
    params.numThreads = 1;
#else
#pragma omp parallel
    #pragma omp single
    params.numThreads = omp_get_num_threads();
#endif

    time(&rawtime);
    ptm = localtime(&rawtime);
    sprintf(fname, "hpcg%04d%02d%02dT%02d%02d%02d.txt", 1900 + ptm->tm_year, ptm->tm_mon + 1, ptm->tm_mday,
        ptm->tm_hour, ptm->tm_min, ptm->tm_sec);

    if (use_output_file)
    {
        if (0 == params.comm_rank)
        {
            HPCG_fout.open(fname);
        }
        else
        {
#if defined(HPCG_DEBUG) || defined(HPCG_DETAILED_DEBUG)
            sprintf(fname, "hpcg%04d%02d%02dT%02d%02d%02d_%d.txt", 1900 + ptm->tm_year, ptm->tm_mon + 1, ptm->tm_mday,
                ptm->tm_hour, ptm->tm_min, ptm->tm_sec, params.comm_rank);
            HPCG_fout.open(fname);
#else
            HPCG_fout.open(NULLDEVICE);
#endif
        }
    }
    free(iparams);

    return 0;
}