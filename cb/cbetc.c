#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <ctype.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>
#include "exitcodes.h"
#include "envar.h"
#include "xstring.h"
#include "osdefs.h"
#include "cb.h"
#include "cbetc.h"
#include "cbcfg.h"

/*
 *  Skynet Unix CB
 *
 *  Miscellaneous functions
 *
 *  Copyright (C) 1988 CPEX Development
 *  All Rights Reserved.
 *  Author: Eric Pederson
 *
 */

extern void xlxm();
extern void distrib();
extern char *aline();
extern void sndfix();
extern char *getenv();
extern int Pu(int sid);
extern int Vu(int sid);
extern int gmatch( int gslot, int glob );

int checksq(int source, int target);

extern char *ctime();

char *ansstr( int aslot );
STATIC int nextpos();

/**********************************************************************/

int xmsg( chan, msg, toslot, toal )
int chan;
char *msg;
int toslot;
char *toal;
{
    /*
     *  Send a message
     *
     *  Returns non-zero on fail
     */

    struct dmsg dm;
    struct cbmsg mbuf;
    int pos, doxl;
    char c;

    pos = nextpos();
    dm.fslot = slot;
    strcpy(dm.from,MYREC.handle);
    strcpy(dm.ufrom,MYREC.userid);
    dm.chan = chan;
    dm.opts = MYREC.opts;
    if( !(MYREC.opts & OP_ACTIVE) &&
            (chan <= MAXCHAN || chan == MSG_PRIV) ){
        printf("Sure? ");
        c = getchar();
        if( toupper(c) == 'Y' )
            puts("Yes");
        else {
            puts("No");
            return(1);
        }
    }
    strcpy(dm.to,toal);
    doxl = ( !(MYREC.opts & OP_LFLOCK) && chan != MSG_PA && chan != MSG_PPA
        && chan != MSG_CPA && chan != MSG_NCPA && chan != MSG_RPA );
/*
    doxl = doxl || (MYREC.opts & OP_JIVE);
*/
    if( doxl )
        xlxm(msg);
    dm.tslot = toslot;
    strcpy(dm.msg,msg);
    lseek(out,(long)(pos*sizeof(struct dmsg)),0);
    write(out,&dm,sizeof(struct dmsg));
    distrib( pos, toslot, chan );
    return(0);
}


STATIC void xlxm( s )
char *s;
{
    /*
     *  Translate message
     *
     *  Changes LFCHARs to newlines
     *  Also handles jive CB, if applicable
     *
     */

    int nl;
    char xj[MSGLEN+40];

    if( MYREC.opts & OP_JIVE ){
        void do_jive(char *in, char *out, int maxlen);
        do_jive(s, xj, MSGLEN);
        strcpy(s, xj);
    }

    if( !PAIDLF || (MYREC.opts & OP_PAID) )
        for( nl= 0; *s != 0; s++ )
            if( *s == MYREC.lfchar )
                if( ++nl <= MAXNL )
                    *s = '\n';
}


int isval( s )
char *s;
{
    /*
     *  Is this handle valid?
     *  Returns zero on fail
     *
     */

    if( (strchr(INVALS,*s) != NULL) ||
            strpbrk(s,GINVALS) != NULL )
        return(0);
    return(-1);
}


int lookup( s, t )
char *s;
char *t;
{
    /*
     *  Lookup handle, check validity, etc.
     *
     *  Returns slot or a status
     *  S_INVALID - invalid handle
     *  S_NOBODY - not found
     *  S_PAID, S_ALL, S_NOPAY - as appropriate
     *  or a slot number
     *  Returns an actual handle in t if t != NULL
     *
     */

    char *x;
    int check, i;
    static char gtos[] = { "*$#@%" };
    static int gtoch[] = { S_ALL, S_PAID, S_NOPAY, S_SYSOP, S_COSYSOP };

    if( *s == 0 )
        return(S_ALL);

    if( slot >= 0 )
        if( MYREC.opts & (OP_PAID|OP_SYSOP) )
            if( (x= strchr(gtos,*s)) != NULL )
                return( gtoch[(int)x - (int)gtos] );

    check = S_NOBODY;
    if( isdigstr( s ) ){
        check = atoi( s );
        if( check < 0 || check > ulsize-1 )
            check = S_NOBODY;
    } else
        for( i= 0; i < ulsize; i++ ){
            if( !stricmp(s,ulog[i].handle) ||
                !stricmp(s,ulog[i].userid) )
                    if( isact(slot,i) ){
                        check = i;
                    }
        }
    if( check == S_NOBODY )
        return(check);
    if( !isact(slot,check) )
        return(S_NOBODY);
    if( t != NULL )
        strcpy(t,ulog[check].handle);
    return(check);
}


