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

#ifdef USE_GRACE
#include "CpuKernels.hpp"

extern bool Use_Hpcg_Mem_Reduction; /*USE HPCG aggresive memory reduction*/

//////////////////////// Allocate CPU/Grace Memory data structures /////////////
size_t AllocateMemCpu(SparseMatrix& A_in)
{
    SparseMatrix* A = &A_in;
    local_int_t numberOfMgLevels = 4;
    local_int_t slice_size = A->slice_size;
    size_t opt_mem = 0;
    for (int level = 0; level < numberOfMgLevels; ++level)
    {
        A->level = level;
        A->slice_size = slice_size;
        size_t nrow = A->localNumberOfRows;
        size_t ncol = A->localNumberOfColumns;

        A->diagonal = new double[nrow];

        A->ref2opt = new local_int_t[ncol];
        A->opt2ref = new local_int_t[ncol];
        A->f2cPerm = new local_int_t[ncol];
        A->totalColors = 2 * 2 * 2; // two colors per axis
        A->cpuAux.color = new local_int_t[nrow];
        A->cpuAux.firstRowOfColor = new local_int_t[nrow];
        A->cpuAux.nRowsWithColor = new local_int_t[A->totalColors];
        A->tempBuffer = new double[nrow];
        // CSR external matrix
        A->csrExtOffsets = new local_int_t[nrow + 1];
        A->csrExtColumns = new local_int_t[A->extNnz];
        A->csrExtValues = new double[A->extNnz];

        opt_mem += 2 * nrow * sizeof(double);
        opt_mem += 3 * ncol * sizeof(local_int_t);
        opt_mem += 4 * nrow * sizeof(local_int_t);

        size_t num_slices = (nrow + slice_size - 1) / slice_size;
        size_t padded_nrow = num_slices * slice_size;

        /*
            Size 512x512x288 is the largest int32 local problem size with
                lowest convergence rate. Hard-coded size is used to avoid
                allocate memory in the middle of the Optimization phase.
            Size 512x512x296 is the largest possible int32 local problem
             size.
            Hard-coding is used only when Use_Hpcg_Mem_Reduction is set
             to true in src/main. Other wise we use an estimation div-
             isor for L and U matrices.
        */
        bool power_two = (nrow & (nrow - 1)) == 0;
        float divisor = nrow < 8192 ? 1.0 : (power_two? 1.85 : 1.60);
        local_int_t estimated_size = (padded_nrow * HPCG_MAX_ROW_LEN * 1.0f) / divisor;
        local_int_t v288x512x512[] = {1057190464, 132276512, 16615072, 2074384};
        local_int_t v296x512x512[] = {1095636608, 136618560, 16967616, 2883872};
        local_int_t* v = nrow == 288 * 512 * 512 ? v288x512x512
            : nrow == 296 * 512 * 512            ? v296x512x512
            : nullptr;
        if (v != nullptr)
        {
            if (level == 0)
                estimated_size = v[0];
            else if (level == 1)
                estimated_size = v[1];
            else if (level == 2)
                estimated_size = v[2];
            else if (level == 3)
                estimated_size = v[3];
        }

        A->sellASliceMrl = new local_int_t[num_slices + 1];
        A->sellLSliceMrl = new local_int_t[num_slices + 1];
        A->sellUSliceMrl = new local_int_t[num_slices + 1];

        A->sellAPermColumns = new local_int_t[padded_nrow * HPCG_MAX_ROW_LEN];
        A->sellLPermColumns = new local_int_t[estimated_size]; // Should be much less
        A->sellUPermColumns = new local_int_t[estimated_size]; // Should be much less

        A->sellAPermValues = new double[padded_nrow * HPCG_MAX_ROW_LEN];
        A->sellLPermValues = new double[estimated_size];
        if (Use_Hpcg_Mem_Reduction)
        {
            // This optimization benefits from strictly L and U
            A->sellUPermValues = A->sellLPermValues;
            opt_mem += 1 * estimated_size * (sizeof(double));
        }
        else
        {
            A->sellUPermValues = new double[estimated_size];
            opt_mem += 2 * estimated_size * (sizeof(double));
        }

        opt_mem += 3 * num_slices * sizeof(local_int_t);
        opt_mem += padded_nrow * HPCG_MAX_ROW_LEN * (sizeof(double) + sizeof(local_int_t));
        opt_mem += 2 * estimated_size * sizeof(local_int_t);

        // SpSV related memory optimization
        // HPCG estimated buffer size
        if (Use_Hpcg_Mem_Reduction && nrow % 8 == 0)
        {
            // Helps SpSV reduce memory footprint/HPCG specific
            opt_mem += 2048 + 8 * sizeof(local_int_t) * nrow;
            A->bufferSvL = new char[2048 + 8 * sizeof(local_int_t) * nrow];
            // Same buffer since they both share the same diagional
            A->bufferSvU = A->bufferSvL;
        }

        // Temporal vectors for permutation
        opt_mem += 2 * A->localNumberOfRows * sizeof(double);

        A = A->Ac;
    }
    return opt_mem;
}

