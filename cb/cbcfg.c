#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "config.h"
#include "osdefs.h"
#include "xstring.h"
#include "cb.h"
#include "cbetc.h"
#define CCBCFG
#include "cbcfg.h"

/*  Unix CB
    Configuration functions
*/

/* These are the defaults for the ctl values.  These values will be used
   if not specified specifically */

int NUMSLOTS=   35;     /* Number of slots to allocate */
int URESSIZ=    15;     /* Number of reserved slots for SLOT= */
int FSIZE=  2000;       /* NUmber of records for circular file */
int CBKEY=  88;     /* Key to be used for IPCs */
int HOMEREC=    0;      /* Define if stored in user's directory */
char *RECNAME=  ".cb";      /* If stored in user's directory */
int MAXCHAN=    99;     /* Max channels */
int LIMCHAN=    0;      /* Limited channels feature */
int LIMBASE=    90;     /* Base channel for limiting */
int LIMIT=  9;      /* Number of limited channels */
int MONCHAN=    0;      /* If defined, what channel to monitor */
int MONPAY= 0;      /* 1 if paying status needed for MON */
int MONBASE=    80;     /* Base channel for monitoring */
int MONLIM= 9;      /* Number of monitoring channels */
char *LFCHAR=   "~";        /* Embedded line feed characters */
int MAXNL=  14;     /* Maximum new lines embedded */
int MAXMYOUT=   7;      /* Max unanswered public messages before */
int MAXPOUT=    24;     /*  auto-logoff (POUT for $ users) */
int MAXUOUT=    50;     /*  auto-logoff (POUT for uglylinks) */
char *FAKEOUT=  "Main Menu";    /* For inactive Skynet users */
char *AFORDEF=  "  %$%S Ch %C %u/%h %d %m"; /* Active format default */
char *FORDEF=   "%*%#%S%*%? %u/%h: %m"; /* Msg  format default */
char *DOSHELL=  "(Shell Escape)";   /* CB doing for /! */
char *DOFORM=   "(Changing Format)";    /* CB doing for /f */
char *DOINACT=  "(Inactive)";       /* CB doing if inactive */
char *DOTYPE=   "(Typing)";     /* CB doing if typing */
char *DODORK=   "(Private chat)";   /* In private chat */
int PAIDLF= 0;      /* Define if linefeeds reqs $ */
int PAIDDC= 0;      /* Define if dorkychat init req $ */
char *CBLOGIN=  "Unix CB version %v";   /* Login message */
char *LOCKEDMSG= "CB is unavailable at this time";
int ICTLVAL=    0;  /* Initial CB control flag value */
            /* Note: can be specified in octal or hex by a
               leading 0 or 0x, respectively */
int INITCHAN=   1;  /* Initial channel for regular users */
int PINITCHAN=  1;  /* Initial channel for subscribers */
int UINITCHAN=  2;  /* Ugly links initial channel */
int RCLOW=  1;  /* Regular user base channel limit */
int RCHIGH= 10; /* Regular user high channel limit */
int FORCEH= 0;  /* Force handle to gcos entry for regular users */
int FORCEL= 0;  /* Force listing on for regular users */
int AFORCEH=    0;  /* Force handle for all users */
int AFORCEL=    0;  /* Force listing on for all users */
char *CMDFILE=  "commands";     /* Command file in cbdir */
char *CBDIR=    "lib/cb";   /* Off skynetpath */
char *RECDIR=   "recs";     /* Off skynetpath/CBDIR if not HOMEREC */
char *OUTFILE=  "cbout";    /* Off skynetpath/CBDIR */
char *MSGPBAD=  "Command requires subscription";    /* $ Req'd mesg */
char *MSGSBAD=  "Invalid command";  /* @ Req'd mesg */
char *MSGVBAD=  "Invalid command";  /* VIP Req'd mesg */
char *MSGCBAD=  "Invalid command";  /* CO reqd mesg */
char *MSGBAD=   "Invalid command";  /* General msg */
char *CBHELP=   NULL;
/* The following are predefined as NULL, which means "don't check"
   the environment variable.  Define then in the cfg file to check */
char *ENVPAID=      NULL;
char *ENVSYSOP=     NULL;
char *ENVCOSYSOP=   NULL;
char *ENVVIP=       NULL;
char *ENVVALID=     NULL;
int WRITELEV=       -1; /* Must be above this level to write
                    -1 disables feature */
char *DAILY=    NULL;       /* Name of daily file */
int STTY0=  0;  /* Do an "stty 0" at logout */
int KPERCENT=   100;    /* Percentage of paying users online that must
               vote to /k someone to activate */
