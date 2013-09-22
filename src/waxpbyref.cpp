
//@HEADER
// ************************************************************************
//
//               HPCG: Simple Conjugate Gradient Benchmark Code
// Questions? Contact Michael A. Heroux (maherou@sandia.gov)
//
// ************************************************************************
//@HEADER

/*!
 @file waxpbyref.cpp

 HPCG routine
 */

#include "waxpbyref.hpp"
#ifndef HPCG_NOOPENMP
#include <omp.h>
#endif

/*!
  Routine to compute the update of a vector with the sum of two
  scaled vectors where: w = alpha*x + beta*y

  This is the reference WAXPBY impmentation.  It CANNOT be modified for the
  purposes of this benchmark.

  @param[in] n the number of vector elements (on this processor)
  @param[in] alpha, beta the scalars applied to x and y respectively.
  @param[in] x, y the input vectors
  @param[out] w the output vector.

  @see waxpby
*/
int waxpbyref (const local_int_t n, const double alpha, const double * const x,
	    const double beta, const double * const y,
		     double * const w) {
  if (alpha==1.0) {
#ifndef HPCG_NOOPENMP
#pragma omp parallel for
#endif
    for (local_int_t i=0; i<n; i++) w[i] = x[i] + beta * y[i];
  }
  else if(beta==1.0) {
#ifndef HPCG_NOOPENMP
#pragma omp parallel for
#endif
    for (local_int_t i=0; i<n; i++) w[i] = alpha * x[i] + y[i];
  }
  else  {
#ifndef HPCG_NOOPENMP
#pragma omp parallel for
#endif
    for (local_int_t i=0; i<n; i++) w[i] = alpha * x[i] + beta * y[i];
  }

  return(0);
}
