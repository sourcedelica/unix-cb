#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>

/*
 *	Semaphore operations
 *
 *	Copyright (C) 1988 CPEX Development
 *	All Rights Reserved.
 *	Author: Eric Pederson
 *
 */

/***********************************************************************/

int P( sid )
int sid;
{
	/*
	 *	Perform P operation on semaphore 0 in set sid
	 *
	 *	Returns Sys V result of system call
	 */

	return( semcall( sid, -1, 0 ) );
}


int Pu( sid )
int sid;
{
	/*
	 *	Perform P operation on semaphore 0 in set sid
	 *	Perform operation with SEM_UNDO flag set
	 *
	 *	Returns Sys V result of system call
	 */

	return( semcall( sid, -1, SEM_UNDO ) );
}


int V( sid )
int sid;
{
	/*
	 *	Perform V operation on semaphore 0 in set sid
	 *
	 *	Returns Sys V result of system call
	 */

	return( semcall( sid, 1, 0 ) );
}


int Vu( sid )
int sid;
{
	/*
	 *	Perform V operation on semaphore 0 in set sid
	 *	Perform operation with SEM_UNDO flag set
	 *
	 *	Returns Sys V result of system call
	 */

	return( semcall( sid, 1, SEM_UNDO ) );
}


static int semcall( sid, op, flag )
int sid;
int op;
int flag;
{
	/*
	 *	Perform a semaphore operation on semaphore 0 in set sid
	 *
	 *	Returns Sys V system call result
	 *
	 */

	struct sembuf buf;

	buf.sem_num = 0;
	buf.sem_op = op;
	buf.sem_flg = flag;

	return( semop( sid, &buf, 1 ) );
}
