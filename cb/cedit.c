#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "getctl.h"
#include "xstring.h"

/*	Skynet miscellaneous utilites

	ASCII control file editor
	
	Copyright (C) 1989 CPEX Development
	All rights reserved.
*/

static char *name, *type, *def, *explain, *prompt;
static char **cpp, **dpp, exdir[80], *msp;
static int skip = 0;

#define FT_CHAR		'C'
#define FT_NUM		'N'
#define FT_FLAG		'F'
#define FTF_OFFON	'o'

#define PATHSEP	"/"

extern char *m_skynetpath();

/*********************************************************************/

main( argc, argv )
int argc;
char **argv;
{
	static char usage[] = { "usage: %s editscript target\n" };
	FILE *f;
	char line[256], save[256];
	int err = 0, lno = 0, res;

	if( argc != 3 ){
		fprintf(stderr, usage, argv[0]);
		exit( 0 );
	}
	if( (f= fopen(argv[1], "r")) == NULL ){
		fprintf(stderr, "Error opening editscript\n");
		perror( argv[1] );
		exit( 0 );
	}
	cpp = rdctl( argv[2] );

	if( fgets(line, 256, f) == NULL ){
		fprintf(stderr, "Error reading ctledit file\n");
		exit( 0 );
	}
	strtok(line, "\n");
	strcpy(exdir, line);
	msp = m_skynetpath();
	dpp = (char **)NULL;
	while( fgets(line, 256, f) != NULL ){
		lno++;
		if( *line == '#' || *line == '\n' )
			continue;
		strcpy(save, line);
		name = strtok2(line, ";");
		if( name == NULL ){ err++; break; }
		type = strtok2(NULL, ";");
		if( type == NULL ){ err++; break; }
		def = strtok2(NULL, ";");
		if( def == NULL ){ err++; break; }
		explain = strtok2(NULL, ";");
		if( explain == NULL ){ err++; break; }
		prompt = strtok2(NULL, "\n");
		if( prompt == NULL ){ err++; break; }
		if( (res = getchg()) != 0 )
			break;
	}
	fclose( f );
	if( err ){
		fprintf(stderr, "Error reading editscript on line %d:\n", lno);
		fputs(save, stderr);
		exit( 0 );
	}
	if( !res ){
		putchar('\n');
		if( !skip )
			skip = affirm( "Save changes? " );
		if( skip ){
			wrctl(dpp, argv[2]);
			printf("\nChanges saved\n");
			exit( 1 );
		}
	}
	printf("\nChanges NOT saved\n");
	exit( 0 );
}

int getchg()
{
	/*	Get change if any and update incore ctl list

		Returns non-zero if update was aborted
	*/

	char *x, *y, exfile[128], entry[80], buf[128], c;
	int mand, bf, go, res, offon, ok;
	static char on[] = { "ON" };
	static char yes[] = { "YES" };

	mand = 0;
	x = getctl(name, cpp);
	if( *type == FT_FLAG ){
		bf = 0;
		if( x != NULL ){
			/* x here will either point to "" or "0" or "1" */
			if( *x != '0' )	/* Support existing NAME=0 entries */
				bf++;
		}
		offon = type[1] == FTF_OFFON;
		if( bf )
			y = ( offon ? "On" : "Yes" );
		else
			y = ( offon ? "Off" : "No" );
	} else {
		if( x == NULL ){
			if( *def == '-' )
				y = "Undefined";
			else if( *def == '+' ){
				y = "Mandatory";
				mand++;
			} else
				y = def;
		} else
			y = x;
	}
	if( *explain != 0 && (!skip || mand) ){
		strcpy(exfile, msp);
		strcat(exfile, PATHSEP);
		strcat(exfile, exdir);
		strcat(exfile, PATHSEP);
		strcat(exfile, explain);
		listsig( exfile );
	}
	for( go= 1; go; go= (mand || res == -1) ){
		if( skip && !mand ){
			res = 0;
			break;
		}
		printf("%s\n", prompt);
		if( *type != FT_FLAG && *def == '-' && x != NULL )
			printf("Use - to mark as undefined\n");
		printf("[%s]", y);
		if( !(res= readdef2(" ", entry, 80, "")) ){
			if( mand ) puts("You must enter a value");
		} else if( res == -1 ){
			printf("Q)uit w/o saving, K)eep current values for remaining, C)ontinue? ");
			for( ok= 1; ok; ){
				ok = 0;
				c = onechar();
				switch( toupper(c) ){
					case 'Q':	puts("Quit");
							return(-1);
					case 'K':	puts("Keep rest");
							skip = 1;
							break;
					case 'C':	puts("Continue\n");
							break;
					default:	ok = 1;
							break;
				}
			}
		} else break;
	}
	if( *type == FT_FLAG ){
		if( res )
			bf = ( offon ? !stricmp(entry, on) :
					toupper(*entry) == 'Y' );
		if( bf )
			dpp = putctl(name, dpp);
	} else
		if( !res ){
			if( x != NULL ){
				sprintf(buf, "%s=%s", name, x);
				dpp = putctl(buf, dpp);
			} 
		} else
			if( strcmp(entry, "-") ){
				sprintf(buf, "%s=%s", name, entry);
				dpp = putctl(buf, dpp);
			} 
	return( 0 );
}
