

//@HEADER
// ***************************************************
//
// HPCG: High Performance Conjugate Gradient Benchmark
//
// Author: SZW
//
// Contact:
// Michael A. Heroux ( maherou@sandia.gov)
// Jack Dongarra     (dongarra@eecs.utk.edu)
// Piotr Luszczek    (luszczek@eecs.utk.edu)
//
// ***************************************************
//@HEADER

/*!
 @file ComputeSPMV_new.cpp

 HPCG routine
 */
#include <mpi.h>
#include <immintrin.h>
#include "ComputeSPMV.hpp"

#include <ctime>
#ifndef HPCG_NO_MPI
#include "ExchangeHalo.hpp"
#endif

#ifndef HPCG_NO_OPENMP
#include <omp.h>
#endif
#include <cassert>

#include <iostream>
using namespace std;
#include "mytimer.hpp"
inline double _mm256_reduce_add_pd(__m256d vec) {
    __m128d hi = _mm256_extractf128_pd(vec, 1);
    __m128d lo = _mm256_castpd256_pd128(vec);
    __m128d sum = _mm_add_pd(lo, hi);
    sum = _mm_hadd_pd(sum, sum);
    return _mm_cvtsd_f64(sum);
}


/*!
  Routine to compute matrix vector product y = Ax where:
  Precondition: First call exchange_externals to get off-processor values of x

  This is the reference SPMV implementation.  It CANNOT be modified for the
  purposes of this benchmark.

  @param[in]  A the known system matrix
  @param[in]  x the known vector
  @param[out] y the On exit contains the result: Ax.

  @return returns 0 upon success and non-zero otherwise

  @see ComputeSPMV
*/
int ComputeSPMV(const SparseMatrix &A, Vector &x, Vector &y) {
    assert(x.localLength >= A.localNumberOfColumns); // Test vector lengths
    assert(y.localLength >= A.localNumberOfRows);

#ifndef HPCG_NO_MPI
    ExchangeHalo(A, x);
#endif
    const double * const xv = x.values;
    double * const yv = y.values;
    const local_int_t nrow = A.localNumberOfRows;

#ifndef HPCG_NO_OPENMP
    #pragma omp parallel for schedule(static)
#endif
    for (local_int_t i = 0; i < nrow; i++) {
        double sum = 0.0;
        const double * const cur_vals = A.matrixValues[i];
        const local_int_t * const cur_inds = A.mtxIndL[i];
        const int cur_nnz = A.nonzerosInRow[i];
        
        int j;
        __m256d vec_sum = _mm256_setzero_pd();
        for (j = 0; j <= cur_nnz - 4; j += 4) {
            __m256d vals = _mm256_loadu_pd(&cur_vals[j]);
            __m256d x_vals = _mm256_set_pd(
              xv[cur_inds[j+3]], 
              xv[cur_inds[j+2]], 
              xv[cur_inds[j+1]], 
              xv[cur_inds[j]]);
            vec_sum = _mm256_add_pd(vec_sum, _mm256_mul_pd(vals, x_vals));
        }
        sum += _mm256_reduce_add_pd(vec_sum);
        for (; j < cur_nnz; j++) {
            sum += cur_vals[j] * xv[cur_inds[j]];
        }
        yv[i] = sum;
    }
    return 0;
}

// int ComputeSPMV_new(const SparseMatrix &A, Vector &x, Vector &y) {
//     assert(x.localLength >= A.localNumberOfColumns);
//     assert(y.localLength >= A.localNumberOfRows);

// #ifndef HPCG_NO_MPI
//     ExchangeHalo(A,x);
// #endif
//     const double * const xv = x.values;
//     double * const yv = y.values;
//     const local_int_t nrow = A.localNumberOfRows;

//     // for(local_int_t i = 0; i<y.localLength; i++)
//     //   yv[i] = 0.0;
//     //double t1 = mytimer();
//     sparse_matrix_t csrMatrix;
//     sparse_status_t status =  mkl_sparse_d_create_csr(&csrMatrix, SPARSE_INDEX_BASE_ZERO, A.localNumberOfRows, A.localNumberOfColumns, A.csrRowOffsets, A.csrRowOffsets+1, A.csrColumnIndices, A.csrValues);
//     //double t2 = mytimer();
//     if (status != SPARSE_STATUS_SUCCESS)
//       printf("Error creating sparse matrix\n");

//     struct matrix_descr descr;
//     descr.type = SPARSE_MATRIX_TYPE_GENERAL;

//     // 执行稀疏矩阵向量乘法
//     mkl_sparse_d_mv(SPARSE_OPERATION_NON_TRANSPOSE, 1.0, csrMatrix, descr, xv, 0.0, yv);
//     //double t3 = mytimer();
//     status = mkl_sparse_destroy(csrMatrix);
//     if (status != SPARSE_STATUS_SUCCESS)
//       printf("Error destroying sparse matrix\n");
//     // double t4 = mytimer();
//     // for (local_int_t i=0; i< nrow; i++)  {
//     //   double sum = 0.0;
//     //   const double * const cur_vals = A.matrixValues[i];
//     //   const local_int_t * const cur_inds = A.mtxIndL[i];
//     //   const int cur_nnz = A.nonzerosInRow[i];

//     //   for (int j=0; j< cur_nnz; j++)
//     //     sum += cur_vals[j]*xv[cur_inds[j]];
//     //   yv[i] = sum;
//     // }
//     //double t5 = mytimer();
//     // printf("T1:%f,T2:%f,T3:%f,T4:%f\n",t2-t1,t3-t2,t4-t3,t5-t4);
//     // for (local_int_t i=0; i< nrow; i++)  {
//     //   double sum = 0.0;
//     //   const double * const cur_vals = A.matrixValues[i];
//     //   const local_int_t * const cur_inds = A.mtxIndL[i];
//     //   const int cur_nnz = A.nonzerosInRow[i];

//     //   for (int j=0; j< cur_nnz; j++)
//     //     sum += cur_vals[j]*xv[cur_inds[j]];
//     //   if(abs(sum - yv[i]) > 1e-6)
//     //   {
//     //     printf("Rank:%d,sum=%f,yv=%f,Column1=%d,Column2=%d\n",rank,sum,yv[i],cur_nnz,A.csrRowOffsets[i+1]-A.csrRowOffsets[i]);
//     //     for (int j=0; j< cur_nnz; j++)
//     //     {
//     //       if(cur_inds[j] != A.csrColumnIndices[A.csrRowOffsets[i]+j] || cur_vals[j] != A.csrValues[A.csrRowOffsets[i]+j] || (cur_vals[j]*xv[cur_inds[j]]) != (A.csrValues[A.csrRowOffsets[i]+j]*xv[A.csrColumnIndices[A.csrRowOffsets[i]+j]]))
//     //         printf("Row:%d,Index:%d,Column1:%d,Column2:%d,Value1:%f,Value2:%f,R1=%f,R2=%f\n",i,j,cur_inds[j],A.csrColumnIndices[A.csrRowOffsets[i]+j],cur_vals[j],A.csrValues[A.csrRowOffsets[i]+j],(cur_vals[j]*xv[cur_inds[j]]),(A.csrValues[A.csrRowOffsets[i]+j]*xv[A.csrColumnIndices[A.csrRowOffsets[i]+j]]));
//     //     }

//     //   }
//     // }

//     return 0; // Success
// }
