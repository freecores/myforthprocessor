/*
 * @(#)HeadSpin.cpp	1.6 03/01/23
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

#include "HeadSpin.h"

#if HS_BUILD_FOR_MAC
	#include <Events.h>
	#include <ToolUtils.h>
	#include <Windows.h>
#endif

#if HS_BUILD_FOR_WIN32
	#define WIN32_LEAN_AND_MEAN
	#define WIN32_EXTRA_LEAN
	#include <windows.h>	// For OutputDebugString()
#endif

//////////////////////////////////////////////////////////////////////////

hsDebugMessageProc gHSDebugProc = nil;

hsDebugMessageProc hsSetDebugMessageProc(hsDebugMessageProc newProc)
{
	hsDebugMessageProc oldProc = gHSDebugProc;

	gHSDebugProc = newProc;

	return oldProc;
}

#ifdef HS_DEBUGGING
#if !(HS_BUILD_FOR_WIN32)
	void hsAssertFunc(int line, const char *file, const char *message)
	{
		char	s[256];
		
		(void)sprintf(s, "Assert Failed %s :: Line:%d  File:%s", message, line, file);
		hsDebugMessage(s, 0);
	}
#endif

void hsDebugMessage(const char message[], long val)
{
	char	s[256];

	s[0] = sprintf(&s[1], "%s: %ld", message, val);

	if (gHSDebugProc)
		gHSDebugProc(&s[1]);
#if HS_BUILD_FOR_MAC
	else
		DebugStr((unsigned char*)s);
#elif HS_BUILD_FOR_WIN32
	else
	{	OutputDebugString(&s[1]);
		OutputDebugString("\n");
		_RPT0(_CRT_WARN, &s[1]);
	}
#endif
}

#endif

///////////////////////////////////////////////////////////////////

hsRefCnt::~hsRefCnt()
{
	hsAssert(fRefCnt == 1, "bad fRefCnt in destructor");
}

void hsRefCnt::Ref()
{
	fRefCnt++;
}

void hsRefCnt::UnRef()
{
	hsAssert(fRefCnt >= 1, "bad ref count in UnRef");

	if (fRefCnt == 1)
		delete this;
	else
		--fRefCnt;
}

////////////////////////////////////////////////////////////////////////////

#if HS_BUILD_FOR_MAC
inline Boolean DoubleClickClose(Point a, Point b)
{
	#define kDoubleClickTolerance	5
	
	int	dh = a.h - b.h;
	int	dv = a.v - b.v;

	return hsABS(dh) <= kDoubleClickTolerance && hsABS(dv) <= kDoubleClickTolerance;
}

hsModifierKeys MacEventToModifiers(const struct EventRecord* event)
{
	hsModifierKeys		modifiers = 0;
	if (event->modifiers & cmdKey)
		modifiers |= kModifierCmd;
	if (event->modifiers & shiftKey)
		modifiers |= kModifierShift;
	if (event->modifiers & controlKey)
		modifiers |= kModifierCtrl;
	if (event->modifiers & optionKey)
		modifiers |= kModifierOption;
		
	KeyMap	keyMap;
	GetKeys(keyMap);
	if (BitTst((Ptr)keyMap, 54))
		modifiers |= kModifierSpace;

	//	Now check for double-clicks
	if (event->what == mouseDown)
	{	static WindowRef	gLastWindow = nil;
		static UInt32		gLastWhen = 0;
		static Point		gLastWhere = { -16000, -16000 };
		WindowRef		wind;
	
		if (	::FindWindow(event->where, &wind) == inContent &&
			gLastWindow == wind &&
			event->when - gLastWhen <= ::GetDblTime() &&
			DoubleClickClose(gLastWhere, event->where))
		{
			modifiers |= kModifierDoubleClick;
		}
		gLastWindow	= wind;
		gLastWhen		= event->when;
		gLastWhere	= event->where;
	}

	return modifiers;
}
#endif

#include "hsInput.h"

hsModifierKeys hsGetModifierKeys()
{
	hsModifierKeys	modifiers = 0;
#if (HS_BUILD_FOR_WIN32 || HS_BUILD_FOR_MAC)
	hsInput			input;

	if (input.KeyDown(hsInput::kSHIFT))
		modifiers |= kModifierShift;
#endif
	return modifiers;
}

////////////////////////////////////////////////////////////////////////////

#if HS_BUILD_FOR_WIN32
UInt32 hsSWAP32(UInt32 swap)
{
	Byte*	c = (Byte*)&swap;
	Byte		t = c[0];

	c[0] = c[3];
	c[3] = t;
	t = c[1];
	c[1] = c[2];
	c[2] = t;

	return swap;
}
#endif
