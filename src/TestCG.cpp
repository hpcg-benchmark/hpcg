
//@HEADER
// ************************************************************************
//
//               HPCG: Simple Conjugate Gradient Benchmark Code
// Questions? Contact Michael A. Heroux (maherou@sandia.gov)
//
// ************************************************************************
//@HEADER

/*!
 @file TestCG.cpp

 HPCG routine
 */

// Changelog
//
// Version 0.4
// - Added timing of setup time for sparse MV
// - Corrected percentages reported for sparse MV with overhead
//
/////////////////////////////////////////////////////////////////////////

#include <fstream>
#include <iostream>
using std::endl;
#include <vector>
#ifndef HPCG_NOMPI
#include <mpi.h> // If this routine is not compiled with HPCG_NOMPI
#endif

#include "hpcg.hpp"

#include "TestCG.hpp"
#include "CG.hpp"
#include "Geometry.hpp"
#include "SparseMatrix.hpp"

/*!
  Test the correctness of the Preconditined CG implementation by using a system matrix with a dominant diagonal.

  @param[in]    geom The description of the problem's geometry.
  @param[in]    A    The known system matrix
  @param[in]    data the data structure with all necessary CG vectors preallocated
  @param[in]    b    The known right hand side vector
  @param[inout] x    On entry: the initial guess; on exit: the new approximate solution
  @param[out]   testcg_data the data structure with the results of the test including pass/fail information

  @return Returns zero on success and a non-zero value otherwise.

  @see CG()
 */
int TestCG(Geometry & geom, SparseMatrix & A, CGData & data, double * const b, double * const x, TestCGData * testcg_data) {


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
  double tolerance = 1.0e-12; // Set tolerance to reasonable value for grossly scaled diagonal terms
  testcg_data->expected_niters_no_prec = 12; // For the unpreconditioned CG call, we should take about 10 iterations, permit 12
  testcg_data->expected_niters_prec = 2;   // For the preconditioned case, we should take about 1 iteration, per 2
  testcg_data->niters_max_no_prec = 0;
  testcg_data->niters_max_prec = 0;
  for (int k=0; k<2; ++k) { // This loop tests both unpreconditioned and preconditioned runs
    int expected_niters = testcg_data->expected_niters_no_prec;
    if (k==1) expected_niters = testcg_data->expected_niters_prec;
    for (int i=0; i< numberOfCgCalls; ++i) {
      for (int j=0; j< A.localNumberOfRows; ++j) x[j] = 0.0; // Zero out x
      int ierr = CG( geom, A, data, b, x, maxIters, tolerance, niters, normr, normr0, &times[0], k==1);
      if (ierr) HPCG_fout << "Error in call to CG: " << ierr << ".\n" << endl;
      if (niters <= expected_niters) {
        ++testcg_data->count_pass;
      } else {
        ++testcg_data->count_fail;
      }
      if (k==0 && niters>testcg_data->niters_max_no_prec) testcg_data->niters_max_no_prec = niters; // Keep track of largest iter count
      if (k==1 && niters>testcg_data->niters_max_prec) testcg_data->niters_max_prec = niters; // Same for preconditioned run
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
  testcg_data->normr = normr;

  return 0;
}
