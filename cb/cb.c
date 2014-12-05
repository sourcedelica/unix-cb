#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>
#include <string.h>
#include <setjmp.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <unistd.h>
#include "exitcodes.h"
#include "mflags.h"
#include "mstf.h"
#include "muserinf.h"
#include "osdefs.h"
#include "alias.h"
#include "sds.h"
#define CCB
#include "cb.h"
#include "cbetc.h"
#include "cbcfg.h"

#if HAVE_LIBCURL
#include <curl/curl.h>
#endif

/*
 *  Unix-CB
 *
 *  Copyright (C) 1988 CPEX Development
 *  All Rights Reserved
 *  Author: Eric Pederson
 *
 */

extern void cbinp();
extern void cbout();
extern void catchit();
extern void exclean();
extern void infinish();
extern void logout();
extern void logclean();
static void loadops(int slot);
static int stty0();
STATIC int rdquota();
STATIC void wrquota();
STATIC int login( int where );
int listsig(char *t);
int Pu(int sid);
int Vu(int sid);
void raw();
int readdefault( char *pr, char *x, int lx, char *def );
void fixdorks();
STATIC int rdrec( struct ulrec *ptemp );
int stricmp(char *s1, char *s2);

int slot = -1;
int mid;
int opts;
long cbpid;
int out;
int wid;
int ulsize;
int done = 0;
char *myformat;
struct ulrec *ulog;
int cbsem;
int *pnext, *pcbflag, *pfixslot;
struct sqrec *sqlog;
int child = 0;
int ipcflag = IPC_NOWAIT;
int sendflag = 0;
int fixsid;
TERMIO_OR_TERMIOS oldterm;
int skynet = 0;
char *cbdoing;
char *cbjs, *cbjt;
int cbjl;
int logging = 1;

static struct cbmsg mbuf;
static jmp_buf env;

/* These are the "sticky" flag bits that stay with you after you log off */

static char *opta[] = {
    "CHLOCK", "SQLOCK", "LFLOCK", "PMLOCK", "SQBEEP", "SQLF",
    "MONIT", "ANSI", "SQSTAT", "PABEEP", "UGLYLINK", NULL };
static int opto[] = {
    OP_CHLOCK, OP_SQLOCK, OP_LFLOCK, OP_PMLOCK, OP_SQBEEP, OP_SQLF,
    OP_MONIT, OP_ANSI, OP_SQSTAT, OP_PABEEP, OP_UGLYLINK, 0 };

extern char *m_doing();

/***********************************************************************/

