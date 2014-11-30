#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "../include/envar.h"
#define CGETMSG
#include "../include/getmsg.h"
#include <setjmp.h>
#include <signal.h>
#include "../include/osdefs.h"

/*
 *	Skynet Mailbox System
 *
 *	Line Editor
 *
 *	Copyright (C) 1987 CPEX Development
 *	All Rights Reserved.
 *	Author: Eric Pederson
 *
 */

#define WARN    5               /* Lines left to warning */
#define WRAPMAX 20              /* Max wrap characters */
#define WRBREAK 3               /* Breaking point for wrap before EOL */
#define TABLEN	5		/* Number of spaces in a tab */
#define CPPNULL (char **)0

#define G_CONT  -1
#define G_OK    0
#define G_QUIT  1
#define G_EDIT  2

#define ACT_BS          1
#define ACT_OK          2
#define ACT_WRAP        3
#define ACT_CMD         4

/*extern char *malloc(), *calloc(), *getenv();*/

static int gabort();
static int gadd();
static void gereplace(int line, char **msg, int maxllen);
static int gcmd();
static void glist( char **msg );
static void ecatch( int blah );
static char *gctr( char *s, int maxllen );
static int gedit( int *pline, char **msg, int maxlines, int maxllen );
static void getitle( int maxllen );
static int genew( int *pline, char **msg, int maxlines );
static void geinsert( int *pline, char **msg, int maxlines, int maxllen );
static void gedelete( int *pline, char **msg );
static int gnum( char *pr, int high, int low, char **msg );
     
static char **msg;
static TERMIO_OR_TERMIOS oldterm;
static int myslot = -1;
static int altmod = 0, alton = 0;

#ifdef UNIX
static jmp_buf env;
#endif

int titav = 0;          /* Set to not zero if title is available */
char *ptitle;           /* Set to pointer to title, if avail */

/********************************************************************/

char **getmsg( maxlines, maxllen )
int maxlines;
int maxllen;
{
        int go, act, line = 0;
        char *s, *w;

	m_at();	/* Secondary calls to m_at are ignored */
	if( (s= getenv( SE_SLOT )) != NULL )
		myslot = atoi(s);
	s = malloc(maxllen+1);
	w = malloc(maxllen+1);
        msg = (char **)calloc(maxlines+1,sizeof(char *));
        printf("Enter up to %d lines. ",maxlines);
        if( maxllen < 80 )
                putchar('\n');
        printf("/E to edit, /S to save or /? for help\n");
        for( go= 0; go < maxllen-WRBREAK; go++ )
                putchar('-');
        puts("!");

        memset(s,0,maxllen+1);
	saveterm(&oldterm);
	raw();

        for( go= G_CONT; go == G_CONT; ){

                if( line == maxlines ){
                        printf("%d lines!\n",maxlines);
                        act = ACT_CMD;
                        s[1] = 'E';
                } else
                        act = _getline( maxllen, s, w );

                switch( act ){

                        case ACT_BS:
                                if( line > 0 ){
                                        line--;
                                        printf("-Back-\n");
                                        strcpy(s,msg[line]);
                                        free(msg[line]);
                                        msg[line] = NULL;
                                } else
                                        printf("-Top-\n");
                                break;
                
                        case ACT_OK:
                        case ACT_WRAP:
                                line = gadd( line, s, msg, maxlines, maxllen );
                                if( (line != maxlines) && (act == ACT_WRAP) )
                                        strcpy(s,w);
                                break;                  

                        case ACT_CMD:
                                go = gcmd( &line, s, msg, s[1], maxlines, maxllen );
                                *s = 0;
                                break;
                }
        }

        free(s);
        free(w);

        if( go != G_OK ){
                free(msg);
                msg = CPPNULL;
        }

	restterm(&oldterm);
        return( msg );
}

/***/

int _getline( maxllen, s, w )
int maxllen;
char *s;
char *w;
{
        char x;
        static char *echoes = { "\b \b" };

        int spos, pos, i;

	spos = 0;
	pos = strlen(s);
        memset(w,0,maxllen+1);
        printf("%s",s);

        for(;;){
                switch( (x= getchar()) ){

			case 0177:
                        case '\b':
                                if( pos == 0 )
                                        return( ACT_BS );
                                s[--pos] = 0;
                                printf("%s",echoes);
                                break;

			case '\r':
                        /*case '\n':*/
                                s[pos] = 0;
                                putchar('\n');
                                return( *s == '/' || *s == '.' ? ACT_CMD : ACT_OK );

			case 022:
			case 030:
				if( pos == 0 )
					return(ACT_BS);
				while( pos > 0 ){
					printf("\b \b");
					if( s[--pos] == ' ' && x == 022 ){
						s[pos] = 0;
						break;
					} else
						s[pos] = 0;
				}
				break;
		
                        case 033:       
                                pos = 0;
                                s[pos] = 0;
                                putchar('\n');
                                break;

			case '\t':
				for( i=0; pos != (maxllen-WRBREAK) && i != TABLEN; i++ ){
					spos = pos;
					s[pos] = ' ';
					s[++pos] = 0;
					putchar(' ');
				}
					break;
			case 001:
				altmod = !altmod;
				break;
                        case ' ':       
                                spos = pos;
				/* FALLS THROUGH */
                        default:
				if( iscntrl(x) )
					break;
				if( altmod && (alton= !alton))
					x = toupper(x);
                                if( pos != (maxllen-WRBREAK) ){
                                        s[pos] = x;
                                        s[++pos] = 0;
                                        putchar(x);
                                } else {
                                        s[pos] = 0;
                                        if( (pos= (maxllen-WRBREAK-spos)) < WRAPMAX ){
                                                s[spos] = 0;
                                                strcpy(w,s+spos+1);
                                                for(;pos>0;pos--)
                                                        printf("%s",echoes);
                                        }
                                        putchar('\n');
                                        if( x != ' ')
                                                w[strlen(w)] = x;
                                        return(ACT_WRAP);
                                }
                                break;
                }
        }
}