void msglog( mslot, mtype )
int mslot;
int mtype;
{
    /*
     *  Send a login/logout message
     *
     */

    struct cbmsg mbuf;
    int chan;
    char s[80], *x;

    if( !(MYREC.opts & OP_ACTIVE) && !COACT )
        return;

    chan = (ulog[mslot].opts & OP_ACTIVE) ? MSG_PA : MSG_PPA;
    switch(done){
        case 0:
        case SIGFPE:
            if( mtype == CB_ACTIVATE )
                x = "Logged on";
            else x = "Logged off";
            break;
        case SIGKILL:
        case SIGQUIT:
            x = "Kicked off";
            break;
        case SIGHUP:
            x = "Disconnected";
            break;
        case SIGTERM:
            x = "Time expired";
            break;
        case SIGUSR1:
            x = "Auto logoff";
            break;
        default:
            return;
    }
    sprintf(s,"#%d %s: %s/%s",mslot,x,ulog[mslot].userid,
            ulog[mslot].handle);
    xmsg(chan,s,0,"");
}


void alist()
{
    /*
     *  List active users
     *
     */

    int i;
    char *s;

    printf("Active users:\n");
    for( i= 0; i < ulsize; i++ ){
        if( !isact(slot,i) )
            continue;
        s= aline( i );
        if( MYREC.opts & OP_ANSI )
            printf( "%s", ansstr( i ) );
        printf("%s",s);
        if( MYREC.opts & OP_ANSI )
            printf( "%s", ANSREG );
        putchar( '\n' );
    }
    putchar('\n');
}


int isact( asking, who )
int asking;
int who;
{
    /*
     *  Is "who" active?  "Asking" wants to know.
     *  If asking is < 0, consider it to be a SYSOP.
     *
     */

    if( asking >= 0 )
        if( !( ulog[asking].opts & OP_SYSOP ) &&
            !((ulog[asking].opts & OP_COSYSOP) && COACT) )
                return( (ulog[who].opts & OP_ACTIVE)
                    && (ulog[who].opts & OP_USED) );
    return( ulog[who].opts & OP_USED );
}


struct sqrec *getsq( source, target )
int source;
int target;
{
    /*
     *  Return a pointer to the shared memory squlech record
     *  requested
     *
     */

/*
    printf("target > ul+ng = %d, target=%d, source=%d\n",
        (target > ulsize+S_NUMGLOB), target, source);
*/
    return( sqlog + ((source * ulsize) + target) );
}


void dosq( source, target, stype )
int source;
int target;
int stype;
{
    /*
     *  Change a squelch record
     *
     */

    struct sqrec *sq;

    sq = getsq( source, target );
    if( stype & SQ_SRESET ){
        sq->sqtype &= ~SQ_STYPE;
        if( DEBUG > 0 && !logging )
            printf("dosq: sreset sq=0x%lx sqtype=0%o\n",(unsigned long)sq,sq->sqtype);
    }
    if( stype & SQ_KRESET ){
        sq->sqtype &= ~SQ_KILL;
        if( DEBUG > 0 && !logging )
            printf("dosq: kreset sq=0x%lx sqtype=0%o\n",(unsigned long)sq,sq->sqtype);
    }
    if( !(stype & (SQ_SRESET|SQ_KRESET)) ){
        if( stype == SQ_KILL )
            sq->sqtype |= stype;
        else {
            if( DEBUG > 0 ){
                printf("dosq: sq=0x%lx sqtype=0%o stype=0%o\n",(unsigned long)sq,sq->sqtype,stype);
            }
            sq->sqtype = stype;
        }
    }
    return;
}


