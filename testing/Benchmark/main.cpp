
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

#ifndef HPCG_NOMPI
#include <mpi.h> // If this routine is not compiled with HPCG_NOMPI
#endif

#ifndef HPCG_NOOPENMP
#include <omp.h> // If this routine is not compiled with HPCG_NOOPENMP
#endif

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

  int numThreads = 1;

#ifndef HPCG_NOOPENMP
#pragma omp parallel
  numThreads = omp_get_num_threads();
#endif

#ifdef DEBUG
    if (size < 100) cout << "Process "<<rank<<" of "<<size<<" is alive with " << numThreads << " threads." <<endl;
#endif
    
#ifdef DEBUG
    if (rank==0)
    {
        int junk = 0;
        cout << "Press enter to continue"<< endl;
        cin >> junk;
    }
#ifndef HPCG_NOMPI
    MPI_Barrier(MPI_COMM_WORLD);
#endif
#endif
    
    
    if(argc!=4) {
        if (rank==0)
            cerr << "Usage:" << endl
            << argv[0] << " nx ny nz" << endl
            << "     where nx, ny and nz are the local sub-block dimensions." << endl;
        exit(1);
    }
#ifdef NO_PRECONDITIONER
    bool doPreconditioning = false;
#else
    bool doPreconditioning = true;
#endif
    
    local_int_t nx,ny,nz;
    nx = atoi(argv[1]);
    ny = atoi(argv[2]);
    nz = atoi(argv[3]);

#ifdef HPCG_DEBUG
    double t1 = mytimer();
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
    if (rank==0) cout << "Total setup time (sec) = " << mytimer() - t1 << endl;
#endif


    //if (geom.size==1) WriteProblem(A, x, b, xexact);

    // Use this array for collecting timing information
    std::vector< double > times(8,0.0);
    double t7 = 0.0;

    // Call user-tunable set up function.
    t7 = mytimer(); OptimizeProblem(geom, A, data, x, b, xexact); t7 = mytimer() - t7;
    times[7] = t7;
    
    int niters = 0;
    int totalNiters = 0;
    double normr = 0.0;
    double normr0 = 0.0;
    int maxIters = 10;
    int numberOfCgCalls = 10;
    double tolerance = 0.0; // Set tolerance to zero to make all runs do max_iter iterations
    for (int i=0; i< numberOfCgCalls; ++i) {
    	for (int j=0; j< A.localNumberOfRows; ++j) x[j] = 0.0; // Zero out x
    	int ierr = CG( geom, A, data, b, x, maxIters, tolerance, niters, normr, normr0, &times[0], doPreconditioning);
    	if (ierr) cerr << "Error in call to CG: " << ierr << ".\n" << endl;
    	if (rank==0) cout << "Call [" << i << "] Scaled Residual [" << normr/normr0 << "]" << endl;
	totalNiters += niters;
    }
    
    // Compute difference between known exact solution and computed solution
    // All processors are needed here.
#ifdef DEBUG
    double residual = 0;
    int ierr = ComputeResidual(A.localNumberOfRows, x, xexact, &residual);
    if (ierr) cerr << "Error in call to compute_residual: " << ierr << ".\n" << endl;
    if (rank==0)
    cout << "Difference between computed and exact  = " << residual << ".\n" << endl;
#endif

    // Report results to YAML file
    ReportResults(geom, A, totalNiters, normr/normr0, &times[0]);

    // Clean up
    destroyMatrix(A);
    destroyCGData(data);
    delete [] x;
    delete [] b;
    delete [] xexact;
    
    // Finish up
#ifndef HPCG_NOMPI
    MPI_Finalize();
#endif
    return 0 ;
} 
