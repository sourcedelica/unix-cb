#
#	Unix-CB
#
#

IOBJ = cbinit.o cbcfg.o

FOBJ = cbfix.o 

CBJIVE = jive.o
#CBOBJ = CB.o cbout.o cbinp.o cbetc.o dorky.o cbcfg.o $(CBJIVE)
CBOBJ = cb.o cbout.o cbinp.o cbetc.o cbcfg.o $(CBJIVE)
BIN = ../bin
SKYUSER= eric
SUB= ../sub/sub.a

CFLAGS= -g -Wno-dangling-else -Wno-implicit-int -DHAVE_TERMIOS -DANSI
EXES= cbinit CB cbtest

OS := $(shell uname)
ifeq ($(OS),Darwin)
LIBS=-ll
else
LIBS=-lfl
endif

#
#	Dependencies
#

#	*** Be sure to su to SKYUSER before running make
#

install: all # setuid.m copy.m

all: $(EXES)

clean:
	rm -f $(EXES) $(CBOBJ)

copy.m: $(EXES)
	cp $? $(BIN)
	touch copy.m

setuid.m: $(EXES)
	chmod 6755 $?
	touch setuid.m

cbinit:	$(IOBJ)
	$(CC) $(CFLAGS) $(IOBJ) -o $@ $(SUB)

CB: $(CBOBJ)
	$(CC) $(CFLAGS) $(CBOBJ) $(LIBS) -o $@ $(SUB)

cbtest: $(CBOBJ)
	$(CC) $(CFLAGS) $(CBOBJ) -o $@ -ll $(SUB)

cedit: cedit.c
	$(CC) $(CFLAGS) cedit.c -o $@ -ll $(SUB)
