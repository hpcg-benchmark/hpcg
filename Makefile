# -*- Makefile -*-

# by default, "arch" is unknown, should be specified in the command line
arch = UNKNOWN

setup_file = setup/Make.$(arch)
include $(setup_file)

HPCG_DEPS = src/CG.o src/CG_ref.o src/TestCG.o src/ComputeResidual.o \
         src/ExchangeHalo.o src/GenerateGeometry.o src/GenerateProblem.o \
         src/GenerateProblem_ref.o src/CheckProblem.o \
	 src/OptimizeProblem.o src/ReadHpcgDat.o src/ReportResults.o \
	 src/SetupHalo.o src/SetupHalo_ref.o src/TestSymmetry.o src/TestNorms.o src/WriteProblem.o \
         src/YAML_Doc.o src/YAML_Element.o src/ComputeDotProduct.o \
         src/ComputeDotProduct_ref.o src/finalize.o src/init.o src/mytimer.o src/ComputeSPMV.o \
         src/ComputeSPMV_ref.o src/ComputeSYMGS.o src/ComputeSYMGS_ref.o src/ComputeWAXPBY.o src/ComputeWAXPBY_ref.o \
         src/ComputeMG_ref.o src/ComputeMG.o src/ComputeProlongation_ref.o src/ComputeRestriction_ref.o src/GenerateCoarseProblem.o \
	 src/ComputeOptimalShapeXYZ.o src/MixedBaseCounter.o src/CheckAspectRatio.o src/OutputFile.o

bin/xhpcg: src/main.o $(HPCG_DEPS)
	$(LINKER) $(LINKFLAGS) src/main.o $(HPCG_DEPS) -o bin/xhpcg $(HPCG_LIBS)

clean:
	rm -f $(HPCG_DEPS) bin/xhpcg src/main.o

.PHONY: clean

