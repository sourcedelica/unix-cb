#include <ctype.h>
#include <string.h>
#include <pwd.h>

#define noextern			/* explained in menu.h		*/
#include "../menu/menu.h"

/*
 * ---------------------------------------------------------------- *
 * Menu utilities for use with outside programs to interface to the
 * menu data structures.
 *
 * (C) 1988 CPEX development
 * All rights reserved.
 * ---------------------------------------------------------------- *
 */

/*
 *	Attach to the shared memory using 'key' in the config file.  
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

void m_who(slot)
int slot;
{
/*
 *	Print a who report
 */
	int	i;
	char	c;

	puts("        Id/Name                     Activity");
	puts("    --------------                  ----------");

	for (i=0; i<ctl->numslots ;i++)

		if (hotlog[i].status & STF_TAKEN && m_samewho(slot, i)) {
				c = ' ';
				if (hotlog[i].flags & F_VALIDATED)
					c = '#';
				if (hotlog[i].status & STF_PAID)
					c = '$';
				if (hotlog[i].flags & F_COSYSOP)
					c = '%';
				if (hotlog[i].flags & F_SYSOP)
					c = '@';
			if (!(hotlog[i].status & STF_ACTIVE) ) {
				if (slot == -1)
					printf("%c %*s/%-*s %s (inactive)\n",c,
					L_cuserid-1, hotlog[i].uname,L_alias-1,
					hotlog[i].alias, hotlog[i].doing);

				else if (hotlog[slot].flags & F_SYSOP)
					printf("%c %*s/%-*s %s (inactive)\n",c,
					L_cuserid-1, hotlog[i].uname,L_alias-1,
					hotlog[i].alias, hotlog[i].doing);
			} else
				printf("%c %*s/%-*s %s\n",c,
				L_cuserid-1, hotlog[i].uname,L_alias-1,
				hotlog[i].alias, hotlog[i].doing);
		}
}

int m_samewho(me, him)
int me, him;
{
	int	i, ret;

	ret = 0;

	if (me == -1)
		return( 1 );

	for (i=0; i<sizeof(int) ;i++)
		if (hotlog[me].who & (1<<i) )
			if (hotlog[him].who & (1<<i) )
				ret++;
	return( ret );
}

int m_act( slot )
int slot;
{
/*
 *	Return true if this slot is active
 */
	if( slot < 0 || slot > ctl->numslots )
		return( 0 );
	if (hotlog[slot].status & STF_TAKEN)
		return( 1 );
	else
		return( 0 );
}

int m_slot( s )
char *s;
{
/*
 *	Return slot # of user s, -1 if not active
 */
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

int m_on( slot )
int slot;
{
/*
 *	Return time (in seconds) on the system.
 */
	long secs;
	secs = time((long *) 0);
	return( secs-hotlog[slot].starttime );
}

int m_left( slot )
int slot;
{
/*
 *	Return time left until cutoff.
 */
	long on, secs;

	secs = time((long *) 0);
	on = secs - hotlog[slot].starttime;
	return( hotlog[slot].maxtime - on );
}

char *m_uname( slot )
int slot;
{
	return( hotlog[slot].uname );
}

char *m_alias( slot )
int slot;
{
	return( hotlog[slot].alias );
}

char *m_doing( slot )
int slot;
{
	return( hotlog[slot].doing );
}

char *m_device( slot )
int slot;
{
	return( hotlog[slot].device );
}

long m_flags( slot )
int slot;
{
	return( hotlog[slot].flags );
}

long m_stf( slot )
int slot;
{
	return( hotlog[slot].status );
}

void m_add( slot, add )
int slot, add;
{
	hotlog[slot].maxtime += add;
}

int m_pageable( slot )
int slot;
{
	if (hotlog[slot].status & STF_NOPAGE)
		return( 0 );
	return( 1 );
}

/*
 * page a a user, return 0 if successful, 1 if that user is not accepting
 * pages
 */
void m_page( slot, msg, from )
int slot;
char *msg;
int from;
{
	mymsgp.mtype = (long)hotlog[slot].bpid;
	mymsgp.act.type = T_PAGE;
	if( from == -1 )
		sprintf(mymsgp.act.msg,"Page from %s: %s",cuserid(NULL),msg);
	else
		sprintf(mymsgp.act.msg,"Page from %s/%s: %s",hotlog[from].uname,
			hotlog[from].alias, msg);
	mymsgp.act.msg[255] = 0;
	msgsnd(m_qid, &mymsgp, sizeof(struct actionstr), 0);
}

/*
 * Change paging availability status
 */
void m_chpage( slot )
int slot;
{
	char c;
	int i;

	printf("Accept any pages at all? ");
	c = getchar();
	c = tolower(c);

	if (c == 'y') {
		puts("Yes");
		hotlog[slot].status &= ~STF_NOPAGE;
	} else {
		puts("No");
		hotlog[slot].status |= STF_NOPAGE;
	}
	printf("Accept pages while in programs? ");
	c = getchar();
	c = tolower(c);

	if (c == 'y') {
		puts("Yes");
		hotlog[slot].status &= ~STF_QPAGES;
	} else {
		puts("No");
		hotlog[slot].status |= STF_QPAGES;
	}
	putchar('\n');
}

void m_chdoing(slot, doing)
int slot;
char *doing;
{
	if (strlen(doing) >= 64)
		doing[65] = 0;
	strcpy(hotlog[slot].doing, doing);
}

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

/*
	char fname[256], line[256];
	FILE *fp;
*/
/*
	if (ctl->skynetpath[0] == '/')
		return(ctl->skynetpath);
*/
/*
	if ((fp=fopen("/etc/skynet", "r")) == NULL)
		return( "" );
	if (fgets(line, 256, fp) == NULL) {
		return( "" );
	}
	line[strlen(line)-1] = 0;
	fclose(fp);
	return( line );
*/
}