//////////////////////// Deallocate CPU/Grace Memory data structures //////////
void DeleteMatrixCpu(SparseMatrix& A)
{
    local_int_t numberOfMgLevels = 4;
    SparseMatrix* AA = &A;
    for (int level = 0; level < numberOfMgLevels; ++level)
    {
#ifndef HPCG_CONTIGUOUS_ARRAYS
        for (local_int_t i = 0; i < AA->localNumberOfRows; ++i)
        {
            delete[] AA->matrixValues[i];
            delete[] AA->mtxIndL[i];
        }
#else
        delete[] AA->matrixValues[0];
        delete[] AA->mtxIndL[0];
#endif
        if (AA->title)
            delete[] AA->title;
        if (AA->nonzerosInRow)
            delete[] AA->nonzerosInRow;
        if (AA->mtxIndG)
            delete[] AA->mtxIndG;
        if (AA->mtxIndL)
            delete[] AA->mtxIndL;
        if (AA->matrixValues)
            delete[] AA->matrixValues;
        if (AA->matrixDiagonal)
            delete[] AA->matrixDiagonal;
        if (AA->tempBuffer)
            delete[] AA->tempBuffer;

        if (AA->ref2opt)
            delete[] AA->ref2opt;
        if (AA->opt2ref)
            delete[] AA->opt2ref;

#ifndef HPCG_NO_MPI
        if (AA->elementsToSend)
            delete[] AA->elementsToSend;
        if (AA->neighbors)
            delete[] AA->neighbors;
        if (AA->receiveLength)
            delete[] AA->receiveLength;
        if (AA->sendLength)
            delete[] AA->sendLength;
        if (AA->sendBuffer)
            delete[] AA->sendBuffer;
#endif

        if (AA->sellASliceMrl)
            delete[] AA->sellASliceMrl;
        if (AA->sellLSliceMrl)
            delete[] AA->sellLSliceMrl;
        if (AA->sellUSliceMrl)
            delete[] AA->sellUSliceMrl;

        if (AA->sellAPermColumns)
            delete[] AA->sellAPermColumns;
        if (AA->sellLPermColumns)
            delete[] AA->sellLPermColumns;
        if (AA->sellUPermColumns)
            delete[] AA->sellUPermColumns;

        if (AA->sellAPermValues)
            delete[] AA->sellAPermValues;
        if (AA->sellLPermValues)
            delete[] AA->sellLPermValues;
        if (AA->sellUPermValues && !Use_Hpcg_Mem_Reduction)
            delete[] AA->sellUPermValues;

        if (AA->diagonal)
            delete[] AA->diagonal;

        if (AA->csrExtOffsets)
            delete[] AA->csrExtOffsets;
        if (AA->csrExtColumns)
            delete[] AA->csrExtColumns;
        if (AA->csrExtValues)
            delete[] AA->csrExtValues;

        if (AA->f2cPerm)
            delete[] AA->f2cPerm;
        if (AA->cpuAux.color)
            delete[] AA->cpuAux.color;
        if (AA->cpuAux.firstRowOfColor)
            delete[] AA->cpuAux.firstRowOfColor;
        if (AA->cpuAux.nRowsWithColor)
            delete[] AA->cpuAux.nRowsWithColor;

        if (AA->bufferSvL)
            delete[] AA->bufferSvL;
        if (AA->nvplSparseOpt.vecX)
            nvpl_sparse_destroy_dn_vec(AA->nvplSparseOpt.vecX);
        if (AA->nvplSparseOpt.vecY)
            nvpl_sparse_destroy_dn_vec(AA->nvplSparseOpt.vecY);

        if (AA->geom != 0)
        {
            DeleteGeometry(*AA->geom);
            delete AA->geom;
            AA->geom = 0;
        }
        if (AA->mgData != 0)
        {
            DeleteMGData(*AA->mgData);
            delete AA->mgData;
            AA->mgData = 0;
        } // Delete MG data

#ifndef HPCG_NO_MPI
        if (P2P_Mode == MPI_CPU_All2allv)
        {
            delete[] AA->scounts;
            delete[] AA->rcounts;
            delete[] AA->sdispls;
            delete[] AA->rdispls;
        }
#endif

        AA = AA->Ac;
    }
}

///////// Find the size of CPU reference allocated memory //
size_t EstimateCpuRefMem(SparseMatrix& A)
{
    // Borrowed from ReportResults.cpp
    const int numberOfMgLevels = 4; // Number of levels including first
    local_int_t fnrow = A.localNumberOfRows;

    const SparseMatrix* Af = &A;
    double numberOfNonzerosPerRow
        = 27.0; // We are approximating a 27-point finite element/volume/difference 3D stencil

    double fnbytes = ((double) sizeof(Geometry));           // Geometry struct in main.cpp
    //fnbytes += ((double) sizeof(double) * fNumberOfCgSets); // testnorms_data in main.cpp

    // Model for GenerateProblem_ref.cpp
    fnbytes += fnrow * sizeof(char);                                             // array nonzerosInRow
    fnbytes += fnrow * ((double) sizeof(global_int_t*));                         // mtxIndG
    fnbytes += fnrow * ((double) sizeof(local_int_t*));                          // mtxIndL
    fnbytes += fnrow * ((double) sizeof(double*));                               // matrixValues
    fnbytes += fnrow * ((double) sizeof(double*));                               // matrixDiagonal
    fnbytes += fnrow * numberOfNonzerosPerRow * ((double) sizeof(local_int_t));  // mtxIndL[1..nrows]
    fnbytes += fnrow * numberOfNonzerosPerRow * ((double) sizeof(double));       // matrixValues[1..nrows]
    //fnbytes += fnrow * numberOfNonzerosPerRow * ((double) sizeof(global_int_t)); // mtxIndG[1..nrows]
    fnbytes += fnrow * ((double) 3 * sizeof(double));                            // x, b, xexact

    // Model for CGData.hpp
    double fncol = ((global_int_t) A.localNumberOfColumns);
    fnbytes += fnrow * ((double) 2 * sizeof(double)); // r, Ap
    fnbytes += fncol * ((double) 2 * sizeof(double)); // z, p

    std::vector<double> fnbytesPerLevel(numberOfMgLevels); // Count byte usage per level (level 0 is main CG level)
    fnbytesPerLevel[0] = fnbytes;

    Af = A.Ac;
    for (int i = 1; i < numberOfMgLevels; ++i)
    {
        double fnrow_Af = Af->localNumberOfRows;
        double fncol_Af = ((global_int_t) Af->localNumberOfColumns);
        double fnbytes_Af = 0.0;
        // Model for GenerateCoarseProblem.cpp
        fnbytes_Af += fnrow_Af * ((double) sizeof(local_int_t)); // f2cOperator
        fnbytes_Af += fnrow_Af * ((double) sizeof(double));      // rc
        fnbytes_Af += 2.0 * fncol_Af
            * ((double) sizeof(double)); // xc, Axf are estimated based on the size of these arrays on rank 0
        fnbytes_Af += ((double) (sizeof(Geometry) + sizeof(SparseMatrix) + 3 * sizeof(Vector)
            + sizeof(MGData))); // Account for structs geomc, Ac, rc, xc, Axf - (minor)

        // Model for GenerateProblem.cpp (called within GenerateCoarseProblem.cpp)
        fnbytes_Af += fnrow_Af * sizeof(char);                                             // array nonzerosInRow
        fnbytes_Af += fnrow_Af * ((double) sizeof(global_int_t*));                         // mtxIndG
        fnbytes_Af += fnrow_Af * ((double) sizeof(local_int_t*));                          // mtxIndL
        fnbytes_Af += fnrow_Af * ((double) sizeof(double*));                               // matrixValues
        fnbytes_Af += fnrow_Af * ((double) sizeof(double*));                               // matrixDiagonal
        fnbytes_Af += fnrow_Af * numberOfNonzerosPerRow * ((double) sizeof(local_int_t));  // mtxIndL[1..nrows]
        fnbytes_Af += fnrow_Af * numberOfNonzerosPerRow * ((double) sizeof(double));       // matrixValues[1..nrows]
        //fnbytes_Af += fnrow_Af * numberOfNonzerosPerRow * ((double) sizeof(global_int_t)); // mtxIndG[1..nrows]

// Model for SetupHalo_ref.cpp
#ifndef HPCG_NO_MPI
        fnbytes_Af += ((double) sizeof(double) * Af->totalToBeSent);              // sendBuffer
        fnbytes_Af += ((double) sizeof(local_int_t) * Af->totalToBeSent);         // elementsToSend
        fnbytes_Af += ((double) sizeof(int) * Af->numberOfSendNeighbors);         // neighbors
        fnbytes_Af += ((double) sizeof(local_int_t) * Af->numberOfSendNeighbors); // receiveLength, sendLength
#endif
        fnbytesPerLevel[i] = fnbytes_Af;
        fnbytes += fnbytes_Af; // Running sum
        Af = Af->Ac;           // Go to next coarse level
    }

    return fnbytes;
}
//////////////////////// Generate Problem /////////////////////////////////////
/*
    Inclusive Prefix Sum
*/
void PrefixsumCpu(int* x, int N)
{
    local_int_t* suma;
#pragma omp parallel
    {
        const int ithread = omp_get_thread_num();
        const int nthreads = omp_get_num_threads();
        local_int_t sum = local_int_t{};

#pragma omp single
        {
            suma = new local_int_t[nthreads + 1];
            suma[0] = local_int_t{};
        }

#pragma omp for schedule(static)
        for (auto i = 0; i < N; i++)
        {
            sum += x[i];
            x[i] = sum;
        }
        suma[ithread + 1] = sum;

#pragma omp barrier

        local_int_t offset = local_int_t{};
        for (local_int_t i = 0; i < (ithread + 1); i++)
        {
            offset += suma[i];
        }

#pragma omp for schedule(static)
        for (auto i = 0; i < N; i++)
        {
            x[i] += offset;
        }

#pragma omp single
        delete[] suma;
    }
}

