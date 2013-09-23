
//@HEADER
// ************************************************************************
// 
//               HPCG: Simple Conjugate Gradient Benchmark Code
// Questions? Contact Michael A. Heroux (maherou@sandia.gov) 
// 
// ************************************************************************
//@HEADER

/*!
 @file SparseMatrix.hpp

 HPCG data structures for the sparse matrix
 */

#ifndef SPARSEMATRIX_HPP
#define SPARSEMATRIX_HPP

#include <map>
#include <vector>
#include "Geometry.hpp"

struct SparseMatrix_STRUCT {
  char   *title; //!< name of the sparse matrix
  global_int_t totalNumberOfRows; //!< total number of matrix rows across all processes
  global_int_t totalNumberOfNonzeros; //!< total number of matrix nonzeros across all processes
  local_int_t localNumberOfRows; //!< number of rows local to this process
  local_int_t localNumberOfColumns;  //!< number of columns local to this process
  local_int_t localNumberOfNonzeros;  //!< number of nonzeros local to this process
  char  * nonzerosInRow;  //!< The number of nonzeros in a row will always be 27 or fewer
  global_int_t ** mtxIndG; //!< matrix indices as global values
  local_int_t ** mtxIndL; //!< matrix indices as local values
  double ** matrixValues; //!< values of matrix entries
  double ** matrixDiagonal; //!< values of matrix diagonal entries
  std::map< global_int_t, local_int_t > globalToLocalMap; //!< global-to-local mapping
  std::vector< global_int_t > localToGlobalMap; //!< local-to-global mapping

  /*!
   This is for storing optimized data structres created in OptimizeProblem and
   used inside optimized spmv().
   */
  void *optimization_data;

#ifndef HPCG_NOMPI
  local_int_t numberOfExternalValues; //!< number of entries that are external to this process
  int numberOfSendNeighbors; //!< number of neighboring processes that will be send local data
  local_int_t totalToBeSent; //!< total number of entries to be sent
  local_int_t *elementsToSend; //!< elements to send to neighboring processes
  int *neighbors; //!< neighboring processes
  local_int_t *receiveLength; //!< lenghts of messages received from neighboring processes
  local_int_t *sendLength; //!< lenghts of messages sent to neighboring processes
  double *sendBuffer; //!< send buffer for non-blocking sends
#endif
};
typedef struct SparseMatrix_STRUCT SparseMatrix;

/*!
  Initializes the known system matrix data structure members to 0.

  @param[in] A the known system matrix
 */
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

/*!
  Deallocates the members of the data structure of the known system matrix provided they are not 0.

  @param[in] A the known system matrix
 */
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

#endif // SPARSEMATRIX_HPP
