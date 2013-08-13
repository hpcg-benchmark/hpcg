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
# 0) Specify compiler and linker:

CXX=/usr/local/bin/g++
LINKER=/usr/local/bin/g++
#CXX=mpicxx
#LINKER=mpicxx


# 1) Build with MPI or not?
#    If you want to run the program with MPI, make sure USE_MPI is set 
#    to -DUSING_MPI

USE_MPI =
#USE_MPI = -DUSING_MPI

# 2) MPI headers:  
#    If you:
#    - Are building MPI mode (-DUSING_MPI is set above).
#    - Do not have the MPI headers installed in a default search directory and
#    - Are not using MPI compiler wrappers
#    Then specify the path to your MPI header file (include a -I)

#MPI_INC = -I/usr/MPICH/SDK.gcc/include


# 3) Specify C++ compiler optimization flags (if any)
#    Typically some reasonably high level of optimization should be used to 
#    enhance performance.

#IA32 with GCC: 
#CPP_OPT_FLAGS = -O3 -funroll-all-loops -malign-double

#CPP_OPT_FLAGS = -O3 -m64
#CPP_OPT_FLAGS = -O3 -ftree-vectorize -ftree-vectorizer-verbose=2 -fopenmp
CPP_OPT_FLAGS = -O0 -g
#CPP_OPT_FLAGS = -O0 -g -DDEBUG
#CPP_OPT_FLAGS = -O0 -g -DDEBUG -DNO_PRECONDITIONER
#CPP_OPT_FLAGS = -O0 -g -DNO_PRECONDITIONER
#CPP_OPT_FLAGS = -O3
#CPP_OPT_FLAGS = -O3 -ffast-math -ftree-vectorize -ftree-vectorizer-verbose=2

#
# 4) MPI library:
#    If you:
#    - Are building MPI mode (-DUSING_MPI is set above).
#    - Do not have the MPI library installed a default search directory and
#    - Are not using MPI compiler wrappers for linking
#    Then specify the path to your MPI library (include -L and -l directives)

#MPI_LIB = -L/usr/MPICH/SDK.gcc/lib -lmpich

#
# 5) System libraries: (May need to add -lg2c before -lm)

SYS_LIB =-lm

#
# 6) Specify name if executable (optional):

TARGET = test_HPCG

################### Derived Quantities (no modification required) ##############

CXXFLAGS= $(CPP_OPT_FLAGS) $(USE_MPI) $(MPI_INC) $(SHAREDMEM_MPI)

LIB_PATHS= $(SYS_LIB) $(MPI_LIB)

TEST_CPP = main.cpp GenerateGeometry.cpp GenerateProblem.cpp \
	  	WriteProblem.cpp ComputeResidual.cpp ReportResults.cpp mytimer.cpp \
          spmv.cpp CG.cpp waxpby.cpp dot.cpp symgs.cpp \
          OptimizeMatrix.cpp ExchangeHalo.cpp \
          YAML_Element.cpp YAML_Doc.cpp

TEST_OBJ          = $(TEST_CPP:.cpp=.o)
all: $(TARGET)

$(TARGET): $(TEST_OBJ)
	$(LINKER) $(CPP_OPT_FLAGS) $(TEST_OBJ) $(LIB_PATHS) -o $(TARGET)

clean:
	@rm -f *.o  *~ $(TARGET) $(TARGET).exe
