
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

/*!
 @file CGData.hpp

 HPCG data structure
 */

#ifndef CGDATA_HPP
#define CGDATA_HPP

#include "SparseMatrix.hpp"

struct CGData_STRUCT {
  double * r; //!< pointer to residual vector
  double * z; //!< pointer to preconditioned residual vector
  double * p; //!< pointer to direction vector
  double * Ap; //!< pointer to Krylov vector
};
typedef struct CGData_STRUCT CGData;

/*!
 Constructor for the data structure of CG vectors.

 @param[in]  A    the data structure that describes the problem matrix and its structure
 @param[out] data the data structure for CG vectors that will be allocated to get it ready for use in CG iterations
 */
inline void InitializeSparseCGData(SparseMatrix & A, CGData & data) {
  local_int_t nrow = A.localNumberOfRows;
  local_int_t ncol = A.localNumberOfColumns;
  data.r = new double [nrow]; // Residual vector
  data.z = new double [nrow]; // Preconditioned residual vector
  data.Ap = new double [nrow];
  data.p = new double [ncol]; // Direction vector (in MPI mode ncol>=nrow)
  return;
}

/*!
 Destructor for the CG vectors data.

 @param[inout] data the CG vectors data structure whose storage is deallocated
 */
inline void DeleteCGData(CGData & data) {

  delete [] data.r;
  delete [] data.z;
  delete [] data.Ap;
  delete [] data.p;
  return;
}

#endif // CGDATA_HPP

