AUTOMAKE_OPTIONS = subdir-objects

NVCC = @NVCC@
NVCFLAGS = -cuda -maxrregcount 32 -ccbin $(CXX) $(NVCCFLAGS)
SUFFIXES = .cu .cpp
.cu.cpp:
	$(NVCC) $(NVCFLAGS) $(INCLUDES) $(CPPFLAGS) --output-file $@.ii $<
	mv $@.ii $@

block_colldir = $(pkglibexecdir)/mpi/collective
block_coll_PROGRAMS = osu_alltoallv osu_alltoallw osu_allgatherv osu_scatterv \
					  osu_gatherv osu_reduce_scatter osu_barrier osu_reduce \
					  osu_allreduce osu_alltoall osu_bcast osu_gather \
					  osu_allgather osu_scatter osu_reduce_scatter_block
AM_CFLAGS = -I${top_srcdir}/c/util

UTILITIES = ../../../util/osu_util.c ../../../util/osu_util.h \
../../../util/osu_util_mpi.c ../../../util/osu_util_mpi.h \
../../../util/osu_util_graph.c ../../../util/osu_util_graph.h \
../../../util/osu_util_papi.c ../../../util/osu_util_papi.h \
../../../util/osu_util_validation.c
if SYCL
UTILITIES += ../../../util/osu_util_sycl.cpp ../../../util/osu_util_sycl.hpp
endif
if CUDA_KERNELS
UTILITIES += ../../../util/kernel.cu
if BUILD_USE_PGI
AM_CXXFLAGS = --nvcchost --no_preincludes
endif
endif

osu_allgatherv_SOURCES = osu_allgatherv.c $(UTILITIES)
osu_scatterv_SOURCES = osu_scatterv.c $(UTILITIES)
osu_gather_SOURCES = osu_gather.c $(UTILITIES)
osu_gatherv_SOURCES = osu_gatherv.c $(UTILITIES)
osu_reduce_scatter_SOURCES = osu_reduce_scatter.c $(UTILITIES)
osu_reduce_scatter_block_SOURCES = osu_reduce_scatter_block.c $(UTILITIES)
osu_barrier_SOURCES = osu_barrier.c $(UTILITIES)
osu_reduce_SOURCES = osu_reduce.c $(UTILITIES)
osu_allreduce_SOURCES = osu_allreduce.c $(UTILITIES)
osu_bcast_SOURCES = osu_bcast.c $(UTILITIES)
osu_alltoall_SOURCES = osu_alltoall.c $(UTILITIES)
osu_alltoallv_SOURCES = osu_alltoallv.c $(UTILITIES)
osu_alltoallw_SOURCES = osu_alltoallw.c $(UTILITIES)
osu_allgather_SOURCES = osu_allgather.c $(UTILITIES)
osu_scatter_SOURCES = osu_scatter.c $(UTILITIES)

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
