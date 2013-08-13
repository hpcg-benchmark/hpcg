
//@HEADER
// ************************************************************************
// 
//               HPCG: Simple Conjugate Gradient Benchmark Code
// Questions? Contact Michael A. Heroux (maherou@sandia.gov) 
// 
// ************************************************************************
//@HEADER

#ifndef SYMGSREF_HPP
#define SYMGSREF_HPP
#include "SparseMatrix.hpp"

int symgsref( const SparseMatrix  & A, const double * const x, double * const y);

#endif // SYMGSREF_HPP
