
AM_CPPFLAGS = -I$(top_srcdir)/include

xhpl_LDADD = ../src/libhpl.a

bin_PROGRAMS = xhpl

xhpl_SOURCES =  \
matgen/HPL_jumpit.c matgen/HPL_rand.c matgen/HPL_setran.c matgen/HPL_xjumpm.c \
matgen/HPL_lmul.c matgen/HPL_ladd.c \
pmatgen/HPL_pdmatgen.c \
ptest/HPL_pddriver.c ptest/HPL_pdinfo.c ptest/HPL_pdtest.c \
ptimer/HPL_ptimer.c ptimer/HPL_ptimer_cputime.c ptimer/HPL_ptimer_walltime.c
