#include <malloc.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

/*
 *	Skynet
 *
 *	More string functions
 *
 */

int stricmp( s, t )
char *s;
char *t;
{
	/*
	 *	Case insensitive string compare
	 */
	int i;

	for( ; (*s != 0) && (*t != 0); s++, t++ ){
		if( toupper(*s) > toupper(*t) ) return(1);
		else if( toupper(*s) < toupper(*t)  ) return(-1);
	}
	if (*s == 0 && *t != 0)
		return(-1);
	else if (*s != 0 && *t == 0)
		return(1);
	else
		return(0);
}


/*
 *	Skynet
 *
 *	More string functions
 *
 */

int strincmp( s, t, n )
char *s;
char *t;
int n;
{
	/*
	 *	Case insensitive string compare, n bytes
	 */
	int i;

	for(i=0 ; (*s != 0) && (*t != 0) && (i < n); s++, t++, i++ ){
		if( toupper(*s) > toupper(*t) ) return(1);
		else if( toupper(*s) < toupper(*t)  ) return(-1);
	}
	return(0);
}

/*******************************************************************/

char *bufptr2;		/* Defined globally so nested strtok2 calls
				can be made */

char *strtok2(inptr,delims)
char *inptr;
char *delims;
{
	/*
	 *	Returns zero-length strings between consecutive delimiters
	 */
	register char *cptr,*delimptr;

	if(inptr != NULL)
		bufptr2 = inptr;
	
	for(cptr = bufptr2;;)
	{
		if(cptr == bufptr2 && *bufptr2 == '\0')
			return(NULL);

		for(delimptr = delims;*delimptr != '\0';delimptr++)
		{
			if(*delimptr == *bufptr2)
			{
				*bufptr2++ = '\0';
				return(cptr);
			}else if(*bufptr2 == '\0')
				break;
		}

		if(*bufptr2 == '\0')
			return(cptr);
		else
			bufptr2++;

	}
}

#ifdef NO_STRDUP
char *strdup( what )
char *what;
{
	/*	Mimic ANSI C strdup() function
	 */

	char *x;

	x = malloc( strlen(what) + 1 );
	return( x != NULL ? strcpy( x, what ) : NULL );

}
#endif

char *strlwr( s1 )
char *s1;
{
	/*	Convert string to lower case
	 */

	for( ; *s1; s1++ )
		*s1 = tolower( *s1 );
}

char *strupr( s1 )
char *s1;
{
	/*	Convert string to upper case
	 */

	for( ; *s1; s1++ )
		*s1 = toupper( *s1 );
}

#if 0
char *strstr( s1, s2 )
char *s1, *s2;
{
	/*	Mimic ANSI C strstr()
	 */

	for( ; *s1; s1++ )
		if( !strncmp( s1, s2, strlen(s2) ) )
			return( s1 );
	return( NULL );
}
#endif

#ifdef TESTING
main()
{
	char s[80], t[80], *strsqz();

	for(;;){
		puts("strsqz test");
		printf("s: ");
		gets(s);
		printf("t: ");
		gets(t);
		printf("%s\n",strsqz(s,t));
	}
}
#endif

int isdigstr( s )
char *s;
{
	/* Does an isdigit() for an entire string
	 */

	char *x;

	for( x= s; *x != 0; x++ )
		if( !isdigit( *x ) )
			return( 0 );

	return( 1 );
}

int strcrchr(s, c)
char *s;
char c;
{
	/* start at the end, return position of first character in s
	 * not matching 'c'
	 */
	int	i;

	for (i=strlen(s)-1; i>0 ;i--)
		if (s[i] != c)
			return( i );
	return( 0 );
}

char *strsqz( str, fat )
char *str, *fat;
{
	/*	Squeezes multiple occurences of characters in 
		in fat to one character.

		Note: This would mean strsqz("x112333y", "123") would
		produce "x123y"
	*/
	
	char *x, *sav, *trl;

	sav= trl= str;
	for( ; *str != 0; str++ ){
		*trl = *str;
		if( (x= strchr(fat, *str)) != NULL ){
			for( ; *x == *(str+1); str++ )
				;
		}
		trl++;
	}
	*trl = 0;
	return( sav );
}

strcatc( s, c )
char *s;
char c;
{
	/*	Append character 'c'  and a null onto the end
		of string 's'
	*/

	while( *s )
		s++;
	*s = c;
	s++;
	*s = 0;
}
