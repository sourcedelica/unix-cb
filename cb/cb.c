#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include <string.h>
#include <setjmp.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <termio.h>
#include <signal.h>
#include <fcntl.h>
#include "../include/exitcodes.h"
#include "../include/mflags.h"
#include "../include/mstf.h"
#include "../include/muserinf.h"
#define CCB
#include "cb.h"
#include "cbetc.h"
#include "cbcfg.h"

/*
 *	Unix-CB
 *
 *	Copyright (C) 1988 CPEX Development
 *	All Rights Reserved
 *	Author: Eric Pederson
 *
 */

extern void cbinp();
extern void cbout();
extern void catchit();
extern void exclean();
extern void infinish();
extern void logout();
extern void logclean();
extern void loadops();

int slot = -1;
int mid;
int opts;
long cbpid;
int out;
int wid;
int ulsize;
int done = 0;
char *myformat;
struct ulrec *ulog;
int cbsem;
int *pnext, *pcbflag, *pfixslot;
struct sqrec *sqlog;
int child = 0;
int ipcflag = IPC_NOWAIT;
int sendflag = 0;
int fixsid;
struct termio oldterm;
int skynet = 0;
char *cbdoing;
char *cbjs, *cbjt;
int cbjl;
int logging = 1;

static struct cbmsg mbuf;
static jmp_buf env;

/* These are the "sticky" flag bits that stay with you after you log off */

static char *opta[] = {
	"CHLOCK", "SQLOCK", "LFLOCK", "PMLOCK", "SQBEEP", "SQLF",
	"MONIT", "ANSI", "SQSTAT", "PABEEP", "UGLYLINK", NULL };
static int opto[] = {
	OP_CHLOCK, OP_SQLOCK, OP_LFLOCK, OP_PMLOCK, OP_SQBEEP, OP_SQLF,
	OP_MONIT, OP_ANSI, OP_SQSTAT, OP_PABEEP, OP_UGLYLINK, 0 };

extern char *malloc(), *getenv(), *shmat(), *m_doing();

/***********************************************************************/

