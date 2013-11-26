
//@HEADER
// ***************************************************
//
// HPCG: High Performance Conjugate Gradient Benchmark
//
// Contact:
// Michael A. Heroux ( maherou@sandia.gov)
// Jack Dongarra     (dongarra@eecs.utk.edu)
// Piotr Luszczek    (luszczek@eecs.utk.edu)
//
// ***************************************************
//@HEADER

/*!
 @file TestSymmetry.cpp

 HPCG routine
 */

#include <fstream>
#include <iostream>
#include <cfloat>
#include <cstdlib>
using std::endl;
#include <vector>
#include <cmath>
#ifndef HPCG_NOMPI
#include <mpi.h> // If this routine is not compiled with -DHPCG_NOMPI then include mpi.h
#endif

#include "hpcg.hpp"

#include "ComputeSPMV.hpp"
#include "ComputeSYMGS.hpp"
#include "ComputeDotProduct.hpp"
#include "ComputeResidual.hpp"
#include "Geometry.hpp"
#include "SparseMatrix.hpp"
#include "TestSymmetry.hpp"
#ifndef HPCG_NOMPI
#include "ExchangeHalo.hpp"
#endif

/*!
  Tests symmetry-preserving properties of the sparse matrix vector multiply and
  symmetric Gauss-Siedel routines.

  @param[in]    geom   The description of the problem's geometry.
  @param[in]    A      The known system matrix
  @param[in]    b      The known right hand side vector
  @param[in]    xexact The exact solution vector
  @param[inout] testsymmetry_data The data structure with the results of the CG symmetry test including pass/fail information

  @return returns 0 upon success and non-zero otherwise

  @see ComputeDotProduct
  @see ComputeDotProduct_ref
  @see ComputeSPMV
  @see ComputeSPMV_ref
  @see ComputeSYMGS
  @see ComputeSYMGS_ref
*/
int TestSymmetry(Geometry & geom, SparseMatrix & A, double * const b, double * const xexact, TestSymmetryData * testsymmetry_data) {

  local_int_t nrow = A.localNumberOfRows;
  local_int_t ncol = A.localNumberOfColumns;

  double * x_overlap = new double [ncol]; // Overlapped copy of x vector
  double * y_overlap = new double [ncol]; // Overlapped copy of y vector
  double * b_computed = new double [nrow]; // Computed RHS vector
  double t4 = 0.0; // Needed for dot-product call, otherwise unused
  testsymmetry_data->count_fail = 0;

  // Test symmetry of matrix

  // First load vectors with random values
  for (int i=0; i<nrow; ++i) {
    x_overlap[i] = rand() / (double)(RAND_MAX) + 1;
    y_overlap[i] = rand() / (double)(RAND_MAX) + 1;
  }

  double xNorm2, yNorm2;
  double ANorm = 2 * 26.0;

  // Next, compute x'*A*y
#ifndef HPCG_NOMPI
  ExchangeHalo(A,y_overlap);
#endif
  ComputeDotProduct(nrow, y_overlap, y_overlap, &yNorm2, t4, A.isDotProductOptimized);
  int ierr = ComputeSPMV(A, y_overlap, b_computed); // b_computed = A*y_overlap
  if (ierr) HPCG_fout << "Error in call to SpMV: " << ierr << ".\n" << endl;
  double xtAy = 0.0;
  ierr = ComputeDotProduct(nrow, x_overlap, b_computed, &xtAy, t4, A.isDotProductOptimized); // b_computed = A*y_overlap
  if (ierr) HPCG_fout << "Error in call to dot: " << ierr << ".\n" << endl;

  // Next, compute y'*A*x
#ifndef HPCG_NOMPI
  ExchangeHalo(A,x_overlap);
#endif
  ComputeDotProduct(nrow, x_overlap, x_overlap, &xNorm2, t4, A.isDotProductOptimized);
  ierr = ComputeSPMV(A, x_overlap, b_computed); // b_computed = A*x_overlap
  if (ierr) HPCG_fout << "Error in call to SpMV: " << ierr << ".\n" << endl;
  double ytAx = 0.0;
  ierr = ComputeDotProduct(nrow, y_overlap, b_computed, &ytAx, t4, A.isDotProductOptimized); // b_computed = A*y_overlap
  if (ierr) HPCG_fout << "Error in call to dot: " << ierr << ".\n" << endl;
  testsymmetry_data->depsym_spmv = std::fabs((long double) (xtAy - ytAx))/((xNorm2*ANorm*yNorm2 + yNorm2*ANorm*xNorm2) * (DBL_EPSILON));
  if (testsymmetry_data->depsym_spmv > 1.0) ++testsymmetry_data->count_fail;  // If the difference is > 1, count it wrong
  if (geom.rank==0) HPCG_fout << "Departure from symmetry (scaled) for SpMV abs(x'*A*y - y'*A*x) = " << testsymmetry_data->depsym_spmv << endl;

  // Test symmetry of symmetric Gauss-Seidel

  // Compute x'*Minv*y
  ierr = ComputeSYMGS(A, y_overlap, b_computed); // b_computed = Minv*y_overlap
  if (ierr) HPCG_fout << "Error in call to SymGS: " << ierr << ".\n" << endl;
  double xtMinvy = 0.0;
  ierr = ComputeDotProduct(nrow, x_overlap, b_computed, &xtMinvy, t4, A.isDotProductOptimized); // b_computed = A*x_overlap
  if (ierr) HPCG_fout << "Error in call to dot: " << ierr << ".\n" << endl;

  // Next, compute y'*Minv*x
  ierr = ComputeSYMGS(A, x_overlap, b_computed); // b_computed = Minv*x_overlap
  if (ierr) HPCG_fout << "Error in call to SymGS: " << ierr << ".\n" << endl;
  double ytMinvx = 0.0;
  ierr = ComputeDotProduct(nrow, y_overlap, b_computed, &ytMinvx, t4, A.isDotProductOptimized); // b_computed = A*y_overlap
  if (ierr) HPCG_fout << "Error in call to dot: " << ierr << ".\n" << endl;
  testsymmetry_data->depsym_symgs = std::fabs((long double) (xtMinvy - ytMinvx))/((xNorm2*ANorm*yNorm2 + yNorm2*ANorm*xNorm2) * (DBL_EPSILON));
  if (testsymmetry_data->depsym_symgs > 1.0) ++testsymmetry_data->count_fail;  // If the difference is > 1, count it wrong
  if (geom.rank==0) HPCG_fout << "Departure from symmetry (scaled) for SymGS abs(x'*Minv*y - y'*Minv*x) = " << testsymmetry_data->depsym_symgs << endl;

  for (int i=0; i< nrow; ++i) x_overlap[i] = xexact[i]; // Copy exact answer into overlap vector

  int numberOfCalls = 2;
  double residual = 0.0;
  for (int i=0; i< numberOfCalls; ++i) {
#ifndef HPCG_NOMPI
    ExchangeHalo(A,x_overlap);
#endif
    ierr = ComputeSPMV(A, x_overlap, b_computed); // b_computed = A*x_overlap
    if (ierr) HPCG_fout << "Error in call to SpMV: " << ierr << ".\n" << endl;
    if ((ierr = ComputeResidual(A.localNumberOfRows, b, b_computed, &residual)))
      HPCG_fout << "Error in call to compute_residual: " << ierr << ".\n" << endl;
    if (geom.rank==0) HPCG_fout << "SpMV call [" << i << "] Residual [" << residual << "]" << endl;
  }
  delete [] x_overlap;
  delete [] y_overlap;
  delete [] b_computed;

  return 0;
}
