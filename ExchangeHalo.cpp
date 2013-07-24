
//@HEADER
// ************************************************************************
// 
//               HPCG: Simple Conjugate Gradient Benchmark Code
// Questions? Contact Michael A. Heroux (maherou@sandia.gov) 
// 
// ************************************************************************
//@HEADER

#ifdef USING_MPI  // Compile this routine only if running in parallel
#include <mpi.h>
#include "Geometry.hpp"
#include "ExchangeHalo.hpp"
#include <cstdlib>

void ExchangeHalo(const SparseMatrix & A, const double *x) {
  int numberOfExternalValues = 0;

  // Extract Matrix pieces

  int localNumberOfRows = A.localNumberOfRows;
  int num_neighbors = A.numberOfSendNeighbors;
  int * receiveLength = A.receiveLength;
  int * sendLength = A.sendLength;
  int * neighbors = A.neighbors;
  double * sendBuffer = A.sendBuffer;
  int totalToBeSent = A.totalToBeSent;
  int * elementsToSend = A.elementsToSend;
  
  int size, rank; // Number of MPI processes, My process ID
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  //
  //  first post receives, these are immediate receives
  //  Do not wait for result to come, will do that at the
  //  wait call below.
  //

  int MPI_MY_TAG = 99;  

  MPI_Request * request = new MPI_Request[num_neighbors];

  //
  // Externals are at end of locals
  //
  double *x_external = (double *) x + localNumberOfRows;

  // Post receives first 
  for (int i = 0; i < num_neighbors; i++) {
      int n_recv = receiveLength[i];
      MPI_Irecv(x_external, n_recv, MPI_DOUBLE, neighbors[i], MPI_MY_TAG, MPI_COMM_WORLD, request+i);
      x_external += n_recv;
    }


  //
  // Fill up send buffer
  //

  for (int i=0; i<totalToBeSent; i++) sendBuffer[i] = x[elementsToSend[i]];

  //
  // Send to each neighbor
  //

  for (int i = 0; i < num_neighbors; i++) {
      int n_send = sendLength[i];
      MPI_Send(sendBuffer, n_send, MPI_DOUBLE, neighbors[i], MPI_MY_TAG, MPI_COMM_WORLD);
      sendBuffer += n_send;
    }

  //
  // Complete the reads issued above
  //

  MPI_Status status;
  for (int i = 0; i < num_neighbors; i++) {
      if ( MPI_Wait(request+i, &status) ) {
	  std::exit(-1); // TODO: have better error exit
	}
    }

  delete [] request;

  return;
}
#endif // USING_MPI
