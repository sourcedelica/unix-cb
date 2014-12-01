#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <setjmp.h>
#include <unistd.h>
#include <sys/msg.h>
#include "../include/osdefs.h"
#include "../include/exitcodes.h"
#include "../include/xstring.h"
#include "cb.h"
#include "cbetc.h"
#include "cbcfg.h"

/*
 *	Unix-CB
 *
 *	Input and command handler
 *	(child)
 *
 *	Copyright (C) 1988 CPEX Development
 *	All Rights Reserved.
 *	Author: Eric Pederson
 *
 */

extern void cmd();
extern void xpriv();
extern void chandle();
extern void cchan();
extern void xformat();
extern void getop();
extern void actit();
extern void info();
extern void browse();
extern void toggle();
extern void modify();
extern void ochange();
extern void sqmsg();
extern void sqit();
extern void boot();
extern void xstd();
extern void chlf();
extern void sendkill();
extern void doboot();
extern char *gname();
extern void tlist();
extern void vuser();
extern void pguser();
extern void dummy();
extern int listsig(char *);
extern void sane();
extern void raw();
extern void sndfix();
extern void logpa(char *);
extern int rdonly();
extern void unnesig();
extern int affirm(char *prompt);
STATIC int chchan( int chan );
void ungag();
STATIC void talist();
STATIC int cmdxlat();
STATIC void helper();
int readdefault(char *pr, char *x, int lx, char *def);
int Pu(int sid);
int Vu(int sid);
int gmatch( int gslot, int glob );
STATIC int inp();
STATIC int ckp2me();
STATIC int oplist( int mopt, int all, int *xi, int *xp, char **xa );
int checksq(int source, int target);
STATIC int sqlist( int slot );
int setnesig();


#define P_PRIV		0
#define P_AGAIN		1
#define P_STATION	2
#define P_VIP		3
#define	P_PA		4
#define P_RAGAIN	5
#define P_XMIT		6
#define P_RPA		7

#define OC_USER		0
#define OC_CF		1

#define max(x,y)	( (x) > (y) ? (x) : (y) )

static int opti[] = {  OP_SQLOCK, OP_CHLOCK, OP_LFLOCK, OP_PMLOCK,
		OP_CCLOCK, OP_SYSOP, OP_COSYSOP, OP_ACTIVE, OP_SQNON,
		OP_PAID, OP_SQBEEP, OP_SQLF, OP_MONIT, OP_LIST,
		OP_ANSI, OP_SQSTAT, OP_JIVE, OP_RDOVER, OP_VALID,
		OP_PABEEP, OP_UGLYLINK, OP_USED };

static int optp[] = {  0, 0, 0, 0,
		0, 0, 0, OP_COSYSOP, OP_COSYSOP,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, OP_VIP };

static char *opta[] = { "Squelching locked", "Handle changing locked",
		 "Linefeeds locked", "Private messages locked",
		 "Channel changing locked", "Sysop", "Co-Sysop",
		 "Active", "Non-chan messages squelched", "Paid", 
		 "Beeps squelched", "Linefeeds squelched",
		 "Monitoring active", "Channels listed", 
		 "ANSI Output", "Station messages squelched", 
		 "Jive translation", "Read-only override", 
		 "Validated", "PA messsages beep", "Ugly link",
		"Used (don't use)", NULL };

static int cfi[] = { CF_LOCKED, CF_RDONLY, CF_VRDONLY };
static int cfp[] = { 0, 0, 0 };
static char *cfa[] = { "CB locked", "Read-only for non-members",
			"Read-only for unvalidated", NULL };

static char *gnames[] =
	{ "All users", "All members", "All non-members",
		"All sysops", "All co-sysops" };
		
static char s[MSGLEN+1];
static struct cbmsg mbuf;
static long parent;
static jmp_buf env;

#ifdef SKYNET
extern char *m_uname(), *m_alias();
#endif
extern TERMIO_OR_TERMIOS oldterm;

/**********************************************************************/

void cbinp()
{
	char c;
	int i;

	signal(SIGHUP,SIG_DFL);
	signal(SIGQUIT,SIG_IGN);
	signal(SIGINT,SIG_IGN);
	cbpid = (long)getpid();
	parent = (long)getppid();

	for(;;){
		memset(s,0,MSGLEN+1);
		i = 0;
		M_R.mfrom = cbpid;
		mbuf.mtype = parent;
		M_R.cbtype = CB_NOIN;
		if( msgsnd(mid,&mbuf,L_cbreq,ipcflag) == -1 )
			sndfix(&mbuf);
		c = getchar();
		M_R.cbtype = CB_INREQ;
		M_R.mfrom = cbpid;
		mbuf.mtype = parent;
		if( msgsnd(mid,&mbuf,L_cbreq,ipcflag) == -1 )
			sndfix(&mbuf);
		if( msgrcv(mid,&mbuf,L_cbreq,cbpid,0) == -1 )
			rcvfix();
		if( c == '/' )
			cmd(c);
		else {
			strcpy( MYREC.doing, DOTYPE );
			i = inp(i,c);
			*MYREC.doing = 0;
			if( i != 0 )
				if( !rdonly() )
					xmsg(MYREC.chan,s,0,"");
				else
					printf("You are able to read only\n");
		}
	}
}


