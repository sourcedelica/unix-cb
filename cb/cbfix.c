#include <stdio.h>
#include <signal.h>
#include "cb.h"

/*
 *	Skynet Unix CB
 *
 *	Fix Xenix fuckups
 *
 *	Copyright (C) 1988 CPEX Development
 *	All Rights Reserved.
 *	Author: Eric Pederson
 */

struct ulrec *ulog;
int ulsize;
int sid;
int *pnext;
struct sqrec *sqlog;
int fixsid;

extern char *shmat();

/************************************************************************/

void main()
{
	int i;
	char *sbase;

	for(;;){
		sleep(30);
		printf("CBFIX loaded\n");
		if( (sid= shmget(CBKEY,0,0)) == -1 ){
			perror("Attaching shared memory");
			exit(0);
		}
		i = 0;
		sbase = shmat( sid, NULL, 0 );
		pnext = (int *)sbase;
		ulsize = *pnext;
		i += sizeof(int);
		pnext = (int *)(sbase + i);
		i += sizeof(int);
		ulog = (struct ulrec *)(sbase + i);
		i += sizeof(struct ulrec)*ulsize;
		sqlog = (struct sqrec *)(sbase + i);

		if( (fixsid= semget(FIXKEY,0,0)) == -1 ){
			perror("semget fixsid");
			exit(0);
		}

		printf("Waiting on semaphore id %d\n",fixsid);
		P(fixsid);
		printf("CBFIX activated\n");
		for( i= 0; i < ulsize; i ++ )
			if( (int)ulog[i].pid ){
				if( kill(SIGTERM,(int)ulog[i].pid) != -1 )
					printf("killed pid %d\n",
						(int)ulog[i].pid);
			}

		system("/u/usr/eric/cb/cbinit 20");
	}
}
