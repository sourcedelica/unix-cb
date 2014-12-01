bin_PROGRAMS = cbinit CB cedit

CB_SOURCES = cb.c cbout.c cbinp.c cbetc.c cbcfg.c jive.l

cbinit_SOURCES = cbinit.c cbcfg.c

cedit_SOURCES = cedit.c

LDADD = ../sub/libsub.a @LEXLIB@

AM_CFLAGS=-Wno-implicit-int -Wno-implicit-function-declaration -Wno-dangling-else -DHAVE_TERMIOS -DANSI
