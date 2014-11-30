/*
 *	Getkey.c
 *
 *		this routine will allow a user to have slick one-key operations
 *		under unix.
 *
 *	copyright (c) 1988 Alan Premselaar
 *
 */

#include <stdio.h>
#include "../include/osdefs.h"
#include "getkey.h"

int getkey(status)
int	status;
{
	TERMIO_OR_TERMIOS	oldterm;
	int	ch;

	saveterm(&oldterm);

	if ( oldterm.c_lflag & (ICANON|ECHO) )
		raw();

	if ( (status & (ECHOCH|YESNO)) > ECHOCH )
		status = status & ~ECHOCH;
							/* if ECHO and YESNO, then turn ECHO off */

	ch = getchar();

	if ((status & ECHOCH) && (ch != '\n'))
		printf("%c",ch);

	if (status & YESNO)
		switch(ch) {
			case 'y':
			case 'Y':
					printf("Yes");
					break;
			default:
					printf("No");
					break;
		}

	if (status & NEWLINE)
		printf("\n");

	if (status & NUMBER)
		ch -= 48;

	if ( oldterm.c_lflag & (ICANON|ECHO) )
		restterm(&oldterm);

	return(ch);
}

#ifdef TESTING
main()
{
	int	ch;

	printf("yes or no? ");
	ch = getkey(YESNO);
	printf("  newline: ");
	ch = getkey(NEWLINE);
	printf("  echo: ");
	ch = getkey(ECHOCH);
	printf("  echo & newline: ");
	ch = getkey(ECHOCH|NEWLINE);
	printf("  yesno & newline: ");
	ch = getkey(YESNO|NEWLINE);
	printf("all: ");
	ch = getkey(YESNO|ECHOCH|NEWLINE);
	printf("Number: ");
	printf("%d  ",getkey(NUMBER));
}
#endif
