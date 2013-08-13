
//@HEADER
// ************************************************************************
// 
//               HPCG: Simple Conjugate Gradient Benchmark Code
// Questions? Contact Michael A. Heroux (maherou@sandia.gov) 
// 
// ************************************************************************
//@HEADER

#ifndef SYMGS_HPP
#define SYMGS_HPP
#include "SparseMatrix.hpp"

int symgs( const SparseMatrix  & A, const double * const x, double * const y);

#endif // SYMGS_HPP
