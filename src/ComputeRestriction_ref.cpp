
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

/*!
 @file ComputeRestriction_ref.cpp

 HPCG routine
 */


#ifndef HPCG_NO_OPENMP
#include <omp.h>
#endif
#include <iostream>
#include "idx.hpp"
#include "ComputeRestriction_ref.hpp"

/*!
  Routine to compute the coarse residual vector.

  @param[inout]  A - Sparse matrix object containing pointers to mgData->Axf, the fine grid matrix-vector product and mgData->rc the coarse residual vector.
  @param[in]    rf - Fine grid RHS.


  Note that the fine grid residual is never explicitly constructed.
  We only compute it for the fine grid points that will be injected into corresponding coarse grid points.

  @return Returns zero on success and a non-zero value otherwise.
*/
int ComputeRestriction_ref(const SparseMatrix & A, const Vector & rf) {

  double * Axfv = A.mgData->Axf->values;
  double * rfv = rf.values;
  double * rcv = A.mgData->rc->values;

  const bool MATRIX_FREE = false;

  if(MATRIX_FREE) {
    local_int_t nxc = A.geom->nx/2;
    local_int_t nyc = A.geom->ny/2;
    local_int_t nzc = A.geom->nz/2;

#ifndef HPCG_NO_OPENMP
#pragma omp parallel for collapse(3)
#endif
    for(local_int_t izc=0; izc < nzc; ++izc){
      for(local_int_t iyc=0; iyc < nyc; ++iyc){
        for(local_int_t ixc=0; ixc < nxc; ++ixc){
          rcv[idx(ixc,iyc,izc,nxc,nyc,nzc)] = rfv[idx(2*ixc,2*iyc,2*izc,2*nxc,2*nyc,2*nzc)] - Axfv[idx(2*ixc,2*iyc,2*izc,2*nxc,2*nyc,2*nzc)];
        }
      }
    }
  } else {
    local_int_t * f2c = A.mgData->f2cOperator;
    local_int_t nc = A.mgData->rc->localLength;
#ifndef HPCG_NO_OPENMP
#pragma omp parallel for
#endif
    for (local_int_t i=0; i<nc; ++i) rcv[i] = rfv[f2c[i]] - Axfv[f2c[i]];
  }

  return 0;
}
