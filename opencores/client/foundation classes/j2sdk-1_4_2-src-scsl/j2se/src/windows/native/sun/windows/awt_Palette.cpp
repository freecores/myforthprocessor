/*
 * @(#)awt_Palette.cpp	1.11 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include <windows.h>
#include "awt_Palette.h"
#include "awt_Component.h"
#include "img_util_md.h"
#include "awt_CustomPaletteDef.h"

BOOL AwtPalette::m_useCustomPalette = TRUE;

#define ERROR_GRAY (-1)
#define NON_GRAY 0
#define LINEAR_STATIC_GRAY 1
#define NON_LINEAR_STATIC_GRAY 2

/**
 * Select the palette into the given HDC.  This will
 * allow operations using this HDC to access the palette
 * colors/indices.
 */
HPALETTE AwtPalette::Select(HDC hDC) 
{
    HPALETTE prevPalette = NULL;
    if (logicalPalette) {
	BOOL background = !(m_useCustomPalette);
	prevPalette = ::SelectPalette(hDC, logicalPalette, background);
    }
    return prevPalette;
}

/**
 * Realize the palette of the given HDC.  This will attempt to 
 * install the palette of the HDC onto the device associated with
 * that HDC.
 */
void AwtPalette::Realize(HDC hDC) 
{
    if (logicalPalette) { 
	if (!m_useCustomPalette ||
	    AwtComponent::QueryNewPaletteCalled() ||
	    AwtToolkit::GetInstance().HasDisplayChanged()) {
	    //!AwtWin32GraphicsDevice::IsPrimaryPalettized()) {
	    // Fix for bug 4178909, workaround for Windows bug.  Shouldn't
	    // do a RealizePalette until the first QueryNewPalette message
	    // has been processed.
	    // But if we are switching the primary monitor from non-8bpp
	    // to 8bpp mode, we may not get any palette messages during
	    // the display change event.  Go ahead and realize the palette
	    // now anyway in this situation.  This was especially noticeable
	    // on win2k in multimon.  Note that there still seems to be some
	    // problem with actually setting the palette on the primary
	    // screen until after QNP is called, but at least the 
	    // secondary devices can correctly realize the palette.
	    ::RealizePalette(hDC);
	}
    }
}

/**
 * Disable the use of our custom palette.  This method is called
 * during initialization if we detect that we are running inside
 * the plugin; we do not want to clobber our parent application's
 * palette with our own in that situation.
 */
void AwtPalette::DisableCustomPalette() 
{
    m_useCustomPalette = FALSE;
}

/**
 * Returns whether we are currently using a custom palette.  Used
 * by AwtWin32GraphicsDevice when creating the colorModel of the
 * device.
 */
BOOL AwtPalette::UseCustomPalette() 
{
    return m_useCustomPalette;
}


/**
 * Constructor.  Initialize the system and logical palettes.
 * used by this object.
 */
AwtPalette::AwtPalette(AwtWin32GraphicsDevice *device) 
{
    this->device = device;
    Update();
    UpdateLogical();
}

/**
 * Retrieves system palette entries. Includes a workaround for for some 
 * video drivers which may not support the GSPE call but may return 
 * valid values from this procedure.
 */
int AwtPalette::FetchPaletteEntries(HDC hDC, PALETTEENTRY* pPalEntries)
{
    LOGPALETTE* pLogPal = 0;
    HPALETTE hPal = 0;
    HPALETTE hPalOld = 0;
    int numEntries;

    numEntries = ::GetSystemPaletteEntries(hDC, 0, 256, pPalEntries);
    
    if (numEntries > 0) {
	return numEntries;
    }
    // Workaround: some drivers do not support GetSysPalEntries

    pLogPal = (LOGPALETTE*) new char[sizeof(LOGPALETTE) 
				    + 256*sizeof(PALETTEENTRY)];
    if (pLogPal == NULL) {
	return 0;
    }

    pLogPal->palVersion = 0x300;
    pLogPal->palNumEntries = 256;
    int iEntry;
    PALETTEENTRY* pEntry;
    for (iEntry = 0; iEntry < 256; iEntry++) {
	pEntry = pLogPal->palPalEntry + iEntry;
	pEntry->peRed = iEntry;
	pEntry->peGreen = pEntry->peBlue = 0;
	pEntry->peFlags = PC_EXPLICIT;
    }
    hPal = ::CreatePalette(pLogPal);
    if ( hPal == 0 ) {
	delete pLogPal;
	return 0;
    }

    hPalOld = ::SelectPalette(hDC, hPal, 1);
    if (hPalOld == 0) {
	delete pLogPal;
	::DeleteObject(hPal);
	return 0;
    }
    ::RealizePalette(hDC);

    COLORREF rgb;
    for (iEntry = 0; iEntry < 256; iEntry++) {
    	rgb = ::GetNearestColor(hDC, PALETTEINDEX(iEntry));
	pPalEntries[iEntry].peRed = GetRValue(rgb);
	pPalEntries[iEntry].peGreen = GetGValue(rgb);
	pPalEntries[iEntry].peBlue = GetBValue(rgb);
    }

    delete pLogPal;
    ::DeleteObject(hPal);
    ::SelectPalette(hDC, hPalOld, 0 );
    ::RealizePalette(hDC);

    return 256;
}