main(argc, argv)
int argc;
char **argv;
{
	char s[80], *x, *sbase, *cfname = NULL;
	int i, j, sid, errflg = 0, bact;
	long sflags, m_flags();
	void acatch();
	extern char *optarg;
	extern int optind;

	muserinf();
	while( (sid= getopt(argc, argv, "y:")) != -1 )
		switch( sid ){
			case 'y':
				cfname = optarg;
				break;
			case '?':
				errflg++;
				break;
		}
	if( optind < argc ) errflg++;
	if( errflg ){
		printf("Usage: %s [-y cfname]\n",argv[0]);
		exit(EXITOK);
	}
	if( cbcfg(cfname) ){
		printf("Error reading configuration file %s\n",
			cfname == NULL ? CBCONFIG : cfname );
		exit(EXITOK);
	}
	ulsize = NUMSLOTS;

	if( (mid= msgget(CBKEY,0)) == -1 ){
		if (errno == ENOENT){
			system(CBINIT);
			if( (mid= msgget(CBKEY,0)) == -1 ){
				perror("CB is unavailable");
				exit(EXITOK);
			}
		} else {
			perror("CB is unavailable");
			exit(EXITOK);
		}
	}
	if( (out= open(cbfn(OUTFILE),O_RDWR|O_CREAT,CBMODE)) == -1 ){
		perror("Cannot open out file");
		exit(EXITOK);
	}
	if( (sid= shmget(CBKEY,0,0)) == -1 ){
		perror("Attaching shared memory");
		exit(EXITOK);
	}
	i = 0;
	sbase = shmat( sid, NULL, 0 );
	pnext = (int *)sbase;
/*
	ulsize = *pnext;
	i += sizeof(int);
*/
	pnext = (int *)(sbase + i);
	i += sizeof(int);
	pcbflag = (int *)(sbase + i);
	i += sizeof(int);
	pfixslot = (int *)(sbase + i);
	i += sizeof(int);
	ulog = (struct ulrec *)(sbase + i);
	i += sizeof(struct ulrec)*ulsize;
	sqlog = (struct sqrec *)(sbase + i);

	if( (cbsem= semget(CBKEY,0,0)) == -1 ){
		perror("semget cbsem");
		exit(EXITOK);
	}

#ifdef SKYNET
	/* Attach to skynet menu shared memory */
	m_at();
#endif

	if( setjmp(env) )
		exclean();
	signal(SIGHUP,catchit);
	signal(SIGTERM,catchit);
	cbpid = (long)getpid();

	if( (x= getenv("SLOTNUM")) != NULL ){
		slot = atoi(x);
		skynet++;
	}
	if( (slot= login(slot)) == -1 ){
		printf("CB is full\n");
		exit(EXITOK);
	}
	/* When finished logging in, logging is reset */
	logging = 0;

	if( MYREC.opts & OP_ANSI )
		printf( "%s", ANSREG );
	printf("%s\n", cfgxstr(CBLOGIN));
	printf("Use /? for help\n\n");

	/* Update options based on environ */
	if( (x= getenv("MAXLLEN")) != NULL ) wid = atoi(x);
	else wid = 80;
#ifdef SKYNET
	if( skynet ){
		cbdoing = strdup(m_doing( slot ));
		MYREC.opts |= OP_SKYNET;
	}
#endif
	loadops( slot );
	saveterm(&oldterm);
	raw();
	signal(SIGQUIT,SIG_IGN);
	signal(SIGINT,SIG_IGN);

	if( !( (MYREC.opts & OP_SYSOP) ||
			(COLOCKOV && (MYREC.opts & OP_COSYSOP)) ) )
		if( *pcbflag & CF_LOCKED ){
			printf("%s\n",LOCKEDMSG);
			logout();
			restterm(&oldterm);
			if( STTY0 ) stty0();
			exit(EXITOK);
		}
	if( MYREC.opts & OP_SYSOP || (COACT && (MYREC.opts & OP_COSYSOP)) ){
		printf("Be active? ");
		i = getchar();
		if( (i= toupper(i)) == 'Q'){
			puts("Quit");
			logout();
			restterm(&oldterm);
			exit(EXITOK);
		}
		i = ( i == 'Y' );
		puts( i ? "Yes" : "No" );
	} else
		i = 1;
	bact = i;
		
	MYREC.chan = ( MYREC.opts & OP_PAID ? PINITCHAN : INITCHAN );
	if( MYREC.opts & OP_UGLYLINK ) MYREC.chan = UINITCHAN;
	if( AFORCEL || ( !(MYREC.opts & OP_PAID) && FORCEL ) )
		MYREC.opts |= OP_LIST;
	if( AFORCEH || ( !(MYREC.opts & OP_PAID) && FORCEH ) )
		strcpy( MYREC.handle, galias );

	/* Log in handle */
	i = ( *MYREC.handle == 0 );
	for( j= 1; j; i = 1 ){
		if( i ){
			if( !readdefault("Handle to use: ",s,L_cbalias,"") ){
				logout();
				while( msgrcv(mid,&mbuf,L_cbmsg,cbpid,
					IPC_NOWAIT) != -1 )
					;
				if( STTY0 ) stty0();
				restterm(&oldterm);
				exit(EXITOK);
			}
		} else
			strcpy(s,MYREC.handle);
		if( !isval(s) ){
			printf("Invalid character in handle\n");
			continue;
		}
		j = chhand(s);
	}
	/* Fix anyone with your handle */
	fixdorks();

	while( msgrcv(mid, &mbuf, L_cbmsg, cbpid, IPC_NOWAIT) != -1 )
		;
	if( DAILY != NULL )
		listsig( DAILY );
	if( bact )
		MYREC.opts |= OP_ACTIVE;
	else
#ifdef SKYNET
		if( skynet )
			m_chdoing( slot, FAKEOUT );
#endif
	alist();
	msglog(slot,CB_ACTIVATE);

	if( child= fork() ){
		MYREC.cpid = child;
		cbout();
	} else
		cbinp();

	signal(SIGHUP,SIG_IGN);
	signal(SIGTERM,SIG_IGN);
	exclean();
}


