
SUBOBJ = sub.a(termio.o) sub.a(readdef.o) sub.a(nesig.o) sub.a(getmsg.o) \
	sub.a(alias.o) sub.a(xstring.o) sub.a(freemsg.o) sub.a(cdate.o) \
	sub.a(listsig.o) sub.a(sem.o) sub.a(getkey.o) \
	sub.a(getctl.o) sub.a(muserinf.o) sub.a(memdump.o) sub.a(readdef2.o) \
	sub.a(datetime.o) sub.a(fm2re.o) sub.a(readdef3.o) sub.a(affirm.o) \
	sub.a(pguser.o) sub.a(protos.o) sub(m_bogus.o)
#	sub.a(m_act.o) sub.a(m_at.o) sub.a(m_ch.o) sub.a(m_msg.o) \
#	sub.a(m_print.o) sub.a(m_who.o)

RANLIB = ranlib sub.a

LIB = /u/skynet/lib

SYSVER= BSD

#CFLAGS= -Zi
CFLAGS= -g -Wtraditional

sub.a: $(SUBOBJ)
	$(CC) $(CFLAGS) -c -DUNIX -D$(SYSVER) -DNO_STRDUP $(?:.o=.c)
	ar rv sub.a $?
	$(RANLIB)
	rm $?
#	rm -f $(LIB)/$@
#	ln $@ $(LIB)

sub(readdef.o): readdef.c

sub(getmsg.o): getmsg.c

.c.a:;

getkey: getkey.c 
	cc -O getkey.c  sub -D TESTING -o getkey

termio: termio.c
	cc -O -D TESTING termio.c -o termio

getmsg: gmtest.c sub
	cc -O gmtest.c -o getmsg -lmalloc sub ../msub/m_sub

readdef: readdef.c termio.o
	cc -O -D TESTING readdef.c termio.o -o readdef

readdef2: readdef2.c termio.o
	cc -O -D TESTING readdef2.c termio.o -o readdef2

readdef3: readdef3.c termio.o
	cc -O -D TESTING $@.c termio.o -o $@

alias: alias.c
	cc -O -DTESTING alias.c -o alias

xstring: xstring.c
	cc -O -DTESTING xstring.c -o xstring

cdate: cdate.c
	$(CC) $(CFLAGS) -DTESTING -DUNIX cdate.c -o cdate

fm2re: fm2re.c
	$(CC) $(CFLAGS) -DTESTING -DUNIX fm2re.c -o fm2re

datetime: datetime.c
	cc -O -D TESTING datetime.c -o datetime
