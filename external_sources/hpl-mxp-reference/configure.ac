AC_PREREQ([2.69])

AC_INIT(hpl-ai, 0.1, hpl-ai@icl.utk.edu)
AC_CONFIG_SRCDIR([hpl-ai.h])
AC_CONFIG_HEADERS([hpl-ai-config.h])

AC_CONFIG_MACRO_DIR([m4])

AC_PROG_INSTALL

AM_INIT_AUTOMAKE([subdir-objects])

AM_PROG_CC_STDC
AM_PROG_CC_C_O

AC_CHECK_HEADERS([sys/time.h math.h])

AC_SEARCH_LIBS(sqrt, m, , AC_MSG_ERROR([sqrt() function not found in standard C math library]))

AC_CHECK_FUNCS([fabs])

AC_CONFIG_FILES([Makefile])


AC_OUTPUT
