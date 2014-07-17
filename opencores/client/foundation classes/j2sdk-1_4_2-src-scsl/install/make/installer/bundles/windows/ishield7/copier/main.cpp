/*
 * @(#)main.cpp	1.3 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
 
#include "windows.h"


// Main entry point.
int APIENTRY
WinMain(HINSTANCE hInstance, 
	HINSTANCE hPrevInstance,
	LPSTR lpCmdLine,
	int nCmdShow)
{
    if (__argc != 3)
	return 1;
    
    return !::CopyFile(__argv[1], __argv[2], FALSE);
}

