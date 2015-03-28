AC_INIT([trans C++], [0.1], [vaidotas.valuckas@gmail.com],
        [trans], [https://github.com/rieske/trans])
AC_PREREQ([2.63])
AC_CONFIG_AUX_DIR(config)
AM_INIT_AUTOMAKE([1.11.1 subdir-objects -Wall no-define foreign parallel-tests color-tests])
AC_LANG(C++)
CXXFLAGS=
AC_PROG_CXX
m4_pattern_allow([AM_PROG_AR])
AC_CONFIG_HEADERS([config.h])
AC_ARG_ENABLE(debug,
    [  --enable-debug    generate debug symbols and instrument code for coverage reporting],
    [case "${enableval}" in
    yes) debug=true ;;
    no)  debug=false ;;
    *) AC_MSG_ERROR(bad value ${enableval} for --enable-debug) ;;
esac],[debug=false])
AM_CONDITIONAL(COND_DEBUG, test x$debug = xtrue)
AC_CONFIG_FILES([Makefile])
AM_PROG_AR
AC_OUTPUT