STATIC void cmd( c )
char c;
{
	/*
	 *	Process commands
	 *
	 */

	char cc, s[80];
	int i;

	putchar(c);
	c = getchar();
	if( c == '\b' || c == 0177 || c == 022 || c == 033 ){
		printf("%c %c",'\b','\b');
		return;
	}
	printf("%c\n",c);
	c = toupper( c );
	if( (c= cmdxlat( c )) == -1 )
		return;
	switch(toupper(c)){
		case 'O':
			if( !affirm("Sure? ") )
				break;
			/* FALLS THROUGH */
		case 'G':
		case 'Q':
			if( ckp2me() ){
				printf("Somebody is sending you a private message right now\n");
				if( !affirm("Sign off? ") )
					break;
			}
			mbuf.mtype = parent;
			M_R.mfrom = cbpid;
			M_R.cbtype =
				toupper(c) == 'O' ? CB_LOGOFF : CB_LOGOUT;
			if( toupper(c) == 'G' )
				MYREC.opts &= ~OP_ACTIVE;
			if( msgsnd(mid,&mbuf,L_cbreq,ipcflag) == -1 )
				sndfix(&mbuf);
			exit(EXITOK);
		case 'P':
			xpriv(P_PRIV);
			break;
		case '/':
			xpriv(P_AGAIN);
			break;
		case ',':
			xpriv(P_RAGAIN);
			break;
		case 'Z':
			xpriv(P_PA);
			break;
		case 'D':
			xpriv(P_STATION);
			break;
		case 'N':
			xpriv(P_XMIT);
			break;
		case '*':
			xpriv(P_VIP);
			break;
		case '@':
			xpriv(P_RPA);
			break;
		case '!':
			if( readdefault("!",s,80,"") ){
				sane();
				strcpy( MYREC.doing, DOSHELL );
				signal( SIGINT, dummy );
				signal( SIGQUIT, dummy );
				i = geteuid();
				setuid( getuid() );
				if( system(s) == -1 )
					perror("system call");
				else puts("!");
				setuid( i );
				signal( SIGINT, SIG_IGN );
				signal( SIGQUIT, SIG_IGN );
				*MYREC.doing = 0;
				raw();
			}
			break;
		case 'H':
			chandle();
			break;
		case 'U':
			chlf();
			break;
		case 'A':
			alist();
			break;
		case 'F':
			xformat();
			break;
		case 'C':
			cchan();
			break;
		case '?':
			if( CBHELP != NULL ) listsig( cbfn(CBHELP) );
			else helper();
			break;
		case '.':
			actit( TRUE, 0 );
			break;
		case '-':
			actit( FALSE, CB_DEACT );
			break;
		case '+':
			actit( FALSE, CB_ACTIVATE );
			break;
		case 'Y':
			info( FALSE );
			break;
		case 'I':
			info( TRUE );
			break;
		case 'J':
			browse();
			break;
		case '^':
			ochange( OC_USER );
			break;
		case '>':
			ochange( OC_CF );
			break;
		case '=':
			modify();
			break;
		case 'B':
			boot(SIGQUIT);
			break;
		case 'K':
			boot(SIGKILL);
			break;
		case '1':
			toggle(OP_SQBEEP,"Beeps","un","squelched");
			break;
		case '2':
			toggle(OP_SQLF,"Linefeeds","un","squelched");
			break;
		case '3':
			toggle(OP_ANSI,"ANSI output","not ","active");
			break;
		case '4':
			toggle(OP_SQSTAT,"Station messages","un","squelched");
			break;
		case '5':
			toggle(OP_JIVE,"Jive translation","not ","active");
			break;
		case '6':
			toggle(OP_PABEEP,"PA Messages now","do not ","beep");
			break;
		case '8':
			toggle(OP_SQNON,"Non-chan","un","squelched");
			break;
		case '9':
			toggle(OP_SQPILF,"Non-chan","un","squelched");
			break;
		case 'L':
			toggle(OP_LIST,"Channels","not ","listed");
			break;
		case 'M':
			toggle(OP_MONIT,"Monitoring","not ","active");
			break;
		case 'X':
			sqit();
			break;
		case '$':
#ifdef SKYNET
			m_who(skynet ? slot : -1);
#else
			system(UNIXWHO);
#endif
			putchar('\n');
			break;
#ifdef SKYNET
		case 'R':
			if( ckp2me() ){
				printf("Somebody is sending you a private message right now\n");
				if( !affirm("Enter private chat? ") )
					break;
			}
			dorky();
			alist();
			break;
		case 'T':
			tlist(slot);
			break;
		case 'V':
			vuser();
			break;
		case '%':
			pguser( skynet ? slot : -1 );
			break;
#endif
		case 'E':
			talist();
			break;
		case '&':
			ungag();
			break;
		default:
			printf("Invalid command\n");
			break;
	}
}

STATIC int cmdxlat( c )
char c;
{
	/*	Translates key 'c' into a cmdval
	 	or returns -1 on failure
	 */

	char *x;
	struct cmdnode *temp;

	for( temp= cmdlist; temp != CNNULL; temp= temp->next )
		if( (x= strchr( temp->acts, c )) != NULL )
			if( cmdops[temp->index] )
				if( !(MYREC.opts & cmdops[temp->index]) ){
					printf("%s\n", cmdbads[temp->index]);
					return( -1 );	
				} else return( temp->type );
			else return( temp->type );
	printf("%s\n",MSGBAD);
	return( -1 );
}