STATIC void catchit( arg )
int arg;
{
	/*
	 *	Handle hangups and terminates
	 *
	 */

	signal(SIGHUP,SIG_IGN);
	signal(SIGTERM,SIG_IGN);

	done = arg;
	longjmp(env,-1);
}


STATIC void exclean()
{
	/*
	 *	Cleanup and exit CB
	 *
	 */

	kill(child,SIGHUP);
	while( msgrcv(mid, &mbuf, L_cbmsg, cbpid, IPC_NOWAIT) != -1 )
		;
	while( msgrcv(mid, &mbuf, L_cbmsg, (long)child, IPC_NOWAIT) != -1 )
		;
#ifndef SILENT
	/* Send logoff message */
	if( !sendflag )
		msglog(slot,CB_DEACT);
#endif
	MYREC.opts &= ~OP_ACTIVE;
	logout();
	if( done == SIGTERM )
		puts("\n-- Time expired\n");
	else if( done == SIGQUIT )
		done = 0;
	while( msgrcv(mid, &mbuf, L_cbmsg, (long)child, IPC_NOWAIT) != -1 )
		;
	while( msgrcv(mid, &mbuf, L_cbmsg, cbpid, IPC_NOWAIT) != -1 )
		;
	if( STTY0 )
		stty0();
	restterm(&oldterm);
	exit( done != 0 ? EXITKILL : EXITOK );
}


STATIC void acatch( arg )
int arg;
{
	/*
	 *	We get here if the server times out initially
	 */

	puts("Server not responding, CB is down");
	exit(EXITOK);
}


STATIC int login( where )
int where;
{
	/*
	 *	Adds user's CB record to the incore userlog
	 *
	 *	Input:  where designates the slot the requestor wants.
	 *		If where is -1, login will assign one
	 *
	 *	Output:	Passes back the assigned slot
	 *		
	 */

	int i;
	struct ulrec temp;

	if( rdrec(&temp) )
		return( -1 );
	
	Pu( cbsem );

	/* Is slot requested available? */
	if( where >= 0 && where < ulsize )
		if( !(ulog[where].opts & OP_USED) ){
			infinish(where,&temp);
			Vu( cbsem );
			return( where );
		}

	/* Can we find a slot in what's already allocated? */
	for( i= ulsize-1; i > URESSIZ-1; i-- ){
		if( !(ulog[i].opts & OP_USED) ){
			infinish(i,&temp);
			Vu( cbsem );
			return( i );
		}
	}

	/* No can do. */
	Vu( cbsem );
	return( -1 );
}


#define R_HANDLE	"HANDLE"
#define R_FORMAT	"FORMAT"
#define R_LFCHAR	"LFCHAR"
#define R_AFORMAT	"AFORMAT"
#define R_LASTP		"LASTP"
#define R_LASTRP	"LASTRP"

STATIC int rdrec( ptemp )
struct ulrec *ptemp;
{
	/*
	 *	Reads user record or loads default if not found
	 *
	 *	Returns non-zero on fail
	 *
	 */

	FILE *f;
	char s[80], *x;
	int i, nodice;

	if( HOMEREC ){
		strcpy(s,homeof(guserid));
		strcat(s,PATHSEP);
		strcat(s,RECNAME);
	} else
		strcpy(s, cbrfn( guserid ));
	
	nodice = 0;
	if( (f= fopen(s,"r")) == NULL )
		nodice++;

