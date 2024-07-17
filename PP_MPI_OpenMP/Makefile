# -*- Makefile -*-

#arch = Linux_Serial
arch = PP_MPI_OMP
setup_file = setup/Make.$(arch)

include $(setup_file)


HPCG_DEPS = src/CG.o \
	    src/CG_ref.o \
	    src/TestCG.o \
	    src/ComputeResidual.o \
	    src/ExchangeHalo.o \
	    src/GenerateGeometry.o \
	    src/GenerateProblem.o \
	    src/GenerateProblem_ref.o \
	    src/CheckProblem.o \
	    src/MixedBaseCounter.o \
	    src/OptimizeProblem.o \
	    src/ReadHpcgDat.o \
	    src/ReportResults.o \
	    src/SetupHalo.o \
	    src/SetupHalo_ref.o \
	    src/TestSymmetry.o \
	    src/TestNorms.o \
	    src/WriteProblem.o \
	    src/YAML_Doc.o \
	    src/YAML_Element.o \
	    src/ComputeDotProduct.o \
	    src/ComputeDotProduct_ref.o \
	    src/mytimer.o \
	    src/ComputeOptimalShapeXYZ.o \
	    src/ComputeSPMV.o \
	    src/ComputeSPMV_ref.o \
	    src/ComputeSYMGS.o \
	    src/ComputeSYMGS_ref.o \
	    src/ComputeWAXPBY.o \
	    src/ComputeWAXPBY_ref.o \
	    src/ComputeMG_ref.o \
	    src/ComputeMG.o \
	    src/ComputeProlongation_ref.o \
	    src/ComputeRestriction_ref.o \
	    src/CheckAspectRatio.o \
	    src/OutputFile.o \
	    src/GenerateCoarseProblem.o \
	    src/init.o \
	    src/finalize.o

# These header files are included in many source files, so we recompile every file if one or more of these header is modified.
PRIMARY_HEADERS = ../src/Geometry.hpp ../src/SparseMatrix.hpp ../src/Vector.hpp ../src/CGData.hpp \
                  ../src/MGData.hpp ../src/hpcg.hpp

