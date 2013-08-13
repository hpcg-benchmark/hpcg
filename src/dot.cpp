
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

/////////////////////////////////////////////////////////////////////////

#ifdef USING_MPI
#include <mpi.h>
#include "mytimer.hpp"
#endif
#include "dot.hpp"

int dot (const local_int_t n, const double * const x, const double * const y,
	  double * const result, double & time_allreduce) {  
  double local_result = 0.0;
  if (y==x)
    for (local_int_t i=0; i<n; i++) local_result += x[i]*x[i];
  else
    for (local_int_t i=0; i<n; i++) local_result += x[i]*y[i];

#ifdef USING_MPI
  // Use MPI's reduce function to collect all partial sums
  double t0 = mytimer();
  double global_result = 0.0;
  MPI_Allreduce(&local_result, &global_result, 1, MPI_DOUBLE, MPI_SUM, 
                MPI_COMM_WORLD);
  *result = global_result;
  time_allreduce += mytimer() - t0;
#else
  *result = local_result;
#endif

  return(0);
}