main(argc, argv)
int argc;
char **argv;
{
    char s[80], *x, *sbase, *cfname = NULL;
    int i, j, sid, errflg = 0, bact;
    long sflags, m_flags();
    void acatch();
    extern char *optarg;
    extern int optind;

    muserinf();
    while( (sid= getopt(argc, argv, "y:")) != -1 )
        switch( sid ){
            case 'y':
                cfname = optarg;
                break;
            case '?':
                errflg++;
                break;
        }
    if( optind < argc ) errflg++;
    if( errflg ){
        printf("Usage: %s [-y cfname]\n",argv[0]);
        exit(EXITOK);
    }
    if( cbcfg(cfname) ){
        printf("Error reading configuration file %s\n",
            cfname == NULL ? CBCONFIG : cfname );
        exit(EXITOK);
    }
    ulsize = NUMSLOTS;

    if( (mid= msgget(CBKEY,0)) == -1 ){
        if (errno == ENOENT){
            system(CBINIT);
            if( (mid= msgget(CBKEY,0)) == -1 ){
                perror("CB is unavailable");
                exit(EXITOK);
            }
        } else {
            perror("CB is unavailable");
            exit(EXITOK);
        }
    }
    if( (out= open(cbfn(OUTFILE),O_RDWR|O_CREAT,CBMODE)) == -1 ){
        perror("Cannot open out file");
        exit(EXITOK);
    }
    if( (sid= shmget(CBKEY,0,0)) == -1 ){
        perror("Attaching shared memory");
        exit(EXITOK);
    }
    i = 0;
    sbase = shmat( sid, NULL, 0 );
    pnext = (int *)sbase;
/*
    ulsize = *pnext;
    i += sizeof(int);
*/
    pnext = (int *)(sbase + i);
    i += sizeof(int);
    pcbflag = (int *)(sbase + i);
    i += sizeof(int);
    pfixslot = (int *)(sbase + i);
    i += sizeof(int);
    ulog = (struct ulrec *)(sbase + i);
    i += sizeof(struct ulrec)*ulsize;
    sqlog = (struct sqrec *)(sbase + i);

    if( (cbsem= semget(CBKEY,0,0)) == -1 ){
        perror("semget cbsem");
        exit(EXITOK);
    }

#ifdef SKYNET
    /* Attach to skynet menu shared memory */
    m_at();
#endif

    if( setjmp(env) )
        exclean();
    signal(SIGHUP,catchit);
    signal(SIGTERM,catchit);
    cbpid = (long)getpid();

    if( (x= getenv("SLOTNUM")) != NULL ){
        slot = atoi(x);
        skynet++;
    }
    if( (slot= login(slot)) == -1 ){
        printf("CB is full\n");
        exit(EXITOK);
    }
    /* When finished logging in, logging is reset */
    logging = 0;

    if( MYREC.opts & OP_ANSI )
        printf( "%s", ANSREG );
    printf("%s\n", cfgxstr(CBLOGIN));
    printf("Use /? for help\n\n");

    /* Update options based on environ */
    if( (x= getenv("MAXLLEN")) != NULL ) wid = atoi(x);
    else wid = 80;
#ifdef SKYNET
    if( skynet ){
        cbdoing = strdup(m_doing( slot ));
        MYREC.opts |= OP_SKYNET;
    }
#endif
    loadops( slot );
    saveterm(&oldterm);
    raw();
    signal(SIGQUIT,SIG_IGN);
    signal(SIGINT,SIG_IGN);

    if( !( (MYREC.opts & OP_SYSOP) ||
            (COLOCKOV && (MYREC.opts & OP_COSYSOP)) ) )
        if( *pcbflag & CF_LOCKED ){
            printf("%s\n",LOCKEDMSG);
            logout();
            restterm(&oldterm);
            if( STTY0 ) stty0();
            exit(EXITOK);
        }
    if( MYREC.opts & OP_SYSOP || (COACT && (MYREC.opts & OP_COSYSOP)) ){
        printf("Be active? ");
        i = getchar();
        if( (i= toupper(i)) == 'Q'){
            puts("Quit");
            logout();
            restterm(&oldterm);
            exit(EXITOK);
        }
        i = ( i == 'Y' );
        puts( i ? "Yes" : "No" );
    } else
        i = 1;
    bact = i;

    MYREC.chan = ( MYREC.opts & OP_PAID ? PINITCHAN : INITCHAN );
    if( MYREC.opts & OP_UGLYLINK ) MYREC.chan = UINITCHAN;
    if( AFORCEL || ( !(MYREC.opts & OP_PAID) && FORCEL ) )
        MYREC.opts |= OP_LIST;
    if( AFORCEH || ( !(MYREC.opts & OP_PAID) && FORCEH ) )
        strcpy( MYREC.handle, galias );

    /* Log in handle */
    i = ( *MYREC.handle == 0 );
    for( j= 1; j; i = 1 ){
        if( i ){
            if( !readdefault("Handle to use: ",s,L_cbalias,"") ){
                logout();
                while( msgrcv(mid,&mbuf,L_cbmsg,cbpid,
                    IPC_NOWAIT) != -1 )
                    ;
                if( STTY0 ) stty0();
                restterm(&oldterm);
                exit(EXITOK);
            }
        } else
            strcpy(s,MYREC.handle);
        if( !isval(s) ){
            printf("Invalid character in handle\n");
            continue;
        }
        j = chhand(s);
    }
    /* Fix anyone with your handle */
    fixdorks();

    while( msgrcv(mid, &mbuf, L_cbmsg, cbpid, IPC_NOWAIT) != -1 )
        ;
    if( DAILY != NULL )
        listsig( DAILY );
    if( bact )
        MYREC.opts |= OP_ACTIVE;
    else
#ifdef SKYNET
        if( skynet )
            m_chdoing( slot, FAKEOUT );
#endif
    alist();
    msglog(slot,CB_ACTIVATE);

    if ((child = fork()) != 0) {
        MYREC.cpid = child;
        cbout();
    } else
        cbinp();

    signal(SIGHUP,SIG_IGN);
    signal(SIGTERM,SIG_IGN);
    exclean();
}


