
//@HEADER
// ************************************************************************
// 
//               HPCG: Simple Conjugate Gradient Benchmark Code
// Questions? Contact Michael A. Heroux (maherou@sandia.gov) 
// 
// ************************************************************************
//@HEADER

/////////////////////////////////////////////////////////////////////////

// Routine to one step of symmetrix Gauss-Seidel:

// A - known matrix 
// x - input vector
// y - On exit contains the result of one symmetric GS sweep with x as the RHS.
//
// Assumption about the structure of matrix A:
// - Each row 'i' of the matrix has nonzero diagonal value whose address is matrixDiagonal[i]
// - Entries in row 'i' are ordered such that:
//      - lower triangular terms are stored before the diagonal element.
//      - upper triangular terms are stored after the diagonal element.
//      - No other assumptions are made about entry ordering.

// Symmetric Gauss-Seidel notes:
// - We use the input vector x as the RHS and start with an initial guess for y of all zeros.
// - We perform one forward sweep.  Since y is initially zero we can ignore the upper triangular terms of A.
// - We then perform one back sweep.
//      - For simplicity we include the diagonal contribution in the for-j loop, then correct the sum after

/////////////////////////////////////////////////////////////////////////

#include "Geometry.hpp"
#include "symgs.hpp"

int symgs( const SparseMatrix & A, const double * const x, double * const y) {

  const int nrow = A.localNumberOfRows;
   double ** matrixDiagonal = A.matrixDiagonal;  // An array of pointers to the diagonal entries A.matrixValues

  for (int i=0; i< nrow; i++) {
      const double * const currentValues = A.matrixValues[i];
      const local_int_t * const currentColIndices = A.mtxIndL[i];
      const double * addressOfCurrentDiagonal = matrixDiagonal[i]; // Current diagonal value
      const int currentNumberOfNonzeros = addressOfCurrentDiagonal - currentValues;
      double sum = x[i]; // RHS value

      for (int j=0; j< currentNumberOfNonzeros; j++) {
    	  local_int_t curCol = currentColIndices[j];
    	  if (curCol<nrow) sum -= currentValues[j] * y[curCol];
      }

      y[i] = sum/(*addressOfCurrentDiagonal);
    }

  // Now the back sweep.

  for (int i=nrow-1; i>=0; i--) {
      const double * const currentValues = A.matrixValues[i];
      const local_int_t * const currentColIndices = A.mtxIndL[i];
      const int currentNumberOfNonzeros = A.nonzerosInRow[i];
      const double  currentDiagonal = matrixDiagonal[i][0]; // Current diagonal value
      double sum = x[i]; // RHS value

      for (int j = 0; j< currentNumberOfNonzeros; j++) {
    	  local_int_t curCol = currentColIndices[j];
    	  if (curCol<nrow) sum -= currentValues[j]*y[curCol];
      }
      sum += y[i]*currentDiagonal; // Remove diagonal contribution from previous loop

      y[i] = sum/currentDiagonal;
    }

  return(0);
}

