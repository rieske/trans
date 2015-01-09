AC_INIT([trans C++], [0.1], [vaidotas.valuckas@gmail.com],
        [trans], [https://github.com/rieske/trans])
AC_PREREQ([2.63])
AC_CONFIG_AUX_DIR(config)
AM_INIT_AUTOMAKE([1.11.1 -Wall no-define foreign])
AC_LANG(C++)
AC_PROG_CXX
AC_ARG_ENABLE([gcov],
    [AS_HELP_STRING([--enable-gcov])],
    [],
    [enable_gcov=no])
AM_CONDITIONAL([COND_GCOV],[test '!' "$enable_gcov" = no])
m4_pattern_allow([AM_PROG_AR])
AC_CONFIG_HEADERS([config.h])
LT_INIT
AC_CONFIG_FILES([Makefile])
AC_OUTPUT
