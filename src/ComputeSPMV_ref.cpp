
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

#include "idx.hpp"
#include "ComputeSPMV_ref.hpp"

#ifndef HPCG_NO_MPI
#include "ExchangeHalo.hpp"
#endif

#ifndef HPCG_NO_OPENMP
#include <omp.h>
#endif
#include <cassert>
#include <iostream>
#include <cmath>

int ComputeSPMV_mf( const SparseMatrix & A, Vector & x, Vector & y) {
  const double * const xv = x.values;
  double * const yv = y.values;

  global_int_t nx = A.geom->nx;
  global_int_t ny = A.geom->ny;
  global_int_t nz = A.geom->nz;
  const int ng = 1;
  const local_int_t nrow_ghost = (nx+2*ng)*(ny+2*ng)*(nz+2*ng); // local number of rows *including* ghost points
  // TODO test performance of unique_ptr to replace this scary new
  double* xg = new double[nrow_ghost]{0.0}; // Copy of x with ghost points

  // Copy xv to xg TODO HACK find better way to do this
#ifndef HPCG_NO_OPENMP
  #pragma omp parallel for collapse(3)
#endif
  for (local_int_t iz=0; iz<nz; ++iz) {
    for (local_int_t iy=0; iy<ny; ++iy) {
      for (local_int_t ix=0; ix<nx; ++ix) {
        xg[idx(ix,iy,iz,nx,ny,nz,ng)] = xv[idx(ix,iy,iz,nx,ny,nz)];
      }
    }
  }


#ifndef HPCG_NO_OPENMP
  #pragma omp parallel for collapse(3)
#endif
  for (local_int_t iz=0; iz<nz; ++iz) {
    for (local_int_t iy=0; iy<ny; ++iy) {
      for (local_int_t ix=0; ix<nx; ++ix) {
        double sum = 0.0;
        const int i = idx(ix,iy,iz,nx,ny,nz);

        // FIXME THIS IS EXTREMELY HACKY!! We need a better way to get the stencil from A
        const double off_diagonal = A.matrixValues[1][0];
        const double diagonal = *A.matrixDiagonal[0];

        for(int sx=-1; sx<=1; ++sx) {
          for(int sy=-1; sy<=1; ++sy) {
            for(int sz=-1; sz<=1; ++sz) {
              bool is_diagonal = (sx==0 && sy==0 && sz==0);
              if(is_diagonal) {
                sum += diagonal*xg[idx(ix, iy, iz, nx, ny, nz, ng)];
              } else {
                sum += off_diagonal*xg[idx(ix+sx, iy+sy, iz+sz, nx, ny, nz, ng)];
              }
            }
          }
        }

        yv[i] = sum;
      }
    }
  }

  delete[] xg;

  return 0;
}

int ComputeSPMV_ma( const SparseMatrix & A, Vector & x, Vector & y) {
  const double * const xv = x.values;
  double * const yv = y.values;
  const local_int_t nrow = A.localNumberOfRows;

#ifndef HPCG_NO_OPENMP
#pragma omp parallel for
#endif
  for (local_int_t i=0; i< nrow; i++)  {
    double sum = 0.0;
    const double * const cur_vals = A.matrixValues[i];
    const local_int_t * const cur_inds = A.mtxIndL[i];
    const int cur_nnz = A.nonzerosInRow[i];

    for (int j=0; j< cur_nnz; j++)
      sum += cur_vals[j]*xv[cur_inds[j]];

    yv[i] = sum;
  }

  return 0;
}

namespace {

bool is_equal(const double f, const double g, const double abs_tol, const double rel_tol) {
  double error = std::abs(f - g);
  if (error < abs_tol) {
    return true;
  } else {
    return error < std::max(std::abs(f), std::abs(g))*rel_tol;
  }
}

void CompareComputeSPMV( const SparseMatrix & A, Vector & x, Vector & y) {

  const local_int_t nrow = A.localNumberOfRows;
  double * const yv = y.values;

  ComputeSPMV_ma(A,x,y);
  double * ma_result = new double[nrow];
  for(int i=0; i<nrow; ++i) {
    ma_result[i] = yv[i];
  }

  ComputeSPMV_mf(A,x,y);
  double * mf_result = new double[nrow];
  for(int i=0; i<nrow; ++i) {
    mf_result[i] = yv[i];
  }

  for(int i=0; i<nrow; ++i) {
    if(!is_equal(mf_result[i], ma_result[i], 1e-10, 1e-8)) {
      printf("ERROR: %d, ma = %.10f, mf = %.10f\n", i, ma_result[i], mf_result[i]);
      exit(-1);
    }
  }

  delete [] mf_result;
  delete [] ma_result;
}

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
    // TODO MPI is not implemented yet
    ExchangeHalo(A,x);
#endif

  //CompareComputeSPMV(A, x, y);

  const bool MATRIX_FREE = true;

  if(MATRIX_FREE) {
    ComputeSPMV_mf(A, x, y);
  } else {
    ComputeSPMV_ma(A, x, y);
  }

  return 0;
}