/***/

static int gadd( line, s, msg, maxlines, maxllen )
int line;
char *s;
char **msg;
int maxlines;
int maxllen;
{
        msg[line] = malloc(maxllen+1);
        strcpy(msg[line],s);
        line++;
        
        if( line == (maxlines-WARN) )
                printf("%d lines left\n",WARN);
        memset(s,0,maxllen+1);
        msg[line] = NULL;
        
        return( line );
}

/***/

static int gcmd( pline, s, msg, cmd, maxlines, maxllen )
int *pline;
char *s;
char **msg;
char cmd;
{
        switch( toupper(cmd) ){

                case 'E':       
                        return( gedit( pline, msg, maxlines, maxllen ) );

                case 'Q':
                case 'A':       
                        return( gabort( G_CONT ) );

                case 'S':       
                        if( *pline == 0 ){
                                puts("No lines");
                                return( G_CONT );
                        } else
                                return( G_OK );

                case 'L':       
                        glist( msg );
                        return( G_CONT );

                case 'D':       
                        if( *pline == 0 )
                                printf("-Top-\n");
                        else {
                                printf("-Delete-\n");
                                free(msg[--(*pline)]);
                                msg[*pline] = NULL;
                        }
                        return( G_CONT );

                case 'C':       
                        gctr( s, maxllen );
                        puts(s);
                        *pline = gadd( *pline, s, msg, maxlines, maxllen );
                        return( G_CONT );

/*
		case '%':
			pguser( myslot );
			return( G_CONT );
			break;
*/

		case '$':
			m_who( myslot );
			return( G_CONT );
			break;

                case '?':
                        printf("/ commands: S)ave, E)dit, Q)uit, L)ist,");
                        if( maxllen < 80 )
                                printf("\n           ");
                        puts  (" A)bort, D)elete, C)enter Text");
			puts  (" %)Page, $)Who");
                        return( G_CONT );

                default:
                        puts("Invalid command.  Use /? for help");
                        return( G_CONT );
        }

}

/***/

static int gabort( noval )
int noval;
{
	int c;

        printf("Abort message? ");
        
	c = getchar();
        if( toupper(c) == 'Y' ){
                puts("Yes");
                return( G_QUIT );
        }

        puts("No");
        return( noval );
}

/***/

static void glist( msg )
char **msg;
{
        char x;
        int i;
        char **curr;
        void ecatch();

        if( *msg == NULL ){
                puts("No Lines");
                return;
        }
        printf("Line numbers? ");
        x = getchar();
	x = toupper(x);

        if( x == 'Q' ){
                puts("Quit");
                return;
        } else if ( x == 'Y' ){
                puts("Yes");
                i = 1;
        } else
                puts("No");

#ifdef UNIX
        if( setjmp(env) ){
                signal(SIGINT,SIG_IGN);
                signal(SIGQUIT,SIG_IGN);
        } else {
                setnesig( ' ', 033, ecatch );
#endif
                curr = msg;

                while( *curr != NULL ){
                        if( x == 'Y' )
                                printf("%.2d ",i++);
                        puts(*curr++);
                }
#ifdef UNIX
        }
        unnesig();
#endif
}

#ifdef UNIX
static void ecatch( blah )
int blah;
{
        putchar('\n');
        longjmp(env,-1);
}
#endif

/***/

static char *gctr( s, maxllen )
char *s;
int maxllen;
{
        int pad;
        char *x;

        pad = ( maxllen-strlen(s) ) / 2;
        x = malloc(maxllen+1);
        strcpy(x,s+2);                  /* Skip over command */
        *s = 0;

        for( ; pad > 0; pad-- )
                strcat(s," ");
        strcat(s,x);
        free(x);

        return( s );
}
        
