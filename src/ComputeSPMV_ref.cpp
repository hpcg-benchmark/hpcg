
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
 @file ComputeSPMV_ref.cpp

 HPCG routine
 */
#include <iostream>

#include "ComputeSPMV_ref.hpp"

#ifndef HPCG_NO_MPI
#include "ExchangeHalo.hpp"
#endif

#ifndef HPCG_NO_OPENMP
#include <omp.h>
#endif
#include <cassert>

/*!
  Routine to compute matrix vector product y = Ax where:
  Precondition: First call exchange_externals to get off-processor values of x

  This is the reference SPMV implementation.  It CANNOT be modified for the
  purposes of this benchmark.

  @param[in]  A the known system matrix
  @param[in]  x the known vector
  @param[out] y the On exit contains the result: Ax.

  @return returns 0 upon success and non-zero otherwise

  @see ComputeSPMV
*/


//add openmp later
int ComputeSPMV_ref( const SparseMatrix & A, Vector & x, Vector & y) {

  //Possibly will pass y > cols - but shouldn't change beyond this point
  assert(x.localLength>=A.localNumberOfColumns); // Test vector lengths
  assert(y.localLength>=A.localNumberOfRows);

#ifndef HPCG_NO_MPI
    ExchangeHalo(A,x);
#endif

  const double * const xv = x.values;
  double * const yv = y.values;

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
//  global_int_t gix0 = A.geom->gix0;
//  global_int_t giy0 = A.geom->giy0;
//  global_int_t giz0 = A.geom->giz0;
//  global_int_t gnx = A.geom->gnx;
//  global_int_t gny = A.geom->gny;
//  global_int_t gnz = A.geom->gnz;
  int npx          = A.geom->npx;
  int npy          = A.geom->npy;
  int npz          = A.geom->npz;
  int ipx          = A.geom->ipx;
  int ipy          = A.geom->ipy;
  int ipz          = A.geom->ipz;

  double diagonal_element = A.matrixDiagonal[0][0];// big assumption?

//  local_int_t x_min = 0;
//  local_int_t x_max = nx;
  //y z
  // use ipx and npx etc rather than gix0+nx=gnx
//  if (gix0 == 0) x_min = 1;
//  if (gix0 + nx == gnx) x_max = nx-1;

  //yv needs nx original and xv needs different
//  if (npx == 1) nx =

  //zero the vector y
//  for (int i = 0; i< y.localLength ; i++) yv[i] = 0.0;
  for (int i = 0; i< nlocal ; i++) yv[i] = 0.0; //Only specifically zero the part this subroutine changes


  //assert rows = nx*... and cols is new_x*new_y etc

  //local dimensions include halo
  //local_int_t nlx = 0;

  assert(npy == 1 && npz==1); //havent done this yet
  if (npx == 1)
  {
     nex = 0;
     ney = 0;
     nez = 0;
//     std::cout << "setting nlx = nx " << std::endl;
  }
  else if (ipx == 0 || ipx==(npx-1))
  {
     // One slab to share
     nex = 1;
     ney = ny;
     nez = nz;
  }
  else
  {
     return 1000;
  }
//  local_int_t nlx = nx; nly = ny;local_int_t nlz = nz; //delete this

  assert(nx*ny*nz + nex*ney*nez == A.localNumberOfColumns);
  assert(nx*ny*nz == A.localNumberOfRows);
//bulk part
for( iz=1; iz< nz-1; iz++){
    for( iy=1; iy< ny-1; iy++){
        for (ix=1; ix < nx-1; ix++){
yv[ix+iy*nx+iz*ny*nx] = diagonal_element * xv[ix+iy*nx+iz*ny*nx]
- xv[ix+-1 + (iy+-1)*nx + (iz+-1)*nx*ny]
- xv[ix+-1 + (iy+-1)*nx + (iz+0)*nx*ny]
- xv[ix+-1 + (iy+-1)*nx + (iz+1)*nx*ny]
- xv[ix+-1 + (iy+0)*nx + (iz+-1)*nx*ny]
- xv[ix+-1 + (iy+0)*nx + (iz+0)*nx*ny]
- xv[ix+-1 + (iy+0)*nx + (iz+1)*nx*ny]
- xv[ix+-1 + (iy+1)*nx + (iz+-1)*nx*ny]
- xv[ix+-1 + (iy+1)*nx + (iz+0)*nx*ny]
- xv[ix+-1 + (iy+1)*nx + (iz+1)*nx*ny]
- xv[ix+0 + (iy+-1)*nx + (iz+-1)*nx*ny]
- xv[ix+0 + (iy+-1)*nx + (iz+0)*nx*ny]
- xv[ix+0 + (iy+-1)*nx + (iz+1)*nx*ny]
- xv[ix+0 + (iy+0)*nx + (iz+-1)*nx*ny]
- xv[ix+0 + (iy+0)*nx + (iz+1)*nx*ny]
- xv[ix+0 + (iy+1)*nx + (iz+-1)*nx*ny]
- xv[ix+0 + (iy+1)*nx + (iz+0)*nx*ny]
- xv[ix+0 + (iy+1)*nx + (iz+1)*nx*ny]
- xv[ix+1 + (iy+-1)*nx + (iz+-1)*nx*ny]
- xv[ix+1 + (iy+-1)*nx + (iz+0)*nx*ny]
- xv[ix+1 + (iy+-1)*nx + (iz+1)*nx*ny]
- xv[ix+1 + (iy+0)*nx + (iz+-1)*nx*ny]
- xv[ix+1 + (iy+0)*nx + (iz+0)*nx*ny]
- xv[ix+1 + (iy+0)*nx + (iz+1)*nx*ny]
- xv[ix+1 + (iy+1)*nx + (iz+-1)*nx*ny]
- xv[ix+1 + (iy+1)*nx + (iz+0)*nx*ny]
- xv[ix+1 + (iy+1)*nx + (iz+1)*nx*ny]
;}
}
}
//sides
iz = 0;
for (ix=1; ix<nx-1;ix++){
    for (iy=1; iy<ny-1;iy++)
{
yv[ix+iy*nx+iz*ny*nx] = diagonal_element * xv[ix+iy*nx+iz*ny*nx]
-xv[(ix+-1)+(iy+-1)*nx+(iz+0)*ny*nx]
-xv[(ix+-1)+(iy+-1)*nx+(iz+1)*ny*nx]
-xv[(ix+-1)+(iy+0)*nx+(iz+0)*ny*nx]
-xv[(ix+-1)+(iy+0)*nx+(iz+1)*ny*nx]
-xv[(ix+-1)+(iy+1)*nx+(iz+0)*ny*nx]
-xv[(ix+-1)+(iy+1)*nx+(iz+1)*ny*nx]
-xv[(ix+0)+(iy+-1)*nx+(iz+0)*ny*nx]
-xv[(ix+0)+(iy+-1)*nx+(iz+1)*ny*nx]
-xv[(ix+0)+(iy+0)*nx+(iz+1)*ny*nx]
-xv[(ix+0)+(iy+1)*nx+(iz+0)*ny*nx]
-xv[(ix+0)+(iy+1)*nx+(iz+1)*ny*nx]
-xv[(ix+1)+(iy+-1)*nx+(iz+0)*ny*nx]
-xv[(ix+1)+(iy+-1)*nx+(iz+1)*ny*nx]
-xv[(ix+1)+(iy+0)*nx+(iz+0)*ny*nx]
-xv[(ix+1)+(iy+0)*nx+(iz+1)*ny*nx]
-xv[(ix+1)+(iy+1)*nx+(iz+0)*ny*nx]
-xv[(ix+1)+(iy+1)*nx+(iz+1)*ny*nx]
;}
}
iz = nz-1;
for (ix=1; ix<nx-1;ix++){
    for (iy=1; iy<ny-1;iy++)
{
yv[ix+iy*nx+iz*ny*nx] = diagonal_element * xv[ix+iy*nx+iz*ny*nx]
-xv[(ix+-1)+(iy+-1)*nx+(iz+-1)*ny*nx]
-xv[(ix+-1)+(iy+-1)*nx+(iz+0)*ny*nx]
-xv[(ix+-1)+(iy+0)*nx+(iz+-1)*ny*nx]
-xv[(ix+-1)+(iy+0)*nx+(iz+0)*ny*nx]
-xv[(ix+-1)+(iy+1)*nx+(iz+-1)*ny*nx]
-xv[(ix+-1)+(iy+1)*nx+(iz+0)*ny*nx]
-xv[(ix+0)+(iy+-1)*nx+(iz+-1)*ny*nx]
-xv[(ix+0)+(iy+-1)*nx+(iz+0)*ny*nx]
-xv[(ix+0)+(iy+0)*nx+(iz+-1)*ny*nx]
-xv[(ix+0)+(iy+1)*nx+(iz+-1)*ny*nx]
-xv[(ix+0)+(iy+1)*nx+(iz+0)*ny*nx]
-xv[(ix+1)+(iy+-1)*nx+(iz+-1)*ny*nx]
-xv[(ix+1)+(iy+-1)*nx+(iz+0)*ny*nx]
-xv[(ix+1)+(iy+0)*nx+(iz+-1)*ny*nx]
-xv[(ix+1)+(iy+0)*nx+(iz+0)*ny*nx]
-xv[(ix+1)+(iy+1)*nx+(iz+-1)*ny*nx]
-xv[(ix+1)+(iy+1)*nx+(iz+0)*ny*nx]
;}
}
iy = 0;
for (ix=1; ix<nx-1;ix++){
    for (iz=1; iz<nz-1;iz++)
{
yv[ix+iy*nx+iz*ny*nx] = diagonal_element * xv[ix+iy*nx+iz*ny*nx]
-xv[(ix+-1)+(iy+0)*nx+(iz+-1)*ny*nx]
-xv[(ix+-1)+(iy+1)*nx+(iz+-1)*ny*nx]
-xv[(ix+-1)+(iy+0)*nx+(iz+0)*ny*nx]
-xv[(ix+-1)+(iy+1)*nx+(iz+0)*ny*nx]
-xv[(ix+-1)+(iy+0)*nx+(iz+1)*ny*nx]
-xv[(ix+-1)+(iy+1)*nx+(iz+1)*ny*nx]
-xv[(ix+0)+(iy+0)*nx+(iz+-1)*ny*nx]
-xv[(ix+0)+(iy+1)*nx+(iz+-1)*ny*nx]
-xv[(ix+0)+(iy+1)*nx+(iz+0)*ny*nx]
-xv[(ix+0)+(iy+0)*nx+(iz+1)*ny*nx]
-xv[(ix+0)+(iy+1)*nx+(iz+1)*ny*nx]
-xv[(ix+1)+(iy+0)*nx+(iz+-1)*ny*nx]
-xv[(ix+1)+(iy+1)*nx+(iz+-1)*ny*nx]
-xv[(ix+1)+(iy+0)*nx+(iz+0)*ny*nx]
-xv[(ix+1)+(iy+1)*nx+(iz+0)*ny*nx]
-xv[(ix+1)+(iy+0)*nx+(iz+1)*ny*nx]
-xv[(ix+1)+(iy+1)*nx+(iz+1)*ny*nx]
;}
}
iy = ny-1;
for (ix=1; ix<nx-1;ix++){
    for (iz=1; iz<nz-1;iz++)
{
yv[ix+iy*nx+iz*ny*nx] = diagonal_element * xv[ix+iy*nx+iz*ny*nx]
-xv[(ix+-1)+(iy+-1)*nx+(iz+-1)*ny*nx]
-xv[(ix+-1)+(iy+0)*nx+(iz+-1)*ny*nx]
-xv[(ix+-1)+(iy+-1)*nx+(iz+0)*ny*nx]
-xv[(ix+-1)+(iy+0)*nx+(iz+0)*ny*nx]
-xv[(ix+-1)+(iy+-1)*nx+(iz+1)*ny*nx]
-xv[(ix+-1)+(iy+0)*nx+(iz+1)*ny*nx]
-xv[(ix+0)+(iy+-1)*nx+(iz+-1)*ny*nx]
-xv[(ix+0)+(iy+0)*nx+(iz+-1)*ny*nx]
-xv[(ix+0)+(iy+-1)*nx+(iz+0)*ny*nx]
-xv[(ix+0)+(iy+-1)*nx+(iz+1)*ny*nx]
-xv[(ix+0)+(iy+0)*nx+(iz+1)*ny*nx]
-xv[(ix+1)+(iy+-1)*nx+(iz+-1)*ny*nx]
-xv[(ix+1)+(iy+0)*nx+(iz+-1)*ny*nx]
-xv[(ix+1)+(iy+-1)*nx+(iz+0)*ny*nx]
-xv[(ix+1)+(iy+0)*nx+(iz+0)*ny*nx]
-xv[(ix+1)+(iy+-1)*nx+(iz+1)*ny*nx]
-xv[(ix+1)+(iy+0)*nx+(iz+1)*ny*nx]
;}
}
ix = 0;
for (iy=1; iy<ny-1;iy++){
    for (iz=1; iz<nz-1;iz++)
{
yv[ix+iy*nx+iz*ny*nx] = diagonal_element * xv[ix+iy*nx+iz*ny*nx]
-xv[(ix+0)+(iy+-1)*nx+(iz+-1)*ny*nx]
-xv[(ix+1)+(iy+-1)*nx+(iz+-1)*ny*nx]
-xv[(ix+0)+(iy+-1)*nx+(iz+0)*ny*nx]
-xv[(ix+1)+(iy+-1)*nx+(iz+0)*ny*nx]
-xv[(ix+0)+(iy+-1)*nx+(iz+1)*ny*nx]
-xv[(ix+1)+(iy+-1)*nx+(iz+1)*ny*nx]
-xv[(ix+0)+(iy+0)*nx+(iz+-1)*ny*nx]
-xv[(ix+1)+(iy+0)*nx+(iz+-1)*ny*nx]
-xv[(ix+1)+(iy+0)*nx+(iz+0)*ny*nx]
-xv[(ix+0)+(iy+0)*nx+(iz+1)*ny*nx]
-xv[(ix+1)+(iy+0)*nx+(iz+1)*ny*nx]
-xv[(ix+0)+(iy+1)*nx+(iz+-1)*ny*nx]
-xv[(ix+1)+(iy+1)*nx+(iz+-1)*ny*nx]
-xv[(ix+0)+(iy+1)*nx+(iz+0)*ny*nx]
-xv[(ix+1)+(iy+1)*nx+(iz+0)*ny*nx]
-xv[(ix+0)+(iy+1)*nx+(iz+1)*ny*nx]
-xv[(ix+1)+(iy+1)*nx+(iz+1)*ny*nx]
;}
}
ix = nx-1;
for (iy=1; iy<ny-1;iy++){
    for (iz=1; iz<nz-1;iz++)
{
yv[ix+iy*nx+iz*ny*nx] = diagonal_element * xv[ix+iy*nx+iz*ny*nx]
-xv[(ix+-1)+(iy+-1)*nx+(iz+-1)*ny*nx]
-xv[(ix+0)+(iy+-1)*nx+(iz+-1)*ny*nx]
-xv[(ix+-1)+(iy+-1)*nx+(iz+0)*ny*nx]
-xv[(ix+0)+(iy+-1)*nx+(iz+0)*ny*nx]
-xv[(ix+-1)+(iy+-1)*nx+(iz+1)*ny*nx]
-xv[(ix+0)+(iy+-1)*nx+(iz+1)*ny*nx]
-xv[(ix+-1)+(iy+0)*nx+(iz+-1)*ny*nx]
-xv[(ix+0)+(iy+0)*nx+(iz+-1)*ny*nx]
-xv[(ix+-1)+(iy+0)*nx+(iz+0)*ny*nx]
-xv[(ix+-1)+(iy+0)*nx+(iz+1)*ny*nx]
-xv[(ix+0)+(iy+0)*nx+(iz+1)*ny*nx]
-xv[(ix+-1)+(iy+1)*nx+(iz+-1)*ny*nx]
-xv[(ix+0)+(iy+1)*nx+(iz+-1)*ny*nx]
-xv[(ix+-1)+(iy+1)*nx+(iz+0)*ny*nx]
-xv[(ix+0)+(iy+1)*nx+(iz+0)*ny*nx]
-xv[(ix+-1)+(iy+1)*nx+(iz+1)*ny*nx]
-xv[(ix+0)+(iy+1)*nx+(iz+1)*ny*nx]
;}
}

//edges
ix = 0;
iy = 0;
for (iz=1; iz<nz-1;iz++){
yv[ix+iy*nx+iz*ny*nx] = diagonal_element * xv[ix+iy*nx+iz*ny*nx]
-xv[(ix+0)+(iy+0)*nx+(iz+-1)*ny*nx]
-xv[(ix+0)+(iy+1)*nx+(iz+-1)*ny*nx]
-xv[(ix+1)+(iy+0)*nx+(iz+-1)*ny*nx]
-xv[(ix+1)+(iy+1)*nx+(iz+-1)*ny*nx]
-xv[(ix+0)+(iy+1)*nx+(iz+0)*ny*nx]
-xv[(ix+1)+(iy+0)*nx+(iz+0)*ny*nx]
-xv[(ix+1)+(iy+1)*nx+(iz+0)*ny*nx]
-xv[(ix+0)+(iy+0)*nx+(iz+1)*ny*nx]
-xv[(ix+0)+(iy+1)*nx+(iz+1)*ny*nx]
-xv[(ix+1)+(iy+0)*nx+(iz+1)*ny*nx]
-xv[(ix+1)+(iy+1)*nx+(iz+1)*ny*nx]
;}
ix = 0;
iy = ny-1;
for (iz=1; iz<nz-1;iz++){
yv[ix+iy*nx+iz*ny*nx] = diagonal_element * xv[ix+iy*nx+iz*ny*nx]
-xv[(ix+0)+(iy+-1)*nx+(iz+-1)*ny*nx]
-xv[(ix+0)+(iy+0)*nx+(iz+-1)*ny*nx]
-xv[(ix+1)+(iy+-1)*nx+(iz+-1)*ny*nx]
-xv[(ix+1)+(iy+0)*nx+(iz+-1)*ny*nx]
-xv[(ix+0)+(iy+-1)*nx+(iz+0)*ny*nx]
-xv[(ix+1)+(iy+-1)*nx+(iz+0)*ny*nx]
-xv[(ix+1)+(iy+0)*nx+(iz+0)*ny*nx]
-xv[(ix+0)+(iy+-1)*nx+(iz+1)*ny*nx]
-xv[(ix+0)+(iy+0)*nx+(iz+1)*ny*nx]
-xv[(ix+1)+(iy+-1)*nx+(iz+1)*ny*nx]
-xv[(ix+1)+(iy+0)*nx+(iz+1)*ny*nx]
;}
ix = nx-1;
iy = 0;
for (iz=1; iz<nz-1;iz++){
yv[ix+iy*nx+iz*ny*nx] = diagonal_element * xv[ix+iy*nx+iz*ny*nx]
-xv[(ix+-1)+(iy+0)*nx+(iz+-1)*ny*nx]
-xv[(ix+-1)+(iy+1)*nx+(iz+-1)*ny*nx]
-xv[(ix+0)+(iy+0)*nx+(iz+-1)*ny*nx]
-xv[(ix+0)+(iy+1)*nx+(iz+-1)*ny*nx]
-xv[(ix+-1)+(iy+0)*nx+(iz+0)*ny*nx]
-xv[(ix+-1)+(iy+1)*nx+(iz+0)*ny*nx]
-xv[(ix+0)+(iy+1)*nx+(iz+0)*ny*nx]
-xv[(ix+-1)+(iy+0)*nx+(iz+1)*ny*nx]
-xv[(ix+-1)+(iy+1)*nx+(iz+1)*ny*nx]
-xv[(ix+0)+(iy+0)*nx+(iz+1)*ny*nx]
-xv[(ix+0)+(iy+1)*nx+(iz+1)*ny*nx]
;}
ix = nx-1;
iy = ny-1;
for (iz=1; iz<nz-1;iz++){
yv[ix+iy*nx+iz*ny*nx] = diagonal_element * xv[ix+iy*nx+iz*ny*nx]
-xv[(ix+-1)+(iy+-1)*nx+(iz+-1)*ny*nx]
-xv[(ix+-1)+(iy+0)*nx+(iz+-1)*ny*nx]
-xv[(ix+0)+(iy+-1)*nx+(iz+-1)*ny*nx]
-xv[(ix+0)+(iy+0)*nx+(iz+-1)*ny*nx]
-xv[(ix+-1)+(iy+-1)*nx+(iz+0)*ny*nx]
-xv[(ix+-1)+(iy+0)*nx+(iz+0)*ny*nx]
-xv[(ix+0)+(iy+-1)*nx+(iz+0)*ny*nx]
-xv[(ix+-1)+(iy+-1)*nx+(iz+1)*ny*nx]
-xv[(ix+-1)+(iy+0)*nx+(iz+1)*ny*nx]
-xv[(ix+0)+(iy+-1)*nx+(iz+1)*ny*nx]
-xv[(ix+0)+(iy+0)*nx+(iz+1)*ny*nx]
;}
ix = 0;
iz = 0;
for (iy=1; iy<ny-1;iy++){
yv[ix+iy*nx+iz*ny*nx] = diagonal_element * xv[ix+iy*nx+iz*ny*nx]
-xv[(ix+0)+(iy+-1)*nx+(iz+0)*ny*nx]
-xv[(ix+0)+(iy+-1)*nx+(iz+1)*ny*nx]
-xv[(ix+1)+(iy+-1)*nx+(iz+0)*ny*nx]
-xv[(ix+1)+(iy+-1)*nx+(iz+1)*ny*nx]
-xv[(ix+0)+(iy+0)*nx+(iz+1)*ny*nx]
-xv[(ix+1)+(iy+0)*nx+(iz+0)*ny*nx]
-xv[(ix+1)+(iy+0)*nx+(iz+1)*ny*nx]
-xv[(ix+0)+(iy+1)*nx+(iz+0)*ny*nx]
-xv[(ix+0)+(iy+1)*nx+(iz+1)*ny*nx]
-xv[(ix+1)+(iy+1)*nx+(iz+0)*ny*nx]
-xv[(ix+1)+(iy+1)*nx+(iz+1)*ny*nx]
;}
ix = 0;
iz = nz-1;
for (iy=1; iy<ny-1;iy++){
yv[ix+iy*nx+iz*ny*nx] = diagonal_element * xv[ix+iy*nx+iz*ny*nx]
-xv[(ix+0)+(iy+-1)*nx+(iz+-1)*ny*nx]
-xv[(ix+0)+(iy+-1)*nx+(iz+0)*ny*nx]
-xv[(ix+1)+(iy+-1)*nx+(iz+-1)*ny*nx]
-xv[(ix+1)+(iy+-1)*nx+(iz+0)*ny*nx]
-xv[(ix+0)+(iy+0)*nx+(iz+-1)*ny*nx]
-xv[(ix+1)+(iy+0)*nx+(iz+-1)*ny*nx]
-xv[(ix+1)+(iy+0)*nx+(iz+0)*ny*nx]
-xv[(ix+0)+(iy+1)*nx+(iz+-1)*ny*nx]
-xv[(ix+0)+(iy+1)*nx+(iz+0)*ny*nx]
-xv[(ix+1)+(iy+1)*nx+(iz+-1)*ny*nx]
-xv[(ix+1)+(iy+1)*nx+(iz+0)*ny*nx]
;}
ix = nx-1;
iz = 0;
for (iy=1; iy<ny-1;iy++){
yv[ix+iy*nx+iz*ny*nx] = diagonal_element * xv[ix+iy*nx+iz*ny*nx]
-xv[(ix+-1)+(iy+-1)*nx+(iz+0)*ny*nx]
-xv[(ix+-1)+(iy+-1)*nx+(iz+1)*ny*nx]
-xv[(ix+0)+(iy+-1)*nx+(iz+0)*ny*nx]
-xv[(ix+0)+(iy+-1)*nx+(iz+1)*ny*nx]
-xv[(ix+-1)+(iy+0)*nx+(iz+0)*ny*nx]
-xv[(ix+-1)+(iy+0)*nx+(iz+1)*ny*nx]
-xv[(ix+0)+(iy+0)*nx+(iz+1)*ny*nx]
-xv[(ix+-1)+(iy+1)*nx+(iz+0)*ny*nx]
-xv[(ix+-1)+(iy+1)*nx+(iz+1)*ny*nx]
-xv[(ix+0)+(iy+1)*nx+(iz+0)*ny*nx]
-xv[(ix+0)+(iy+1)*nx+(iz+1)*ny*nx]
;}
ix = nx-1;
iz = nz-1;
for (iy=1; iy<ny-1;iy++){
yv[ix+iy*nx+iz*ny*nx] = diagonal_element * xv[ix+iy*nx+iz*ny*nx]
-xv[(ix+-1)+(iy+-1)*nx+(iz+-1)*ny*nx]
-xv[(ix+-1)+(iy+-1)*nx+(iz+0)*ny*nx]
-xv[(ix+0)+(iy+-1)*nx+(iz+-1)*ny*nx]
-xv[(ix+0)+(iy+-1)*nx+(iz+0)*ny*nx]
-xv[(ix+-1)+(iy+0)*nx+(iz+-1)*ny*nx]
-xv[(ix+-1)+(iy+0)*nx+(iz+0)*ny*nx]
-xv[(ix+0)+(iy+0)*nx+(iz+-1)*ny*nx]
-xv[(ix+-1)+(iy+1)*nx+(iz+-1)*ny*nx]
-xv[(ix+-1)+(iy+1)*nx+(iz+0)*ny*nx]
-xv[(ix+0)+(iy+1)*nx+(iz+-1)*ny*nx]
-xv[(ix+0)+(iy+1)*nx+(iz+0)*ny*nx]
;}
iy = 0;
iz = 0;
for (ix=1; ix<nx-1;ix++){
yv[ix+iy*nx+iz*ny*nx] = diagonal_element * xv[ix+iy*nx+iz*ny*nx]
-xv[(ix+-1)+(iy+0)*nx+(iz+0)*ny*nx]
-xv[(ix+-1)+(iy+0)*nx+(iz+1)*ny*nx]
-xv[(ix+-1)+(iy+1)*nx+(iz+0)*ny*nx]
-xv[(ix+-1)+(iy+1)*nx+(iz+1)*ny*nx]
-xv[(ix+0)+(iy+0)*nx+(iz+1)*ny*nx]
-xv[(ix+0)+(iy+1)*nx+(iz+0)*ny*nx]
-xv[(ix+0)+(iy+1)*nx+(iz+1)*ny*nx]
-xv[(ix+1)+(iy+0)*nx+(iz+0)*ny*nx]
-xv[(ix+1)+(iy+0)*nx+(iz+1)*ny*nx]
-xv[(ix+1)+(iy+1)*nx+(iz+0)*ny*nx]
-xv[(ix+1)+(iy+1)*nx+(iz+1)*ny*nx]
;}
iy = 0;
iz = nz-1;
for (ix=1; ix<nx-1;ix++){
yv[ix+iy*nx+iz*ny*nx] = diagonal_element * xv[ix+iy*nx+iz*ny*nx]
-xv[(ix+-1)+(iy+0)*nx+(iz+-1)*ny*nx]
-xv[(ix+-1)+(iy+0)*nx+(iz+0)*ny*nx]
-xv[(ix+-1)+(iy+1)*nx+(iz+-1)*ny*nx]
-xv[(ix+-1)+(iy+1)*nx+(iz+0)*ny*nx]
-xv[(ix+0)+(iy+0)*nx+(iz+-1)*ny*nx]
-xv[(ix+0)+(iy+1)*nx+(iz+-1)*ny*nx]
-xv[(ix+0)+(iy+1)*nx+(iz+0)*ny*nx]
-xv[(ix+1)+(iy+0)*nx+(iz+-1)*ny*nx]
-xv[(ix+1)+(iy+0)*nx+(iz+0)*ny*nx]
-xv[(ix+1)+(iy+1)*nx+(iz+-1)*ny*nx]
-xv[(ix+1)+(iy+1)*nx+(iz+0)*ny*nx]
;}
iy = ny-1;
iz = 0;
for (ix=1; ix<nx-1;ix++){
yv[ix+iy*nx+iz*ny*nx] = diagonal_element * xv[ix+iy*nx+iz*ny*nx]
-xv[(ix+-1)+(iy+-1)*nx+(iz+0)*ny*nx]
-xv[(ix+-1)+(iy+-1)*nx+(iz+1)*ny*nx]
-xv[(ix+-1)+(iy+0)*nx+(iz+0)*ny*nx]
-xv[(ix+-1)+(iy+0)*nx+(iz+1)*ny*nx]
-xv[(ix+0)+(iy+-1)*nx+(iz+0)*ny*nx]
-xv[(ix+0)+(iy+-1)*nx+(iz+1)*ny*nx]
-xv[(ix+0)+(iy+0)*nx+(iz+1)*ny*nx]
-xv[(ix+1)+(iy+-1)*nx+(iz+0)*ny*nx]
-xv[(ix+1)+(iy+-1)*nx+(iz+1)*ny*nx]
-xv[(ix+1)+(iy+0)*nx+(iz+0)*ny*nx]
-xv[(ix+1)+(iy+0)*nx+(iz+1)*ny*nx]
;}
iy = ny-1;
iz = nz-1;
for (ix=1; ix<nx-1;ix++){
yv[ix+iy*nx+iz*ny*nx] = diagonal_element * xv[ix+iy*nx+iz*ny*nx]
-xv[(ix+-1)+(iy+-1)*nx+(iz+-1)*ny*nx]
-xv[(ix+-1)+(iy+-1)*nx+(iz+0)*ny*nx]
-xv[(ix+-1)+(iy+0)*nx+(iz+-1)*ny*nx]
-xv[(ix+-1)+(iy+0)*nx+(iz+0)*ny*nx]
-xv[(ix+0)+(iy+-1)*nx+(iz+-1)*ny*nx]
-xv[(ix+0)+(iy+-1)*nx+(iz+0)*ny*nx]
-xv[(ix+0)+(iy+0)*nx+(iz+-1)*ny*nx]
-xv[(ix+1)+(iy+-1)*nx+(iz+-1)*ny*nx]
-xv[(ix+1)+(iy+-1)*nx+(iz+0)*ny*nx]
-xv[(ix+1)+(iy+0)*nx+(iz+-1)*ny*nx]
-xv[(ix+1)+(iy+0)*nx+(iz+0)*ny*nx]
;}

//corners
ix = 0;
iy = 0;
iz = 0;
yv[ix+iy*nx+iz*ny*nx] = diagonal_element * xv[ix+iy*nx+iz*ny*nx]
-xv[(ix+0)+(iy+0)*nx+(iz+1)*ny*nx]
-xv[(ix+0)+(iy+1)*nx+(iz+0)*ny*nx]
-xv[(ix+0)+(iy+1)*nx+(iz+1)*ny*nx]
-xv[(ix+1)+(iy+0)*nx+(iz+0)*ny*nx]
-xv[(ix+1)+(iy+0)*nx+(iz+1)*ny*nx]
-xv[(ix+1)+(iy+1)*nx+(iz+0)*ny*nx]
-xv[(ix+1)+(iy+1)*nx+(iz+1)*ny*nx]
;
ix = 0;
iy = 0;
iz = nz-1;
yv[ix+iy*nx+iz*ny*nx] = diagonal_element * xv[ix+iy*nx+iz*ny*nx]
-xv[(ix+0)+(iy+0)*nx+(iz+-1)*ny*nx]
-xv[(ix+0)+(iy+1)*nx+(iz+-1)*ny*nx]
-xv[(ix+0)+(iy+1)*nx+(iz+0)*ny*nx]
-xv[(ix+1)+(iy+0)*nx+(iz+-1)*ny*nx]
-xv[(ix+1)+(iy+0)*nx+(iz+0)*ny*nx]
-xv[(ix+1)+(iy+1)*nx+(iz+-1)*ny*nx]
-xv[(ix+1)+(iy+1)*nx+(iz+0)*ny*nx]
;
ix = 0;
iy = ny-1;
iz = 0;
yv[ix+iy*nx+iz*ny*nx] = diagonal_element * xv[ix+iy*nx+iz*ny*nx]
-xv[(ix+0)+(iy+-1)*nx+(iz+0)*ny*nx]
-xv[(ix+0)+(iy+-1)*nx+(iz+1)*ny*nx]
-xv[(ix+0)+(iy+0)*nx+(iz+1)*ny*nx]
-xv[(ix+1)+(iy+-1)*nx+(iz+0)*ny*nx]
-xv[(ix+1)+(iy+-1)*nx+(iz+1)*ny*nx]
-xv[(ix+1)+(iy+0)*nx+(iz+0)*ny*nx]
-xv[(ix+1)+(iy+0)*nx+(iz+1)*ny*nx]
;
ix = 0;
iy = ny-1;
iz = nz-1;
yv[ix+iy*nx+iz*ny*nx] = diagonal_element * xv[ix+iy*nx+iz*ny*nx]
-xv[(ix+0)+(iy+-1)*nx+(iz+-1)*ny*nx]
-xv[(ix+0)+(iy+-1)*nx+(iz+0)*ny*nx]
-xv[(ix+0)+(iy+0)*nx+(iz+-1)*ny*nx]
-xv[(ix+1)+(iy+-1)*nx+(iz+-1)*ny*nx]
-xv[(ix+1)+(iy+-1)*nx+(iz+0)*ny*nx]
-xv[(ix+1)+(iy+0)*nx+(iz+-1)*ny*nx]
-xv[(ix+1)+(iy+0)*nx+(iz+0)*ny*nx]
;
ix = nx-1;
iy = 0;
iz = 0;
yv[ix+iy*nx+iz*ny*nx] = diagonal_element * xv[ix+iy*nx+iz*ny*nx]
-xv[(ix+-1)+(iy+0)*nx+(iz+0)*ny*nx]
-xv[(ix+-1)+(iy+0)*nx+(iz+1)*ny*nx]
-xv[(ix+-1)+(iy+1)*nx+(iz+0)*ny*nx]
-xv[(ix+-1)+(iy+1)*nx+(iz+1)*ny*nx]
-xv[(ix+0)+(iy+0)*nx+(iz+1)*ny*nx]
-xv[(ix+0)+(iy+1)*nx+(iz+0)*ny*nx]
-xv[(ix+0)+(iy+1)*nx+(iz+1)*ny*nx]
;
ix = nx-1;
iy = 0;
iz = nz-1;
yv[ix+iy*nx+iz*ny*nx] = diagonal_element * xv[ix+iy*nx+iz*ny*nx]
-xv[(ix+-1)+(iy+0)*nx+(iz+-1)*ny*nx]
-xv[(ix+-1)+(iy+0)*nx+(iz+0)*ny*nx]
-xv[(ix+-1)+(iy+1)*nx+(iz+-1)*ny*nx]
-xv[(ix+-1)+(iy+1)*nx+(iz+0)*ny*nx]
-xv[(ix+0)+(iy+0)*nx+(iz+-1)*ny*nx]
-xv[(ix+0)+(iy+1)*nx+(iz+-1)*ny*nx]
-xv[(ix+0)+(iy+1)*nx+(iz+0)*ny*nx]
;
ix = nx-1;
iy = ny-1;
iz = 0;
yv[ix+iy*nx+iz*ny*nx] = diagonal_element * xv[ix+iy*nx+iz*ny*nx]
-xv[(ix+-1)+(iy+-1)*nx+(iz+0)*ny*nx]
-xv[(ix+-1)+(iy+-1)*nx+(iz+1)*ny*nx]
-xv[(ix+-1)+(iy+0)*nx+(iz+0)*ny*nx]
-xv[(ix+-1)+(iy+0)*nx+(iz+1)*ny*nx]
-xv[(ix+0)+(iy+-1)*nx+(iz+0)*ny*nx]
-xv[(ix+0)+(iy+-1)*nx+(iz+1)*ny*nx]
-xv[(ix+0)+(iy+0)*nx+(iz+1)*ny*nx]
;
ix = nx-1;
iy = ny-1;
iz = nz-1;
yv[ix+iy*nx+iz*ny*nx] = diagonal_element * xv[ix+iy*nx+iz*ny*nx]
-xv[(ix+-1)+(iy+-1)*nx+(iz+-1)*ny*nx]
-xv[(ix+-1)+(iy+-1)*nx+(iz+0)*ny*nx]
-xv[(ix+-1)+(iy+0)*nx+(iz+-1)*ny*nx]
-xv[(ix+-1)+(iy+0)*nx+(iz+0)*ny*nx]
-xv[(ix+0)+(iy+-1)*nx+(iz+-1)*ny*nx]
-xv[(ix+0)+(iy+-1)*nx+(iz+0)*ny*nx]
-xv[(ix+0)+(iy+0)*nx+(iz+-1)*ny*nx]
;


  if (nex*ney*nez == 0) return 0;
  //extras
  //corner becomes edge
  if (ipx < npx -1)
  {
  ix = nx-1;
  iy = 0;
  iz = 0;
 yv[ix+iy*nx+iz*ny*nx] +=
-xv[nlocal]
-xv[nlocal+1*nex]
-xv[nlocal+ney*nex]
-xv[nlocal+1+ney*nex]
;

  ix = nx-1;
  iy = ny-1;
  iz = 0;
 yv[ix+iy*nx+iz*ny*nx] +=
-xv[nlocal-2+2*ney*nex]
-xv[nlocal-1+2*ney*nex]
-xv[nlocal-2+ney*nex]
-xv[nlocal-1+ney*nex]
;

  ix = nx-1;
  iy = 0;
  iz = nz-1;
 yv[ix+iy*nx+iz*ny*nx] +=
-xv[nlocal+ iz*ney*nex]
-xv[nlocal+1+iz*ney*nex]
-xv[nlocal+(iz-1)*ney*nex]
-xv[nlocal+1+(iz-1)*ney*nex]
;

  ix = nx-1;
  iy = ny-1;
  iz = nz-1;
 yv[ix+iy*nx+iz*ny*nx] +=
-xv[nlocal+(iy)*nex+iz*nex*ney]
-xv[nlocal+(iy-1)*nex+iz*nex*ney]
-xv[nlocal+(iy)*nex+(iz-1)*nex*ney]
-xv[nlocal+(iy-1)*nex+(iz-1)*nex*ney]
;

  //edge becomes face
  ix = nx-1;
  iz = 0;
for (iy=1; iy<ny-1;iy++){
yv[ix+iy*nx+iz*ny*nx] +=
-xv[nlocal+iy*nex-1]
-xv[nlocal+iy*nex+1]
-xv[nlocal+iy*nex]
-xv[nlocal+iy*nex-1+nex*ney]
-xv[nlocal+iy*nex+1+nex*ney]
-xv[nlocal+iy*nex+nex*ney]
;
}
  ix = nx-1;
  iz = nz-1;
for (iy=1; iy<ny-1;iy++){
yv[ix+iy*nx+iz*ny*nx] +=
-xv[nlocal+iy*nex-1+(iz)*nex*ney]
-xv[nlocal+iy*nex+1+(iz)*nex*ney]
-xv[nlocal+iy*nex+(iz)*nex*ney]
-xv[nlocal+iy*nex-1+(iz-1)*nex*ney]
-xv[nlocal+iy*nex+1+(iz-1)*nex*ney]
-xv[nlocal+iy*nex+(iz-1)*nex*ney]
;
}
  ix = nx-1;
  iy = 0;
for (iz=1; iz<nz-1;iz++){
yv[ix+iy*nx+iz*ny*nx] +=
-xv[nlocal+(iz)*nex*ney]
-xv[nlocal+1+(iz)*nex*ney]
-xv[nlocal+(iz+1)*nex*ney]
-xv[nlocal+1+(iz+1)*nex*ney]
-xv[nlocal+(iz-1)*nex*ney]
-xv[nlocal+1+(iz-1)*nex*ney]
;
}
  ix = nx-1;
  iy = ny-1;
for (iz=1; iz<nz-1;iz++){
yv[ix+iy*nx+iz*ny*nx] +=
-xv[nlocal+iy*nex+(iz)*nex*ney]
-xv[nlocal+iy*nex-1+(iz)*nex*ney]
-xv[nlocal+iy*nex+(iz+1)*nex*ney]
-xv[nlocal+iy*nex+-1+(iz+1)*nex*ney]
-xv[nlocal+iy*nex+(iz-1)*nex*ney]
-xv[nlocal+iy*nex-1+(iz-1)*nex*ney]
;
}

  //face becomes bulk
  ix = nx-1;
for (iy=1; iy<ny-1;iy++){
   for (iz=1; iz<nz-1;iz++){
    yv[ix+iy*nx+iz*ny*nx] +=
-xv[nlocal+iy*nex+iz*nex*ney]
-xv[nlocal+(1+iy)*nex+iz*nex*ney]
-xv[nlocal+(-1+iy)*nex+iz*nex*ney]
-xv[nlocal+iy*nex+(1+iz)*nex*ney]
-xv[nlocal+(1+iy)*nex+(1+iz)*nex*ney]
-xv[nlocal+(-1+iy)*nex+(1+iz)*nex*ney]
-xv[nlocal+iy*nex+(-1+iz)*nex*ney]
-xv[nlocal+(1+iy)*nex+(-1+iz)*nex*ney]
-xv[nlocal+(-1+iy)*nex+(-1+iz)*nex*ney]
;
   }
}

} //if

  //corner becomes edge
  if (ipx > 0)
{
  ix = 0;
  iy = 0;
  iz = 0;
 yv[ix+iy*nx+iz*ny*nx] +=
-xv[nlocal]
-xv[nlocal+1*nex]
-xv[nlocal+ney*nex]
-xv[nlocal+1+ney*nex]
;

  iy = ny-1;
  iz = 0;
 yv[ix+iy*nx+iz*ny*nx] +=
-xv[nlocal-2+2*ney*nex]
-xv[nlocal-1+2*ney*nex]
-xv[nlocal-2+ney*nex]
-xv[nlocal-1+ney*nex]
;

  iy = 0;
  iz = nz-1;
 yv[ix+iy*nx+iz*ny*nx] +=
-xv[nlocal+ iz*ney*nex]
-xv[nlocal+1+iz*ney*nex]
-xv[nlocal+(iz-1)*ney*nex]
-xv[nlocal+1+(iz-1)*ney*nex]
;

  iy = ny-1;
  iz = nz-1;
 yv[ix+iy*nx+iz*ny*nx] +=
-xv[nlocal+(iy)*nex+iz*nex*ney]
-xv[nlocal+(iy-1)*nex+iz*nex*ney]
-xv[nlocal+(iy)*nex+(iz-1)*nex*ney]
-xv[nlocal+(iy-1)*nex+(iz-1)*nex*ney]
;

  //edge becomes face
  iy = 0;
for (iz=1; iz<nz-1;iz++){
yv[ix+iy*nx+iz*ny*nx] +=
-xv[nlocal+iy*nex+iz*(nex*ney)]
-xv[nlocal+(iy+1)*nex+iz*(nex*ney)]
-xv[nlocal+iy*nex+(iz+1)*(nex*ney)]
-xv[nlocal+(iy+1)*nex+(iz+1)*(nex*ney)]
-xv[nlocal+iy*nex+(iz-1)*(nex*ney)]
-xv[nlocal+(iy+1)*nex+(iz-1)*(nex*ney)]
;
}
  iy = ny-1;
for (iz=1; iz<nz-1;iz++){
yv[ix+iy*nx+iz*ny*nx] +=
-xv[nlocal+iy*nex+iz*(nex*ney)]
-xv[nlocal+(iy-1)*nex+iz*(nex*ney)]
-xv[nlocal+iy*nex+(iz+1)*(nex*ney)]
-xv[nlocal+(iy-1)*nex+(iz+1)*(nex*ney)]
-xv[nlocal+iy*nex+(iz-1)*(nex*ney)]
-xv[nlocal+(iy-1)*nex+(iz-1)*(nex*ney)]
;
}

  iz = 0;
for (iy=1; iy<ny-1;iy++){
yv[ix+iy*nx+iz*ny*nx] +=
-xv[nlocal+iy*nex-1]
-xv[nlocal+iy*nex+1]
-xv[nlocal+iy*nex]
-xv[nlocal+iy*nex-1+nex*ney]
-xv[nlocal+iy*nex+1+nex*ney]
-xv[nlocal+iy*nex+nex*ney]
;
}
  iz = nz-1;
for (iy=1; iy<ny-1;iy++){
yv[ix+iy*nx+iz*ny*nx] +=
-xv[nlocal+iy*nex+iz*(nex*ney)]
-xv[nlocal+iy*nex+(iz-1)*(nex*ney)]
-xv[nlocal+(iy+1)*nex+iz*(nex*ney)]
-xv[nlocal+(iy+1)*nex+(iz-1)*(nex*ney)]
-xv[nlocal+(iy-1)*nex+iz*(nex*ney)]
-xv[nlocal+(iy-1)*nex+(iz-1)*(nex*ney)]
;
}


  //face becomes bulk
for (iy=1; iy<ny-1;iy++){
   for (iz=1; iz<nz-1;iz++){
    yv[ix+iy*nx+iz*ny*nx] +=
-xv[nlocal+iy*nex+iz*nex*ney]
-xv[nlocal+(1+iy)*nex+iz*nex*ney]
-xv[nlocal+(-1+iy)*nex+iz*nex*ney]
-xv[nlocal+iy*nex+(1+iz)*nex*ney]
-xv[nlocal+(1+iy)*nex+(1+iz)*nex*ney]
-xv[nlocal+(-1+iy)*nex+(1+iz)*nex*ney]
-xv[nlocal+iy*nex+(-1+iz)*nex*ney]
-xv[nlocal+(1+iy)*nex+(-1+iz)*nex*ney]
-xv[nlocal+(-1+iy)*nex+(-1+iz)*nex*ney]
;
   }
}


}
  return 0;
}

