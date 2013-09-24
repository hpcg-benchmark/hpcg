//@HEADER
// ************************************************************************
// 
//               HPCG: Simple Conjugate Gradient Benchmark Code
// Questions? Contact Michael A. Heroux (maherou@sandia.gov) 
// 
// ************************************************************************
//@HEADER

#ifndef COMPUTEDOTPRODUCT_HPP
#define COMPUTEDOTPRODUCT_HPP
#include "Geometry.hpp"
int ComputeDotProduct(const local_int_t n, const double * const x, const double * const y,
	  double * const result, double & time_allreduce);

#endif // COMPUTEDOTPRODUCT_HPP