//////////////////////// Optimize Problem /////////////////////////////////////
/*
    //Assumes 8 colors
*/
void Prefixsum8ColCpu(local_int_t* colors, local_int_t* temp, local_int_t N)
{
    local_int_t* sum_acc;
#pragma omp parallel
    {
        const int ithread = omp_get_thread_num();
        const int nthreads = omp_get_num_threads();
        local_int_t sum[8] = {0, 0, 0, 0, 0, 0, 0, 0};

#pragma omp single
        {
            sum_acc = new local_int_t[8 * (nthreads + 1)];
            sum_acc[0] = local_int_t{};
            sum_acc[1] = local_int_t{};
            sum_acc[2] = local_int_t{};
            sum_acc[3] = local_int_t{};
            sum_acc[4] = local_int_t{};
            sum_acc[5] = local_int_t{};
            sum_acc[6] = local_int_t{};
            sum_acc[7] = local_int_t{};
        }

#pragma omp for schedule(static)
        for (auto i = 0; i < N; i++)
        {
            local_int_t c = colors[i];
            sum[c]++;
            temp[i] = sum[c] - 1;
        }

        for (int i = 0; i < 8; i++)
            sum_acc[8 * (ithread + 1) + i] = sum[i];

#pragma omp barrier

        local_int_t offset[8] = {0, 0, 0, 0, 0, 0, 0, 0};
        for (local_int_t i = 1; i < (ithread + 1); i++)
        {
            for (int j = 0; j < 8; j++)
                offset[j] += sum_acc[8 * i + j];
        }

#pragma omp for schedule(static)
        for (local_int_t i = 0; i < N; i++)
        {
            temp[i] += offset[colors[i]];
        }
    }

    delete[] sum_acc;
}

local_int_t parCount(local_int_t* arr, local_int_t N, local_int_t val)
{
    local_int_t count = 0;
#pragma omp parallel for reduction(+:count)
    for (local_int_t i  = 0; i < N; i++)
    {
        count += (arr[i] == val);
    }
    return count;
}

local_int_t parMaxElement(local_int_t* arr, local_int_t N)
{
    local_int_t maxElement = arr[0];
#pragma omp parallel for reduction(max:maxElement)
    for (local_int_t i  = 1; i < N; i++)
    {
        maxElement = arr[i] > maxElement ? arr[i] : maxElement;
    }
    return maxElement;
}

/*
    CPU Kernel
    Minmax hashing used for graph coloring
    Colring is based on Jones-Plassmann Luby algorithm
    Create hash by reversing bits
*/
unsigned int hash(unsigned int n)
{
    unsigned int rev = 0;
    for (int i = 0; i < sizeof(unsigned int) * 8; i++)
    {
        rev = rev << 1;
        if ((n & 1) > 0)
        {
            rev = rev ^ 1;
        }
        n = n >> 1;
    }
    return rev;
}

/*
    CPU Kernel
    Minmax hashing used for graph coloring
    Colring is based on Jones-Plassmann Luby algorithm
*/
void minmaxHashStep(SparseMatrix& A, int next_color, int next_color_p1, local_int_t nrow,
    local_int_t* color, std::vector<local_int_t>& count_colors)
{
#pragma omp parallel for
    for (local_int_t i = 0; i < nrow; i++)
    {
        // Skip already colored rows
        if (color[i] != -1)
            continue;
        unsigned int i_rand = hash(i);
        // is it local min or max?
        bool not_min = false;
        bool not_max = false;
        for (int j = 0; j < A.nonzerosInRow[i]; j++)
        {
            local_int_t col_index = A.mtxIndL[i][j];
            // Skip diagonal and external values
            if (col_index == i || col_index >= nrow)
                continue;
            int j_color = color[col_index];
            // skip colored neighbours (consider only graph formed by removing them)
            if (j_color != -1 && j_color != next_color && j_color != next_color_p1)
            {
                continue;
            }
            unsigned int j_rand = hash(col_index);
            // stop if any neighbour is greater or least than (i.e., not local min/max)
            if (i_rand <= j_rand)
                not_max = true;
            if (i_rand >= j_rand)
                not_min = true;
            if (not_max && not_min)
                break;
        }
        // we made it here, which means we have no higher/lower uncoloured neighbour
        // so, we are selected
        if (!not_min)
        {
            color[i] = next_color;
        }
        else if (!not_max)
        { // else b/c we can be either min or max
            color[i] = next_color_p1;
        }
    }
}

