/*
 * @(#)systime.c	1.10 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * File: systime.c	1.9 12/22/97
 *
 *	Contains:	This module contains OS independent functions for
 *				time related stuff.
 *				Created by PGT, Dec 2, 1993
 *
 *	Written by:	The Kodak CMS MS Windows Team
 *
 *	Macintosh
 *	Change History (most recent first):
 *
 *	Windows Revision Level:
 *		$Workfile$
 *		$Logfile$
 *		$Revision$
 *		$Date$
 *		$Author$
 *
 ****************************************************************
 *    COPYRIGHT (c) 1993-1997 Eastman Kodak Company.
 *    As an unpublished work pursuant to Title 17 of the United
 *    States Code.  All rights reserved.
 *****************************************************************
 */

#include "kcms_sys.h"


#if defined(KPWIN32) && defined(ICM)


/*--------------------------------------------------------------------
 * DESCRIPTION
 *	This is a function to return the time and date in a
 *  standard ANSI tm structure. This is an implementation for
 *  Win32 when the ANSI runtime is not available. It currently
 *  does not return the day of the year correctly and returns
 *  unknown for whether daylight savings time is in effect.
 *
 * AUTHOR
 * 	pgt
 *
 * DATE CREATED
 *	Dec 2, 1993
 *------------------------------------------------------------------*/

void KpGetLocalTime( struct kpTm FAR *localTime )
{
	SYSTEMTIME systemTime;

	GetLocalTime( &systemTime );

	localTime->sec   = systemTime.wSecond;
	localTime->min   = systemTime.wMinute;
	localTime->hour  = systemTime.wHour;
	localTime->mday  = systemTime.wDay;
	localTime->mon   = systemTime.wMonth-1;
	localTime->year  = systemTime.wYear-1900;
	localTime->wday  = systemTime.wDayOfWeek;
	localTime->yday  = 0;
	localTime->isdst = (-1);

}

#elif defined(KPSOLARIS) || defined (KPLINUX)

#include <time.h>

/*--------------------------------------------------------------------
 * DESCRIPTION
 *	This is a function to return the time and date for sun solaris
 *	systems
 *
 * AUTHOR
 * 	pgt
 *
 * DATE CREATED
 *	Dec 2, 1993
 *------------------------------------------------------------------*/

void KpGetLocalTime( struct kpTm FAR *localTime )
{
	time_t	currentTime;
	struct tm ltime;

	time( &currentTime );

	localtime_r( &currentTime, &ltime );

	localTime->sec   = ltime.tm_sec;
	localTime->min   = ltime.tm_min;
	localTime->hour  = ltime.tm_hour;
	localTime->mday  = ltime.tm_mday;
	localTime->mon   = ltime.tm_mon;
	localTime->year  = ltime.tm_year;
	localTime->wday  = ltime.tm_wday;
	localTime->yday  = ltime.tm_yday;
	localTime->isdst = ltime.tm_isdst;

}

#else

#include <time.h>



/*--------------------------------------------------------------------
 * DESCRIPTION
 *	This is a function to return the time and date in a
 *  standard ANSI tm structure. This just uses the ANSI functions.
 *
 * AUTHOR
 * 	pgt
 *
 * DATE CREATED
 *	Dec 2, 1993
 *------------------------------------------------------------------*/

void KpGetLocalTime( struct kpTm FAR *localTime )
{
	time_t	currentTime;
	struct tm *ltime;

#if defined(KPWIN32)
	SYSTEMTIME systemTime;
#endif

	time( &currentTime );

	ltime = localtime( &currentTime );

#if defined(KPWIN32)
	if (ltime) {
#endif

		localTime->sec   = ltime->tm_sec;
		localTime->min   = ltime->tm_min;
		localTime->hour  = ltime->tm_hour;
		localTime->mday  = ltime->tm_mday;
		localTime->mon   = ltime->tm_mon;
		localTime->year  = ltime->tm_year;
		localTime->wday  = ltime->tm_wday;
		localTime->yday  = ltime->tm_yday;
		localTime->isdst = ltime->tm_isdst;

#if defined(KPWIN32)
	}
	else {	/* after Jan. 2038  */
		GetLocalTime( &systemTime );

		localTime->sec   = systemTime.wSecond;
		localTime->min   = systemTime.wMinute;
		localTime->hour  = systemTime.wHour;
		localTime->mday  = systemTime.wDay;
		localTime->mon   = systemTime.wMonth-1;
		localTime->year  = systemTime.wYear-1900;
		localTime->wday  = systemTime.wDayOfWeek;
		localTime->yday  = 0;
		localTime->isdst = (-1);
	}
#endif

}

#endif		/* KPWIN32 && ICM */


