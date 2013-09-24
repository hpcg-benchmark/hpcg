
//@HEADER
// ************************************************************************
// 
//               HPCG: Simple Conjugate Gradient Benchmark Code
// Questions? Contact Michael A. Heroux (maherou@sandia.gov) 
// 
// ************************************************************************
//@HEADER
#ifndef COMPUTEWAXPBY_HPP
#define COMPUTEWAXPBY_HPP
#include "Geometry.hpp"
int ComputeWAXPBY(const local_int_t n, const double alpha, const double * const x,
	    const double beta, const double * const y, 
		     double * const w);
#endif // COMPUTEWAXPBY_HPP
