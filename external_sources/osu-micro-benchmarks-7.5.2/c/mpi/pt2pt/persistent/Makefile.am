AUTOMAKE_OPTIONS = subdir-objects

NVCC = @NVCC@
NVCFLAGS = -cuda -maxrregcount 32 -ccbin $(CXX) $(NVCCFLAGS)
SUFFIXES = .cu .cpp
.cu.cpp:
	$(NVCC) $(NVCFLAGS) $(INCLUDES) $(CPPFLAGS) --output-file $@.ii $<
	mv $@.ii $@

persistent_pt2ptdir = $(pkglibexecdir)/mpi/pt2pt/persistent
persistent_pt2pt_PROGRAMS = osu_bibw_persistent osu_bw_persistent \
							osu_latency_persistent

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

osu_bw_persistent_SOURCES = osu_bw_persistent.c $(UTILITIES)
osu_bibw_persistent_SOURCES = osu_bibw_persistent.c $(UTILITIES)
osu_latency_persistent_SOURCES = osu_latency_persistent.c $(UTILITIES)

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

