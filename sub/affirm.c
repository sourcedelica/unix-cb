#include <stdio.h>
#include <string.h>
#include "osdefs.h"

static TERMIO_OR_TERMIOS oldterm;
int ocnorest= 0;

static int waffirm(char *prompt, int isd, int dval);

int affirm( prompt )
char *prompt;
{
	return( waffirm(prompt, 0, 0) );
}

int affirmd( prompt, dval )
char *prompt;
{
	return( waffirm(prompt, 1, dval) );
}

static int waffirm( prompt, isd, dval )
char *prompt;
int isd, dval;
{
	char c, res;

	ocnorest = 1;
	c = ponechar( prompt );
	res = 0;
	if( isd && (c == '\r' || c == '\n') )
		res = (dval ? 1 : 2);
	if( !res )
		res = (toupper(c) == 'Y' ? 1 : (toupper(c) == 'N' ? 2 : 0) );
	while( !res ){
		res = 0;
		c = getchar();
		if( isd && (c == '\r' || c == '\n') )
			res = (dval ? 1 : 2);
		if( !res )
			res = (toupper(c) == 'Y' ? 1 :	
				(toupper(c) == 'N' ? 2 : 0) );
	}
	puts( res == 1 ? "Yes" : "No" );
	restterm( &oldterm );
	return( res == 1 ? -1 : 0 );
}

int onechar()
{
	char c;

	saveterm( &oldterm );
	raw();
	c = getchar();
	if( !ocnorest )
		restterm( &oldterm );
	ocnorest = 0;

	return( c );
}

int ponechar( prompt )
char *prompt;
{
	printf("%s", prompt);
	return( onechar() );
}