static int gedit( pline, msg, maxlines, maxllen )
int *pline;
char **msg;
int maxlines;
int maxllen;
{
        int go;
        char x;
        static char *gecmds = { "?SAQCRLDITN$%" };

	printf("- %d lines - \n",*pline);
        for( go= G_EDIT; go == G_EDIT; ){
                printf("\nEdit> ");

		x = getchar();
                while( strchr(gecmds,x= toupper(x)) == NULL )
                        x = getchar();

                switch( x ){

                        case '?':       
                                puts("Help");
                                printf("S)ave, C)ontinue, Q)uit, L)ist, I)nsert, ");
                                if( maxllen < 80 )
                                        putchar('\n');
                                puts("D)elete, R)eplace, T)itle, N)ew");
				puts("$)Who, %)Page");
                                break;

                        case 'S':       
                                puts("Save");
                                if( *pline == 0 )
                                        puts("No lines");
                                else
                                        go = G_OK;
                                break;

                        case 'A':
                        case 'Q':
                                puts( x == 'A' ? "Abort" : "Quit");
                                go= gabort( G_EDIT );
                                break;

                        case 'C':       
                                puts("Continue");
                                if( (*pline > 0) && (*pline < maxlines) )
                                        puts(msg[(*pline)-1]);
                                go = G_CONT;
                                break;

                        case 'L':       
                                puts("List");
                                glist( msg );
                                break;

                        case 'R':       
                                puts("Replace");
                                gereplace( *pline, msg, maxllen );
                                break;

                        case 'D':       
                                puts("Delete");
                                gedelete( pline, msg );
                                break;

                        case 'I':       
                                puts("Insert");
                                geinsert( pline, msg, maxlines, maxllen );
                                break;

                        case 'T':       
                                puts("Title");
                                getitle( maxllen );
                                break;

                        case 'N':       
                                puts("New");
                                go = genew( pline, msg, maxlines );
                                break;
			case '%':
				puts("Page");
				pguser( myslot );
				break;
			case '$':
				puts("Who");
				m_who( myslot );
				break;
                        default:
                                puts("Not installed");
                                break;
                }
        }
        return( go );
}

/***/

static void getitle( maxllen )
int maxllen;
{
        char *x;
        int i;

        if( titav == 0 ){
                puts("Cannot change title");
                return;
        }

        x = malloc(maxllen+1);
        i = readdefault("New title: ",x,maxllen,"");
        if( i == 0 ){
                free(x);
                return;
        }

        strcpy(ptitle,x);
        free(x);
        return;
}
 
/***/

static int genew( pline, msg, maxlines )
int *pline;
char **msg;
int maxlines;
{
	int c;

        printf("Sure? ");

	c = getchar();
        if( toupper(c) != 'Y' ){
                puts("No");
                return( G_EDIT );
        }

        puts("Yes");
        *pline = 0;
        memset(msg,0,sizeof(char *)*(maxlines+1));
        puts("-New message-");

        return( G_CONT );
}

/***/

static void geinsert( pline, msg, maxlines, maxllen )
int *pline;
char **msg;
int maxlines;
int maxllen;
{
        char *x;
        int i, n;

        if( *pline == maxlines ){
                puts("Message full");
                return;
        }

        printf("Message has %d line%s\n",*pline,*pline==1?"":"s");
        if( (n= gnum( "Insert before: ", *pline, 1, msg )) == 0 )
                return;

        x= malloc(maxllen+1);
        readdefault("> ",x,maxllen,"");
        for( i= (*pline)-1; i >= n-1; i-- )
                msg[i+1] = msg[i];
        msg[n-1] = x;
        (*pline)++;
        msg[*pline] = NULL;
}
        
/***/

static void gedelete( pline, msg )
int *pline;
char **msg;
{
        int i,j,k;

        if( *pline == 0 ){
                puts("Message empty");
                return;
        }

        if( (j= gnum( "Delete from: ", *pline, 1, msg )) == 0 )
                return;
        if( (k= gnum( "To: ", *pline, j, msg )) == 0 )
                k = j;

        for( i= j-1; i < k; i++ )
                free(msg[i]);
        for( i= 0; i < (*pline)-k; i++ )
                msg[j+i-1] = msg[k+i];

        *pline -= k-j+1;
        msg[*pline] = NULL;
}
                
/***/

static void gereplace( line, msg, maxllen )
int line;
char **msg;
int maxllen;
{
        char *x;
        int i;

        if( (i= gnum( "Replace line: ", line, 1, msg )) == 0 )
                return;

        x = malloc(maxllen+1);
        puts("Replace:");
        puts(msg[i-1]);
        puts("With:");
        readdefault("",x,maxllen,"");
        free(msg[i-1]);
        msg[i-1] = x;
}

/***/
        
static int gnum( pr, high, low, msg )
char *pr;
int high;
int low;
char **msg;
{
        int i;
        char sx[8];

        do{
                memset(sx,0,8);
                i= readdefault(pr,sx,7,"");
                if( i == 0 )
                        return(0);
                i = atoi( sx );
                if( toupper(*sx) == 'L' ){
                        glist( msg );
                        i = -1;
                } else if( (i < low) || (i > high) ){
                        printf("%s is out of range\n",sx);
                        i = -1;
                }
        } while( i == -1 );

        return( i );
}
