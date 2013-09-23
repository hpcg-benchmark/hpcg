
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
  int count_pass; //!< number of succesful tests
  int count_fail;  //!< number of succesful tests
  int expected_niters_no_prec; //!< expected number of test CG iterations without preconditioning with diagonally dominant matrix (~12)
  int expected_niters_prec; //!< expected number of test CG iterations with preconditioning and with diagonally dominant matrix (~1-2)
  int niters_max_no_prec; //!< maximum number of test CG iterations without predictitioner
  int niters_max_prec; //!< maximum number of test CG iterations without predictitioner
  double normr; //!< residual norm achieved during test CG iterations
};
typedef struct CGtestData_STRUCT CGtestData;

extern int CGtest(Geometry & geom, SparseMatrix & A, CGData & data, double * const b, double * const x, CGtestData * cgtest_data);

#endif  // CGTEST_HPP

