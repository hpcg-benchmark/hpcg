
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
 @file main.cpp

 - All emums are in Geomerty.hpp
 - Supports GPU-only, Grace-only, and GPU-Grace. GPU and Grace are different MPI ranks.
 - The dimensions of GPU rank and CPU rank can only differ in one dimension (nx, ny, or nz).
 - Parameters are explained in bin/RUNNING-*
 */

// Main routine of a program that calls the HPCG conjugate gradient
// solver to solve the problem, and then prints results.

#ifndef HPCG_NO_MPI
#include <mpi.h>
#endif

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <vector>

#ifdef USE_GRACE
#include <nvpl_sparse.h>
#endif

#include "CG.hpp"
#include "CGData.hpp"
#include "CG_ref.hpp"
#include "CheckAspectRatio.hpp"
#include "CheckProblem.hpp"
#include "ComputeMG_ref.hpp"
#include "ComputeResidual.hpp"
#include "ComputeSPMV_ref.hpp"
#include "CpuKernels.hpp"
#include "CudaKernels.hpp"
#include "ExchangeHalo.hpp"
#include "GenerateCoarseProblem.hpp"
#include "GenerateGeometry.hpp"
#include "GenerateProblem.hpp"
#include "Geometry.hpp"
#include "OptimizeProblem.hpp"
#include "ReportResults.hpp"
#include "SetupHalo.hpp"
#include "SparseMatrix.hpp"
#include "TestCG.hpp"
#include "TestNorms.hpp"
#include "TestSymmetry.hpp"
#include "Vector.hpp"
#include "WriteProblem.hpp"
#include "hpcg.hpp"
#include "mytimer.hpp"

#ifdef HPCG_DETAILED_DEBUG
using std::cin;
#endif
using std::endl;

// Prints in a file or terminal
extern int use_output_file;

#ifdef USE_CUDA
cusparseHandle_t cusparsehandle;
cublasHandle_t cublashandle;
cudaStream_t stream;
cudaEvent_t copy_done;
cudaStream_t copy_stream;
int* ranktoId;
#ifdef USE_NCCL
double* d_dot_nccl_allreduce_local;
double* d_dot_nccl_allreduce_global;
#endif
#endif

#ifdef USE_GRACE
nvpl_sparse_handle_t nvpl_sparse_handle;
#endif

// The communication mode used to send point-to-point messages
#ifndef HPCG_NO_MPI
p2p_comm_mode_t P2P_Mode;
#endif

// USE CUDA L2 compression
bool Use_Compression;

// USE HPCG aggresive memory reduction
bool Use_Hpcg_Mem_Reduction;

#ifndef HPCG_NO_MPI
// Used to find ranks for CPU and GPU programs
int* rankToId_h;
int* idToRank_h;
extern int* physical_rank_dims;
extern int* logical_rank_to_phys;
#endif