/*
    CPU Kernel
    Minmax hashing used for graph coloring
    Colring is based on Jones-Plassmann Luby algorithm
    Create hash by reversing bits
*/
void testHashStep3(SparseMatrix& A,local_int_t* color, int next_color, unsigned int seed, int check_color)
{
    local_int_t nrow = A.localNumberOfRows;
#pragma omp parallel
    for (int i = 0; i < nrow; i++)
    {
        if (color[i] != check_color)
            continue;
        int trial_color;
        int iter;
        for (iter = 0; iter < check_color; iter++)
        {
            trial_color = (iter + (i % seed) + next_color) % seed;
            bool color_used = false;
            for (int jj = 0; jj < A.nonzerosInRow[i]; jj++)
            {
                local_int_t j = A.mtxIndL[i][jj];

                // skip diagonal and external values
                if (j == 1 || j == -1 || j >= nrow)
                    continue;

                int j_color = color[j];
                if (j_color == trial_color)
                {
                    color_used = true;
                    break;
                }
            }
            if (!color_used)
            {
                color[i] = trial_color;
            }
        }
    }
}

/*
    Based on Jones-Plassmann Luby algorithm
*/
void ColorMatrixCpu(SparseMatrix& A, int* num_colors)
{
    local_int_t nrow = A.localNumberOfRows;
    local_int_t* color = A.cpuAux.color;

#pragma omp parallel for
    for (auto c = 0; c < nrow; c++)
    {
        color[c] = -1;
    }

    int color_order[8] = {7, 4, 2, 6, 5, 1, 3, 0};
    int perm_colors[8] = {color_order[0], color_order[7], color_order[4], color_order[3], color_order[2],
        color_order[5], color_order[6], color_order[1]};
    int next_color = 0;
    int seed = 0;
    int done = 0;
    int step = 0;
    int max_colors = 8;
    *num_colors = 0;
    local_int_t colored = 0;
    std::vector<local_int_t> count_colors(max_colors * 2);
    while (!done && step < max_colors / 2)
    {
        done = 1;
        if (next_color < 7)
        {
            // minmax hash step
            minmaxHashStep(A, perm_colors[next_color], perm_colors[next_color + 1], nrow, color, count_colors);
            // count how many rows we just colored
            count_colors[perm_colors[next_color]] = parCount(color, nrow, perm_colors[next_color]);//std::count(color, color + nrow, perm_colors[next_color]);
            count_colors[perm_colors[next_color + 1]] = parCount(color, nrow, perm_colors[next_color + 1]);
            colored += count_colors[perm_colors[next_color]] + count_colors[perm_colors[next_color + 1]];
        }
        else
        {
            // minmax hash step
            minmaxHashStep(A, next_color, next_color + 1, nrow, color, count_colors);
            // count how many rows we just colored
            count_colors[next_color] = parCount(color, nrow, perm_colors[next_color]);
            count_colors[next_color + 1] = parCount(color, nrow, perm_colors[next_color + 1]);
            colored += count_colors[next_color] + count_colors[next_color + 1];
        }
        // Are we done?
        if (colored < nrow)
            done = 0;
        step++;
        next_color += 2;
    }

    // Up to here, matrix has been colored
    // We'll try to improve the coloring
    int check_color;
    int color_target = 1;
    int recolor_times = 10;
    int maxx = parMaxElement(color, nrow);

    int max_used_color = maxx;
    if (maxx > 15)
    {
        for (int target_color_count = maxx - 1; target_color_count > 13; target_color_count--)
        {
            int it_count = 0;
            while (it_count < recolor_times && maxx > target_color_count)
            {
                for (check_color = maxx; check_color >= color_target; check_color--)
                {
                    testHashStep3(A, color, it_count, 15, check_color);
                }
                maxx = parMaxElement(color, nrow);
                count_colors[maxx] = parCount(color, nrow, maxx);
                it_count++;
            }
        }
    }

    for (int i = 0; i < max_colors; i++)
    {
        count_colors[i] = 0;
    }

    for (check_color = 0; check_color < next_color; check_color++)
    {
        count_colors[check_color] = parCount(color, nrow, check_color);
    }
    max_used_color = 0;
    for (int i = 0; i < max_colors; i++)
    {
        if (count_colors[i] > 0)
            max_used_color = i;
    }

    *num_colors = max_used_color + 1;
    local_int_t nvcolors = *num_colors;
    local_int_t *counter, nthreads;
    #pragma omp parallel shared(counter, nthreads)
    {
        const int ithread = omp_get_thread_num();
        nthreads = omp_get_num_threads();
        #pragma omp single
        {
            counter = new local_int_t[nvcolors * omp_get_num_threads()];
        }

#pragma omp for
        for(int i = 0; i < nvcolors * nthreads; i++)
        {
            counter[i] = 0;
        }

#pragma omp for
        for (local_int_t i = 0; i < nrow; i++)
        {
            local_int_t c = color[i];
            counter[ithread * nvcolors + c]++;
        }
    }

    // Sequential instead of atomics inside the omp region
    for (int j = 0; j < nvcolors; j++)
    {
        local_int_t cc = 0;
        for(int i = 0; i < nthreads; i++)
        {
            cc+= counter[i * nvcolors + j];
        }
        
        A.cpuAux.nRowsWithColor[j] = cc;
    }

    delete [] counter;
}

