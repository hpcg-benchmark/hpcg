
//@HEADER
// ************************************************************************
// 
//               HPCG: Simple Conjugate Gradient Benchmark Code
// Questions? Contact Michael A. Heroux (maherou@sandia.gov) 
// 
// ************************************************************************
//@HEADER

/*!
 @file spmv.cpp

 HPCG routine
 */

#include "spmv.hpp"
#include "spmvref.hpp"

/*!
  Routine to compute matrix vector product y = Ax where:
  Precondition: First call exchange_externals to get off-processor values of x

  This routine calls the reference spmv implementation by default, but
  can be replaced by a custom, optimized routine suited for
  the target system.

  @param[in]  A the known system matrix 
  @param[in]  x the known vector
  @param[out] y the On exit contains the result: Ax.

  @see spmvref
*/
int spmv( const SparseMatrix & A, const double * const x, double * const y) {

	return(spmvref(A, x, y));

}
