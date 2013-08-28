#HEADER
#  -- High Performance Conjugate Gradient Benchmark (HPCG)
#     HPCG - 0.1 - August 22, 2013

#     Michael A. Heroux
#     Scalable Algorithms Group, Computing Research Division
#     Sandia National Laboratories, Albuquerque, NM
#
#     Piotr Luszczek
#     Jack Dongarra
#     University of Tennessee, Knoxville
#     Innovative Computing Laboratory
#
#     (C) Copyright 2013 All Rights Reserved
#
#
#  -- Copyright notice and Licensing terms:
#
#  Redistribution  and  use in  source and binary forms, with or without
#  modification, are  permitted provided  that the following  conditions
#  are met:
#
#  1. Redistributions  of  source  code  must retain the above copyright
#  notice, this list of conditions and the following disclaimer.
#
#  2. Redistributions in binary form must reproduce  the above copyright
#  notice, this list of conditions,  and the following disclaimer in the
#  documentation and/or other materials provided with the distribution.
#
#  3. All  advertising  materials  mentioning  features  or  use of this
#  software must display the following acknowledgement:
#  This  product  includes  software  developed  at Sandia National
#  Laboratories, Albuquerque, NM and the  University  of
#  Tennessee, Knoxville, Innovative Computing Laboratory.
#
#  4. The name of the  University,  the name of the  Laboratory,  or the
#  names  of  its  contributors  may  not  be used to endorse or promote
#  products  derived   from   this  software  without  specific  written
#  permission.
#
#  -- Disclaimer:
#
#  THIS  SOFTWARE  IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
#  ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES,  INCLUDING,  BUT NOT
#  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
#  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE UNIVERSITY
#  OR  CONTRIBUTORS  BE  LIABLE FOR ANY  DIRECT,  INDIRECT,  INCIDENTAL,
#  SPECIAL,  EXEMPLARY,  OR  CONSEQUENTIAL DAMAGES  (INCLUDING,  BUT NOT
#  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
#  DATA OR PROFITS; OR BUSINESS INTERRUPTION)  HOWEVER CAUSED AND ON ANY
#  THEORY OF LIABILITY, WHETHER IN CONTRACT,  STRICT LIABILITY,  OR TORT
#  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
#  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
# ######################################################################
#@HEADER

# Include definitions that are architecture-specific

include Make.inc

################### Derived Quantities (no modification required) ##############

TARGET = $(BINdir)/HPCG-Benchmark.exe

TEST_CPP = main.cpp init.cpp finalize.cpp GenerateGeometry.cpp GenerateProblem.cpp SetupHalo.cpp \
	   WriteProblem.cpp ComputeResidual.cpp ReportResults.cpp mytimer.cpp \
           spmvref.cpp symgsref.cpp spmv.cpp CG.cpp waxpby.cpp waxpbyref.cpp dot.cpp symgs.cpp \
           OptimizeProblem.cpp ExchangeHalo.cpp \
           YAML_Element.cpp YAML_Doc.cpp

TEST_OBJ          = $(TEST_CPP:.cpp=.o)

all: $(TARGET)

# This rule is for the main driver
main.o: ../main.cpp
	$(CXX) -c $(CXXFLAGS) ../main.cpp

# This rule is for all the source files that are part of the Benchmark
%.o : $(SRCdir)/%.cpp
	$(CXX) -c $(CXXFLAGS) $<

$(TARGET): $(TEST_OBJ)
	$(LINKER) $(LINKFLAGS) $(TEST_OBJ) $(HPCG_LIBS) -o $(TARGET)

clean:
	$(RM) *.o  $(TARGET)
