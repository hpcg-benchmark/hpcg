
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
int waxpby (const int n, const double alpha, const double * const x, 
	    const double beta, const double * const y, 
		     double * const w);
#endif // WAXPBY_HPP
