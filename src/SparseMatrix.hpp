
//@HEADER
// ************************************************************************
// 
//               HPCG: Simple Conjugate Gradient Benchmark Code
// Questions? Contact Michael A. Heroux (maherou@sandia.gov) 
// 
// ************************************************************************
//@HEADER

#ifndef SPARSEMATRIX_HPP
#define SPARSEMATRIX_HPP

#include <map>
#include <vector>
#include "Geometry.hpp"
#ifdef DETAILEDDEBUG
#include <iostream>
#endif

struct SparseMatrix_STRUCT {
  char   *title;
  global_int_t totalNumberOfRows;
  global_int_t totalNumberOfNonzeros;
  local_int_t localNumberOfRows;
  local_int_t localNumberOfColumns;  // Must be defined in make_local_matrix
  global_int_t localNumberOfNonzeros;  //Make this a globlal since it can get big
  char  * nonzerosInRow;  // The number of nonzeros in a row will always be 27 or fewer
  global_int_t ** mtxIndG;
  local_int_t ** mtxIndL;
  double ** matrixValues;
  double ** matrixDiagonal;
  std::map< global_int_t, local_int_t > globalToLocalMap;
  std::vector< global_int_t > localToGlobalMap;

  /*
   This is for storing optimized data structres created in OptimizeProblem and
   used inside optimized spmv().
   */
  void *optimization_data;

#ifndef HPCG_NOMPI
  local_int_t numberOfExternalValues;
  int numberOfSendNeighbors;
  local_int_t totalToBeSent;
  local_int_t *elementsToSend;
  int *neighbors;
  local_int_t *receiveLength;
  local_int_t *sendLength;
  double *sendBuffer;
#endif
};
typedef struct SparseMatrix_STRUCT SparseMatrix;

inline void initializeMatrix(SparseMatrix & A) {
	A.title = 0;
	A.totalNumberOfRows = 0;
	A.totalNumberOfNonzeros = 0;
	A.localNumberOfRows = 0;
	A.localNumberOfColumns = 0;
	A.localNumberOfNonzeros = 0;
	A.nonzerosInRow = 0;
	A.mtxIndG = 0;
	A.mtxIndL = 0;
	A.matrixValues = 0;
	A.matrixDiagonal = 0;

	#ifndef HPCG_NOMPI
	A.numberOfExternalValues = 0;
	A.numberOfSendNeighbors = 0;
	A.totalToBeSent = 0;
	A.elementsToSend = 0;
	A.neighbors = 0;
	A.receiveLength = 0;
	A.sendLength = 0;
	A.sendBuffer = 0;
	#endif
	return;
}

inline void destroyMatrix(SparseMatrix & A) {

	for (int i = 0; i< A.localNumberOfRows; ++i) {
		delete [] A.matrixValues[i];
		delete [] A.mtxIndG[i];
		delete [] A.mtxIndL[i];
	}
	if(A.title)                  delete [] A.title;
	if(A.nonzerosInRow)             delete [] A.nonzerosInRow;
	if(A.mtxIndG) delete [] A.mtxIndG;
	if(A.mtxIndL) delete [] A.mtxIndL;
	if(A.matrixValues) delete [] A.matrixValues;
	if(A.matrixDiagonal)           delete [] A.matrixDiagonal;
	
#ifndef HPCG_NOMPI
	if(A.elementsToSend)       delete [] A.elementsToSend;
	if(A.neighbors)              delete [] A.neighbors;
	if(A.receiveLength)            delete [] A.receiveLength;
	if(A.sendLength)            delete [] A.sendLength;
	if(A.sendBuffer)            delete [] A.sendBuffer;
#endif
	initializeMatrix(A);
	return;
}

inline int getRankOfMatrixRow(const Geometry & geom, const SparseMatrix & A, global_int_t index) {
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
#ifdef DETAILEDDEBUG
	//std::cout << "In getRank, rank = " << geom.rank << " index, gnx, gny, ipz, ipy, ipx, rank = " << index << " " << gnx << " "  << " " << gny << " " << ipz << " " << ipy << " " << ipx << " " << rank << std::endl;
#endif
	return(rank);

}

#endif // SPARSEMATRIX_HPP

