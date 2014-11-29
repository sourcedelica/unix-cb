/*
 *	Skynet Mailbox System
 *
 *	Line Editor Definitions
 *
 */

#ifdef ANSI
	char **getmsg( int maxlines, int maxllen );
	void freemsg( char **msg );
#else
	extern char **getmsg();
	extern void freemsg();
#endif

#ifndef CGETMSG
extern int titav;	/* Set to non-zero if title is available to change */
extern char *ptitle;	/* Point to title area is titav set */
#endif
