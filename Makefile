# -*- Makefile -*-

# by default, "arch" is unknown
arch = UNKNOWN

include setup/Make.$(arch)

HPCG_DEPS = src/CG.o src/CGref.o src/CGtest.o src/ComputeResidual.o \
         src/ExchangeHalo.o src/GenerateGeometry.o src/GenerateProblem.o \
         src/OptimizeProblem.o src/ReportResults.o src/SetupHalo.o src/SymTest.o src/NormTest.o \
         src/WriteProblem.o src/YAML_Doc.o src/YAML_Element.o src/dot.o \
         src/dotref.o src/finalize.o src/init.o src/mytimer.o src/spmv.o \
         src/spmvref.o src/symgs.o src/symgsref.o src/waxpby.o src/waxpbyref.o

bin/xhpcg: testing/main.o $(HPCG_DEPS)
	$(LINKER) $(LINKFLAGS) $? -o $@ $(HPCG_LIBS)

clean:
	rm -f $(HPCG_DEPS) bin/xhpcg testing/main.o

.PHONY: clean

