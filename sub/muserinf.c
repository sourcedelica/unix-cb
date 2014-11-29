#include <stdio.h>
#include <sys/utsname.h>
#include "../include/alias.h"
#include "../include/muserinf.h"
#define CMUSERINF

/*
 *	Skynet
 *
 *	User info operations
 *
 *	Copyright (C) 1987 CPEX Development
 *	All Rights Reserved.
 *	Author: Eric Pederson
 *
 */

extern char *getenv(), *maliasof(), *cuserid();

char guserid[L_cuserid];
char galias[L_alias];
char gnode[L_cuserid];

/***********************************************************************/

void muserinf()
{
	/*	Load user information into global vars
	 *
	 *	(void)
	 */

	char *x;
	struct utsname name;

	/* Skynet stuff here or... */
	if( (x= getenv("LOGNAME")) != NULL )
		strcpy(guserid,x);
	else
		(void)cuserid(guserid);

	if( (x= getenv("NAME")) != NULL )
		strncpy(galias,x,L_alias-1);
	else
		strcpy(galias,maliasof(guserid));
	uname( &name );
	strcpy(gnode, name.nodename);
}	

char *maliastr( source )
char *source;
{
	/*
	 *	Translate an alias into a user-id
	 *
	 *	Uses Skynet aliastr()
	 *
	 *	Returns the corresponding login ID for the alias
	 *	given.  If a login ID is given it is returned.
	 *	If the alias given is not found, NULL is returned.
	 */

	/* Skynet stuff here or... */
	/* decl for aliastr given in <skynet/alias.h>	*/
	return(aliastr(source));
}	

char *maliasof( source )
char *source;
{
	/*
	 *	Translate a user-id into an alias
	 *
	 *	Uses Skynet aliasof()
	 *
	 *	Returns the corresponding alias for the login-id
	 *	given. If the login-id given is not found, NULL
	 *	is returned.
	 */

	/* Skynet stuff here or... */
	/* decl for aliasof given in <skynet/alias.h>	*/
	return(aliasof(source));
}	
