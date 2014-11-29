#include <signal.h>
#include <setjmp.h>
#include <stdio.h>

static jmp_buf env;
static int done;

int listsig( t )
char *t;
{
	/*
	 *	List a file, interruptable by signals
	 *	Returns a status:
	 *	 0 - ok
	 *	-1 - file was not found
	 *	SIGQUIT, SIGINT - signal that aborted
	 *
	 */

	void catch();
	FILE *f;
	char s[255];

	done = 0;
	if( (f= fopen(t,"r")) == NULL )
		return(-1);
	if( setjmp(env) ){
		putchar('\n');
	} else {
		setnesig( ' ', 033, catch );
		while( fgets(s,80,f) != NULL )
			fputs(s,stdout);
	}
	unnesig();
	fclose(f);
}

static void catch( blah )
int blah;
{
	done = blah;
	signal(SIGINT,SIG_IGN);
	signal(SIGQUIT,SIG_IGN);
	longjmp(env,-1);
}
