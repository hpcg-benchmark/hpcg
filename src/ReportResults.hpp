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
#include "TestSymmetry.hpp"
#include "TestNorms.hpp"

void ReportResults(const Geometry & geom, const SparseMatrix & A, int numberOfCgSets, int niters, double times[],
    TestCGData * testcg_data, TestSymmetryData * testsymmetry_data, TestNormsData * testnorms_data, int global_failure);

#endif // REPORTRESULTS_HPP