STATIC int inp( i, c )
int i;
char c;
{
	/*
	 *	Get a public message
	 *
	 */

	int acaps, cnow;

	acaps = 0;
	for(;;c= getchar()){
		if( c == oldterm.c_cc[VKILL] && c != '@' )
			c = 033;
		switch(c){
			case 001:
				acaps = !acaps;
				break;
			case 033:	
				if( i > 0 )
					putchar('\n');
				return(0);
			case '\n':
			case '\r':
				if( i > 0 )
					putchar('\n');
				return(i);

			case 0177:
			case '\b':
				if( i > 0 ){
					if( s[i-1] != 007 )
						printf("\b \b");
					s[--i] = 0;
					if( i == 0 )
						return(0);
				} else
					return(0);
				break;
			case 022:
			case 030:
				while( i > 0 ){
					if( s[i-1] != 007 )
						printf("\b \b");
					if( s[--i] == ' ' && c == 022 ){
						s[i] = 0;
						break;
					} else
						s[i] = 0;
				}
				if( i == 0 )
					return(0);
				break;
			default:
				if( acaps && islower(c) && (cnow= !cnow) )
					c = toupper(c);
				if( i < MSGLEN && (c == 007 || !iscntrl(c)) ){
					putchar(c);
					s[i++] = c;
					s[i] = 0;
				}
				break;
		}
	}
}

STATIC void xpriv( ptype )
int ptype;
{
	/*
	 *	Send a private message
	 *
	 *	Ptype is
	 *	P_PRIV - regular private message
	 *	P_AGAIN - send to last person
	 *	P_RAGAIN - send to last person who sent you
	 *	...
	 *	P_XMIT	- transmit a public message on a given chan
	 *	P_RPA - transmit a RPA to someone specific
	 *
	 */

	static char last[L_cbalias];
	char s[L_cbalias], t[L_cbalias];
	char msg[MSGLEN+1];
	int to, i, j, chan, dolast;

	if( rdonly() ){
		printf("You are able to read only\n");
		return;
	}
	if( MYREC.opts & OP_PMLOCK ){
		printf("Private messages locked\n");
		return;
	}
	if( ptype == P_RAGAIN ){
		if( MYREC.rlastp == S_NOBODY ){
			printf("Never received private message\n");
			return;
		}
		ptype = P_AGAIN;
		dolast = MYREC.rlastp;
	} else if( ptype == P_AGAIN ){
		if( MYREC.lastp == S_NOBODY ){
			printf("Never sent private message\n");
			return;
		}
		dolast = MYREC.lastp;
	}

	if( ptype == P_AGAIN ){
		if( !isact(slot,dolast) ){
			printf("%s no longer active\n",
				ulog[dolast].handle);
			return;
		}
		printf("To: %s (%s)\n",ulog[dolast].userid,
			ulog[dolast].handle);
		strcpy(s,ulog[dolast].handle);
		strcpy(t,s);
		i = dolast;
	} else if( ptype == P_STATION || ptype == P_PA )
		*s = 0;
	else if( ptype != P_XMIT )
		if( readdefault("To: ",s,L_cbalias,"") == 0 )
			return;

	if( ptype != P_AGAIN && ptype != P_XMIT )
		if( (i= lookup(s,t)) < 0 ){
			if( i == S_NOBODY ){
				printf("%s is not active\n",s);
				return;
			} else if( i == S_INVALID ){
				printf("%s is invalid\n",s);	
				return;
			}
		}

	if( ptype == P_XMIT ){
		if( readdefault("Chan: ", s, 5, "") == 0 )
			return;
		chan = atoi( s );
		if( chan < 1 || chan > MAXCHAN ){
			printf("Invalid channel\n");
			return;
		}
		i = 0;
		*t = 0;
	}
	if( i >= 0 && ptype != P_XMIT && ptype != P_RPA ){
		if( ptype == P_PRIV || ptype == P_AGAIN )
			if( checksq(i,slot) & SQ_BOTH ){
				printf("Squelched by %s\n",t);
				return;
			}
		if( ulog[i].dcreq != -1 ){
			printf("%s is in private chat\n",s);
			return;
		}
	}
	if( ptype == P_AGAIN || ptype == P_PRIV )
		MYREC.towho = i;
	else MYREC.towho = S_NOBODY;
	strcpy( MYREC.doing, DOTYPE );
	j = readdefault("Msg: ",msg,MSGLEN+1,"");
	MYREC.towho = S_NOBODY;
	*MYREC.doing = 0;
	if( j == 0 )
		return;

	if( i >= 0 && ptype != P_XMIT ){
		if( !isact(slot,i) ){
			printf("%s is no longer active\n",s);
			return;
		}
		if( ulog[i].dcreq != -1 ){
			printf("%s has entered private chat\n",s);
			return;
		}
	}

	if( ptype == P_STATION )
		chan = MSG_STATION;
	else if( ptype == P_VIP )
		chan = MSG_VIP;
	else if( ptype == P_PA )
		chan = MSG_PA;
	else if( ptype == P_RPA )
		chan = MSG_RPA;
	else if( ptype != P_XMIT )
		chan = MSG_PRIV;

	xmsg(chan,msg,i,t);
	if( ptype == P_PRIV || ptype == P_AGAIN || ptype == P_RAGAIN ){
		strcpy(last,t);
		if( i >= 0 )
			MYREC.lastp = i;
		xmsg(MSG_RPA,"Sent",slot,"");
	}
	if( ptype == P_RPA )
		puts("Sent");
}

STATIC void chandle()
{
	/*
	 *	Change handle
	 *
	 */

	char s[L_cbalias], old[L_cbalias], msg[128];

	if( MYREC.opts & OP_CHLOCK ){
		printf("Cannot change handle\n");
		return;
	}
	if( readdefault("New: ",s,L_cbalias,"") == 0 )
		return;

	if( !isval(s) ){
		printf("Invalid handle\n");
		return;
	}

	strcpy(old,MYREC.handle);

	if( chhand( s ) )
		printf("Handle already used\n");
	else {
		printf("Changed\n");
		sprintf(msg,"%s/%s is now %s/%s",MYREC.userid,old,
				MYREC.userid,MYREC.handle);
		xmsg( MSG_PPA, msg, 0, "" );
	}
}


