
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


#include "ComputeProlongation_ref.hpp"

#ifndef HPCG_NOMPI
#include <mpi.h> // If this routine is not compiled with HPCG_NOMPI
#endif

#ifndef HPCG_NOOPENMP
#include <omp.h> // If this routine is not compiled with HPCG_NOOPENMP
#endif

/*!
  Routine to compute the coarse residual vector.

  @param[inout]  A - Sparse matrix object containing pointers to A*y from the ComputeMG_ref function
  @param[in] x - RHS for preconditioner

  Note that the fine grid residual is never explicitly constructed.
  We only compute it for the fine grid points that will be injected into corresponding coarse grid points.

  @return Returns zero on success and a non-zero value otherwise.
*/
int ComputeRestriction_ref(const SparseMatrix & A, const Vector & x) {

  double * Axfv = A.mgData->Axf->values;
  double * xv = x.values;
  double * xcv = A.mgData->xc->values;
  local_int_t * f2c = A.mgData->f2cOperator;
  local_int_t nc = A.mgData->rc->localLength;

#ifndef HPCG_NOOPENMP
#pragma omp parallel for
#endif
#pragma vector
  for (local_int_t i=0; i<nc; ++i) xv[f2c[i]] = xv[f2c[i]] - xcv[i]; // This loop is safe to vectorize

  return(0);
}
