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

int dot (const int n, const double * const x, const double * const y, 
	  double * const result, double & time_allreduce);

#endif // DOT_HPP
