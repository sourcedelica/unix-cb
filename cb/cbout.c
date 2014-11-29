#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <errno.h>
#include "cb.h"
#include "cbetc.h"
#include "cbcfg.h"

/*
 *	Unix-CB
 *
 *	Incoming message handler
 *	(Parent)
 *
 *	Copyright (C) 1988 CPEX Development
 *	All Rights Reserved.
 *	Author: Eric Pederson
 *
 */

extern void addit();
extern void printem();
extern void printit();
extern char *xlat();

static int work = 0;
static int busyin = 0;
static struct cbmsg mbuf;

#define SQLF	0001
#define SQBEEP	0002

struct dlist {
	struct dmsg d;
	struct dlist *next;
};

#define DLNULL (struct dlist *)0

static struct dlist *first = DLNULL;
static struct dlist *last = DLNULL;

extern char *malloc();
extern char *strtok2();

/**********************************************************************/

void cbout()
{
	struct dmsg dm;
	static int myout = 0, maxout;

	close( out );
	if( (out= open(cbfn(OUTFILE),O_RDWR)) == -1 ){
		perror("Cannot open out file");
		return;
	}
	for(;;){
		if( !work ){
			if( msgrcv(mid,&mbuf,L_cbmsg,cbpid,0) == -1 ){
				rcvfix();
			}
		} else
			work = 0;

		switch( M_R.cbtype ){
			case CB_LOGOFF:
				done = SIGFPE;
				/* FALLS THROUGH */
			case CB_LOGOUT:
				return;
			case CB_KILL:
				done = M_U.a.a2;
				printf("Adios, pal\n");
				return;
			case CB_NOIN:
				busyin = 0;
				break;
			case CB_INREQ:
				busyin = 1;
				mbuf.mtype = M_R.mfrom;
				if( msgsnd(mid,&mbuf,L_cbreq,ipcflag) == -1 )
					sndfix(&mbuf);
				break;
			case CB_NEWMSG:
				lseek(out,
					(long)(M_R.arg*sizeof(struct dmsg)),0);
				read(out,&dm,sizeof(struct dmsg));
				if( dm.chan == MSG_PRIV && dm.tslot != slot
					&& dm.fslot == slot && dm.tslot >= 0 )
						break;
				if( dm.chan == MSG_PRIV && dm.tslot == slot )
					MYREC.rlastp = dm.fslot;
				if( dm.chan < 100 && dm.fslot == slot ){
					myout++;
					maxout = (MYREC.opts & OP_PAID) ? 
						MAXPOUT : MAXMYOUT;
					maxout = (MYREC.opts & OP_UGLYLINK) ?
						MAXUOUT : maxout;
					if( maxout != -1 )
						if( myout == maxout ){
							done = SIGUSR1;
							return;
						}	
				} else
					myout = 0;
				if( MYREC.dcreq == -1 )
					addit(&dm);
				break;
		}

		if( !busyin )
			printem();
	}
}

STATIC void addit( pdm )
struct dmsg *pdm;
{
	/*
	 *	Add a message from disk to the list
	 *
	 */

	struct dlist *temp;
	char *malloc();

	temp = (struct dlist *)malloc(sizeof(struct dlist));
	memcpy(&(temp->d),pdm,sizeof(struct dmsg));
	temp->next = DLNULL;
	if( last != DLNULL )
		last->next = temp;
	else 
		first = temp;
	last = temp;
}


STATIC void printem()
{
	/*
	 *	Print out pending messages
	 *	Allow interruption of printout by new messages
	 *
	 */

	struct dlist *temp;

	for( temp= first; temp != DLNULL; temp= first ){
		printone(&temp->d);
		first = temp->next;
		free( temp );
		if( first == DLNULL )
			last = first;
		if( msgrcv(mid,&mbuf,L_cbmsg,cbpid,IPC_NOWAIT) != -1 ){
			work = 1;
			return;
		} else
			if( errno != ENOMSG )
				rcvfix();
	}
}


/* Note printone is not STATIC - prototype is in cbetc.h but code is here */

void printone( temp )
struct dmsg *temp;
{
	/*
	 *	Print one message via myformat
	 *
	 */

