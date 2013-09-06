
//@HEADER
// ************************************************************************
//
//               HPCG: Simple Conjugate Gradient Benchmark Code
// Questions? Contact Michael A. Heroux (maherou@sandia.gov)
//
// ************************************************************************
//@HEADER

#ifndef SYMTEST_HPP
#define SYMTEST_HPP

#include "hpcg.hpp"
#include "SparseMatrix.hpp"
#include "Geometry.hpp"

struct SymTestData_STRUCT {
  double depsym_spmv, depsym_symgs;
};
typedef struct SymTestData_STRUCT SymTestData;

extern int SymTest(HPCG_Params *params, Geometry& geom, int size, int rank, SymTestData *symtest_data);

#endif  // SYMTEST_HPP
