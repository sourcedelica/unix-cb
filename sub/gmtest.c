#include <stdio.h>
#include <skynet/muserinf.h>
#include <skynet/getmsg.h>

main(argc,argv)
int argc;
char **argv;
{
        char **msg;
        char tit[80];
	int len;

        if( argc == 1 ){
                printf("usage: getmsg maxlines [maxllen]\n");
                exit(0);
        }
	len = ( argc == 3 ? atoi(argv[2]) : 80 );
        strcpy(ptitle= tit,"Title man");
        titav = 1;
        raw();
        msg = getmsg(atoi(argv[1]),len);
        if( msg == (char **)0 )
                puts("no message");
        else {
		printf("%s ----------\n",tit);
                for(;*msg != NULL;msg++)
                        puts(*msg);
	}
        sane();
}
