
//@HEADER
// ************************************************************************
// 
//               HPCG: Simple Conjugate Gradient Benchmark Code
// Questions? Contact Michael A. Heroux (maherou@sandia.gov) 
// 
// ************************************************************************
//@HEADER
#ifndef WAXPBYREF_HPP
#define WAXPBYREF_HPP
#include "Geometry.hpp"
int waxpbyref (const local_int_t n, const double alpha, const double * const x,
	    const double beta, const double * const y, 
		     double * const w);
#endif // WAXPBYREF_HPP
