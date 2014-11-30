/*
 *	DateTime.c
 *
 *	Date/time routines to allow the user to define the output
 *	format.
 *
 * (C) 1989 Alan Premselaar
 * All rights reserved.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

char	*xweekday[] = {	"Sunday",
			"Monday",
			"Tuesday",
			"Wednesday",
			"Thursday",
			"Friday",
			"Saturday",
			NULL };

char	*mnth[] = {	"January",
			"February",
			"March",
			"April",
			"May",
			"June",
			"July",
			"August",
			"September",
			"October",
			"November",
			"December",
			NULL };
	
char *datetime(format,timenow)
char	*format;
long	timenow;
{
	static	char	temp[256], line[256];
	int	hour;
	int	pm = 0;
	struct	tm	*now;
	char	*x;
	extern	char	*tzname[2];

	memset( temp , 0 , 256 );


	now = localtime(&timenow);	/* get the tm structure for the time
					   given */

	for ( x = format; *x != 0; x++ ) {
		if ( *x != '%' ) {
			temp[strlen(temp)] = *x;
		}
		else 
			switch (*(++x)) {
				case '_':
					temp[strlen(temp)] = '\n';
					break;
				case 'a':
					if ((now->tm_hour >= 12) || pm)
						strncat(temp,"pm",2);
					else
						strncat(temp,"am",2);
					break;
				case 'A':
					if ((now->tm_hour >= 12) || pm)
						strncat(temp,"PM",2);
					else
						strncat(temp,"AM",2);
					break;
				case 'w':
					strncat(temp,xweekday[now->tm_wday],3);
					break;
				case 'W':
					strcat(temp,xweekday[now->tm_wday]);
					break;
				case 'o':
					strncat(temp,mnth[now->tm_mon],3);
					break;
				case 'O':
					strcat(temp,mnth[now->tm_mon]);
					break;
				case 't':
					strcat(temp,tzname[now->tm_isdst]);
					break;
				case 'T':
					strcat(temp,getenv("TZ"));
					break;
				case 'Y':
					if (now->tm_year < 70)
						sprintf(line,"%04d",
							now->tm_year+2000);
					else
						sprintf(line,"%04d",
							now->tm_year+1900);
					strcat(temp,line);
					break;
				case 'y':
					sprintf(line,"%02d",now->tm_year);
					strcat(temp,line);
					break;
				case 'H':
					sprintf(line,"%02d",now->tm_hour);
					strcat(temp,line);
					break;
				case 'h':
					if (now->tm_hour == 0) 
						hour = 24;
					else
						hour = now->tm_hour;

					if (hour > 12) {
						hour -= 12;
						pm = 1;
					}

					sprintf(line,"%02d",hour);
					strcat(temp,line);
					break;
				case 's':
					sprintf(line,"%02d",now->tm_sec);
					strcat(temp,line);
					break;
				case 'm':
					sprintf(line,"%02d",now->tm_min);
					strcat(temp,line);
					break;
				case 'd':
					sprintf(line,"%02d",now->tm_mday);
					strcat(temp,line);
					break;
				case 'D':
					sprintf(line,"%d",now->tm_mday);
					strcat(temp,line);
					break;
				case 'n':
					sprintf(line,"%02d",now->tm_mon+1);
					strcat(temp,line);
					break;
				case 'N':
					sprintf(line,"%d",now->tm_mon+1);
					strcat(temp,line);
					break;
				case '#':
					sprintf(line,"%d",now->tm_yday);
					strcat(temp,line);
					break;
			}
	}

	return( temp );
}

#ifdef TESTING

main()
{
	char	format[256];
	char	*datetime();
	long	timenow;

	printf("enter format: ");
	gets(format);

	timenow = time( (long *) 0);
	printf("%s\n",datetime(format,timenow));
}
#endif
