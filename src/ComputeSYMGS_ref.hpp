
//@HEADER
// ************************************************************************
//
//               HPCG: Simple Conjugate Gradient Benchmark Code
// Questions? Contact Michael A. Heroux (maherou@sandia.gov)
//
// ************************************************************************
//@HEADER

#ifndef COMPUTESYMGS_REF_HPP
#define COMPUTESYMGS_REF_HPP
#include "SparseMatrix.hpp"

int ComputeSYMGS_ref( const SparseMatrix  & A, const double * const x, double * const y);

#endif // COMPUTESYMGS_REF_HPP
