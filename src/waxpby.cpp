
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

/////////////////////////////////////////////////////////////////////////

#include "waxpby.hpp"

int waxpby (const int n, const double alpha, const double * const x, 
	    const double beta, const double * const y, 
		     double * const w)
{  
  if (alpha==1.0)
    for (int i=0; i<n; i++) w[i] = x[i] + beta * y[i];
  else if(beta==1.0)
    for (int i=0; i<n; i++) w[i] = alpha * x[i] + y[i];
  else 
    for (int i=0; i<n; i++) w[i] = alpha * x[i] + beta * y[i];

  return(0);
}
