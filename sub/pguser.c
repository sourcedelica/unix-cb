#include <stdio.h>
#include "alias.h"
#include "muserinf.h"

extern char *m_uname(), *m_alias();

void pguser( slot )
int slot;
{
	/*	Page a Skynet User
	 *
	 *	slot is from who the page is said to be said
	 *	accepts -1 for unknown
	 *
	 *	Must have previously called m_at().
	 */

	int i;
	char s[L_alias], msg[80];

	if( readdefault("Page user: ",s,L_alias,"") == 0 )
		return;
	i = m_slot(s);
	if( i == -1 ){
		printf("%s not active\n",s);
		return;
	}
	printf("Paging %s/%s\n",m_uname(i),m_alias(i));
	if( readdefault("Msg: ",msg,80,"") == 0 )
		return;
	if( m_page( i, msg, slot ) )
		printf("%s/%s is not accepting pages at this time\n",
			m_uname(i),m_alias(i) );
	else
		printf("Page sent to %s/%s\n",m_uname(i),m_alias(i));
}
