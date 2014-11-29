#include <stdio.h>
#include <string.h>
#include "../include/protos.h"

/*	Skynet external file transfer protocol handling
*/

extern char *prodefault();

/**********************************************************************/

int prochange( fn, pbuf )
char *fn;
struct probuf *pbuf;
{
	/*	Change working protocol

		If fn is NULL, uses default proto file name
		Assumes pbuf has been previously loaded

		Also assumes raw mode

		Returns -1 if no change or the letter of the new
		protocol
	*/

	char c, s[128];
	int go;

	putchar('\n');
	printf("Current protocol: %s (%c)\n", pbuf->pname, pbuf->pletter );
	for( go= 1; go; ){
		printf("Enter letter for new protocol (? for list): ");
		c = getchar();
		c = toupper(c);
		if( c == '\r' || c == '\n' || c == 033 ){
			putchar('\n');
			return( -1 );
		}
		if( c == '?' ){
			puts("Help");
			prolist( fn );
			putchar('\n');
			continue;
		}
		go = 0;
		if( proload(fn, c, pbuf) ){
			puts("Invalid selection");
			go = 1;
		} else {
			printf( "%s", pbuf->pname );
			putchar('\n');
		}
	}
	return( c );
}

/*********************************************************************/

prolist( fn )
char *fn;
{
	/*	List protcols in PROTONAME file
	*/

	FILE *f;
	char s[256], *x, *y;

	if( fn == NULL ) fn = prodefault();
	if( (f= fopen(fn, "r")) == NULL ){
		printf("Error opening %s\n", fn);
		return;
	}

	while( fgets( s, 256, f ) != NULL ){
		x = strtok( s, ";" );
		y = strtok( NULL, ";" );
		printf("%c) %s\n", *x, y );
	}
	fclose( f );
}


int proload( fn, cp, pbuf )
char *fn;
char cp;
struct probuf *pbuf;
{
	/*	Loads pbuf for the protocol associated with cp

		fn is the name of the file to load from.  If NULL,
		the default protocol file is used.

		Note: protocol variables that are char *'s are
		loaded with pointers to static areas in memory
		so each call makes the old pointers invalid

		Returns non-zero on failure
		pbuf is unchanged on failure
	*/

	FILE *f;
	static char s[256], sbak[256];
	char *x;
	struct probuf ptemp;
	int lno;

	if( fn == NULL ) fn = prodefault();
	if( (f= fopen( fn, "r" )) == NULL ){
		fprintf( stderr, "(proload 1) " );
		perror( fn );
		return( -1 );
	}

	lno = 0;
	cp = toupper( cp );
	while( fgets( s, 256, f ) != NULL ){
		lno++;
		if( *s == '#' ) continue;
		strcpy(sbak, s);
		x = strtok( s, ";\n" );
		if( toupper(*x) == cp ){
			ptemp.pletter = toupper(*x);
			x = strtok( NULL, ";\n" );
			if( x == NULL ){
				printf("Error in NAME field in %s\n", fn);
				printf("Line: %s\n", sbak);
				break;
			}
			ptemp.pname = x;
			x = strtok( NULL, ";\n" );
			if( x == NULL ){
				printf("Error in FLAGS field in %s\n", fn);
				printf("Line: %s\n", sbak);
				break;
			}
			ptemp.pbatch = (strchr(x, PF_BATCH) != NULL);
			ptemp.pstdin = (strchr(x, PF_STDIN) != NULL);
			x = strtok( NULL, ";\n" );
			if( x == NULL ){
				printf("Error in BSIZE field in %s\n", fn);
				printf("Line: %s\n", sbak);
				break;
			}
			ptemp.pbsize = atoi(x);
			x = strtok( NULL, ";\n" );
			if( x == NULL ){
				printf("Error in BJUNK field in %s\n", fn);
				printf("Line: %s\n", sbak);
				break;
			}
			ptemp.pbjunk = atoi(x);
			x = strtok( NULL, ";\n" );
			if( x == NULL ){
				printf("Error in AMSG field in %s\n", fn);
				printf("Line: %s\n", sbak);
				break;
			}
			ptemp.pamsg = x;
			x = strtok( NULL, ";\n" );
			if( x == NULL ){
				printf("Error in LFOP field in %s\n", fn);
				printf("Line: %s\n", sbak);
				break;
			}
			ptemp.plfop = x;
			x = strtok( NULL, ";\n" );
			if( x == NULL ){
				printf("Error in SCMD field in %s\n", fn);
				printf("Line: %s\n", sbak);
				break;
			}
			ptemp.pscmd = x;
			x = strtok( NULL, ";\n" );
			if( x == NULL ){
				printf("Error in RCMD field in %s\n", fn);
				printf("Line: %s\n", sbak);
				break;
			}
			ptemp.prcmd = x;
			fclose( f );
			memcpy(pbuf, &ptemp, sizeof(struct probuf));
			return( 0 );
		}
	}
	fclose( f );
	return( -1 );
}


/*static*/ char *prodefault()
{
	/*	Handles requests for the default protocol file

		Forms file name on the 1st call only
	*/

	static char *xfn = NULL;
	static char xbuf[128];

	if( xfn == NULL ){
		strcpy( xbuf, m_skynetpath() );
		strcat( xbuf, "/lib/" );
		strcat( xbuf, PROTOFILE );
		xfn = xbuf;
	}
	return( xbuf );
}
