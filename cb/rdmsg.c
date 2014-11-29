#include "menu.h"

main()
{
	int	i;
	char	line[256];

	m_at();
	for (;;) {
		i = msgrcv(m_qid, &mymsgp, sizeof(struct actionstr),
		0, 0);

		if (i == -1) {
			puts("Error waiting on message queue.");
			exit(EXITERR);
		}
		printf("mtype is %d\ntype is %d\n msg:%s\n",mymsgp.mtype,mymsgp.act.type,mymsgp.act.msg);
		puts("hit return to read next msg");
		gets(line);
	}
}
