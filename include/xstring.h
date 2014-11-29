/*
 *	Skynet
 *
 *	Extended String Function definitions
 *
 */

#ifdef ANSI
	int stricmp( char *s, char *t );
	char *strtok2( char *inp, char *delims );
#else
	extern char *strtok2();
	extern char *strstr();
	extern char *strdup();
#endif
