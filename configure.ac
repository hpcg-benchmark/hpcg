AC_PREREQ([2.69])

AC_INIT(hpcg, 3.1, info@hpcg-benchmark.org)
AC_CONFIG_SRCDIR([src/main.cpp])
dnl HPCG requires command line configuration
dnl AC_CONFIG_HEADERS([src/hpcgconfig.hpp])

AC_CONFIG_MACRO_DIR([m4])

m4_define(hpcg_contiguous_arrays_help,[enable contiguous arrays storage for better cache pre-fetch])
AC_ARG_ENABLE(contiguous_arrays,
    [AS_HELP_STRING([--enable-contiguous-arrays], hpcg_contiguous_arrays_help [@<:@default=no@:>@])],
    AS_IF([test "x${enable_contiguous_arrays}" = xyes], AC_DEFINE([HPCG_CONTIGUOUS_ARRAYS], [1], hpcg_contiguous_arrays_help)))

AC_ARG_WITH(debug,
    [AS_HELP_STRING([--with-debug=none,basic,detailed], [enable debugging output @<:@default=none@:>@])],
    AS_IF([test xnone != x$with_debug], AC_DEFINE([HPCG_DEBUG], [1], [Define to enable basic debugging output]))
    AS_IF([test xdetailed = x$with_debug],
    AC_DEFINE([HPCG_DETAILED_DEBUG], [1], [Define to enable detailed debugging output])))

AC_ARG_ENABLE(mpi,
    [AS_HELP_STRING([--enable-mpi],[enable MPI support @<:@default=no@:>@])],
    dnl detect MPI later due to Autoconf syntax error in `configure'
    AS_IF([test "x${enable_mpi}" = xno], AC_DEFINE([HPCG_NO_MPI], [1], [Define if MPI support is disabled])),
    AC_DEFINE([HPCG_NO_MPI], [1], [Define if MPI support is disabled]))

m4_define(hpcg_cubic_radical_search_help, [enable cubic radical search for 3D process grid])
AC_ARG_ENABLE(cubic_radical_search,
    [AS_HELP_STRING([--enable-cubic-radical-search], hpcg_cubic_radical_search_help [@<:@default=no@:>@])],
    AS_IF([test "x${enable_cubic_radical_search}" = xyes], AC_DEFINE([HPCG_CONTIGUOUS_ARRAYS], [1], hpcg_cubic_radical_search_help)))

m4_define(hpcg_longlong_help, [enable use of long long type for global indices])
AC_ARG_ENABLE(longlong,
    [AS_HELP_STRING([--enable-longlong], hpcg_longlong_help [@<:@default=yes@:>@])],
    AS_IF([test "x${enable_longlong}" = xno], AC_DEFINE([HPCG_NO_LONG_LONG], [1], hpcg_longlong_help)))

AC_ARG_ENABLE(openmp,
    [AS_HELP_STRING([--enable-openmp], [enable OpenMP support [@<:@default=no@:>@]])],
    AS_IF([test "x${enable_openmp}" = xno], AC_DEFINE([HPCG_NO_OPENMP], [1], [disable OpenMP support])),
    AC_DEFINE([HPCG_NO_OPENMP], [1], [disable OpenMP support]))

AC_PROG_RANLIB

AC_PROG_INSTALL

AM_INIT_AUTOMAKE([subdir-objects])

dnl select programming language as OpenMP tests require it
AC_LANG(C++)

AS_IF([test "x${enable_mpi}" != xyes], AC_MSG_NOTICE([MPI not enabled and it is OK for the following tests to fail.]))

AX_PROG_CXX_MPI([test x"$enable_mpi" != xno],[],[
    if test x"$with_mpi" = xyes; then
    AC_MSG_FAILURE([MPI compiler requested, but MPI unusable.])
    else
    AC_MSG_WARN([No MPI compiler found and MPI is diabled.])
    fi])

dnl AX_PROG_CC_MPI and AX_PROG_CXX_MPI must not be inside raw shell "if"
dnl construct because Autoconf macros create 'configure' with syntax errors.
dnl For example, this will cause syntax error when running "configure":
dnl AS_IF([test "x${enable_mpi}" = yes], AX_PROG_CXX_MPI, AC_PROG_CXX)

dnl Cannot use AS_IF() because of syntax errors.
dnl AS_IF([test "x${enable_mpi}" = xyes], AC_CHECK_HEADER([mpi.h], [], AC_MSG_ERROR([MPI not found])))
if test "x${enable_mpi}" = xyes ; then :
    AC_CHECK_HEADER([mpi.h], [], AC_MSG_ERROR([MPI not found]))
fi

dnl
dnl AC_PROG_CC and AC_PROG_CXX have to be after AX_PROG_CXX_MPI to properly pickup
dnl mpic++ and link C++ bindings (if present or if they get added to the object
dnl files by sloppy compiler/linker). In other words, the order matters.
dnl

dnl AC_PROG_CXX has to be outside of conditionals not to cause Automake variable AMDEP error.
dnl The first AC_PROG_CXX will be used to set AMDEP variables and if they happen
dnl to execute conditionally then "configure" invocation fails.
AC_PROG_CXX
dnl AC_PROG_CXX_C_O

dnl Configuring C compiler is required for am__fastdepCC or Automake errors out (unless the C++ langauge is enforced).
dnl AC_PROG_CC
dnl AC_PROG_CC_C_O

if test "x${enable_openmp}" = xyes ; then :
    AX_OPENMP
    CXXFLAGS="$CXXFLAGS $OPENMP_CXXFLAGS"
else
    AC_DEFINE([HPCG_NO_OPENMP], [1], [disable OpenMP support])
fi

AC_CONFIG_FILES([Makefile])

AC_OUTPUT
