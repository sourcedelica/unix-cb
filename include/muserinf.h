/*
 *	Skynet Mailbox System
 *	User information definitions
 *
 */

#ifdef ANSI
	void muserinf( void );
	char *maliastr( char * );
	char *maliasof( char * );
	int muidof( char * );
#else
	extern void muserinf();
	extern char *maliasof();
	extern char *maliastr();
	extern int muidof();
#endif

#ifndef CMUSERINF
extern char guserid[];
extern char galias[];
extern char gnode[];
#endif

#ifndef L_cuserid
#define L_cuserid 256
#endif



