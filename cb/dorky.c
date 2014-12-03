#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <errno.h>
#include "osdefs.h"
#include "cb.h"
#include "cbetc.h"
#include "cbcfg.h"

/*	Skynet Unix CB
 *
 *	Private Chat Module
 *
 *	Copyright (C) 1989 CPEX Development
 *	All rights reserved.
 *	Author: Eric Pedeson
 */

extern char *m_device();
extern void dinp(), dccatch();

static FILE *f;
static int dcfd;

void dorky()
{
	char s[256], *tty, fn[80];
	int i, j, idc, oum;
	long tdum;

	if( (i= who("Who: ",s,NULL)) == S_NOBODY )
		return;
	else if( i < 0 ){
		printf("%s not active\n",s);
		return;
	}
	if( i == slot ){
		printf( "You may not private chat with yourself\n" );
		return;
	}
	#ifdef SKYNET
	if( !(ulog[i].opts & OP_SKYNET) ){
		printf("%s not on Skynet\n",s);
		return;
	}
	#endif
	if( checksq( i, slot ) & SQ_BOTH ){
		printf("Squelched by %s\n",s);
		return;
	}
	#ifdef SKYNET
	tty = m_device( i );
	#else
	tty = ulog[i].ttyname;
	#endif
	if( (f= fopen( tty, "a" )) == NULL ){
		printf("Cannot private chat with %s\n",s);
		return;
	}
	if( strcmp( MYREC.userid, ulog[i].userid ) < 0 )
		sprintf( fn, "%s/%s/.dc.%s.%s", basepath(),
			DCPDIR, MYREC.userid, ulog[i].userid );
	else
		sprintf( fn, "%s/%s/.dc.%s.%s", basepath(),
			DCPDIR, ulog[i].userid, MYREC.userid );
	oum = umask( 0 );
	dcfd = open( fn, O_WRONLY|O_APPEND|O_CREAT, 0666 );
	umask( oum );
	time( &tdum );
	sprintf( s, "%s: %s", MYREC.userid, ctime( &tdum ) );
	if( dcfd != -1 )
		write( dcfd, s, strlen(s) );
	else
		printf("errno= %d\n",errno);

/* BEGIN MUTEX */
	Pu( cbsem );
	MYREC.dcreq = i;
	idc = ulog[i].dcreq;
	j = (idc == slot);
	Vu( cbsem );
/* END MUTEX */

	if( j ){
		fprintf( f, "Entering private chat with %s/%s\n",
			MYREC.userid, MYREC.handle );
		printf( "Entering private chat with %s/%s\n",
			ulog[i].userid, ulog[i].handle );
	} else {

		if( PAIDDC && !(MYREC.opts & OP_PAID) ){
			printf("You must be a member to initiate a private chat request\n");
			MYREC.dcreq = -1;
			if( dcfd != -1 )
				close( dcfd );
			return;
		}

		if( idc != -1 ){
			printf("%s/%s is already in private chat with someone else\n",ulog[i].userid, ulog[i].handle );
			MYREC.dcreq = -1;
			if( dcfd != -1 )
				close( dcfd );
			return;
		}
		sprintf( s, "## %s/%s is requesting a private chat",
			MYREC.userid, MYREC.handle );
		xmsg( MSG_RPA, s, i, "" );
		xmsg( MSG_RPA, "## Use /r to respond", i, "" );
		printf("%s/%s has been notified of your private chat request\n",
			ulog[i].userid, ulog[i].handle );
	}

	strcpy( MYREC.doing, DODORK );
	sprintf( s, "#%d Entered private chat: %s/%s", slot, MYREC.userid, MYREC.handle );
	xmsg( MSG_PA, s, 0, "" );
	signal( SIGHUP, dccatch );
	dinp( i );
	signal( SIGHUP, SIG_DFL );

/* When dinp finishes, it will have the semaphore, so MUTEX here */
	MYREC.dcreq = -1;
	Vu( cbsem );
/* END MUTEX */
	fclose( f );
	*MYREC.doing = 0;
	sprintf( s, "#%d Returned from private chat: %s/%s", slot, MYREC.userid, MYREC.handle );
	xmsg( MSG_PA, s, 0, "" );

	time( &tdum );
	sprintf( s, "(%s): %s", MYREC.userid, ctime( &tdum ) );
	if( dcfd != -1 ){
		write( dcfd, s, strlen(s) );
		close( dcfd );
	}
	return;
}

/************************************************************************/

STATIC void dinp( i )
int i;
{
	/*	Input routine for private chat
	 */

	char c;

	printf("Press <Esc> or <Ctrl C> to abort\n\n");
	for(;;){
		c = getchar();
		if( c == 033 || c == 003 ){
			printf("Leaving private chat\n");
			Pu( cbsem );
			if( ulog[i].dcreq == slot ){
				fprintf( f, "%s/%s has left private chat\n",
					MYREC.userid, MYREC.handle );
				fprintf( f, "Press <Esc> or <Ctrl C> to abort\n" );
			}
			return;
		}
		if( dcrstat( i ) == slot ){
			if( c == 0177 )
				c = '\b';
			fputc( c, f );
			putchar( c );
			if( dcfd != -1 )
				write( dcfd, &c, 1 );
			if( c == '\r' ){
				fputc( '\n', f );
				putchar( '\n' );
			}
		}
	}
}

/*************************************************************************/

STATIC int dcrstat( i )
int i;
{
	/*	Get dcreq status under mutex for i
	 */

	int j;

	Pu( cbsem );
	j = ulog[i].dcreq;
	Vu( cbsem );

	return( j );
}

/*********************************************************************/

STATIC void dccatch( sig )
int sig;
{
	signal( sig, SIG_IGN );
	
	if( ulog[MYREC.dcreq].dcreq == slot ){
		fprintf( f, "\n%s/%s has disconnected\n",
			MYREC.userid, MYREC.handle );
		fprintf( f, "Press <Esc> or <Ctrl C> to leave private chat\n" );
	}
	MYREC.dcreq = -1;
	exit( 0 );
}
