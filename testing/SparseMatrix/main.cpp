
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

#include <fstream>
#include <iostream>
using std::cin;
using std::endl;
#include <cstdlib>
#include <vector>
#include <cmath>
#ifndef HPCG_NOMPI
#include <mpi.h> // If this routine is not compiled with -DHPCG_NOMPI then include mpi.h
#endif

#ifndef HPCG_NOOPENMP
#include <omp.h> // If this routine is not compiled with HPCG_NOOPENMP
#endif

#include "hpcg.hpp"

#include "GenerateGeometry.hpp"
#include "GenerateProblem.hpp"
#include "SetupHalo.hpp"
#include "ExchangeHalo.hpp"
#include "OptimizeProblem.hpp"
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

#ifndef HPCG_NOMPI

	MPI_Init(&argc, &argv);
	int size, rank; // Number of MPI processes, My process ID
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

#else

	int size = 1; // Serial case (not using MPI)
	int rank = 0;

#endif

	HPCG_Init();

  int numThreads = 1;

#ifndef HPCG_NOOPENMP
#pragma omp parallel
  numThreads = omp_get_num_threads();
#endif

#ifdef DEBUG
    if (size < 100) HPCG_fout << "Process "<<rank<<" of "<<size<<" is alive with " << numThreads << " threads." <<endl;
#endif

#ifdef DEBUG
	if (rank==0)
	{
		int junk = 0;
		HPCG_fout << "Press enter to continue"<< endl;
		cin >> junk;
	}
#ifndef HPCG_NOMPI
	MPI_Barrier(MPI_COMM_WORLD);
#endif
#endif


    local_int_t nx,ny,nz;
	nx = atoi(argv[1]);
	ny = atoi(argv[2]);
	nz = atoi(argv[3]);
#ifdef HPCG_DEBUG
    double tsetup = mytimer();
#endif

    Geometry geom;
    GenerateGeometry(size, rank, numThreads, nx, ny, nz, geom);

    SparseMatrix A;
    CGData data;
    double *x, *b, *xexact;
    GenerateProblem(geom, A, &x, &b, &xexact);
    SetupHalo(geom, A);
    initializeCGData(A, data);

#ifdef HPCG_DEBUG
    if (rank==0) HPCG_fout << "Total setup time (sec) = " << mytimer() - tsetup << endl;
#endif


    //if (geom.size==1) WriteProblem(A, x, b, xexact);

    // Use this array for collecting timing information
    std::vector< double > times(8,0.0);
    double t0 = 0.0, t1 = 0.0, t2 = 0.0, t3 = 0.0, t4 = 0.0, t5 = 0.0, t6 = 0.0, t7 = 0.0;

    // Call user-tunable set up function.
    t7 = mytimer(); OptimizeProblem(geom, A, data, x, b, xexact); t7 = mytimer() - t7;
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
#ifndef HPCG_NOMPI
	ExchangeHalo(A,y_overlap);
#endif
	int ierr = spmv(A, y_overlap, b_computed); // b_computed = A*y_overlap
	if (ierr) HPCG_fout << "Error in call to spmv: " << ierr << ".\n" << endl;
	double xtAy = 0.0;
	ierr = dot(nrow, x_overlap, b_computed, &xtAy, t4); // b_computed = A*y_overlap
	if (ierr) HPCG_fout << "Error in call to dot: " << ierr << ".\n" << endl;

	// Next, compute y'*A*x
#ifndef HPCG_NOMPI
	ExchangeHalo(A,x_overlap);
#endif
	ierr = spmv(A, x_overlap, b_computed); // b_computed = A*y_overlap
	if (ierr) HPCG_fout << "Error in call to spmv: " << ierr << ".\n" << endl;
	double ytAx = 0.0;
	ierr = dot(nrow, y_overlap, b_computed, &ytAx, t4); // b_computed = A*y_overlap
	if (ierr) HPCG_fout << "Error in call to dot: " << ierr << ".\n" << endl;
	if (rank==0) HPCG_fout << "Departure from symmetry for spmv abs(x'*A*y - y'*A*x)        = " << std::fabs(xtAy - ytAx) << endl;

	// Test symmetry of symmetric Gauss-Seidel

	// Compute x'*Minv*y
	TICK(); ierr = symgs(A, y_overlap, b_computed); TOCK(t5); // b_computed = Minv*y_overlap
	if (ierr) HPCG_fout << "Error in call to symgs: " << ierr << ".\n" << endl;
	double xtMinvy = 0.0;
	ierr = dot(nrow, x_overlap, b_computed, &xtMinvy, t4); // b_computed = A*y_overlap
	if (ierr) HPCG_fout << "Error in call to dot: " << ierr << ".\n" << endl;

	// Next, compute y'*Minv*x
	TICK(); ierr = symgs(A, x_overlap, b_computed); TOCK(t5); // b_computed = Minv*y_overlap
	if (ierr) HPCG_fout << "Error in call to symgs: " << ierr << ".\n" << endl;
	double ytMinvx = 0.0;
	ierr = dot(nrow, y_overlap, b_computed, &ytMinvx, t4); // b_computed = A*y_overlap
	if (ierr) HPCG_fout << "Error in call to dot: " << ierr << ".\n" << endl;
	if (rank==0) HPCG_fout << "Departure from symmetry for symgs abs(x'*Minv*y - y'*Minv*x) = " << std::fabs(xtMinvy - ytMinvx) << endl;

	for (int i=0; i< nrow; ++i) x_overlap[i] = xexact[i]; // Copy exact answer into overlap vector

	t1 = mytimer();   // Initialize it (if needed)
	int numberOfCalls = 10;
	double residual = 0.0;
	double t_begin = mytimer();
	for (int i=0; i< numberOfCalls; ++i) {
#ifndef HPCG_NOMPI
		TICK(); ExchangeHalo(A,x_overlap); TOCK(t6);
#endif
		TICK(); ierr = spmv(A, x_overlap, b_computed); TOCK(t3); // b_computed = A*x_overlap
		if (ierr) HPCG_fout << "Error in call to spmv: " << ierr << ".\n" << endl;
		if ((ierr = ComputeResidual(A.localNumberOfRows, b, b_computed, &residual)))
			HPCG_fout << "Error in call to compute_residual: " << ierr << ".\n" << endl;
		if (rank==0) HPCG_fout << "SpMV call [" << i << "] Residual [" << residual << "]" << endl;
	}
    times[0] += mytimer() - t_begin;  // Total time. All done...
    times[3] = t3; // spmv time
    times[5] = t5; // symgs time
#ifndef HPCG_NOMPI
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

	HPCG_Finalize();

	// Finish up
#ifndef HPCG_NOMPI
	MPI_Finalize();
#endif
	return 0 ;
} 
