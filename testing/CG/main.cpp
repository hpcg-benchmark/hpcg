
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
#ifndef HPCG_NOMPI
#include <mpi.h> // If this routine is not compiled with HPCG_NOMPI
#endif

#ifndef HPCG_NOOPENMP
#include <omp.h> // If this routine is not compiled with HPCG_NOOPENMP
#endif

#include "hpcg.hpp"

#include "GenerateGeometry.hpp"
#include "GenerateProblem.hpp"
#include "SetupHalo.hpp"
#include "OptimizeProblem.hpp"
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


	int nx,ny,nz;
	nx = atoi(argv[1]);
	ny = atoi(argv[2]);
	nz = atoi(argv[3]);

	if (size*nx*ny*nz<=10) {
		if (rank==0)
			HPCG_fout << "This test requires a global problem size of 10 or more."
			<< "     Your size = " << size*nx*ny*nz << "." << endl;
		exit(1);

	}

    Geometry geom;
    GenerateGeometry(size, rank, numThreads, nx, ny, nz, geom);

    SparseMatrix A;
    CGData data;
    double *x, *b, *xexact;
    GenerateProblem(geom, A, &x, &b, &xexact);
    SetupHalo(geom, A);
    initializeCGData(A, data);

#ifdef HPCG_DEBUG
    if (rank==0) HPCG_fout << "Total setup time (sec) = " << mytimer() - t1 << endl;
#endif


    //if (geom.size==1) WriteProblem(A, x, b, xexact);

    // Use this array for collecting timing information
    std::vector< double > times(8,0.0);
    double t7 = 0.0;

    // Call user-tunable set up function.
    t7 = mytimer(); OptimizeProblem(geom, A, data, x, b, xexact); t7 = mytimer() - t7;
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
			int ierr = CG( geom, A, data, b, x, maxIters, tolerance, niters, normr, normr0, &times[0], k==1);
			if (ierr) HPCG_fout << "Error in call to CG: " << ierr << ".\n" << endl;
			if (rank==0) {
				HPCG_fout << "Call [" << i << "] Number of Iterations [" << niters <<"] Scaled Residual [" << normr/normr0 << "]";
				if (niters<=expected_niters) {
					HPCG_fout << ".  Result OK" << endl;
				}
				else {
					HPCG_fout << " Expected " << expected_niters << " iterations.  Performed " << niters << "." << endl;
				}
			}
		}
	}

	// Compute difference between known exact solution and computed solution
	// All processors are needed here.
#ifdef DEBUG
	double residual = 0;
	int ierr = ComputeResidual(A.localNumberOfRows, x, xexact, &residual);
	if (ierr) HPCG_fout << "Error in call to compute_residual: " << ierr << ".\n" << endl;
	if (rank==0)
		HPCG_fout << "Difference between computed and exact  = " << residual << ".\n" << endl;
#endif

	// Report results to YAML file
	ReportResults(geom, A, niters, normr, &times[0]);

	// Clean up
	destroyMatrix(A);
	destroyCGData(data);
	delete [] x;
	delete [] b;
	delete [] xexact;

	HPCG_Finalize();

	// Finish up
#ifndef HPCG_NOMPI
	MPI_Finalize();
#endif
	return 0 ;
}
