
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
 @file ComputeSYMGS_ref.cpp

 HPCG routine
 */

#ifndef HPCG_NO_MPI
#include "ExchangeHalo.hpp"
#endif
#include "ComputeSYMGS_ref.hpp"
#include <cassert>
#include <iostream>

/*!
  Computes one step of symmetric Gauss-Seidel:

  Assumption about the structure of matrix A:
  - Each row 'i' of the matrix has nonzero diagonal value whose address is matrixDiagonal[i]
  - Entries in row 'i' are ordered such that:
       - lower triangular terms are stored before the diagonal element.
       - upper triangular terms are stored after the diagonal element.
       - No other assumptions are made about entry ordering.

  Symmetric Gauss-Seidel notes:
  - We use the input vector x as the RHS and start with an initial guess for y of all zeros.
  - We perform one forward sweep.  x should be initially zero on the first GS sweep, but we do not attempt to exploit this fact.
  - We then perform one back sweep.
  - For simplicity we include the diagonal contribution in the for-j loop, then correct the sum after

  @param[in] A the known system matrix
  @param[in] r the input vector
  @param[inout] x On entry, x should contain relevant values, on exit x contains the result of one symmetric GS sweep with r as the RHS.


  @warning Early versions of this kernel (Version 1.1 and earlier) had the r and x arguments in reverse order, and out of sync with other kernels.

  @return returns 0 upon success and non-zero otherwise

  @see ComputeSYMGS
*/
int ComputeSYMGS_ref( const SparseMatrix & A, const Vector & r, Vector & x) {

  assert(x.localLength==A.localNumberOfColumns); // Make sure x contain space for halo values

#ifndef HPCG_NO_MPI
  ExchangeHalo(A,x);
#endif

  local_int_t ix = 0;
  local_int_t iy = 0;
  local_int_t iz = 0;
  local_int_t nex = 0;
  local_int_t ney = 0;
  local_int_t nez = 0;
  local_int_t nx = A.geom->nx;
  local_int_t ny = A.geom->ny;
  local_int_t nz = A.geom->nz;
  local_int_t nlocal = nx*ny*nz;
  int npx          = A.geom->npx;
  int npy          = A.geom->npy;
  int npz          = A.geom->npz;
  int ipx          = A.geom->ipx;
  int ipy          = A.geom->ipy;
  int ipz          = A.geom->ipz;

//  const local_int_t nrow = A.localNumberOfRows;
//  double ** matrixDiagonal = A.matrixDiagonal;  // An array of pointers to the diagonal entries A.matrixValues
  const double * const rv = r.values;
  double * const xv = x.values;

  double diagonal_element = A.matrixDiagonal[0][0];// big assumption?

// forward sweep
for( iz=0; iz< nz; iz++){
    for( iy=0; iy< ny; iy++){
        for (ix=0; ix < nx; ix++){

if(ix == 0&& iy == 0&& iz == 0){
xv[ix+iy*nx+iz*ny*nx] = (rv[ix+iy*nx+iz*ny*nx]
+xv[(ix+0)+(iy+0)*nx+(iz+1)*ny*nx]
+xv[(ix+0)+(iy+1)*nx+(iz+0)*ny*nx]
+xv[(ix+0)+(iy+1)*nx+(iz+1)*ny*nx]
+xv[(ix+1)+(iy+0)*nx+(iz+0)*ny*nx]
+xv[(ix+1)+(iy+0)*nx+(iz+1)*ny*nx]
+xv[(ix+1)+(iy+1)*nx+(iz+0)*ny*nx]
+xv[(ix+1)+(iy+1)*nx+(iz+1)*ny*nx]
)/diagonal_element;}
else if(ix == 0&& iy == 0&& iz == nz-1){
xv[ix+iy*nx+iz*ny*nx] = (rv[ix+iy*nx+iz*ny*nx]
+xv[(ix+0)+(iy+0)*nx+(iz+-1)*ny*nx]
+xv[(ix+0)+(iy+1)*nx+(iz+-1)*ny*nx]
+xv[(ix+0)+(iy+1)*nx+(iz+0)*ny*nx]
+xv[(ix+1)+(iy+0)*nx+(iz+-1)*ny*nx]
+xv[(ix+1)+(iy+0)*nx+(iz+0)*ny*nx]
+xv[(ix+1)+(iy+1)*nx+(iz+-1)*ny*nx]
+xv[(ix+1)+(iy+1)*nx+(iz+0)*ny*nx]
)/diagonal_element;}
else if(ix == 0&& iy == ny-1&& iz == 0){
xv[ix+iy*nx+iz*ny*nx] = (rv[ix+iy*nx+iz*ny*nx]
+xv[(ix+0)+(iy+-1)*nx+(iz+0)*ny*nx]
+xv[(ix+0)+(iy+-1)*nx+(iz+1)*ny*nx]
+xv[(ix+0)+(iy+0)*nx+(iz+1)*ny*nx]
+xv[(ix+1)+(iy+-1)*nx+(iz+0)*ny*nx]
+xv[(ix+1)+(iy+-1)*nx+(iz+1)*ny*nx]
+xv[(ix+1)+(iy+0)*nx+(iz+0)*ny*nx]
+xv[(ix+1)+(iy+0)*nx+(iz+1)*ny*nx]
)/diagonal_element;}
else if(ix == 0&& iy == ny-1&& iz == nz-1){
xv[ix+iy*nx+iz*ny*nx] = (rv[ix+iy*nx+iz*ny*nx]
+xv[(ix+0)+(iy+-1)*nx+(iz+-1)*ny*nx]
+xv[(ix+0)+(iy+-1)*nx+(iz+0)*ny*nx]
+xv[(ix+0)+(iy+0)*nx+(iz+-1)*ny*nx]
+xv[(ix+1)+(iy+-1)*nx+(iz+-1)*ny*nx]
+xv[(ix+1)+(iy+-1)*nx+(iz+0)*ny*nx]
+xv[(ix+1)+(iy+0)*nx+(iz+-1)*ny*nx]
+xv[(ix+1)+(iy+0)*nx+(iz+0)*ny*nx]
)/diagonal_element;}
else if(ix == nx-1&& iy == 0&& iz == 0){
xv[ix+iy*nx+iz*ny*nx] = (rv[ix+iy*nx+iz*ny*nx]
+xv[(ix+-1)+(iy+0)*nx+(iz+0)*ny*nx]
+xv[(ix+-1)+(iy+0)*nx+(iz+1)*ny*nx]
+xv[(ix+-1)+(iy+1)*nx+(iz+0)*ny*nx]
+xv[(ix+-1)+(iy+1)*nx+(iz+1)*ny*nx]
+xv[(ix+0)+(iy+0)*nx+(iz+1)*ny*nx]
+xv[(ix+0)+(iy+1)*nx+(iz+0)*ny*nx]
+xv[(ix+0)+(iy+1)*nx+(iz+1)*ny*nx]
)/diagonal_element;}
else if(ix == nx-1&& iy == 0&& iz == nz-1){
xv[ix+iy*nx+iz*ny*nx] = (rv[ix+iy*nx+iz*ny*nx]
+xv[(ix+-1)+(iy+0)*nx+(iz+-1)*ny*nx]
+xv[(ix+-1)+(iy+0)*nx+(iz+0)*ny*nx]
+xv[(ix+-1)+(iy+1)*nx+(iz+-1)*ny*nx]
+xv[(ix+-1)+(iy+1)*nx+(iz+0)*ny*nx]
+xv[(ix+0)+(iy+0)*nx+(iz+-1)*ny*nx]
+xv[(ix+0)+(iy+1)*nx+(iz+-1)*ny*nx]
+xv[(ix+0)+(iy+1)*nx+(iz+0)*ny*nx]
)/diagonal_element;}
else if(ix == nx-1&& iy == ny-1&& iz == 0){
xv[ix+iy*nx+iz*ny*nx] = (rv[ix+iy*nx+iz*ny*nx]
+xv[(ix+-1)+(iy+-1)*nx+(iz+0)*ny*nx]
+xv[(ix+-1)+(iy+-1)*nx+(iz+1)*ny*nx]
+xv[(ix+-1)+(iy+0)*nx+(iz+0)*ny*nx]
+xv[(ix+-1)+(iy+0)*nx+(iz+1)*ny*nx]
+xv[(ix+0)+(iy+-1)*nx+(iz+0)*ny*nx]
+xv[(ix+0)+(iy+-1)*nx+(iz+1)*ny*nx]
+xv[(ix+0)+(iy+0)*nx+(iz+1)*ny*nx]
)/diagonal_element;}
else if(ix == nx-1&& iy == ny-1&& iz == nz-1){
xv[ix+iy*nx+iz*ny*nx] = (rv[ix+iy*nx+iz*ny*nx]
+xv[(ix+-1)+(iy+-1)*nx+(iz+-1)*ny*nx]
+xv[(ix+-1)+(iy+-1)*nx+(iz+0)*ny*nx]
+xv[(ix+-1)+(iy+0)*nx+(iz+-1)*ny*nx]
+xv[(ix+-1)+(iy+0)*nx+(iz+0)*ny*nx]
+xv[(ix+0)+(iy+-1)*nx+(iz+-1)*ny*nx]
+xv[(ix+0)+(iy+-1)*nx+(iz+0)*ny*nx]
+xv[(ix+0)+(iy+0)*nx+(iz+-1)*ny*nx]
)/diagonal_element;}

//edges
else if (ix == 0 && iy == 0 &&iz>0 && iz<nz-1){
xv[ix+iy*nx+iz*ny*nx] = (rv[ix+iy*nx+iz*ny*nx]
+xv[(ix+0)+(iy+0)*nx+(iz+-1)*ny*nx]
+xv[(ix+0)+(iy+1)*nx+(iz+-1)*ny*nx]
+xv[(ix+1)+(iy+0)*nx+(iz+-1)*ny*nx]
+xv[(ix+1)+(iy+1)*nx+(iz+-1)*ny*nx]
+xv[(ix+0)+(iy+1)*nx+(iz+0)*ny*nx]
+xv[(ix+1)+(iy+0)*nx+(iz+0)*ny*nx]
+xv[(ix+1)+(iy+1)*nx+(iz+0)*ny*nx]
+xv[(ix+0)+(iy+0)*nx+(iz+1)*ny*nx]
+xv[(ix+0)+(iy+1)*nx+(iz+1)*ny*nx]
+xv[(ix+1)+(iy+0)*nx+(iz+1)*ny*nx]
+xv[(ix+1)+(iy+1)*nx+(iz+1)*ny*nx]
)/diagonal_element;}
else if (ix == 0 && iy == ny-1 &&iz>0 && iz<nz-1){
xv[ix+iy*nx+iz*ny*nx] = (rv[ix+iy*nx+iz*ny*nx]
+xv[(ix+0)+(iy+-1)*nx+(iz+-1)*ny*nx]
+xv[(ix+0)+(iy+0)*nx+(iz+-1)*ny*nx]
+xv[(ix+1)+(iy+-1)*nx+(iz+-1)*ny*nx]
+xv[(ix+1)+(iy+0)*nx+(iz+-1)*ny*nx]
+xv[(ix+0)+(iy+-1)*nx+(iz+0)*ny*nx]
+xv[(ix+1)+(iy+-1)*nx+(iz+0)*ny*nx]
+xv[(ix+1)+(iy+0)*nx+(iz+0)*ny*nx]
+xv[(ix+0)+(iy+-1)*nx+(iz+1)*ny*nx]
+xv[(ix+0)+(iy+0)*nx+(iz+1)*ny*nx]
+xv[(ix+1)+(iy+-1)*nx+(iz+1)*ny*nx]
+xv[(ix+1)+(iy+0)*nx+(iz+1)*ny*nx]
)/diagonal_element;}
else if (ix == nx-1 && iy == 0 && iz>0 && iz<nz-1){
xv[ix+iy*nx+iz*ny*nx] = (rv[ix+iy*nx+iz*ny*nx]
+xv[(ix+-1)+(iy+0)*nx+(iz+-1)*ny*nx]
+xv[(ix+-1)+(iy+1)*nx+(iz+-1)*ny*nx]
+xv[(ix+0)+(iy+0)*nx+(iz+-1)*ny*nx]
+xv[(ix+0)+(iy+1)*nx+(iz+-1)*ny*nx]
+xv[(ix+-1)+(iy+0)*nx+(iz+0)*ny*nx]
+xv[(ix+-1)+(iy+1)*nx+(iz+0)*ny*nx]
+xv[(ix+0)+(iy+1)*nx+(iz+0)*ny*nx]
+xv[(ix+-1)+(iy+0)*nx+(iz+1)*ny*nx]
+xv[(ix+-1)+(iy+1)*nx+(iz+1)*ny*nx]
+xv[(ix+0)+(iy+0)*nx+(iz+1)*ny*nx]
+xv[(ix+0)+(iy+1)*nx+(iz+1)*ny*nx]
)/diagonal_element;}
else if (ix == nx-1 && iy == ny-1 &&iz>0 && iz<nz-1){
xv[ix+iy*nx+iz*ny*nx] = (rv[ix+iy*nx+iz*ny*nx]
+xv[(ix+-1)+(iy+-1)*nx+(iz+-1)*ny*nx]
+xv[(ix+-1)+(iy+0)*nx+(iz+-1)*ny*nx]
+xv[(ix+0)+(iy+-1)*nx+(iz+-1)*ny*nx]
+xv[(ix+0)+(iy+0)*nx+(iz+-1)*ny*nx]
+xv[(ix+-1)+(iy+-1)*nx+(iz+0)*ny*nx]
+xv[(ix+-1)+(iy+0)*nx+(iz+0)*ny*nx]
+xv[(ix+0)+(iy+-1)*nx+(iz+0)*ny*nx]
+xv[(ix+-1)+(iy+-1)*nx+(iz+1)*ny*nx]
+xv[(ix+-1)+(iy+0)*nx+(iz+1)*ny*nx]
+xv[(ix+0)+(iy+-1)*nx+(iz+1)*ny*nx]
+xv[(ix+0)+(iy+0)*nx+(iz+1)*ny*nx]
)/diagonal_element;}
else if (ix == 0 && iz == 0 &&iy>0 && iy<ny-1){
xv[ix+iy*nx+iz*ny*nx] = (rv[ix+iy*nx+iz*ny*nx]
+xv[(ix+0)+(iy+-1)*nx+(iz+0)*ny*nx]
+xv[(ix+0)+(iy+-1)*nx+(iz+1)*ny*nx]
+xv[(ix+1)+(iy+-1)*nx+(iz+0)*ny*nx]
+xv[(ix+1)+(iy+-1)*nx+(iz+1)*ny*nx]
+xv[(ix+0)+(iy+0)*nx+(iz+1)*ny*nx]
+xv[(ix+1)+(iy+0)*nx+(iz+0)*ny*nx]
+xv[(ix+1)+(iy+0)*nx+(iz+1)*ny*nx]
+xv[(ix+0)+(iy+1)*nx+(iz+0)*ny*nx]
+xv[(ix+0)+(iy+1)*nx+(iz+1)*ny*nx]
+xv[(ix+1)+(iy+1)*nx+(iz+0)*ny*nx]
+xv[(ix+1)+(iy+1)*nx+(iz+1)*ny*nx]
)/diagonal_element;}
else if (ix == 0 && iz == nz-1 &&iy>0 && iy<ny-1){
xv[ix+iy*nx+iz*ny*nx] = (rv[ix+iy*nx+iz*ny*nx]
+xv[(ix+0)+(iy+-1)*nx+(iz+-1)*ny*nx]
+xv[(ix+0)+(iy+-1)*nx+(iz+0)*ny*nx]
+xv[(ix+1)+(iy+-1)*nx+(iz+-1)*ny*nx]
+xv[(ix+1)+(iy+-1)*nx+(iz+0)*ny*nx]
+xv[(ix+0)+(iy+0)*nx+(iz+-1)*ny*nx]
+xv[(ix+1)+(iy+0)*nx+(iz+-1)*ny*nx]
+xv[(ix+1)+(iy+0)*nx+(iz+0)*ny*nx]
+xv[(ix+0)+(iy+1)*nx+(iz+-1)*ny*nx]
+xv[(ix+0)+(iy+1)*nx+(iz+0)*ny*nx]
+xv[(ix+1)+(iy+1)*nx+(iz+-1)*ny*nx]
+xv[(ix+1)+(iy+1)*nx+(iz+0)*ny*nx]
)/diagonal_element;}
else if (ix == nx-1 && iz == 0 && iy>0 && iy<ny-1){
xv[ix+iy*nx+iz*ny*nx] = (rv[ix+iy*nx+iz*ny*nx]
+xv[(ix+-1)+(iy+-1)*nx+(iz+0)*ny*nx]
+xv[(ix+-1)+(iy+-1)*nx+(iz+1)*ny*nx]
+xv[(ix+0)+(iy+-1)*nx+(iz+0)*ny*nx]
+xv[(ix+0)+(iy+-1)*nx+(iz+1)*ny*nx]
+xv[(ix+-1)+(iy+0)*nx+(iz+0)*ny*nx]
+xv[(ix+-1)+(iy+0)*nx+(iz+1)*ny*nx]
+xv[(ix+0)+(iy+0)*nx+(iz+1)*ny*nx]
+xv[(ix+-1)+(iy+1)*nx+(iz+0)*ny*nx]
+xv[(ix+-1)+(iy+1)*nx+(iz+1)*ny*nx]
+xv[(ix+0)+(iy+1)*nx+(iz+0)*ny*nx]
+xv[(ix+0)+(iy+1)*nx+(iz+1)*ny*nx]
)/diagonal_element;}
else if (ix == nx-1 && iz == nz-1 &&iy>0 && iy<ny-1){
xv[ix+iy*nx+iz*ny*nx] = (rv[ix+iy*nx+iz*ny*nx]
+xv[(ix+-1)+(iy+-1)*nx+(iz+-1)*ny*nx]
+xv[(ix+-1)+(iy+-1)*nx+(iz+0)*ny*nx]
+xv[(ix+0)+(iy+-1)*nx+(iz+-1)*ny*nx]
+xv[(ix+0)+(iy+-1)*nx+(iz+0)*ny*nx]
+xv[(ix+-1)+(iy+0)*nx+(iz+-1)*ny*nx]
+xv[(ix+-1)+(iy+0)*nx+(iz+0)*ny*nx]
+xv[(ix+0)+(iy+0)*nx+(iz+-1)*ny*nx]
+xv[(ix+-1)+(iy+1)*nx+(iz+-1)*ny*nx]
+xv[(ix+-1)+(iy+1)*nx+(iz+0)*ny*nx]
+xv[(ix+0)+(iy+1)*nx+(iz+-1)*ny*nx]
+xv[(ix+0)+(iy+1)*nx+(iz+0)*ny*nx]
)/diagonal_element;}
else if (iy == 0 && iz == 0 &&ix>0 && ix<nx-1){
xv[ix+iy*nx+iz*ny*nx] = (rv[ix+iy*nx+iz*ny*nx]
+xv[(ix+-1)+(iy+0)*nx+(iz+0)*ny*nx]
+xv[(ix+-1)+(iy+0)*nx+(iz+1)*ny*nx]
+xv[(ix+-1)+(iy+1)*nx+(iz+0)*ny*nx]
+xv[(ix+-1)+(iy+1)*nx+(iz+1)*ny*nx]
+xv[(ix+0)+(iy+0)*nx+(iz+1)*ny*nx]
+xv[(ix+0)+(iy+1)*nx+(iz+0)*ny*nx]
+xv[(ix+0)+(iy+1)*nx+(iz+1)*ny*nx]
+xv[(ix+1)+(iy+0)*nx+(iz+0)*ny*nx]
+xv[(ix+1)+(iy+0)*nx+(iz+1)*ny*nx]
+xv[(ix+1)+(iy+1)*nx+(iz+0)*ny*nx]
+xv[(ix+1)+(iy+1)*nx+(iz+1)*ny*nx]
)/diagonal_element;}
else if (iy == 0 && iz == nz-1 &&ix>0 && ix<nx-1){
xv[ix+iy*nx+iz*ny*nx] = (rv[ix+iy*nx+iz*ny*nx]
+xv[(ix+-1)+(iy+0)*nx+(iz+-1)*ny*nx]
+xv[(ix+-1)+(iy+0)*nx+(iz+0)*ny*nx]
+xv[(ix+-1)+(iy+1)*nx+(iz+-1)*ny*nx]
+xv[(ix+-1)+(iy+1)*nx+(iz+0)*ny*nx]
+xv[(ix+0)+(iy+0)*nx+(iz+-1)*ny*nx]
+xv[(ix+0)+(iy+1)*nx+(iz+-1)*ny*nx]
+xv[(ix+0)+(iy+1)*nx+(iz+0)*ny*nx]
+xv[(ix+1)+(iy+0)*nx+(iz+-1)*ny*nx]
+xv[(ix+1)+(iy+0)*nx+(iz+0)*ny*nx]
+xv[(ix+1)+(iy+1)*nx+(iz+-1)*ny*nx]
+xv[(ix+1)+(iy+1)*nx+(iz+0)*ny*nx]
)/diagonal_element;}
else if (iy == ny-1 && iz == 0 && ix>0 && ix<nx-1){
xv[ix+iy*nx+iz*ny*nx] = (rv[ix+iy*nx+iz*ny*nx]
+xv[(ix+-1)+(iy+-1)*nx+(iz+0)*ny*nx]
+xv[(ix+-1)+(iy+-1)*nx+(iz+1)*ny*nx]
+xv[(ix+-1)+(iy+0)*nx+(iz+0)*ny*nx]
+xv[(ix+-1)+(iy+0)*nx+(iz+1)*ny*nx]
+xv[(ix+0)+(iy+-1)*nx+(iz+0)*ny*nx]
+xv[(ix+0)+(iy+-1)*nx+(iz+1)*ny*nx]
+xv[(ix+0)+(iy+0)*nx+(iz+1)*ny*nx]
+xv[(ix+1)+(iy+-1)*nx+(iz+0)*ny*nx]
+xv[(ix+1)+(iy+-1)*nx+(iz+1)*ny*nx]
+xv[(ix+1)+(iy+0)*nx+(iz+0)*ny*nx]
+xv[(ix+1)+(iy+0)*nx+(iz+1)*ny*nx]
)/diagonal_element;}
else if (iy == ny-1 && iz == nz-1 &&ix>0 && ix<nx-1){
xv[ix+iy*nx+iz*ny*nx] = (rv[ix+iy*nx+iz*ny*nx]
+xv[(ix+-1)+(iy+-1)*nx+(iz+-1)*ny*nx]
+xv[(ix+-1)+(iy+-1)*nx+(iz+0)*ny*nx]
+xv[(ix+-1)+(iy+0)*nx+(iz+-1)*ny*nx]
+xv[(ix+-1)+(iy+0)*nx+(iz+0)*ny*nx]
+xv[(ix+0)+(iy+-1)*nx+(iz+-1)*ny*nx]
+xv[(ix+0)+(iy+-1)*nx+(iz+0)*ny*nx]
+xv[(ix+0)+(iy+0)*nx+(iz+-1)*ny*nx]
+xv[(ix+1)+(iy+-1)*nx+(iz+-1)*ny*nx]
+xv[(ix+1)+(iy+-1)*nx+(iz+0)*ny*nx]
+xv[(ix+1)+(iy+0)*nx+(iz+-1)*ny*nx]
+xv[(ix+1)+(iy+0)*nx+(iz+0)*ny*nx]
)/diagonal_element;}

//sides
else if (iz == 0 && ix>0 && ix<nx-1 && iy>0 && iy<ny-1){
xv[ix+iy*nx+iz*ny*nx] = (rv[ix+iy*nx+iz*ny*nx]
+xv[(ix+-1)+(iy+-1)*nx+(iz+0)*ny*nx]
+xv[(ix+-1)+(iy+-1)*nx+(iz+1)*ny*nx]
+xv[(ix+-1)+(iy+0)*nx+(iz+0)*ny*nx]
+xv[(ix+-1)+(iy+0)*nx+(iz+1)*ny*nx]
+xv[(ix+-1)+(iy+1)*nx+(iz+0)*ny*nx]
+xv[(ix+-1)+(iy+1)*nx+(iz+1)*ny*nx]
+xv[(ix+0)+(iy+-1)*nx+(iz+0)*ny*nx]
+xv[(ix+0)+(iy+-1)*nx+(iz+1)*ny*nx]
+xv[(ix+0)+(iy+0)*nx+(iz+1)*ny*nx]
+xv[(ix+0)+(iy+1)*nx+(iz+0)*ny*nx]
+xv[(ix+0)+(iy+1)*nx+(iz+1)*ny*nx]
+xv[(ix+1)+(iy+-1)*nx+(iz+0)*ny*nx]
+xv[(ix+1)+(iy+-1)*nx+(iz+1)*ny*nx]
+xv[(ix+1)+(iy+0)*nx+(iz+0)*ny*nx]
+xv[(ix+1)+(iy+0)*nx+(iz+1)*ny*nx]
+xv[(ix+1)+(iy+1)*nx+(iz+0)*ny*nx]
+xv[(ix+1)+(iy+1)*nx+(iz+1)*ny*nx]
)/diagonal_element;}
else if (iz == nz-1 && ix>0 && ix<nx-1 && iy>0 && iy<ny-1){
xv[ix+iy*nx+iz*ny*nx] = (rv[ix+iy*nx+iz*ny*nx]
+xv[(ix+-1)+(iy+-1)*nx+(iz+-1)*ny*nx]
+xv[(ix+-1)+(iy+-1)*nx+(iz+0)*ny*nx]
+xv[(ix+-1)+(iy+0)*nx+(iz+-1)*ny*nx]
+xv[(ix+-1)+(iy+0)*nx+(iz+0)*ny*nx]
+xv[(ix+-1)+(iy+1)*nx+(iz+-1)*ny*nx]
+xv[(ix+-1)+(iy+1)*nx+(iz+0)*ny*nx]
+xv[(ix+0)+(iy+-1)*nx+(iz+-1)*ny*nx]
+xv[(ix+0)+(iy+-1)*nx+(iz+0)*ny*nx]
+xv[(ix+0)+(iy+0)*nx+(iz+-1)*ny*nx]
+xv[(ix+0)+(iy+1)*nx+(iz+-1)*ny*nx]
+xv[(ix+0)+(iy+1)*nx+(iz+0)*ny*nx]
+xv[(ix+1)+(iy+-1)*nx+(iz+-1)*ny*nx]
+xv[(ix+1)+(iy+-1)*nx+(iz+0)*ny*nx]
+xv[(ix+1)+(iy+0)*nx+(iz+-1)*ny*nx]
+xv[(ix+1)+(iy+0)*nx+(iz+0)*ny*nx]
+xv[(ix+1)+(iy+1)*nx+(iz+-1)*ny*nx]
+xv[(ix+1)+(iy+1)*nx+(iz+0)*ny*nx]
)/diagonal_element;}
else if (iy == 0 && ix>0 && ix<nx-1 && iz>0 && iz<nz-1){
xv[ix+iy*nx+iz*ny*nx] = (rv[ix+iy*nx+iz*ny*nx]
+xv[(ix+-1)+(iy+0)*nx+(iz+-1)*ny*nx]
+xv[(ix+-1)+(iy+1)*nx+(iz+-1)*ny*nx]
+xv[(ix+-1)+(iy+0)*nx+(iz+0)*ny*nx]
+xv[(ix+-1)+(iy+1)*nx+(iz+0)*ny*nx]
+xv[(ix+-1)+(iy+0)*nx+(iz+1)*ny*nx]
+xv[(ix+-1)+(iy+1)*nx+(iz+1)*ny*nx]
+xv[(ix+0)+(iy+0)*nx+(iz+-1)*ny*nx]
+xv[(ix+0)+(iy+1)*nx+(iz+-1)*ny*nx]
+xv[(ix+0)+(iy+1)*nx+(iz+0)*ny*nx]
+xv[(ix+0)+(iy+0)*nx+(iz+1)*ny*nx]
+xv[(ix+0)+(iy+1)*nx+(iz+1)*ny*nx]
+xv[(ix+1)+(iy+0)*nx+(iz+-1)*ny*nx]
+xv[(ix+1)+(iy+1)*nx+(iz+-1)*ny*nx]
+xv[(ix+1)+(iy+0)*nx+(iz+0)*ny*nx]
+xv[(ix+1)+(iy+1)*nx+(iz+0)*ny*nx]
+xv[(ix+1)+(iy+0)*nx+(iz+1)*ny*nx]
+xv[(ix+1)+(iy+1)*nx+(iz+1)*ny*nx]
)/diagonal_element;}
else if (iy == ny-1 && ix>0 && ix<nx-1 && iz>0 && iz<nz-1){
xv[ix+iy*nx+iz*ny*nx] = (rv[ix+iy*nx+iz*ny*nx]
+xv[(ix+-1)+(iy+-1)*nx+(iz+-1)*ny*nx]
+xv[(ix+-1)+(iy+0)*nx+(iz+-1)*ny*nx]
+xv[(ix+-1)+(iy+-1)*nx+(iz+0)*ny*nx]
+xv[(ix+-1)+(iy+0)*nx+(iz+0)*ny*nx]
+xv[(ix+-1)+(iy+-1)*nx+(iz+1)*ny*nx]
+xv[(ix+-1)+(iy+0)*nx+(iz+1)*ny*nx]
+xv[(ix+0)+(iy+-1)*nx+(iz+-1)*ny*nx]
+xv[(ix+0)+(iy+0)*nx+(iz+-1)*ny*nx]
+xv[(ix+0)+(iy+-1)*nx+(iz+0)*ny*nx]
+xv[(ix+0)+(iy+-1)*nx+(iz+1)*ny*nx]
+xv[(ix+0)+(iy+0)*nx+(iz+1)*ny*nx]
+xv[(ix+1)+(iy+-1)*nx+(iz+-1)*ny*nx]
+xv[(ix+1)+(iy+0)*nx+(iz+-1)*ny*nx]
+xv[(ix+1)+(iy+-1)*nx+(iz+0)*ny*nx]
+xv[(ix+1)+(iy+0)*nx+(iz+0)*ny*nx]
+xv[(ix+1)+(iy+-1)*nx+(iz+1)*ny*nx]
+xv[(ix+1)+(iy+0)*nx+(iz+1)*ny*nx]
)/diagonal_element;}
else if (ix == 0 && iy>0 && iy<ny-1 && iz>0 && iz<nz-1){
xv[ix+iy*nx+iz*ny*nx] = (rv[ix+iy*nx+iz*ny*nx]
+xv[(ix+0)+(iy+-1)*nx+(iz+-1)*ny*nx]
+xv[(ix+1)+(iy+-1)*nx+(iz+-1)*ny*nx]
+xv[(ix+0)+(iy+-1)*nx+(iz+0)*ny*nx]
+xv[(ix+1)+(iy+-1)*nx+(iz+0)*ny*nx]
+xv[(ix+0)+(iy+-1)*nx+(iz+1)*ny*nx]
+xv[(ix+1)+(iy+-1)*nx+(iz+1)*ny*nx]
+xv[(ix+0)+(iy+0)*nx+(iz+-1)*ny*nx]
+xv[(ix+1)+(iy+0)*nx+(iz+-1)*ny*nx]
+xv[(ix+1)+(iy+0)*nx+(iz+0)*ny*nx]
+xv[(ix+0)+(iy+0)*nx+(iz+1)*ny*nx]
+xv[(ix+1)+(iy+0)*nx+(iz+1)*ny*nx]
+xv[(ix+0)+(iy+1)*nx+(iz+-1)*ny*nx]
+xv[(ix+1)+(iy+1)*nx+(iz+-1)*ny*nx]
+xv[(ix+0)+(iy+1)*nx+(iz+0)*ny*nx]
+xv[(ix+1)+(iy+1)*nx+(iz+0)*ny*nx]
+xv[(ix+0)+(iy+1)*nx+(iz+1)*ny*nx]
+xv[(ix+1)+(iy+1)*nx+(iz+1)*ny*nx]
)/diagonal_element;}
else if (ix == nx-1 && iy>0 && iy<ny-1 && iz>0 && iz<nz-1){
xv[ix+iy*nx+iz*ny*nx] = (rv[ix+iy*nx+iz*ny*nx]
+xv[(ix+-1)+(iy+-1)*nx+(iz+-1)*ny*nx]
+xv[(ix+0)+(iy+-1)*nx+(iz+-1)*ny*nx]
+xv[(ix+-1)+(iy+-1)*nx+(iz+0)*ny*nx]
+xv[(ix+0)+(iy+-1)*nx+(iz+0)*ny*nx]
+xv[(ix+-1)+(iy+-1)*nx+(iz+1)*ny*nx]
+xv[(ix+0)+(iy+-1)*nx+(iz+1)*ny*nx]
+xv[(ix+-1)+(iy+0)*nx+(iz+-1)*ny*nx]
+xv[(ix+0)+(iy+0)*nx+(iz+-1)*ny*nx]
+xv[(ix+-1)+(iy+0)*nx+(iz+0)*ny*nx]
+xv[(ix+-1)+(iy+0)*nx+(iz+1)*ny*nx]
+xv[(ix+0)+(iy+0)*nx+(iz+1)*ny*nx]
+xv[(ix+-1)+(iy+1)*nx+(iz+-1)*ny*nx]
+xv[(ix+0)+(iy+1)*nx+(iz+-1)*ny*nx]
+xv[(ix+-1)+(iy+1)*nx+(iz+0)*ny*nx]
+xv[(ix+0)+(iy+1)*nx+(iz+0)*ny*nx]
+xv[(ix+-1)+(iy+1)*nx+(iz+1)*ny*nx]
+xv[(ix+0)+(iy+1)*nx+(iz+1)*ny*nx]
)/diagonal_element;}

//bulk
else{
xv[ix+iy*nx+iz*ny*nx] = (rv[ix+iy*nx+iz*ny*nx]
+ xv[ix+-1 + (iy+-1)*nx + (iz+-1)*nx*ny]
+ xv[ix+-1 + (iy+-1)*nx + (iz+0)*nx*ny]
+ xv[ix+-1 + (iy+-1)*nx + (iz+1)*nx*ny]
+ xv[ix+-1 + (iy+0)*nx + (iz+-1)*nx*ny]
+ xv[ix+-1 + (iy+0)*nx + (iz+0)*nx*ny]
+ xv[ix+-1 + (iy+0)*nx + (iz+1)*nx*ny]
+ xv[ix+-1 + (iy+1)*nx + (iz+-1)*nx*ny]
+ xv[ix+-1 + (iy+1)*nx + (iz+0)*nx*ny]
+ xv[ix+-1 + (iy+1)*nx + (iz+1)*nx*ny]
+ xv[ix+0 + (iy+-1)*nx + (iz+-1)*nx*ny]
+ xv[ix+0 + (iy+-1)*nx + (iz+0)*nx*ny]
+ xv[ix+0 + (iy+-1)*nx + (iz+1)*nx*ny]
+ xv[ix+0 + (iy+0)*nx + (iz+-1)*nx*ny]
+ xv[ix+0 + (iy+0)*nx + (iz+1)*nx*ny]
+ xv[ix+0 + (iy+1)*nx + (iz+-1)*nx*ny]
+ xv[ix+0 + (iy+1)*nx + (iz+0)*nx*ny]
+ xv[ix+0 + (iy+1)*nx + (iz+1)*nx*ny]
+ xv[ix+1 + (iy+-1)*nx + (iz+-1)*nx*ny]
+ xv[ix+1 + (iy+-1)*nx + (iz+0)*nx*ny]
+ xv[ix+1 + (iy+-1)*nx + (iz+1)*nx*ny]
+ xv[ix+1 + (iy+0)*nx + (iz+-1)*nx*ny]
+ xv[ix+1 + (iy+0)*nx + (iz+0)*nx*ny]
+ xv[ix+1 + (iy+0)*nx + (iz+1)*nx*ny]
+ xv[ix+1 + (iy+1)*nx + (iz+-1)*nx*ny]
+ xv[ix+1 + (iy+1)*nx + (iz+0)*nx*ny]
+ xv[ix+1 + (iy+1)*nx + (iz+1)*nx*ny]
)/diagonal_element;}


        }
      }
    }

// barwards sweep
for( iz=nz-1; iz>=0; iz--){
    for( iy=ny-1; iy>=0; iy--){
        for (ix=nx-1; ix>=0; ix--){

if(ix == 0&& iy == 0&& iz == 0){
xv[ix+iy*nx+iz*ny*nx] = (rv[ix+iy*nx+iz*ny*nx]
+xv[(ix+0)+(iy+0)*nx+(iz+1)*ny*nx]
+xv[(ix+0)+(iy+1)*nx+(iz+0)*ny*nx]
+xv[(ix+0)+(iy+1)*nx+(iz+1)*ny*nx]
+xv[(ix+1)+(iy+0)*nx+(iz+0)*ny*nx]
+xv[(ix+1)+(iy+0)*nx+(iz+1)*ny*nx]
+xv[(ix+1)+(iy+1)*nx+(iz+0)*ny*nx]
+xv[(ix+1)+(iy+1)*nx+(iz+1)*ny*nx]
)/diagonal_element;}
else if(ix == 0&& iy == 0&& iz == nz-1){
xv[ix+iy*nx+iz*ny*nx] = (rv[ix+iy*nx+iz*ny*nx]
+xv[(ix+0)+(iy+0)*nx+(iz+-1)*ny*nx]
+xv[(ix+0)+(iy+1)*nx+(iz+-1)*ny*nx]
+xv[(ix+0)+(iy+1)*nx+(iz+0)*ny*nx]
+xv[(ix+1)+(iy+0)*nx+(iz+-1)*ny*nx]
+xv[(ix+1)+(iy+0)*nx+(iz+0)*ny*nx]
+xv[(ix+1)+(iy+1)*nx+(iz+-1)*ny*nx]
+xv[(ix+1)+(iy+1)*nx+(iz+0)*ny*nx]
)/diagonal_element;}
else if(ix == 0&& iy == ny-1&& iz == 0){
xv[ix+iy*nx+iz*ny*nx] = (rv[ix+iy*nx+iz*ny*nx]
+xv[(ix+0)+(iy+-1)*nx+(iz+0)*ny*nx]
+xv[(ix+0)+(iy+-1)*nx+(iz+1)*ny*nx]
+xv[(ix+0)+(iy+0)*nx+(iz+1)*ny*nx]
+xv[(ix+1)+(iy+-1)*nx+(iz+0)*ny*nx]
+xv[(ix+1)+(iy+-1)*nx+(iz+1)*ny*nx]
+xv[(ix+1)+(iy+0)*nx+(iz+0)*ny*nx]
+xv[(ix+1)+(iy+0)*nx+(iz+1)*ny*nx]
)/diagonal_element;}
else if(ix == 0&& iy == ny-1&& iz == nz-1){
xv[ix+iy*nx+iz*ny*nx] = (rv[ix+iy*nx+iz*ny*nx]
+xv[(ix+0)+(iy+-1)*nx+(iz+-1)*ny*nx]
+xv[(ix+0)+(iy+-1)*nx+(iz+0)*ny*nx]
+xv[(ix+0)+(iy+0)*nx+(iz+-1)*ny*nx]
+xv[(ix+1)+(iy+-1)*nx+(iz+-1)*ny*nx]
+xv[(ix+1)+(iy+-1)*nx+(iz+0)*ny*nx]
+xv[(ix+1)+(iy+0)*nx+(iz+-1)*ny*nx]
+xv[(ix+1)+(iy+0)*nx+(iz+0)*ny*nx]
)/diagonal_element;}
else if(ix == nx-1&& iy == 0&& iz == 0){
xv[ix+iy*nx+iz*ny*nx] = (rv[ix+iy*nx+iz*ny*nx]
+xv[(ix+-1)+(iy+0)*nx+(iz+0)*ny*nx]
+xv[(ix+-1)+(iy+0)*nx+(iz+1)*ny*nx]
+xv[(ix+-1)+(iy+1)*nx+(iz+0)*ny*nx]
+xv[(ix+-1)+(iy+1)*nx+(iz+1)*ny*nx]
+xv[(ix+0)+(iy+0)*nx+(iz+1)*ny*nx]
+xv[(ix+0)+(iy+1)*nx+(iz+0)*ny*nx]
+xv[(ix+0)+(iy+1)*nx+(iz+1)*ny*nx]
)/diagonal_element;}
else if(ix == nx-1&& iy == 0&& iz == nz-1){
xv[ix+iy*nx+iz*ny*nx] = (rv[ix+iy*nx+iz*ny*nx]
+xv[(ix+-1)+(iy+0)*nx+(iz+-1)*ny*nx]
+xv[(ix+-1)+(iy+0)*nx+(iz+0)*ny*nx]
+xv[(ix+-1)+(iy+1)*nx+(iz+-1)*ny*nx]
+xv[(ix+-1)+(iy+1)*nx+(iz+0)*ny*nx]
+xv[(ix+0)+(iy+0)*nx+(iz+-1)*ny*nx]
+xv[(ix+0)+(iy+1)*nx+(iz+-1)*ny*nx]
+xv[(ix+0)+(iy+1)*nx+(iz+0)*ny*nx]
)/diagonal_element;}
else if(ix == nx-1&& iy == ny-1&& iz == 0){
xv[ix+iy*nx+iz*ny*nx] = (rv[ix+iy*nx+iz*ny*nx]
+xv[(ix+-1)+(iy+-1)*nx+(iz+0)*ny*nx]
+xv[(ix+-1)+(iy+-1)*nx+(iz+1)*ny*nx]
+xv[(ix+-1)+(iy+0)*nx+(iz+0)*ny*nx]
+xv[(ix+-1)+(iy+0)*nx+(iz+1)*ny*nx]
+xv[(ix+0)+(iy+-1)*nx+(iz+0)*ny*nx]
+xv[(ix+0)+(iy+-1)*nx+(iz+1)*ny*nx]
+xv[(ix+0)+(iy+0)*nx+(iz+1)*ny*nx]
)/diagonal_element;}
else if(ix == nx-1&& iy == ny-1&& iz == nz-1){
xv[ix+iy*nx+iz*ny*nx] = (rv[ix+iy*nx+iz*ny*nx]
+xv[(ix+-1)+(iy+-1)*nx+(iz+-1)*ny*nx]
+xv[(ix+-1)+(iy+-1)*nx+(iz+0)*ny*nx]
+xv[(ix+-1)+(iy+0)*nx+(iz+-1)*ny*nx]
+xv[(ix+-1)+(iy+0)*nx+(iz+0)*ny*nx]
+xv[(ix+0)+(iy+-1)*nx+(iz+-1)*ny*nx]
+xv[(ix+0)+(iy+-1)*nx+(iz+0)*ny*nx]
+xv[(ix+0)+(iy+0)*nx+(iz+-1)*ny*nx]
)/diagonal_element;}

//edges
else if (ix == 0 && iy == 0 &&iz>0 && iz<nz-1){
xv[ix+iy*nx+iz*ny*nx] = (rv[ix+iy*nx+iz*ny*nx]
+xv[(ix+0)+(iy+0)*nx+(iz+-1)*ny*nx]
+xv[(ix+0)+(iy+1)*nx+(iz+-1)*ny*nx]
+xv[(ix+1)+(iy+0)*nx+(iz+-1)*ny*nx]
+xv[(ix+1)+(iy+1)*nx+(iz+-1)*ny*nx]
+xv[(ix+0)+(iy+1)*nx+(iz+0)*ny*nx]
+xv[(ix+1)+(iy+0)*nx+(iz+0)*ny*nx]
+xv[(ix+1)+(iy+1)*nx+(iz+0)*ny*nx]
+xv[(ix+0)+(iy+0)*nx+(iz+1)*ny*nx]
+xv[(ix+0)+(iy+1)*nx+(iz+1)*ny*nx]
+xv[(ix+1)+(iy+0)*nx+(iz+1)*ny*nx]
+xv[(ix+1)+(iy+1)*nx+(iz+1)*ny*nx]
)/diagonal_element;}
else if (ix == 0 && iy == ny-1 &&iz>0 && iz<nz-1){
xv[ix+iy*nx+iz*ny*nx] = (rv[ix+iy*nx+iz*ny*nx]
+xv[(ix+0)+(iy+-1)*nx+(iz+-1)*ny*nx]
+xv[(ix+0)+(iy+0)*nx+(iz+-1)*ny*nx]
+xv[(ix+1)+(iy+-1)*nx+(iz+-1)*ny*nx]
+xv[(ix+1)+(iy+0)*nx+(iz+-1)*ny*nx]
+xv[(ix+0)+(iy+-1)*nx+(iz+0)*ny*nx]
+xv[(ix+1)+(iy+-1)*nx+(iz+0)*ny*nx]
+xv[(ix+1)+(iy+0)*nx+(iz+0)*ny*nx]
+xv[(ix+0)+(iy+-1)*nx+(iz+1)*ny*nx]
+xv[(ix+0)+(iy+0)*nx+(iz+1)*ny*nx]
+xv[(ix+1)+(iy+-1)*nx+(iz+1)*ny*nx]
+xv[(ix+1)+(iy+0)*nx+(iz+1)*ny*nx]
)/diagonal_element;}
else if (ix == nx-1 && iy == 0 && iz>0 && iz<nz-1){
xv[ix+iy*nx+iz*ny*nx] = (rv[ix+iy*nx+iz*ny*nx]
+xv[(ix+-1)+(iy+0)*nx+(iz+-1)*ny*nx]
+xv[(ix+-1)+(iy+1)*nx+(iz+-1)*ny*nx]
+xv[(ix+0)+(iy+0)*nx+(iz+-1)*ny*nx]
+xv[(ix+0)+(iy+1)*nx+(iz+-1)*ny*nx]
+xv[(ix+-1)+(iy+0)*nx+(iz+0)*ny*nx]
+xv[(ix+-1)+(iy+1)*nx+(iz+0)*ny*nx]
+xv[(ix+0)+(iy+1)*nx+(iz+0)*ny*nx]
+xv[(ix+-1)+(iy+0)*nx+(iz+1)*ny*nx]
+xv[(ix+-1)+(iy+1)*nx+(iz+1)*ny*nx]
+xv[(ix+0)+(iy+0)*nx+(iz+1)*ny*nx]
+xv[(ix+0)+(iy+1)*nx+(iz+1)*ny*nx]
)/diagonal_element;}
else if (ix == nx-1 && iy == ny-1 &&iz>0 && iz<nz-1){
xv[ix+iy*nx+iz*ny*nx] = (rv[ix+iy*nx+iz*ny*nx]
+xv[(ix+-1)+(iy+-1)*nx+(iz+-1)*ny*nx]
+xv[(ix+-1)+(iy+0)*nx+(iz+-1)*ny*nx]
+xv[(ix+0)+(iy+-1)*nx+(iz+-1)*ny*nx]
+xv[(ix+0)+(iy+0)*nx+(iz+-1)*ny*nx]
+xv[(ix+-1)+(iy+-1)*nx+(iz+0)*ny*nx]
+xv[(ix+-1)+(iy+0)*nx+(iz+0)*ny*nx]
+xv[(ix+0)+(iy+-1)*nx+(iz+0)*ny*nx]
+xv[(ix+-1)+(iy+-1)*nx+(iz+1)*ny*nx]
+xv[(ix+-1)+(iy+0)*nx+(iz+1)*ny*nx]
+xv[(ix+0)+(iy+-1)*nx+(iz+1)*ny*nx]
+xv[(ix+0)+(iy+0)*nx+(iz+1)*ny*nx]
)/diagonal_element;}
else if (ix == 0 && iz == 0 &&iy>0 && iy<ny-1){
xv[ix+iy*nx+iz*ny*nx] = (rv[ix+iy*nx+iz*ny*nx]
+xv[(ix+0)+(iy+-1)*nx+(iz+0)*ny*nx]
+xv[(ix+0)+(iy+-1)*nx+(iz+1)*ny*nx]
+xv[(ix+1)+(iy+-1)*nx+(iz+0)*ny*nx]
+xv[(ix+1)+(iy+-1)*nx+(iz+1)*ny*nx]
+xv[(ix+0)+(iy+0)*nx+(iz+1)*ny*nx]
+xv[(ix+1)+(iy+0)*nx+(iz+0)*ny*nx]
+xv[(ix+1)+(iy+0)*nx+(iz+1)*ny*nx]
+xv[(ix+0)+(iy+1)*nx+(iz+0)*ny*nx]
+xv[(ix+0)+(iy+1)*nx+(iz+1)*ny*nx]
+xv[(ix+1)+(iy+1)*nx+(iz+0)*ny*nx]
+xv[(ix+1)+(iy+1)*nx+(iz+1)*ny*nx]
)/diagonal_element;}
else if (ix == 0 && iz == nz-1 &&iy>0 && iy<ny-1){
xv[ix+iy*nx+iz*ny*nx] = (rv[ix+iy*nx+iz*ny*nx]
+xv[(ix+0)+(iy+-1)*nx+(iz+-1)*ny*nx]
+xv[(ix+0)+(iy+-1)*nx+(iz+0)*ny*nx]
+xv[(ix+1)+(iy+-1)*nx+(iz+-1)*ny*nx]
+xv[(ix+1)+(iy+-1)*nx+(iz+0)*ny*nx]
+xv[(ix+0)+(iy+0)*nx+(iz+-1)*ny*nx]
+xv[(ix+1)+(iy+0)*nx+(iz+-1)*ny*nx]
+xv[(ix+1)+(iy+0)*nx+(iz+0)*ny*nx]
+xv[(ix+0)+(iy+1)*nx+(iz+-1)*ny*nx]
+xv[(ix+0)+(iy+1)*nx+(iz+0)*ny*nx]
+xv[(ix+1)+(iy+1)*nx+(iz+-1)*ny*nx]
+xv[(ix+1)+(iy+1)*nx+(iz+0)*ny*nx]
)/diagonal_element;}
else if (ix == nx-1 && iz == 0 && iy>0 && iy<ny-1){
xv[ix+iy*nx+iz*ny*nx] = (rv[ix+iy*nx+iz*ny*nx]
+xv[(ix+-1)+(iy+-1)*nx+(iz+0)*ny*nx]
+xv[(ix+-1)+(iy+-1)*nx+(iz+1)*ny*nx]
+xv[(ix+0)+(iy+-1)*nx+(iz+0)*ny*nx]
+xv[(ix+0)+(iy+-1)*nx+(iz+1)*ny*nx]
+xv[(ix+-1)+(iy+0)*nx+(iz+0)*ny*nx]
+xv[(ix+-1)+(iy+0)*nx+(iz+1)*ny*nx]
+xv[(ix+0)+(iy+0)*nx+(iz+1)*ny*nx]
+xv[(ix+-1)+(iy+1)*nx+(iz+0)*ny*nx]
+xv[(ix+-1)+(iy+1)*nx+(iz+1)*ny*nx]
+xv[(ix+0)+(iy+1)*nx+(iz+0)*ny*nx]
+xv[(ix+0)+(iy+1)*nx+(iz+1)*ny*nx]
)/diagonal_element;}
else if (ix == nx-1 && iz == nz-1 &&iy>0 && iy<ny-1){
xv[ix+iy*nx+iz*ny*nx] = (rv[ix+iy*nx+iz*ny*nx]
+xv[(ix+-1)+(iy+-1)*nx+(iz+-1)*ny*nx]
+xv[(ix+-1)+(iy+-1)*nx+(iz+0)*ny*nx]
+xv[(ix+0)+(iy+-1)*nx+(iz+-1)*ny*nx]
+xv[(ix+0)+(iy+-1)*nx+(iz+0)*ny*nx]
+xv[(ix+-1)+(iy+0)*nx+(iz+-1)*ny*nx]
+xv[(ix+-1)+(iy+0)*nx+(iz+0)*ny*nx]
+xv[(ix+0)+(iy+0)*nx+(iz+-1)*ny*nx]
+xv[(ix+-1)+(iy+1)*nx+(iz+-1)*ny*nx]
+xv[(ix+-1)+(iy+1)*nx+(iz+0)*ny*nx]
+xv[(ix+0)+(iy+1)*nx+(iz+-1)*ny*nx]
+xv[(ix+0)+(iy+1)*nx+(iz+0)*ny*nx]
)/diagonal_element;}
else if (iy == 0 && iz == 0 &&ix>0 && ix<nx-1){
xv[ix+iy*nx+iz*ny*nx] = (rv[ix+iy*nx+iz*ny*nx]
+xv[(ix+-1)+(iy+0)*nx+(iz+0)*ny*nx]
+xv[(ix+-1)+(iy+0)*nx+(iz+1)*ny*nx]
+xv[(ix+-1)+(iy+1)*nx+(iz+0)*ny*nx]
+xv[(ix+-1)+(iy+1)*nx+(iz+1)*ny*nx]
+xv[(ix+0)+(iy+0)*nx+(iz+1)*ny*nx]
+xv[(ix+0)+(iy+1)*nx+(iz+0)*ny*nx]
+xv[(ix+0)+(iy+1)*nx+(iz+1)*ny*nx]
+xv[(ix+1)+(iy+0)*nx+(iz+0)*ny*nx]
+xv[(ix+1)+(iy+0)*nx+(iz+1)*ny*nx]
+xv[(ix+1)+(iy+1)*nx+(iz+0)*ny*nx]
+xv[(ix+1)+(iy+1)*nx+(iz+1)*ny*nx]
)/diagonal_element;}
else if (iy == 0 && iz == nz-1 &&ix>0 && ix<nx-1){
xv[ix+iy*nx+iz*ny*nx] = (rv[ix+iy*nx+iz*ny*nx]
+xv[(ix+-1)+(iy+0)*nx+(iz+-1)*ny*nx]
+xv[(ix+-1)+(iy+0)*nx+(iz+0)*ny*nx]
+xv[(ix+-1)+(iy+1)*nx+(iz+-1)*ny*nx]
+xv[(ix+-1)+(iy+1)*nx+(iz+0)*ny*nx]
+xv[(ix+0)+(iy+0)*nx+(iz+-1)*ny*nx]
+xv[(ix+0)+(iy+1)*nx+(iz+-1)*ny*nx]
+xv[(ix+0)+(iy+1)*nx+(iz+0)*ny*nx]
+xv[(ix+1)+(iy+0)*nx+(iz+-1)*ny*nx]
+xv[(ix+1)+(iy+0)*nx+(iz+0)*ny*nx]
+xv[(ix+1)+(iy+1)*nx+(iz+-1)*ny*nx]
+xv[(ix+1)+(iy+1)*nx+(iz+0)*ny*nx]
)/diagonal_element;}
else if (iy == ny-1 && iz == 0 && ix>0 && ix<nx-1){
xv[ix+iy*nx+iz*ny*nx] = (rv[ix+iy*nx+iz*ny*nx]
+xv[(ix+-1)+(iy+-1)*nx+(iz+0)*ny*nx]
+xv[(ix+-1)+(iy+-1)*nx+(iz+1)*ny*nx]
+xv[(ix+-1)+(iy+0)*nx+(iz+0)*ny*nx]
+xv[(ix+-1)+(iy+0)*nx+(iz+1)*ny*nx]
+xv[(ix+0)+(iy+-1)*nx+(iz+0)*ny*nx]
+xv[(ix+0)+(iy+-1)*nx+(iz+1)*ny*nx]
+xv[(ix+0)+(iy+0)*nx+(iz+1)*ny*nx]
+xv[(ix+1)+(iy+-1)*nx+(iz+0)*ny*nx]
+xv[(ix+1)+(iy+-1)*nx+(iz+1)*ny*nx]
+xv[(ix+1)+(iy+0)*nx+(iz+0)*ny*nx]
+xv[(ix+1)+(iy+0)*nx+(iz+1)*ny*nx]
)/diagonal_element;}
else if (iy == ny-1 && iz == nz-1 &&ix>0 && ix<nx-1){
xv[ix+iy*nx+iz*ny*nx] = (rv[ix+iy*nx+iz*ny*nx]
+xv[(ix+-1)+(iy+-1)*nx+(iz+-1)*ny*nx]
+xv[(ix+-1)+(iy+-1)*nx+(iz+0)*ny*nx]
+xv[(ix+-1)+(iy+0)*nx+(iz+-1)*ny*nx]
+xv[(ix+-1)+(iy+0)*nx+(iz+0)*ny*nx]
+xv[(ix+0)+(iy+-1)*nx+(iz+-1)*ny*nx]
+xv[(ix+0)+(iy+-1)*nx+(iz+0)*ny*nx]
+xv[(ix+0)+(iy+0)*nx+(iz+-1)*ny*nx]
+xv[(ix+1)+(iy+-1)*nx+(iz+-1)*ny*nx]
+xv[(ix+1)+(iy+-1)*nx+(iz+0)*ny*nx]
+xv[(ix+1)+(iy+0)*nx+(iz+-1)*ny*nx]
+xv[(ix+1)+(iy+0)*nx+(iz+0)*ny*nx]
)/diagonal_element;}

//sides
else if (iz == 0 && ix>0 && ix<nx-1 && iy>0 && iy<ny-1){
xv[ix+iy*nx+iz*ny*nx] = (rv[ix+iy*nx+iz*ny*nx]
+xv[(ix+-1)+(iy+-1)*nx+(iz+0)*ny*nx]
+xv[(ix+-1)+(iy+-1)*nx+(iz+1)*ny*nx]
+xv[(ix+-1)+(iy+0)*nx+(iz+0)*ny*nx]
+xv[(ix+-1)+(iy+0)*nx+(iz+1)*ny*nx]
+xv[(ix+-1)+(iy+1)*nx+(iz+0)*ny*nx]
+xv[(ix+-1)+(iy+1)*nx+(iz+1)*ny*nx]
+xv[(ix+0)+(iy+-1)*nx+(iz+0)*ny*nx]
+xv[(ix+0)+(iy+-1)*nx+(iz+1)*ny*nx]
+xv[(ix+0)+(iy+0)*nx+(iz+1)*ny*nx]
+xv[(ix+0)+(iy+1)*nx+(iz+0)*ny*nx]
+xv[(ix+0)+(iy+1)*nx+(iz+1)*ny*nx]
+xv[(ix+1)+(iy+-1)*nx+(iz+0)*ny*nx]
+xv[(ix+1)+(iy+-1)*nx+(iz+1)*ny*nx]
+xv[(ix+1)+(iy+0)*nx+(iz+0)*ny*nx]
+xv[(ix+1)+(iy+0)*nx+(iz+1)*ny*nx]
+xv[(ix+1)+(iy+1)*nx+(iz+0)*ny*nx]
+xv[(ix+1)+(iy+1)*nx+(iz+1)*ny*nx]
)/diagonal_element;}
else if (iz == nz-1 && ix>0 && ix<nx-1 && iy>0 && iy<ny-1){
xv[ix+iy*nx+iz*ny*nx] = (rv[ix+iy*nx+iz*ny*nx]
+xv[(ix+-1)+(iy+-1)*nx+(iz+-1)*ny*nx]
+xv[(ix+-1)+(iy+-1)*nx+(iz+0)*ny*nx]
+xv[(ix+-1)+(iy+0)*nx+(iz+-1)*ny*nx]
+xv[(ix+-1)+(iy+0)*nx+(iz+0)*ny*nx]
+xv[(ix+-1)+(iy+1)*nx+(iz+-1)*ny*nx]
+xv[(ix+-1)+(iy+1)*nx+(iz+0)*ny*nx]
+xv[(ix+0)+(iy+-1)*nx+(iz+-1)*ny*nx]
+xv[(ix+0)+(iy+-1)*nx+(iz+0)*ny*nx]
+xv[(ix+0)+(iy+0)*nx+(iz+-1)*ny*nx]
+xv[(ix+0)+(iy+1)*nx+(iz+-1)*ny*nx]
+xv[(ix+0)+(iy+1)*nx+(iz+0)*ny*nx]
+xv[(ix+1)+(iy+-1)*nx+(iz+-1)*ny*nx]
+xv[(ix+1)+(iy+-1)*nx+(iz+0)*ny*nx]
+xv[(ix+1)+(iy+0)*nx+(iz+-1)*ny*nx]
+xv[(ix+1)+(iy+0)*nx+(iz+0)*ny*nx]
+xv[(ix+1)+(iy+1)*nx+(iz+-1)*ny*nx]
+xv[(ix+1)+(iy+1)*nx+(iz+0)*ny*nx]
)/diagonal_element;}
else if (iy == 0 && ix>0 && ix<nx-1 && iz>0 && iz<nz-1){
xv[ix+iy*nx+iz*ny*nx] = (rv[ix+iy*nx+iz*ny*nx]
+xv[(ix+-1)+(iy+0)*nx+(iz+-1)*ny*nx]
+xv[(ix+-1)+(iy+1)*nx+(iz+-1)*ny*nx]
+xv[(ix+-1)+(iy+0)*nx+(iz+0)*ny*nx]
+xv[(ix+-1)+(iy+1)*nx+(iz+0)*ny*nx]
+xv[(ix+-1)+(iy+0)*nx+(iz+1)*ny*nx]
+xv[(ix+-1)+(iy+1)*nx+(iz+1)*ny*nx]
+xv[(ix+0)+(iy+0)*nx+(iz+-1)*ny*nx]
+xv[(ix+0)+(iy+1)*nx+(iz+-1)*ny*nx]
+xv[(ix+0)+(iy+1)*nx+(iz+0)*ny*nx]
+xv[(ix+0)+(iy+0)*nx+(iz+1)*ny*nx]
+xv[(ix+0)+(iy+1)*nx+(iz+1)*ny*nx]
+xv[(ix+1)+(iy+0)*nx+(iz+-1)*ny*nx]
+xv[(ix+1)+(iy+1)*nx+(iz+-1)*ny*nx]
+xv[(ix+1)+(iy+0)*nx+(iz+0)*ny*nx]
+xv[(ix+1)+(iy+1)*nx+(iz+0)*ny*nx]
+xv[(ix+1)+(iy+0)*nx+(iz+1)*ny*nx]
+xv[(ix+1)+(iy+1)*nx+(iz+1)*ny*nx]
)/diagonal_element;}
else if (iy == ny-1 && ix>0 && ix<nx-1 && iz>0 && iz<nz-1){
xv[ix+iy*nx+iz*ny*nx] = (rv[ix+iy*nx+iz*ny*nx]
+xv[(ix+-1)+(iy+-1)*nx+(iz+-1)*ny*nx]
+xv[(ix+-1)+(iy+0)*nx+(iz+-1)*ny*nx]
+xv[(ix+-1)+(iy+-1)*nx+(iz+0)*ny*nx]
+xv[(ix+-1)+(iy+0)*nx+(iz+0)*ny*nx]
+xv[(ix+-1)+(iy+-1)*nx+(iz+1)*ny*nx]
+xv[(ix+-1)+(iy+0)*nx+(iz+1)*ny*nx]
+xv[(ix+0)+(iy+-1)*nx+(iz+-1)*ny*nx]
+xv[(ix+0)+(iy+0)*nx+(iz+-1)*ny*nx]
+xv[(ix+0)+(iy+-1)*nx+(iz+0)*ny*nx]
+xv[(ix+0)+(iy+-1)*nx+(iz+1)*ny*nx]
+xv[(ix+0)+(iy+0)*nx+(iz+1)*ny*nx]
+xv[(ix+1)+(iy+-1)*nx+(iz+-1)*ny*nx]
+xv[(ix+1)+(iy+0)*nx+(iz+-1)*ny*nx]
+xv[(ix+1)+(iy+-1)*nx+(iz+0)*ny*nx]
+xv[(ix+1)+(iy+0)*nx+(iz+0)*ny*nx]
+xv[(ix+1)+(iy+-1)*nx+(iz+1)*ny*nx]
+xv[(ix+1)+(iy+0)*nx+(iz+1)*ny*nx]
)/diagonal_element;}
else if (ix == 0 && iy>0 && iy<ny-1 && iz>0 && iz<nz-1){
xv[ix+iy*nx+iz*ny*nx] = (rv[ix+iy*nx+iz*ny*nx]
+xv[(ix+0)+(iy+-1)*nx+(iz+-1)*ny*nx]
+xv[(ix+1)+(iy+-1)*nx+(iz+-1)*ny*nx]
+xv[(ix+0)+(iy+-1)*nx+(iz+0)*ny*nx]
+xv[(ix+1)+(iy+-1)*nx+(iz+0)*ny*nx]
+xv[(ix+0)+(iy+-1)*nx+(iz+1)*ny*nx]
+xv[(ix+1)+(iy+-1)*nx+(iz+1)*ny*nx]
+xv[(ix+0)+(iy+0)*nx+(iz+-1)*ny*nx]
+xv[(ix+1)+(iy+0)*nx+(iz+-1)*ny*nx]
+xv[(ix+1)+(iy+0)*nx+(iz+0)*ny*nx]
+xv[(ix+0)+(iy+0)*nx+(iz+1)*ny*nx]
+xv[(ix+1)+(iy+0)*nx+(iz+1)*ny*nx]
+xv[(ix+0)+(iy+1)*nx+(iz+-1)*ny*nx]
+xv[(ix+1)+(iy+1)*nx+(iz+-1)*ny*nx]
+xv[(ix+0)+(iy+1)*nx+(iz+0)*ny*nx]
+xv[(ix+1)+(iy+1)*nx+(iz+0)*ny*nx]
+xv[(ix+0)+(iy+1)*nx+(iz+1)*ny*nx]
+xv[(ix+1)+(iy+1)*nx+(iz+1)*ny*nx]
)/diagonal_element;}
else if (ix == nx-1 && iy>0 && iy<ny-1 && iz>0 && iz<nz-1){
xv[ix+iy*nx+iz*ny*nx] = (rv[ix+iy*nx+iz*ny*nx]
+xv[(ix+-1)+(iy+-1)*nx+(iz+-1)*ny*nx]
+xv[(ix+0)+(iy+-1)*nx+(iz+-1)*ny*nx]
+xv[(ix+-1)+(iy+-1)*nx+(iz+0)*ny*nx]
+xv[(ix+0)+(iy+-1)*nx+(iz+0)*ny*nx]
+xv[(ix+-1)+(iy+-1)*nx+(iz+1)*ny*nx]
+xv[(ix+0)+(iy+-1)*nx+(iz+1)*ny*nx]
+xv[(ix+-1)+(iy+0)*nx+(iz+-1)*ny*nx]
+xv[(ix+0)+(iy+0)*nx+(iz+-1)*ny*nx]
+xv[(ix+-1)+(iy+0)*nx+(iz+0)*ny*nx]
+xv[(ix+-1)+(iy+0)*nx+(iz+1)*ny*nx]
+xv[(ix+0)+(iy+0)*nx+(iz+1)*ny*nx]
+xv[(ix+-1)+(iy+1)*nx+(iz+-1)*ny*nx]
+xv[(ix+0)+(iy+1)*nx+(iz+-1)*ny*nx]
+xv[(ix+-1)+(iy+1)*nx+(iz+0)*ny*nx]
+xv[(ix+0)+(iy+1)*nx+(iz+0)*ny*nx]
+xv[(ix+-1)+(iy+1)*nx+(iz+1)*ny*nx]
+xv[(ix+0)+(iy+1)*nx+(iz+1)*ny*nx]
)/diagonal_element;}

//bulk
else{
xv[ix+iy*nx+iz*ny*nx] = (rv[ix+iy*nx+iz*ny*nx]
+ xv[ix+-1 + (iy+-1)*nx + (iz+-1)*nx*ny]
+ xv[ix+-1 + (iy+-1)*nx + (iz+0)*nx*ny]
+ xv[ix+-1 + (iy+-1)*nx + (iz+1)*nx*ny]
+ xv[ix+-1 + (iy+0)*nx + (iz+-1)*nx*ny]
+ xv[ix+-1 + (iy+0)*nx + (iz+0)*nx*ny]
+ xv[ix+-1 + (iy+0)*nx + (iz+1)*nx*ny]
+ xv[ix+-1 + (iy+1)*nx + (iz+-1)*nx*ny]
+ xv[ix+-1 + (iy+1)*nx + (iz+0)*nx*ny]
+ xv[ix+-1 + (iy+1)*nx + (iz+1)*nx*ny]
+ xv[ix+0 + (iy+-1)*nx + (iz+-1)*nx*ny]
+ xv[ix+0 + (iy+-1)*nx + (iz+0)*nx*ny]
+ xv[ix+0 + (iy+-1)*nx + (iz+1)*nx*ny]
+ xv[ix+0 + (iy+0)*nx + (iz+-1)*nx*ny]
+ xv[ix+0 + (iy+0)*nx + (iz+1)*nx*ny]
+ xv[ix+0 + (iy+1)*nx + (iz+-1)*nx*ny]
+ xv[ix+0 + (iy+1)*nx + (iz+0)*nx*ny]
+ xv[ix+0 + (iy+1)*nx + (iz+1)*nx*ny]
+ xv[ix+1 + (iy+-1)*nx + (iz+-1)*nx*ny]
+ xv[ix+1 + (iy+-1)*nx + (iz+0)*nx*ny]
+ xv[ix+1 + (iy+-1)*nx + (iz+1)*nx*ny]
+ xv[ix+1 + (iy+0)*nx + (iz+-1)*nx*ny]
+ xv[ix+1 + (iy+0)*nx + (iz+0)*nx*ny]
+ xv[ix+1 + (iy+0)*nx + (iz+1)*nx*ny]
+ xv[ix+1 + (iy+1)*nx + (iz+-1)*nx*ny]
+ xv[ix+1 + (iy+1)*nx + (iz+0)*nx*ny]
+ xv[ix+1 + (iy+1)*nx + (iz+1)*nx*ny]
)/diagonal_element;}


        }
      }
    }


return 0;
}
