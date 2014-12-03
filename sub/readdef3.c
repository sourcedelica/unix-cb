#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include "osdefs.h"

int readdef3( pr, x, lx, def )
char *pr;
char *x;
int lx;
char *def;
{
    int i, max;
    char c;
    static char echoe[] = { "\b_\b" }, bl = { '_' };
    TERMIO_OR_TERMIOS oldterm;

    saveterm(&oldterm);
    if( oldterm.c_lflag & (ICANON|ECHO) )
        raw();
    printf("%s",pr);
/*
 *  slick feature based on 80 col display
 */
    max = 79 - strlen(pr);

    for (i=0; i<lx-1 && i < max; i++)
        putchar( bl );
    for (i=0; i<lx-1 && i < max; i++)
        putchar( '\b' );
    memset(x,0,lx);
    i = 0;

    do{
        c = getchar();
        switch(c){
            case 0177:
            case '\b':
                if( i > 0 ){
                    if( x[--i] != 007 )
                        printf("%s",echoe);
                    x[i] = 0;
                }
                break;

            case '\n':
            case '\r':
                putchar('\n');
                if( !i )
                    strcpy(x,def);
                break;

            case 033:
                x[i=0] = 0;
                putchar('\n');
                break;

            default:
                if( i < lx-1 && (!iscntrl(c) || c == 007) ){
                    putchar(c);
                    x[i++] = c;
                }
                break;
        }
    } while( (c != '\r') && ( c != 033 ) && (c != '\n') );

    if( oldterm.c_lflag & (ICANON|ECHO) )
        restterm(&oldterm);

    if (c == 033)
        return( -1 );
    else
        return( strlen(x) );
}

#ifdef TESTING
main()
{
    int i;
    char x[100];

    i = readdef3( "hey: ", x, 100, "blah" );
    printf("%d %s\n",i,x);
    i = readdef3( "hey: ", x, 100, "" );
    printf("%d %s\n",i,x);
}
#endif

