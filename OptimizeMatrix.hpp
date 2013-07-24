
//@HEADER
// ************************************************************************
// 
//               HPCG: Simple Conjugate Gradient Benchmark Code
// Questions? Contact Michael A. Heroux (maherou@sandia.gov) 
// 
// ************************************************************************
//@HEADER

#ifndef OPTIMIZEMATRIX_HPP
#define OPTIMIZEMATRIX_HPP
#include "Geometry.hpp"
#include "SparseMatrix.hpp"

void OptimizeMatrix(const Geometry & geom, SparseMatrix & A);

#endif // OPTIMIZEMATRIX_HPP
