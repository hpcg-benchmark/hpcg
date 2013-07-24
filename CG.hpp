
//@HEADER
// ************************************************************************
// 
//               HPCG: Simple Conjugate Gradient Benchmark Code
// Questions? Contact Michael A. Heroux (maherou@sandia.gov) 
// 
// ************************************************************************
//@HEADER

#ifndef CG_HPP
#define CG_HPP

#include "SparseMatrix.hpp"
#include "Geometry.hpp"

int CG(const Geometry & geom, const SparseMatrix & A, const double * const b, double * const x,
	  const int max_iter, const double tolerance, int & niters, double & normr, double * times);

// this function will compute the Conjugate Gradient iterations.
// geom - Domain and processor topology information
// A - Matrix
// b - constant
// x - used for return value
// max_iter - how many times we iterate
// tolerance - Stopping tolerance for preconditioned iterations.
// niters - number of iterations performed
// normr - computed residual norm
// times - array of timing information

#endif  // CG_HPP
