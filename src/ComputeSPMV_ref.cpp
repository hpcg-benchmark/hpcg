
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
 @file ComputeSPMV_ref.cpp

 HPCG routine
 */

#include "ComputeSPMV_ref.hpp"

#ifndef HPCG_NO_MPI
#include "ExchangeHalo.hpp"
#endif

#ifndef HPCG_NO_OPENMP
#include <omp.h>
#endif
#include <cassert>

inline int idx(const int ix, const int iy, const int iz, const int nx, const int ny, const int nz, const int ng=0) {
  // input index of (0, 0, 0) corresponds to first *interior* point. Ghost points are accessed with (-1, -1, -1).
  return (iz+ng)*(nx+2*ng)*(ny+2*ng)+(iy+ng)*(nx+2*ng)+(ix+ng);
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
int ComputeSPMV_ref( const SparseMatrix & A, Vector & x, Vector & y) {

  assert(x.localLength>=A.localNumberOfColumns); // Test vector lengths
  assert(y.localLength>=A.localNumberOfRows);

#ifndef HPCG_NO_MPI
    ExchangeHalo(A,x);
#endif
  const double * const xv = x.values;
  double * const yv = y.values;
  const local_int_t nrow = A.localNumberOfRows;

  global_int_t nx = A.geom->nx;
  global_int_t ny = A.geom->ny;
  global_int_t nz = A.geom->nz;
  global_int_t ng = 1; // number of ghost points
  const local_int_t nrow_ghost = (nx+2*ng)*(ny+2*ng)*(nz+2*ng); // local number of rows *including* ghost points
  double* xg = new double[nrow_ghost]; // Copy of x with ghost points

  // Copy xv to xg
  for (local_int_t ix=0; ix<nx; ++ix) {
    for (local_int_t iy=0; iy<ny; ++iy) {
      for (local_int_t iz=0; iz<nz; ++iz) {
        xg[idx(ix,iy,iz,nx,ny,nz,ng)] = xv[idx(ix,iy,iz,nx,ny,nz)];
      }
    }
  }

#ifndef HPCG_NO_OPENMP
  #pragma omp parallel for
#endif
  for (local_int_t ix=0; ix<nx; ++ix) {
    for (local_int_t iy=0; iy<ny; ++iy) {
      for (local_int_t iz=0; iz<nz; ++iz) {
        double sum = 0.0;
        for(int sx=-1; sx<=1; ++sx) {
          for(int sy=-1; sy<=1; ++sy) {
            for(int sz=-1; sz<=1; ++sz) {
              sum += -1.*xg[idx(ix+sx, iy+sy, iz+sz, nx, ny, nz, ng)];
            }
          }
        }
        sum -= -1.*xg[idx(ix+0, iy+0, iz+0, nx, ny, nz, ng)];
        sum += 26.*xg[idx(ix+0, iy+0, iz+0, nx, ny, nz, ng)];
        yv[idx(ix,iy,iz,nx,ny,nz)] = sum;
      }
    }
  }

  return 0;
}
