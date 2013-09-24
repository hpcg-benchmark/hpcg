//@HEADER
// ************************************************************************
//
//               HPCG: Simple Conjugate Gradient Benchmark Code
// Questions? Contact Michael A. Heroux (maherou@sandia.gov)
//
// ************************************************************************
//@HEADER

/*!
 @file ReportResults.cpp

 HPCG routine
 */

#include "ReportResults.hpp"
#include "YAML_Element.hpp"
#include "YAML_Doc.hpp"

#ifndef HPCG_NOMPI
#include <mpi.h> // If this routine is not compiled with HPCG_NOMPI
#endif

#ifdef HPCG_DEBUG
#include <fstream>
using std::endl;

#include "hpcg.hpp"
#endif

/*!
 Creates a YAML file and writes the information about the HPCG run, its results, and validity.

  @param[in] geom The description of the problem's geometry.
  @param[in] A    The known system matrix
  @param[in] numberOfCgSets Number of CG runs performed
  @param[in] niters Number of preconditioned CG iterations performed to lower the residual below a threshold
  @param[in] times  Vector of cumulative timings for each of the phases of a preconditioned CG iteration
  @param[in] testcg_data    the data structure with the results of the CG-correctness test including pass/fail information
  @param[in] symtest_data   the data structure with the results of the CG symmetry test including pass/fail information
  @param[in] testnorms_data the data structure with the results of the CG norm test including pass/fail information
  @param[in] global_failure indicates whether a failure occured during the correctness tests of CG

  @see YAML_Doc
*/
void ReportResults(const Geometry & geom, const SparseMatrix & A, int numberOfCgSets, int niters, double times[],
  TestCGData * testcg_data, SymTestData * symtest_data, TestNormsData * testnorms_data, int global_failure) {

#ifndef HPCG_NOMPI
    double t4 = times[4];
    double t4min = 0.0;
    double t4max = 0.0;
    double t4avg = 0.0;
    MPI_Allreduce(&t4, &t4min, 1, MPI_DOUBLE, MPI_MIN, MPI_COMM_WORLD);
    MPI_Allreduce(&t4, &t4max, 1, MPI_DOUBLE, MPI_MAX, MPI_COMM_WORLD);
    MPI_Allreduce(&t4, &t4avg, 1, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);
    t4avg = t4avg/((double) geom.size);
#endif
    
    // initialize YAML doc
    
    if (geom.rank==0) { // Only PE 0 needs to compute and report timing results

    	double fNumberOfCgSets = numberOfCgSets;
        double fniters = niters;
        double fnrow = A.totalNumberOfRows;
        double fnnz = A.totalNumberOfNonzeros;

        // Op counts come from implementation of CG in CG.cpp
        double fnops_ddot = fniters*6.0*fnrow; // 3 ddots with nrow adds and nrow mults
        double fnops_waxpby = fniters*6.0*fnrow; // 3 waxpbys with nrow adds and nrow mults
        double fnops_sparsemv = fniters*2.0*fnnz; // 1 spmv with nnz adds and nnz mults
        double fnops_precond = fniters*3.0*fnnz; // Two GS sweeps, but only use lower triangle for first sweep
        double fnops = fnops_ddot+fnops_waxpby+fnops_sparsemv+fnops_precond;
        
        YAML_Doc doc("HPCG-Benchmark", "0.2");
        doc.add("HPCG Benchmark","Version 0.2 September 20, 2013");
        
        doc.add("Machine Summary","");
        doc.get("Machine Summary")->add("Distributed Processes",geom.size);
        doc.get("Machine Summary")->add("Threads per processes",geom.numThreads);

        doc.add("Global Problem Dimensions","");
        doc.get("Global Problem Dimensions")->add("Global nx",geom.npx*geom.nx);
        doc.get("Global Problem Dimensions")->add("Global ny",geom.npy*geom.ny);
        doc.get("Global Problem Dimensions")->add("Global nz",geom.npz*geom.nz);

        doc.add("Processor Dimensions","");
        doc.get("Processor Dimensions")->add("npx",geom.npx);
        doc.get("Processor Dimensions")->add("npy",geom.npy);
        doc.get("Processor Dimensions")->add("npz",geom.npz);
        
        doc.add("Local Domain Dimensions","");
        doc.get("Local Domain Dimensions")->add("nx",geom.nx);
        doc.get("Local Domain Dimensions")->add("ny",geom.ny);
        doc.get("Local Domain Dimensions")->add("nz",geom.nz);
        
        
        doc.add("Linear System Information","");
        doc.get("Linear System Information")->add("Number of Equations",A.totalNumberOfRows);
        doc.get("Linear System Information")->add("Number of Nonzero Terms",A.totalNumberOfNonzeros);

        doc.add("********** Validation Testing Summary  ***********","");
        doc.add("Spectral Convergence Tests","");
        if (testcg_data->count_fail==0)
            doc.get("Spectral Convergence Tests")->add("Result", "PASSED");
        else
        	doc.get("Spectral Convergence Tests")->add("Result", "FAILED");
        doc.get("Spectral Convergence Tests")->add("Unpreconditioned","");
        doc.get("Spectral Convergence Tests")->get("Unpreconditioned")->add("Maximum iteration count", testcg_data->niters_max_no_prec);
        doc.get("Spectral Convergence Tests")->get("Unpreconditioned")->add("Expected iteration count", testcg_data->expected_niters_no_prec);
        doc.get("Spectral Convergence Tests")->add("Preconditioned","");
        doc.get("Spectral Convergence Tests")->get("Preconditioned")->add("Maximum iteration count", testcg_data->niters_max_prec);
        doc.get("Spectral Convergence Tests")->get("Preconditioned")->add("Expected iteration count", testcg_data->expected_niters_prec);

        doc.add("Departure from Symmetry (x'Ay-y'Ax)","");
        if (symtest_data->count_fail==0)
            doc.get("Departure from Symmetry (x'Ay-y'Ax)")->add("Result", "PASSED");
        else
        	doc.get("Departure from Symmetry (x'Ay-y'Ax)")->add("Result", "FAILED");
        doc.get("Departure from Symmetry (x'Ay-y'Ax)")->add("Departure for SPMV", symtest_data->depsym_spmv);
        doc.get("Departure from Symmetry (x'Ay-y'Ax)")->add("Departure for SYMGS", symtest_data->depsym_symgs);

        doc.add("********** Iterations Summary  ***********","");
        doc.add("Iteration Count Information","");
        if (!global_failure)
            doc.get("Iteration Count Information")->add("Result", "PASSED");
        else
        	doc.get("Iteration Count Information")->add("Result", "FAILED");
        doc.get("Iteration Count Information")->add("Number of CG sets", numberOfCgSets);
        doc.get("Iteration Count Information")->add("Average iterations per set", fniters/fNumberOfCgSets);
        doc.get("Iteration Count Information")->add("Total number of iterations", niters);

        doc.add("********** Reproducibility Summary  ***********","");
        doc.add("Reproducibility Information","");
        if (testnorms_data->pass)
             doc.get("Reproducibility Information")->add("Result", "PASSED");
         else
         	doc.get("Reproducibility Information")->add("Result", "FAILED");
        doc.get("Reproducibility Information")->add("Scaled residual mean", testnorms_data->mean);
        doc.get("Reproducibility Information")->add("Scaled residual variance", testnorms_data->variance);

        doc.add("********** Performance Summary (times in sec) ***********","");
        
        doc.add("Benchmark Time Summary","");
        doc.get("Benchmark Time Summary")->add("Optimization phase",times[7]);
        doc.get("Benchmark Time Summary")->add("DDOT",times[1]);
        doc.get("Benchmark Time Summary")->add("WAXPBY",times[2]);
        doc.get("Benchmark Time Summary")->add("SpMV",times[3]);
        doc.get("Benchmark Time Summary")->add("SymGS",times[5]);
        doc.get("Benchmark Time Summary")->add("Total",times[0]);
        
        doc.add("Floating Point Operations Summary","");
        doc.get("Floating Point Operations Summary")->add("DDOT",fnops_ddot);
        doc.get("Floating Point Operations Summary")->add("WAXPBY",fnops_waxpby);
        doc.get("Floating Point Operations Summary")->add("SpMV",fnops_sparsemv);
        doc.get("Floating Point Operations Summary")->add("SymGS",fnops_precond);
        doc.get("Floating Point Operations Summary")->add("Total",fnops);

        doc.add("GFLOP/s Summary","");
        doc.get("GFLOP/s Summary")->add("DDOT",fnops_ddot/times[1]/1.0E9);
        doc.get("GFLOP/s Summary")->add("WAXPBY",fnops_waxpby/times[2]/1.0E9);
        doc.get("GFLOP/s Summary")->add("SpMV",fnops_sparsemv/(times[3])/1.0E9);
        doc.get("GFLOP/s Summary")->add("SymGS",fnops_precond/(times[5])/1.0E9);
        doc.get("GFLOP/s Summary")->add("Total",fnops/times[0]/1.0E9);
        // This final GFLOP/s rating includes the overhead of optimizing the data structures vs 50 iterations of CG
        double totalGflops = fnops/(times[0]+fNumberOfCgSets*times[7])/1.0E9;
        doc.get("GFLOP/s Summary")->add("Total with Optimization phase overhead",totalGflops);

        double totalSparseMVTime = times[3] + times[6];
         doc.add("Sparse Operations Overheads","");
         doc.get("Sparse Operations Overheads")->add("SpMV GFLOP/s with overhead",fnops_sparsemv/(totalSparseMVTime)/1.0E9);
         doc.get("Sparse Operations Overheads")->add("Overhead time (sec)", (times[7]+times[6]));
         doc.get("Sparse Operations Overheads")->add("Overhead as percentage of time", (times[7]+times[6])/totalSparseMVTime*100.0);
         doc.get("Sparse Operations Overheads")->add("Optimization phase time (sec)", (times[7]));
         doc.get("Sparse Operations Overheads")->add("Optimization phase time vs reference SpMV+SymGS time", (times[7])/times[8]);

 #ifndef HPCG_NOMPI
        doc.get("Sparse Operations Overheads")->add("DDOT Timing Variations","");
        doc.get("Sparse Operations Overheads")->get("DDOT Timing Variations")->add("Min DDOT MPI_Allreduce time",t4min);
        doc.get("Sparse Operations Overheads")->get("DDOT Timing Variations")->add("Max DDOT MPI_Allreduce time",t4max);
        doc.get("Sparse Operations Overheads")->get("DDOT Timing Variations")->add("Avg DDOT MPI_Allreduce time",t4avg);
        
        doc.get("Sparse Operations Overheads")->add("Halo exchange time (sec)", (times[6]));
        doc.get("Sparse Operations Overheads")->add("Halo exchange as percentage of SpMV time", (times[6])/totalSparseMVTime*100.0);
#endif
        doc.add("********** Final Summary **********","");
        bool isValidRun = (testcg_data->count_fail==0) && (symtest_data->count_fail==0) && (testnorms_data->pass);
        if (isValidRun) {
        	doc.get("********** Final Summary **********")->add("HPCG result is VALID with a GFLOP/s rating of", totalGflops);
        	doc.get("********** Final Summary **********")->add("Please send the YAML file contents to","HPCG-Results@software.sandia.gov");
        }
        else {
        	doc.get("********** Final Summary **********")->add("HPCG result is","INVALID.");
        	doc.get("********** Final Summary **********")->add("Please review the YAML file contents","You may NOT submit these results for consideration.");
        }
        
            std::string yaml = doc.generateYAML();
#ifdef HPCG_DEBUG
            HPCG_fout << yaml;
#endif
    }
	return;
}
