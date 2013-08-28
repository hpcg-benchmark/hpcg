
//@HEADER
// ************************************************************************
//
//               HPCG: Simple Conjugate Gradient Benchmark Code
// Questions? Contact Michael A. Heroux (maherou@sandia.gov)
//
// ************************************************************************
//@HEADER

/////////////////////////////////////////////////////////////////////////

// Routine to compute the update of a vector with the sum of two
// scaled vectors where:

// w = alpha*x + beta*y
// n - number of vector elements (on this processor)
// x, y - input vectors
// alpha, beta - scalars applied to x and y respectively.
// w - output vector.

// This routine calls the reference WAXPBY implementation by default, but
// can be replaced by a custom, optimized routine suited for
// the target system.

/////////////////////////////////////////////////////////////////////////

#include "waxpby.hpp"
#include "waxpbyref.hpp"

int waxpby (const local_int_t n, const double alpha, const double * const x,
	    const double beta, const double * const y,
		     double * const w) {
  return(waxpbyref(n, alpha, x, beta, y, w));
}
