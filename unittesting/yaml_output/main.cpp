
#include <GenerateGeometry.hpp>
#include <ReportResults.hpp>

int
main() {
  int comm_size = 1, comm_rank = 0, numThreads = 1;
  int nx = 13, ny = 17, nz = 19;
  Geometry geom;
  SparseMatrix A, *Af;
  int numberOfMgLevels = 4;
  int numberOfCgSets = 97;
  int refMaxIters = 53;
  int optMaxIters = 57;
  double times[8] = {1000.5, 1001.5, 1002.5, 1003.5, 1004.5, 1005.5, 1006.5, 1007.5};
  TestCGData testcg_data;
  TestSymmetryData testsymmetry_data;
  TestNormsData testnorms_data;
  int global_failure;

  GenerateGeometry( comm_size, comm_rank, numThreads, nx, ny, nz, &geom);

  Af = &A;
  for (int i=1; i<numberOfMgLevels; ++i) {
    Af->geom = &geom;

    MGData *mgData = new MGData();
    mgData->numberOfPresmootherSteps = 1;
    mgData->numberOfPostsmootherSteps = 1;
    Af->mgData = mgData;

    Af->Ac = new SparseMatrix();
    Af = Af->Ac;
  }

  ReportResults( A, numberOfMgLevels, numberOfCgSets, refMaxIters, optMaxIters, times,
    testcg_data, testsymmetry_data, testnorms_data, global_failure );

  return 0;
}
