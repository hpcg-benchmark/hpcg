AUTOMAKE_OPTIONS = subdir-objects

openshmemdir = $(pkglibexecdir)/openshmem

openshmem_PROGRAMS = osu_oshm_get osu_oshm_put osu_oshm_put_mr \
					 osu_oshm_atomics osu_oshm_barrier osu_oshm_broadcast \
					 osu_oshm_collect osu_oshm_fcollect osu_oshm_reduce \
					 osu_oshm_get_nb osu_oshm_put_nb osu_oshm_put_overlap \
					 osu_oshm_get_mr_nb osu_oshm_put_mr_nb \
					 osu_oshm_get_bw osu_oshm_get_nb_bw \
					 osu_oshm_put_bw osu_oshm_put_nb_bw \
					 osu_oshm_get_overlap

AM_CFLAGS = -I${top_srcdir}/c/util

UTILITIES = ../util/osu_util.c ../util/osu_util.h ../util/osu_util_pgas.c \
			../util/osu_util_pgas.h ../util/osu_util_options.h


if EMBEDDED_BUILD
     AM_CPPFLAGS = -I$(top_builddir)/../src/include \
                  -I${top_srcdir}/util \
                  -I${top_srcdir}/../src/include
endif

osu_oshm_get_SOURCES = osu_oshm_get.c $(UTILITIES)
osu_oshm_put_SOURCES = osu_oshm_put.c $(UTILITIES)
osu_oshm_put_mr_SOURCES = osu_oshm_put_mr.c $(UTILITIES)
osu_oshm_atomics_SOURCES = osu_oshm_atomics.c $(UTILITIES)
osu_oshm_barrier_SOURCES = osu_oshm_barrier.c $(UTILITIES)
osu_oshm_broadcast_SOURCES = osu_oshm_broadcast.c $(UTILITIES)
osu_oshm_collect_SOURCES = osu_oshm_collect.c $(UTILITIES)
osu_oshm_fcollect_SOURCES = osu_oshm_fcollect.c $(UTILITIES)
osu_oshm_reduce_SOURCES = osu_oshm_reduce.c $(UTILITIES)
osu_oshm_get_nb_SOURCES = osu_oshm_get_nb.c $(UTILITIES)
osu_oshm_put_nb_SOURCES = osu_oshm_put_nb.c $(UTILITIES)
osu_oshm_put_overlap_SOURCES = osu_oshm_put_overlap.c $(UTILITIES)
osu_oshm_get_mr_nb_SOURCES = osu_oshm_get_mr_nb.c $(UTILITIES)
osu_oshm_put_mr_nb_SOURCES = osu_oshm_put_mr_nb.c $(UTILITIES)
osu_oshm_get_bw_SOURCES = osu_oshm_get_bw.c $(UTILITIES)
osu_oshm_get_nb_bw_SOURCES = osu_oshm_get_nb_bw.c $(UTILITIES)
osu_oshm_put_bw_SOURCES = osu_oshm_put_bw.c $(UTILITIES)
osu_oshm_put_nb_bw_SOURCES = osu_oshm_put_nb_bw.c $(UTILITIES)
osu_oshm_get_overlap_SOURCES = osu_oshm_get_overlap.c $(UTILITIES)
