#include "../menu/menu.h"

/*
 *	Print a who report
 */

void m_who(slot)
int slot;
{
	int	i;
	char	c;

	puts("        Id/Name                     Activity");
	puts("    --------------                  ----------");

	for (i=0; i<ctl->numslots ;i++)

		if (hotlog[i].status & STF_TAKEN && m_samewho(slot, i) ) {
			c = ' ';
			if (hotlog[i].flags & F_VALIDATED)
				c = '#';
			if (hotlog[i].status & STF_PAID)
				c = '$';
			if (hotlog[i].flags & F_COSYSOP)
				c = '%';
			if (hotlog[i].flags & F_SYSOP)
				c = '@';
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
