
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

//typedef int local_int_t;      // This defines the type for integers that have local subdomain dimension
typedef long long local_int_t;      // Define as long long when local problem dimension is > 2^31

// This macro should be defined if the global_int_t is not long long
// in order to stop complaints from non-C++11 compliant compilers
//#define HPCG_NO_LONG_LONG
//typedef int global_int_t;     // This defines the type for integers that have global dimension
typedef long long global_int_t;     // Define as long long when global problem dimension is > 2^31

// This is a data structure to contain all processor navigation information

struct Geometry_STRUCT {
  int size; // Number of MPI processes
  int rank; // This process' rank in the range [0 to size - 1]
  int numThreads; // This process' number of threads
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

inline int getRankOfMatrixRow(const Geometry & geom, global_int_t index) {
	// For the global row id given in the argument index, return the MPI process rank that is assigned that row
	int gnx = geom.nx*geom.npx;
	int gny = geom.ny*geom.npy;

	int iz = index/(gny*gnx);
	int iy = (index-iz*gny*gnx)/gnx;
	int ix = index%gnx;
	int ipz = iz/geom.nz;
	int ipy = iy/geom.ny;
	int ipx = ix/geom.nx;
	int rank = ipx+ipy*geom.npx+ipz*geom.npy*geom.npx;
	return(rank);

}


#endif // GEOMETRY_HPP