/*!
  Main driver program: Construct synthetic problem, run V&V tests, compute benchmark parameters, run benchmark, report
  results.

  @param[in]  argc Standard argument count.  Should equal 1 (no arguments passed in) or 4 (nx, ny, nz passed in)
  @param[in]  argv Standard argument array.  If argc==1, argv is unused.  If argc==4, argv[1], argv[2], argv[3] will be
  interpreted as nx, ny, nz, resp.

  @return Returns zero on success and a non-zero value otherwise.

*/
int main(int argc, char* argv[])
{
#ifndef HPCG_NO_MPI
    MPI_Init(&argc, &argv);
#endif

    // Here I read all the parameters, including the execution mode (CPUONLY, GPUONLY, GPUCPU)
    HPCG_Params params;
    HPCG_Init(&argc, &argv, params);
    bool quickPath = (params.runningTime == 0);
    int size = params.comm_size, rank = params.comm_rank; // Number of MPI processes, My process ID

    bool benchmark_mode = params.benchmark_mode;
    Use_Compression = params.use_l2compression;
    Use_Hpcg_Mem_Reduction = true; // params.use_hpcg_mem_reduction;
    P2P_Mode = params.p2_mode;

    if (rank == 0)
    {
        printf("Build v0.6.1 \n");

#ifdef HPCG_ENG_VERSION
        printf("\n%s%s\n", "========================================", "========================================");
#ifdef HPCG_COMMIT_HASH
        printf("Engineering version of HPCG-NVIDIA. Results cannot be shared with third parties\nCommit: %s\n",
            XSTR(HPCG_COMMIT_HASH));
#else
        printf("Engineering version of HPCG-NVIDIA. Results cannot be shared with third parties\nCommit:\n");
#endif
        printf("%s%s\n", "========================================", "========================================");
#endif
        printf("\nStart of application (%s) ...\n",
            params.exec_mode == GPUONLY       ? "GPU-Only"
                : params.exec_mode == CPUONLY ? "Grace-Only"
                                              : "GPU+Grace");

        if (benchmark_mode)
            printf(" | Benchmark Mode !!!! CPU reference code is not performed \n");

        if (params.exec_mode == GPUONLY || params.exec_mode == GPUCPU)
            if (Use_Compression)
                printf(
                    " | L2 compression is activated !!!! Currently, it is not legal to submit HPCG results with L2 "
                    "compression\n");
        #ifdef INDEX_64
            printf(" | Using INT64 Indexing \n");
        #endif
    }

    // Check P2P comm mode
    if (params.exec_mode == CPUONLY || params.exec_mode == GPUCPU)
    {
#ifndef USE_GRACE
        if (rank == 0)
            printf(
                "Error: HPCG was not compiled for Grace execution. USE --exm=0 for GPU-only execution or add "
                "-DUSE_GRACE. Exiting ...\n");
#ifndef HPCG_NO_MPI
        MPI_Finalize();
#endif
        return 0;
#endif // USE_GRACE

        bool invalid = false;
        // P2P=NCCL is only valid with --exm=0 (GPUONLY). CPU-only and GPU+CPU runs exit here; Nccl_Comm is never built for them.
        if (P2P_Mode == NCCL)
        {
            if (rank == 0)
            {
                if (params.exec_mode == GPUCPU)
                    printf(
                        "Error: P2P mode NCCL is not supported in GPU+CPU hybrid execution (--exm=2): CPU ranks cannot "
                        "participate in NCCL. Use another --p2p value (e.g. 0 MPI_CPU). Exiting ...\n");
                else
                    printf(
                        "Invalid P2P communication mode (NCCL) for CPU-only execution (no GPU NCCL endpoints). "
                        "Exiting ...\n");
            }
            invalid = true;
        }
        if (P2P_Mode == MPI_GPU_All2allv)
        {
            if (rank == 0)
                printf("Invalid P2P communication mode (MPI GPU All2allv) for CPUs, Exiting ...\n");
            invalid = true;
        }
        if (P2P_Mode == MPI_CUDA_AWARE)
        {
            if (rank == 0)
                printf("Invalid P2P communication mode (CUDA-Aware MPI) for CPUs, Exiting ...\n");
            invalid = true;
        }
        if (invalid)
        {
#ifndef HPCG_NO_MPI
            MPI_Finalize();
#endif
            return 0;
        }
    }

#ifndef USE_NCCL
    if (params.exec_mode == GPUONLY)
    {
        if (rank == 0)
            printf(
                "Error: HPCG was not compiled with NCCL. USE --exm=1 for Grace-only execution or add -DUSE_NCCL. "
                "Exiting ...\n");
#ifndef HPCG_NO_MPI
        MPI_Finalize();
#endif
        return 0;
    }
#endif // USE_NCCL

    // Check whether total number of ranks == npx*npy*npz
    auto rank_grid_size = params.npx * params.npy * params.npz;
    if (rank_grid_size > 0 && size != rank_grid_size)
    {
        if (rank == 0)
            printf("Error: Total Number of ranks != npx*npy*npz. Exiting ...\n");
#ifndef HPCG_NO_MPI
        MPI_Finalize();
#endif
        return 0;
    }

#ifndef USE_CUDA
    if (params.exec_mode != CPUONLY)
    {
        if (rank == 0)
            printf(
                "Error: HPCG was not compiled for GPU execution. USE --exm=1 for Grace-only execution or add "
                "-DUSE_CUDA. Exiting ...\n");
#ifndef HPCG_NO_MPI
        MPI_Finalize();
#endif
        return 0;
    }
#endif

    // Here, we decide the rank type
    // assign a rank to GPU and CPU
    InitializeRanks(params);

// Check if QuickPath option is enabled.
// If the running time is set to zero, we minimize all paths through the program
#ifdef HPCG_DETAILED_DEBUG
    if (size < 100 && rank == 0)
        HPCG_fout << "Process " << rank << " of " << size << " is alive with " << params.numThreads << " threads."
                  << endl;
    if (rank == 0)
    {
        char c;
        std::cout << "Press key to continue" << std::endl;
        std::cin.get(c);
    }
#ifndef HPCG_NO_MPI
    MPI_Barrier(MPI_COMM_WORLD);
#endif
#endif

/////////////////////////
// Problem setup Phase //
/////////////////////////
#ifdef HPCG_DEBUG
    double t1 = mytimer();
#endif

    // Construct the geometry and linear system
    Geometry* geom = new Geometry;
    GenerateGeometry(params, geom);
    int ierr = CheckAspectRatio(0.125, geom->nx, geom->ny, geom->nz, "local problem", rank == 0);
    if (ierr)
        return ierr;

    ierr = CheckAspectRatio(0.125, geom->npx, geom->npy, geom->npz, "process grid", rank == 0);
    if (ierr)
        return ierr;

// Sync All Ranks
#ifndef HPCG_NO_MPI
    MPI_Barrier(MPI_COMM_WORLD);
#endif

    // Test Library versions for cuSPARSE or NVPL Sparse
    // The two librray versions has to be tested in
    // GPU or Grace ranks
    int cusparseMajor = 0, cusparseMinor = 0;
    if (params.exec_mode == GPUONLY || params.exec_mode == GPUCPU)
    {
#ifdef USE_CUDA
        // Cusparse Version
        CHECK_CUSPARSE(cusparseGetProperty(MAJOR_VERSION, &cusparseMajor));
        CHECK_CUSPARSE(cusparseGetProperty(MINOR_VERSION, &cusparseMinor));

        if (cusparseMajor < 12 || (cusparseMajor == 12 && cusparseMinor < 2))
        {
            if (rank == 0)
                printf("cuSPARSE version must be 12.2 or higher (found v%d.%d) \n", cusparseMajor, cusparseMinor);
#ifndef HPCG_NO_MPI
            MPI_Finalize();
#endif
            return 0;
        }
#endif
    }

    int nvspMajor = 0, nvspMinor = 0, nvspPatch = 0, nvspVersion = 0;
    if (params.exec_mode == CPUONLY || params.exec_mode == GPUCPU)
    {
#ifdef USE_GRACE
        // NVPL Sparse Version
        nvpl_sparse_create(&(nvpl_sparse_handle));
        nvpl_sparse_get_version(nvpl_sparse_handle, &nvspVersion);
        nvspMajor = nvspVersion / 1000;
        nvspMinor = (nvspVersion % 1000) / 100;
        nvspPatch = nvspVersion % 100;
        if (nvspMajor < 0 || (nvspMajor == 0 && nvspMinor < 2))
        {
            if (rank == 0)
                printf("NVPL Sparse version must be 0.2 or higher (found v%d.%d) \n", nvspMajor, nvspMinor);
#ifndef HPCG_NO_MPI
            MPI_Finalize();
#endif
            return 0;
        }
#endif // USE_GRACE
    }

    SparseMatrix A;
    Vector x_overlap, b_computed;
    Vector b, x, xexact;
    std::vector<double> times(10, 0.0);
    CGData data;
    InitializeSparseMatrix(A, geom);
    size_t cpuRefMemory = 0;
    int numberOfMgLevels = 4; // Number of levels including first
    SparseMatrix* curLevelMatrix = &A;
    if (params.rank_type == GPU)
    {
#ifdef USE_CUDA
        A.rankType = GPU;
        A.slice_size = params.gpu_slice_size;
        cublasCreate(&(cublashandle));
        CHECK_CUSPARSE(cusparseCreate(&(cusparsehandle)));
        CHECK_CUDART(cudaStreamCreate(&(stream)));
        CHECK_CUDART(cudaStreamCreate(&(copy_stream)));
        CHECK_CUSPARSE(cusparseSetStream(cusparsehandle, stream));
        cublasSetStream(cublashandle, stream);
        CHECK_CUSPARSE(cusparseSetPointerMode(cusparsehandle, CUSPARSE_POINTER_MODE_HOST));
        CHECK_CUDART(cudaEventCreate(&copy_done));
#ifdef USE_NCCL
        if (params.p2_mode == NCCL)
            cublasSetPointerMode(cublashandle, CUBLAS_POINTER_MODE_DEVICE);
        else
#endif
            cublasSetPointerMode(cublashandle, CUBLAS_POINTER_MODE_HOST);
#ifdef USE_NCCL
        CHECK_CUDART(cudaMalloc(&d_dot_nccl_allreduce_local, sizeof(double)));
        CHECK_CUDART(cudaMalloc(&d_dot_nccl_allreduce_global, sizeof(double)));
#endif

        // Allocate GPU related data
        AllocateMemCuda(A);

        double setup_time = mytimer();
        GenerateProblem(A, &b, &x, &xexact);
        SetupHalo(A);
        for (int level = 1; level < numberOfMgLevels; ++level)
        {
            GenerateCoarseProblem(*curLevelMatrix);
            curLevelMatrix = curLevelMatrix->Ac; // Make the just-constructed coarse grid the next level
        }
        setup_time = mytimer() - setup_time; // Capture total time of setup
        delete[] physical_rank_dims;
        delete[] logical_rank_to_phys;
        times[9] = setup_time; // Save it for reporting

        // Copy data from Device to Host.
        // Note: exclude this from setup_time, as soon as it is needed only for reference calls.
        cpuRefMemory = CopyDataToHostCuda(A, &b, &x, &xexact);

        // Alocate the GPU data for optimized data structures
        AllocateMemOptCuda(A);
#endif
    }
    else
    {
#ifdef USE_GRACE
        A.rankType = CPU;
        A.slice_size = params.cpu_slice_size;
        // Use this array for collecting timing information
        double setup_time = mytimer();
        GenerateProblem(A, &b, &x, &xexact);
        SetupHalo(A);
        for (int level = 1; level < numberOfMgLevels; ++level)
        {
            GenerateCoarseProblem(*curLevelMatrix);
            curLevelMatrix = curLevelMatrix->Ac; // Make the just-constructed coarse grid the next level
        }

        // These global buffers only needed for problem setup
        delete[] rankToId_h;
        delete[] idToRank_h;
        delete[] physical_rank_dims;
        delete[] logical_rank_to_phys;

        setup_time = mytimer() - setup_time; // Capture total time of setup
        times[9] = setup_time;               // Save it for reporting
#endif                                       // USE_GRACE
    }

    curLevelMatrix = &A;
    Vector* curb = &b;
    Vector* curx = &x;
    Vector* curxexact = &xexact;
    for (int level = 0; level < numberOfMgLevels; ++level)
    {
        // Doesn't work for GPU or GPUCPU cases
        // Data need to be transfered between CPU and GPU, which is not feasible
        if (params.exec_mode == CPUONLY)
        {
            CheckProblem(*curLevelMatrix, curb, curx, curxexact);
            //Delete mtxIndG since it is not needed anymore
            delete [] curLevelMatrix->mtxIndG[0];
        }
        curLevelMatrix = curLevelMatrix->Ac; // Make the nextcoarse grid the next level
        curb = 0;                            // No vectors after the top level
        curx = 0;
        curxexact = 0;
    }

    InitializeSparseCGData(A, data);

    ////////////////////////////////////
    // Reference SpMV+MG Timing Phase //
    ////////////////////////////////////
    // Call Reference SpMV and MG. Compute Optimization time as ratio of times in these routines
    local_int_t nrow = A.localNumberOfRows;
    local_int_t ncol = A.localNumberOfColumns;
    InitializeVector(x_overlap, ncol, A.rankType);  // Overlapped copy of x vector
    InitializeVector(b_computed, nrow, A.rankType); // Computed RHS vector

    // Record execution time of reference SpMV and MG kernels for reporting times
    // First load vector with random values
    FillRandomVector(x_overlap);

    int numberOfCalls = 10;
    if (quickPath)
        numberOfCalls = 1; // QuickPath means we do on one call of each block of repetitive code
    if (!benchmark_mode)
    {
        double t_begin = mytimer();
        for (int i = 0; i < numberOfCalls; ++i)
        {
            ierr = ComputeSPMV_ref(A, x_overlap, b_computed); // b_computed = A*x_overlap
            if (ierr)
                if (use_output_file)
                {
                    HPCG_fout << "Error in call to SpMV: " << ierr << ".\n" << endl;
                }
                else
                {
                    std::cout << "Error in call to SpMV: " << ierr << ".\n" << endl;
                }
            ierr = ComputeMG_ref(A, b_computed, x_overlap); // b_computed = Minv*y_overlap
            if (ierr)
                if (use_output_file)
                {
                    HPCG_fout << "Error in call to MG: " << ierr << ".\n" << endl;
                }
                else
                {
                    std::cout << "Error in call to MG: " << ierr << ".\n" << endl;
                }
        }
        times[8] = (mytimer() - t_begin) / ((double) numberOfCalls); // Total time divided by number of calls.
#ifdef HPCG_DEBUG
        if (rank == 0)
            HPCG_fout << "Total SpMV+MG timing phase execution time in main (sec) = " << mytimer() - t1 << endl;
#endif
    }

    ///////////////////////////////
    // Reference CG Timing Phase //
    ///////////////////////////////

#ifdef HPCG_DEBUG
    t1 = mytimer();
#endif
    int global_failure = 0; // assume all is well: no failures

    int niters = 0;
    int totalNiters_ref = 0;
    double normr = 1.0;
    double normr0 = 1.0;
    int refMaxIters = 50;
    numberOfCalls = 1; // Only need to run the residual reduction analysis once

    // Compute the residual reduction for the natural ordering and reference kernels
    std::vector<double> ref_times(9, 0.0);
    double tolerance = 0.0; // Set tolerance to zero to make all runs do maxIters iterations
    int err_count = 0;
    double refTolerance = 0.0055;
    if (!benchmark_mode)
    {
        for (int i = 0; i < numberOfCalls; ++i)
        {
            ZeroVector(x);
            ierr = CG_ref(A, data, b, x, refMaxIters, tolerance, niters, normr, normr0, &ref_times[0], true,
                i == 0); // TODO: TRUE
            if (ierr)
                ++err_count; // count the number of errors in CG
            totalNiters_ref += niters;
        }
        if (rank == 0 && err_count)
            if (use_output_file)
            {
                HPCG_fout << err_count << " error(s) in call(s) to reference CG." << endl;
            }
            else
            {
                std::cout << err_count << " error(s) in call(s) to reference CG." << endl;
            }
        refTolerance = normr / normr0;
    }

    if (params.exec_mode == GPUONLY || params.exec_mode == GPUCPU)
    {
#ifdef USE_CUDA
        if (cusparseMajor < 12 || (cusparseMajor == 12 && cusparseMinor < 5))
        {
            // Test for the most course matrix
            if(A.localNumberOfRows/(8 * 8 * 8) < A.slice_size) {
                if (rank == 0)
                    printf("cuSPARSE version must be 12.5 or higher (found v%d.%d) to allow a GPU slice size (%lld) larger than the matrix number of rows (%lld). Use --gss to set GPU slice size \n", 
                    cusparseMajor, cusparseMinor, (long long)A.slice_size, (long long)(A.localNumberOfRows/(8*8*8)));
#ifndef HPCG_NO_MPI
                MPI_Finalize();
#endif
                return 0;
            }
        }
#endif
    }

    // Call user-tunable set up function.
    double t7 = mytimer();
    size_t opt_mem = OptimizeProblem(A, data, b, x, xexact);
    t7 = mytimer() - t7;
    times[7] = t7;
#ifdef HPCG_DEBUG
    if (rank == 0)
        std::cout << "Total problem optimize in main (sec) = " << t7 << endl;
#endif

    if (params.rank_type == GPU)
    {
#ifdef USE_CUDA
        int dev;
        cudaDeviceProp props;
        CHECK_CUDART(cudaGetDevice(&dev));
        CHECK_CUDART(cudaGetDeviceProperties(&props, dev));
        size_t free_bytes, total_bytes;
        CHECK_CUDART(cudaMemGetInfo(&free_bytes, &total_bytes));

        //Find the number of SMS
        int numSMS = props.multiProcessorCount;

        if (rank == 0)
            printf(
                "GPU Rank Info:\n"
                " | cuSPARSE version %d.%d\n%s"
                " | Reference CPU memory = %.2f MB\n"
                " | GPU Name: '%s'\n"
                " | Number of SMs: %d\n"
                " | GPU Memory Use: %ld MB / %ld MB\n"
                " | Process Grid: %dx%dx%d\n"
                " | Local Domain: %dx%dx%d\n"
                " | Number of CPU Threads: %d\n"
                " | Slice Size: %lld\n",
                cusparseMajor, cusparseMinor, Use_Compression ? " | L2 compression is activated\n" : "",
                cpuRefMemory / 1024.0 / 1024.0, props.name, numSMS, (total_bytes - free_bytes) >> 20, total_bytes >> 20,
                A.geom->npx, A.geom->npy, A.geom->npz, (int)A.geom->nx, (int)A.geom->ny, (int)A.geom->nz, params.numThreads, (long long)A.slice_size);
        CHECK_CUDART(cudaDeviceSynchronize());
#endif
    }
    else
    {
#ifdef USE_GRACE
        cpuRefMemory = EstimateCpuRefMem(A);
        if (rank == 0 || (params.exec_mode == GPUCPU && params.cpu_allowed_to_print))
            printf(
                "CPU Rank Info:\n"
                " | NVPL Sparse version %d.%d.%d\n"
                " | Reference CPU memory = %.2f MB\n"
                " | Optimization Memory Use: %.2f MB\n"
                " | Process Grid: %dx%dx%d\n"
                " | Local Domain: %dx%dx%d\n"
                " | Number of CPU Threads: %d\n"
                " | Slice Size: %d\n",
                nvspMajor, nvspMinor, nvspPatch, cpuRefMemory / 1024.0 / 1024.0, opt_mem / 1024.0 / 1024.0, A.geom->npx,
                A.geom->npy, A.geom->npz, A.geom->nx, A.geom->ny, A.geom->nz, params.numThreads, A.slice_size);
#endif // USE_GRACE
    }

#ifdef HPCG_DETAILED_DEBUG
    if (geom->size == 1)
        WriteProblem(*geom, A, b, x, xexact);
#endif

    MPI_Barrier(MPI_COMM_WORLD);

//////////////////////////////
// Validation Testing Phase //
//////////////////////////////
#ifdef HPCG_DEBUG
    t1 = mytimer();
#endif
    TestCGData testcg_data;
    testcg_data.count_pass = testcg_data.count_fail = 0;
    TestCG(A, data, b, x, testcg_data);

    TestSymmetryData testsymmetry_data;
    TestSymmetry(A, b, xexact, testsymmetry_data);

#ifdef HPCG_DEBUG
    if (rank == 0)
        HPCG_fout << "Total validation (TestCG and TestSymmetry) execution time in main (sec) = " << mytimer() - t1
                  << endl;
#endif

    //////////////////////////////
    // Optimized CG Setup Phase //
    //////////////////////////////

    // Need to permute the b vector
    if (A.rankType == GPU)
    {
#ifdef USE_CUDA
        PermVectorCuda(A.opt2ref, b, A.localNumberOfRows);
#endif
    }
    else
    {
#ifdef USE_GRACE
        PermVectorCpu(A.opt2ref, b, A.localNumberOfRows);
#endif
    }

    niters = 0;
    normr = 0.0;
    normr0 = 0.0;
    err_count = 0;
    int tolerance_failures = 0;

    int optMaxIters = 10 * refMaxIters;
    int optNiters = refMaxIters;
    double opt_worst_time = 0.0;
    double opt_best_time = 9999999.0;

    std::vector<double> bleh_times(9, 0.0);
    ZeroVector(x); // start x at all zeros
    ierr = CG(A, data, b, x, optMaxIters, refTolerance, niters, normr, normr0, &bleh_times[0], true, 1);
    int warmupNiters = niters;

    /////////////////////
    // Warmup CG Phase //
    /////////////////////

    if (params.warmupTime > 0)
    {
        double warmup_runtime = params.warmupTime;
        double rough_time_per_set = bleh_times[0];
#ifndef HPCG_NO_MPI
        double local_rough_time = rough_time_per_set;
        MPI_Allreduce(&local_rough_time, &rough_time_per_set, 1, MPI_DOUBLE, MPI_MAX, MPI_COMM_WORLD);
#endif
        int numberOfWarmupCgSets = (rough_time_per_set > 0.0) ? int(warmup_runtime / rough_time_per_set) + 1 : 1;
        if (rank == 0)
            printf("\n========== Warmup Phase ==========\n"
                   " | Target time:  %.0fs\n",
                   warmup_runtime);

        std::vector<double> warmup_times(10, 0.0);
        double warmup_start = mytimer();
        for (int i = 0; i < numberOfWarmupCgSets; ++i)
        {
            ZeroVector(x);
            ierr = CG(A, data, b, x, warmupNiters, 0.0, niters, normr, normr0, &warmup_times[0], true, 0);
            if (ierr)
                if (use_output_file)
                {
                    HPCG_fout << "Error in call to CG (warmup): " << ierr << ".\n" << endl;
                }
                else
                {
                    std::cout << "Error in call to CG (warmup): " << ierr << ".\n" << endl;
                }
        }
        double warmup_elapsed = mytimer() - warmup_start;

        if (rank == 0)
            printf(" | Elapsed time: %.2fs\n"
                   "========== Warmup Complete =======\n\n", warmup_elapsed);
    }

    //////////////////////////////////////
    // Optimized CG Calibration Phase   //
    //////////////////////////////////////

    std::vector<double> opt_times(9, 0.0);
    numberOfCalls = 1;

    // Compute the residual reduction and residual count for the user ordering and optimized kernels.
    for (int i = 0; i < numberOfCalls; ++i)
    {
        ZeroVector(x); // start x at all zeros
        double last_cummulative_time = opt_times[0];
        ierr = CG(A, data, b, x, optMaxIters, refTolerance, niters, normr, normr0, &opt_times[0], true, 0); // TODO:
                                                                                                            // TRUE
        if (ierr)
            ++err_count; // count the number of errors in CG
        if (normr / normr0 > refTolerance)
            ++tolerance_failures; // the number of failures to reduce residual

        // pick the largest number of iterations to guarantee convergence
        if (niters > optNiters)
            optNiters = niters;

        double current_time = opt_times[0] - last_cummulative_time;
        if (current_time > opt_worst_time)
            opt_worst_time = current_time;
        if (current_time < opt_best_time)
            opt_best_time = current_time;
    }

#ifndef HPCG_NO_MPI
    // Get the absolute worst time across all MPI ranks (time in CG can be different)
    double local_opt_worst_time = opt_worst_time;
    MPI_Allreduce(&local_opt_worst_time, &opt_worst_time, 1, MPI_DOUBLE, MPI_MAX, MPI_COMM_WORLD);
#endif

    if (rank == 0 && err_count)
        if (use_output_file)
        {
            HPCG_fout << err_count << " error(s) in call(s) to optimized CG." << endl;
        }
        else
        {
            std::cout << err_count << " error(s) in call(s) to optimized CG." << endl;
        }
    if (tolerance_failures)
    {
        global_failure = 1;
        if (rank == 0)
            if (use_output_file)
            {
                HPCG_fout << "Failed to reduce the residual " << tolerance_failures << " times." << endl;
            }
            else
            {
                std::cout << "Failed to reduce the residual " << tolerance_failures << " times." << endl;
            }
    }

    ///////////////////////////////
    // Optimized CG Timing Phase //
    ///////////////////////////////

    // Here we finally run the benchmark phase
    // The variable total_runtime is the target benchmark execution time in seconds

    double total_runtime = params.runningTime;
    int numberOfCgSets = int(total_runtime / opt_worst_time) + 1; // Run at least once, account for rounding

#ifdef HPCG_DEBUG
    if (rank == 0)
    {
        HPCG_fout << "Projected running time: " << total_runtime << " seconds" << endl;
        HPCG_fout << "Number of CG sets: " << numberOfCgSets << endl;
    }
#endif

    /* This is the timed run for a specified amount of time. */

    optMaxIters = optNiters;
    double optTolerance = 0.0; // Force optMaxIters iterations
    TestNormsData testnorms_data;
    testnorms_data.samples = numberOfCgSets;
    testnorms_data.values = new double[numberOfCgSets];

#ifndef HPCG_NOMPI
    MPI_Barrier(MPI_COMM_WORLD);
#endif

    for (int i = 0; i < numberOfCgSets; ++i)
    {
        ZeroVector(x);                                                                                  // Zero out x
        ierr = CG(A, data, b, x, optMaxIters, optTolerance, niters, normr, normr0, &times[0], true, 0); // TODO: TRUE
        if (ierr)
            if (use_output_file)
            {
                HPCG_fout << "Error in call to CG: " << ierr << ".\n" << endl;
            }
            else
            {
                std::cout << "Error in call to CG: " << ierr << ".\n" << endl;
            }
        if (rank == 0)
            if (use_output_file)
            {
                HPCG_fout << "Call [" << i << "] Scaled Residual [" << normr / normr0 << "]" << endl;
            }
            else
            {
                std::cout << "Call [" << i << "] Scaled Residual [" << normr / normr0 << "]" << endl;
            }
        testnorms_data.values[i] = normr / normr0; // Record scaled residual from this run
    }

    if (params.rank_type == GPU)
    {
#ifdef USE_CUDA
        PermVectorCuda(A.ref2opt, x, A.localNumberOfRows);
        CopyVectorD2H(x);
#endif
    }
    else
    {
#ifdef USE_GRACE
        // Reorder vector
        Vector xOrdered;
        InitializeVector(xOrdered, x.localLength, A.rankType);
        CopyVector(x, xOrdered);
        CopyAndReorderVector(xOrdered, x, A.ref2opt);
        DeleteVector(xOrdered);
#endif
    }

// Compute difference between known exact solution and computed solution
// All processors are needed here.
#ifdef HPCG_DEBUG
    double residual = 0;
    ierr = ComputeResidual(A.localNumberOfRows, x, xexact, residual);
    if (ierr)
        HPCG_fout << "Error in call to compute_residual: " << ierr << ".\n" << endl;
    if (rank == 0)
        HPCG_fout << "Difference between computed and exact  = " << residual << ".\n" << endl;
#endif

    // Test Norm Results
    ierr = TestNorms(testnorms_data);

    //////////////////
    // Report Results //
    //////////////////

    // Report results to YAML file
    ReportResults(A, numberOfMgLevels, numberOfCgSets, refMaxIters, optMaxIters, &times[0], testcg_data,
        testsymmetry_data, testnorms_data, global_failure, quickPath);

    if (params.rank_type == GPU)
    {
#ifdef USE_CUDA
       DeleteMatrixGpu(A); // This delete will recursively delete all coarse grid data
#endif
    }
    else
    {
#ifdef USE_GRACE
     DeleteMatrixCpu(A); // This delete will recursively delete all coarse grid data
#endif
    }
 
    DeleteCGData(data);
    DeleteVector(x);
    DeleteVector(b);
    DeleteVector(xexact);
    DeleteVector(x_overlap);
    DeleteVector(b_computed);
    delete[] testnorms_data.values;

    // Clean cuSPARSE data
    if (params.rank_type == GPU)
    {
#ifdef USE_CUDA
        cublasDestroy(cublashandle);
        CHECK_CUSPARSE(cusparseDestroy(cusparsehandle));
        CHECK_CUDART(cudaStreamDestroy(stream));
        CHECK_CUDART(cudaStreamDestroy(copy_stream));
        CHECK_CUDART(cudaEventDestroy(copy_done));
#ifdef USE_NCCL
        CHECK_CUDART(cudaFree(d_dot_nccl_allreduce_local));
        CHECK_CUDART(cudaFree(d_dot_nccl_allreduce_global));
#endif
#endif
    }

    // We create the handle even in GPU ranks tp find library version
    if (params.exec_mode == CPUONLY || params.exec_mode == GPUCPU)
    {
#ifdef USE_GRACE
        nvpl_sparse_destroy(nvpl_sparse_handle);
#endif
    }

    HPCG_Finalize();

// Finish up
#ifndef HPCG_NO_MPI
    MPI_Finalize();
#endif
    return 0;
}