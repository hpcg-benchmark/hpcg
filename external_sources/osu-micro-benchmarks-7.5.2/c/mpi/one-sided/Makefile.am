AUTOMAKE_OPTIONS = subdir-objects

NVCC = @NVCC@
NVCFLAGS = -cuda -maxrregcount 32 -ccbin $(CXX) $(NVCCFLAGS)
SUFFIXES = .cu .cpp
.cu.cpp:
	$(NVCC) $(NVCFLAGS) $(INCLUDES) $(CPPFLAGS) --output-file $@.ii $<
	mv $@.ii $@

one_sideddir = $(pkglibexecdir)/mpi/one-sided
one_sided_PROGRAMS = osu_acc_latency osu_get_bw osu_get_latency osu_put_bibw osu_put_bw osu_put_latency

if MPI3_LIBRARY
    one_sided_PROGRAMS += osu_get_acc_latency osu_fop_latency osu_cas_latency
endif

AM_CFLAGS = -I${top_srcdir}/c/util

UTILITIES = ../../util/osu_util.c ../../util/osu_util.h 					   \
			../../util/osu_util_mpi.c ../../util/osu_util_mpi.h 			   \
			../../util/osu_util_graph.c ../../util/osu_util_graph.h 		   \
			../../util/osu_util_papi.c ../../util/osu_util_papi.h 			   \
			../../util/osu_util_validation.c
if SYCL
UTILITIES += ../../util/osu_util_sycl.cpp ../../util/osu_util_sycl.hpp
endif

if CUDA_KERNELS
UTILITIES += ../../util/kernel.cu
if BUILD_USE_PGI
AM_CXXFLAGS = --nvcchost --no_preincludes
endif
endif

osu_put_latency_SOURCES = osu_put_latency.c $(UTILITIES)
osu_put_bw_SOURCES = osu_put_bw.c $(UTILITIES)
osu_put_bibw_SOURCES = osu_put_bibw.c $(UTILITIES)
osu_get_latency_SOURCES = osu_get_latency.c $(UTILITIES)
osu_get_bw_SOURCES = osu_get_bw.c $(UTILITIES)
osu_acc_latency_SOURCES = osu_acc_latency.c $(UTILITIES)
osu_fop_latency_SOURCES = osu_fop_latency.c $(UTILITIES)
osu_cas_latency_SOURCES = osu_cas_latency.c $(UTILITIES)
osu_get_acc_latency_SOURCES = osu_get_acc_latency.c $(UTILITIES)

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
