/*
 *	Skynet
 *
 *	Cdate definitions
 *
 */

#ifdef ANSI
	time_t cdate( char *cd );
	char *datec( time_t ttime );
#else
	extern time_t cdate();
	extern char *datec();
#endif
