
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
#include <cmath>
#ifdef USING_MPI
#include <mpi.h> // If this routine is compiled with -DUSING_MPI then include mpi.h
#endif
#include "GenerateGeometry.hpp"
#include "GenerateProblem.hpp"
#include "ExchangeHalo.hpp"
#include "OptimizeMatrix.hpp" // Also include this function
#include "WriteProblem.hpp"
#include "ReportResults.hpp"
#include "mytimer.hpp"
#include "spmv.hpp"
#include "symgs.hpp"
#include "dot.hpp"
#include "ComputeResidual.hpp"
#include "Geometry.hpp"
#include "SparseMatrix.hpp"

#define TICK()  t0 = mytimer() // Use TICK and TOCK to time a code section
#define TOCK(t) t += mytimer() - t0

int main(int argc, char *argv[]) {

	Geometry geom;
	SparseMatrix A;
	double *x, *b, *xexact;
	double norm, d;
	int ierr = 0;
	int i, j;
	int ione = 1;
	std::vector< double > times(8,0.0);
	int nx,ny,nz;
    double t0 = 0.0, t1 = 0.0, t2 = 0.0, t3 = 0.0, t4 = 0.0, t5 = 0.0, t6 = 0.0, t7 = 0.0;

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
			<< "     where nx, ny and nz are the local sub-block dimensions, or" << endl;
		exit(1);
	}

	nx = atoi(argv[1]);
	ny = atoi(argv[2]);
	nz = atoi(argv[3]);
	GenerateGeometry(size, rank, nx, ny, nz, geom);
	GenerateProblem(geom, A, &x, &b, &xexact);

	//if (geom.size==1) WriteProblem(A, x, b, xexact);

	// Transform matrix indices from global to local values.
	// Define number of columns for the local matrix.

	t7 = mytimer(); OptimizeMatrix(geom, A);  t7 = mytimer() - t7;
	times[7] = t7;

	local_int_t nrow = A.localNumberOfRows;
	local_int_t ncol = A.localNumberOfColumns;

	double * x_overlap = new double [ncol]; // Overlapped copy of x vector
	double * y_overlap = new double [ncol]; // Overlapped copy of y vector
	double * b_computed = new double [nrow]; // Computed RHS vector

	// Test symmetry of matrix

	// First load vectors with random values
	for (int i=0; i<nrow; ++i) {
		x_overlap[i] = ((double) rand() / (RAND_MAX)) + 1;
		y_overlap[i] = ((double) rand() / (RAND_MAX)) + 1;
	}

	// Next, compute x'*A*y
#ifdef USING_MPI
	ExchangeHalo(A,y_overlap);
#endif
	ierr = spmv(A, y_overlap, b_computed); // b_computed = A*y_overlap
	if (ierr) cerr << "Error in call to spmv: " << ierr << ".\n" << endl;
	double xtAy = 0.0;
	ierr = dot(nrow, x_overlap, b_computed, &xtAy, t4); // b_computed = A*y_overlap
	if (ierr) cerr << "Error in call to dot: " << ierr << ".\n" << endl;

	// Next, compute y'*A*x
#ifdef USING_MPI
	ExchangeHalo(A,x_overlap);
#endif
	ierr = spmv(A, x_overlap, b_computed); // b_computed = A*y_overlap
	if (ierr) cerr << "Error in call to spmv: " << ierr << ".\n" << endl;
	double ytAx = 0.0;
	ierr = dot(nrow, y_overlap, b_computed, &ytAx, t4); // b_computed = A*y_overlap
	if (ierr) cerr << "Error in call to dot: " << ierr << ".\n" << endl;
	if (rank==0) cout << "Departure from symmetry for spmv abs(x'*A*y - y'*A*x)        = " << std::fabs(xtAy - ytAx) << endl;

	// Test symmetry of symmetric Gauss-Seidel

	// Compute x'*Minv*y
	TICK(); ierr = symgs(A, y_overlap, b_computed); TOCK(t5); // b_computed = Minv*y_overlap
	if (ierr) cerr << "Error in call to symgs: " << ierr << ".\n" << endl;
	double xtMinvy = 0.0;
	ierr = dot(nrow, x_overlap, b_computed, &xtMinvy, t4); // b_computed = A*y_overlap
	if (ierr) cerr << "Error in call to dot: " << ierr << ".\n" << endl;

	// Next, compute y'*Minv*x
	TICK(); ierr = symgs(A, x_overlap, b_computed); TOCK(t5); // b_computed = Minv*y_overlap
	if (ierr) cerr << "Error in call to symgs: " << ierr << ".\n" << endl;
	double ytMinvx = 0.0;
	ierr = dot(nrow, y_overlap, b_computed, &ytMinvx, t4); // b_computed = A*y_overlap
	if (ierr) cerr << "Error in call to dot: " << ierr << ".\n" << endl;
	if (rank==0) cout << "Departure from symmetry for symgs abs(x'*Minv*y - y'*Minv*x) = " << std::fabs(xtMinvy - ytMinvx) << endl;

	for (int i=0; i< nrow; ++i) x_overlap[i] = xexact[i]; // Copy exact answer into overlap vector

	t1 = mytimer();   // Initialize it (if needed)
	int numberOfCalls = 10;
	double residual = 0.0;
	double t_begin = mytimer();
	for (int i=0; i< numberOfCalls; ++i) {
#ifdef USING_MPI
		TICK(); ExchangeHalo(A,x_overlap); TOCK(t6);
#endif
		TICK(); ierr = spmv(A, x_overlap, b_computed); TOCK(t3); // b_computed = A*x_overlap
		if (ierr) cerr << "Error in call to spmv: " << ierr << ".\n" << endl;
		if ((ierr = ComputeResidual(A.localNumberOfRows, b, b_computed, &residual)))
			cerr << "Error in call to compute_residual: " << ierr << ".\n" << endl;
		if (rank==0) cout << "SpMV call [" << i << "] Residual [" << residual << "]" << endl;
	}
    times[0] += mytimer() - t_begin;  // Total time. All done...
    times[3] = t3; // spmv time
    times[5] = t5; // symgs time
#ifdef USING_MPI
		times[6] = t6; // exchange halo time
		times[7] = t7; // matrix set up time
#endif


	// Compute difference between known exact solution and computed solution
	// All processors are needed here.

	// Report results to YAML file
	ReportResults(geom, A, numberOfCalls, residual, &times[0]);

	// Clean up
	destroyMatrix(A);
	delete [] x;
	delete [] b;
	delete [] xexact;
	delete [] x_overlap;
	delete [] b_computed;

	// Finish up
#ifdef USING_MPI
	MPI_Finalize();
#endif
	return 0 ;
} 
