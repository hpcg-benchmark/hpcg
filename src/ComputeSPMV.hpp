
//@HEADER
// ************************************************************************
//
//               HPCG: Simple Conjugate Gradient Benchmark Code
// Questions? Contact Michael A. Heroux (maherou@sandia.gov)
//
// ************************************************************************
//@HEADER

#ifndef COMPUTESPMV_HPP
#define COMPUTESPMV_HPP
#include "Geometry.hpp"
#include "SparseMatrix.hpp"

int ComputeSPMV( const SparseMatrix & A, const double * const x, double * const y);

#endif  // COMPUTESPMV_HPP
