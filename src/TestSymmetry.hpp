
//@HEADER
// ************************************************************************
//
//               HPCG: Simple Conjugate Gradient Benchmark Code
// Questions? Contact Michael A. Heroux (maherou@sandia.gov)
//
// ************************************************************************
//@HEADER

/*!
 @file TestSymmetry.hpp

 HPCG data structures for symmetry testing
 */

#ifndef TESTSYMMETRY_HPP
#define TESTSYMMETRY_HPP

#include "hpcg.hpp"
#include "SparseMatrix.hpp"
#include "Geometry.hpp"
#include "CGData.hpp"

struct TestSymmetryData_STRUCT {
  double depsym_spmv;  //!< departure from symmetry for the SPMV kernel
  double depsym_symgs; //!< departure from symmetry for the SYMGS kernel
  int    count_fail;   //!< number of failures in the symmetry tests
};
typedef struct TestSymmetryData_STRUCT TestSymmetryData;

extern int TestSymmetry(Geometry & geom, SparseMatrix & A, double * const b, double * const xexact, TestSymmetryData * testsymmetry_data);

#endif  // TESTSYMMETRY_HPP
