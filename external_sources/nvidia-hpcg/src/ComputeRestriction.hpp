
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

#ifndef COMPUTERESTRICTION_HPP
#define COMPUTERESTRICTION_HPP
#include "SparseMatrix.hpp"
#include "Vector.hpp"
int ComputeRestriction(const SparseMatrix& A, const Vector& rf);
#endif // COMPUTERESTRICTION_HPP
