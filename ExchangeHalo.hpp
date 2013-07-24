
//@HEADER
// ************************************************************************
// 
//               HPCG: Simple Conjugate Gradient Benchmark Code
// Questions? Contact Michael A. Heroux (maherou@sandia.gov) 
// 
// ************************************************************************
//@HEADER

#ifndef EXCHANGEHALO_HPP
#define EXCHANGEHALO_HPP
#include "SparseMatrix.hpp"
void ExchangeHalo(const SparseMatrix & A, const double *x);
#endif // EXCHANGEHALO_HPP
