
//@HEADER
// ************************************************************************
// 
//               HPCG: Simple Conjugate Gradient Benchmark Code
// Questions? Contact Michael A. Heroux (maherou@sandia.gov) 
// 
// ************************************************************************
//@HEADER

/*!
 @file dot.cpp

 HPCG routine
 */

#include "dot.hpp"
#include "dotref.hpp"

/*!
  Routine to compute the dot product of two vectors.

  This routine calls the reference DOT implementation by default, but can be
  replaced by a custom, optimized routine suited for the target system.

  @param[in]  n the number of vector elements (on this processor)
  @param[in]  x, y the input vectors
  @param[out] result a pointer to scalar value, on exit will contain the result.
  @param[out] time_allreduce the time it took to perform the communication between processes

  @see dotref
*/
int dot (const local_int_t n, const double * const x, const double * const y,
	  double * const result, double & time_allreduce) {  

  return(dotref(n, x, y, result, time_allreduce));
}
