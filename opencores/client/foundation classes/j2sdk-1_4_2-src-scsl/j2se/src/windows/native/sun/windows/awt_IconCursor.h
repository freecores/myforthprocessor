/*
 * @(#)awt_IconCursor.h	1.10 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "awt.h"

#ifndef _AWTICONCURSOR_H_
#define _AWTICONCURSOR_H_

typedef struct tagBitmapheader  {
    BITMAPINFOHEADER bmiHeader;
    DWORD            dwMasks[256];
}   Bitmapheader, *LPBITMAPHEADER;

HBITMAP create_BMP(HWND hW,int* imageData,int nSS, int nW, int nH);

void destroy_BMP(HBITMAP hBMP);

#endif // _AWTICONCURSOR_H_
