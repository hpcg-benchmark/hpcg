
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

#include "hpcg.hpp"

#include "CGtest.hpp"
#include "CG.hpp"
#include "Geometry.hpp"
#include "SparseMatrix.hpp"

int CGtest(Geometry & geom, SparseMatrix & A, CGData & data, double * const b, double * const x, CGtestData * cgtest_data) {


    // Use this array for collecting timing information
    std::vector< double > times(8,0.0);
    // Temporary storage for holding original diagonal and RHS
    std::vector< double > diagA(A.localNumberOfRows), origB(A.localNumberOfRows);

    // Modify the matrix diagonal to greatly exaggerate diagonal values.
    // CG should converge in about 10 iterations for this problem, regardless of problem size
    for (int i=0; i< A.localNumberOfRows; ++i) {
      global_int_t globalRowID = A.localToGlobalMap[i];
      double * curDiagA = A.matrixDiagonal[i];
      origB[i] = b[i]; // Save original RHS value
      diagA[i] = *curDiagA; // Save original diagonal value
      if (globalRowID<9) {
        *curDiagA *= (globalRowID+2)*1.0e6; // Multiply the first 9 diagonal values by RowID+2 times 1M.
        b[i] *= (globalRowID+1)*1.0e6;
      } else {
        *(A.matrixDiagonal[i]) *= 1.0e6; // The rest are multiplied by 1M.
        b[i] *= 1.0e6;
      }
    }
    int niters = 0;
    double normr = 0.0;
    double normr0 = 0.0;
    int maxIters = 50;
    int numberOfCgCalls = 2;
    int err_count = 0;
    double tolerance = 1.0e-12; // Set tolerance to reasonable value for grossly scaled diagonal terms
    for (int k=0; k<2; ++k) { // This loop tests both unpreconditioned and preconditioned runs
      int expected_niters = 11; // For the unpreconditioned CG call, we should take about 10 iterations
      if (k==1) expected_niters = 1;  // For the preconditioned case, we should take about 1 iteration
      for (int i=0; i< numberOfCgCalls; ++i) {
        for (int j=0; j< A.localNumberOfRows; ++j) x[j] = 0.0; // Zero out x
        int ierr = CG( geom, A, data, b, x, maxIters, tolerance, niters, normr, normr0, &times[0], k==1);
        if (ierr) HPCG_fout << "Error in call to CG: " << ierr << ".\n" << endl;
        if (niters <= expected_niters) {
          ++cgtest_data->count_pass;
        } else {
          ++cgtest_data->count_fail;
        }
        if (geom.rank==0) {
          HPCG_fout << "Call [" << i << "] Number of Iterations [" << niters <<"] Scaled Residual [" << normr/normr0 << "]" << endl;
          if (niters > expected_niters)
            HPCG_fout << " Expected " << expected_niters << " iterations.  Performed " << niters << "." << endl;
        }
      }
    }

    // Restore matrix diagonal and RHS
    for (int i=0; i< A.localNumberOfRows; ++i) {
      *(A.matrixDiagonal[i]) = diagA[i];
      b[i] = origB[i];
    }
        cgtest_data->niters = niters;
        cgtest_data->normr = normr;

	return 0;
}
