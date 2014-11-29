#include <termio.h>
#include <stdio.h>

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

static struct termio rawsave;

void saveterm( save )
struct termio *save;
{
	ioctl( 0, TCGETA, save );
}

void restterm( save )
struct termio *save;
{
	ioctl( 0, TCSETAW, save );
}

void sane()
{
	struct termio temp;
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
	ioctl( 0, TCSETAW, &temp );
}

void raw()
{
	struct termio temp;

	saveterm( &temp );
	saveterm( &rawsave );
	temp.c_lflag &= ~( ISIG | ICANON | ECHO | ECHOE | ECHOK );
	temp.c_iflag &= ~( IGNCR | ICRNL );
	temp.c_cc[VMIN] = 1;
	temp.c_cc[VTIME] = 0;
	ioctl( 0 , TCSETAW, &temp );
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
	struct termio temp;

	saveterm( &temp );
	temp.c_lflag &= ~( ICANON | ECHO | ECHOE | ECHOK );
	temp.c_lflag |= ISIG;
	temp.c_cc[VINTR] = cintr;
	temp.c_cc[VQUIT] = cquit;
	temp.c_cc[VMIN] = 1;
	temp.c_cc[VTIME] = 0;
	ioctl( 0 , TCSETAW, &temp );
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
