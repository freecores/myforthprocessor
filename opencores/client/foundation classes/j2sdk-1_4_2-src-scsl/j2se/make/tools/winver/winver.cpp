/*
 * @(#)winver.cpp	1.3 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

// getver.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <windows.h>

int main(int argc, char* argv[])
{
	OSVERSIONINFO verInfo;

	memset(&verInfo,0,sizeof(verInfo));
	verInfo.dwOSVersionInfoSize = sizeof(verInfo);

	if (GetVersionEx(&verInfo))
	{
		printf("%d %d %s",verInfo.dwMajorVersion,verInfo.dwMinorVersion,verInfo.szCSDVersion);
	}
	else
	{
		printf("No version info available");
	}

	return 0;
}

