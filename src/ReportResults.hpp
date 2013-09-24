//@HEADER
// ************************************************************************
//
//               HPCG: Simple Conjugate Gradient Benchmark Code
// Questions? Contact Michael A. Heroux (maherou@sandia.gov)
//
// ************************************************************************
//@HEADER

#ifndef REPORTRESULTS_HPP
#define REPORTRESULTS_HPP
#include "Geometry.hpp"
#include "SparseMatrix.hpp"
#include "TestCG.hpp"
#include "SymTest.hpp"
#include "NormTest.hpp"

void ReportResults(const Geometry & geom, const SparseMatrix & A, int numberOfCgSets, int niters, double times[],
  TestCGData * testcg_data, SymTestData * symtest_data, NormTestData * normtest_data, int global_failure);

#endif // REPORTRESULTS_HPP
