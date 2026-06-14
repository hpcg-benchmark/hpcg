AUTOMAKE_OPTIONS = subdir-objects

NVCC = @NVCC@
NVCFLAGS = -cuda -maxrregcount 32 -ccbin $(CXX) $(NVCCFLAGS)
SUFFIXES = .cu .cpp
.cu.cpp:
	$(NVCC) $(NVCFLAGS) $(INCLUDES) $(CPPFLAGS) --output-file $@.ii $<
	mv $@.ii $@

nonblk_colldir = $(pkglibexecdir)/mpi/collective
nonblk_coll_PROGRAMS = osu_iallgather osu_ibcast osu_ialltoall osu_ibarrier    \
					   osu_igather osu_iscatter osu_iscatterv osu_igatherv     \
					   osu_iallgatherv osu_ialltoallv osu_ialltoallw           \
					   osu_ireduce osu_iallreduce osu_ireduce_scatter          \
					   osu_ireduce_scatter_block

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

osu_ialltoall_SOURCES = osu_ialltoall.c $(UTILITIES)
osu_ialltoallv_SOURCES = osu_ialltoallv.c $(UTILITIES)
osu_ialltoallw_SOURCES = osu_ialltoallw.c $(UTILITIES)
osu_ibarrier_SOURCES = osu_ibarrier.c $(UTILITIES)
osu_ibcast_SOURCES = osu_ibcast.c $(UTILITIES)
osu_igather_SOURCES = osu_igather.c $(UTILITIES)
osu_igatherv_SOURCES = osu_igatherv.c $(UTILITIES)
osu_iallgather_SOURCES = osu_iallgather.c $(UTILITIES)
osu_iallgatherv_SOURCES = osu_iallgatherv.c $(UTILITIES)
osu_iscatter_SOURCES = osu_iscatter.c $(UTILITIES)
osu_iscatterv_SOURCES = osu_iscatterv.c $(UTILITIES)
osu_ireduce_SOURCES = osu_ireduce.c $(UTILITIES)
osu_iallreduce_SOURCES = osu_iallreduce.c $(UTILITIES)
osu_ireduce_scatter_SOURCES = osu_ireduce_scatter.c $(UTILITIES)
osu_ireduce_scatter_block_SOURCES = osu_ireduce_scatter_block.c $(UTILITIES)

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