int chhand( s )
char *s;
{
    /*
     *  Physically change handle
     *
     *  Returns non-zero on fail
     */

    int i;

    Pu( cbsem );
    if( (i= lookup(s,NULL)) < 0 )
        i = 0;
    else
        if( i == slot )
            i = 0;
        else
            i = -1;
    if( i == 0 )
        strcpy(MYREC.handle,s);
    Vu( cbsem );

    return(i);
}


STATIC int nextpos()
{
    /*
     *  Test & Set next file position
     *
     */

    int i;

    Pu( cbsem );
    i = (*pnext + 1) % FSIZE;
    *pnext = i;
    Vu( cbsem );

    return(i);
}


STATIC void distrib( pos, priv, chan )
int pos;
int priv;
int chan;
{
    /*
     *  Distributes a new message
     *  Channel or PRIV/PA/PPA in chan
     *  Rfpos in pos
     *  Private destination in priv
     *
     */

    int i, j;
    struct cbmsg mbuf;

    M_R.cbtype = CB_NEWMSG;
    for( i= 0; i < ulsize; i++ ){
        if( !(ulog[i].opts & OP_USED) )
            continue;
        if( (chan <= MAXCHAN && chan == ulog[i].chan) ||
            ( MONCHAN != 0 && chan == MONCHAN &&
                ulog[i].chan >= MONBASE &&
                ( MONPAY ? ulog[i].opts & OP_PAID : 1 ) &&
                ulog[i].chan <= MONBASE+MONLIM ) ||
            ( MONCHAN != 0 && chan == MONCHAN &&
                (ulog[i].opts & OP_MONIT)) ||
            (chan <= MAXCHAN &&
                 (ulog[i].opts & OP_SYSOP) &&
                !(ulog[i].opts & OP_SQNON)) ||
            (chan == MSG_PRIV && i == priv) ||
            (chan == MSG_VIP && i == priv) ||
            (chan == MSG_RPA && i == priv) ||
            (chan == MSG_CPA && ulog[i].chan == priv) ||
            (chan == MSG_NCPA && ulog[i].chan != priv) ||
            (chan == MSG_PRIV && !(checksq(i,slot) & SQ_BOTH) &&
                ((priv == S_ALL) ||
                (priv == S_SYSOP &&
                    ulog[i].opts & OP_SYSOP) ||
                (priv == S_COSYSOP &&
                    ulog[i].opts & OP_COSYSOP) ||
                (priv == S_PAID &&
                    ulog[i].opts & OP_PAID) ||
                (priv == S_NOPAY &&
                    !(ulog[i].opts & OP_PAID)))) ||
            (chan == MSG_PA) || (chan == MSG_STATION) ||
            (chan == MSG_PPA &&
                ((ulog[i].opts & OP_SYSOP) ||
                 (COPPA && (ulog[i].opts & OP_COSYSOP)))) ||
            (chan == MSG_PRIV &&
                     (ulog[i].opts & OP_VIP) &&
                    !(ulog[i].opts & OP_SQPILF)) ){
                if( chan <= MAXCHAN || chan == MSG_STATION){
                    j = checksq(slot,i);
                    if( (chan <= MAXCHAN &&
                        (j & SQ_REVERSE)) ||
                        (chan == MSG_STATION &&
                        (j & SQ_BOTH)) )
                            continue;
                    if( chan <= MAXCHAN ){
                        j = checksq(i,slot);
                        if(j & SQ_COMP)
                            continue;
                    }
                }
                if( chan == MSG_STATION &&
                    (ulog[i].opts & OP_SQSTAT) )
                        continue;
                if( chan == MSG_PRIV ){
                    j = checksq(i,slot);
                    if( j & SQ_PASSIVE )
                        continue;
                }
                mbuf.mtype = (long)ulog[i].pid;
                M_R.arg = pos;
                if( msgsnd(mid,&mbuf,L_cbreq,ipcflag) == -1 )
                    sndfix(&mbuf);
        }
    }
}