int AwtPalette::GetGSType(PALETTEENTRY* pPalEntries)
{
    int isGray = 1;
    int isLinearStaticGray = 1;
    int isNonLinearStaticGray = 1;
    int iEntry;
    char bUsed[256];
    BYTE r, g, b;

    memset(bUsed, 0, sizeof(bUsed));
    for (iEntry = 0; iEntry < 256; iEntry++) {
	r = pPalEntries[iEntry].peRed;
	g = pPalEntries[iEntry].peGreen;
	b = pPalEntries[iEntry].peBlue;
	if (r != g || r != b) {
	    isGray = 0;
	    break;
	} else {
	    // the values are gray
	    if (r != iEntry) {
		// it's not linear
		// but it could be non-linear static gray
		isLinearStaticGray = 0;
	    }
	    bUsed[r] = 1;
	}
    }

    if (isGray && !isLinearStaticGray) {
	// check if all 256 grays are there
	// if that's the case, it's non-linear static gray
	for (iEntry = 0; iEntry < 256; iEntry++ ) {
	    if (!bUsed[iEntry]) {
		// not non-linear (not all 256 colors are used)
		isNonLinearStaticGray = 0;
		break;
	    }
	}
    }

    if (!isGray) { return NON_GRAY; }
    if (isLinearStaticGray) { return LINEAR_STATIC_GRAY; }
    if (isNonLinearStaticGray) { return NON_LINEAR_STATIC_GRAY; }
    
    // not supposed to be here, error
    return ERROR_GRAY;
}

/**
 * Updates our system palette variables to make sure they match
 * the current state of the actual system palette.  This method
 * is called during AwtPalette creation and after palette changes.
 * Return whether there were any palette changes from the previous
 * system palette.
 */
