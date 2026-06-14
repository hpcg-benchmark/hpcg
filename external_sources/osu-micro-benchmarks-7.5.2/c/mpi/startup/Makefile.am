AUTOMAKE_OPTIONS = subdir-objects

NVCC = @NVCC@
NVCFLAGS = -cuda -maxrregcount 32 -ccbin $(CXX) $(NVCCFLAGS)
SUFFIXES = .cu .cpp
.cu.cpp:
	$(NVCC) $(NVCFLAGS) $(INCLUDES) $(CPPFLAGS) --output-file $@.ii $<
	mv $@.ii $@

startupdir = $(pkglibexecdir)/mpi/startup
startup_PROGRAMS = osu_init osu_hello

AM_CFLAGS = -I${top_srcdir}/c/util

UTILITIES = ../../util/osu_util.c ../../util/osu_util.h \
../../util/osu_util_mpi.c ../../util/osu_util_mpi.h \
../../util/osu_util_papi.c ../../util/osu_util_papi.h
if SYCL
UTILITIES += ../../util/osu_util_sycl.cpp ../../util/osu_util_sycl.hpp
endif

if CUDA_KERNELS
UTILITIES += ../../util/kernel.cu
if BUILD_USE_PGI
AM_CXXFLAGS = --nvcchost --no_preincludes
endif
endif

osu_init_SOURCES = osu_init.c $(UTILITIES)

if EMBEDDED_BUILD
    AM_LDFLAGS =
    AM_CPPFLAGS = -I$(top_builddir)/../src/include \
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
