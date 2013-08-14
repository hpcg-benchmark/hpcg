#HEADER
# ************************************************************************
# 
#               HPCG: High Performance Conjugate Gradient Code
# Questions? Contact Michael A. Heroux (maherou@sandia.gov) 
# 
# ************************************************************************
#@HEADER


# Simple hand-tuned makefile.  Modify as necessary for your environment.
# Questions? Contact Mike Heroux (maherou@sandia.gov).
#

#
# Specify compiler and linker:

#CXX=/usr/local/bin/g++
#LINKER=/usr/local/bin/g++
CXX=g++
LINKER=g++
#CXX=/usr/local/openmpi-gcc-4.8/bin/mpicxx
#LINKER=/usr/local/openmpi-gcc-4.8/bin/mpicxx


#    If you want to run the program without MPI, make sure NOMPI is set 
#    to -DHPCG_NOMPI
#NOMPI = 
NOMPI = -DHPCG_NOMPI

#    If you want to run the program without OpenMP, make sure NOOPENMP is set 
#    to -DHPCG_NOOPENMP
#OOPENMP = 
NOOPENMP = -DHPCG_NOOPENMP



#CPP_OPT_FLAGS = -O3 -funroll-all-loops -malign-double
#CPP_OPT_FLAGS = -O3 -m64
#CPP_OPT_FLAGS = -O3 -ftree-vectorize -ftree-vectorizer-verbose=2 -fopenmp
CPP_OPT_FLAGS = -O0 -g
#CPP_OPT_FLAGS = -O0 -g -DDEBUG
#CPP_OPT_FLAGS = -O0 -g -DDEBUG -DNO_PRECONDITIONER
#CPP_OPT_FLAGS = -O0 -g -DNO_PRECONDITIONER
#CPP_OPT_FLAGS = -O3
#CPP_OPT_FLAGS = -O3 -ffast-math -ftree-vectorize -ftree-vectorizer-verbose=0 -fopenmp


#
# Specify name if executable (optional):

TARGET = test_HPCG

################### Derived Quantities (no modification required) ##############

CXXFLAGS= $(CPP_OPT_FLAGS) $(NOMPI) $(NOOPENMP)

LIB_PATHS=

TEST_CPP = main.cpp GenerateGeometry.cpp GenerateProblem.cpp \
	   WriteProblem.cpp ComputeResidual.cpp ReportResults.cpp mytimer.cpp \
           spmvref.cpp symgsref.cpp spmv.cpp CG.cpp waxpby.cpp dot.cpp symgs.cpp \
           OptimizeMatrix.cpp ExchangeHalo.cpp \
           YAML_Element.cpp YAML_Doc.cpp

TEST_OBJ          = $(TEST_CPP:.cpp=.o)
all: $(TARGET)

$(TARGET): $(TEST_OBJ)
	$(LINKER) $(CPP_OPT_FLAGS) $(TEST_OBJ) $(LIB_PATHS) -o $(TARGET)

clean:
	@rm -f *.o  *~ $(TARGET) $(TARGET).exe
