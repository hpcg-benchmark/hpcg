AUTOMAKE_OPTIONS = subdir-objects

NVCC = @NVCC@
NVCFLAGS = -cuda -maxrregcount 32 -ccbin $(CXX) $(NVCCFLAGS)
SUFFIXES = .cu .cpp
.cu.cpp:
	$(NVCC) $(NVCFLAGS) $(INCLUDES) $(CPPFLAGS) --output-file $@.ii $<
	mv $@.ii $@

nhbr_colldir = $(pkglibexecdir)/mpi/collective
nhbr_coll_PROGRAMS =  osu_neighbor_allgather osu_neighbor_allgatherv \
					  osu_neighbor_alltoall osu_neighbor_alltoallv \
					  osu_neighbor_alltoallw osu_ineighbor_allgather \
					  osu_ineighbor_allgatherv osu_ineighbor_alltoall \
					  osu_ineighbor_alltoallv osu_ineighbor_alltoallw

AM_CFLAGS = -I${top_srcdir}/c/util

UTILITIES = ../../../util/osu_util.c ../../../util/osu_util.h \
../../../util/osu_util_mpi.c ../../../util/osu_util_mpi.h \
../../../util/osu_util_graph.c ../../../util/osu_util_graph.h \
../../../util/osu_util_papi.c ../../../util/osu_util_papi.h
if SYCL
UTILITIES += ../../../util/osu_util_sycl.cpp ../../../util/osu_util_sycl.hpp
endif
if CUDA_KERNELS
UTILITIES += ../../../util/kernel.cu
if BUILD_USE_PGI
AM_CXXFLAGS = --nvcchost --no_preincludes
endif
endif

osu_neighbor_allgather_SOURCES = osu_neighbor_allgather.c $(UTILITIES)
osu_neighbor_allgatherv_SOURCES = osu_neighbor_allgatherv.c $(UTILITIES)
osu_neighbor_alltoall_SOURCES = osu_neighbor_alltoall.c $(UTILITIES)
osu_neighbor_alltoallv_SOURCES = osu_neighbor_alltoallv.c $(UTILITIES)
osu_neighbor_alltoallw_SOURCES = osu_neighbor_alltoallw.c $(UTILITIES)
osu_ineighbor_allgather_SOURCES = osu_ineighbor_allgather.c $(UTILITIES)
osu_ineighbor_allgatherv_SOURCES = osu_ineighbor_allgatherv.c $(UTILITIES)
osu_ineighbor_alltoall_SOURCES = osu_ineighbor_alltoall.c $(UTILITIES)
osu_ineighbor_alltoallv_SOURCES = osu_ineighbor_alltoallv.c $(UTILITIES)
osu_ineighbor_alltoallw_SOURCES = osu_ineighbor_alltoallw.c $(UTILITIES)

if EMBEDDED_BUILD
    AM_LDFLAGS =
    AM_CPPFLAGS = -I$(top_builddir)/../src/include \
          -I${top_srcdir}/util \
		  -I${top_srcdir}/../src/include
if BUILD_PROFILING_LIB
    AM_LDFLAGS += $(top_builddir)/../lib/lib@PMPILIBNAME@.la
endif
    AM_LDFLAGS += $(top_builddir)/../lib/lib@MPILIBNAME@.la
endif

if OPENACC
    AM_CFLAGS += -acc -ta=tesla:nordc
    AM_CXXFLAGS = -acc -ta=tesla:nordc
endif