/*
    Permute matrix rows and create A, L, and U in Sliced-Ellpack format
*/
void CreateSellPermCpu(SparseMatrix& A)
{
    local_int_t nrow = A.localNumberOfRows;
    local_int_t slice_size = A.slice_size;
    local_int_t* temp = new local_int_t[nrow + 1];
    local_int_t* color = A.cpuAux.color;
    Prefixsum8ColCpu(color, temp, nrow);

#pragma omp parallel for
    for (auto i = 0; i < nrow; i++)
    {
        auto c = color[i];
        local_int_t targetRowIdx = A.cpuAux.firstRowOfColor[c] + temp[i];
        A.ref2opt[i] = targetRowIdx;
        A.opt2ref[targetRowIdx] = i;
    }
    delete[] temp;

    A.csrExtOffsets[0] = 0;
#pragma omp parallel for
    for (auto i = 0; i < nrow; i++)
    {
        local_int_t originalRow = A.opt2ref[i];
        A.csrExtOffsets[i + 1] = A.cpuAux.tempIndex[originalRow];
    }
    delete[] A.cpuAux.tempIndex;
    PrefixsumCpu(A.csrExtOffsets + 1, nrow);

// Don't translate external rows
#pragma omp parallel for
    for (global_int_t i = nrow; i < A.localNumberOfColumns; i++)
    {
        A.ref2opt[i] = i;
        A.opt2ref[i] = i;
    }

    // Create data structures
    local_int_t num_slices = (nrow + slice_size - 1) / slice_size;
    local_int_t num_threads = omp_get_max_threads();
    local_int_t rows_per_thread = (nrow + num_threads - 1) / num_threads;

#pragma omp parallel for
    for (auto slice = 0; slice < num_slices + 1; slice++)
    {
        A.sellASliceMrl[slice] = (slice) *HPCG_MAX_ROW_LEN * slice_size;
        A.sellLSliceMrl[slice] = 0;
        A.sellUSliceMrl[slice] = 0;
    }

// Create A in SELL
#pragma omp parallel for
    for (local_int_t i = 0; i < nrow; i++)
    {
        local_int_t l_nnz = 0;
        local_int_t u_nnz = 0;
        local_int_t ext_nnz = 0;
        local_int_t originalRow = A.opt2ref[i];
        local_int_t slice_id = i / slice_size;
        local_int_t in_slice_id = i % slice_size;

        local_int_t nnz_counter = 0;
        local_int_t ext_nnz_index = A.csrExtOffsets[i];
        for (local_int_t j = 0; j < A.nonzerosInRow[originalRow]; j++)
        {
            local_int_t col = A.mtxIndL[originalRow][j];
            double val = A.matrixValues[originalRow][j];
            local_int_t new_col = A.ref2opt[col];
            if (col < nrow)
            {
                // Locality is bad, consider blocking
                auto index = slice_id * slice_size * HPCG_MAX_ROW_LEN + nnz_counter * slice_size + in_slice_id;
                A.sellAPermColumns[index] = new_col;
                A.sellAPermValues[index] = val;
                nnz_counter++;

                if (col == originalRow)
                { // diagonal
                    A.diagonal[i] = val;
                }
                else if (new_col < i)
                { // lower diagonal
                    l_nnz++;
                }
                else if (new_col > i)
                { // upper diagonal
                    u_nnz++;
                }
            }
            else
            {
                A.csrExtColumns[ext_nnz_index] = col;
                A.csrExtValues[ext_nnz_index] = val;   
                ext_nnz_index++;
            }
        }

        for (; nnz_counter < HPCG_MAX_ROW_LEN; nnz_counter++)
        {
            auto index = slice_id * slice_size * HPCG_MAX_ROW_LEN + nnz_counter * slice_size + in_slice_id;
            A.sellAPermColumns[index] = -1;
            A.sellAPermValues[index] = 0.0f;
        }

        local_int_t thread_slice0 = (slice_id * slice_size) / rows_per_thread;
        local_int_t thread_slice1 = ((slice_id + 1) * slice_size - 1) / rows_per_thread;

        if (thread_slice0 == thread_slice1 && rows_per_thread > slice_size)
        {
            A.sellLSliceMrl[slice_id + 1] = std::max(A.sellLSliceMrl[slice_id + 1], l_nnz * slice_size);
            A.sellUSliceMrl[slice_id + 1] = std::max(A.sellUSliceMrl[slice_id + 1], u_nnz * slice_size);
        }
        else
        {
#pragma omp critical
            {
                A.sellLSliceMrl[slice_id + 1] = std::max(A.sellLSliceMrl[slice_id + 1], l_nnz * slice_size);
                A.sellUSliceMrl[slice_id + 1] = std::max(A.sellUSliceMrl[slice_id + 1], u_nnz * slice_size);
            }
        }
    }

    // Next Pefix-Sum
    for (auto slice = 0; slice < num_slices; slice++)
    {
        A.sellLSliceMrl[slice + 1] += A.sellLSliceMrl[slice];
        A.sellUSliceMrl[slice + 1] += A.sellUSliceMrl[slice];
    }

// Create lower and upper SELL formats
#pragma omp parallel for
    for (auto i = 0; i < nrow; i++)
    {
        local_int_t row_inblock_id = i % slice_size;
        local_int_t row_block_id = i / slice_size;

        local_int_t row_start_index = row_block_id * HPCG_MAX_ROW_LEN * slice_size + row_inblock_id;
        local_int_t row_end_index = row_start_index + HPCG_MAX_ROW_LEN * slice_size;

        local_int_t l_row_start = A.sellLSliceMrl[row_block_id] + row_inblock_id;
        local_int_t u_row_start = A.sellUSliceMrl[row_block_id] + row_inblock_id;

        local_int_t l_len = (A.sellLSliceMrl[row_block_id + 1] - A.sellLSliceMrl[row_block_id]) / slice_size;
        local_int_t u_len = (A.sellUSliceMrl[row_block_id + 1] - A.sellUSliceMrl[row_block_id]) / slice_size;

        local_int_t l_row_end = l_row_start + l_len * slice_size;
        local_int_t u_row_end = u_row_start + u_len * slice_size;

        for (local_int_t j = row_start_index; j < row_end_index; j += slice_size)
        {
            local_int_t col = A.sellAPermColumns[j];
            double val = A.sellAPermValues[j];
            if (col != -1 && col < i)
            {
                A.sellLPermColumns[l_row_start] = col;
                A.sellLPermValues[l_row_start] = -1;
                l_row_start += slice_size;
            }
            else if (col != -1 && col > i)
            {
                A.sellUPermColumns[u_row_start] = col;
                A.sellUPermValues[u_row_start] = -1;
                u_row_start += slice_size;
            }
        }

        // Padd lower
        for (local_int_t j = l_row_start; j < l_row_end; j += slice_size)
        {
            A.sellLPermColumns[j] = -1;
            A.sellLPermValues[j] = -1;
        }

        // Padd upper
        for (local_int_t j = u_row_start; j < u_row_end; j += slice_size)
        {
            A.sellUPermColumns[j] = -1;
            A.sellUPermValues[j] = -1;
        }
    }
}

/*
    Permute a vector using coloring buffer
    Note: Allocate and deallocates temp buffer (not efficient,
    but part of auxiliary compuattaions that in optimized CG
*/
void PermVectorCpu(local_int_t* perm, Vector& x, local_int_t length)
{
    Vector reorder;
    InitializeVector(reorder, length, CPU);
    CopyVector(x, reorder);
    CopyAndReorderVector(reorder, x, perm);
    DeleteVector(reorder);
}

/*
    Permutes the space injection operator
*/
void F2cPermCpu(local_int_t nrow_c, local_int_t* f2c, local_int_t* f2cPerm, local_int_t* perm_f, local_int_t* iperm_c)
{
#pragma omp parallel for
    for (local_int_t i = 0; i < nrow_c; i++)
    {
        f2cPerm[i] = perm_f[f2c[iperm_c[i]]];
    }
}

