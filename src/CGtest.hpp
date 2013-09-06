
//@HEADER
// ************************************************************************
// 
//               HPCG: Simple Conjugate Gradient Benchmark Code
// Questions? Contact Michael A. Heroux (maherou@sandia.gov) 
// 
// ************************************************************************
//@HEADER

#ifndef CGTEST_HPP
#define CGTEST_HPP

#include "hpcg.hpp"
#include "SparseMatrix.hpp"
#include "Geometry.hpp"

struct CGtestData_STRUCT {
  int count_pass, count_fail, niters;
  double normr;
};
typedef struct CGtestData_STRUCT CGtestData;

extern int CGtest(HPCG_Params *params, Geometry& geom, int size, int rank, CGtestData *cgtest_data);

#endif  // CGTEST_HPP

