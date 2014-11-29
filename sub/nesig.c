#include <termio.h>
#include <signal.h>

static struct termio temp;
static void (*oldi)();
static void (*oldq)();

int setnesig( cintr, cquit, catch )
char cintr;
char cquit;
void (*catch)();
{
	saveterm( &temp );
	oldi = (void(*)())signal(SIGINT,catch);
	oldq = (void(*)())signal(SIGQUIT,catch); 
	sigraw( cintr, cquit );
	return(0);
}

void unnesig()
{
	restterm( &temp );
	signal(SIGINT,oldi);
	signal(SIGQUIT,oldq);
}
