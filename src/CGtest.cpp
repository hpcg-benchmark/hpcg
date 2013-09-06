
//@HEADER
// ************************************************************************
//
//               HPCG: Simple Conjugate Gradient Benchmark Code
// Questions? Contact Michael A. Heroux (maherou@sandia.gov)
//
// ************************************************************************
//@HEADER

// Changelog
//
// Version 0.3
// - Added timing of setup time for sparse MV
// - Corrected percentages reported for sparse MV with overhead
//
/////////////////////////////////////////////////////////////////////////

// Main routine of a program that calls the HPCG conjugate gradient
// solver to solve the problem, and then prints results.

#include <fstream>
#include <iostream>
using std::endl;
#ifdef DEBUG
using std::cin;
#endif
#include <cstdlib>
#include <vector>
#ifndef HPCG_NOMPI
#include <mpi.h> // If this routine is not compiled with HPCG_NOMPI
#endif

#ifndef HPCG_NOOPENMP
#include <omp.h> // If this routine is not compiled with HPCG_NOOPENMP
#endif

#include "hpcg.hpp"

#include "GenerateGeometry.hpp"
#include "GenerateProblem.hpp"
#include "SetupHalo.hpp"
#include "OptimizeProblem.hpp"
#include "mytimer.hpp"
#include "ComputeResidual.hpp"
#include "CG.hpp"
#include "CGtest.hpp"
#include "Geometry.hpp"
#include "SparseMatrix.hpp"
#include "CGData.hpp"

int CGtest(HPCG_Params * params, Geometry & geom, int size, int rank, CGtestData * cgtest_data) {


  int numThreads = 1;

#ifndef HPCG_NOOPENMP
#pragma omp parallel
  numThreads = omp_get_num_threads();
#endif

    local_int_t nx,ny,nz;
    nx = (local_int_t)params->nx;
    ny = (local_int_t)params->ny;
    nz = (local_int_t)params->nz;

    SparseMatrix A;
    CGData data;
    double *x, *b, *xexact;
    GenerateProblem(geom, A, &x, &b, &xexact);
    SetupHalo(geom, A);
    initializeCGData(A, data);

#ifdef HPCG_DEBUG
    if (rank==0) HPCG_fout << "Total setup time (sec) = " << mytimer() - t1 << endl;
#endif


    // Use this array for collecting timing information
    std::vector< double > times(8,0.0);
    double t7 = 0.0;

    // Call user-tunable set up function.
    t7 = mytimer(); OptimizeProblem(geom, A, data, x, b, xexact); t7 = mytimer() - t7;
    times[7] = t7;

    // Modify the matrix diagonal to greatly exaggerate diagonal values
    for (int i=0; i< A.localNumberOfRows; ++i) {
      global_int_t globalRowID = A.localToGlobalMap[i];
      if (globalRowID<9) {
        *(A.matrixDiagonal[i]) *= (globalRowID+2)*1.0e6; // Multiply the first 9 diagonal values by RowID+2 times 1M.
        b[i] *= (globalRowID+1)*1.0e6;
      } else {
        *(A.matrixDiagonal[i]) *= 1.0e6; // The rest are multiplied by 1M.
        b[i] *= 1.0e6;
      }
    }
    double t1 = mytimer();   // Initialize it (if needed)
    int niters = 0;
    double normr = 0.0;
    double normr0 = 0.0;
    int maxIters = 100;
    int numberOfCgCalls = 10;
    int err_count = 0;
    double tolerance = 1.0e-12; // Set tolerance to reasonable value for grossly scaled diagonal terms
    for (int k=0; k<2; ++k) { // This loop tests both unpreconditioned and preconditioned runs
      int expected_niters = 1;
      if (k==0) expected_niters = 11;
      for (int i=0; i< numberOfCgCalls; ++i) {
        for (int j=0; j< A.localNumberOfRows; ++j) x[j] = 0.0; // Zero out x
        int ierr = CG( geom, A, data, b, x, maxIters, tolerance, niters, normr, normr0, &times[0], k==1);
        if (ierr) HPCG_fout << "Error in call to CG: " << ierr << ".\n" << endl;
        if (niters <= expected_niters) {
          ++cgtest_data->count_pass;
        } else {
          ++cgtest_data->count_fail;
        }
        if (rank==0) {
          HPCG_fout << "Call [" << i << "] Number of Iterations [" << niters <<"] Scaled Residual [" << normr/normr0 << "]" << endl;
          if (niters > expected_niters)
            HPCG_fout << " Expected " << expected_niters << " iterations.  Performed " << niters << "." << endl;
        }
      }
    }

	// Compute difference between known exact solution and computed solution
	// All processors are needed here.
#ifdef DEBUG
	double residual = 0;
	int ierr = ComputeResidual(A.localNumberOfRows, x, xexact, &residual);
	if (ierr && 0 == rank) HPCG_fout << "Error in call to compute_residual: " << ierr << ".\n" << endl;
	if (rank == 0)
		HPCG_fout << "Difference between computed and exact  = " << residual << ".\n" << endl;
#endif

        cgtest_data->niters = niters;
        cgtest_data->normr = normr;

	// Clean up
	destroyMatrix(A);
	destroyCGData(data);
	delete [] x;
	delete [] b;
	delete [] xexact;

	return 0;
}