int COACT=  0;  /* Do cosysops have inactive powers */
int COPASSX=    0;  /* Do cosysops have passive squelching powers */
int COONE=  0;  /* Do cosysops have one vote kill powers */
int COPPA=  0;  /* Can cosysops read PPAs */
int COLOCKOV=   0;  /* Cosysops can sign on a locked out CB */
int CONEGV= 0;  /* Cosysops can take away time */
int COKGLOB=    0;  /* Cosysops can kill global */
int COKINACT=   0;  /* Cosysops can kill while inactive */
int COKPAID=    0;  /* Co's can kill subscribers */
int TSINTERVAL= 15; /* Time stamp PA interval, if 0 no timestamp */
int LOGCOPAS=   0;  /* Log cosysop action PA messages */
int QUOTA=  -1; /* Quota of /v minutes per time period, -1 for none */
char *CBINIT=   "cbinit";   /* Name including path if needed of cbinit */
int DEBUG=  0;  /* Debug value */
char *UNIXWHO=  "who";  /* Name of who program to run for /w */

/* Make sure you update the cbcfg() fucnction for any added values,
    in addition to the cbcfg.h file */

#define CMD_REG     '#'
#define CMD_PAID    '$'
#define CMD_SYSOP   '@'
#define CMD_VIP     '*'
#define CMD_COSYSOP '%'

#define CMDTYPES    5

char cmdchar[CMDTYPES+1] = { CMD_REG, CMD_PAID, CMD_COSYSOP, CMD_SYSOP,
            CMD_VIP, 0 };
int cmdops[CMDTYPES] = { 0, OP_PAID, OP_COSYSOP, OP_SYSOP, OP_VIP };
char *cmdbads[CMDTYPES];

struct cmdnode *cmdlist = CNNULL;
char **rdctl();

/*********************************************************************/

char *cbfn( dataf )
char *dataf;
{
    /*  Generic file name former for cb

        Returns a pointer to a static area whose
        contents are overwritten on each call
        Make your own copy!
     */

    static char buf[128];

    strcpy( buf, basepath() );
    strcat( buf, PATHSEP );
    strcat( buf, CBDIR );
    strcat( buf, PATHSEP );
    strcat( buf, dataf );
    return( buf );
}

char *cbqfn( user )
char *user;
{
    /*  Create a quota file name for user
        This function knows that cbfn has ample room to strcat
        on to
    */

    char *x;

    x = cbfn( QUOTADIR );
    strcat(x, PATHSEP);
    strcat(x, user);

    return( x );
}

char *cbrfn( user )
char *user;
{
    /*  Create a record file name for user
        This function knows that cbfn has ample room to strcat
        on to
    */

    char *x;

    x = cbfn( RECDIR );
    strcat(x, PATHSEP);
    strcat(x, user);

    return( x );
}

/********************************************************************/

char *cfgxstr( instr )
char *instr;
{
    static char s[400], ds[FORMAX];
    char *x;

    memset( s, 0, 400 );

    for( x= instr; *x != 0; x++ ){
        if( *x != '%' )
            s[strlen(s)] = *x;
        else
            switch( *(++x) ){
                case 'n':
                    strcat( s, "\n" );
                    break;
                case 'v':
                    strcat( s, CBVERS );
                    break;
            }
    }
    return( s );
}

/******************************************************************/

STATIC int loadcmds( fn )
char *fn;
{
    /*  Returns non-zero on failure
     */

    char s[128], *x;
    FILE *f;
    struct cmdnode *temp, *curr = CNNULL;
    int i;

    strcpy( s, basepath() );
    strcat( s, PATHSEP );
    strcat( s, CBDIR );
    strcat( s, PATHSEP );
    strcat( s, fn );

    if( (f= fopen(s, "r")) == NULL ){
        printf("Could not load commands from %s\n", s);
        return( -1 );
    }
    /* These must be loaded at runtime */
    cmdbads[CI_REG] = "You suck";
    cmdbads[CI_PAID] = MSGPBAD;
    cmdbads[CI_COSYSOP] = MSGCBAD;
    cmdbads[CI_SYSOP] = MSGSBAD;
    cmdbads[CI_VIP] = MSGVBAD;

    while( fgets( s, 128, f ) != NULL ){
        if( *s == '#' || *s == '\n' )
            continue;
        temp = (struct cmdnode *)malloc(sizeof(struct cmdnode));
        x = strtok(s, ";");
        temp->type = *x;
        x = strtok(NULL, ";");
        for( i= 0; cmdchar[i] != 0; i++ )
            if( cmdchar[i] == *x ) temp->index = i;
        x = strtok(NULL, ";");
        temp->acts = strdup( x );
        x = strtok(NULL, "\n");
        temp->desc =
            strdup( x == NULL ? "" : x );
        temp->next = CNNULL;
        if( curr == CNNULL )
            cmdlist = temp;
        else curr->next = temp;
        curr = temp;
    }
    fclose( f );

    return( 0 );
}

