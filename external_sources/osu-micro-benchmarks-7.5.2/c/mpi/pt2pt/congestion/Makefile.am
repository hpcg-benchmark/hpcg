AUTOMAKE_OPTIONS = subdir-objects

NVCC = @NVCC@
NVCFLAGS = -cuda -maxrregcount 32 -ccbin $(CXX) $(NVCCFLAGS)
SUFFIXES = .cu .cpp
.cu.cpp:
	$(NVCC) $(NVCFLAGS) $(INCLUDES) $(CPPFLAGS) --output-file $@.ii $<
	mv $@.ii $@

congestiondir = $(pkglibexecdir)/mpi/congestion
congestion_PROGRAMS = osu_bw_fan_out osu_bw_fan_in

AM_CFLAGS = -I${top_srcdir}/c/util -I${top_srcdir}/c/mpi/pt2pt/congestion/utils

UTILITIES = ../../../util/osu_util.c ../../../util/osu_util.h \
	../../../util/osu_util_mpi.c ../../../util/osu_util_mpi.h \
	../../../util/osu_util_graph.c ../../../util/osu_util_graph.h \
	../../../util/osu_util_papi.c ../../../util/osu_util_papi.h \
	./utils/osu_bw_fan_util.c ./utils/osu_bw_fan_util.h
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

osu_bw_fan_in_SOURCES = osu_bw_fan_in.c $(UTILITIES)
osu_bw_fan_out_SOURCES = osu_bw_fan_out.c $(UTILITIES)

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

