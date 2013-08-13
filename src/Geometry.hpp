
//@HEADER
// ************************************************************************
// 
//               HPCG: Simple Conjugate Gradient Benchmark Code
// Questions? Contact Michael A. Heroux (maherou@sandia.gov) 
// 
// ************************************************************************
//@HEADER

#ifndef GEOMETRY_HPP
#define GEOMETRY_HPP

typedef long long local_int_t;
typedef long long global_int_t;

// This is a data structure to contain all processor navigation information

struct Geometry_STRUCT {
  int size; // Number of MPI processes
  int rank; // This process' rank in the range [0 to size - 1]
  int nx;   // Number of x-direction grid points for each local subdomain
  int ny;   // Number of y-direction grid points for each local subdomain
  int nz;   // Number of z-direction grid points for each local subdomain
  int npx;  // Number of processors in x-direction
  int npy;  // Number of processors in y-direction
  int npz;  // Number of processors in z-direction
  int ipx;  // Current rank's x location in the npx by npy by npz processor grid
  int ipy;  // Current rank's y location in the npx by npy by npz processor grid
  int ipz;  // Current rank's z location in the npx by npy by npz processor grid

};
typedef struct Geometry_STRUCT Geometry;


#endif // GEOMETRY_HPP
