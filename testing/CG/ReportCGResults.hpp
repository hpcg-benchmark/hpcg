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

void ReportResults(const Geometry & geom, const SparseMatrix & A, int niters, double normr, double times[]);

#endif // REPORTRESULTS_HPP