	ptemp->sqs = SQNULL;
	ptemp->kvotes = 0;
	ptemp->chan = 1;
	ptemp->opts = OP_SQNON | OP_SQPILF;
	ptemp->lastp = ptemp->rlastp = S_NOBODY;
	strcpy(ptemp->userid,guserid);
	ptemp->pid = cbpid;
	ptemp->lfchar = *LFCHAR;
	*ptemp->handle = 0;
	*ptemp->doing = 0;
	strcpy( ptemp->format, FORDEF);
	strcpy( ptemp->aformat, AFORDEF );
	ptemp->dcreq = -1;
	ptemp->towho = S_NOBODY;
	ptemp->qremain = rdquota();

	if( nodice )
		return( 0 );

	while( fgets(s,80,f) != NULL )
		if( strtok(s,"=\n") != NULL )
			if( !strcmp(s,R_HANDLE) ){
				x = strtok(NULL,"\n");
				if( x != NULL && *x != 0 )
					strcpy(ptemp->handle,x);
			} else if( !strcmp(s,R_LFCHAR) )
				ptemp->lfchar = *strtok(NULL,"\n");
			else if( !strcmp(s,R_FORMAT) ){
				x = strtok(NULL,"\n");
				memset(ptemp->format,0,FORMAX);
				strncpy(ptemp->format,x,FORMAX-1);
			} else if( !strcmp(s,R_AFORMAT) ){
				x = strtok(NULL,"\n");
				memset(ptemp->aformat,0,FORMAX);
				strncpy(ptemp->aformat,x,FORMAX-1);
			} else if( !strcmp( s, R_LASTP ) ){
				x = strtok( NULL, "\n" );
				ptemp->lastp = lookup( x, NULL );
			} else if( !strcmp( s, R_LASTRP ) ){
				x = strtok( NULL, "\n" );
				ptemp->rlastp = lookup( x, NULL );
			} else
				for( i= 0; opta[i] != NULL; i++ )
					if( !strcmp(opta[i],s) )
						ptemp->opts |= opto[i];

	fclose(f);
	return(0);	
}


STATIC int rdquota()
{
	/*	Read quota file to get current time quota for /v
	*/

	FILE *f;
	char s[80];

	if( (f= fopen(cbqfn( guserid ), "r")) == NULL )
		return( QUOTA );

	fgets(s, 80, f);
	fclose( f );
	return( atoi( s ) );
}


STATIC void wrrec( slot )
int slot;
{
	/*
	 *	Writes user record file
	 *
	 */

	FILE *f;
	char s[80];
	int i, um;

	if( slot < 0 || slot >= ulsize )
		return;
	if( HOMEREC ){
		strcpy(s,homeof(ulog[slot].userid));
		strcat(s,PATHSEP);
		strcat(s,RECNAME);
	} else
		strcpy(s, cbrfn( ulog[slot].userid ));
	
	um = umask( CBUMASK );
	if( (f= fopen(s,"w+")) == NULL ){
		umask( um );
		return;
	}
	fprintf(f,"%s=%s\n",R_HANDLE,ulog[slot].handle);
	fprintf(f,"%s=%s\n",R_FORMAT,ulog[slot].format);
	fprintf(f,"%s=%s\n",R_AFORMAT,ulog[slot].aformat);
	fprintf(f,"%s=%c\n",R_LFCHAR,ulog[slot].lfchar);
	if( ulog[slot].lastp != S_NOBODY )
		fprintf(f, "%s=%s\n", R_LASTP, ulog[ulog[slot].lastp].userid);
	if( ulog[slot].rlastp != S_NOBODY )
		fprintf(f, "%s=%s\n", R_LASTRP, ulog[ulog[slot].rlastp].userid);
	for( i=0; opta[i] != NULL; i++ )
		if( ulog[slot].opts & opto[i] )
			fprintf(f,"%s\n",opta[i]);
	fclose(f);
	wrquota();
	umask( um );
}	


STATIC wrquota()
{
	/*	Write quota file from qremain if needed
	*/

	FILE *f;

	if( !(MYREC.opts & OP_COSYSOP) )
		return;

	if( (f= fopen(cbqfn( guserid ), "w+")) == NULL ){
		printf("Error creating quota file\n");
		perror( cbqfn( guserid ) );
		return;
	}

	fprintf(f, "%d\n", MYREC.qremain);
	fclose( f );
	return;
}