//////////////////////// Test CG //////////////////////////////////////////////
/*
    CPU Kernel
    Replaces matrix, in sliced ELLPACK format, diagonal with values in
        diagonal
*/
void ReplaceMatrixDiagonalCpu(SparseMatrix& A, Vector diagonal)
{
#pragma omp parallel for
    for (local_int_t i = 0; i < A.localNumberOfRows; i++)
    {
        local_int_t slice_id = i / A.slice_size;
        local_int_t in_slice_id = i % A.slice_size;
        local_int_t slice_start = A.sellASliceMrl[slice_id];
        local_int_t slice_len = A.sellASliceMrl[slice_id + 1] - slice_start;
        local_int_t start = slice_start + in_slice_id;
        local_int_t end = start + slice_len;
        for (local_int_t offset = start; offset < end; offset += A.slice_size)
        {
            local_int_t col = A.sellAPermColumns[offset];
            if (col == i)
                A.sellAPermValues[offset] = diagonal.values[i];
        }

        A.diagonal[i] = diagonal.values[i];
    }
}

//////////////////////// CG Support Kernels ///////////////////////////////////
/*
    CPU Kernel
    Computes dot product using SVE if available
*/
void ComputeDotProductCpu(
    const local_int_t n, const Vector& x, const Vector& y, double& local_result, bool& isOptimized)
{

    local_result = 0.0;
    double* xv = x.values;
    double* yv = y.values;
#if defined(__ARM_FEATURE_SVE) && !defined(INDEX_64)
    local_int_t offset0 = 0 * svcntd();
    local_int_t offset1 = 1 * svcntd();
    local_int_t offset2 = 2 * svcntd();
    local_int_t offset3 = 3 * svcntd();
    if (yv == xv)
    {
#ifndef HPCG_NO_OPENMP
#pragma omp parallel
        {
            int threadId = omp_get_thread_num();
            int nthreads = omp_get_num_threads();
            local_int_t rowsperthread = (n + nthreads - 1) / nthreads;
            local_int_t first = threadId * rowsperthread;
            local_int_t last = std::min(first + rowsperthread, n);
            svfloat64_t lres0 = svdup_f64(0.0);
            svfloat64_t lres1 = svdup_f64(0.0);
            svfloat64_t lres2 = svdup_f64(0.0);
            svfloat64_t lres3 = svdup_f64(0.0);

            local_int_t stride = 4 * svcntd();
            local_int_t len = last - first;
            local_int_t limit = ((len + stride - 1) / stride) * stride;

            for (local_int_t i = first; i < first + limit; i += stride)
            {
                svbool_t pg0 = svwhilelt_b64(i + offset0, last);
                svbool_t pg1 = svwhilelt_b64(i + offset1, last);
                svbool_t pg2 = svwhilelt_b64(i + offset2, last);
                svbool_t pg3 = svwhilelt_b64(i + offset3, last);
                svfloat64_t sv_xv0 = svld1_f64(pg0, &xv[i + offset0]);
                svfloat64_t sv_xv1 = svld1_f64(pg1, &xv[i + offset1]);
                svfloat64_t sv_xv2 = svld1_f64(pg2, &xv[i + offset2]);
                svfloat64_t sv_xv3 = svld1_f64(pg3, &xv[i + offset3]);
                lres0 = svmla_f64_m(pg0, lres0, sv_xv0, sv_xv0);
                lres1 = svmla_f64_m(pg1, lres1, sv_xv1, sv_xv1);
                lres2 = svmla_f64_m(pg2, lres2, sv_xv2, sv_xv2);
                lres3 = svmla_f64_m(pg3, lres3, sv_xv3, sv_xv3);
            }
#pragma omp critical
            {
                local_result += svaddv_f64(svptrue_b64(), lres0);
                local_result += svaddv_f64(svptrue_b64(), lres1);
                local_result += svaddv_f64(svptrue_b64(), lres2);
                local_result += svaddv_f64(svptrue_b64(), lres3);
            }
        }
#endif // HPCG_NO_OPENMP
    }
    else
    {
#ifndef HPCG_NO_OPENMP
#pragma omp parallel
        {
            int threadId = omp_get_thread_num();
            int nthreads = omp_get_num_threads();
            local_int_t rowsperthread = (n + nthreads - 1) / nthreads;
            local_int_t first = threadId * rowsperthread;
            local_int_t last = std::min(first + rowsperthread, n);
            svfloat64_t lres0 = svdup_f64(0.0);
            svfloat64_t lres1 = svdup_f64(0.0);
            svfloat64_t lres2 = svdup_f64(0.0);
            svfloat64_t lres3 = svdup_f64(0.0);

            local_int_t stride = 4 * svcntd();
            local_int_t len = last - first;
            local_int_t limit = ((len + stride - 1) / stride) * stride;

            for (local_int_t i = first; i < first + limit; i += stride)
            {
                svbool_t pg0 = svwhilelt_b64(i + offset0, last);
                svbool_t pg1 = svwhilelt_b64(i + offset1, last);
                svbool_t pg2 = svwhilelt_b64(i + offset2, last);
                svbool_t pg3 = svwhilelt_b64(i + offset3, last);
                svfloat64_t sv_xv0 = svld1_f64(pg0, &xv[i + offset0]);
                svfloat64_t sv_xv1 = svld1_f64(pg1, &xv[i + offset1]);
                svfloat64_t sv_xv2 = svld1_f64(pg2, &xv[i + offset2]);
                svfloat64_t sv_xv3 = svld1_f64(pg3, &xv[i + offset3]);
                svfloat64_t sv_yv0 = svld1_f64(pg0, &yv[i + offset0]);
                svfloat64_t sv_yv1 = svld1_f64(pg1, &yv[i + offset1]);
                svfloat64_t sv_yv2 = svld1_f64(pg2, &yv[i + offset2]);
                svfloat64_t sv_yv3 = svld1_f64(pg3, &yv[i + offset3]);
                lres0 = svmla_f64_m(pg0, lres0, sv_xv0, sv_yv0);
                lres1 = svmla_f64_m(pg1, lres1, sv_xv1, sv_yv1);
                lres2 = svmla_f64_m(pg2, lres2, sv_xv2, sv_yv2);
                lres3 = svmla_f64_m(pg3, lres3, sv_xv3, sv_yv3);
            }
#pragma omp critical
            {
                local_result += svaddv_f64(svptrue_b64(), lres0);
                local_result += svaddv_f64(svptrue_b64(), lres1);
                local_result += svaddv_f64(svptrue_b64(), lres2);
                local_result += svaddv_f64(svptrue_b64(), lres3);
            }
#endif // HPCG_NO_OPENMP
        }
    }
#else
    if (yv == xv)
    {
#ifndef HPCG_NO_OPENMP
#pragma omp parallel for reduction(+ : local_result)
#endif
        for (local_int_t i = 0; i < n; i++)
            local_result += xv[i] * xv[i];
    }
    else
    {
#ifndef HPCG_NO_OPENMP
#pragma omp parallel for reduction(+ : local_result)
#endif
        for (local_int_t i = 0; i < n; i++)
            local_result += xv[i] * yv[i];
    }
#endif
}

