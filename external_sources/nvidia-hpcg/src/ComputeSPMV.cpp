
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
 @file ComputeSPMV.cpp

 HPCG routine
 */

#include "ComputeSPMV.hpp"
#include "ComputeSPMV_ref.hpp"
#ifndef HPCG_NO_MPI
#include "ExchangeHalo.hpp"
#endif
#ifdef USE_CUDA
#include "Cuda.hpp"
#include "CudaKernels.hpp"
#endif

#include "CpuKernels.hpp"
/*!
  Routine to compute sparse matrix vector product y = Ax where:
  Precondition: First call exchange_externals to get off-processor values of x

  This routine calls the reference SpMV implementation by default, but
  can be replaced by a custom, optimized routine suited for
  the target system.

  @param[in]  A the known system matrix
  @param[in]  x the known vector
  @param[out] y the On exit contains the result: Ax.

  @return returns 0 upon success and non-zero otherwise

  @see ComputeSPMV_ref
*/
int ComputeSPMV(const SparseMatrix& A, Vector& x, Vector& y)
{

    double one = 1.0, zero = 0.0;
    if (A.rankType == GPU)
    {
#ifdef USE_CUDA
#ifndef HPCG_NO_MPI
        PackSendBufferCuda(A, x, false, copy_stream);
#endif

        CHECK_CUSPARSE(cusparseDnVecSetValues(A.cusparseOpt.vecX, x.values_d));
        CHECK_CUSPARSE(cusparseDnVecSetValues(A.cusparseOpt.vecY, y.values_d));
        CHECK_CUSPARSE(cusparseSpMV(cusparsehandle, CUSPARSE_OPERATION_NON_TRANSPOSE, &one, A.cusparseOpt.matA,
            A.cusparseOpt.vecX, &zero, A.cusparseOpt.vecY, CUDA_R_64F, CUSPARSE_SPMV_ALG_DEFAULT, A.bufferMvA));

#ifndef HPCG_NO_MPI
        if (A.totalToBeSent > 0)
        {
            ExchangeHaloCuda(A, x, copy_stream);
            ExtSpMVCuda((SparseMatrix&) A, one, x.values_d + A.localNumberOfRows, y.values_d);
        }
#endif

        CHECK_CUDART(cudaStreamSynchronize(stream));
#endif
    }
    else
    {
#ifdef USE_GRACE
        nvpl_sparse_dn_vec_set_values(A.nvplSparseOpt.vecX, x.values);
        nvpl_sparse_dn_vec_set_values(A.nvplSparseOpt.vecY, y.values);
        nvpl_sparse_spmv(nvpl_sparse_handle, NVPL_SPARSE_OPERATION_NON_TRANSPOSE, &one, A.nvplSparseOpt.matA,
            A.nvplSparseOpt.vecX, &zero, A.nvplSparseOpt.vecY, A.nvplSparseOpt.vecY, NVPL_SPARSE_R_64F,
            NVPL_SPARSE_SPMV_ALG_DEFAULT, A.nvplSparseOpt.spmvADescr);

#ifndef HPCG_NO_MPI
        if (A.totalToBeSent > 0)
        {
            ExchangeHaloCpu(A, x);
            ExtSpMVCpu(A, A.localNumberOfRows, 1.0, x.values, y.values);
        }
#endif
#endif // USE_GRACE
    }
    return 0;
}