/*********************************************************************/

int cbcfg( cfname )
char *cfname;
{
    /*  Load global variables with configuration values
        returns non-zero on fail
     */

    char s[128], *x, **p;
    int i;
    extern char *getctl();

    strcpy( s, basepath() );
    strcat( s, PATHSEP );
    strcat( s, SKYCFDIR );
    strcat( s, PATHSEP );
    strcat( s, cfname == NULL ? CBCONFIG : cfname );

printf("%s\n", s);
    if( (p= rdctl( s )) == (char **)NULL )
        return( -1 );

    if( (x= getctl("URESSIZ", p)) != NULL )
        URESSIZ = atoi(x);
    if( (x= getctl("FSIZE", p)) != NULL )
        FSIZE = atoi(x);
    if( (x= getctl("CBKEY", p)) != NULL )
        CBKEY = atoi(x);
    if( (x= getctl("HOMEREC", p)) != NULL )
        if( *x == 0 ) HOMEREC = 1;
        else HOMEREC = atoi(x);
    if( (x= getctl("RECNAME", p)) != NULL )
        RECNAME = x;
    if( (x= getctl("MAXCHAN", p)) != NULL )
        MAXCHAN = atoi(x);
    if( (x= getctl("LIMCHAN", p)) != NULL )
        if( *x == 0 ) LIMCHAN = 1;
        else LIMCHAN = atoi(x);
    if( (x= getctl("LIMBASE", p)) != NULL )
        LIMBASE = atoi(x);
    if( (x= getctl("LIMIT", p)) != NULL )
        LIMIT = atoi(x);
    if( (x= getctl("MONCHAN", p)) != NULL )
        MONCHAN = atoi(x);
    if( (x= getctl("MONPAY", p)) != NULL )
        if( *x == 0 ) MONPAY = 1;
        else MONPAY = atoi(x);
    if( (x= getctl("MONBASE", p)) != NULL )
        MONBASE = atoi(x);
    if( (x= getctl("MONLIM", p)) != NULL )
        MONLIM = atoi(x);
    if( (x= getctl("LFCHAR", p)) != NULL )
        LFCHAR = x;
    if( (x= getctl("MAXNL", p)) != NULL )
        MAXNL = atoi(x);
    if( (x= getctl("MAXMYOUT", p)) != NULL )
        MAXMYOUT = atoi(x);
    if( (x= getctl("MAXPOUT", p)) != NULL )
        MAXPOUT = atoi(x);
    if( (x= getctl("MAXUOUT", p)) != NULL )
        MAXUOUT = atoi(x);
    if( (x= getctl("FAKEOUT", p)) != NULL )
        FAKEOUT = x;
    if( (x= getctl("AFORDEF", p)) != NULL )
        AFORDEF = x;
    if( (x= getctl("DOSHELL", p)) != NULL )
        DOSHELL = x;
    if( (x= getctl("DOFORM", p)) != NULL )
        DOFORM = x;
    if( (x= getctl("DOINACT", p)) != NULL )
        DOINACT = x;
    if( (x= getctl("DOTYPE", p)) != NULL )
        DOTYPE = x;
    if( (x= getctl("DODORK", p)) != NULL )
        DODORK = x;
    if( (x= getctl("PAIDLF", p)) != NULL )
        if( *x == 0 ) PAIDLF = 1;
        else PAIDLF = atoi(x);
    if( (x= getctl("PAIDDC", p)) != NULL )
        if( *x == 0 ) PAIDDC = 1;
        else PAIDDC = atoi(x);
    if( (x= getctl("CBLOGIN", p)) != NULL )
        CBLOGIN = x;
    if( (x= getctl("LOCKEDMSG", p)) != NULL )
        LOCKEDMSG = x;
    if( (x= getctl("ICTLVAL", p)) != NULL )
        ICTLVAL = (int)strtol(x, (char **)NULL, 0);
    if( (x= getctl("INITCHAN", p)) != NULL )
        INITCHAN = atoi(x);
    if( (x= getctl("PINITCHAN", p)) != NULL )
        PINITCHAN = atoi(x);
    if( (x= getctl("UINITCHAN", p)) != NULL )
        UINITCHAN = atoi(x);
    if( (x= getctl("RCLOW", p)) != NULL )
        RCLOW = atoi(x);
    if( (x= getctl("RCHIGH", p)) != NULL )
        RCHIGH = atoi(x);
    if( (x= getctl("FORCEH", p)) != NULL )
        if( *x == 0 ) FORCEH = 1;
        else FORCEH= atoi(x);
    if( (x= getctl("FORCEL", p)) != NULL )
        if( *x == 0 ) FORCEL = 1;
        else FORCEL = atoi(x);
    if( (x= getctl("AFORCEH", p)) != NULL )
        if( *x == 0 ) AFORCEH = 1;
        else AFORCEH = atoi(x);
    if( (x= getctl("AFORCEL", p)) != NULL )
        if( *x == 0 ) AFORCEL = 1;
        else AFORCEL = atoi(x);
    if( (x= getctl("CMDFILE", p)) != NULL )
        CMDFILE = x;
    if( (x= getctl("OUTFILE", p)) != NULL )
        OUTFILE = x;
    if( (x= getctl("CBDIR", p)) != NULL )
        CBDIR = x;
    if( (x= getctl("RECDIR", p)) != NULL )
        RECDIR = x;
    if( (x= getctl("MSGPBAD", p)) != NULL )
        MSGPBAD = x;
    if( (x= getctl("MSGSBAD", p)) != NULL )
        MSGSBAD = x;
    if( (x= getctl("MSGVBAD", p)) != NULL )
        MSGVBAD = x;
    if( (x= getctl("MSGCBAD", p)) != NULL )
        MSGCBAD = x;
    if( (x= getctl("ENVPAID", p)) != NULL )
        ENVPAID = x;
    if( (x= getctl("ENVSYSOP", p)) != NULL )
        ENVSYSOP = x;
    if( (x= getctl("ENVVIP", p)) != NULL )
        ENVVIP = x;
    if( (x= getctl("ENVCOSYSOP", p)) != NULL )
        ENVCOSYSOP = x;
    if( (x= getctl("ENVVALID", p)) != NULL )
        ENVVALID = x;
    if( (x= getctl("WRITELEV", p)) != NULL )
        WRITELEV = atoi(x);
    if( (x= getctl("STTY0", p)) != NULL )
        if( *x == 0 ) STTY0 = 1;
        else STTY0 = atoi(x);
    if( (x= getctl("DAILY", p)) != NULL )
        DAILY = x;
    if( (x= getctl("KPERCENT", p)) != NULL )
        KPERCENT = atoi(x);
    if( (x= getctl("COACT", p)) != NULL )
        if( *x == 0 ) COACT = 1;
        else COACT = atoi(x);
    if( (x= getctl("COPASSX", p)) != NULL )
        if( *x == 0 ) COPASSX = 1;
        else COPASSX = atoi(x);
    if( (x= getctl("COONE", p)) != NULL )
        if( *x == 0 ) COONE = 1;
        else COONE = atoi(x);
    if( (x= getctl("COPPA", p)) != NULL )
        if( *x == 0 ) COPPA = 1;
        else COPPA = atoi(x);
    if( (x= getctl("COLOCKOV", p)) != NULL )
        if( *x == 0 ) COLOCKOV = 1;
        else COLOCKOV = atoi(x);
    if( (x= getctl("CONEGV", p)) != NULL )
        if( *x == 0 ) CONEGV = 1;
        else CONEGV = atoi(x);
    if( (x= getctl("COKGLOB", p)) != NULL )
        if( *x == 0 ) COKGLOB = 1;
        else COKGLOB = atoi(x);
    if( (x= getctl("COKINACT", p)) != NULL )
        if( *x == 0 ) COKINACT = 1;
        else COKINACT = atoi(x);
    if( (x= getctl("COKPAID", p)) != NULL )
        if( *x == 0 ) COKPAID = 1;
        else COKPAID = atoi(x);
    if( (x= getctl("LOGCOPAS", p)) != NULL )
        if( *x == 0 ) LOGCOPAS = 1;
        else LOGCOPAS = atoi(x);
    if( (x= getctl("NUMSLOTS", p)) != NULL )
        NUMSLOTS = atoi(x);
    if( (x= getctl("QUOTA", p)) != NULL )
        QUOTA = atoi(x);
    if( (x= getctl("TSINTERVAL", p)) != NULL )
        TSINTERVAL = atoi(x);
    if( (x= getctl("CBINIT", p)) != NULL )
        CBINIT = x;
    if( (x= getctl("DEBUG", p)) != NULL )
        if( *x == 0 ) DEBUG = 1;
        else DEBUG = atoi(x);
    if( (x= getctl("UNIXWHO", p)) != NULL )
        UNIXWHO = x;

    return( loadcmds( CMDFILE ) );
}

char *basepath()
{
#ifdef SKYNET
    return( m_skynetpath() );
#else
    return( DEFBASE );
#endif
}
