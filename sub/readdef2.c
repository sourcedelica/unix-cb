#include <ctype.h>
#include <termio.h>

int readdef2( pr, x, lx, def )
char *pr;
char *x;
int lx;
char *def;
{
	int i;
	char c;
	static char echoe[] = { "\b \b" };
	struct termio oldterm;

	saveterm(&oldterm);
	if( oldterm.c_lflag & (ICANON|ECHO) )
		raw();
	printf("%s",pr);
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
	char x[10];

	raw();
	i = readdefault( "hey: ", x, 9, "blah" );
	printf("%d %s\n",i,x);
	sane();
}
#endif
			