STATIC void xformat()
{
	/*
	 *	Change output format
	 *
	 */

	char s[80], c, *dest, help[80], *fn;
	int go, typ;
	struct cbmsg mbuf;

	for( go= 1; go; ){
		go = 0;
		printf("M,A,Q,?: ");
		c = getchar();
		putchar(c);
		putchar('\n');
		switch( toupper(c) ){
			case 'M':	typ = 1;
					dest = MYREC.format;
					strcpy(help, cbfn(FORMSG));
					fn = cbfn(BUILTIN);
					break;
			case 'A':	typ = 0;
					dest = MYREC.aformat;
					strcpy(help, cbfn(AFORMSG));
					fn = cbfn(ABUILTIN);
					break;
			case '\n':
			case '\r':
			case 'Q':	puts("Format not changed");
					return;
			case '?':	puts("Change format");
					puts("M)essage, A)ctive, Q)uit, ?)Help");
					go = 1;
					break;
			default:	puts("Invalid option");
					go = 1;
		}
	}
	
	strcpy( MYREC.doing, DOFORM );
	printf("Current format is:\n%s\n",dest);
	printf("Press RETURN to keep current format\n");
	do {
		printf("Press ? for help\n");
		if( !readdefault("",s,FORMAX-1,"") ){
			printf("Format unchanged - you are back in CB\n");
			*MYREC.doing = 0;
			return;
		}
		if( *s == '?' )
			if( listsig( help ) == -1 )
				printf("%s not found\n",help);
		if( *s == '@' )
			xstd( s, fn );
	} while( *s == '?' );
	if( typ )
		if( strstr( s, "%m" ) == NULL ){
			printf("%%m not included in format string.  Added at end.\n");
			s[FORMAX-5] = 0;
			strcat( s, " %m" );
		}
	*MYREC.doing = 0;
	strcpy(dest,s);
	puts("Changed - you are back in CB");
}


STATIC void cchan()
{
	/*
	 *	Change channel
	 *
	 */

	char s[128];
	int i, old;

	if( MYREC.opts & OP_CHLOCK ){
		printf("Cannot change channel\n");
		return;
	}
	if( !readdefault("Ch: ",s,9,"") )
		return;
	i = atoi(s);
	if( i < 1 || i > MAXCHAN ){
		printf("%s is an invalid channel\n",s);
		return;
	}
	if( !(MYREC.opts & OP_PAID) && ( i < RCLOW || i > RCHIGH ) ){
		printf("Channel %d requires a subscription\n",i);
		return;
	}

	old = MYREC.chan;

	if( chchan( i ) )
		puts("Channel busy");
	else {
		if( i != old && (MYREC.opts & OP_ACTIVE) ){
			sprintf( s, "Left channel: %s/%s",
				MYREC.userid, MYREC.handle );
			xmsg( MSG_CPA, s, old, "" );
			MYREC.chan = -1;	/* Fake out CPA */
			sprintf( s, "Joined channel: %s/%s",
				MYREC.userid, MYREC.handle );
			xmsg( MSG_CPA, s, i, "" );
			MYREC.chan = i;
		}
		puts("Changed");
	}
}


int who( pr, s, t )
char *pr, *s, *t;
{
	/*
	 *	Prompt for user and lookup
	 *	
	 *	Returns S_INVALID if not found,
	 *	S_NOBODY if c/r hit.
	 */

	int i;

	if( !readdefault(pr,s,L_cbalias,"") )
		return(S_NOBODY);

	if( (i= lookup(s,t)) == S_NOBODY )
		return(S_INVALID);
	return(i);
}


STATIC void actit( me, atype )
int me;
int atype;
{
	/*
	 *	Activate/Deactivate user.
	 *	
	 *	Me is non-zero to modify requestor's slot
	 *	atype = ( CB_ACT, CB_DEACT )
	 *
	 */

	char s[80];
	int i;

	if( !me ){
		if( (i= who("Who: ",s,NULL)) == S_NOBODY )
			return;
		else if( i < 0 ){
			printf("%s not found\n",s);
			return;
		}
	} else
		i = slot;

	if( me )
		atype = (MYREC.opts & OP_ACTIVE) ? CB_DEACT : CB_ACTIVATE;
	sprintf(s,"%d",i);
	if( atype == CB_ACTIVATE ){
		if( ulog[i].opts & OP_ACTIVE )
			printf("Already active\n");
		else {
			ulog[i].opts |= OP_ACTIVE;
			msglog(i,atype);
#ifdef SKYNET
			if( m_slot( s ) != -1 )
				m_chdoing( i, cbdoing );
#endif
		}
	} else
		if( ulog[i].opts & OP_ACTIVE ){
			msglog(i,atype);
			ulog[i].opts &= ~OP_ACTIVE;
#ifdef SKYNET
			if( m_slot( s ) != -1 )
				m_chdoing( i, FAKEOUT );
#endif
		} else
			printf("Already inactive\n");
}


