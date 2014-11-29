/*
 *	Skynet Unix CB
 *
 *	Miscellaneous function definitions
 *
 */

#ifdef ANSI
	int xmsg( int chan, char *msg, int toslot, char *toal );
	int isval( char *s );
	int lookup( char *s, char *t );
	void msglog( int mslot, int mtype );
	void alist( void );
	int isact( int asking, int who );
	struct sqrec *getsq( int source, int target );
	void dosq( int source, int target, int stype );
	int chhand( char *s );
	int issq( int source, int target );
	void printone( struct dmsg *temp );
	void rcvfix( void );
#else
	extern void msglog();
	extern void alist();
	extern struct sqrec *getsq();
	extern void dosq();
	extern void printone();
	extern void rcvfix();
	extern char *ansstr();
#endif