	char *x, s[400], ds[FORMAX], *y;
	int pop, pit, i, j;
	char pchar;

	memset(s,0,400);
	if( temp->chan == MSG_PA || temp->chan == MSG_PPA
			|| temp->chan == MSG_CPA || temp->chan == MSG_NCPA ){
		if( MYREC.opts & OP_ANSI )
			printf( "%s", ANSPA );
		strcat(s,"-- ");
		if( temp->chan == MSG_PPA )
			strcat(s,"*");
		if( temp->chan == MSG_PA && (MYREC.opts & OP_PABEEP) )
			strcat(s,"\007");
		strcat(s,temp->msg);
		printit(s);
		if( MYREC.opts & OP_ANSI ){
			printf( "%s", ANSREG );
			fflush( stdout );
		}
		return;
	}
	if( ulog[temp->fslot].opts & OP_UGLYLINK ){
		if( temp->fslot == slot )
			return;
		sprintf(s, "#%d %s", temp->fslot, temp->msg );
		strsqz( s, " " );
		printit( s );
		fflush( stdout );
		return;
	}
	if( temp->chan == MSG_RPA ){
		if( MYREC.opts & OP_ANSI )
			printf( "%s", ANSPA );
		strcat( s, temp->msg );
		printit( s );
		if( MYREC.opts & OP_ANSI ){
			printf( "%s", ANSREG );
			fflush( stdout );
		}
		return;
	}
	pit = ( (MYREC.opts & OP_VIP) && temp->chan == MSG_PRIV &&
			(temp->tslot != slot || MYREC.pid != cbpid)
			&& (temp->tslot >= 0) );
	if( temp->chan == MSG_VIP && strcmp(temp->ufrom,MYREC.userid) )
		return;

	pchar = '*';
	if( MYREC.opts & OP_ANSI )
		printf( "%s", ansstr( temp->fslot ) );
	for( pop= 0, x= MYREC.format; *x != 0; x++ ){
		if( *x != '%' )
			s[strlen(s)] = *x;
		else
			switch( *(++x) ){
				case '1':
					pop |= SQBEEP;
					break;
				case '2':
					pop |= SQLF;
					break;
				case 'm':
					strcat(s,xlat(temp->msg,pop));
					break;
				case 'u':	
					strcat(s,temp->ufrom);
					break;
				case 'U':	
					sprintf(ds,"%*s",L_cuserid,temp->ufrom);
					strcat(s,ds);
					break;
				case 'h':	
					strcat(s,temp->from);
					if( pit ){
						strcat(s,"->");
						strcat(s,temp->to);
					}
					break;
				case 'e':
					strcat( s, "\033" );
					break;
				case 's':
					sprintf(ds,"%d",temp->fslot);
					strcat(s,ds);
					break;
				case 'c':	
					if( temp->chan > MAXCHAN )
						if( pit )
							sprintf(ds,">");
						else sprintf(ds,"%c",pchar);
					else sprintf(ds,"%d",temp->chan);
					strcat(s,ds);
					break;
				case 'S':
					sprintf(ds,"%.2d",temp->fslot);
					strcat(s,ds);
					break;
				case 'C':
					if( temp->chan == MSG_PRIV ) 
						if( pit )
							sprintf(ds,"*>");
						else sprintf(ds,"%c%c",pchar,pchar);
					else if( temp->chan == MSG_STATION )
						sprintf(ds,"%%%%");
					else if( temp->chan == MSG_VIP )
						sprintf(ds,"*!");
					else sprintf(ds,"%.2d",temp->chan);
					strcat(s,ds);
					break;
				case '$':	
					if( temp->opts & OP_SYSOP ){
						strcat(s,"@");
						break;
					} else
					if( temp->opts & OP_COSYSOP ){
						strcat(s,"%");
						break;
					} else
					if( temp->opts & OP_PAID )
						strcat(s,"$");
					else strcat(s," ");
					break;
				case '[':
					if( temp->opts & OP_SYSOP ){
						strcat(s,"<");
						break;
					} else
					if( temp->opts & OP_COSYSOP ){
						strcat(s,"{");
						break;
					} else
					if( temp->opts & OP_PAID )
						strcat(s,"[");
					else strcat(s,"(");
					break;
				case ']':
					if( temp->opts & OP_SYSOP ){
						strcat(s,">");
						break;
					} else
					if( temp->opts & OP_COSYSOP ){
						strcat(s,"}");
						break;
					} else
					if( temp->opts & OP_PAID )
						strcat(s,"]");
					else strcat(s,")");
					break;
				case '#':	
					if( temp->opts & OP_SYSOP ){
						strcat(s,"@");
						break;
					} else
					if( temp->opts & OP_COSYSOP ){
						strcat(s,"%");
						break;
					} else
					if( temp->opts & OP_PAID )
						strcat(s,"$");
					else strcat(s,"#");
					break;
				case '*':	
					if( temp->chan == MSG_PRIV )
						if( pit )
							strcat(s,">");
						else {
							sprintf(ds,"%c",pchar);
							strcat(s,ds);
						}
					else if( temp-> chan == MSG_STATION )
						strcat(s,"%");
					else if( temp->chan == MSG_VIP )
						strcat(s,"!");
					break;
				case 'P':	
					if( temp->chan == MSG_PRIV )
						strcat(s,"P");
					else if( temp->chan == MSG_STATION )
						strcat(s,"S");
					else if( temp->chan == MSG_VIP )
						strcat(s,"V");
					break;
				case 'p':
					if( temp->chan == MSG_PRIV ){
						sprintf(ds,"%c",*(++x));
						strcat(s,ds);
					} else
						++x;
					break;
				case '\\':
					strcpy( ds, ++x );
					if( (y= strchr( ds, '\\' )) == NULL )
						break;
					*y = 0;
					x += strlen(ds);
					if( temp->chan == MSG_PRIV )
						strcat(s,ds);
					break;
				case '_':
					strcat(s,"\n");
					break;
				case '?':
					if( temp->chan != MSG_PRIV )
						break;
					if( temp->tslot == S_ALL )
						strcat(s,"*");
					if( temp->tslot == S_SYSOP )
						strcat(s,"@");
					if( temp->tslot == S_COSYSOP )
						strcat(s,"%");
					if( temp->tslot == S_PAID )
						strcat(s,"$");
					if( temp->tslot == S_NOPAY )
						strcat(s,"#");
					break;
				default:	
					s[strlen(s)] = *x;
					break;
			}
	}
	printit(s);
	return;
}