STATIC void info( ask )
int ask;
{
	/*
	 *	Display pertinent info for a user
	 *
	 *	If ask is non-zero, ask for a slot, otherwise
	 *	use slot
	 *
	 */

	char s[L_cbalias];
	int i, j;

	if( ask ){
		if( (i= who("Who: ",s,NULL)) == S_NOBODY )
			return;
		if( i < 0 ){
			printf("%s not active\n",s);
			return;
		}
	} else
		i = slot;

	printf("%s (%s)\n",ulog[i].handle,ulog[i].userid);
	printf("Slot #%d\n",i);
	printf("Channel %.2d\n",ulog[i].chan);
	printf("Linefeed character: %c\n",ulog[i].lfchar);
	printf("Msg Format: %s\n",ulog[i].format);
	printf("Active Format: %s\n",ulog[i].aformat);
	if( (ulog[i].opts & OP_COSYSOP) && !(ulog[i].opts & OP_SYSOP) &&
		QUOTA != -1 )
			printf("Grant time remaining: %d mins\n",
					ulog[i].qremain);
#ifdef SKYNET
	tlist(i);
#endif
	oplist(ulog[i].opts,FALSE, opti, optp, opta);
	j = sqlist(i);
	if( j > 0 )
		printf("Love taps: %d\n",j);
}

STATIC void browse()
{
	/*
	 *	Read previous messages
	 *
	 */

	int last, cursize, given, i, j, saveslot;
	long pos;
	struct dmsg dm;		
	struct cbmsg mbuf;
	char s[80];
	int bcatch();

	last = *pnext + 1;
	pos = lseek(out, 0L, 2);
	cursize = pos / sizeof(struct dmsg);
	printf("Messages are 1 through %d\n",cursize);
	if( !readdefault("Start at: ",s,10,"") )
			return;
	given = atoi(s);
	if( given < 1 || given > cursize ) {
			printf("%s out of range\n",s);
			return;
	}
	if( given == 1 )
		given++;
	i = last - ( cursize - given + 1 );
	if( i < 0 )
		i += cursize;
	if( last == cursize ){
		last = 0;
		i = 1;
	}
	if( setjmp(env) ){
		putchar('\n');
	} else {
		printf("------\n");
		setnesig(' ',033, bcatch );
		for( j= i; j != last ;j = (j+1) % cursize){
				lseek(out,(long)(j*sizeof(struct dmsg)),0);
				read(out,&dm,sizeof(dm));
				if( (MYREC.opts & OP_SQPILF) &&
					dm.chan == MSG_PRIV )
						continue;
				printone(&dm);
		}
	}
	printf("------\n");
	unnesig();
}


STATIC int bcatch( arg )
int arg;
{
	signal(arg,SIG_IGN);
	longjmp(env,-1);
}


STATIC void toggle( flag, m1, m2, m3 )
int flag;
char *m1;
char *m2;
char *m3;
{
	/*
	 *	Toggle option bits for current user
	 *
	 *	Prints m2 when result is unset
	 */

	MYREC.opts ^= flag;
	printf("%s %s%s\n",m1,((MYREC.opts & flag) ? "" : m2),m3);
}


STATIC void modify()
{
	/*
	 *	Modify User
	 *
	 */

	char s[80];
	int i, go;

	if ( (i=who("Who: ",s,NULL)) == S_NOBODY )
		return;
	if ( i < 0 ) {
		printf("%s not active\n",s);
		return;
	}

	printf("Handle (%s): ",ulog[i].handle);
	if ( readdefault("",s,L_cbalias,""))
			strcpy(ulog[i].handle,s);
	printf("Userid (%s): ",ulog[i].userid);
	if ( readdefault("",s,L_cbalias,""))
			strcpy(ulog[i].userid,s);
	printf("Channel (%d): ",ulog[i].chan);
	if ( readdefault("",s,L_cbalias,""))
			ulog[i].chan = atoi(s);
	printf("Linefeed character (%c): ",ulog[i].lfchar);
	if ( readdefault("",s,L_cbalias,""))
			ulog[i].lfchar = *s;
	printf("Doing (%s): ",ulog[i].doing);
	if ( readdefault("",s,FORMAX,""))
			strcpy(ulog[i].doing,s);
	printf("Grant time (%d): ", ulog[i].qremain);
	if( readdefault("", s, 10, "") )
		ulog[i].qremain = atoi( s );
	for( go= 1; go; ){
		printf("Format (%s): ",ulog[i].format);
		if ( readdefault("",s,FORMAX,"")){
			if( *s == '@' ){
				xstd( s, cbfn(BUILTIN) );
				if( *(s+1) == '?' )
					continue;
			}
			strcpy(ulog[i].format,s);
		}
		go = 0;
	}
	for( go= 1; go; ){
		printf("Active format (%s): ",ulog[i].aformat);
		if ( readdefault("",s,FORMAX,"")){
			if( *s == '@' ){
				xstd( s, cbfn(ABUILTIN) );
				if( *(s+1) == '?' )
					continue;
			}
			strcpy(ulog[i].aformat,s);
		}
		go = 0;
	}

	printf("Updated\n");
}


STATIC void ochange( octype )
int octype;
{
	/*
	 *	Change another user's options
	 *
	 */

	int i, j, mopt, mwho, temp;
	char s[L_cbalias];
	int topid;
	int *xi, *xp;
	char **xa;

	if( octype == OC_USER ){
		if( (mwho= who("Who: ",s,NULL)) == S_NOBODY )
				return;
		if( mwho < 0 ){
			printf("%s not active\n",s);
			return;
		}
		temp = ulog[mwho].opts;
		xi = opti; xp = optp; xa = opta;
	} else {
		temp = *pcbflag;
		xi = cfi; xp = cfp; xa = cfa;
	}

	for(;;){
		i= oplist( temp, TRUE, xi, xp, xa );
		for(;;){
			if ( !readdefault("Toggle: ",s,3,"") ){
				if( octype == OC_USER )
					ulog[mwho].opts = temp;
				else
					*pcbflag = temp;
				return;
			}
			if ( *s == '?' )
				break;
			j = atoi(s);
			if( j < 1 || j > i ){
				printf("%s is not valid\n",s);
				continue;
			}
			temp ^= opti[j-1];
		}
	}
}


