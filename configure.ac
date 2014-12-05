AC_INIT([CB], 1.0, [Eric Pederson (@sourcedelica)])
AC_CONFIG_MACRO_DIR([m4])
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
CC_CHECK_CFLAGS_APPEND([-Wno-implicit-int -Wno-dangling-else -Wno-implicit-function-declaration -Wno-unused-result])
LIBCURL_CHECK_CONFIG()
AC_OUTPUT