BOOL AwtPalette::Update() 
{
    PALETTEENTRY pe[256];
    int numEntries = 0;
    int bitsPerPixel;
    int i;
    HDC hDC;
 
    hDC = device->GetDC();    
    if (!hDC) {
	return FALSE;
    }
    bitsPerPixel = ::GetDeviceCaps(hDC, BITSPIXEL);
    device->ReleaseDC(hDC);
    if (8 != bitsPerPixel) {	
	return FALSE;
    }

    /* 
     * Note that here we are using the primary display's DC to get
     * our system palette entries.  This is contrary to intuition since
     * one would expect each device to have its own palette.  But apparently
     * the system palette is always stored or associated with the primary
     * display, even when that display is in a non-8-bit mode.  This 
     * was discovered mainly through experimentation; the docs on multimon
     * are very sketchy.
     */
    if (AwtWin32GraphicsDevice::IsPrimaryPalettized()) {
	hDC = ::GetDC(NULL);
    } else {
	hDC = device->GetDC();
    }

    numEntries = FetchPaletteEntries(hDC, pe);

    if (AwtWin32GraphicsDevice::IsPrimaryPalettized()) {
	::ReleaseDC(NULL, hDC);
    } else {
	// Workaround for palette weirdness: When primary is not
	// in 8-bit mode but other devices are, normal palette usage
	// does not work.  The only thing we can depend upon is the
	// first and last 8 colors of the system palette.  Set all 
	// other entries to 0 in our cache of the system palette
	// so that we do not use other values in our color calculations.
	for (i = 8; i < 248; i++) {
	    pe[i].peRed = 0;
	    pe[i].peGreen = 0;
	    pe[i].peBlue = 0;
	}
	device->ReleaseDC(hDC);
    }

    if ((numEntries == numSystemEntries) &&
	(0 == memcmp(pe, systemEntriesWin32, numEntries * sizeof(PALETTEENTRY)))) 
    {
	return FALSE;
    }

    // make this system palette the new cached win32 palette
    numEntries = (numEntries > 256)? 256: numEntries;
    memcpy(systemEntriesWin32, pe, numEntries * sizeof(PALETTEENTRY));
    numSystemEntries = numEntries;

    // Create jdk-style system palette
    int startIndex = 0, endIndex = numEntries-1;
    char ebuf[4];
    int staticGrayType = GetGSType(systemEntriesWin32);

    if (staticGrayType == LINEAR_STATIC_GRAY) {
	device->SetGrayness(GS_STATICGRAY);
    } else if (staticGrayType == NON_LINEAR_STATIC_GRAY) {
	device->SetGrayness(GS_NONLINGRAY);
    } else if (GetEnvironmentVariable("FORCEGRAY", ebuf, 0)) {
	// Need to zero first and last ten
	// palette entries. Otherwise in UpdateDynamicColorModel
	// we could set non-gray values to the palette.
	for (i = 0; i < 10; i++) {
	    systemEntries[i] = 0x00000000;
	    systemEntries[i+246] = 0x00000000;
	}
	numEntries -= 20;
	startIndex = 10;
	endIndex -= 10;
	device->SetGrayness(GS_INDEXGRAY);
    } else {
	device->SetGrayness(GS_NOTGRAY);
    }

    for (i = startIndex; i <= endIndex; i++) {
	systemEntries[i] =  0xff000000
			| (pe[i].peRed << 16)
			| (pe[i].peGreen << 8)
			| (pe[i].peBlue);
    }

    systemInverseLUT =
	initCubemap((int *)systemEntries, numEntries, 32);

    ColorData *cData = device->GetColorData();
    if ((device->GetGrayness() == GS_NONLINGRAY || 
	 device->GetGrayness() == GS_INDEXGRAY) && 
	cData != NULL) {

	if (cData->pGrayInverseLutData != NULL) {
	    free(cData->pGrayInverseLutData);
	    cData->pGrayInverseLutData = NULL;
	}
	initInverseGrayLut((int*)systemEntries, 256, device->GetColorData());
    }

    return TRUE;
}


/**
 * Creates our custom palette based on: the current system palette, 
 * the grayscale-ness of the system palette, and the state of the
 * primary device.
 */
void AwtPalette::UpdateLogical()
{
    // Create and initialize a palette
    int nEntries = 256;
    char *buf = NULL;
    buf = new char[sizeof(LOGPALETTE) + nEntries *
	sizeof(PALETTEENTRY)];
    
    LOGPALETTE *pLogPal = (LOGPALETTE*)buf;
    PALETTEENTRY *pPalEntries = (PALETTEENTRY *)(&(pLogPal->palPalEntry[0])); 
    
    memcpy(pPalEntries, systemEntriesWin32, 256 * sizeof(PALETTEENTRY));

    PALETTEENTRY *pPal = pPalEntries;
    int i;

    if (!AwtWin32GraphicsDevice::IsPrimaryPalettized()) {
	// Workaround for palette weirdness: When primary is not
	// in 8-bit mode but other devices are, normal palette usage
	// does not work.  The only thing we can depend upon is the
	// first and last 8 colors of the system palette.  Set all 
	// other entries to 0 in our cache of the logical palette
	// so that we do not use other values in our color calculations.
	for (i = 8; i < 248; i++) {
	    pPalEntries[i].peRed = 0;
	    pPalEntries[i].peGreen = 0;
	    pPalEntries[i].peBlue = 0;
	}
    } else {
	int staticGrayType = device->GetGrayness();
	if (staticGrayType == GS_INDEXGRAY) {
	    float m = 255.0f / 235.0f;
	    float g = 0.5f;
	    pPal = &pPalEntries[10];
	    for (i = 10; i < 246; i++, pPal++) {
		pPal->peRed = pPal->peGreen = pPal->peBlue =
		    (int)g;
		g += m;
		pPal->peFlags = PC_NOCOLLAPSE;
	    }
	} else if (staticGrayType == GS_NOTGRAY) {
	    for (i = 10; i < 246; i++) {
		pPalEntries[i] = customPalette[i-10];
	    }
	}
    }
    pLogPal->palNumEntries = 256;
    pLogPal->palVersion = 0x300;
    logicalPalette = ::CreatePalette(pLogPal);
    
    for (i = 0; i < nEntries; i++) {
	logicalEntries[i] =  0xff000000
			| (pPalEntries[i].peRed << 16)
			| (pPalEntries[i].peGreen << 8)
			| (pPalEntries[i].peBlue);
    }
    delete [] buf;    
}


