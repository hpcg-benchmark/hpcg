
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

#ifndef COMPUTESPMV_REF_HPP
#define COMPUTESPMV_REF_HPP
#include "Geometry.hpp"
#include "SparseMatrix.hpp"

int ComputeSPMV_ref( const SparseMatrix & A, const double * const x, double * const y);

#endif  // COMPUTESPMV_REF_HPP
