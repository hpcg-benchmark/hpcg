
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
  local_int_t * f2c = Af.mgData->f2cOperator;
  local_int_t nc = Af.mgData->rc->localLength;

  local_int_t ix = 0;
  local_int_t iy = 0;
  local_int_t iz = 0;
//  local_int_t nex = 0;
//  local_int_t ney = 0;
//  local_int_t nez = 0;
  local_int_t nx = Af.geom->nx;
  local_int_t ny = Af.geom->ny;
  local_int_t nz = Af.geom->nz;
  local_int_t nlocal = nx*ny*nz;
/*  int npx          = Af.geom->npx;
  int npy          = Af.geom->npy;
  int npz          = Af.geom->npz;
  int ipx          = Af.geom->ipx;
  int ipy          = Af.geom->ipy;
  int ipz          = Af.geom->ipz;*/

#ifndef HPCG_NO_OPENMP
#pragma omp parallel for
#endif
// TODO: Somehow note that this loop can be safely vectorized since f2c has no repeated indices
/*  for (local_int_t i=0; i<nc; ++i){
        xfv[f2c[i]] += xcv[i]; // This loop is safe to vectorize
        std::cout << "restrict fine to course " << f2c[i] << " " << i << std::endl;
  }*/
for( iz=0; iz< nz; iz +=2){
    for( iy=0; iy< ny; iy +=2){
        for (ix=0; ix < nx; ix +=2){
            xfv[ix+iy*nx+iz*ny*nx] += xcv[ix/2+iy*nx/4+iz*ny*nx/8];
//        std::cout << "restrict fine to course " << ix+iy*nx+iz*ny*nx << " " << ix/2+iy*nx/4+iz*ny*nx/8 << std::endl;
        }
    }
}

  return 0;
}