/*
    CPU Kernel
    Computes WAXPBY in SVE if available
*/
int ComputeWAXPBYCpu(const local_int_t n, const double alpha, const Vector& x, const double beta, const Vector& y,
    Vector& w, bool& isOptimized)
{
    const double* const xv = x.values;
    const double* const yv = y.values;
    double* const wv = w.values;

#ifndef HPCG_NO_OPENMP
#pragma omp parallel
    {
        int threadId = omp_get_thread_num();
        int nthreads = omp_get_num_threads();
        local_int_t rowsperthread = (n + nthreads - 1) / nthreads;
        local_int_t first = rowsperthread * threadId;
        local_int_t last = std::min(first + rowsperthread, n);
#else  // HPCG_NO_OPENMP
    local_int_t first = 0;
    local_int_t last = n;
#endif // HPCG_NO_OPENMP
        local_int_t i;

#if defined(__ARM_FEATURE_SVE) && !defined(INDEX_64) // INDEX_64 in Geometry.hpp
        local_int_t stride = 4 * svcntd();
        local_int_t len = last - first;
        local_int_t limit = ((len + stride - 1) / stride) * stride;
#endif

        if (alpha == 1.0)
        {
#if defined(__ARM_FEATURE_SVE) && !defined(INDEX_64) // INDEX_64 in Geometry.hpp
            svfloat64_t sv_beta = svdup_f64(beta);
            local_int_t off0 = svcntd() * 0;
            local_int_t off1 = svcntd() * 1;
            local_int_t off2 = svcntd() * 2;
            local_int_t off3 = svcntd() * 3;

            for (i = first; i < last; i += stride)
            {
                // Generate predicates
                svbool_t pg0 = svwhilelt_b64(i + off0, last);
                svbool_t pg1 = svwhilelt_b64(i + off1, last);
                svbool_t pg2 = svwhilelt_b64(i + off2, last);
                svbool_t pg3 = svwhilelt_b64(i + off3, last);

                // Load xv values
                svfloat64_t sv_xv0 = svld1_f64(pg0, &xv[i + off0]);
                svfloat64_t sv_xv1 = svld1_f64(pg1, &xv[i + off1]);
                svfloat64_t sv_xv2 = svld1_f64(pg2, &xv[i + off2]);
                svfloat64_t sv_xv3 = svld1_f64(pg3, &xv[i + off3]);

                // Load yv values
                svfloat64_t sv_yv0 = svld1_f64(pg0, &yv[i + off0]);
                svfloat64_t sv_yv1 = svld1_f64(pg1, &yv[i + off1]);
                svfloat64_t sv_yv2 = svld1_f64(pg2, &yv[i + off2]);
                svfloat64_t sv_yv3 = svld1_f64(pg3, &yv[i + off3]);

                // Compute result
                svfloat64_t sv_wv0 = svmla_f64_z(pg0, sv_xv0, sv_beta, sv_yv0);
                svfloat64_t sv_wv1 = svmla_f64_z(pg1, sv_xv1, sv_beta, sv_yv1);
                svfloat64_t sv_wv2 = svmla_f64_z(pg2, sv_xv2, sv_beta, sv_yv2);
                svfloat64_t sv_wv3 = svmla_f64_z(pg3, sv_xv3, sv_beta, sv_yv3);

                // Store
                svst1_f64(pg0, &wv[i + off0], sv_wv0);
                svst1_f64(pg1, &wv[i + off1], sv_wv1);
                svst1_f64(pg2, &wv[i + off2], sv_wv2);
                svst1_f64(pg3, &wv[i + off3], sv_wv3);
            }
#else  // ! __ARM_FEATURE_SVE
        for (i = first; i < last; i++)
        {
            wv[i] = xv[i] + beta * yv[i];
        }
#endif // __ARM_FEATURE_SVE
        }
        else if (beta == 1.0)
        {
#if defined(__ARM_FEATURE_SVE) && !defined(INDEX_64) // INDEX_64 in Geometry.hpp
            svfloat64_t sv_alpha = svdup_f64(alpha);
            local_int_t off0 = svcntd() * 0;
            local_int_t off1 = svcntd() * 1;
            local_int_t off2 = svcntd() * 2;
            local_int_t off3 = svcntd() * 3;

            for (i = first; i < last; i += stride)
            {
                // Generate predicates
                svbool_t pg0 = svwhilelt_b64(i + off0, last);
                svbool_t pg1 = svwhilelt_b64(i + off1, last);
                svbool_t pg2 = svwhilelt_b64(i + off2, last);
                svbool_t pg3 = svwhilelt_b64(i + off3, last);

                // Load xv values
                svfloat64_t sv_xv0 = svld1_f64(pg0, &xv[i + off0]);
                svfloat64_t sv_xv1 = svld1_f64(pg1, &xv[i + off1]);
                svfloat64_t sv_xv2 = svld1_f64(pg2, &xv[i + off2]);
                svfloat64_t sv_xv3 = svld1_f64(pg3, &xv[i + off3]);

                // Load yv values
                svfloat64_t sv_yv0 = svld1_f64(pg0, &yv[i + off0]);
                svfloat64_t sv_yv1 = svld1_f64(pg1, &yv[i + off1]);
                svfloat64_t sv_yv2 = svld1_f64(pg2, &yv[i + off2]);
                svfloat64_t sv_yv3 = svld1_f64(pg3, &yv[i + off3]);

                // Compute result
                svfloat64_t sv_wv0 = svmla_f64_z(pg0, sv_yv0, sv_alpha, sv_xv0);
                svfloat64_t sv_wv1 = svmla_f64_z(pg1, sv_yv1, sv_alpha, sv_xv1);
                svfloat64_t sv_wv2 = svmla_f64_z(pg2, sv_yv2, sv_alpha, sv_xv2);
                svfloat64_t sv_wv3 = svmla_f64_z(pg3, sv_yv3, sv_alpha, sv_xv3);

                // Store
                svst1_f64(pg0, &wv[i + off0], sv_wv0);
                svst1_f64(pg1, &wv[i + off1], sv_wv1);
                svst1_f64(pg2, &wv[i + off2], sv_wv2);
                svst1_f64(pg3, &wv[i + off3], sv_wv3);
            }
#else  // ! __ARM_FEATURE_SVE
        for (i = first; i < last; i++)
        {
            wv[i] = alpha * xv[i] + yv[i];
        }
#endif // __ARM_FEATURE_SVE
        }
        else
        {
#if defined(__ARM_FEATURE_SVE) && !defined(INDEX_64) // INDEX_64 in Geometry.hpp
            svfloat64_t sv_alpha = svdup_f64(alpha);
            svfloat64_t sv_beta = svdup_f64(beta);
            local_int_t off0 = svcntd() * 0;
            local_int_t off1 = svcntd() * 1;
            local_int_t off2 = svcntd() * 2;
            local_int_t off3 = svcntd() * 3;
            for (i = first; i < last; i += svcntd())
            {
                // Generate predicates
                svbool_t pg0 = svwhilelt_b64(i + off0, last);
                svbool_t pg1 = svwhilelt_b64(i + off1, last);
                svbool_t pg2 = svwhilelt_b64(i + off2, last);
                svbool_t pg3 = svwhilelt_b64(i + off3, last);

                // Load xv values
                svfloat64_t sv_xv0 = svld1_f64(pg0, &xv[i + off0]);
                svfloat64_t sv_xv1 = svld1_f64(pg1, &xv[i + off1]);
                svfloat64_t sv_xv2 = svld1_f64(pg2, &xv[i + off2]);
                svfloat64_t sv_xv3 = svld1_f64(pg3, &xv[i + off3]);

                // Load yv values
                svfloat64_t sv_yv0 = svld1_f64(pg0, &yv[i + off0]);
                svfloat64_t sv_yv1 = svld1_f64(pg1, &yv[i + off1]);
                svfloat64_t sv_yv2 = svld1_f64(pg2, &yv[i + off2]);
                svfloat64_t sv_yv3 = svld1_f64(pg3, &yv[i + off3]);

                // Compute xva = xv * alpha
                svfloat64_t sv_xva0 = svmul_f64_z(pg0, sv_xv0, sv_alpha);
                svfloat64_t sv_xva1 = svmul_f64_z(pg1, sv_xv1, sv_alpha);
                svfloat64_t sv_xva2 = svmul_f64_z(pg2, sv_xv2, sv_alpha);
                svfloat64_t sv_xva3 = svmul_f64_z(pg3, sv_xv3, sv_alpha);

                // Compute xva + yv * beta
                svfloat64_t sv_wv0 = svmla_f64_z(pg0, sv_xva0, sv_beta, sv_yv0);
                svfloat64_t sv_wv1 = svmla_f64_z(pg1, sv_xva1, sv_beta, sv_yv1);
                svfloat64_t sv_wv2 = svmla_f64_z(pg2, sv_xva2, sv_beta, sv_yv2);
                svfloat64_t sv_wv3 = svmla_f64_z(pg3, sv_xva3, sv_beta, sv_yv3);

                // Store
                svst1_f64(pg0, &wv[i + off0], sv_wv0);
                svst1_f64(pg1, &wv[i + off1], sv_wv1);
                svst1_f64(pg2, &wv[i + off2], sv_wv2);
                svst1_f64(pg3, &wv[i + off3], sv_wv3);
            }
#else  // ! __ARM_FEATURE_SVE
        for (i = first; i < last; i++)
        {
            wv[i] = alpha * xv[i] + beta * yv[i];
        }
#endif // __ARM_FEATURE_SVE
        }
#ifndef HPCG_NO_OPENMP
    } // omp parallel
#endif // HPCG_NO_OPENMP

    return 0;
}

