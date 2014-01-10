
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

#ifndef REPORTRESULTS_HPP
#define REPORTRESULTS_HPP
#include "SparseMatrix.hpp"
#include "TestCG.hpp"
#include "TestSymmetry.hpp"
#include "TestNorms.hpp"

void ReportResults(const SparseMatrix & A, int numberOfCgSets, int niters, double times[],
    const TestCGData & testcg_data, const TestSymmetryData & testsymmetry_data, const TestNormsData & testnorms_data, int global_failure);

#endif // REPORTRESULTS_HPP
