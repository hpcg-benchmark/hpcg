
//@HEADER
// ************************************************************************
//
//               HPCG: Simple Conjugate Gradient Benchmark Code
// Questions? Contact Michael A. Heroux (maherou@sandia.gov)
//
// ************************************************************************
//@HEADER

/*!
 @file waxpby.cpp

 HPCG routine
 */

#include "waxpby.hpp"
#include "waxpbyref.hpp"

/*!
  Routine to compute the update of a vector with the sum of two
  scaled vectors where: w = alpha*x + beta*y

  This routine calls the reference WAXPBY implementation by default, but
  can be replaced by a custom, optimized routine suited for
  the target system.

  @param[in] n the number of vector elements (on this processor)
  @param[in] alpha, beta the scalars applied to x and y respectively.
  @param[in] x, y the input vectors
  @param[out] w the output vector

  @see waxpbyref
*/
int waxpby (const local_int_t n, const double alpha, const double * const x,
	    const double beta, const double * const y,
		     double * const w) {
  return(waxpbyref(n, alpha, x, beta, y, w));
}
