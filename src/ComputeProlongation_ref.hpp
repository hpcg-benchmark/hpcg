
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
#include "Vector.hpp"
#include "SparseMatrix.hpp"
int ComputeRestriction_ref(const SparseMatrix & A, const Vector & x);
#endif // COMPUTEPROLONGATION_REF_HPP
