
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

#ifndef COMPUTEPROLONGATION_HPP
#define COMPUTEPROLONGATION_HPP
#include "SparseMatrix.hpp"
#include "Vector.hpp"
int ComputeProlongation(const SparseMatrix& Af, Vector& xf);
#endif // COMPUTEPROLONGATION_HPP
