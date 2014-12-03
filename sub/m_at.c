#define noextern
#include "../menu/menu.h"
#include <pwd.h>
#include <string.h>

/*
 **********************************************************************
 *
 *  general functions like m_at(), m_skynetpath(), formfname(), etc.
 *
 **********************************************************************
 */

/*
 *  Attach to the shared memory using the key in the config file.
 */

void m_at()
{
    char **p, fname[256], *m_skynetpath();
    FILE *fp;
    long atol();
    static int attached = 0;

    if( attached )
        return;
    sprintf(fname,"%s/cf/config",m_skynetpath() );
    if ((p=(char **) rdctl(fname)) == NULL)
        printf("Error reading %s\n",fname);

    if ((key=atol(getctl("KEY", p))) == 0L) {
        puts("Unable to attach to shared memory, KEY not found");
        return;
    }

    if ((m_mid=shmget( (key_t) key, 0, 0) ) == -1) {
        puts("Error getting shared memory id");
        exit(EXITERR);
    }
    if ((m_qid=msgget( (key_t) key, 0) ) == -1) {
        puts("Error getting message queue id");
        exit(EXITERR);
    }
    ctl = (struct ctlstr *) shmat(m_mid, NULL, 0);
    hotlog = (struct urec *) (ctl + 1);
    freectl(p);
    attached++;
}


/*
 *  return the path to the skynet directory
 */

char *m_skynetpath()
{
    /* Modified to use "skynet" id as source for
       skynet path */

    /* Reads password file only on first access, and returns
       a pointer to a static area that is NOT overwritten */

    struct passwd *pent, *getpwnam();
    static char *mspval;
    static mspok = 0;

    if( !mspok ){
        if( (pent= getpwnam( SKYUSER )) == (struct passwd *)NULL ){
            fprintf(stderr,"Cannot find user '%s'\n", SKYUSER );
            return( NULL );
        }
        mspval = strdup( pent->pw_dir );
        mspok++;
    }
    return( mspval );
}

/*
 *  Form the filename based on 'dir'.
 *  if dir is "$HOME" then 'file' is in our home directory
 *
 *  (c) 1988 CPEX development
 *  All rights reserved
 */

char *formfname(file,dir)
char *file, *dir;
{
    static char fullname[80];
    char *getenv();

    if (!strcmp(dir, "$HOME") )
        sprintf(fullname,"%s/%s",getenv("HOME"),file);
    else
        sprintf(fullname,"%s/%s/%s",m_skynetpath(),dir,file);
    return(fullname);
}