STATIC int sqlist( slot )
int slot;
{
	/*
	 *	Print list of squelched people for slot
	 *
	 *	As a side benefit, returns number of kill votes slot has
	 *
	 */

	int i, j;
	struct sqrec *sq;

	for( i=j= 0; i < ulsize; i++ ){
		if( isact(slot, i) ){
			sq = getsq( slot, i );
			if( sq->sqtype & SQ_STYPE )
				sqmsg(ulog[i].handle,sq->sqtype);
			if( sq->sqtype & SQ_KILL )
				j++;
		}
	}
	for( i= 0; i < S_NUMGLOB; i++ ){
		sq = getsq( slot, ulsize+i );
		if( sq->sqtype & SQ_STYPE )
			sqmsg(gname(S_ALL+i),sq->sqtype);
	}
	return(j);
}


STATIC void sqmsg( t, type )
char *t;
int type;
{
	/*
	 *	Readable squelch status
	 *
	 */

	static char *sqc[] =
		{ "Reverse", "Private", "Comprehensive", "Passive", NULL };
	static int sqi[] = { SQ_REVERSE, SQ_PRIV, SQ_COMP, SQ_PASSIVE };
	int i, j;

	printf("%s: ",t);
	if( !(type & SQ_STYPE) ){
		printf("Unsquelched\n");
		return;
	}
	for( i= 0, j= 0; sqc[i] != NULL; i++ ){
		if( type & sqi[i] ){
			if( j++ )
				printf(", ");
			printf("%s",sqc[i]);
		}
	}
	putchar('\n');
}


STATIC int oplist( mopt, all, xi, xp, xa )
int mopt;
int all;
int *xi, *xp;
char **xa;
{
	/*
	 *	List current options in mopt
	 *	
	 *	If all, list all options and indicate set by *
	 *	Returns number of options
	 *
	 */

	int i;
	
	for( i=0; xa[i] != NULL; i++ ){
		if( all )
			printf(" %c %d) ",((mopt & xi[i]) ? '*' : ' '),
				i+1 );
		else {
if( mopt != MYREC.opts && !(MYREC.opts & OP_VIP) && (mopt & OP_VIP)
	&& xi[i] == OP_SQNON )
		mopt |= OP_SQNON;
			if( xp[i] != 0 && !(MYREC.opts & xp[i]) )
				continue;
			if( !(mopt & xi[i]) )
				continue;
		}
		printf("%s\n",xa[i]);
	}
	return(i);
}


STATIC void sqit()
{
	/*
	 *	Squelch/Unsquelch
	 *
	 */

	char s[L_cbalias], t[L_cbalias];
	int i, j, go, type, sqnow, passok, absi;

	if( MYREC.opts & OP_SQLOCK ){
		printf("Cannot squelch\n");
		return;
	}
	if( (i= who("Who: ",s,t)) == S_NOBODY )
		return;
	if( (!(MYREC.opts & OP_PAID) && i < 0) || i < S_ALL ){
		printf("%s not active\n",s);
		return;
	}
	absi = ( i >= 0 ? i : 
			ulsize+
			(i - S_ALL) );
	sqnow = issq(slot, absi);
	if( i < 0 ) strcpy(t, gname(i));
	sqmsg(t, sqnow);
	passok = (MYREC.opts & OP_SYSOP ||
			(COPASSX && (MYREC.opts & OP_COSYSOP)));
	for( go= 1; go--; ){
		printf("P,C,R,A,U,?: ");
		*s = getchar();
		*s = toupper(*s);
		printf("%c\n",*s);
		switch( toupper(*s) ){
			case 'U':
				type = SQ_SRESET;
				break;
			case 'R':
				type = SQ_REVERSE;
				break;
			case 'C':
				type = SQ_COMP;
				break;
			case 'P':
				type = SQ_PRIV;
				break;
			case 'A':
				type = SQ_REVERSE|SQ_COMP|SQ_PRIV;
				break;
			case 'Z':
				if( passok ) type = SQ_PASSIVE;
				else go = 1;
				break;
			case '\n':
			case '\r':
				return;
			case '?':
			default:
				puts("P)rivate messages, C)omprehensive, R)everse, A)ll, U)nsquelch");
				if( passok )
					puts("Z) Passive");
				go = 1;
				break;
		}
	}
	if( type == SQ_SRESET ) sqnow = type;
	else
		sqnow |= type;
	if( DEBUG > 0 )
		printf("sqit: sqnow=0%o absi=%d slot=%d\n",sqnow,absi,slot);
	dosq(slot, absi, sqnow);
	sqnow = issq(slot, absi);
	if( DEBUG > 0 )
		printf("sqit: sqnow=0%o\n",sqnow);
	sqmsg(t, sqnow);
}


