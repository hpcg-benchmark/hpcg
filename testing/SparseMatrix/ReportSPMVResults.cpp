//@HEADER
// ************************************************************************
//
//               HPCG: Simple Conjugate Gradient Benchmark Code
// Questions? Contact Michael A. Heroux (maherou@sandia.gov)
//
// ************************************************************************
//@HEADER
// Input
// size - Number of MPI processes
// rank - My process id
//

#include "ReportResults.hpp"
#include "YAML_Element.hpp"
#include "YAML_Doc.hpp"

#ifdef USING_MPI
#include <mpi.h> // If this routine is compiled with -DUSING_MPI then include mpi.h
#endif

#ifdef DEBUG
#include <fstream>
using std::endl;

#include "hpcg.hpp"
#endif

void ReportResults(const Geometry & geom, const SparseMatrix & A, int numcalls, double residual, double times[]) {
    
    // initialize YAML doc
    
    if (geom.rank==0) { // Only PE 0 needs to compute and report timing results

        double fniters = numcalls;
        double fnrow = A.totalNumberOfRows;
        double fnnz = A.totalNumberOfNonzeros;
        double fnops_sparsemv = fniters*2*fnnz;
        double fnops_precond = 2*3*fnnz; // Two calls and two GS sweeps, but only use lower triangle for first sweep
        double fnops = fnops_sparsemv+fnops_precond;
        
        YAML_Doc doc("hpcg-sparsematrix", "0.1");
        
        doc.add("Dimensions","");
        doc.get("Dimensions")->add("nx",geom.nx);
        doc.get("Dimensions")->add("ny",geom.ny);
        doc.get("Dimensions")->add("nz",geom.nz);
        
        
        
        doc.add("Number of SPMV calls: ", numcalls);
        doc.add("inf-norm of b_exact - b_computed: ", residual);
        doc.add("********** Performance Summary (times in sec) ***********","");
        
        doc.add("Time Summary","");
        doc.get("Time Summary")->add("Total   ",times[0]);
        doc.get("Time Summary")->add("SPARSEMV",times[3]);
        doc.get("Time Summary")->add("PRECOND ",times[5]);
        
        doc.add("FLOPS Summary","");
        doc.get("FLOPS Summary")->add("Total   ",fnops);
        doc.get("FLOPS Summary")->add("SPARSEMV",fnops_sparsemv);
        doc.get("FLOPS Summary")->add("PRECOND ",fnops_precond);
        
        doc.add("MFLOPS Summary","");
        doc.get("MFLOPS Summary")->add("Total   ",fnops/times[0]/1.0E6);
        doc.get("MFLOPS Summary")->add("SPARSEMV",fnops_sparsemv/(times[3])/1.0E6);
        doc.get("MFLOPS Summary")->add("PRECOND ",fnops_precond/(times[5])/1.0E6);
        
#ifdef USING_MPI
        
        double totalSparseMVTime = times[3] + times[5]+ times[6];
        doc.add("SPARSEMV OVERHEADS","");
        doc.get("SPARSEMV OVERHEADS")->add("SPARSEMV MFLOPS W OVERHEAD",fnops_sparsemv/(totalSparseMVTime)/1.0E6);
        doc.get("SPARSEMV OVERHEADS")->add("SPARSEMV PARALLEL OVERHEAD Time", (times[7]+times[6]));
        doc.get("SPARSEMV OVERHEADS")->add("SPARSEMV PARALLEL OVERHEAD Pct", (times[7]+times[6])/totalSparseMVTime*100.0);
        doc.get("SPARSEMV OVERHEADS")->add("SPARSEMV PARALLEL OVERHEAD Setup Time", (times[7]));
        doc.get("SPARSEMV OVERHEADS")->add("SPARSEMV PARALLEL OVERHEAD Setup Pct", (times[7])/totalSparseMVTime*100.0);
        doc.get("SPARSEMV OVERHEADS")->add("SPARSEMV PARALLEL OVERHEAD Bdry Exch Time", (times[6]));
        doc.get("SPARSEMV OVERHEADS")->add("SPARSEMV PARALLEL OVERHEAD Bdry Exch Pct", (times[6])/totalSparseMVTime*100.0);
#endif
        
            std::string yaml = doc.generateYAML();
#ifdef DEBUG
            HPCG_fout << yaml;
#endif
    }
	return;
}
