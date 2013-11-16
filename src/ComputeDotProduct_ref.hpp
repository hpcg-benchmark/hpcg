
//@HEADER
// ************************************************************************
//
//               HPCG: Simple Conjugate Gradient Benchmark Code
// Questions? Contact Michael A. Heroux (maherou@sandia.gov)
//
// ************************************************************************
//@HEADER

#ifndef COMPUTEDOTPRODUCT_REF_HPP
#define COMPUTEDOTPRODUCT_REF_HPP
#include "Geometry.hpp"
int ComputeDotProduct_ref(const local_int_t n, const double * const x, const double * const y,
    double * const result, double & time_allreduce);

#endif // COMPUTEDOTPRODUCT_REF_HPP
