
//@HEADER
// ************************************************************************
// 
//               HPCG: Simple Conjugate Gradient Benchmark Code
// Questions? Contact Michael A. Heroux (maherou@sandia.gov) 
// 
// ************************************************************************
//@HEADER

#ifndef CGREF_HPP
#define CGREF_HPP

#include "SparseMatrix.hpp"
#include "Geometry.hpp"
#include "CGData.hpp"

int CGref(const Geometry & geom, const SparseMatrix & A, CGData & data, const double * const b, double * const x,
	  const int max_iter, const double tolerance, int & niters, double & normr,  double & normr0,
          double * times, bool doPreconditioning);

// this function will compute the Conjugate Gradient iterations.
// geom - Domain and processor topology information
// A - Matrix
// b - constant
// x - used for return value
// max_iter - how many times we iterate
// tolerance - Stopping tolerance for preconditioned iterations.
// niters - number of iterations performed
// normr - computed residual norm
// normr0 - Original residual
// times - array of timing information
// doPreconditioning - bool to specify whether or not symmetric GS will be applied.

#endif  // CGREF_HPP
