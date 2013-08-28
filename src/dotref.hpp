//@HEADER
// ************************************************************************
// 
//               HPCG: Simple Conjugate Gradient Benchmark Code
// Questions? Contact Michael A. Heroux (maherou@sandia.gov) 
// 
// ************************************************************************
//@HEADER

#ifndef DOTREF_HPP
#define DOTREF_HPP
#include "Geometry.hpp"
int dotref (const local_int_t n, const double * const x, const double * const y,
	  double * const result, double & time_allreduce);

#endif // DOTREF_HPP
