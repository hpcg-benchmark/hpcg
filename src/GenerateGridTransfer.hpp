
//@HEADER
// ***************************************************
//
// HPCG: High Performance Conjugate Gradient Benchmark
//
// Contact:
// Michael A. Heroux ( maherou@sandia.gov)
// Jack Dongarra     (dongarra@eecs.utk.edu)
// Piotr Luszczek    (luszczek@eecs.utk.edu)
//
// ***************************************************
//@HEADER

#ifndef GENERATEGRIDTRANSFER_HPP
#define GENERATEGRIDTRANSFER_HPP
#include "Geometry.hpp"
#include "SparseMatrix.hpp"

void GenerateGridTransfer(const SparseMatrix & A, SparseMatrix & RP);
#endif // GENERATEGRIDTRANSFER_HPP
