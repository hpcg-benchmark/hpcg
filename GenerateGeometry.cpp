//@HEADER
// ************************************************************************
//
//               HPCG: Simple Conjugate Gradient Benchmark Code
// Questions? Contact Michael A. Heroux (maherou@sandia.gov)
//
// ************************************************************************
//@HEADER

// Input
// size - Number of MPI processes
// rank - My process id
// nx, ny, nz - Number of grid points for each local block in the x, y, z dimensions, resp.
//
// Output
// npx, npy, npz - Factoring of np into 3D cubed
// ipx, ipy, ipz - x, y, z coordinate of this process in the npx by npy by npz processor cube.

#include "GenerateGeometry.hpp"
#ifdef DEBUG
#include <iostream>
using std::cout;
using std::cerr;
using std::endl;

#include <cassert>
#endif
void GenerateGeometry(int size, int rank, int nx, int ny, int nz, Geometry & geom) {

	int npx = 1; // Processor decomposition in X
	for (int i = 2; i*i*i<=size; ++i) if (size%i==0) npx = i; // increase npx if an integral divide

	int npy = 1; // Processor decomposition in Y
	int nq = size/npx; // Remaining yz-dimension to factor
	for (int i = 2; i*i<=nq; ++i) if (nq%i==0) npy = i; // increase npy if an integral divide

	int npz = nq/npy; // npz is easy to compute

	// Now compute this process's indices in the 3D cube
	int ipz = rank/(npx*npy);
	int ipy = (rank-ipz*npx*npy)/npx;
	int ipx = rank%npx;

#ifdef DEBUG
	if (rank==0)
	cout 	<< "size = "<< size << endl
			<< "nx  = " << nx << endl
			<< "ny  = " << ny << endl
			<< "nz  = " << nz << endl
			<< "npx = " << npx << endl
			<< "npy = " << npy << endl
			<< "npz = " << npz << endl;

	cout    << "For rank = " << rank << endl
			<< "ipx = " << ipx << endl
			<< "ipy = " << ipy << endl
			<< "ipz = " << ipz << endl;

	assert(size==npx*npy*npz);
#endif
	geom.size = size;
	geom.rank = rank;
	geom.nx = nx;
	geom.ny = ny;
	geom.nz = nz;
	geom.npx = npx;
	geom.npy = npy;
	geom.npz = npz;
	geom.ipx = ipx;
	geom.ipy = ipy;
	geom.ipz = ipz;
	return;
}
