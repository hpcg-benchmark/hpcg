
//@HEADER
// ************************************************************************
//
//               HPCG: Simple Conjugate Gradient Benchmark Code
// Questions? Contact Michael A. Heroux (maherou@sandia.gov)
//
// ************************************************************************
//@HEADER

// Changelog
//
// Version 0.3
// - Added timing of setup time for sparse MV
// - Corrected percentages reported for sparse MV with overhead
//
/////////////////////////////////////////////////////////////////////////

// Main routine of a program that calls the HPCG conjugate gradient
// solver to solve the problem, and then prints results.

#include <iostream>
using std::cout;
using std::cin;
using std::cerr;
using std::endl;
#include <cstdlib>
#include <vector>
#ifdef USING_MPI
#include <mpi.h> // If this routine is compiled with -DUSING_MPI then include mpi.h
#endif
#include "GenerateGeometry.hpp"
#include "GenerateProblem.hpp"
#include "OptimizeMatrix.hpp" // Also include this function
#include "WriteProblem.hpp"
#include "ReportResults.hpp"
#include "mytimer.hpp"
#include "spmv.hpp"
#include "ComputeResidual.hpp"
#include "CG.hpp"
#include "Geometry.hpp"
#include "SparseMatrix.hpp"
#include "CGData.hpp"

int main(int argc, char *argv[]) {

	Geometry geom;
	SparseMatrix A;
	CGData data;
	double *x, *b, *xexact;
	double norm, d;
	int ierr = 0;
	std::vector< double > times(8,0.0);
	double t7 = 0.0;
	int nx,ny,nz;

#ifdef USING_MPI

	MPI_Init(&argc, &argv);
	int size, rank; // Number of MPI processes, My process ID
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

#ifdef DEBUG
	if (size < 100) cout << "Process "<<rank<<" of "<<size<<" is alive." <<endl;
#endif

#else

	int size = 1; // Serial case (not using MPI)
	int rank = 0;

#endif


#ifdef DEBUG
	if (rank==0)
	{
		int junk = 0;
		cout << "Press enter to continue"<< endl;
		cin >> junk;
	}
#ifdef USING_MPI
	MPI_Barrier(MPI_COMM_WORLD);
#endif
#endif


	if(argc!=4) {
		if (rank==0)
			cerr << "Usage:" << endl
			<< argv[0] << " nx ny nz" << endl
			<< "     where nx, ny and nz are the local sub-block dimensions" << endl;
		exit(1);
	}

	nx = atoi(argv[1]);
	ny = atoi(argv[2]);
	nz = atoi(argv[3]);
	if (size*nx*ny*nz<=10) {
		if (rank==0)
			cerr << "This test requires a global problem size of 10 or more."
			<< "     Your size = " << size*nx*ny*nz << "." << endl;
		exit(1);

	}
	GenerateGeometry(size, rank, nx, ny, nz, geom);
	GenerateProblem(geom, A, &x, &b, &xexact);

	//if (geom.size==1) WriteProblem(A, x, b, xexact);

	// Transform matrix indices from global to local values.
	// Define number of columns for the local matrix.

	t7 = mytimer(); OptimizeMatrix(geom, A);  initializeCGData(A, data); t7 = mytimer() - t7;
	times[7] = t7;

	// Modify the matrix diagonal to greatly exaggerate diagonal values
	for (int i=0; i< A.localNumberOfRows; ++i) {
		global_int_t globalRowID = A.localToGlobalMap[i];
		if (globalRowID<9) {
			*(A.matrixDiagonal[i]) *= (globalRowID+2)*1.0e6; // Multiply the first 9 diagonal values by RowID+2 times 1M.
			b[i] *= (globalRowID+1)*1.0e6;
		}
		else {
			*(A.matrixDiagonal[i]) *= 1.0e6; // The rest are multiplied by 1M.
			b[i] *= 1.0e6;

		}
	}
	double t1 = mytimer();   // Initialize it (if needed)
	int niters = 0;
	double normr = 0.0;
	double normr0 = 0.0;
	int maxIters = 100;
	int numberOfCgCalls = 10;
	double tolerance = 1.0e-12; // Set tolerance to reasonable value for grossly scaled diagonal terms
	for (int k=0; k<2; ++k) { // This loop tests both unpreconditioned and preconditioned runs
		int expected_niters = 1;
		if (k==0) expected_niters = 11;
		for (int i=0; i< numberOfCgCalls; ++i) {
			for (int j=0; j< A.localNumberOfRows; ++j) x[j] = 0.0; // Zero out x
			ierr = CG( geom, A, data, b, x, maxIters, tolerance, niters, normr, normr0, &times[0], k==1);
			if (ierr) cerr << "Error in call to CG: " << ierr << ".\n" << endl;
			if (rank==0) {
				cout << "Call [" << i << "] Number of Iterations [" << niters <<"] Scaled Residual [" << normr/normr0 << "]";
				if (niters==expected_niters) {
					cout << ".  Result OK" << endl;
				}
				else {
					cout << "Expected " << expected_niters << " iterations.  Performed " << niters << "." << endl;
				}
			}
		}
	}

	// Compute difference between known exact solution and computed solution
	// All processors are needed here.
#ifdef DEBUG
	double residual = 0;
	if ((ierr = ComputeResidual(A.localNumberOfRows, x, xexact, &residual)))
		cerr << "Error in call to compute_residual: " << ierr << ".\n" << endl;
	if (rank==0)
		cout << "Difference between computed and exact  = " << residual << ".\n" << endl;
#endif

	// Report results to YAML file
	ReportResults(geom, A, niters, normr, &times[0]);

	// Clean up
	destroyMatrix(A);
	destroyCGData(data);
	delete [] x;
	delete [] b;
	delete [] xexact;

	// Finish up
#ifdef USING_MPI
	MPI_Finalize();
#endif
	return 0 ;
}
