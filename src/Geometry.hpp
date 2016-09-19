
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
 @file Geometry.hpp

 HPCG data structure for problem geometry
 */

#ifndef GEOMETRY_HPP
#define GEOMETRY_HPP

/*!
  This defines the type for integers that have local subdomain dimension.

  Define as "long long" when local problem dimension is > 2^31
*/
typedef int local_int_t;
//typedef long long local_int_t;

/*!
  This defines the type for integers that have global dimension

  Define as "long long" when global problem dimension is > 2^31
*/
//typedef int global_int_t;
typedef long long global_int_t;

// This macro should be defined if the global_int_t is not long long
// in order to stop complaints from non-C++11 compliant compilers.
//#define HPCG_NO_LONG_LONG

/*!
  This is a data structure to contain all processor geometry information
*/
struct Geometry_STRUCT {
  int size; //!< Number of MPI processes
  int rank; //!< This process' rank in the range [0 to size - 1]
  int numThreads; //!< This process' number of threads
  int nx;   //!< Number of x-direction grid points for each local subdomain TODO: The type for nx, ny and nz should be local_int_t
  int ny;   //!< Number of y-direction grid points for each local subdomain
  int nz;   //!< Number of z-direction grid points for each local subdomain
  int npx;  //!< Number of processors in x-direction
  int npy;  //!< Number of processors in y-direction
  int npz;  //!< Number of processors in z-direction
  global_int_t npartz; //!< Number of partitions with varying nz values
  global_int_t * partz_ids; //!< Array of partition ids of processor in z-direction where new value of nz starts (valid values are 0 to npz-1)
  global_int_t * partz_nz; //!< Array of nz values for each partition
  int ipx;  //!< Current rank's x location in the npx by npy by npz processor grid
  int ipy;  //!< Current rank's y location in the npx by npy by npz processor grid
  int ipz;  //!< Current rank's z location in the npx by npy by npz processor grid
  global_int_t gnx;  //!< Global number of x-direction grid points
  global_int_t gny;  //!< Global number of y-direction grid points
  global_int_t gnz;  //!< Global number of z-direction grid points
  global_int_t gix0;  //!< Base global x index for this rank in the npx by npy by npz processor grid
  global_int_t giy0;  //!< Base global y index for this rank in the npx by npy by npz processor grid
  global_int_t giz0;  //!< Base global z index for this rank in the npx by npy by npz processor grid

};
typedef struct Geometry_STRUCT Geometry;

/*!
  Returns the rank of the MPI process that is assigned the global row index
  given as the input argument.

  @param[in] geom  The description of the problem's geometry.
  @param[in] index The global row index

  @return Returns the MPI rank of the process assigned the row
*/
inline int ComputeRankOfMatrixRow(const Geometry & geom, global_int_t index) {
  global_int_t gnx = geom.gnx;
  global_int_t gny = geom.gny;

  global_int_t iz = index/(gny*gnx);
  global_int_t iy = (index-iz*gny*gnx)/gnx;
  global_int_t ix = index%gnx;
  global_int_t ipartz = 0;
  int inz = geom.partz_nz[0];
  for (int i=0; i< geom.npartz; ++i) {

  }
  global_int_t ipz = iz/geom.nz;
  global_int_t ipy = iy/geom.ny;
  global_int_t ipx = ix/geom.nx;
  int rank = ipx+ipy*geom.npx+ipz*geom.npy*geom.npx;
  return rank;
}


#endif // GEOMETRY_HPP
