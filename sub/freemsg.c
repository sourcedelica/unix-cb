#include <stdio.h>

void freemsg( msg )
char **msg;
{
	/*
	 *	Skynet
	 *
	 *	Free up dynamic memory used by a message
	 *
	 */

	char **temp;

	for( temp = msg; *temp != NULL; temp++ )
		free( *temp );
	free( msg );
}
