
//@HEADER
// ************************************************************************
// 
//               HPCG: Simple Conjugate Gradient Benchmark Code
// Questions? Contact Michael A. Heroux (maherou@sandia.gov) 
// 
// ************************************************************************
//@HEADER
#ifndef WAXPBY_HPP
#define WAXPBY_HPP
#include "Geometry.hpp"
int waxpby (const local_int_t n, const double alpha, const double * const x,
	    const double beta, const double * const y, 
		     double * const w);
#endif // WAXPBY_HPP
