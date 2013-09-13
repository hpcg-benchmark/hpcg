
//@HEADER
// ************************************************************************
// 
//               HPCG: Simple Conjugate Gradient Benchmark Code
// Questions? Contact Michael A. Heroux (maherou@sandia.gov) 
// 
// ************************************************************************
//@HEADER

/////////////////////////////////////////////////////////////////////////

// Routine to dump:
// - matrix in row, col, val format for analysis with Matlab
// - x, xexact, b as simple arrays of numbers.
//
// Writes to A.dat, x.dat, xexact.dat and b.dat, respectivly.
// NOTE:  THIS CODE ONLY WORKS ON SINGLE PROCESSOR RUNS
// Read into matlab using:
//   load A.dat
//   A=spconvert(A);
//   load x.dat
//   load xexact.dat
//   load b.dat

// A - known matrix 
// x - Initial guess to solution
// xexact - Generated exact solution
// b - Right hand side

// Returns with -1 if used with more than one MPI process.
// Returns with 0 otherwise.

/////////////////////////////////////////////////////////////////////////

#include <cstdio>
#include "WriteProblem.hpp"
#include "Geometry.hpp"
int WriteProblem( const Geometry & geom, const SparseMatrix & A,
			const double * const b, const double * const x, const double * const xexact) {

	if (geom.size!=1) return(-1); //TODO Only works on one processor.  Need better error handler
	const global_int_t nrow = A.totalNumberOfRows;

	FILE * fA = 0, * fx = 0, * fxexact = 0, * fb = 0;
    fA = fopen("A.dat", "w");
    fx = fopen("x.dat", "w");
    fxexact = fopen("xexact.dat", "w");
    fb = fopen("b.dat", "w");

  for (global_int_t i=0; i< nrow; i++) {
      const double * const currentRowValues = A.matrixValues[i];
      const global_int_t * const currentRowIndices = A.mtxIndG[i];
      const int currentNumberOfNonzeros = A.nonzerosInRow[i];
      for (int j=0; j< currentNumberOfNonzeros; j++)
#ifdef HPCG_NO_LONG_LONG
    	  fprintf(fA, " %d %d %22.16e\n",i+1,(global_int_t)(currentRowIndices[j]+1),currentRowValues[j]);
#else
	  	  fprintf(fA, " %d %lld %22.16e\n",i+1,(global_int_t)(currentRowIndices[j]+1),currentRowValues[j]);
#endif
      fprintf(fx, "%22.16e\n",x[i]);
      fprintf(fxexact, "%22.16e\n",xexact[i]);
      fprintf(fb, "%22.16e\n",b[i]);
    }

  fclose(fA);
  fclose(fx);
  fclose(fxexact);
  fclose(fb);
  return(0);
}