all: bin/xhpcg
	#sbatch ./bin/job.sh
	export OMP_NUM_THREADS=2 && mpiexec -n 4 ./bin/xhpcg
	mv ./*.txt ./result/HPCG

bin/xhpcg: src/main.o $(HPCG_DEPS)
	$(LINKER) $(LINKFLAGS) src/main.o $(HPCG_DEPS) $(HPCG_LIBS) -o bin/xhpcg

clean:
	rm -f src/*.o bin/xhpcg

.PHONY: all clean

src/main.o: ../src/main.cpp $(PRIMARY_HEADERS)
	$(CXX) -c $(CXXFLAGS) -I../src $< -o $@

src/CG.o: ../src/CG.cpp ../src/CG.hpp $(PRIMARY_HEADERS)
	$(CXX) -c $(CXXFLAGS) -I../src $< -o $@

src/CG_ref.o: ../src/CG_ref.cpp ../src/CG_ref.hpp $(PRIMARY_HEADERS)
	$(CXX) -c $(CXXFLAGS) -I../src $< -o $@

src/TestCG.o: ../src/TestCG.cpp ../src/TestCG.hpp $(PRIMARY_HEADERS)
	$(CXX) -c $(CXXFLAGS) -I../src $< -o $@

src/ComputeResidual.o: ../src/ComputeResidual.cpp ../src/ComputeResidual.hpp $(PRIMARY_HEADERS)
	$(CXX) -c $(CXXFLAGS) -I../src $< -o $@

src/ExchangeHalo.o: ../src/ExchangeHalo.cpp ../src/ExchangeHalo.hpp $(PRIMARY_HEADERS)
	$(CXX) -c $(CXXFLAGS) -I../src $< -o $@

src/GenerateGeometry.o: ../src/GenerateGeometry.cpp ../src/GenerateGeometry.hpp $(PRIMARY_HEADERS)
	$(CXX) -c $(CXXFLAGS) -I../src $< -o $@

src/GenerateProblem.o: ../src/GenerateProblem.cpp ../src/GenerateProblem.hpp $(PRIMARY_HEADERS)
	$(CXX) -c $(CXXFLAGS) -I../src $< -o $@

src/GenerateProblem_ref.o: ../src/GenerateProblem_ref.cpp ../src/GenerateProblem_ref.hpp $(PRIMARY_HEADERS)
	$(CXX) -c $(CXXFLAGS) -I../src $< -o $@

src/CheckProblem.o: ../src/CheckProblem.cpp ../src/CheckProblem.hpp $(PRIMARY_HEADERS)
	$(CXX) -c $(CXXFLAGS) -I../src $< -o $@

src/MixedBaseCounter.o: ../src/MixedBaseCounter.cpp ../src/MixedBaseCounter.hpp $(PRIMARY_HEADERS)
	$(CXX) -c $(CXXFLAGS) -I../src $< -o $@

src/OptimizeProblem.o: ../src/OptimizeProblem.cpp ../src/OptimizeProblem.hpp $(PRIMARY_HEADERS)
	$(CXX) -c $(CXXFLAGS) -I../src $< -o $@

src/ReadHpcgDat.o: ../src/ReadHpcgDat.cpp ../src/ReadHpcgDat.hpp $(PRIMARY_HEADERS)
	$(CXX) -c $(CXXFLAGS) -I../src $< -o $@

src/ReportResults.o: ../src/ReportResults.cpp ../src/ReportResults.hpp $(PRIMARY_HEADERS)
	$(CXX) -c $(CXXFLAGS) -I../src $< -o $@

src/SetupHalo.o: ../src/SetupHalo.cpp ../src/SetupHalo.hpp $(PRIMARY_HEADERS)
	$(CXX) -c $(CXXFLAGS) -I../src $< -o $@

src/SetupHalo_ref.o: ../src/SetupHalo_ref.cpp ../src/SetupHalo_ref.hpp $(PRIMARY_HEADERS)
	$(CXX) -c $(CXXFLAGS) -I../src $< -o $@

src/TestSymmetry.o: ../src/TestSymmetry.cpp ../src/TestSymmetry.hpp $(PRIMARY_HEADERS)
	$(CXX) -c $(CXXFLAGS) -I../src $< -o $@

src/TestNorms.o: ../src/TestNorms.cpp ../src/TestNorms.hpp $(PRIMARY_HEADERS)
	$(CXX) -c $(CXXFLAGS) -I../src $< -o $@

src/WriteProblem.o: ../src/WriteProblem.cpp ../src/WriteProblem.hpp $(PRIMARY_HEADERS)
	$(CXX) -c $(CXXFLAGS) -I../src $< -o $@

src/YAML_Doc.o: ../src/YAML_Doc.cpp ../src/YAML_Doc.hpp $(PRIMARY_HEADERS)
	$(CXX) -c $(CXXFLAGS) -I../src $< -o $@

src/YAML_Element.o: ../src/YAML_Element.cpp ../src/YAML_Element.hpp $(PRIMARY_HEADERS)
	$(CXX) -c $(CXXFLAGS) -I../src $< -o $@

src/ComputeDotProduct.o: ../src/ComputeDotProduct.cpp ../src/ComputeDotProduct.hpp $(PRIMARY_HEADERS)
	$(CXX) -c $(CXXFLAGS) -I../src $< -o $@

src/ComputeDotProduct_ref.o: ../src/ComputeDotProduct_ref.cpp ../src/ComputeDotProduct_ref.hpp $(PRIMARY_HEADERS)
	$(CXX) -c $(CXXFLAGS) -I../src $< -o $@

src/finalize.o: ../src/finalize.cpp $(PRIMARY_HEADERS)
	$(CXX) -c $(CXXFLAGS) -I../src $< -o $@

src/init.o: ../src/init.cpp $(PRIMARY_HEADERS)
	$(CXX) -c $(CXXFLAGS) -I../src $< -o $@

src/mytimer.o: ../src/mytimer.cpp ../src/mytimer.hpp $(PRIMARY_HEADERS)
	$(CXX) -c $(CXXFLAGS) -I../src $< -o $@

src/ComputeOptimalShapeXYZ.o: ../src/ComputeOptimalShapeXYZ.cpp ../src/ComputeOptimalShapeXYZ.hpp $(PRIMARY_HEADERS)
	$(CXX) -c $(CXXFLAGS) -I../src $< -o $@

src/ComputeSPMV.o: ../src/ComputeSPMV.cpp ../src/ComputeSPMV.hpp $(PRIMARY_HEADERS)
	$(CXX) -c -march=native $(CXXFLAGS) -I../src $< -o $@

src/ComputeSPMV_ref.o: ../src/ComputeSPMV_ref.cpp ../src/ComputeSPMV_ref.hpp $(PRIMARY_HEADERS)
	$(CXX) -c $(CXXFLAGS) -I../src $< -o $@

src/ComputeSYMGS.o: ../src/ComputeSYMGS.cpp ../src/ComputeSYMGS.hpp $(PRIMARY_HEADERS)
	$(CXX) -c $(CXXFLAGS) -I../src $< -o $@

src/ComputeSYMGS_ref.o: ../src/ComputeSYMGS_ref.cpp ../src/ComputeSYMGS_ref.hpp $(PRIMARY_HEADERS)
	$(CXX) -c $(CXXFLAGS) -I../src $< -o $@

src/ComputeWAXPBY.o: ../src/ComputeWAXPBY.cpp ../src/ComputeWAXPBY.hpp $(PRIMARY_HEADERS)
	$(CXX) -c $(CXXFLAGS) -I../src $< -o $@

src/ComputeWAXPBY_ref.o: ../src/ComputeWAXPBY_ref.cpp ../src/ComputeWAXPBY_ref.hpp $(PRIMARY_HEADERS)
	$(CXX) -c $(CXXFLAGS) -I../src $< -o $@

src/ComputeMG_ref.o: ../src/ComputeMG_ref.cpp ../src/ComputeMG_ref.hpp $(PRIMARY_HEADERS)
	$(CXX) -c $(CXXFLAGS) -I../src $< -o $@

src/ComputeMG.o: ../src/ComputeMG.cpp ../src/ComputeMG.hpp $(PRIMARY_HEADERS)
	$(CXX) -c $(CXXFLAGS) -I../src $< -o $@

src/ComputeProlongation_ref.o: ../src/ComputeProlongation_ref.cpp ../src/ComputeProlongation_ref.hpp $(PRIMARY_HEADERS)
	$(CXX) -c $(CXXFLAGS) -I../src $< -o $@

src/ComputeRestriction_ref.o: ../src/ComputeRestriction_ref.cpp ../src/ComputeRestriction_ref.hpp $(PRIMARY_HEADERS)
	$(CXX) -c $(CXXFLAGS) -I../src $< -o $@

src/GenerateCoarseProblem.o: ../src/GenerateCoarseProblem.cpp ../src/GenerateCoarseProblem.hpp $(PRIMARY_HEADERS)
	$(CXX) -c $(CXXFLAGS) -I../src $< -o $@

src/CheckAspectRatio.o: ../src/CheckAspectRatio.cpp ../src/CheckAspectRatio.hpp $(PRIMARY_HEADERS)
	$(CXX) -c $(CXXFLAGS) -I../src $< -o $@

src/OutputFile.o: ../src/OutputFile.cpp ../src/OutputFile.hpp $(PRIMARY_HEADERS)
	$(CXX) -c $(CXXFLAGS) -I../src $< -o $@

