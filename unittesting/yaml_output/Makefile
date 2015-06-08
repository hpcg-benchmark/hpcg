
SRCD = ../../src

OBJS = $(SRCD)/ComputeOptimalShapeXYZ.o $(SRCD)/GenerateGeometry.o $(SRCD)/MixedBaseCounter.o $(SRCD)/ReportResults.o $(SRCD)/YAML_Doc.o $(SRCD)/YAML_Element.o

CXXFLAGS = -I../../src -pipe -g -DHPCG_NO_MPI
LDFLAGS = -g

main: main.o $(OBJS)
	$(CXX) $(LDFLAGS) -o main main.o $(OBJS) $(LDLIBS)
