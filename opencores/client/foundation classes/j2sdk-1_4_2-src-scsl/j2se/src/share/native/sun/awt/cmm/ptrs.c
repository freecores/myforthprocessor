/*
 * @(#)ptrs.c	1.8 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
	File:		PTRS.c	@(#)ptrs.c	1.6 1/21/94

	Contains:	This module contains routines to check the validity of
			memory pointers, in order to avoid any GPF's in MS
			Windows.

				Created by HTR, December 2, 1993

	Written by:	The Kodak CMS MS Windows Team

	Copyright:	(C) 1993-1999 by Eastman Kodak Company,
			all rights reserved.

	Macintosh
	Change History (most recent first):

		 <1>	12/29/93	HTR		1'st checked in

	Windows Revision Level:

	SCCS Revision:
		@(#)ptrs.c	1.15 02/01/99

	To Do:
 */

/***********************************************************************/

#include	"kcms_sys.h"

/***********************************************************************/

KpInt32_t FAR Kp_IsBadHugeReadPtr (
			const void	KPHUGE *Ptr,
			KpUInt32_t	Size);

KpInt32_t FAR Kp_IsBadHugeWritePtr (
			void		KPHUGE *Ptr,
			KpUInt32_t	Size);

KpInt32_t FAR Kp_IsBadReadPtr (
			const void	FAR *Ptr,
			KpUInt32_t	Size);

KpInt32_t FAR Kp_IsBadWritePtr (
			void		FAR *Ptr,
			KpUInt32_t	Size);

KpInt32_t FAR Kp_IsBadStringPtr (
			const void	FAR *Ptr,
			KpUInt32_t	Size);

KpInt32_t FAR Kp_IsBadCodePtr (
			KpGenericPtr_t	Ptr);

#if defined(KPWIN32) && !defined(KPMSMAC) && defined(KPSYS_DEBUG)

KpInt32_t	Kp_IsBadHugeReadPtr(const void KPHUGE* hPtr, KpUInt32_t nbytes)
{
	return (IsBadHugeReadPtr(hPtr, (UINT) nbytes));
}


KpInt32_t	Kp_IsBadHugeWritePtr(void KPHUGE* hPtr, KpUInt32_t nbytes)
{
	return (IsBadHugeWritePtr(hPtr, (UINT) nbytes));
}


KpInt32_t	Kp_IsBadReadPtr(const void FAR* lPtr, KpUInt32_t nbytes)
{
	return (IsBadReadPtr(lPtr, (UINT) nbytes));
}


KpInt32_t	Kp_IsBadWritePtr(void FAR* lPtr, KpUInt32_t nbytes)
{
	return (IsBadWritePtr(lPtr, (UINT) nbytes));
}


KpInt32_t	Kp_IsBadCodePtr(KpGenericPtr_t	Ptr)
{
	return (IsBadCodePtr(Ptr));
}


KpInt32_t	Kp_IsBadStringPtr(const void FAR* lPtr, KpUInt32_t nbytes)
{
	return (IsBadStringPtr(lPtr, (UINT) nbytes));
}

#elif defined(KPWIN16)

KpInt32_t	Kp_IsBadHugeReadPtr(const void KPHUGE* hPtr, KpUInt32_t nbytes)
{
	return (IsBadHugeReadPtr(hPtr, (DWORD) nbytes));
}


KpInt32_t	Kp_IsBadHugeWritePtr(void KPHUGE* hPtr, KpUInt32_t nbytes)
{
	return (IsBadHugeWritePtr(hPtr, (DWORD) nbytes));
}


KpInt32_t	Kp_IsBadReadPtr(const void FAR* lPtr, KpUInt32_t nbytes)
{
	return (IsBadReadPtr(lPtr, (UINT) nbytes));
}


KpInt32_t	Kp_IsBadWritePtr(void FAR* lPtr, KpUInt32_t nbytes)
{
	return (IsBadWritePtr(lPtr, (UINT) nbytes));
}


KpInt32_t	Kp_IsBadCodePtr(KpGenericPtr_t	Ptr)
{
	return (IsBadCodePtr(Ptr));
}


KpInt32_t	Kp_IsBadStringPtr(const void FAR* lPtr, KpUInt32_t nbytes)
{
	return (IsBadStringPtr(lPtr, (UINT) nbytes));
}

	/* end of _WIN32 */
#else

KpInt32_t	Kp_IsBadHugeReadPtr(const void KPHUGE* hPtr, KpUInt32_t nbytes)
{
	if (hPtr){}
	if (nbytes){}

	return (KPFALSE);
}


KpInt32_t	Kp_IsBadHugeWritePtr(void KPHUGE* hPtr, KpUInt32_t nbytes)
{
	if (hPtr){}
	if (nbytes){}

	return (KPFALSE);
}


KpInt32_t	Kp_IsBadReadPtr(const void FAR* lPtr, KpUInt32_t nbytes)
{
	if (lPtr){}
	if (nbytes){}

	return (KPFALSE);
}


KpInt32_t	Kp_IsBadWritePtr(void FAR* lPtr, KpUInt32_t nbytes)
{
	if (lPtr){}
	if (nbytes){}

	return (KPFALSE);
}


KpInt32_t	Kp_IsBadStringPtr(const void FAR* lPtr, KpUInt32_t nbytes)
{
	if (lPtr){}
	if (nbytes){}

	return (KPFALSE);
}


KpInt32_t	Kp_IsBadCodePtr(KpGenericPtr_t	Ptr)
{
	if (Ptr){}

	return (KPFALSE);
}

#endif
