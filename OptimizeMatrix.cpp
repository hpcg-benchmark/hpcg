
//@HEADER
// ************************************************************************
// 
//               HPCG: Simple Conjugate Gradient Benchmark Code
// Questions? Contact Michael A. Heroux (maherou@sandia.gov) 
// 
// ************************************************************************
//@HEADER

#ifdef DEBUG
#include <iostream>
using std::cout;
using std::cerr;
using std::endl;
#include <cstdlib>
#include <cstdio>
#include <cassert>
#endif

#include "OptimizeMatrix.hpp"
#include "mytimer.hpp"

#ifdef USING_MPI
#include <mpi.h>
#include <algorithm>
#include <map>
#include <set>
#include <vector>
#endif

void OptimizeMatrix(const Geometry & geom, SparseMatrix & A) {

#ifdef USING_MPI  // Compile this routine only if running in parallel
  double t0;
  int debug_details = 0; // Set to 1 for voluminous output
#ifdef DEBUG
  int debug = 1;
#else
  int debug = 0;
#endif
  
  // Extract Matrix pieces

  int localNumberOfRows = A.localNumberOfRows;
  int  * nonzerosInRow = A.nonzerosInRow;
  int ** matrixIndices = A.matrixIndices;
  
  // Scan global IDs of the nonzeros in the matrix.  Determine if the column ID matches a row ID.  If not:
  // 1) We call the getRankOfMatrixRow function, which tells us the rank of the processor owning the row ID.
  //	We need to receive this value of the x vector during the halo exchange.
  // 2) We record our row ID since we know that the other processor will need this value from us, due to symmetry.
  
  std::map< int, std::set< int> > sendList, receiveList;
  typedef std::map< int, std::set< int> >::iterator map_iter;
  typedef std::set<int>::iterator set_iter;
  std::map< int, int > externalToLocalMap;

	for (int i=0; i< localNumberOfRows; i++) {
		for (int j=0; j<nonzerosInRow[i]; j++) {
			int curIndex = matrixIndices[i][j];
			int rankIdOfColumnEntry = getRankOfMatrixRow(geom, A, curIndex);
			if (geom.rank!=rankIdOfColumnEntry) {// If column index is not a row index, then it comes from another processor
				receiveList[rankIdOfColumnEntry].insert(curIndex);
				sendList[rankIdOfColumnEntry].insert(i); // Matrix symmetry means we know the neighbor process wants my value
			}
		}
	}

	// Count number of matrix entries to send and receive
	int totalToBeSent = 0;
	for (map_iter curNeighbor = sendList.begin(); curNeighbor != sendList.end(); ++curNeighbor) {
		totalToBeSent += (curNeighbor->second).size();
	}
	int totalToBeReceived = 0;
	for (map_iter curNeighbor = receiveList.begin(); curNeighbor != receiveList.end(); ++curNeighbor) {
		totalToBeReceived += (curNeighbor->second).size();
	}

#ifdef DEBUG
	// These are all attributes that should be true, due to symmetry
	if (debug_details) cout << "totalToBeSent = " << totalToBeSent << " totalToBeReceived = " << totalToBeReceived << endl;
	assert(totalToBeSent==totalToBeReceived); // Number of sent entry should equal number of received
	assert(sendList.size()==receiveList.size()); // Number of send-to neighbors should equal number of receive-from
	// Each receive-from neighbor should be a send-to neighbor, and send the same number of entries
	for (map_iter curNeighbor = receiveList.begin(); curNeighbor != receiveList.end(); ++curNeighbor) {
		assert(sendList.find(curNeighbor->first)!=sendList.end());
		assert(sendList[curNeighbor->first].size()==receiveList[curNeighbor->first].size());
	}
#endif

	// Build the arrays and lists needed by the ExchangeHalo function.
	double * sendBuffer = new double[totalToBeSent];
	int * elementsToSend = new int[totalToBeSent];
	int * neighbors = new int[sendList.size()];
	int * receiveLength = new int[receiveList.size()];
	int * sendLength = new int[sendList.size()];
	int neighborCount = 0;
	int receiveEntryCount = 0;
	int sendEntryCount = 0;
	for (map_iter curNeighbor = receiveList.begin(); curNeighbor != receiveList.end(); ++curNeighbor, ++neighborCount) {
		int neighborId = curNeighbor->first; // rank of current neighbor we are processing
		neighbors[neighborCount] = neighborId; // store rank ID of current neighbor
		receiveLength[neighborCount] = receiveList[neighborId].size();
		sendLength[neighborCount] = sendList[neighborId].size(); // Get count if sends/receives
		for (set_iter i = receiveList[neighborId].begin(); i != receiveList[neighborId].end(); ++i, ++receiveEntryCount) {
			externalToLocalMap[*i] = localNumberOfRows + receiveEntryCount; // The remote columns are indexed at end of internals
		}
		for (set_iter i = sendList[neighborId].begin(); i != sendList[neighborId].end(); ++i, ++sendEntryCount) {
			elementsToSend[sendEntryCount] = A.globalToLocalMap[*i]; // store local ids of entry to send
		}
	}

// Convert matrix indices to local IDs
	for (int i=0; i< localNumberOfRows; i++) {
		for (int j=0; j<nonzerosInRow[i]; j++) {
			int curIndex = matrixIndices[i][j];
			int rankIdOfColumnEntry = getRankOfMatrixRow(geom, A, curIndex);
			if (geom.rank==rankIdOfColumnEntry) { // My column index, so convert to local index
				matrixIndices[i][j] = A.globalToLocalMap[curIndex];
			}
			else { // If column index is not a row index, then it comes from another processor
				matrixIndices[i][j] = externalToLocalMap[curIndex];
			}
		}
	}

	// Store contents in our matrix struct
	A.numberOfExternalValues = externalToLocalMap.size();
	A.localNumberOfColumns = A.localNumberOfRows + A.numberOfExternalValues;
	A.numberOfSendNeighbors = sendList.size();
	A.totalToBeSent = totalToBeSent;
	A.elementsToSend = elementsToSend;
	A.neighbors = neighbors;
	A.receiveLength = receiveLength;
	A.sendLength = sendLength;
	A.sendBuffer = sendBuffer;

#endif // USING_MPI
	return;
}
