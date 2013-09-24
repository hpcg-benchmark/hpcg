
//@HEADER
// ************************************************************************
//
//               HPCG: Simple Conjugate Gradient Benchmark Code
// Questions? Contact Michael A. Heroux (maherou@sandia.gov)
//
// ************************************************************************
//@HEADER

#ifndef SETUPHALO_HPP
#define SETUPHALO_HPP
#include "Geometry.hpp"
#include "SparseMatrix.hpp"

void SetupHalo(const Geometry & geom, SparseMatrix & A);

#endif // SETUPHALO_HPP
