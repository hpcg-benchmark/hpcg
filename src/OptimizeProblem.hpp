
//@HEADER
// ************************************************************************
//
//               HPCG: Simple Conjugate Gradient Benchmark Code
// Questions? Contact Michael A. Heroux (maherou@sandia.gov)
//
// ************************************************************************
//@HEADER

#ifndef OPTIMIZEPROBLEM_HPP
#define OPTIMIZEPROBLEM_HPP

#include "SparseMatrix.hpp"
#include "Geometry.hpp"
#include "CGData.hpp"

int OptimizeProblem(const Geometry & geom, SparseMatrix & A, CGData & data, double * b, double * x, double * xexact);

#endif  // OPTIMIZEPROBLEM_HPP
