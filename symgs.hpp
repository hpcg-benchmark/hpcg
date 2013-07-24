
//@HEADER
// ************************************************************************
// 
//               HPCG: Simple Conjugate Gradient Benchmark Code
// Questions? Contact Michael A. Heroux (maherou@sandia.gov) 
// 
// ************************************************************************
//@HEADER

#ifndef SYMGS_HPP
#define SYMGS_HPP
#include "SparseMatrix.hpp"
#ifdef USING_MPI
#include <mpi.h> // If this routine is compiled with -DUSING_MPI then include mpi.h
#endif

int symgs( const SparseMatrix  & A, const double * const x, double * const y);

#endif // SYMGS_HPP
