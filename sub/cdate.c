#include <stdio.h>
#include <sys/types.h>
#include <time.h>
#include "../include/cdate.h"

/*
 *	Skynet
 *
 *	Convert 'Touch' style date to time_t
 *
 *	Copyright (C) 1987, 1988 CPEX Development
 *	All Rights Reserved.
 *	Author: Eric Pederson
 *
 */

#define TIME0	(time_t)0
#define YRSEC	(long)(365L*24L*60L*60L)
#define DAYSEC	(long)(24L*60L*60L)
#define HRSEC	(long)(60L*60L)

/**********************************************************************/

time_t cdate( cd )
char *cd;
{
	/*
	 *	Converts a mmddhhmm[ss][yy] string
	 *
	 *	Returns (time_t)0 on fail
	 *	will assume "this year" if no yy given
	 *
	 *	Bugs:  Uses Brute Force Method
	 *	       DST handling is less than optimal
	 */

	char *x;
	time_t temp, now;
	int i, j, mon, yr;
	static int mons[] = {
		 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
	struct tm *tim;
	
	if ( (strlen(cd) > 12) || (strlen(cd) < 8) )
		return(TIME0);

	tzset();
	x = cd;
	temp = TIME0;
	i = 0;
	i = (*(x++) - '0') * 10;
	mon = i += *(x++) - '0';
	for( j=0; j < i-1; j++ )
		temp += mons[j];
	i = (*(x++) - '0') * 10;
	i += *(x++) - '0';
	temp += i - 1;
	temp *= DAYSEC;
	i = (*(x++) - '0') * 10;
	i += *(x++) - '0';
	temp += i * HRSEC;
	i = (*(x++) - '0') * 10;
	i += *(x++) - '0';
	temp += i*60L;
	if( strlen(cd) >= 10 ){
		i = (*(x++) - '0') * 10;
		i += *(x++) - '0';
		temp += i;
	} else {
		x++; x++;
	}
	time(&now);
	tim = localtime(&now);
	if( strlen(cd) == 12 ){
		i = (*(x++) - '0') * 10;
		i += *(x++) - '0';
	} else {
		i = tim->tm_year;
	}
	yr = i -= 70;
	i = (i+2) / 4;
	/* If before leap day on leap year, adjust */
	if( (((yr+2) % 4) == 0) && (mon < 3) )
		i--;
	temp += i*DAYSEC;
	temp += yr*YRSEC;
#ifdef BSD
	temp += tim->tm_gmtoff;
#else
	temp += timezone;
#endif
	tim = localtime(&temp);
	if( tim->tm_isdst )
		temp -= HRSEC;
	
	return(temp);
}

char *datec( ttime )
time_t ttime;
{
	/*
	 *	Returns a string in mmddhhmmyy form
	 *	from ttime
	 *
	 */

	static char sdatec[13];
	struct tm *tim;

	if( ttime == TIME0 )
		return("010100000070");

	tim = localtime( &ttime );
	sprintf(sdatec,"%.2d%.2d%.2d%.2d%.2d%.2d",
		tim->tm_mon+1,tim->tm_mday,tim->tm_hour,tim->tm_min,
		tim->tm_sec,tim->tm_year);

	sdatec[12] = 0;
	return(sdatec);
}


#ifdef TESTING
void main()
{
	char x[13];
	time_t what;

	time( &what );
	printf("today is %s\n%s",datec(what),ctime(&what));
	for(;;){
		gets(x);
		what = cdate(x);
		printf("returns %ld which is %s",what,ctime(&what));
	}
}
#endif
