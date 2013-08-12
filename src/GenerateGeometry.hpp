//@HEADER
// ************************************************************************
//
//               HPCG: Simple Conjugate Gradient Benchmark Code
// Questions? Contact Michael A. Heroux (maherou@sandia.gov)
//
// ************************************************************************
//@HEADER
// We provide our own cube root function because the C++11 standard now define std::cbrt in cmath, but this function
// is not universally available right now.

#ifndef GENERATEGEOMETRY_HPP
#define GENERATEGEOMETRY_HPP
#include "Geometry.hpp"
void GenerateGeometry(int size, int rank, int nx, int ny, int nz, Geometry & geom);
#endif // GENERATEGEOMETRY_HPP