int issq( source, target )
int source;
int target;
{
    /*  Exactly how are we squelched by slot
     */

    struct sqrec *sq;

    sq = getsq( source, target );
    return( sq->sqtype );
}

int checksq( source, target )
int source;
int target;
{
    /*  Check squelching status with global handling
     */

    int i, sqnow;

    sqnow = issq(source, target);
    for( i= 0; i < S_NUMGLOB; i++ )
        if( gmatch(target, S_ALL+i) )
            sqnow |= issq(source, ulsize+i);
    return( sqnow );
}

void rcvfix()
{
    /*
     *  If we get an error on msgrcv, come here by all means
     *
     */

    int i;

printf("rcvfix..\n");
    if( errno == EIDRM ){
        i = (getpid() == MYREC.pid ? MYREC.cpid : MYREC.pid);
        kill(SIGKILL,i);
        MYREC.opts = 0;
        exit( 0 );
    }
}


void sndfix( pmbuf )
struct cbmsg *pmbuf;
{
    /*  Come here on a msgsnd error
     */

    int go;
    struct cbmsg temp;

    printf("sndkix %d\n",errno);
    Pu( cbsem );
        if( !(go= (*pfixslot == -1)) )
            *pfixslot = slot;
    Vu( cbsem );
    if( go ){
        for( go= 0; msgrcv(mid,&temp,L_cbmsg,0L, IPC_NOWAIT) != -1; )
            go++;
        printf("Ulsize=%d 1st pass=%d\n", ulsize,go);
        for( go= 0; go < ulsize; go++ ){
            if( go == slot )
                continue;
            ulog[go].opts = 0;
            if( ulog[go].pid >= 1 )
                kill(ulog[go].pid, SIGHUP);
            if( ulog[go].cpid >= 1 )
                kill(ulog[go].cpid, SIGHUP);
        }
        for( go= 0; msgrcv(mid,&temp,L_cbmsg,0L, IPC_NOWAIT) != -1; )
            go++;
        printf("second pass=%d\n", go);
        *pfixslot = -1;
    }
    MYREC.opts = 0;
    /* Kill sibling */
    go = (getpid() == MYREC.pid ? MYREC.cpid : MYREC.pid);
    kill(go, SIGHUP);
    exit( 0 );
}

