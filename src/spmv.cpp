
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
// Precondition: First call exchange_externals to get off-processor values of x

// A - known matrix 
// x - known vector
// y - On exit contains Ax.

// This routine calls the reference spmv implementation by default, but
// can be replaced by a custom, optimized routine suited for
// the target system.

/////////////////////////////////////////////////////////////////////////

#include "spmv.hpp"
#include "spmvref.hpp"

int spmv( const SparseMatrix & A, const double * const x, double * const y) {

	return(spmvref(A, x, y));

}
