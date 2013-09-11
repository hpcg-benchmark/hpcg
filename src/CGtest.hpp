
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
#include "CGData.hpp"


struct CGtestData_STRUCT {
  int count_pass, count_fail, niters;
  double normr;
};
typedef struct CGtestData_STRUCT CGtestData;

extern int CGtest(HPCG_Params * params, Geometry & geom, SparseMatrix & A, CGData & data, double * const b, double * const x, CGtestData * cgtest_data);

#endif  // CGTEST_HPP