STATIC void infinish( where, ptemp )
int where;
struct ulrec *ptemp;
{
	/*
	 *	Load user rec into user log
	 *
	 *	(void)
	 *
	 */

	int i;

	if( strcmp(ulog[where].userid,ptemp->userid) ){
		/* I wasn't the last person on this slot */
		logclean(where);
	}
	memcpy( ulog+where, ptemp, sizeof(struct ulrec) );
#ifndef SILENT
	ulog[where].opts |= OP_USED;
#endif
	return;
}


STATIC void logout()
{
	/*
	 *	Log user out and mark space as free
	 *	Write out record
	 *	Clean pending messages
	 *
	 */

	int i;
	struct cbmsg mtemp;

	ulog[slot].opts &= ~OP_USED;
	while( msgrcv(mid,&mtemp,L_cbmsg,
			(long)ulog[slot].pid,IPC_NOWAIT) != -1 )
		;
	wrrec(slot);
}



STATIC void logclean( slot )
int slot;
{
	/*	Update other's squelching, etc. on new activation

		This should be called only when a new userid is logging
		into this slot
	 */

	int i;

	if( slot < 0 || slot >= ulsize )
		return;
	for( i= 0; i < ulsize; i++ ){
		if( ulog[i].lastp == slot )
			ulog[i].lastp == S_NOBODY;
		dosq( i, slot, SQ_SRESET|SQ_KRESET );
	}
	for( i= 0; i < ulsize+S_NUMGLOB; i++ )
		dosq( slot, i, SQ_SRESET|SQ_KRESET );
}

/*static*/ void loadops( slot )
int slot;
{
	/*	Loads the "opts" flag word with the correct
		options.  If on skynet, checks incore flags,
		otherwise checks environment vars
	*/
		
	int flags, sflags;

#ifdef SKYNET
	if( skynet ){
		flags = m_flags( slot );
		sflags = m_stf( slot );
		if( sflags & STF_PAID )
			MYREC.opts |= OP_PAID;
		if( flags & F_SYSOP )
			MYREC.opts |= OP_SYSOP;
		if( flags & F_COSYSOP )
			MYREC.opts |= OP_COSYSOP;
		if( flags & F_VALIDATED )
			MYREC.opts |= OP_VALID;
	}
#endif
	if( ENVSYSOP != NULL )
		if( getenv( ENVSYSOP ) != NULL )
			MYREC.opts |= OP_SYSOP;
	if( ENVPAID != NULL )
		if( getenv( ENVPAID ) != NULL )
			MYREC.opts |= OP_PAID;
	if( ENVVALID != NULL )
		if( getenv( ENVVALID ) != NULL )
			MYREC.opts |= OP_VALID;
	if( ENVVIP != NULL )
		if( getenv( ENVVIP ) != NULL )
			MYREC.opts |= OP_VIP;
	if( ENVCOSYSOP != NULL )
		if( getenv( ENVCOSYSOP ) != NULL )
			MYREC.opts |= OP_COSYSOP;
}

/*static*/ stty0()
{
	/*	Forces a hangup by setting the terminal's baud rate to
		zero.  Implemented as a kludge to workaround
		a flaky serial driver that refused to honor HUPCL.
	
		Use only if running CB standalone (as login shell, etc)
		VIP users are not effected
	*/

	if( MYREC.opts & OP_VIP )
		return( 0 );
	oldterm.c_cflag &= ~CBAUD;
	restterm(&oldterm);
	/* Most likely never makes it to here */
	return( 0 );
}

fixdorks()
{
	int i;

	for( i= 0; i < ulsize; i++ )
		if( isact( slot, i ) )
			if( !stricmp( ulog[i].handle, guserid ) )
				strcpy( ulog[i].handle, ulog[i].userid );
}
