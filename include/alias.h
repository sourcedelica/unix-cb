/*
 *	Skynet Alias Definitions
 *
 */

#ifdef ANSI
	char *aliasof( char * );
	char *aliastr( char * );
	char *homeof( char * );
	int uidof( char * );
#else
	extern char *aliasof();
	extern char *aliastr();
	extern char *homeof();
	extern int uidof();
#endif

#define L_alias 25
