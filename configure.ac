AC_INIT([CB], 1.0, [Eric Pederson (@sourcedelica)])
AM_INIT_AUTOMAKE([foreign])
AC_PROG_CC
AC_PROG_RANLIB
AC_CHECK_PROG([FLEX], [flex lex], [flex])
AM_MISSING_PROG([LEX], [$FLEX])
AM_PROG_LEX
AC_CONFIG_SRCDIR([cb/cb.c])
AC_CHECK_HEADERS(termios.h termio.h)
AC_CONFIG_FILES([Makefile
                 cb/Makefile
                 sub/Makefile])
AC_OUTPUT
