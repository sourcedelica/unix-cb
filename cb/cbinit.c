#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include "../include/exitcodes.h"
#define CCB
#include "cb.h"
#include "cbcfg.h"

/*
 *	Skynet Unix CB
 *
 *	Initialization Program
 *
 *	Copyright (C) 1988 CPEX Development
 *	All Rights Reserved.
 *	Author: Eric Pederson
 *
 */

int ulsize;
struct ulrec *ulog;

extern char *shmat();

/**********************************************************************/

void main( argc, argv )
int argc;
char **argv;
{
	int sid, iid, errflg = 0;
	int bytes;
	struct cbmsg mbuf;
	char *x, *cfname = NULL;
	int *pi;
	extern char *optarg;
	extern int optind, errno;
	union semun semarg;

	while( (sid= getopt(argc, argv, "y:")) != -1 )
		switch( sid ){
			case 'y':
				cfname = optarg;
				break;
			case '?':
				errflg++;
				break;
		}
	if( errflg ){
		printf("Usage: %s [-y cfname]\n",argv[0]);
		exit(EXITOK);
	}
	if( cbcfg( cfname ) ){
		printf("CB configuration file not found: %s\n",
			cfname == NULL ? CBCONFIG : cfname );
		exit( 0 );
	}

	if( (iid= msgget(CBKEY,0)) != -1 )
		if( msgctl(iid,IPC_RMID,0) == -1 ){
			perror("cbinit msgctl rmid");
			exit(EXITOK);
		}

	if( (iid= msgget(CBKEY,IPC_CREAT|CBMODE)) == -1 ){
		perror("cbinit msgget");
		exit(EXITOK);
	}

	while( msgrcv(iid, &mbuf, L_cbmsg, 0L, IPC_NOWAIT) != -1 )
		;

	ulsize = NUMSLOTS;

	if( (sid= shmget( CBKEY, 0, 0 )) != -1 )
		shmctl( sid, IPC_RMID, NULL );

/* Increase size of shared memory here */
	bytes = 2*sizeof(int) +
			/* File pointer and CB control flag */
			ulsize*sizeof(struct ulrec) +
				/* User log */
			ulsize*(ulsize+S_NUMGLOB)*sizeof(struct sqrec);
				/* Squelch records */

	if( (sid= shmget( CBKEY, bytes, IPC_CREAT|CBMODE)) == -1 ){
		perror("Getting shared memory");
		exit(EXITOK);
	}

	if( (iid= semget( CBKEY, 0, 0 )) != -1 )
		semctl( iid, 0, IPC_RMID, 0 );
	if( (iid= semget( CBKEY, 1, IPC_CREAT|CBMODE )) == -1 ){
		perror("Getting CB semaphore");
		exit(EXITOK);
	}

	semarg.val = 1;
	if( semctl( iid, 0, SETVAL, semarg ) == -1 ){
		perror("Setting semaphore value");
		exit(EXITOK);
	}	

	if( (iid= open(cbfn(OUTFILE), O_CREAT|O_RDWR, CBMODE)) != -1 )
		close(iid);

	printf("%s\n", cfgxstr( CBLOGIN ));
	printf("Allocating %d slots\n", NUMSLOTS);
	printf("Shared memory size is %d\n",bytes);

	x= shmat( sid, NULL, 0 );
	memset( x, 0, bytes );

	pi = (int *)x;
	pi += 1;	/* skip over "next" */
	*pi = ICTLVAL;
	pi++;
	*pi = -1;	/* fixslot */
}
