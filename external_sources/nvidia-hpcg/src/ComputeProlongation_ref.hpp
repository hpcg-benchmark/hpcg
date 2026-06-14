
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

#ifndef COMPUTEPROLONGATION_REF_HPP
#define COMPUTEPROLONGATION_REF_HPP
#include "SparseMatrix.hpp"
#include "Vector.hpp"
int ComputeProlongation_ref(const SparseMatrix& Af, Vector& xf);
#endif // COMPUTEPROLONGATION_REF_HPP
