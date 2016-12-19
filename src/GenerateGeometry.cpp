
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
 @file GenerateGeometry.cpp

 HPCG routine
 */

#include <cmath>
#include <cstdlib>
#include <cassert>

#include "ComputeOptimalShapeXYZ.hpp"
#include "GenerateGeometry.hpp"

#ifdef HPCG_DEBUG
#include <fstream>
#include "hpcg.hpp"
using std::endl;

#endif

/*!
  Computes the factorization of the total number of processes into a
  3-dimensional process grid that is as close as possible to a cube. The
  quality of the factorization depends on the prime number structure of the
  total number of processes. It then stores this decompostion together with the
  parallel parameters of the run in the geometry data structure.

  @param[in]  size total number of MPI processes
  @param[in]  rank this process' rank among other MPI processes
  @param[in]  numThreads number of OpenMP threads in this process
  @param[in]  pz z-dimension processor ID where second zone of nz values start
  @param[in]  nx, ny, nz number of grid points for each local block in the x, y, and z dimensions, respectively
  @param[out] geom data structure that will store the above parameters and the factoring of total number of processes into three dimensions
*/
void GenerateGeometry(int size, int rank, int numThreads,
  int pz, local_int_t zl, local_int_t zu,
  local_int_t nx, local_int_t ny, local_int_t nz,
  int npx, int npy, int npz,
  Geometry * geom)
{

  if (npx * npy * npz <= 0 || npx * npy * npz > size)
    ComputeOptimalShapeXYZ( size, npx, npy, npz );

  int * partz_ids = 0;
  local_int_t * partz_nz = 0;
  int npartz = 0;
  if (pz==0) { // No variation in nz sizes
    npartz = 1;
    partz_ids = new int[1];
    partz_nz = new local_int_t[1];
    partz_ids[0] = npz;
    partz_nz[0] = nz;
  }
  else {
    npartz = 2;
    partz_ids = new int[2];
    partz_ids[0] = pz;
    partz_ids[1] = npz;
    partz_nz = new local_int_t[2];
    partz_nz[0] = zl;
    partz_nz[1] = zu;
  }
//  partz_ids[npartz-1] = npz; // The last element of this array is always npz
  int ipartz_ids = 0;
  for (int i=0; i< npartz; ++i) {
    assert(ipartz_ids<partz_ids[i]);  // Make sure that z partitioning is consistent with computed npz value
    ipartz_ids = partz_ids[i];
  }

  // Now compute this process's indices in the 3D cube
  int ipz = rank/(npx*npy);
  int ipy = (rank-ipz*npx*npy)/npx;
  int ipx = rank%npx;

#ifdef HPCG_DEBUG
  if (rank==0)
    HPCG_fout   << "size = "<< size << endl
        << "nx  = " << nx << endl
        << "ny  = " << ny << endl
        << "nz  = " << nz << endl
        << "npx = " << npx << endl
        << "npy = " << npy << endl
        << "npz = " << npz << endl;

  HPCG_fout    << "For rank = " << rank << endl
      << "ipx = " << ipx << endl
      << "ipy = " << ipy << endl
      << "ipz = " << ipz << endl;

  assert(size>=npx*npy*npz);
#endif
  geom->size = size;
  geom->rank = rank;
  geom->numThreads = numThreads;
  geom->nx = nx;
  geom->ny = ny;
  geom->nz = nz;
  geom->npx = npx;
  geom->npy = npy;
  geom->npz = npz;
  geom->pz = pz;
  geom->npartz = npartz;
  geom->partz_ids = partz_ids;
  geom->partz_nz = partz_nz;
  geom->ipx = ipx;
  geom->ipy = ipy;
  geom->ipz = ipz;

// These values should be defined to take into account changes in nx, ny, nz values
// due to variable local grid sizes
  global_int_t gnx = npx*nx;
  global_int_t gny = npy*ny;
  //global_int_t gnz = npz*nz;
  // We now permit varying values for nz for any nx-by-ny plane of MPI processes.
  // npartz is the number of different groups of nx-by-ny groups of processes.
  // partz_ids is an array of length npartz where each value indicates the z process of the last process in the ith nx-by-ny group.
  // partz_nz is an array of length npartz containing the value of nz for the ith group.

  //        With no variation, npartz = 1, partz_ids[0] = npz, partz_nz[0] = nz

  global_int_t gnz = 0;
  ipartz_ids = 0;

  for (int i=0; i< npartz; ++i) {
    ipartz_ids = partz_ids[i] - ipartz_ids;
    gnz += partz_nz[i]*ipartz_ids;
  }
  //global_int_t giz0 = ipz*nz;
  global_int_t giz0 = 0;
  ipartz_ids = 0;
  for (int i=0; i< npartz; ++i) {
    int ipart_nz = partz_nz[i];
    if (ipz < partz_ids[i]) {
      giz0 += (ipz-ipartz_ids)*ipart_nz;
      break;
    } else {
      ipartz_ids = partz_ids[i];
      giz0 += ipartz_ids*ipart_nz;
    }

  }
  global_int_t gix0 = ipx*nx;
  global_int_t giy0 = ipy*ny;

// Keep these values for later
  geom->gnx = gnx;
  geom->gny = gny;
  geom->gnz = gnz;
  geom->gix0 = gix0;
  geom->giy0 = giy0;
  geom->giz0 = giz0;

  return;
}
