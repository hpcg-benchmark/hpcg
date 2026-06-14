AUTOMAKE_OPTIONS = subdir-objects

NVCC = @NVCC@
NVCFLAGS = -cuda -maxrregcount 32 -ccbin $(CXX) $(NVCCFLAGS)
SUFFIXES = .cu .cpp
.cu.cpp:
	$(NVCC) $(NVCFLAGS) $(INCLUDES) $(CPPFLAGS) --output-file $@.ii $<
	mv $@.ii $@

standard_pt2ptdir = $(pkglibexecdir)/mpi/pt2pt
standard_pt2pt_PROGRAMS = osu_bibw osu_bw osu_latency osu_mbw_mr osu_multi_lat

if MPI4_PARTITION_PT2PT
standard_pt2pt_PROGRAMS += osu_partitioned_latency
endif
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
CLEANFILES = ../../../util/kernel.cpp
if BUILD_USE_PGI
AM_CXXFLAGS = --nvcchost --no_preincludes
endif
endif

osu_bw_SOURCES = osu_bw.c $(UTILITIES)
osu_bibw_SOURCES = osu_bibw.c $(UTILITIES)
osu_latency_SOURCES = osu_latency.c $(UTILITIES)
osu_mbw_mr_SOURCES = osu_mbw_mr.c $(UTILITIES)
osu_multi_lat_SOURCES = osu_multi_lat.c $(UTILITIES)
osu_latency_mt_SOURCES = osu_latency_mt.c $(UTILITIES)
osu_latency_mp_SOURCES = osu_latency_mp.c $(UTILITIES)
if MPI4_PARTITION_PT2PT
osu_partitioned_latency_SOURCES = osu_partitioned_latency.c $(UTILITIES)
endif

if MPI2_LIBRARY
    standard_pt2pt_PROGRAMS += osu_latency_mt osu_latency_mp
endif

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

