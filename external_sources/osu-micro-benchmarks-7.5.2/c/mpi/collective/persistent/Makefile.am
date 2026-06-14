AUTOMAKE_OPTIONS = subdir-objects

NVCC = @NVCC@
NVCFLAGS = -cuda -maxrregcount 32 -ccbin $(CXX) $(NVCCFLAGS)
SUFFIXES = .cu .cpp
.cu.cpp:
	$(NVCC) $(NVCFLAGS) $(INCLUDES) $(CPPFLAGS) --output-file $@.ii $<
	mv $@.ii $@

pers_colldir = $(pkglibexecdir)/mpi/collective
pers_coll_PROGRAMS =  osu_allgather_persistent osu_allgatherv_persistent \
						osu_allreduce_persistent osu_alltoall_persistent \
						osu_alltoallv_persistent osu_alltoallw_persistent \
						osu_barrier_persistent osu_bcast_persistent \
						osu_gather_persistent osu_gatherv_persistent \
						osu_reduce_persistent osu_reduce_scatter_persistent \
						osu_scatter_persistent osu_scatterv_persistent

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

osu_allgather_persistent_SOURCES = osu_allgather_persistent.c $(UTILITIES)
osu_allgatherv_persistent_SOURCES = osu_allgatherv_persistent.c $(UTILITIES)
osu_allreduce_persistent_SOURCES = osu_allreduce_persistent.c $(UTILITIES)
osu_alltoall_persistent_SOURCES = osu_alltoall_persistent.c $(UTILITIES)
osu_alltoallv_persistent_SOURCES = osu_alltoallv_persistent.c $(UTILITIES)
osu_alltoallw_persistent_SOURCES = osu_alltoallw_persistent.c $(UTILITIES)
osu_barrier_persistent_SOURCES = osu_barrier_persistent.c $(UTILITIES)
osu_bcast_persistent_SOURCES =  osu_bcast_persistent.c $(UTILITIES)
osu_gather_persistent_SOURCES = osu_gather_persistent.c $(UTILITIES)
osu_gatherv_persistent_SOURCES = osu_gatherv_persistent.c $(UTILITIES)
osu_reduce_persistent_SOURCES = osu_reduce_persistent.c $(UTILITIES)
osu_reduce_scatter_persistent_SOURCES = osu_reduce_scatter_persistent.c $(UTILITIES)
osu_scatter_persistent_SOURCES = osu_scatter_persistent.c $(UTILITIES)
osu_scatterv_persistent_SOURCES = osu_scatterv_persistent.c $(UTILITIES)

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
