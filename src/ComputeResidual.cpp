
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
#ifdef HPCG_DETAILEDDEBUG
#include <fstream>
#include "hpcg.hpp"
#endif

#include <cmath>  // needed for fabs
#include "ComputeResidual.hpp"
#ifdef HPCG_DETAILEDDEBUG
#include <iostream>
#endif

#ifndef HPCG_NOMPI
#include <mpi.h> // If this routine is not compiled with HPCG_NOMPI
#endif

#ifndef HPCG_NOOPENMP
#include <omp.h> // If this routine is not compiled with HPCG_NOOPENMP
#endif

int ComputeResidual(const local_int_t n, const double * const v1,
                    const double * const v2, double * const residual) {

  double local_residual = 0.0;

#ifndef HPCG_NOOPENMP
#pragma omp parallel default(none) shared(local_residual)
{
  double threadlocal_residual = 0.0;
#pragma omp for
  for (local_int_t i=0; i<n; i++) {
    double diff = std::fabs(v1[i] - v2[i]);
    if (diff > threadlocal_residual) threadlocal_residual = diff;
  }
#pragma omp critical
  {
   if (threadlocal_residual>local_residual) local_residual = threadlocal_residual;
  }
}
#else // No threading
  for (local_int_t i=0; i<n; i++) {
    double diff = std::fabs(v1[i] - v2[i]);
    if (diff > local_residual) local_residual = diff;
#ifdef HPCG_DETAILEDDEBUG
    HPCG_fout << " Computed, exact, diff = " << v1[i] << " " << v2[i] << " " << diff << std::endl;
#endif
  }
#endif

#ifndef HPCG_NOMPI
  // Use MPI's reduce function to collect all partial sums
  double global_residual = 0;
  MPI_Allreduce(&local_residual, &global_residual, 1, MPI_DOUBLE, MPI_MAX, MPI_COMM_WORLD);
  *residual = global_residual;
#else
  *residual = local_residual;
#endif

  return(0);
}
