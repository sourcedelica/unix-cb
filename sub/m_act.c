#include <ctype.h>
#include "../menu/menu.h"

/*
 *************************************************************************
 *
 *  These routines return active statuses, names, values, etc
 *
 *************************************************************************
 */

/*
 *  Return true if this slot is active
 */

int m_act( slot )
int slot;
{
    if( slot < 0 || slot > ctl->numslots )
        return( 0 );
    if (hotlog[slot].status & STF_TAKEN)
        return( 1 );
    else
        return( 0 );
}


/*
 *  Return slot # of user s, -1 if not active
 */

int m_slot( s )
char *s;
{
    int i;

    if( isdigit(*s) ){
        i = atoi(s);
        if( i < 0 || i > ctl->numslots )
            return( -1 );
        if (hotlog[i].status & STF_TAKEN)
            return( i );
        else
            return( -1 );
    }
    for (i=0; i<ctl->numslots ;i++) {
        if (hotlog[i].status & STF_TAKEN)
            if (!stricmp(s, hotlog[i].uname) || !stricmp(s, hotlog[i].alias) )
                return( i );
    }
    return( -1 );
}


/*
 *  Return time (in seconds) on the system.
 */

int m_on( slot )
int slot;
{
    long secs;
    secs = time((long *) 0);
    return( secs-hotlog[slot].starttime );
}


/*
 *  return logname of a slot
 */

char *m_uname( slot )
int slot;
{
    return( hotlog[slot].uname );
}


/*
 *  return alias of a slot
 */

char *m_alias( slot )
int slot;
{
    return( hotlog[slot].alias );
}


/*
 *  return the doing field of a slot
 */

char *m_doing( slot )
int slot;
{
    return( hotlog[slot].doing );
}


/*
 *  return the device associated with a slot
 */

char *m_device( slot )
int slot;
{
    return( hotlog[slot].device );
}


/*
 *  return the flags value of a slot
 */

long m_flags( slot )
int slot;
{
    return( hotlog[slot].flags );
}

/*
 *  return the privs value of a slot
 */

long m_privs( slot )
int slot;
{
    return( hotlog[slot].privs );
}


/*
 *  return the status flags of a slot
 */

long m_stf( slot )
int slot;
{
    return( hotlog[slot].status );
}


/*
 *  return true of a slot is pageable
 */

int m_pageable( slot )
int slot;
{
    if (hotlog[slot].status & STF_NOPAGE)
        return( 0 );
    return( 1 );
}


/*
 *  return the credits of a slot
 */

long m_credits(i)
int i;
{
    return (hotlog[i].credits);
}


/*
 *  return true of a record is paid
 */

int m_paid( hisrec )
struct urec *hisrec;
{
    long secs;
    int paid;

    secs = time( (long *)0);
    paid = 0;

    switch (hisrec->from) {
        case -1:        /* new subscription, set the date */
            paid = 1;
            break;

        case 0:  /* no subscription, check for credits */
            if (hisrec->credits > 0)
                paid =  1;
            break;

        default:  /* he paid, but is it still in effect? */
            if (secs > (hisrec->from + hisrec->days*24*60*60) ) {

                if (hisrec->credits)
                    paid = 1;
            } else
                paid = 1;
            break;
    }
    return( paid );
}

int m_left( slot )
int slot;
{
/*
 *  Return time left until cutoff.
 */
    long on, secs;

    secs = time((long *) 0);
    on = secs - hotlog[slot].starttime;
    return( hotlog[slot].maxtime - on );
}

int m_ruload()
{
    int i, uload;

    uload = 0;

    for (i=0; i<ctl->numslots ;i++)
        if (hotlog[i].status & STF_TAKEN &&
            hotlog[i].status & STF_ACTIVE &&
            hotlog[i].status & STF_DIALUP &&
            !(hotlog[i].status & STF_SYSTEMSLOT) )
            uload++;
    return(uload);
}

int m_uload()
{
    int i, uload;

    uload = 0;

    for (i=0; i<ctl->numslots ;i++)
        if (hotlog[i].status & STF_TAKEN &&
            hotlog[i].status & STF_ACTIVE &&
            !(hotlog[i].status & STF_SYSTEMSLOT) )
            uload++;
    return(uload);
}