/*
 *	Form the filename based on 'dir'.
 *	if dir is "$HOME" then 'file' is in our home directory
 *
 *	(c) 1988 CPEX development
 *	All rights reserved
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

int m_msg( uname, msg, type )
char *uname, *msg;
int type;
{
	int i;

	if ((i=m_slot(uname)) != -1) {
		mymsgp.mtype = (long)hotlog[i].bpid;
		mymsgp.act.type = type;
		msg[255] = 0;
		strcpy(mymsgp.act.msg, msg);
		return( msgsnd(m_qid, &mymsgp, sizeof(struct actionstr), 0) );
	}
}

int m_msgslot( i, msg, type )
char *msg;
int i, type;
{
	if (m_act(i) ) {
		mymsgp.mtype = (long)hotlog[i].bpid;
		mymsgp.act.type = type;
		msg[255] = 0;
		strcpy(mymsgp.act.msg, msg);
		return( msgsnd(m_qid, &mymsgp, sizeof(struct actionstr), 0) );
	}
}

void *privprint(privs, header, showall)
long	privs;
char *header;
int showall;
{
	int	j=0;
	FILE	*pfp;
	char	fname[256], str[256];

	strcpy(fname, formfname("privs", "cf") );

	if ((pfp = fopen(fname,"r")) == NULL)
		return(NULL);	

	while (fgets(str,256,pfp) != NULL) {
		j = atoi(strtok(str,"\t"));

		if (showall)
			printf("%s%d) %s: %s\n",header,j,strtok(NULL, "\n"),
				(privs & (1 << j)) ? "Yes" : "No");

		else if (privs & (1 << j) )
			printf("%s%s\n",header,strtok(NULL, "\n") );
	}
	fclose(pfp);
}
	
void plevel(level, header)
int	level;
char *header;
{
	FILE	*lfp;
	char	str[256];
	int	i;
	char	*strtok(), fname[256];

	strcpy(fname, formfname("levels", "cf") );

	if ((lfp = fopen(fname,"r")) == NULL)
		return;

	while (fgets(str,256,lfp) != NULL) {
		i = atoi(strtok(str,"\t"));

		if (i > level)
			break;

		if (i <= level)
			printf("%s%s\n",header,strtok(NULL,"\n") );
	}
	fclose(lfp);
}	

long m_credits(i)
int i;
{
	return (hotlog[i].credits);
}

int m_xfer(from, to, n)
int from, to;
long n;
{
	char	line[256];

	if (hotlog[from].credits-n >= 0) {
		hotlog[from].credits -= n;
		hotlog[to].credits += n;
		hotlog[to].flags |= F_TICKCREDITS;
		sprintf(line,"%d Credit%s transferred from %s/%s",n,
			n == 1 ? "" : "s", m_uname(from), m_alias(from) );
		return( m_msgslot(to, line, T_XFERFUNDS) );
	} else
		return( -1 );
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

int m_paid( hisrec )
struct urec *hisrec;
{
	long secs;
	int paid;

	secs = time( (long *)0);
	paid = 0;

	switch (hisrec->from) {
		case -1:		/* new subscription, set the date */
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