STATIC void catchit( arg )
int arg;
{
    /*
     *  Handle hangups and terminates
     *
     */

    signal(SIGHUP,SIG_IGN);
    signal(SIGTERM,SIG_IGN);

    done = arg;
    longjmp(env,-1);
}


STATIC void exclean()
{
    /*
     *  Cleanup and exit CB
     *
     */

    kill(child,SIGHUP);
    while( msgrcv(mid, &mbuf, L_cbmsg, cbpid, IPC_NOWAIT) != -1 )
        ;
    while( msgrcv(mid, &mbuf, L_cbmsg, (long)child, IPC_NOWAIT) != -1 )
        ;
#ifndef SILENT
    /* Send logoff message */
    if( !sendflag )
        msglog(slot,CB_DEACT);
#endif
    MYREC.opts &= ~OP_ACTIVE;
    logout();
    if( done == SIGTERM )
        puts("\n-- Time expired\n");
    else if( done == SIGQUIT )
        done = 0;
    while( msgrcv(mid, &mbuf, L_cbmsg, (long)child, IPC_NOWAIT) != -1 )
        ;
    while( msgrcv(mid, &mbuf, L_cbmsg, cbpid, IPC_NOWAIT) != -1 )
        ;
    if( STTY0 )
        stty0();
    restterm(&oldterm);
    exit( done != 0 ? EXITKILL : EXITOK );
}


STATIC void acatch( arg )
int arg;
{
    /*
     *  We get here if the server times out initially
     */

    puts("Server not responding, CB is down");
    exit(EXITOK);
}


STATIC int login( where )
int where;
{
    /*
     *  Adds user's CB record to the incore userlog
     *
     *  Input:  where designates the slot the requestor wants.
     *      If where is -1, login will assign one
     *
     *  Output: Passes back the assigned slot
     *
     */

    int i;
    struct ulrec temp;

    if( rdrec(&temp) )
        return( -1 );

    Pu( cbsem );

    /* Is slot requested available? */
    if( where >= 0 && where < ulsize )
        if( !(ulog[where].opts & OP_USED) ){
            infinish(where,&temp);
            Vu( cbsem );
            return( where );
        }

    /* Can we find a slot in what's already allocated? */
    for( i= ulsize-1; i > URESSIZ-1; i-- ){
        if( !(ulog[i].opts & OP_USED) ){
            infinish(i,&temp);
            Vu( cbsem );
            return( i );
        }
    }

    /* No can do. */
    Vu( cbsem );
    return( -1 );
}


#define R_HANDLE    "HANDLE"
#define R_FORMAT    "FORMAT"
#define R_LFCHAR    "LFCHAR"
#define R_AFORMAT   "AFORMAT"
#define R_LASTP     "LASTP"
#define R_LASTRP    "LASTRP"

sds get_user_record_path(char *userid)
{
    if (HOMEREC) {
        return sdscat(sdscat(sdsnew(homeof(userid)), PATHSEP), RECNAME);
    } else {
        return sdsnew(cbrfn(userid));
    }
}

