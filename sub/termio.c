#include <stdio.h>
#include "../include/osdefs.h"

/*
**  curses-ish termio routines
**
**  saveterm (restterm)
**	- saves (restores) termio parameters to (from) the given struct
**  sane
**	- canonical input, signals allowed
**  raw
**	- single key, no echo, no signals, CR != NL
**  unraw
**	- undoes last raw
**  sigraw
**	- single key, no echo, signals on user defined chars 
*/

static TERMIO_OR_TERMIOS rawsave;

void saveterm( save )
TERMIO_OR_TERMIOS *save;
{
    tcgetattr(0, save);
}

void restterm( save )
TERMIO_OR_TERMIOS *save;
{
    tcsetattr(0, 0, save);
}

void sane()
{
	TERMIO_OR_TERMIOS temp;
	char *x, *getenv();

	saveterm( &temp );
	temp.c_lflag |= ( ISIG | ICANON | ECHO | ECHOE | ECHOK );
	temp.c_iflag |= ( ICRNL );
	temp.c_cc[VINTR] = CINTR;
	temp.c_cc[VQUIT] = CQUIT;
	if( (x= getenv( "BSCHAR" )) == NULL )
		temp.c_cc[VERASE] = 010;
	else {
		if( *x == 0 )
			temp.c_cc[VERASE] = 010;
		else {
			if( *x == '^' )
				temp.c_cc[VERASE] = *(x+1) - 64;
			else
				temp.c_cc[VERASE] = *x;
		}
	}
	temp.c_cc[VEOF] = CEOF;
    tcsetattr(0, 0, &temp);
}

void raw()
{
	TERMIO_OR_TERMIOS temp;

	saveterm( &temp );
	saveterm( &rawsave );
	temp.c_lflag &= ~( ISIG | ICANON | ECHO | ECHOE | ECHOK );
	temp.c_iflag &= ~( IGNCR | ICRNL );
	temp.c_cc[VMIN] = 1;
	temp.c_cc[VTIME] = 0;
    tcsetattr(0, 0, &temp);
}

void unraw()
{
	restterm( &rawsave );
	return;
}

void sigraw( cintr, cquit )
char cintr;
char cquit;
{
	TERMIO_OR_TERMIOS temp;

	saveterm( &temp );
	temp.c_lflag &= ~( ICANON | ECHO | ECHOE | ECHOK );
	temp.c_lflag |= ISIG;
	temp.c_cc[VINTR] = cintr;
	temp.c_cc[VQUIT] = cquit;
	temp.c_cc[VMIN] = 1;
	temp.c_cc[VTIME] = 0;
    tcsetattr(0, 0, &temp);
}

#ifdef TESTING
main()
{
	char x;
	char s[80];

	gets(s);
	puts(s);

	raw();
	while( (x = getchar()) != '~' )
		putchar(x);

	puts("DONE");
	sane();
	gets(s);
	puts(s);
	sigraw('%','&');
	for(; (x= getchar()) != '~'; )
		;
	sane();
}
#endif
