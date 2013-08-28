
//@HEADER
// ************************************************************************
// 
//               HPCG: Simple Conjugate Gradient Benchmark Code
// Questions? Contact Michael A. Heroux (maherou@sandia.gov) 
// 
// ************************************************************************
//@HEADER

/////////////////////////////////////////////////////////////////////////

// Routine to compute the dot product of two vectors where:

// n - number of vector elements (on this processor)

// x, y - input vectors

// residual - pointer to scalar value, on exit will contain result.

// This routine calls the reference DOT implementation by default, but
// can be replaced by a custom, optimized routine suited for
// the target system.

/////////////////////////////////////////////////////////////////////////

#include "dot.hpp"
#include "dotref.hpp"

int dot (const local_int_t n, const double * const x, const double * const y,
	  double * const result, double & time_allreduce) {  

  return(dotref(n, x, y, result, time_allreduce));
}