#if HAVE_LIBCURL
sds get_user_record_url(char *userid)
{
    return sdscat(sdscat(sdsnew(API_URL), "/users/"), userid);
}

static size_t write_callback(void *contents, size_t size, size_t nmemb, void *userp)
{
    size_t realsize = size*nmemb;
    sds *text = (sds*)userp;
    *text = sdscatlen(*text, contents, realsize);
    return realsize;
}
#endif

static sds read_user_record_text(char *username)
{
    sds path, text;
    FILE *fp;
    size_t len;

    #if HAVE_LIBCURL
    CURL *curl;
    CURLcode res;
    sds url;
    if (API_URL) {
        curl = curl_easy_init();
        if (!curl) {
            return NULL;
        }
        url = get_user_record_url(username);
        text = sdsempty();
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&text);
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "Unix-CB/1.0");
        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
        if (res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
            return NULL;
        }
        sdsfree(url);
        return text;
    }
    #endif

    path = get_user_record_path(username);
    if ((fp = fopen(path, "r")) == NULL) {
        return NULL;
    } else {
        fseek(fp, 0L, SEEK_END);
        len = ftell(fp);
        rewind(fp);
        text = sdsnewlen(NULL, len);
        fread(text, len, 1, fp);
        fclose(fp);
        return text;
    }
}

#if HAVE_LIBCURL
struct curl_read_data
{
    char *data;
    size_t len;
};

static size_t read_callback(void *contents, size_t size, size_t nmemb, void *userp)
{
    struct curl_read_data *read_data = (struct curl_read_data*)userp;
    size_t curl_size = nmemb * size;
    size_t to_copy = (read_data->len < curl_size) ? read_data->len : curl_size;
    memcpy(contents, read_data->data, to_copy);
    read_data->len -= to_copy;
    read_data->data += to_copy;
    return to_copy;
}

static size_t dummy_write_callback(void *contents, size_t size, size_t nmemb, void *userp)
{
    size_t realsize = size*nmemb;
    return realsize;
}
#endif

static int write_user_record_text(char *username, char *text)
{
    sds path;
    FILE *fp;
    int um;
    size_t len = strlen(text);

    #if HAVE_LIBCURL
    struct curl_read_data read_data;
    CURL *curl;
    CURLcode res;
    sds url;
    if (API_URL) {
        curl = curl_easy_init();
        if (!curl) {
            return -1;
        }
        url = get_user_record_url(username);
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_PUT, 1L);
        curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
        curl_easy_setopt(curl, CURLOPT_READFUNCTION, read_callback);
        curl_easy_setopt(curl, CURLOPT_READDATA, (void *)&read_data);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, dummy_write_callback);
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "Unix-CB/1.0");
        read_data.data = text;
        read_data.len = len;
        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
        if (res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
            return -1;
        }
        sdsfree(url);
        return 0;
    }
    #endif

    path = get_user_record_path(username);
    um = umask( CBUMASK );
    fp = fopen(path, "w+");
    sdsfree(path);
    if (fp == NULL) {
        umask(um);
        return -1;
    } else {
        fwrite(text, len, 1, fp);
        fclose(fp);
        wrquota();
        umask(um);
        return 0;
    }
}

STATIC int rdrec( ptemp )
struct ulrec *ptemp;
{
    /*
     *  Reads user record or loads default if not found
     *
     *  Returns non-zero on fail
     *
     */

    char *x;
    int i, numlines, lineno;
    sds text, *lines;

    ptemp->sqs = SQNULL;
    ptemp->kvotes = 0;
    ptemp->chan = 1;
    ptemp->opts = OP_SQNON | OP_SQPILF;
    ptemp->lastp = ptemp->rlastp = S_NOBODY;
    strcpy(ptemp->userid,guserid);
    ptemp->pid = cbpid;
    ptemp->lfchar = *LFCHAR;
    *ptemp->handle = 0;
    *ptemp->doing = 0;
    strcpy( ptemp->format, FORDEF);
    strcpy( ptemp->aformat, AFORDEF );
    ptemp->dcreq = -1;
    ptemp->towho = S_NOBODY;
    ptemp->qremain = rdquota();

