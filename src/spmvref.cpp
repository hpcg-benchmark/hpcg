
//@HEADER
// ************************************************************************
// 
//               HPCG: Simple Conjugate Gradient Benchmark Code
// Questions? Contact Michael A. Heroux (maherou@sandia.gov) 
// 
// ************************************************************************
//@HEADER

/*!
 @file spmvref.cpp

 HPCG routine
 */

#include "spmvref.hpp"

#ifndef HPCG_NOOPENMP
#include <omp.h>
#endif

/*!
  Routine to compute matrix vector product y = Ax where:
  Precondition: First call exchange_externals to get off-processor values of x

  This is the reference SPMV implementation.  It CANNOT be modified for the
  purposes of this benchmark.

  @param[in]  A the known system matrix 
  @param[in]  x the known vector
  @param[out] y the On exit contains the result: Ax.

  @see spmv
*/
int spmvref( const SparseMatrix & A, const double * const x, double * const y) {

	const local_int_t nrow = A.localNumberOfRows;
#ifndef HPCG_NOOPENMP
#pragma omp parallel for
#endif
	for (local_int_t i=0; i< nrow; i++)  {
		double sum = 0.0;
		const double * const cur_vals = A.matrixValues[i];
		const local_int_t * const cur_inds = A.mtxIndL[i];
		const int cur_nnz = A.nonzerosInRow[i];

		for (int j=0; j< cur_nnz; j++)
			sum += cur_vals[j]*x[cur_inds[j]];
		y[i] = sum;
	}
	return(0);
}
