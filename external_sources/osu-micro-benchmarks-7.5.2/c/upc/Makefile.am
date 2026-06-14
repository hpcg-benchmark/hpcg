AUTOMAKE_OPTIONS = subdir-objects

upcdir = $(pkglibexecdir)/upc
upc_PROGRAMS = osu_upc_memget osu_upc_memput osu_upc_all_barrier \
	       osu_upc_all_broadcast osu_upc_all_exchange \
	       osu_upc_all_gather_all osu_upc_all_gather osu_upc_all_reduce \
	       osu_upc_all_scatter

AM_CFLAGS = -I${top_srcdir}/c/util

UTILITIES = ../util/osu_util.c ../util/osu_util.h  ../util/osu_util_pgas.c ../util/util_pgas.h

if EMBEDDED_BUILD
     AM_CPPFLAGS = -I$(top_builddir)/../src/include \
                  -I${top_srcdir}/util \
                  -I${top_srcdir}/../src/include
endif

osu_upc_all_barrier_SOURCES = osu_upc_all_barrier.c $(UTILITIES)
osu_upc_all_broadcast_SOURCES = osu_upc_all_broadcast.c $(UTILITIES)
osu_upc_all_exchange_SOURCES = osu_upc_all_exchange.c $(UTILITIES)
osu_upc_all_gather_SOURCES = osu_upc_all_gather.c $(UTILITIES)
osu_upc_all_gather_all_SOURCES = osu_upc_all_gather_all.c $(UTILITIES)
osu_upc_all_reduce_SOURCES = osu_upc_all_reduce.c $(UTILITIES)
osu_upc_all_scatter_SOURCES = osu_upc_all_scatter.c $(UTILITIES)
