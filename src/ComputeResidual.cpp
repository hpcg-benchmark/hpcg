
//@HEADER
// ************************************************************************
// 
//               HPCG: Simple Conjugate Gradient Benchmark Code
// Questions? Contact Michael A. Heroux (maherou@sandia.gov) 
// 
// ************************************************************************
//@HEADER

/////////////////////////////////////////////////////////////////////////

// Routine to compute the inf-norm difference between two vectors where:

// n - number of vector elements (on this processor)

// v1, v2 - input vectors

// residual - pointer to scalar value, on exit will contain result.

/////////////////////////////////////////////////////////////////////////
#include <cmath>  // needed for fabs
#include "ComputeResidual.hpp"
#ifdef DETAILEDDEBUG
#include <iostream>
#endif
using std::fabs;
#ifdef USING_MPI
#include <mpi.h> // If this routine is compiled with -DUSING_MPI then include mpi.h
#endif

int ComputeResidual(const int n, const double * const v1, 
                    const double * const v2, double * const residual) {

  double local_residual = 0.0;
  for (int i=0; i<n; i++) {
    double diff = fabs(v1[i] - v2[i]);
    if (diff > local_residual) local_residual = diff;
#ifdef DETAILEDDEBUG
    std::cout << " Computed, exact, diff = " << v1[i] << " " << v2[i] << " " << diff << std::endl;
#endif

  }

#ifdef USING_MPI
  // Use MPI's reduce function to collect all partial sums
  double global_residual = 0;
  MPI_Allreduce(&local_residual, &global_residual, 1, MPI_DOUBLE, MPI_MAX, MPI_COMM_WORLD);
  *residual = global_residual;
#else
  *residual = local_residual;
#endif

  return(0);
}
