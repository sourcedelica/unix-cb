#include <stdio.h>
#include <pwd.h>
#include "../include/alias.h"

#define L_cuserid 16
/*
 *	Skynet
 *
 *	Alias/userid subroutines
 *
 */

char *aliasof( lname )
char *lname;
{
	/*
	 *	Returns alias of the given logname
	 *
	 *	Returns the logname if not found
	 *
	 */

	struct passwd *p, *getpwnam();
	static char xalias[L_alias];

	if( (p= getpwnam(lname)) == (struct passwd *)0 )
		return( lname );

	strcpy( xalias, p->pw_gecos );
	return( xalias );
}

char *aliastr( alname )
char *alname;
{
	/*
	 *	Returns user id of given alias
	 *	if user if is given, it is returned
	 *
	 *	Returns alname if not found in passwd file
	 *
	 */

	struct passwd *p, *getpwent();
	char *x, *y;

	setpwent();
	
	while( (p= getpwent()) != (struct passwd *)0 ){
		for( x=alname, y=p->pw_name; (*y!=0)&&(*x!=0); x++, y++ )
			if( toupper(*x) != toupper(*y) )
				break;
		if( (*x == 0)&&(*y == 0) )
			return(p->pw_name);
		for( x=alname, y=p->pw_gecos; (*y!=0)&&(*x!=0); x++, y++ )
			if( toupper(*x) != toupper(*y) )
				break;
		if( (*x == 0)&&(*y == 0) )
			return(p->pw_name);
	}
	return(alname);
}

char *homeof( lname )
char *lname;
{
	/*
	 *	Returns home directory of the given logname
	 *
	 *	Returns NULL on fail.
	 *
	 */

	struct passwd *p, *getpwnam();
	static char xname[L_cuserid];
	static char xhome[100];

	if( !strcmp(xname,lname) )
		return(xhome);

	if( (p= getpwnam(lname)) == (struct passwd *)0 )
		return( NULL );

	strcpy( xhome, p->pw_dir );
	strcpy( xname, lname );
	return( xhome );
}

int uidof( lname )
char *lname;
{
	/*
	 *	Returns userid of the given logname
	 *
	 *	Returns -1 on fail.
	 *
	 */

	struct passwd *p, *getpwnam();
	static char xname[L_cuserid];
	static int xuid;

	if( !strcmp(xname,lname) )
		return( xuid );

	if( (p= getpwnam(lname)) == (struct passwd *)0 )
		return( -1 );

	xuid = p->pw_uid;
	strcpy( xname, lname );
	return( xuid );
}

#ifdef TESTING
main()
{
	char s[80];
	char *x;

	printf("Userid: ");
	gets(s);
	if( (x= aliasof(s)) == NULL )
		printf("(null)\n");
	else
		printf("alias %s\n",x);

	printf("ALias: ");
	gets(s);
	if( (x= aliastr(s)) == NULL )
		printf("(null)\n");
	else
		printf("logname: %s\n",x);
}
#endif
