
//@HEADER
// ***************************************************
//
// HPCG: High Performance Conjugate Gradient Benchmark
//
// Author: SZW
//
// Contact:
// Michael A. Heroux ( maherou@sandia.gov)
// Jack Dongarra     (dongarra@eecs.utk.edu)
// Piotr Luszczek    (luszczek@eecs.utk.edu)
//
// ***************************************************
//@HEADER

#ifndef COMPUTESPMV_NEW_HPP
#define COMPUTESPMV_NEW_HPP
#include "Vector.hpp"
#include "SparseMatrix.hpp"

int ComputeSPMV( const SparseMatrix & A, Vector  & x, Vector & y);

#endif  // COMPUTESPMV_REF_HPP
