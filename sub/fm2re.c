#include <stdio.h>

/*	File mask -to- Regular expression

	For use in creating regcmp(3X)able vectors
	It is up to YOU to call regcmp(3X).

	It is also up to YOU to supply the destination storage
	for the RE to fill up.

	Note: only handles * and ? type wildcard characters

	fm2re() delimits the RE with ^ and $ so that an entire string
	must be matched.  gm2re() does not.
*/

void gm2re(char *mask, char *dest);

void fm2re( mask, dest )
char *mask;
char *dest;
{
	char buf[128];

	gm2re( mask, buf );
	sprintf( dest, "^%s$", buf );
}
	
void gm2re( mask, dest )
char *mask;
char *dest;
{
	*dest = 0;
	for( ; *mask != 0; mask++ )
		switch( *mask ){
			case '?':	*dest = '.';
					*(++dest) = 0;
					break;
			case '*':	*dest = '.';
					*(++dest) = '*';
					*(++dest) = 0;
					break;
			case '$':
			case '^':
			case '+':
			case '.':	*dest = '\\';
					*(++dest) = *mask;
					*(++dest) = 0;
					break;
			default:	*dest = *mask;
					*(++dest) = 0;
					break;
		}
}

#ifdef TESTING
#include <stdio.h>
main(argc, argv)
int argc;
char **argv;
{
	char s[80], t[80], *x;
	extern char *regcmp(), *regex();

	if( argc == 1 ){
		printf("pass some args, please\n");
		exit( 0 );
	}
	printf("enter mask: ");
	gets(s);
	fm2re( s, t );
	printf("re computed: %s\n",t);
	x = regcmp( t, (char *)0 );
	if( x == NULL ){
		printf("invalid regcmp expression: %s\n",s);
		exit(0);
	}
	for( ; argc > 1; argc-- )
		if( regex( x, argv[argc] ) != NULL )
			printf("%s\n",argv[argc]);
}
#endif