STATIC void printit(s)
char *s;
{
	/*
	 *	Seriously, print it.  Wrap it, etc.
	 *
	 */

	int i;
	char *x;

	i = 0;
	for( x= strtok2(s," "); x != NULL; ){
		if( i + strlen(x) > wid - 1 ){
			if( i < wid - 15 )
				for( ;i < wid - 1 && *x != 0; x++, i++ )
					putchar(*x);
			printf("\n ");
			i = 1;
		}
		printf("%s",x);
		if( x[strlen(x)-1] == '\n' ) i = 0;
		else if( strlen(x) == 0 ) i++;
		else
			i += strlen(x);
		x = strtok2(NULL," ");
		if( x != NULL && i < wid - 1 ){
			putchar(' ');
			i++;
		}
	}
	if( MYREC.opts & OP_ANSI ){
		printf( "%s", ANSREG );
		fflush( stdout );
	}
	putchar('\n');
}


STATIC char *xlat( msg, pop )
char *msg;
int pop;
{
	/*
	 *	Translate for output
	 *
	 *	Returns a pointer to a static area in memory
	 *	of the translated string
	 */

	static char s[300];
	char *x;
	int beeps;

	beeps = 0;
	memset(s,0,300);
	for(x= msg; *x != 0; x++ )
		switch(*x){
			case '\n':
				if( (pop & SQLF) || (MYREC.opts & OP_SQLF) )
					s[strlen(s)] = MYREC.lfchar;
				else strcat(s,"\n ");
				break;
			case 007:
				if((pop & SQBEEP) || (MYREC.opts & OP_SQBEEP))
					break;
				if( beeps > 0 )
					break;
				beeps++;
				/* FALLS THROUGH */
			default:
				s[strlen(s)] = *x;
		}

	return(s);
}
