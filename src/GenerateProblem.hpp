
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

#ifndef GENERATEPROBLEM_HPP
#define GENERATEPROBLEM_HPP
#include "Geometry.hpp"
#include "SparseMatrix.hpp"

void GenerateProblem(const Geometry & geom, SparseMatrix & A, double ** b, double ** x, double ** xexact);
#endif // GENERATEPROBLEM_HPP
