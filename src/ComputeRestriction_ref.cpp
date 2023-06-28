
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
  local_int_t * f2c = A.mgData->f2cOperator;
  local_int_t nc = A.mgData->rc->localLength;
  local_int_t ix = 0;
  local_int_t iy = 0;
  local_int_t iz = 0;
  local_int_t nx = A.geom->nx;
  local_int_t ny = A.geom->ny;
  local_int_t nz = A.geom->nz;
  local_int_t nlocal = nx*ny*nz;
#ifndef HPCG_NO_OPENMP
#pragma omp parallel for
#endif
for (ix=0; ix < nx; ix +=2){
    for( iy=0; iy< ny; iy +=2){
        for( iz=0; iz< nz; iz +=2){
            rcv[idx(ix/2,iy/2,iz/2,nx/2,ny/2,nz/2)] = rfv[idx(ix,iy,iz,nx,ny,nz)] - Axfv[idx(ix,iy,iz,nx,ny,nz)];
        }
    }
}
  return 0;
}
