
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
 @file ComputeProlongation_ref.cpp

 HPCG routine
 */

#ifndef HPCG_NO_OPENMP
#include <omp.h>
#endif

#include "idx.hpp"

#include "ComputeProlongation_ref.hpp"
#include <iostream>

/*!
  Routine to compute the coarse residual vector.

  @param[in]  Af - Fine grid sparse matrix object containing pointers to current coarse grid correction and the f2c operator.
  @param[inout] xf - Fine grid solution vector, update with coarse grid correction.

  Note that the fine grid residual is never explicitly constructed.
  We only compute it for the fine grid points that will be injected into corresponding coarse grid points.

  @return Returns zero on success and a non-zero value otherwise.
*/
int ComputeProlongation_ref(const SparseMatrix & Af, Vector & xf) {

  double * xfv = xf.values;
  double * xcv = Af.mgData->xc->values;

  const bool MATRIX_FREE = false;

  if(MATRIX_FREE) {
    local_int_t nxc = Af.geom->nx/2;
    local_int_t nyc = Af.geom->ny/2;
    local_int_t nzc = Af.geom->nz/2;

#ifndef HPCG_NO_OPENMP
#pragma omp parallel for collapse(3)
#endif
    for(local_int_t izc=0; izc < nzc; ++izc) {
      for(local_int_t iyc=0; iyc < nyc; ++iyc) {
        for(local_int_t ixc=0; ixc < nxc; ++ixc) {
          xfv[idx(2*ixc,2*iyc,2*izc,2*nxc,2*nyc,2*nzc)] += xcv[idx(ixc,iyc,izc,nxc,nyc,nzc)];
        }
      }
    }
  } else {
    local_int_t * f2c = Af.mgData->f2cOperator;
    local_int_t nc = Af.mgData->rc->localLength;
#ifndef HPCG_NO_OPENMP
#pragma omp parallel for
#endif
    for (local_int_t i=0; i<nc; ++i) xfv[f2c[i]] += xcv[i]; // This loop is safe to vectorize
  }

  return 0;
}
