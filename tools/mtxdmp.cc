
#include "CheckAspectRatio.hpp"
#include "GenerateGeometry.hpp"
#include "GenerateProblem.hpp"
#include "GenerateCoarseProblem.hpp"
#include "SetupHalo.hpp"
#include "CheckProblem.hpp"
#include "ExchangeHalo.hpp"
#include "OptimizeProblem.hpp"
#include "WriteProblem.hpp"
#include "ReportResults.hpp"
#include "mytimer.hpp"
#include "ComputeSPMV_ref.hpp"
#include "ComputeMG_ref.hpp"
#include "ComputeResidual.hpp"
#include "CG.hpp"
#include "CG_ref.hpp"
#include "Geometry.hpp"
#include "SparseMatrix.hpp"
#include "Vector.hpp"
#include "CGData.hpp"
#include "TestCG.hpp"
#include "TestSymmetry.hpp"
#include "TestNorms.hpp"

#if 0
int main(int argc, char * argv[]) {
  int size =1, rank=0;
  HPCG_Params params;
  Geometry * geom = new Geometry;
  GenerateGeometry(size, rank, params.numThreads, params.pz, params.zl, params.zu, nx, ny, nz, params.npx, params.npy, params.npz, geom);
  SparseMatrix A;
  InitializeSparseMatrix(A, geom);
  GenerateProblem(A, &b, &x, &xexact);
  return 0;
}
#endif

void SetupHalo(SparseMatrix & A) {
  printf("nx=%ld;\n",(long)A.geom->nx);
  printf("ny=%ld;\n",(long)A.geom->ny);
  printf("nz=%ld;\n",(long)A.geom->nz);
  printf("gnx=%ld;\n",(long)A.geom->gnx);
  printf("gny=%ld;\n",(long)A.geom->gny);
  printf("gnz=%ld;\n",(long)A.geom->gnz);

  printf("totalNumberOfRows=%ld;\n",(long)A.totalNumberOfRows);
  printf("totalNumberOfNonzeros=%ld;\n",(long)A.totalNumberOfNonzeros);
  printf("localNumberOfRows=%ld;\n",(long)A.localNumberOfRows);
  printf("localNumberOfColumns=%ld;\n",(long)A.localNumberOfColumns);
  printf("localNumberOfNonzeros=%ld;\n",(long)A.localNumberOfNonzeros);
  printf("nonzerosInRow=%ld;\n",(long)A.nonzerosInRow[0]);

  local_int_t numberOfNonzerosPerRow = 27; // We are approximating a 27-point finite element/volume/difference 3D stencil
  printf("a=zeros(%ld,%ld);\n", (long)A.localNumberOfRows, (long)A.localNumberOfRows);
  for (local_int_t j=0; j< A.localNumberOfRows; ++j) {
    printf("idx=[");
    char ch=' ';
    for (local_int_t i=0; i< numberOfNonzerosPerRow; ++i)
      if (A.mtxIndG[j][i]) {
        printf("%c%ld", ch, (long)(A.mtxIndG[j][i]+1) );
        ch = ';';
      }
    printf("];\nfor i = 1:size(idx,1)\na(%ld,idx(i))=-1;\nend\n", (long)(j+1));
  }
  printf("for i=1:%ld\na(i,i)=26;\nend\n", (long)A.localNumberOfRows);
  printf("spy(sparse(a));\n");

  exit(0);
}
