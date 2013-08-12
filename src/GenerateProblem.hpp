
//@HEADER
// ************************************************************************
// 
//               HPCG: Simple Conjugate Gradient Benchmark Code
// Questions? Contact Michael A. Heroux (maherou@sandia.gov) 
// 
// ************************************************************************
//@HEADER

#ifndef GENERATEPROBLEM_HPP
#define GENERATEPROBLEM_HPP
#include "Geometry.hpp"
#include "SparseMatrix.hpp"

void GenerateProblem(const Geometry & geom, SparseMatrix &A, double **x, double **b, double **xexact);
#endif // GENERATEPROBLEM_HPP
