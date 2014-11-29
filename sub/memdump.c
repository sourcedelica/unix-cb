#include <stdio.h>
#include <ctype.h>

/**********************************************************************

	memdump

	Displays contents of memory in "dump" style for length
	given, with offset "address" in left-hand column.

	The "Address" is computed by adding ibase to the current
	index within 0 and len.  The actual data is taken from
	addr.

	Dump is fixed at 80 columns

***********************************************************************/

void memdump( ibase, addr, len )
int ibase;
unsigned char *addr;
int len;
{
	int base, i;
	char c;

	base = 0;

	while( base < len ){
		printf( "%.8x  ", ibase+base );
		for( i= 0; i < 16; i++ ){
			if( base+i == len ) break;
			printf( "%.2X ", *(addr+base+i) );
			if( i == 7 || i == 15 )
				putchar(' ');
		}
		if( i < 16 ){
			putchar(' ');
			if( i < 8 )
				putchar(' ');
		}
		for( ; i < 16; i++ )
			printf("   ");
		putchar('|');
		for( i= 0; i < 16; i++ ){
			if( base+i == len ) break;
			c = *(addr+base+i);
			printf( "%c", isascii(c) && !iscntrl(c) ? c : '.' );
		}
		for( ; i < 16; i++ )
			putchar(' ');
		puts("|");
		base = base + i;
	}
}
