
//@HEADER
// ************************************************************************
// 
//               HPCG: Simple Conjugate Gradient Benchmark Code
// Questions? Contact Michael A. Heroux (maherou@sandia.gov) 
// 
// ************************************************************************
//@HEADER

#ifndef CGDATA_HPP
#define CGDATA_HPP

#include "SparseMatrix.hpp"

struct CGData_STRUCT {
  double *r;
  double *z;
  double *p;
  double *Ap;
};
typedef struct CGData_STRUCT CGData;

inline void initializeCGData(SparseMatrix & A, CGData & data) {
	local_int_t nrow = A.localNumberOfRows;
	local_int_t ncol = A.localNumberOfColumns;
	data.r = new double [nrow]; // Residual vector
	data.z = new double [nrow]; // Preconditioned residual vector
	data.Ap = new double [nrow];
	data.p = new double [ncol]; // Direction vector (in MPI mode ncol>=nrow)
	return;
}

inline void destroyCGData(CGData & data) {

	delete [] data.r;
	delete [] data.z;
	delete [] data.Ap;
	delete [] data.p;
	return;
}

#endif // CGDATA_HPP

