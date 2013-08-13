//@HEADER
// ************************************************************************
// 
//               HPCG: Simple Conjugate Gradient Benchmark Code
// Questions? Contact Michael A. Heroux (maherou@sandia.gov) 
// 
// ************************************************************************
//@HEADER

#ifndef DOT_HPP
#define DOT_HPP
#include "Geometry.hpp"
int dot (const local_int_t n, const double * const x, const double * const y,
	  double * const result, double & time_allreduce);

#endif // DOT_HPP
