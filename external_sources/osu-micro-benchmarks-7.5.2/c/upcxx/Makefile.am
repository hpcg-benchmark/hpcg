AUTOMAKE_OPTIONS = subdir-objects

upcxxdir = $(pkglibexecdir)/upcxx
upcxx_PROGRAMS = osu_upcxx_allgather osu_upcxx_alltoall osu_upcxx_bcast \
	       osu_upcxx_gather osu_upcxx_reduce osu_upcxx_scatter \
	       osu_upcxx_async_copy_get osu_upcxx_async_copy_put

AM_CPPFLAGS = -I${top_srcdir}/c/util

UTILITIES =  ../util/osu_util.c ../util/osu_util.h ../util/osu_util_pgas.c ../util/osu_util_pgas.h

if EMBEDDED_BUILD
     AM_CPPFLAGS += -I$(top_builddir)/../src/include \
                  -I${top_srcdir}/util \
                  -I${top_srcdir}/../src/include
endif

osu_upcxx_allgather_SOURCES = osu_upcxx_allgather.cpp $(UTILITIES)
osu_upcxx_alltoall_SOURCES = osu_upcxx_alltoall.cpp $(UTILITIES)
osu_upcxx_bcast_SOURCES = osu_upcxx_bcast.cpp $(UTILITIES)
osu_upcxx_gather_SOURCES = osu_upcxx_gather.cpp $(UTILITIES)
osu_upcxx_reduce_SOURCES = osu_upcxx_reduce.cpp $(UTILITIES)
osu_upcxx_scatter_SOURCES = osu_upcxx_scatter.cpp $(UTILITIES)
osu_upcxx_async_copy_get_SOURCES = osu_upcxx_async_copy_get.cpp $(UTILITIES)
osu_upcxx_async_copy_put_SOURCES = osu_upcxx_async_copy_put.cpp $(UTILITIES)
