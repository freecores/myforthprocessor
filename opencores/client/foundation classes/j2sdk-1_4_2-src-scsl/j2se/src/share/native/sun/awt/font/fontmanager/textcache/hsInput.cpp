/*
 * @(#)hsInput.cpp	1.6 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * Copyright (C) 1996-1997 all rights reserved by HeadSpin Technology Inc. Chapel Hill, NC USA
 *
 * This software is the property of HeadSpin Technology Inc. and it is furnished
 * under a license and may be used and copied only in accordance with the
 * terms of such license and with the inclusion of the above copyright notice.
 * This software or any other copies thereof may not be provided or otherwise
 * made available to any other person or entity except as allowed under license.
 * No title to and ownership of the software or intellectual property
 * therewithin is hereby transferred.
 *
 * HEADSPIN MAKES NO REPRESENTATIONS OR WARRANTIES ABOUT THE SUITABILITY
 * OF THE SOFTWARE, EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED
 * TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
 * PARTICULAR PURPOSE, OR NON-INFRINGEMENT. SUN SHALL NOT BE LIABLE FOR
 * ANY DAMAGES SUFFERED BY LICENSEE AS A RESULT OF USING, MODIFYING OR
 * DISTRIBUTING THIS SOFTWARE OR ITS DERIVATIVES.
 *
 * This information in this software is subject to change without notice
*/

#include "hsInput.h"

#if HS_BUILD_FOR_MAC
	#include <Events.h>
	#include <TextUtils.h>
	#include <Memory.h>
	#include <Quickdraw.h>
	#include <Windows.h>
	#include <Fonts.h>
	#include <Dialogs.h>
	#include <stdio.h>
	#include <string.h>
	#include <toolutils.h>
#endif

//
// Is a key down or not?
//
Boolean hsInput::KeyDown(hsKeyType keyCode)
{
#if HS_BUILD_FOR_WIN32
	if ((GetAsyncKeyState(keyCode) & 0x8000) )
		return true;
	return false;
#endif

#if HS_BUILD_FOR_MAC
	FlushEvents(0xffff,0);
	KeyMap	keys;
	GetKeys(keys);
	if ((BitTst((Ptr)(keys), keyCode)))
		return true;
	return false;
#endif
}