    strncpy(ptemp->ttyname, ttyname(0), L_ttyname-1);
    ptemp->ttyname[L_ttyname-1] = '\0';

    text = read_user_record_text(guserid);
    if (text == NULL) {
        return (0);
    }

    lines = sdssplitlen(text, sdslen(text), "\n", 1, &numlines);
    for (lineno=0; lineno<numlines; lineno++) {
        sds s = lines[lineno];
        if( strtok(s,"=\n") != NULL ) {
            if( !strcmp(s,R_HANDLE) ){
                x = strtok(NULL,"\n");
                if( x != NULL && *x != 0 )
                    strcpy(ptemp->handle,x);
            } else if( !strcmp(s,R_LFCHAR) )
                ptemp->lfchar = *strtok(NULL,"\n");
            else if( !strcmp(s,R_FORMAT) ){
                x = strtok(NULL,"\n");
                memset(ptemp->format,0,FORMAX);
                strncpy(ptemp->format,x,FORMAX-1);
            } else if( !strcmp(s,R_AFORMAT) ){
                x = strtok(NULL,"\n");
                memset(ptemp->aformat,0,FORMAX);
                strncpy(ptemp->aformat,x,FORMAX-1);
            } else if( !strcmp( s, R_LASTP ) ){
                x = strtok( NULL, "\n" );
                ptemp->lastp = lookup( x, NULL );
            } else if( !strcmp( s, R_LASTRP ) ){
                x = strtok( NULL, "\n" );
                ptemp->rlastp = lookup( x, NULL );
            } else {
                for( i= 0; opta[i] != NULL; i++ ) {
                    if( !strcmp(opta[i],s) ) {
                        ptemp->opts |= opto[i];
                    }
                }
            }
        }
    }
    sdsfreesplitres(lines, numlines);
    sdsfree(text);

    return(0);
}


STATIC int rdquota()
{
    /*  Read quota file to get current time quota for /v
    */

    FILE *f;
    char s[80];

    if( (f= fopen(cbqfn( guserid ), "r")) == NULL )
        return( QUOTA );

    fgets(s, 80, f);
    fclose( f );
    return( atoi( s ) );
}


STATIC void wrrec( slot )
int slot;
{
    /*
     *  Writes user record file
     *
     */
    sds s;
    int i;

    if( slot < 0 || slot >= ulsize )
        return;

    s = sdsempty();
    s = sdscatprintf(s,"%s=%s\n",R_HANDLE,ulog[slot].handle);
    s = sdscatprintf(s,"%s=%s\n",R_FORMAT,ulog[slot].format);
    s = sdscatprintf(s,"%s=%s\n",R_AFORMAT,ulog[slot].aformat);
    s = sdscatprintf(s,"%s=%c\n",R_LFCHAR,ulog[slot].lfchar);
    if( ulog[slot].lastp != S_NOBODY )
        s = sdscatprintf(s, "%s=%s\n", R_LASTP, ulog[ulog[slot].lastp].userid);
    if( ulog[slot].rlastp != S_NOBODY )
        s = sdscatprintf(s, "%s=%s\n", R_LASTRP, ulog[ulog[slot].rlastp].userid);
    for( i=0; opta[i] != NULL; i++ )
        if( ulog[slot].opts & opto[i] )
            s = sdscatprintf(s,"%s\n",opta[i]);

    write_user_record_text(ulog[slot].userid, s);
    sdsfree(s);
}


STATIC void wrquota()
{
    /*  Write quota file from qremain if needed
    */

    FILE *f;

    if( !(MYREC.opts & OP_COSYSOP) )
        return;

    if( (f= fopen(cbqfn( guserid ), "w+")) == NULL ){
        printf("Error creating quota file\n");
        perror( cbqfn( guserid ) );
        return;
    }

    fprintf(f, "%d\n", MYREC.qremain);
    fclose( f );
    return;
}


