/*
 * @(#)cmmdll.c	1.5 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*

	Contains:	This module contains the required DLL entry points.
			Created by acr, October 94, 1997

	Written by:	The Kodak CMS  Team

 **********************************************************************
 *** COPYRIGHT (c) Eastman Kodak Company, 1997-1998
 *** As an unpublished work pursuant to Title 17 of the United
 *** States Code.  All rights reserved.
 **********************************************************************
 */

#include "kcmsos.h"

void KpMemInit();

/* Global handle to save the DLL's Instance */
HANDLE	SpHInst;

/* Macro for debug strings */
#if defined(_DEBUG)
#define SP_DEBUG_STRING(dbgstr) OutputDebugString (dbgstr)
#else
#define SP_DEBUG_STRING(dbgstr) 
#endif

/****************************************************************************
 * Function: DllMain (HINSTANCE, WORD, LPVOID)
 *
 * Purpose:  Entry point to DLL.
 *
 * Parameters: None
 *          hInstance == Handle to this instance
 *       reasonCalled == Situation when DllMain is called
 *
 * Comments:
 *
 * History:  Date       Author        Reason
 *           8/5/93		P. Tracy      Created
 ****************************************************************************/
INT APIENTRY DllMain (HINSTANCE hInstance, DWORD reasonCalled, LPVOID lpNot)
{
	lpNot = lpNot;
	hInstance = hInstance;

	SP_DEBUG_STRING ("jprofile ");
	switch (reasonCalled) {
	case DLL_PROCESS_ATTACH:
		SP_DEBUG_STRING ("Process Attach");
		KpMemInit();
		SpHInst = hInstance;
		break;

	case DLL_PROCESS_DETACH:
		SP_DEBUG_STRING ("Process Detach");
		SpHInst = NULL;
		break;

	case DLL_THREAD_ATTACH:
		SP_DEBUG_STRING ("Thread Attach");
		break;

	case DLL_THREAD_DETACH:
		SP_DEBUG_STRING ("Thread Detach");
		break;
	}
	SP_DEBUG_STRING ("\n");

    return TRUE;
}