STATIC void boot( btype )
int btype;
{
	/*
	 *	Kill/Boot user
	 *
	 */

	int i, j, onev;
	char s[80], t[128];

	if( (i= who("Who: ",s,NULL)) == S_NOBODY )
		return;
	if( (!(MYREC.opts & OP_PAID) && i < 0) || i < S_ALL ||
		(!(MYREC.opts & OP_SYSOP) && !COKGLOB && i < 0) ){
			printf("%s is not active\n",s);
			return;
	}
	onev = ( (MYREC.opts & OP_SYSOP) ||
		(COONE && (MYREC.opts & OP_COSYSOP)) );
	if( !(MYREC.opts & OP_SYSOP) && onev && !(MYREC.opts & OP_ACTIVE)
		&& !COKINACT ){
			puts("Cosysops cannot kill while inactive");
			return;
		}
	if( i >= 0 && !(MYREC.opts & OP_SYSOP) && onev )
		if( (ulog[i].opts & OP_PAID) && !COKPAID ){
			puts("Cosysops cannot kill subscribers");
			return;
		}
	if( !(MYREC.opts & OP_SYSOP) && onev ){
		if( i >= 0 )
			sprintf( s, "%s/%s", ulog[i].userid, ulog[i].handle );
		sprintf(t,"%s/%s %s %s", ulog[slot].userid, ulog[slot].handle,
			( btype == SIGQUIT ? "boots" : "kills" ), s);
		xmsg( MSG_PPA, t, 0, "" );
		logpa( t );
	}
	if( i >= 0 )
		doboot( i, btype, onev );
	else
		for( j= 0; j < ulsize; j++ )
			if( gmatch( j, i ) ){
				doboot( j, btype, onev );
			}
	if( btype != SIGQUIT )
		if( !onev )
			printf("Vote cast\n");
}


STATIC void xstd( s, fn )
char *s, *fn;
{
	/*
	 *	Set format to one of the presets
	 *
	 */

	FILE *f;
	char *x, *y;
	int i, num;
	char line[256];

	*s = '?';

	if( s[1] != '?' ){
		num = atoi(&s[1]);
		if( num < 1 ){
			printf("%s out of range\n",&s[1]);
			return;
		}
	}

	if( (f= fopen(fn,"r")) == NULL ){
		printf("No built-in formats\n");
		return;
	}

	for( i= 1; fgets(line,255,f) != NULL; i++ ){
		x = strtok(line,";");
		y = strtok(NULL,"\n");
		if( s[1] == '?' )
			printf("@%.2d: %s\n",i,y);
		else
			if( i == num ){
				strcpy(s,x);
				fclose(f);
				return;
			}
	}
	if( s[1] != '?' )
		printf("%s is out of range\n",&s[1]);

	fclose(f);
	return;
}


STATIC void chlf()
{
	/*
	 *	Change linefeed character
	 *
	 */

	char s[2];

	if( !readdefault("New linefeed character: ",s,2,"") )
		return;
	MYREC.lfchar = *s;
}


STATIC void sendkill( who, btype )
int who;
int btype;
{
	/*
	 *	Send kill message to victim 
	 *
	 */

	struct cbmsg mbuf;

	mbuf.mtype = (long)ulog[who].pid;
	M_R.cbtype = CB_KILL;
	M_U.a.a2 = btype;
	M_R.mfrom = cbpid;
	if( msgsnd(mid,&mbuf,L_cbmsg,ipcflag) == -1 )
		sndfix(&mbuf);
}


STATIC int chchan( chan )
int chan;
{
	/*
	 *	Physically change channel
	 *
	 *	Returns non-zero on fail
	 *
	 */

	int i, j, ret;

	ret = 0;
	if( LIMCHAN == 0 )
		return( 0 );

	Pu( cbsem );
	if( chan > LIMBASE && chan < LIMBASE+LIMIT ){
		for( i=j= 0; i < ulsize; i++ )
			if( (ulog[i].opts & OP_ACTIVE) &&
				ulog[i].chan == chan )
					j++;
		if( j >= chan % LIMIT )
			ret = -1;
	}
	Vu( cbsem );

	if( ret == 0 )
		MYREC.chan = chan;

	return( ret );
}


STATIC void doboot( who, btype, onev )
int who;
int btype;
int onev;
{
	/*
	 *	Check to see if we should boot
	 *
	 *	If onev, only one vote is needed
	 */

	int i, j, k;

	if( onev ){
		sendkill( who, btype );
		return;
	} else
		if( ulog[who].opts & OP_SYSOP )
			return;

	if( !(ulog[slot].opts & OP_SYSOP) )
		if( ulog[who].opts & OP_PAID )
			return;

	dosq( who, slot, SQ_KILL );
	i = 0;

	for( j= 0; j < ulsize; j++ )
		if( issq( who, j ) & SQ_KILL )
			i++;
	for( j= 0; j < ulsize; j++ )
		if( isact(slot, j) && (ulog[j].opts & OP_PAID) )
			k++;

	if( ((float)i/(float)k)*100 >= KPERCENT && i >= 2 )
		sendkill( who, btype );
}


int gmatch( gslot, glob )
int gslot;
int glob;
{
	/*
	 *	Determines if a given slot matches a global
	 *	attribute
	 *
	 *	Returns non-zero on match
	 *
	 */

	if( !(ulog[gslot].opts & OP_USED) )
		return(0);
	if( !(MYREC.opts & OP_SYSOP) &&
		!(COACT && (MYREC.opts & OP_COSYSOP)) &&
		!(ulog[gslot].opts & OP_ACTIVE) )
			return(0);
	switch( glob ){
		case S_ALL:	return(-1);
		case S_PAID:	if( ulog[gslot].opts & OP_PAID )
					return(-1);
				break;
		case S_NOPAY:	if( !(ulog[gslot].opts & OP_PAID) )
					return(-1);
				break;
		case S_SYSOP:	if( ulog[gslot].opts & OP_SYSOP )
					return(-1);
		case S_COSYSOP:	if( ulog[gslot].opts & OP_COSYSOP )
					return(-1);
				break;
	}
	return(0);
}


STATIC char *gname( glob )
int glob;
{
	/*
	 *	Give a readable name for a globism
	 *
	 */

	/* The following assumes S_ALL is negative and the smallest
		valid global value */
	return( gnames[glob-(S_ALL)] );
}


