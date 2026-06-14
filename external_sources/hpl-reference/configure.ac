AC_PREREQ([2.69])

AC_INIT(hpl, 2.3, hpl@icl.utk.edu)
AC_CONFIG_SRCDIR([include/hpl.h])
AC_CONFIG_HEADERS([include/hplconfig.h])

AC_CONFIG_MACRO_DIR([m4])

AX_PROG_CC_MPI

AC_PROG_RANLIB

AC_PROG_INSTALL

AM_INIT_AUTOMAKE([subdir-objects])

AM_PROG_CC_STDC
AM_PROG_CC_C_O

dnl
dnl AX_BLAS requires Fortran compiler and detects fortran libraries in $FLIBS
dnl
dnl AX_BLAS(LIBS="$BLAS_LIBS $LIBS $FLIBS")
dnl

HPL_BLAS(LIBS="$BLAS_LIBS $LIBS",AC_MSG_ERROR([BLAS not found]))

dnl FIXME: test for CBLAS: Atlas, MKL, OpenBLAS, ESSL, ...
dnl FIXME: test for GSL CBLAS

AC_CHECK_FUNCS([dgemm_])

AC_CHECK_HEADER([mpi.h], [], AC_MSG_ERROR([MPI not found]))

AC_CONFIG_FILES([Makefile src/Makefile testing/Makefile])

AC_OUTPUT
