
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

#ifndef COMPUTEWAXPBY_HPP
#define COMPUTEWAXPBY_HPP
#include "Geometry.hpp"
int ComputeWAXPBY(const local_int_t n, const double alpha, const double * const x,
    const double beta, const double * const y,
    double * const w, bool & isOptimized);
#endif // COMPUTEWAXPBY_HPP
