#include "../menu/menu.h"

/*
 *  add time to a slot
 */

int m_add( slot, add )
int slot, add;
{
    int ret;

    ret = 1;

    if (m_act(slot)) {
        hotlog[slot].maxtime += add;
        ret = 0;
    }
    return( ret );
}


/*
 * Change paging availability status
 */

void m_chpage( slot )
int slot;
{
    char    c;
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


/*
 *  change the doing field of a slot
 */

void m_chdoing(slot, doing)
int slot;
char *doing;
{
    if (strlen(doing) >= 64)
        doing[65] = 0;
    strcpy(hotlog[slot].doing, doing);
}


/*
 *  transfer credits from one slot to another
 */

int m_xfer(from, to, n)
int from, to;
long n;
{
    char    line[256];

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
