
//@HEADER
// ************************************************************************
// 
//               HPCG: Simple Conjugate Gradient Benchmark Code
// Questions? Contact Michael A. Heroux (maherou@sandia.gov) 
// 
// ************************************************************************
//@HEADER

/////////////////////////////////////////////////////////////////////////

// Routine to compute matrix vector product y = Ax where:
// First call exchange_externals to get off-processor values of x

// A - known matrix 
// x - known vector
// y - On exit contains Ax.

/////////////////////////////////////////////////////////////////////////

#include "spmv.hpp"

int spmv( const SparseMatrix & A, const double * const x, double * const y) {

	const int nrow = A.localNumberOfRows;

	for (int i=0; i< nrow; i++)  {
		double sum = 0.0;
		const double * const cur_vals = A.matrixValues[i];
		const local_int_t * const cur_inds = A.mtxIndL[i];
		const int cur_nnz = A.nonzerosInRow[i];

		for (int j=0; j< cur_nnz; j++)
			sum += cur_vals[j]*x[cur_inds[j]];
		y[i] = sum;
	}
	return(0);
}
