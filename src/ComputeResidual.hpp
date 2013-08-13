
//@HEADER
// ************************************************************************
// 
//               HPCG: Simple Conjugate Gradient Benchmark Code
// Questions? Contact Michael A. Heroux (maherou@sandia.gov) 
// 
// ************************************************************************
//@HEADER

#ifndef COMPUTERESIDUAL_HPP
#define COMPUTERESIDUAL_HPP
#include "Geometry.hpp"
int ComputeResidual(const local_int_t n, const double * const v1,
                    const double * const v2, double * const residual);
#endif // COMPUTERESIDUAL_HPP
