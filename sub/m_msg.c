#include "../menu/menu.h"

/*
 ************************************************************************
 *
 *	send a msg or do-something type routines
 *
 ************************************************************************
 */

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

m_newmail( i )
int i;
{
	/*	Send a "new mail" message to slot i
	*/

	if( i == -1 ) return;

	m_msgslot( i, "", T_NEWMAIL );
}