char *aline( i )
int i;
{
    static char s[400], ds[FORMAX];
    char *x;

    memset( s, 0, 400 );

    for( x= MYREC.aformat; *x != 0; x++ ){
        if( *x != '%' )
            s[strlen(s)] = *x;
        else
            switch( *(++x) ){
                case 'e':
                    strcat(s,"\033");
                    break;
                case 'u':
                    strcat(s,ulog[i].userid);
                    break;
                case 'U':
                    sprintf(ds,"%*s",L_cuserid,ulog[i].userid);
                    strcat(s,ds);
                    break;
                case 'd':
                    if( !(ulog[i].opts & OP_ACTIVE) )
                        strcat(s,DOINACT);
                    else
                        strcat(s,ulog[i].doing);
                    break;
                case 'm':
                    break;
                case 'h':
                    strcat(s,ulog[i].handle);
                    break;
                case 's':
                    sprintf(ds,"%d",i);
                    strcat(s,ds);
                    break;
                case 'c':
                    if( (MYREC.opts & OP_SYSOP) ||
#ifdef ACTCHAN
                        (COACT &&
#endif
(
                          (MYREC.opts & OP_COSYSOP)) ||
                        (ulog[i].opts & OP_LIST) ){
                        sprintf(ds,"%d",ulog[i].chan);
                        strcat( s, ds );
                    } else {
                        if( ulog[i].chan == MYREC.chan )
                            strcat( s, "*" );
                        else
                            strcat( s, "?" );
                    }
                    break;
                case 'L':
                    if( ulog[i].opts & OP_LIST )
                        strcat( s, "L" );
                    else
                        strcat( s, " " );
                    break;
                case 'S':
                    sprintf(ds,"%.2d",i);
                    strcat(s,ds);
                    break;
                case 'C':
                    if( (MYREC.opts & OP_SYSOP) ||
#ifdef ACTCHAN
                        (COACT &&
#endif
(
                          (MYREC.opts & OP_COSYSOP)) ||
                        ( ulog[i].opts & OP_LIST) ){
                        sprintf(ds,"%.2d",ulog[i].chan);
                        strcat( s, ds );
                    } else {
                        if( ulog[i].chan == MYREC.chan )
                            strcat( s, "**" );
                        else
                            strcat( s, "??" );
                    }
                    break;
                case '$':
                    if( ulog[i].opts & OP_SYSOP ){
                        strcat(s,"@");
                        break;
                    } else
                    if( ulog[i].opts & OP_COSYSOP ){
                        strcat(s,"%");
                        break;
                    } else
                    if( ulog[i].opts & OP_PAID )
                        strcat(s,"$");
                    else strcat(s," ");
                    break;
                case '#':
                    if( ulog[i].opts & OP_SYSOP ){
                        strcat(s,"@");
                        break;
                    } else
                    if( ulog[i].opts & OP_COSYSOP ){
                        strcat(s,"%");
                        break;
                    } else
                    if( ulog[i].opts & OP_PAID )
                        strcat(s,"$");
                    else strcat(s,"#");
                    break;
                case '[':
                    if( ulog[i].opts & OP_SYSOP ){
                        strcat(s,"<");
                        break;
                    } else
                    if( ulog[i].opts & OP_COSYSOP ){
                        strcat(s,"{");
                        break;
                    } else
                    if( ulog[i].opts & OP_PAID )
                        strcat(s,"[");
                    else strcat(s,"(");
                    break;
                case ']':
                    if( ulog[i].opts & OP_SYSOP ){
                        strcat(s,">");
                        break;
                    } else
                    if( ulog[i].opts & OP_COSYSOP ){
                        strcat(s,"}");
                        break;
                    } else
                    if( ulog[i].opts & OP_PAID )
                        strcat(s,"]");
                    else strcat(s,")");
                    break;
                case '_':
                    strcat(s,"\n");
                    break;
                default:
                    s[strlen(s)] = *x;
                    break;
            }
    }
    return( s );
}

#define COMBOS 15

char *ansstr( aslot )
int aslot;
{
    /*  Generates a ANSI SGR string for aslot
     */

    static char ds[80];
    static int fores[COMBOS] = { 31, 32, 33, 34, 35, 36,
                    36, 32, 37, 30, 30,
                    30, 40, 30, 34 };
    static int backs[COMBOS] = { 40, 40, 40, 40, 40, 40,
                    44, 44, 44, 46, 45,
                    42, 43, 47, 47 };

    if( aslot > COMBOS )
        aslot %= COMBOS;
    sprintf(ds,"\033[%d;%dm",backs[aslot],fores[aslot]);
    return( ds );
}

int rdonly()
{
    /*  Am I a readonly type of guy?

        Returns non-zero if read only
     */

    char *x;

    if( MYREC.opts & OP_RDOVER )
        return( 0 );

    if( skynet && WRITELEV != -1 )
        if( (x= getenv( SE_LEVEL)) != NULL )
            if( atoi(x) < WRITELEV )
                return( -1 );

    return( (!(MYREC.opts & OP_PAID) && (*pcbflag & CF_RDONLY)) ||
        (!(MYREC.opts & OP_VALID) && (*pcbflag & CF_VRDONLY)) );
}

void logpa( s )
char *s;
{
    /*  Log 's' to the CB log file for posterity
    */

    char fn[80], *ds;
    FILE *f;
    int um;
    long tim;

    if( !LOGCOPAS )
        return;
    strcpy(fn, cbfn( LOGFILE ));

    um = umask( CBUMASK );
    if( (f= fopen(fn, "a")) == NULL ){
        printf("Could not open log file\n");
        perror( fn );
    } else {
        time( &tim );
        ds = ctime( &tim );
        *(ds+19) = 0;
        fprintf(f, "%s %s\n", ds+4, s);
        fclose( f );
    }
    umask( um );
}
