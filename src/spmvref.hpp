
//@HEADER
// ************************************************************************
// 
//               HPCG: Simple Conjugate Gradient Benchmark Code
// Questions? Contact Michael A. Heroux (maherou@sandia.gov) 
// 
// ************************************************************************
//@HEADER

#ifndef SPMVREF_HPP
#define SPMVREF_HPP
#include "Geometry.hpp"
#include "SparseMatrix.hpp"

int spmvref( const SparseMatrix & A, const double * const x, double * const y);

#endif  // SPMVREF_HPP
