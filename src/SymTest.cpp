
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
#include <cstdlib>
using std::endl;
#include <vector>
#include <cmath>
#ifndef HPCG_NOMPI
#include <mpi.h> // If this routine is not compiled with -DHPCG_NOMPI then include mpi.h
#endif

#include "hpcg.hpp"

#include "spmv.hpp"
#include "symgs.hpp"
#include "dot.hpp"
#include "ComputeResidual.hpp"
#include "Geometry.hpp"
#include "SparseMatrix.hpp"
#include "SymTest.hpp"
#ifndef HPCG_NOMPI
#include "ExchangeHalo.hpp"
#endif

int SymTest(Geometry & geom, SparseMatrix & A, double * const b, double * const xexact, SymTestData * symtest_data) {

    local_int_t nrow = A.localNumberOfRows;
    local_int_t ncol = A.localNumberOfColumns;

    double * x_overlap = new double [ncol]; // Overlapped copy of x vector
    double * y_overlap = new double [ncol]; // Overlapped copy of y vector
    double * b_computed = new double [nrow]; // Computed RHS vector
    double t4 = 0.0; // Needed for dot call, otherwise unused
    symtest_data->count_fail = 0;

    // Test symmetry of matrix

    // First load vectors with random values
    for (int i=0; i<nrow; ++i) {
      x_overlap[i] = rand() / (double)(RAND_MAX) + 1;
      y_overlap[i] = rand() / (double)(RAND_MAX) + 1;
    }

    // Next, compute x'*A*y
#ifndef HPCG_NOMPI
    ExchangeHalo(A,y_overlap);
#endif
    int ierr = spmv(A, y_overlap, b_computed); // b_computed = A*y_overlap
    if (ierr) HPCG_fout << "Error in call to spmv: " << ierr << ".\n" << endl;
    double xtAy = 0.0;
    ierr = dot(nrow, x_overlap, b_computed, &xtAy, t4); // b_computed = A*y_overlap
    if (ierr) HPCG_fout << "Error in call to dot: " << ierr << ".\n" << endl;

    // Next, compute y'*A*x
#ifndef HPCG_NOMPI
    ExchangeHalo(A,x_overlap);
#endif
    ierr = spmv(A, x_overlap, b_computed); // b_computed = A*y_overlap
    if (ierr) HPCG_fout << "Error in call to spmv: " << ierr << ".\n" << endl;
    double ytAx = 0.0;
    ierr = dot(nrow, y_overlap, b_computed, &ytAx, t4); // b_computed = A*y_overlap
    if (ierr) HPCG_fout << "Error in call to dot: " << ierr << ".\n" << endl;
    symtest_data->depsym_spmv = std::fabs((long double) (xtAy - ytAx));
    if (symtest_data->depsym_spmv > 1.0e-4) ++symtest_data->count_fail;  // If the difference is > 10e-4, count it wrong
    if (geom.rank==0) HPCG_fout << "Departure from symmetry for spmv abs(x'*A*y - y'*A*x) = " << symtest_data->depsym_spmv << endl;

    // Test symmetry of symmetric Gauss-Seidel

    // Compute x'*Minv*y
    ierr = symgs(A, y_overlap, b_computed); // b_computed = Minv*y_overlap
    if (ierr) HPCG_fout << "Error in call to symgs: " << ierr << ".\n" << endl;
    double xtMinvy = 0.0;
    ierr = dot(nrow, x_overlap, b_computed, &xtMinvy, t4); // b_computed = A*y_overlap
    if (ierr) HPCG_fout << "Error in call to dot: " << ierr << ".\n" << endl;

    // Next, compute y'*Minv*x
    ierr = symgs(A, x_overlap, b_computed); // b_computed = Minv*y_overlap
    if (ierr) HPCG_fout << "Error in call to symgs: " << ierr << ".\n" << endl;
    double ytMinvx = 0.0;
    ierr = dot(nrow, y_overlap, b_computed, &ytMinvx, t4); // b_computed = A*y_overlap
    if (ierr) HPCG_fout << "Error in call to dot: " << ierr << ".\n" << endl;
    symtest_data->depsym_symgs = std::fabs((long double) (xtMinvy - ytMinvx));
    if (symtest_data->depsym_symgs > 1.0e-4) ++symtest_data->count_fail;  // If the difference is > 10e-4, count it wrong
    if (geom.rank==0) HPCG_fout << "Departure from symmetry for symgs abs(x'*Minv*y - y'*Minv*x) = " << symtest_data->depsym_symgs << endl;

    for (int i=0; i< nrow; ++i) x_overlap[i] = xexact[i]; // Copy exact answer into overlap vector

    int numberOfCalls = 2;
    double residual = 0.0;
    for (int i=0; i< numberOfCalls; ++i) {
#ifndef HPCG_NOMPI
      ExchangeHalo(A,x_overlap);
#endif
      ierr = spmv(A, x_overlap, b_computed); // b_computed = A*x_overlap
      if (ierr) HPCG_fout << "Error in call to spmv: " << ierr << ".\n" << endl;
      if ((ierr = ComputeResidual(A.localNumberOfRows, b, b_computed, &residual)))
        HPCG_fout << "Error in call to compute_residual: " << ierr << ".\n" << endl;
      if (geom.rank==0) HPCG_fout << "SpMV call [" << i << "] Residual [" << residual << "]" << endl;
    }
    delete [] x_overlap;
    delete [] y_overlap;
    delete [] b_computed;

    return 0;
}
