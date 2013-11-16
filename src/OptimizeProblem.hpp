
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

#ifndef OPTIMIZEPROBLEM_HPP
#define OPTIMIZEPROBLEM_HPP

#include "SparseMatrix.hpp"
#include "Geometry.hpp"
#include "CGData.hpp"

int OptimizeProblem(const Geometry & geom, SparseMatrix & A, CGData & data, double * b, double * x, double * xexact);

#endif  // OPTIMIZEPROBLEM_HPP