STATIC void infinish( where, ptemp )
int where;
struct ulrec *ptemp;
{
    /*
     *  Load user rec into user log
     *
     *  (void)
     *
     */

    int i;

    if( strcmp(ulog[where].userid,ptemp->userid) ){
        /* I wasn't the last person on this slot */
        logclean(where);
    }
    memcpy( ulog+where, ptemp, sizeof(struct ulrec) );
#ifndef SILENT
    ulog[where].opts |= OP_USED;
#endif
    return;
}


STATIC void logout()
{
    /*
     *  Log user out and mark space as free
     *  Write out record
     *  Clean pending messages
     *
     */

    int i;
    struct cbmsg mtemp;

    ulog[slot].opts &= ~OP_USED;
    while( msgrcv(mid,&mtemp,L_cbmsg,
            (long)ulog[slot].pid,IPC_NOWAIT) != -1 )
        ;
    wrrec(slot);
}



STATIC void logclean( slot )
int slot;
{
    /*  Update other's squelching, etc. on new activation

        This should be called only when a new userid is logging
        into this slot
     */

    int i;

    if( slot < 0 || slot >= ulsize )
        return;
    for( i= 0; i < ulsize; i++ ){
        if( ulog[i].lastp == slot )
            ulog[i].lastp = S_NOBODY;
        dosq( i, slot, SQ_SRESET|SQ_KRESET );
    }
    for( i= 0; i < ulsize+S_NUMGLOB; i++ )
        dosq( slot, i, SQ_SRESET|SQ_KRESET );
}

static void loadops( slot )
int slot;
{
    /*  Loads the "opts" flag word with the correct
        options.  If on skynet, checks incore flags,
        otherwise checks environment vars
    */

    int flags, sflags;

#ifdef SKYNET
    if( skynet ){
        flags = m_flags( slot );
        sflags = m_stf( slot );
        if( sflags & STF_PAID )
            MYREC.opts |= OP_PAID;
        if( flags & F_SYSOP )
            MYREC.opts |= OP_SYSOP;
        if( flags & F_COSYSOP )
            MYREC.opts |= OP_COSYSOP;
        if( flags & F_VALIDATED )
            MYREC.opts |= OP_VALID;
    }
#endif
    if( ENVSYSOP != NULL )
        if( getenv( ENVSYSOP ) != NULL )
            MYREC.opts |= OP_SYSOP;
    if( ENVPAID != NULL )
        if( getenv( ENVPAID ) != NULL )
            MYREC.opts |= OP_PAID;
    if( ENVVALID != NULL )
        if( getenv( ENVVALID ) != NULL )
            MYREC.opts |= OP_VALID;
    if( ENVVIP != NULL )
        if( getenv( ENVVIP ) != NULL )
            MYREC.opts |= OP_VIP;
    if( ENVCOSYSOP != NULL )
        if( getenv( ENVCOSYSOP ) != NULL )
            MYREC.opts |= OP_COSYSOP;
}

static int stty0()
{
    /*  Forces a hangup by setting the terminal's baud rate to
        zero.  Implemented as a kludge to workaround
        a flaky serial driver that refused to honor HUPCL.

        Use only if running CB standalone (as login shell, etc)
        VIP users are not affected
    */

    if( MYREC.opts & OP_VIP )
        return( 0 );
    #ifdef CBAUD
    oldterm.c_cflag &= ~CBAUD;
    #endif
    restterm(&oldterm);
    /* Most likely never makes it to here */
    return( 0 );
}

void fixdorks()
{
    int i;

    for( i= 0; i < ulsize; i++ )
        if( isact( slot, i ) )
            if( !stricmp( ulog[i].handle, guserid ) )
                strcpy( ulog[i].handle, ulog[i].userid );
}
