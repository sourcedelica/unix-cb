include $(top_srcdir)/Makefile.common

bin_PROGRAMS = cbinit CB cedit

CB_SOURCES = cb.c cbout.c cbinp.c cbetc.c cbcfg.c jive.l dorky.c

cbinit_SOURCES = cbinit.c cbcfg.c

cedit_SOURCES = cedit.c

LDADD = ../sub/libsub.a @LEXLIB@