//////////////////////// CG Support Kernels: SYMG /////////////////////////////
/*
    Multiplies x values with d and accumultaes back to x
*/
void SpmvDiagCpu(local_int_t n, const double* x, double* y, double* z)
{
#if defined(__ARM_FEATURE_SVE) && !defined(INDEX_64) // INDEX_64 in Geometry.hpp
#pragma omp parallel for
    for (local_int_t i = 0; i < n; i += svcntd())
    {
        svbool_t pg = svwhilelt_b64(i, n);
        svfloat64_t xv = svld1_f64(pg, &x[i]);
        svfloat64_t yv = svld1_f64(pg, &y[i]);
        svst1_f64(pg, &z[i], svmul_f64_z(pg, xv, yv));
    }
#else
#pragma omp parallel for
    for (local_int_t i = 0; i < n; i++)
    {
        z[i] = x[i] * y[i];
    }
#endif
}

/*
     Computes z = x - r
*/
void AxpbyCpu(local_int_t n, double* x, double* y, double* z)
{
#if defined(__ARM_FEATURE_SVE) && !defined(INDEX_64) // INDEX_64 in Geometry.hpp
#pragma omp parallel for
    for (local_int_t i = 0; i < n; i += svcntd())
    {
        svbool_t pg = svwhilelt_b64(i, n);
        svfloat64_t xv = svld1_f64(pg, &x[i]);
        svfloat64_t yv = svld1_f64(pg, &y[i]);
        svst1_f64(pg, &z[i], svsub_f64_z(pg, xv, yv));
    }
#else
#pragma omp parallel for
    for (local_int_t i = 0; i < n; i++)
    {
        z[i] = x[i] - y[i];
    }
#endif
}

/*
    Computes z += x * y
*/
void SpFmaCpu(local_int_t n, const double* x, double* y, double* z)
{
#if defined(__ARM_FEATURE_SVE) && !defined(INDEX_64) // INDEX_64 in Geometry.hpp
#pragma omp parallel for
    for (local_int_t i = 0; i < n; i += svcntd())
    {
        svbool_t pg = svwhilelt_b64(i, n);
        svfloat64_t xv = svld1_f64(pg, &x[i]);
        svfloat64_t yv = svld1_f64(pg, &y[i]);
        svfloat64_t zv = svld1_f64(pg, &z[i]);
        svst1_f64(pg, &z[i], svmad_f64_z(pg, xv, yv, zv));
    }
#else
#pragma omp parallel for
    for (local_int_t i = 0; i < n; i++)
    {
        z[i] += x[i] * y[i];
    }
#endif
}

/*
    SpMV for external matrix
*/
void ExtSpMVCpu(const SparseMatrix& A, const local_int_t n, const double alpha, const double* x, double* y)
{
#pragma omp parallel for
    for (local_int_t i = 0; i < n; i++)
    {
        double sum = 0.0;
        local_int_t first = A.csrExtOffsets[i];
        local_int_t last = A.csrExtOffsets[i + 1];
        for (local_int_t j = first; j < last; j++)
        {
            local_int_t col = A.csrExtColumns[j];
            double val = A.csrExtValues[j];
            sum += val * x[col];
        }
        y[i] += alpha * sum;
    }
}
#endif // USE_GRACE