#ifdef SKYNET
STATIC void tlist( tslot )
int tslot;
{
	/*
	 *	Give Skynet time info
	 *
	 */

	extern long time()
	extern long m_credits();
	long now;
	int m1, m2, s1, s2;

	now = time(0L);
	printf("%s",ctime(&now));
	if( !m_act(tslot) )
		return;
	m1 = m_on(tslot) / 60;
	m2 = m_left(tslot) / 60;
	s1 = m_on(tslot) % 60;
	s2 = m_left(tslot) % 60;
	s1 = ( s1 < 0 ? -s1 : s1 );
	s2 = ( s2 < 0 ? -s2 : s2 );
	printf("Time on: %.2d:%.2d  Time left: %.2d:%.2d\n",
			m1,s1,m2,s2);
	now = m_credits(tslot);
	if( now > 0 )
		printf("Credits remaining: %ld\n", now);
}


STATIC void vuser()
{
	/*
	 *	Add time for a Skynet User
	 *
	 */

	int i,j;
	char s[128];

	if( readdefault("User: ",s,L_cbalias,"") == 0 )
		return;
	if( (i= m_slot(s)) == -1 ){
		printf("%s not active\n",s);
		return;
	}
	printf("%s/%s\n",m_uname(i),m_alias(i));
	tlist(i);
	if( QUOTA != -1 && !(MYREC.opts & OP_SYSOP) )
		printf("%d minutes remaining in todays quota\n", MYREC.qremain);
	if( readdefault("Add: ",s,L_cbalias,"") == 0 )
		return;
	j = atoi(s);
	if( !(MYREC.opts & OP_SYSOP) && j < 0 && !CONEGV ){
		puts("Cannot subtract time");
		return;
	}
	if( ckquota( j ) )
		return;
	m_add(i,j*60);
	printf("%s/%s\n",m_uname(i),m_alias(i));
	tlist(i);
	if( !(MYREC.opts & OP_SYSOP) ){
		sprintf(s,"%s/%s %s %d min %s %s/%s", ulog[slot].userid,
			ulog[slot].handle, (j < 0 ? "subtracts" : "adds"),
			j, (j < 0 ? "from" : "to"), m_uname(i), m_alias(i));
		xmsg( MSG_PPA, s, 0, "" );
		logpa( s );
	}
}


STATIC int ckquota( add )
int add;
{
	/*	Make sure this /v is good for us

		Returns non-zero if bad
	*/

	if( MYREC.opts & OP_SYSOP )
		return( 0 );

	if( MYREC.qremain - add < 0 ){
		printf("Request would exceed todays remaining quota\n",
				MYREC.qremain);
		return( -1 );
	}
	MYREC.qremain -= add;
	return( 0 );
}
#endif

STATIC void talist()
{
	int i, j, k;
	char s[80];
	
	for( i= j= 0; i < ulsize; i++ )
		if( isact( slot, i ) )
			if( !strcmp( ulog[i].doing, DOTYPE ) ){
				if( !j++ )
					puts("Users currently typing:");
				printf(      "    %s/%s", ulog[i].userid,
					ulog[i].handle);
				if( (MYREC.opts & OP_SYSOP) &&
						!(MYREC.opts & OP_SQPILF) ){
					if( (k= ulog[i].towho) >= 0 )
						sprintf(s,"%s/%s",
							ulog[k].userid,
							ulog[k].handle);
					else if( k >= S_ALL )
						strcpy(s,gname(k));
					if( k >= S_ALL )
						printf("\t(%s)", s);
				}
				putchar('\n');
			}
	if( !j )
		puts("Nobody currently typing");
}


#define COP	(cmdops[temp->index])

STATIC void helper()
{
	/*	Print a description of pertinent commands
	*/

	struct cmdnode *temp;
	char s[80];
	int cnt, pos;

	printf("All commands are proceeded by a / (slash)\n\n");
	cnt = pos = 0;
	for( temp= cmdlist; temp != CNNULL; temp= temp->next )
		if( !COP || (MYREC.opts & COP) )
			if( *(temp->desc) ){
				sprintf(s, "%c  %s", *(temp->acts), temp->desc);
				if( pos > 37 || cnt == 2 ||
						pos+strlen(s) > 79 ){
					cnt = pos = 0;
					putchar('\n');
				} else if( cnt == 1 )
					printf("   ");
				printf("%-37s", s);
				pos += max(strlen(s), 37);
				cnt++;
			}
	putchar('\n');
	if( cnt == 1 ) putchar('\n');
}

STATIC int ckp2me()
{
	/*	Returns non-zero if somebody is currently sending a
		private message to YOU
	*/

	int i;

	for( i= 0; i < ulsize; i++ )
		if( isact( slot, i ) )
			if( ulog[i].towho == slot )
				return( -1 );
	return( 0 );
}


void ungag()
{
	/*	Set read-only override for a user
	*/

	int i;
	char s[80];

	if( (i= who("Who: ",s,NULL)) == S_NOBODY )
		return;
	if( i < 0 ){
		printf("%s not active\n",s);
		return;
	}
	ulog[i].opts |= OP_RDOVER;
	if( !(MYREC.opts & OP_SYSOP) ){
		sprintf(s,"%s/%s ungags %s/%s", MYREC.userid,
			MYREC.handle, ulog[i].userid, ulog[i].handle);
		xmsg( MSG_PPA, s, 0, "" );
		logpa( s );
	}
	printf("%s/%s ungagged\n", ulog[i].userid, ulog[i].handle);
}


STATIC void dummy( duh )
int duh;
{
	signal( duh, SIG_IGN );
	return;
}
