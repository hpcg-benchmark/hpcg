
//@HEADER
// ************************************************************************
// 
//               HPCG: Simple Conjugate Gradient Benchmark Code
// Questions? Contact Michael A. Heroux (maherou@sandia.gov) 
// 
// ************************************************************************
//@HEADER

#ifndef SPMV_HPP
#define SPMV_HPP
#include "Geometry.hpp"
#include "SparseMatrix.hpp"

int spmv( const SparseMatrix & A, const double * const x, double * const y);

#endif  // SPMV_HPP
