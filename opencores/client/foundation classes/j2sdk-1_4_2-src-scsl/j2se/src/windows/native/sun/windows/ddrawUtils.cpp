/*
 * @(#)ddrawUtils.cpp	1.48 03/04/28
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#define INITGUID
#include "ddrawUtils.h"
#include "ddrawObject.h"
#include "awt_Multimon.h"
#include "awt_MMStub.h"
#include "d3dTestRaster.h"

//
// Globals
//
static DDrawObjectStruct    **ddInstance;
static int		    maxDDDevices = 0;
int			    currNumDevices = 0;
static CriticalSection	    ddInstanceLock;
BOOL			    useDD = FALSE;	// ddraw enabled flag
BOOL			    useD3D = FALSE;
BOOL			    forceD3DUsage = FALSE;
int			    j2dTraceLevel = -1;
extern BOOL                 isAppActive;
HINSTANCE		    hLibDDraw = NULL; // DDraw Library handle
extern                      jfieldID ddSurfacePuntedID;


//
// Constants
//
#define MAX_BUSY_ATTEMPTS 50	// Arbitrary number of times to attempt
				// an operation that returns a busy error
#define J2D_D3D_LINES_FAIL 		0
#define J2D_D3D_LINES_OK 		1
#define J2D_D3D_LINE_CLIPPING_OK 	2
//
// Macros
//

/**
 * This macro is just a shortcut for the various places in the
 * code where we want to call a ddraw function and print any error 
 * if the result is not equal to DD_OK.  The errorString passed
 * in is for debugging/tracing purposes only.
 */
#define DD_FUNC(func, errorString) { \
    HRESULT ddResult = func; \
    if (ddResult != DD_OK) { \
	DebugPrintDirectDrawError(ddResult, errorString); \
    } \
}
/**
 * Same as above, only return FALSE when done (to be used only in
 * functions that should return FALSE on a ddraw failure).
 */
#define DD_FUNC_RETURN(func, errorString) { \
    HRESULT ddResult = func; \
    if (ddResult != DD_OK) { \
	DebugPrintDirectDrawError(ddResult, errorString); \
	return FALSE; \
    } \
}

// Note that J2dTraceImpl is only called within DEBUG code
void J2dTraceImpl(int level, BOOL cr, const char *string, ...)
{
    if (j2dTraceLevel < J2D_TRACE_OFF) {
	// need to init trace level
	j2dTraceLevel = J2D_TRACE_OFF;
	char *j2dTraceLevelString = getenv("J2D_TRACE_LEVEL");
	if (j2dTraceLevelString) {
	    int traceLevelTmp = -1;
	    int args = sscanf(j2dTraceLevelString, "%d", &traceLevelTmp);
	    if (args > 0 && 
	    	traceLevelTmp > J2D_TRACE_OFF && 
	    	traceLevelTmp <= J2D_TRACE_VERBOSE) 
	    {
		j2dTraceLevel = traceLevelTmp;
	    }
	}
    }
    va_list args;
    if (level <= j2dTraceLevel) {
	va_start(args, string);
	vprintf(string, args);
	va_end(args);
	if (cr) {
	    printf("\n");
	}
    }
}


//
// INLINE functions
//

// Attaches the clipper object of a given surface to 
// the primary.  Note that this action only happens if the
// surface is onscreen (clipping only makes sense for onscreen windows)
INLINE void AttachClipper(Win32SDOps *wsdo) {
    if (wsdo->window && wsdo->ddInstance->hwndFullScreen == NULL) {
	HRESULT ddResult;
	ddResult = wsdo->ddInstance->clipper->SetHWnd(0, wsdo->window);
	if (ddResult != DD_OK) {
	    DebugPrintDirectDrawError(ddResult, "AttachClipper");
	}
    }
}

//
// Functions
//

/**
 * Returns the ddInstance associated with a particular HMONITOR
 */
DDrawObjectStruct *GetDDInstanceForDevice(HMONITOR hMon)
{
    DDrawObjectStruct *tmpDdInstance = NULL;
    ddInstanceLock.Enter();
    if (currNumDevices == 1) {
	// Non multimon situation
	if (ddInstance[0]) 
	{
	    tmpDdInstance = ddInstance[0];
	}
    } else {
	for (int i = 0; i < currNumDevices; ++i) {
	    if (ddInstance[i]
                && hMon == ddInstance[i]->hMonitor) 
	    {
		tmpDdInstance = ddInstance[i];
		break;
	    }
	}
    }
    ddInstanceLock.Leave();
    return tmpDdInstance;
}

/**
 * Can return FALSE if there was some problem during ddraw
 * initialization for this screen, or if this screen does not
 * support some of the capabilities necessary for running ddraw
 * correctly.
 */
BOOL DDCanCreatePrimary(HMONITOR hMon) {
    DDrawObjectStruct *tmpDdInstance = GetDDInstanceForDevice(hMon);
    return (useDD && ddInstance && tmpDdInstance);
}

/**
 * Can return FALSE if the device that the surfaceData object
 * resides on cannot support accelerated Blt's.  Some devices
 * can perform basic ddraw Lock/Unlock commands but cannot
 * handle the ddraw Blt command.
 */
BOOL DDCanBlt(Win32SDOps *wsdo) {
    return (useDD && wsdo->ddInstance && wsdo->ddInstance->canBlt);
}

/**
 * Can return FALSE if either ddraw is not enabled at all (problems
 * during initialization) or the device that the surfaceData object
 * resides on cannot support the basic required capabilities (in
 * which case the ddInstance for that device will be set to NULL).
 */
BOOL DDUseDDraw(Win32SDOps *wsdo) {
    return (useDD && wsdo->ddInstance && wsdo->ddInstance->valid);
}


/**
 * Can return FALSE if our check for d3d capabilities on this
 * device failed (See D3DTest()).
 */
BOOL D3DEnabled(Win32SDOps *wsdo) {
    return (wsdo->ddInstance && wsdo->ddInstance->canDrawD3dLines);
}

/**
 * Initialize ddraw library and global ddraw objects.  We load the
 * library and create the necessary ddraw object and primary surface.
 */
BOOL DDInit()
{
    DTRACE_PRINTLN("DDInit");

    // Grab the ddInstance lock to prevent simultaneous init operations
    ddInstanceLock.Enter();
    // Only initialize if not already initialized
    if (!useDD) {
	hLibDDraw = ::LoadLibrary("ddraw.dll");
	if (!hLibDDraw) {
	    DTRACE_PRINTLN("Could not load library");
	    return FALSE;
	}
	maxDDDevices = 1;
	ddInstance = (DDrawObjectStruct**)safe_Malloc(maxDDDevices * 
	    sizeof(DDrawObjectStruct*));
	if (!DDCreateObject()) {
	    ddInstanceLock.Leave();
	    return FALSE;
	}
	useDD = TRUE;
    }
    ddInstanceLock.Leave();
    return TRUE;
}


/**
 * Release the resources consumed by ddraw.  This will be called 
 * by the DllMain function when it receives a PROCESS_DETACH method,
 * meaning that the application is done with awt.  We need to release
 * these ddraw resources because of potential memory leaks, but
 * more importantly, because if we don't release a primary surface
 * that has been locked and not unlocked, then we may cause
 * ddraw to be corrupted on this system until reboot.
 * IMPORTANT: because we do not use any locks around this release,
 * we assume that this function is called only during the 
 * PROCESS_DETACH procedure described above.  Any other situation
 * could cause unpredictable results.
 */   
void DDRelease()
{
    DTRACE_PRINTLN("DDRelease");
 
    // Note that we do not lock the ddInstanceLock CriticalSection.
    // Normally we should do that in this kind of situation (to ensure
    // that the ddInstance used in all release calls is the same on).
    // But in this case we do not want the release of a locked surface
    // to be hampered by some bad CriticalSection deadlock, so we
    // will just release ddInstance anyway.
    // Anyway, if users of this function call it properly (as 
    // documented above), then there should be no problem.
    try {
	if (hLibDDraw) {
	    ::FreeLibrary(hLibDDraw);
	    hLibDDraw = NULL;
	}
	hLibDDraw = NULL;
	if (ddInstance) {
	    for (int i = 0; i < currNumDevices; ++i) {
		ReleaseDDInstance(ddInstance[i]);
	    }
	    free(ddInstance);
	}
    } catch (...) {
	// Handle all exceptions by simply returning.
	// There are some cases where the OS may have already
	// released our objects for us (e.g., NT4) and we have
	// no way of knowing, but the above call into Release will
	// cause an exception to be thrown by dereferencing
	// already-released ddraw objects
    }
}


/**
 * Output test raster (produced in D3DTest function).  Utility
 * used in debugging only.  Enable by setting J2D_TRACE_LEVEL=J2D_VERBOSE
 * prior to running application with java_g.  The output from this will
 * be seen only if D3DTest fails.
 */
void TestRasterOutput(byte *rasPtr, int x, int y, int w, int h, 
		      int scanStride, int pixelStride)
{
    for (int traceRow = y; traceRow < h; ++traceRow) {
	byte *tmpRasPtr = rasPtr + traceRow * scanStride;
	for (int traceCol = x; traceCol < w; ++traceCol) {
	    DWORD pixelVal;
	    switch (pixelStride) {
	    case 1:
		pixelVal = *tmpRasPtr;
		break;
	    case 2: 
		pixelVal = *((unsigned short*)tmpRasPtr);
		break;
	    default: 
		pixelVal = *((unsigned int*)tmpRasPtr);
		break;
	    }
	    tmpRasPtr += pixelStride;
	    if (pixelVal) {
		J2dTrace(J2D_TRACE_VERBOSE, "1");
	    } else {
		J2dTrace(J2D_TRACE_VERBOSE, "0");
	    }
	}
	J2dTrace(J2D_TRACE_VERBOSE, "\n");
    }
}

/**
 * Test whether we should enable d3d rendering on this device.
 * This includes checking whether there were problems creating
 * the necessary offscreen surface, problems during any of the
 * rendering calls (Blts and d3d lines) and any rendering artifacts
 * caused by d3d lines.  The rendering artifact tests are
 * performed by checking a pre-rendered test pattern (produced
 * by our software renderer) against that same pattern rendered
 * on this device.  If there are any pixels which differ between
 * the two patterns we disable d3d line rendering on the device.
 * Differences in the test pattern rendering can be caused
 * by different rendering algorithms used by our software
 * renderer and the driver or hardware on this device.  For example,
 * some Intel cards (e.g., i815) are known to use polygon renderers
 * for their lines, which sometimes result in wide lines.  
 * The test pattern is stored in d3dTestRaster.h, which is generated
 * by a Java test program 
 * (src/share/test/java2d/VolatileImage/Lines/LinePattern.java).
 */
int D3DTest(DDrawObjectStruct *tmpDdInstance)
{
    DTRACE_PRINTLN("D3DTest");
    HRESULT dxResult;
    // note the "d3dTestRasterH + 2" value; we are adding an 
    // additional clip test at the bottom of the test raster
    DDrawSurface *lpSurface = 
	tmpDdInstance->ddObject->CreateDDOffScreenSurface(d3dTestRasterW, 
            (d3dTestRasterH + 2), 32/*doesn't matter*/, TRUE, TR_OPAQUE, 
            DDSCAPS_VIDEOMEMORY);
    if (!lpSurface) {
	return J2D_D3D_LINES_FAIL;
    }
    DDBLTFX ddBltFx;
    ddBltFx.dwSize = sizeof(ddBltFx);
    ddBltFx.dwFillColor = 0;
    dxResult = lpSurface->Blt(NULL, NULL, NULL, DDBLT_COLORFILL | DDBLT_WAIT,
			      &ddBltFx);
    if (dxResult != D3D_OK) {
	DebugPrintDirectDrawError(dxResult, "D3DTest erasing background");
	delete lpSurface;
	return J2D_D3D_LINES_FAIL;
    }
    int color = 0xffffffff;
    int i;
    int vpLeft = 0;
    int vpTop = 0;
    int vpRight = d3dTestRasterW;
    int vpBottom = d3dTestRasterH;
    for (i = 0; i < d3dNumTestLines * 4; i += 4) {
	static D3DTLVERTEX lineVerts[] = {
	    D3DTLVERTEX(D3DVECTOR(0, 0, .1f), 1.0f, color, 0, 0, 0),
	    D3DTLVERTEX(D3DVECTOR(0, 0, .1f), 1.0f, color, 0, 0, 0),
	};
	lineVerts[0].sx = (float)d3dTestLines[i];
	lineVerts[0].sy = (float)d3dTestLines[i + 1];
	lineVerts[1].sx = (float)d3dTestLines[i + 2];
	lineVerts[1].sy = (float)d3dTestLines[i + 3];
	dxResult = lpSurface->DrawLineStrip(lineVerts, 2, TRUE, 
					    vpLeft, vpTop, 
					    vpRight, vpBottom);
	if (dxResult != D3D_OK) {
	    DebugPrintDirectDrawError(dxResult, "D3DTest line drawing");
	    delete lpSurface;
	    return J2D_D3D_LINES_FAIL;
	}
    }
    for (i = 0; i < d3dNumTestRects * 4; i += 4) {
	static D3DTLVERTEX lineVerts[5] = {
	    D3DTLVERTEX(D3DVECTOR(0, 0, .1f), 1.0f, color, 0, 0, 0),
	    D3DTLVERTEX(D3DVECTOR(0, 0, .1f), 1.0f, color, 0, 0, 0),
	    D3DTLVERTEX(D3DVECTOR(0, 0, .1f), 1.0f, color, 0, 0, 0),
	    D3DTLVERTEX(D3DVECTOR(0, 0, .1f), 1.0f, color, 0, 0, 0),
	    D3DTLVERTEX(D3DVECTOR(0, 0, .1f), 1.0f, color, 0, 0, 0),
	};
	int x = d3dTestRects[i];
	int y = d3dTestRects[i + 1];
	int width = d3dTestRects[i + 2];
	int height = d3dTestRects[i + 3];
	lineVerts[0].sx = (float)x;
	lineVerts[0].sy = (float)y;
	lineVerts[1].sx = (float)x + width;
	lineVerts[1].sy = (float)y;
	lineVerts[2].sx = (float)x + width;
	lineVerts[2].sy = (float)y + height;
	lineVerts[3].sx = (float)x;
	lineVerts[3].sy = (float)y + height;
	lineVerts[4].sx = (float)x;
	lineVerts[4].sy = (float)y;
	HRESULT dxResult = lpSurface->DrawLineStrip(lineVerts, 5, FALSE,
						    vpLeft, vpTop, 
						    vpRight, vpBottom);
	if (dxResult != D3D_OK) {
	    DebugPrintDirectDrawError(dxResult, "D3DTest rect drawing");
	    delete lpSurface;
	    return J2D_D3D_LINES_FAIL;
	}
    }
    // Now for an additional clipping check
    // Draw the same nearly-horizontal line several times; unclipped
    // at first and then with progressively smaller clip areas.  The 
    // pixels filled should be the same every time.  In particular, there
    // should only be one pixel per column.  A clipping error (which
    // could indicate either integer truncation at the clip bounds or
    // a floating point precision problem for lines; we've seen both
    // on different hardware and with different drivers) results in
    // more than one pixel being filled in y as the line steps down
    // at different points on the line based on different clipping.
    // Ideally, the line should look like this:
    //          xxxxxxxxxxxxxxxxxxxxx
    //                               xxxxxxxxxxxxxxxxx
    // A flawed clipping approach might end up filling pixels like so:
    //          xxxxxxxxxxxxxxxxxxxxxxxxx
    //                               xxxxxxxxxxxxxxxxx
    // To check for errors, lock the surface and check that each y value
    // has only one filled pixel.
    for (int testNum = 0; testNum < 20; ++testNum) {
	int vpLeft = testNum;
	int vpTop = 0;
	int vpRight = d3dTestRasterW;
	int vpBottom = d3dTestRasterH + 2; // viewport must include last 2 rows
    
	static D3DTLVERTEX lineVerts[] = {
	    D3DTLVERTEX(D3DVECTOR(0, 0, .1f), 1.0f, color, 0, 0, 0),
	    D3DTLVERTEX(D3DVECTOR(0, 0, .1f), 1.0f, color, 0, 0, 0),
	};
	lineVerts[0].sx = (float)0;
	lineVerts[0].sy = (float)d3dTestRasterH;
	lineVerts[1].sx = (float)d3dTestRasterW - 1;
	lineVerts[1].sy = (float)d3dTestRasterH + 1;
	dxResult = lpSurface->DrawLineStrip(lineVerts, 2, TRUE, 
					    vpLeft, vpTop, 
					    vpRight, vpBottom);
	if (dxResult != D3D_OK) {
	    DebugPrintDirectDrawError(dxResult, "D3DTest line drawing");
	    delete lpSurface;
	    return J2D_D3D_LINES_FAIL;
	}
    }
    // Now, check the results of the test raster against our d3d drawing
    SurfaceDataRasInfo rasInfo;
    dxResult = lpSurface->Lock(NULL, &rasInfo, DDLOCK_WAIT, NULL);
    if (dxResult != DD_OK) {
	delete lpSurface;
	return J2D_D3D_LINES_FAIL;
    }
    byte *rasPtr = (byte*)rasInfo.rasBase;
    int pixelStride = rasInfo.pixelStride;
    int scanStride = rasInfo.scanStride;
    for (int row = 0; row < d3dTestRasterH; ++row) {
	byte *tmpRasPtr = rasPtr + row * scanStride;
	for (int col = 0; col < d3dTestRasterW; ++col) {
	    DWORD pixelVal;
	    switch (pixelStride) {
	    case 1:
		pixelVal = *tmpRasPtr;
		break;
	    case 2: 
		pixelVal = *((unsigned short*)tmpRasPtr);
		break;
	    default: 
		pixelVal = *((unsigned int*)tmpRasPtr);
		break;
	    }
	    tmpRasPtr += pixelStride;
	    // The test is simple: if the test raster pixel has value 0, then
	    // we expect 0 in the d3d surface.  If the test raster has a nonzero
	    // value, then we expect the d3d surface to have that same value.
	    // All other results represent failure.
	    if ((d3dTestRaster[row][col] == 0 && pixelVal != 0) ||
		(d3dTestRaster[row][col] != 0 && pixelVal == 0))
	    {
		J2dTraceLn3(J2D_TRACE_WARNING, 
			    "d3dlines fail due to value %d at (%d, %d)",
		    	    pixelVal, col, row);
#ifdef DEBUG
		// This section is not necessary, but it might be
		// nice to know why we are failing D3DTest on some
		// systems.  If tracing is enabled, this section will
		// produce an ascii representation of the test pattern,
		// the result on this device, and the pixels that were
		// in error.
		J2dTraceLn(J2D_TRACE_VERBOSE, "TestRaster:");
		TestRasterOutput((byte*)d3dTestRaster, 0, 0, d3dTestRasterW, 
				 d3dTestRasterH, d3dTestRasterW, 1);
		J2dTraceLn(J2D_TRACE_VERBOSE, "D3D Raster:");
		TestRasterOutput(rasPtr, 0, 0, d3dTestRasterW, 
				 d3dTestRasterH, scanStride, pixelStride);
		J2dTraceLn(J2D_TRACE_VERBOSE, "Deltas (x indicates problem pixel):");
		for (int traceRow = 0; traceRow < d3dTestRasterH; ++traceRow) {
		    byte *tmpRasPtr = rasPtr + traceRow * scanStride;
		    for (int traceCol = 0; traceCol < d3dTestRasterW; ++traceCol) {
			DWORD pixelVal;
			switch (pixelStride) {
			case 1:
			    pixelVal = *tmpRasPtr;
			    break;
			case 2: 
			    pixelVal = *((unsigned short*)tmpRasPtr);
			    break;
			default: 
			    pixelVal = *((unsigned int*)tmpRasPtr);
			    break;
			}
			tmpRasPtr += pixelStride;
			if ((d3dTestRaster[traceRow][traceCol] == 0 && 
			     pixelVal != 0) ||
			    (d3dTestRaster[traceRow][traceCol] != 0 && 
			     pixelVal == 0))
			{
			    J2dTrace(J2D_TRACE_VERBOSE, "x");
			} else {
			    J2dTrace(J2D_TRACE_VERBOSE, "-");
			}
		    }
		    J2dTrace(J2D_TRACE_VERBOSE, "\n");
		}
#endif // DEBUG
		lpSurface->Unlock(NULL);
		delete lpSurface;
		return J2D_D3D_LINES_FAIL;
	    }
	}
    }
    // Now check the clipped pixels
    row = d3dTestRasterH;
    byte *currRowPtr = rasPtr + row * scanStride;
    byte *nextRowPtr = rasPtr + (row + 1) * scanStride;
    for (int col = 0; col < d3dTestRasterW; ++col) {
	DWORD currPixelVal, nextPixelVal;
	switch (pixelStride) {
	case 1:
	    currPixelVal = *currRowPtr;
	    nextPixelVal = *nextRowPtr;
	    break;
	case 2: 
	    currPixelVal = *((unsigned short*)currRowPtr);
	    nextPixelVal = *((unsigned short*)nextRowPtr);
	    break;
	default: 
	    currPixelVal = *((unsigned int*)currRowPtr);
	    nextPixelVal = *((unsigned int*)nextRowPtr);
	    break;
	}
	if (currPixelVal == nextPixelVal) {
	    J2dTraceLn1(J2D_TRACE_WARNING, "d3dlines fail clipping at column %d", col);
#ifdef DEBUG
	    // This section is not necessary, but it might be
	    // nice to know why we are failing D3DTest on some
	    // systems.  If tracing is enabled, this section will
	    // produce an ascii representation of the test pattern,
	    // the result on this device, and the pixels that were
	    // in error.
	    J2dTraceLn(J2D_TRACE_VERBOSE, "D3D Raster:");
	    TestRasterOutput(rasPtr, 0, d3dTestRasterH, d3dTestRasterW, 
			     d3dTestRasterH + 2, scanStride, pixelStride);
#endif // DEBUG
	    lpSurface->Unlock(NULL);
	    delete lpSurface;
	    J2dTraceLn(J2D_TRACE_INFO, "D3DTest lines ok, but not clipping");
	    return J2D_D3D_LINES_OK; // lines are ok, but clipping is not
	}
	currRowPtr += pixelStride;		
	nextRowPtr += pixelStride;
    }
    // Success: we can render d3d lines on this device
    lpSurface->Unlock(NULL);
    delete lpSurface;
    J2dTraceLn(J2D_TRACE_INFO, "D3DTest lines and clipping ok");
    return (J2D_D3D_LINES_OK | J2D_D3D_LINE_CLIPPING_OK);
}

BOOL DDSetupDevice(DDrawObjectStruct *tmpDdInstance)
{
    DTRACE_PRINTLN("DDSetupDevice");

    // create primary surface. There is one of these per ddraw object
    tmpDdInstance->primary = tmpDdInstance->ddObject->CreateDDPrimarySurface(
        (DWORD)tmpDdInstance->backBufferCount);
    if (!tmpDdInstance->capsSet) {
	DDCAPS caps;
	tmpDdInstance->ddObject->GetDDCaps(&caps);
	tmpDdInstance->canBlt = (caps.dwCaps & DDCAPS_BLT);
	BOOL canCreateOffscreen = tmpDdInstance->canBlt &&
	    (caps.dwVidMemTotal > 0);
	tmpDdInstance->canDrawD3dLines = forceD3DUsage;
	tmpDdInstance->canClipD3dLines = forceD3DUsage;
	// Only register offscreen creation ok if we can Blt and if there
	// is available video memory.  Otherwise it
	// is useless functionality.  The Barco systems apparently allow
	// offscreen creation but do not allow hardware Blt's
	if (canCreateOffscreen) {
	    // triple-check offscreen capabilities; actually try to create
	    // a surface and see if it succeeds
	    DDrawSurface *lpSurface = 
		tmpDdInstance->ddObject->CreateDDOffScreenSurface(1, 1, 32/*doesn't matter*/,
                    TRUE, TR_OPAQUE, DDSCAPS_VIDEOMEMORY);
	    if (!lpSurface) {
		canCreateOffscreen = FALSE;
	    } else {
		delete lpSurface;
		// Now that we know we can create a surface, let's test its 
		// capabilities for D3D
		if (useD3D && !tmpDdInstance->canDrawD3dLines) {
		    int testResults = D3DTest(tmpDdInstance);
		    tmpDdInstance->canDrawD3dLines = 
		    	(testResults & J2D_D3D_LINES_OK);
		    tmpDdInstance->canClipD3dLines = 
		    	(testResults & J2D_D3D_LINE_CLIPPING_OK);
		}
	    }
	}
	if ((caps.dwCaps & DDCAPS_NOHARDWARE) || !canCreateOffscreen) {
	    AwtWin32GraphicsDevice::DisableOffscreenAccelerationForDevice(
		tmpDdInstance->hMonitor);
	    if (caps.dwCaps & DDCAPS_NOHARDWARE) {
		// Does not have basic functionality we need; release
		// ddraw instance and return FALSE for this device.
		DTRACE_PRINTLN("Disabling ddraw on device: no hw support\n");
		ReleaseDDInstance(tmpDdInstance);
		return FALSE;
	    }
	}
	tmpDdInstance->capsSet = TRUE;
    }
    HRESULT lockResult = DD_OK;
    if (tmpDdInstance->primary) {
        lockResult = tmpDdInstance->primary->Lock(NULL, NULL, DDLOCK_WAIT, NULL);
        if (lockResult == DD_OK) {
            tmpDdInstance->primary->Unlock(NULL);
        } else {
            DebugPrintDirectDrawError(lockResult, "DDSetupDevice Lock");
        }
    }
    if (tmpDdInstance->primary == NULL || lockResult != DD_OK) {
	// Do not release ddInstance here as we can fail for reasons
	// that do not invalidate our use of ddraw on this device for
	// all time
	return FALSE;
    }
    // Do NOT create a clipper in full-screen mode
    if (tmpDdInstance->hwndFullScreen == NULL) {
	tmpDdInstance->clipper = tmpDdInstance->ddObject->CreateDDClipper();
	if (tmpDdInstance->clipper != NULL) {
	    if (tmpDdInstance->primary->SetClipper(tmpDdInstance->clipper)
		!= DD_OK) {
		ReleaseDDInstance(tmpDdInstance);
		return FALSE;
	    }
	} else {
	    ReleaseDDInstance(tmpDdInstance);
	    return FALSE;
	}
    }
    return TRUE;
}

DDrawObjectStruct *CreateDevice(GUID *lpGUID, HMONITOR hMonitor)
{
    DTRACE_PRINTLN2("CreateDevice: lpGUID, hMon = 0x%x, 0x%x\n", lpGUID,
        hMonitor);
    DDrawObjectStruct *tmpDdInstance = 
	(DDrawObjectStruct*)safe_Calloc(1, sizeof(DDrawObjectStruct));
    tmpDdInstance->valid = TRUE;
    tmpDdInstance->capsSet = FALSE;
    tmpDdInstance->hMonitor = hMonitor;
    tmpDdInstance->hwndFullScreen = NULL;
    tmpDdInstance->backBufferCount = 0;
    tmpDdInstance->context = CONTEXT_NORMAL;
    // Create ddraw object
    tmpDdInstance->ddObject = DDraw::CreateDDrawObject(lpGUID);
    if (!tmpDdInstance->ddObject) {
	// REMIND: might want to shut down ddraw (useDD == FALSE?)
	// if this error occurs
	return NULL;
    }
    if (DDSetupDevice(tmpDdInstance)) {
	return tmpDdInstance;
    } else {
	return NULL;
    }
}

BOOL CALLBACK EnumDeviceCallback(GUID FAR* lpGUID, LPSTR szName, LPSTR szDevice, 
				 LPVOID lParam, HMONITOR hMonitor)
{
    if (currNumDevices == maxDDDevices) {
	maxDDDevices *= 2;
	DDrawObjectStruct **tmpDDDevices = 
	    (DDrawObjectStruct**)safe_Malloc(maxDDDevices * 
	    sizeof(DDrawObjectStruct*));
	for (int i = 0; i < currNumDevices; ++i) {
	    tmpDDDevices[i] = ddInstance[i];
	}
	DDrawObjectStruct **oldDDDevices = ddInstance;
	ddInstance = tmpDDDevices;
	free(oldDDDevices);
    }
    if (hMonitor != NULL) {
	DDrawObjectStruct    *tmpDdInstance;
	tmpDdInstance = CreateDevice(lpGUID, hMonitor);
	ddInstance[currNumDevices] = tmpDdInstance;
	DTRACE_PRINTLN2("ddInstance[%d] = 0x%x\n", currNumDevices,
	    tmpDdInstance);
	// Increment currNumDevices on success or failure; a null device
	// is perfectly fine; we may have an unaccelerated device
	// in the midst of our multimon configuration
	currNumDevices++;
    }
    return TRUE;
}

extern HRESULT WINAPI DirectDrawEnumerateExA(LPDDENUMCALLBACKEXA lpCallback,
    LPVOID lpContext, DWORD dwFlags);
typedef HRESULT (WINAPI *FnDDEnumerateFunc)(LPDDENUMCALLBACK cb,
    LPVOID lpContext);

/**
 * Create the primary surface.  Note that we do not take the ddInstanceLock
 * here; we assume that our callers are taking that lock for us.
 */
BOOL DDCreatePrimary(Win32SDOps *wsdo) {
    DTRACE_PRINTLN("DDCreatePrimary");
    BOOL ret = TRUE;

    if (wsdo != NULL && wsdo->device != NULL) {
        HMONITOR hMon;
        hMon = (HMONITOR)wsdo->device->GetMonitor();
        DDrawObjectStruct *tmpDdInstance = GetDDInstanceForDevice(hMon);
        // Check if we need to recreate the primary for this device.
        // If we are in full-screen mode, we do not need to change
        // the primary unless the number of back buffers has changed.
        if (tmpDdInstance == NULL) {
            return FALSE;
	}
        if (tmpDdInstance->hwndFullScreen == NULL ||
            tmpDdInstance->context != CONTEXT_NORMAL) {
            ret = DDSetupDevice(tmpDdInstance);
            tmpDdInstance->context = CONTEXT_NORMAL;
        }
        if (ret) {
            tmpDdInstance->valid = TRUE;
        }
        return ret;
    }
    return ret;
}

/**
 * Create the ddraw object and the global
 * ddInstance structure.  Note that we do not take the ddInstanceLock
 * here; we assume that our callers are taking that lock for us.
 */
BOOL DDCreateObject() {
    LPDIRECTDRAWENUMERATEEX lpDDEnum;
    currNumDevices = 0;
#ifdef UNICODE
    lpDDEnum = (LPDIRECTDRAWENUMERATEEX)
        GetProcAddress(hLibDDraw, "DirectDrawEnumerateExW");
#else UNICODE
    lpDDEnum = (LPDIRECTDRAWENUMERATEEX)
        GetProcAddress(hLibDDraw, "DirectDrawEnumerateExA");
#endif UNICODE
    if (lpDDEnum) {
        HRESULT ddResult = (lpDDEnum)(EnumDeviceCallback, 
            NULL, DDENUM_ATTACHEDSECONDARYDEVICES);
        if (ddResult != DD_OK) {
            DebugPrintDirectDrawError(ddResult, "DDEnumerate");
        }
    } 
    if (currNumDevices == 0) {
        // Either there was no ddEnumEx function or there was a problem during 
        // enumeration; just create a device on the primary.
        ddInstance[currNumDevices++] = CreateDevice(NULL, NULL);
    }
    DTRACE_PRINTLN("DDCreateDDObject done");
    return TRUE;
}


/**
 * Synchronize graphics pipeline by calling Lock/Unlock on primary
 * surface
 */
void DDSync()
{
    int attempts = 0;
    HRESULT ddResult;

    DTRACE_PRINTLN("DDSync");
    // REMIND: need to handle errors here
    for (int i = 0; i < currNumDevices; ++i) {
	DDrawObjectStruct *tmpDdInstance = ddInstance[i];

	if (!tmpDdInstance || !tmpDdInstance->valid) {
	    return;
	}
	// Spin while busy up to some finite number of times
	do {
	    ddResult = tmpDdInstance->primary->Lock(NULL, NULL,
		DDLOCK_WAIT, NULL);
	} while ((ddResult == DDERR_SURFACEBUSY) && 
		 (++attempts < MAX_BUSY_ATTEMPTS));
        if (ddResult == DD_OK) {
	    ddResult = tmpDdInstance->primary->Unlock(NULL);
        }
    }
    DTRACE_PRINTLN("DDSync done");
}


/**
 * Simple clip check against the window of the given surface data.
 * If the clip list is complex or if the clip list intersects
 * the visible region of the window then return FALSE, meaning
 * that the clipping is sufficiently complex that the caller
 * may want to find an alternative means (other than ddraw) of
 * performing an operation.
 */
BOOL DDClipCheck(Win32SDOps *wsdo, RECT *operationRect)
{
    static struct {
	RGNDATAHEADER	rdh;
	RECT		rects[1];
    } rgnData;
    unsigned long rgnSize = sizeof(rgnData);
    HRESULT ddResult;

    if (!wsdo->window) {
	// Offscreen surfaces need no clipping
	return TRUE;
    }
    DTRACE_PRINTLN("DDClipCheck");

    // If ddResult not OK, could be because of a complex clipping region
    // (Our rgnData structure only has space for a simple rectangle region).
    // Thus, we return FALSE and attach the clipper object.
    DDrawObjectStruct *tmpDdInstance = wsdo->ddInstance;
    if (!tmpDdInstance || !tmpDdInstance->valid) {
	return FALSE;
    }
    if (wsdo->window == tmpDdInstance->hwndFullScreen) {
        // Fullscreen surfaces need no clipping
        return TRUE;
    }
    DD_FUNC(tmpDdInstance->clipper->SetHWnd(0, wsdo->window),
	"DDClipCheck: SetHWnd");
    ddResult = tmpDdInstance->clipper->GetClipList(NULL, (RGNDATA*)&rgnData,
	&rgnSize);
    if (ddResult == DDERR_REGIONTOOSMALL) {
	// Complex clipping region
	// REMIND: could be more clever here and actually check operationRect
	// against all rectangles in clipList, but this works for now.
	return FALSE;
    }
    // Check intersection of clip region with operationRect.  If clip region
    // smaller, then we have a simple clip case.
    // If no operationRect, then check against entire window bounds.
    if (operationRect) {
	if (operationRect->left   < rgnData.rects[0].left ||
	    operationRect->top    < rgnData.rects[0].top  ||
	    operationRect->right  > rgnData.rects[0].right ||
	    operationRect->bottom > rgnData.rects[0].bottom) 
	{
	    return FALSE;
	}
    } else {
	RECT winrect;
	::GetWindowRect(wsdo->window, &winrect);
	if (winrect.left   < rgnData.rects[0].left ||
	    winrect.top    < rgnData.rects[0].top  ||
	    winrect.right  > rgnData.rects[0].right ||
	    winrect.bottom > rgnData.rects[0].bottom) 
	{
	    return FALSE;
	}
    }
    return TRUE;
}


/**
 * Lock the surface.  
 */
BOOL DDLock(JNIEnv *env, Win32SDOps *wsdo, RECT *lockRect, 
	    SurfaceDataRasInfo *pRasInfo)
{
    DTRACE_PRINTLN1("DDLock, lpSurf = 0x%x", wsdo->lpSurface);

    int attempts = 0;

    while (attempts++ < MAX_BUSY_ATTEMPTS) {
	if (!wsdo->ddInstance->valid) {
	    // If dd object became invalid, don't bother calling Lock
	    // Note: This check should not be necessary because we should
	    // do the right thing in any case - catch the error, try to 
	    // restore the surface, fai, etc.  But there seem to be problems
	    // with ddraw that sometimes cause it to hang in the Restore and
	    // Lock calls. Better to avoid the situation as much as we can and
	    // bail out early.
	    DTRACE_PRINTLN("DDLock error: wsdo->ddInstance invalid");
	    return FALSE;
	}
	HRESULT ddResult = wsdo->lpSurface->Lock(lockRect, pRasInfo,
	    DDLOCK_WAIT, NULL);
	// Spin on the busy-type errors, else return having failed or succeeded
	switch (ddResult) {
	case DD_OK:
	    return TRUE;
	case DDERR_WASSTILLDRAWING:
	case DDERR_SURFACEBUSY:
	    DTRACE_PRINTLN("DDLock: surface busy...");
	    break;
	case DDERR_SURFACELOST:
	    DTRACE_PRINTLN("DDLock: surface lost");
	    wsdo->RestoreSurface(env, wsdo);
	    return FALSE;
	default:
	    DebugPrintDirectDrawError(ddResult, "DDLock");
	    return FALSE;
	}
    }
    // If we get here, then there was an error in the function and we
    // should return false
    return FALSE;
} 


/**
 * Unlock the surface
 */
void DDUnlock(JNIEnv *env, Win32SDOps *wsdo)
{
    DTRACE_PRINTLN1("DDUnlock: lpSurf = 0x%x", wsdo->lpSurface);

    HRESULT ddResult = wsdo->lpSurface->Unlock(NULL);
    // Spin on the busy-type errors, else return having failed or succeeded
    switch (ddResult) {
    case DD_OK:
	return;
    case DDERR_NOTLOCKED:
	DTRACE_PRINTLN("DDUnlock:Surface not locked");
	return;
    case DDERR_SURFACELOST:
	wsdo->RestoreSurface(env, wsdo);
	return;
    default:
	DebugPrintDirectDrawError(ddResult, "DDUnlock");
	return;
    }
} 


/**
 * Draw single line between 2 points using Direct3D.  We basically
 * stuff the values into a pre-allocated vertex structure and
 * call DDrawSurface::DrawLineStrip().
 * A return value of FALSE means that the primitive did not
 * get rendered and the caller must handle it somehow.
 */
BOOL D3DLine(JNIEnv *env, Win32SDOps *wsdo, int x1, int y1, 
	     int x2, int y2, int clipX1, int clipY1, 
	     int clipX2, int clipY2, jint color)
{
    DTRACE_PRINTLN5("D3DLine: x1, y1, x2, y2, color = %d, %d, %d, %d, 0x%x\n",
	x1, y1, x2, y2, color);
    // Pre-allocate and -populate structures with default values
    static D3DTLVERTEX lineVerts[] = {
	D3DTLVERTEX(D3DVECTOR(0, 0, .1f), 1.0f, 0, 0, 0, 0),
	D3DTLVERTEX(D3DVECTOR(0, 0, .1f), 1.0f, 0, 0, 0, 0),
    };
    lineVerts[0].sx = (float)x1;
    lineVerts[0].sy = (float)y1;
    lineVerts[0].color = color;
    lineVerts[1].sx = (float)x2;
    lineVerts[1].sy = (float)y2;
    lineVerts[1].color = color;
    HRESULT d3dResult = wsdo->lpSurface->DrawLineStrip(lineVerts, 2, TRUE,
    						       clipX1, clipY1, 
    						       clipX2, clipY2);
    if (d3dResult != D3D_OK) {
	DebugPrintDirectDrawError(d3dResult, "D3DLine");
	return FALSE;
    }
    return TRUE;
}

/**
 * Draw a rectangle with the given boundaries using Direct3D.
 * We do this by calling DDrawSurface::DrawLineStrip with
 * a strip of 4 lines.
 */
BOOL D3DRect(JNIEnv *env, Win32SDOps *wsdo, int x, int y, 
	     int width, int height, int clipX1, int clipY1,
	     int clipX2, int clipY2, jint color)
{
    static D3DTLVERTEX lineVerts[5] = {
	D3DTLVERTEX(D3DVECTOR(0, 0, .1f), 1.0f, 0, 0, 0, 0),
	D3DTLVERTEX(D3DVECTOR(0, 0, .1f), 1.0f, 0, 0, 0, 0),
	D3DTLVERTEX(D3DVECTOR(0, 0, .1f), 1.0f, 0, 0, 0, 0),
	D3DTLVERTEX(D3DVECTOR(0, 0, .1f), 1.0f, 0, 0, 0, 0),
	D3DTLVERTEX(D3DVECTOR(0, 0, .1f), 1.0f, 0, 0, 0, 0),
    };

    lineVerts[0].sx = (float)x;
    lineVerts[0].sy = (float)y;
    lineVerts[0].color = color;
    lineVerts[1].sx = (float)x + width;
    lineVerts[1].sy = (float)y;
    lineVerts[1].color = color;
    lineVerts[2].sx = (float)x + width;
    lineVerts[2].sy = (float)y + height;
    lineVerts[2].color = color;
    lineVerts[3].sx = (float)x;
    lineVerts[3].sy = (float)y + height;
    lineVerts[3].color = color;
    lineVerts[4].sx = (float)x;
    lineVerts[4].sy = (float)y;
    lineVerts[4].color = color;
    HRESULT d3dResult = wsdo->lpSurface->DrawLineStrip(lineVerts, 5, FALSE,
    						       clipX1, clipY1, 
    						       clipX2, clipY2);
    if (d3dResult != D3D_OK) {
	DebugPrintDirectDrawError(d3dResult, "D3DRect");
	return FALSE;
    }
    return TRUE;
}

/**
 * Fill given surface with given color in given RECT bounds
 */
BOOL DDColorFill(JNIEnv *env, jobject sData, Win32SDOps *wsdo, 
		 RECT *fillRect, jint color)
{
    DDBLTFX ddBltFx;
    HRESULT ddResult;
    int attempts = 0;

    DTRACE_PRINTLN5("DDColorFill: color, l, t, r, b = 0x%x, %d, %d, %d, %d",
		    color, fillRect->left, fillRect->top, fillRect->right, 
		    fillRect->bottom);
    ddBltFx.dwSize = sizeof(ddBltFx);
    ddBltFx.dwFillColor = color;
    AttachClipper(wsdo);
    while (attempts++ < MAX_BUSY_ATTEMPTS) {
	ddResult = wsdo->lpSurface->Blt(fillRect, NULL, NULL,
					DDBLT_COLORFILL | DDBLT_WAIT,
					&ddBltFx);
	// Spin on the busy-type errors, else return having failed or succeeded
	switch (ddResult) {
	case DD_OK:
	    return TRUE;
	case DDERR_INVALIDRECT:
	    DTRACE_PRINTLN4("Invalid rect for colorfill: %d, %d, %d, %d",
			    fillRect->left, fillRect->top, 
			    fillRect->right, fillRect->bottom);
	    return FALSE;
	case DDERR_SURFACEBUSY:
	    DTRACE_PRINTLN("DDColorFill: surface busy...");
	    break;
	case DDERR_SURFACELOST:
	    DTRACE_PRINTLN("DDColorfill: error, surfaceLost");
	    wsdo->RestoreSurface(env, wsdo);
	    return FALSE;
	default:
	    DebugPrintDirectDrawError(ddResult, "DDColorFill");
	}
    }
    DTRACE_PRINTLN("DDColorFill done");
    return FALSE;
}

void ManageOffscreenSurfaceBlt(JNIEnv *env, Win32SDOps *wsdo)
{
    wsdo->surfacePuntData.pixelsReadSinceBlt = 0;
    if (wsdo->surfacePuntData.numBltsSinceRead >= 
	wsdo->surfacePuntData.numBltsThreshold) 
    {
	if (wsdo->surfacePuntData.usingDDSystem) {
	    if (wsdo->surfacePuntData.lpSurfaceVram->Blt(NULL, 
		    wsdo->surfacePuntData.lpSurfaceSystem, 
		    NULL, DDBLT_WAIT, NULL) == DD_OK) 
	    {
		J2dTraceLn2(J2D_TRACE_INFO,
			    "Unpunting sys to VRAM: 0x%x -> 0x%x\n",
			    wsdo->surfacePuntData.lpSurfaceVram,
			    wsdo->surfacePuntData.lpSurfaceSystem);
		wsdo->lpSurface = wsdo->surfacePuntData.lpSurfaceVram;
		wsdo->surfacePuntData.usingDDSystem = FALSE;
		// Now: double our threshhold to prevent thrashing; we
		// don't want to keep punting and un-punting our surface
		wsdo->surfacePuntData.numBltsThreshold *= 2;
		// Notify the Java level that this surface has
		// been unpunted so that future copies to this surface
		// from accelerated src surfaces will do the right thing.
		jobject sdObject = env->NewLocalRef(wsdo->sdOps.sdObject);
		if (sdObject) {
		    // Only bother with this optimization if the
		    // reference is still valid
		    env->SetBooleanField(sdObject, ddSurfacePuntedID, JNI_FALSE);
		    env->DeleteLocalRef(sdObject);
		}
	    }
	}
    } else {
	wsdo->surfacePuntData.numBltsSinceRead++;
    }
}

/**
 * Copy data from src to dst using src and dst rectangles
 */
BOOL DDBlt(JNIEnv *env, Win32SDOps *wsdoSrc, Win32SDOps *wsdoDst,
	       RECT *rDst, RECT *rSrc, CompositeInfo *compInfo)
{
    int attempts = 0;
    DWORD bltFlags = DDBLT_WAIT;

    DTRACE_PRINT("DDBlt: ");
    DTRACE_PRINTLN4("  src rect: %d, %d, %d, %d", 
		    rSrc->left, rSrc->top, rSrc->right, rSrc->bottom);
    DTRACE_PRINTLN4("  dst rect: %d, %d, %d, %d", 
		    rDst->left, rDst->top, rDst->right, rDst->bottom);

    // Note: the primary can only have one clipper attached to it at 
    // any time.  This seems weird to set it to src then dst, but this
    // works because either: both are the same window (devCopyArea), 
    // neither are windows (both offscreen), or only one is a window
    // (Blt).  We can't get here from a windowA -> windowB copy operation.
    AttachClipper(wsdoSrc);
    AttachClipper(wsdoDst);

    // Administrate system-surface punt mechanism for offscreen images
    if (!wsdoSrc->window && !wsdoSrc->surfacePuntData.disablePunts) {
	ManageOffscreenSurfaceBlt(env, wsdoSrc);
    }
    if (wsdoSrc->transparency == TR_BITMASK) {
	bltFlags |= DDBLT_KEYSRC;
    }
    while (attempts++ < MAX_BUSY_ATTEMPTS) {
	HRESULT ddResult = DD_OK;
	if (wsdoSrc->transparency == TR_TRANSLUCENT) {
	    ddResult = wsdoDst->lpSurface->TextureBlt(rSrc, rDst, 
						      wsdoSrc->lpSurface,
						      compInfo);

	} else {
	    ddResult = wsdoDst->lpSurface->Blt(rDst, wsdoSrc->lpSurface, 
					       rSrc, bltFlags, NULL);
	}

	// Spin on the busy-type errors or return having failed or succeeded
	switch (ddResult) {
	case DD_OK:
	    return TRUE;
	case DDERR_SURFACEBUSY:
	    DTRACE_PRINTLN("DDBlt: surface busy...");
	    break;
	case DDERR_SURFACELOST:
	    wsdoDst->RestoreSurface(env, wsdoDst);
	    // Can't nest exceptions - only call Restore on src if the
	    // restore on dst didn't throw an exception
	    if (!safe_ExceptionOccurred(env)) {
		wsdoSrc->RestoreSurface(env, wsdoSrc);
	    } else {
		// At least want to set the surfaceLost flag on the src
		wsdoSrc->surfaceLost = TRUE;
	    }
	    return FALSE;
	default:
	    DebugPrintDirectDrawError(ddResult, "DDBlt");
	    return FALSE;
	}
    }
    return FALSE;
}

/**
 * Set the color key information for this surface.  During a 
 * blit operation, pixels of the specified color will not be 
 * drawn (resulting in transparent areas of the image).  Note
 * that the "transparency" field in the Win32SDOps structure must
 * be set to TR_BITMASK for the color key information to have an effect.
 */
void DDSetColorKey(JNIEnv *env, Win32SDOps *wsdo, jint color)
{
    DDCOLORKEY    ddck;
    HRESULT       ddResult;

    ddck.dwColorSpaceLowValue  = color;
    ddck.dwColorSpaceHighValue = color;

    ddResult = wsdo->lpSurface->SetColorKey(DDCKEY_SRCBLT, &ddck);

    if (ddResult != DD_OK) {
	DebugPrintDirectDrawError(ddResult, "DDSetColorKey");
    }
}


/**
 * Swaps the surface memory of the front and back buffers.
 * Flips memory from the source surface to the destination surface.
 */
BOOL DDFlip(JNIEnv *env, Win32SDOps *src, Win32SDOps *dest)
{
    int attempts = 0;
    while (attempts++ < MAX_BUSY_ATTEMPTS) {
	HRESULT ddResult = src->lpSurface->Flip(dest->lpSurface);
	// Spin on the busy-type errors or return having failed or succeeded
	switch (ddResult) {
	case DD_OK:
	    return TRUE;
	case DDERR_SURFACEBUSY:
	    DTRACE_PRINTLN("DDFlip: surface busy...");
	    break;
	case DDERR_SURFACELOST:
	    src->RestoreSurface(env, src);
	    return FALSE;
	default:
	    DebugPrintDirectDrawError(ddResult, "DDFlip");
	    return FALSE;
	}
    }
    return FALSE;
}


/**
 * Mark the given ddInstance structure as invalid.  This flag
 * can then be used to detect rendering with an invalid ddraw
 * object later (to avoid further ddraw errors) or to detect
 * when it is time to create a new ddraw object.  Recreation
 * happens when we are asked to create a new surface but the
 * current ddInstance global structure is invalid.
 */
void DDInvalidateDDInstance(DDrawObjectStruct *ddInst) {
    if (useDD) {
	if (ddInst != NULL) {
	    // Invalidate given instance of ddInstance
	    ddInst->valid = FALSE;
	} else {
	    // Invalidate global ddInstance.  This occurs at the start
	    // of a display-change event.
	    for (int i = 0; i < currNumDevices; ++i) {
		if (ddInstance[i] && ddInstance[i]->hwndFullScreen == NULL) {
		    ddInstance[i]->valid = FALSE;
		}
	    }
	}
    }
}

/**
 * Utility routine: release all elements of given ddInst structure
 * and free the memory consumed by ddInst.  Note that this may be 
 * called during a failed DDCreateDDObject, so any null fields were
 * not yet initialized and should not be released.
 */
void ReleaseDDInstance(DDrawObjectStruct *ddInst)
{
    if (ddInst) {
	if (ddInst->primary) {
	    delete ddInst->primary;
	    ddInst->primary = NULL;
	}
	if (ddInst->clipper) {
	    delete ddInst->clipper;
	    ddInst->clipper = NULL;
	}
	if (ddInst->ddObject) {
	    delete ddInst->ddObject;
	    ddInst->ddObject = NULL;
	}
	free(ddInst);
    }
}

/**
 * Enters full-screen exclusive mode, setting the hwnd as the screen
 */
BOOL DDEnterFullScreen(HMONITOR hMon, HWND hwnd)
{
    HRESULT ddResult = DD_OK;
    // Sleep so that programatically full-screen cannot be entered
    // and left multiple times quickly enough to crash the driver
    static DWORD prevTime = 0;
    DWORD currTime = ::GetTickCount();
    DWORD timeDiff = (currTime - prevTime);
    if (timeDiff < 500) {
        ::Sleep(500 - timeDiff);
    }
    prevTime = currTime;

    DDrawObjectStruct *tmpDdInstance = GetDDInstanceForDevice(hMon);
    tmpDdInstance->ddObject->SetCooperativeLevel(hwnd,
        DDSCL_FULLSCREEN | DDSCL_EXCLUSIVE);
    if (ddResult != DD_OK) {
        DebugPrintDirectDrawError(ddResult, "DDEnterFullScreen");
        return FALSE;
    }
    if (tmpDdInstance->primary) {
	// No clipping necessary in fullscreen mode.  Elsewhere,
	// we avoid setting the clip list for the fullscreen window,
	// so we should also null-out the clipper object for the
	// primary surface in that case.  Bug 4737785.
	tmpDdInstance->primary->SetClipper(NULL);
    }
    tmpDdInstance->hwndFullScreen = hwnd;
    tmpDdInstance->context = CONTEXT_ENTER_FULL_SCREEN;

    return TRUE;
}


/**
 * Exits full-screen exclusive mode
 */
BOOL DDExitFullScreen(HMONITOR hMon, HWND hwnd)
{
    // Sleep so that programatically full-screen cannot be entered
    // and left multiple times quickly enough to crash the driver
    static DWORD prevTime = 0;
    DWORD currTime = ::GetTickCount();
    DWORD timeDiff = (currTime - prevTime);
    if (timeDiff < 500) {
        ::Sleep(500 - timeDiff);
    }
    prevTime = currTime;
    
    DTRACE_PRINTLN("Restoring display mode");
    DDrawObjectStruct *tmpDdInstance = GetDDInstanceForDevice(hMon);
    tmpDdInstance->context = CONTEXT_EXIT_FULL_SCREEN;
    if (!tmpDdInstance || !tmpDdInstance->ddObject ||
        !tmpDdInstance->ddObject->RestoreDDDisplayMode()) {
        return FALSE;
    }
    DTRACE_PRINTLN1("Restoring cooperative level 0x%x", hwnd);
    HRESULT ddResult = 
	tmpDdInstance->ddObject->SetCooperativeLevel(NULL, DDSCL_NORMAL);
    if (ddResult != DD_OK) {
        DebugPrintDirectDrawError(ddResult, "DDExitFullScreen");
        return FALSE;
    }
    if (tmpDdInstance->clipper == NULL) {
	// May not have created clipper if we were in FS mode during 
	// primary creation
	tmpDdInstance->clipper = tmpDdInstance->ddObject->CreateDDClipper();
    }
    if (tmpDdInstance->clipper != NULL) {
	tmpDdInstance->primary->SetClipper(tmpDdInstance->clipper);
    }
    DTRACE_PRINTLN("Restored cooperative level");
    tmpDdInstance->hwndFullScreen = NULL;
    tmpDdInstance->context = CONTEXT_NORMAL;
    return TRUE;
}

/**
 * Gets the current display mode; sets the values in displayMode
 */
BOOL DDGetDisplayMode(HMONITOR hMon, DDrawDisplayMode& displayMode)
{
    DDrawObjectStruct *tmpDdInstance = GetDDInstanceForDevice(hMon);
    if (tmpDdInstance && tmpDdInstance->ddObject) {
        return tmpDdInstance->ddObject->GetDDDisplayMode(displayMode);
    } else {
        return FALSE;
    }
}

/**
 * Sets the display mode to the supplied mode
 */
BOOL DDSetDisplayMode(HMONITOR hMon, DDrawDisplayMode& displayMode)
{
    DDrawObjectStruct *tmpDdInstance = GetDDInstanceForDevice(hMon);
    if (tmpDdInstance) {
        tmpDdInstance->context = CONTEXT_DISPLAY_CHANGE;
    }
    if (tmpDdInstance && tmpDdInstance->ddObject) {
        int attempts = 0;
        while (attempts++ < MAX_BUSY_ATTEMPTS) {
            HRESULT ddResult = tmpDdInstance->ddObject->SetDDDisplayMode(
                displayMode);
            // Spin on the busy-type errors or return having failed or succeeded
            switch (ddResult) {
                case DD_OK:
                    return TRUE;
                case DDERR_SURFACEBUSY:
                    DTRACE_PRINTLN("DDSetDisplayMode: surface busy...");
                    break;
                default:
                    DebugPrintDirectDrawError(ddResult, "DDSetDisplayMode");
                    return FALSE;
            }
        }
        return FALSE;
    } else {
        return FALSE;
    }
}

/**
 * Enumerates all display modes, calling the supplied callback for each
 * display mode returned by the system
 */
BOOL DDEnumDisplayModes(HMONITOR hMon, DDrawDisplayMode* constraint,
    DDrawDisplayMode::Callback callback, void* context)
{
    DDrawObjectStruct *tmpDdInstance = GetDDInstanceForDevice(hMon);
    if (tmpDdInstance && tmpDdInstance->ddObject) {
        return tmpDdInstance->ddObject->EnumDDDisplayModes(
            constraint, callback, context);
    } else {
        return FALSE;
    }
}

/**
 * Attempts to restore surface.  This will only succeed if the system is
 * in a state that allows the surface to be restored.  If a restore
 * results in a DDERR_WRONGMODE, then the surface must be recreated
 * entirely; we do this by invalidating the surfaceData and recreating
 * it from scratch (at the Java level).
 */
BOOL DDRestoreSurface(Win32SDOps *wsdo) 
{
    DTRACE_PRINTLN1("DDRestoreSurface, lpSurface = 0x%x", wsdo->lpSurface);

    DDrawObjectStruct *tmpDdInstance = wsdo->ddInstance;
    if (tmpDdInstance == NULL) {
        return FALSE;
    }
    // Don't try to restore an inactive primary in full-screen mode
    if (!isAppActive && wsdo->window &&
        wsdo->window == tmpDdInstance->hwndFullScreen) {
        return FALSE;
    }
    if (wsdo->lpSurface->IsLost() == DD_OK) {
	DTRACE_PRINTLN("  surface memory ok");
    }
    else {
	DTRACE_PRINTLN("  surface memory lost, trying to restore");
	HRESULT ddResult;
	ddResult = wsdo->lpSurface->Restore();
	if (ddResult == DDERR_WRONGMODE) {
	    // Strange full-screen bug; return false to avoid a hang.
	    // Note that we should never get this error in full-screen mode.
	    if (wsdo->window == tmpDdInstance->hwndFullScreen) {
                return FALSE;
	    }
	    // Wrong mode: display depth has been changed.  Invalidate this
	    // object's ddInstance and return the approriate error.  The
	    // surfaceData will later be invalidated, disposed, and 
	    // re-created with the new and correct depth information
	    DTRACE_PRINTLN("DDRestoreSurface failure: DDERR_WRONGMODE");
	    DDInvalidateDDInstance(wsdo->ddInstance);
	    return FALSE;
	} else if (ddResult != DD_OK) {
	    DebugPrintDirectDrawError(ddResult, "DDRestoreSurface");
	    return FALSE;
	}
    }
    if (!tmpDdInstance->valid) {
        tmpDdInstance->valid = TRUE;
    }
    return TRUE;
}

jint DDGetAvailableMemory(HMONITOR hMon)
{
    DWORD dwFree; 
    DDrawObjectStruct *tmpDdInstance = GetDDInstanceForDevice(hMon);
    if (!useDD || !tmpDdInstance || !tmpDdInstance->valid) {
	return 0;
    }

    HRESULT ddResult = tmpDdInstance->ddObject->GetDDAvailableVidMem(&dwFree); 
    if (ddResult != DD_OK) {
	DebugPrintDirectDrawError(ddResult, "GetAvailableMemory");
    }

    return (jint)dwFree;
}


/**
 * Creates either an offscreen or onscreen ddraw surface, depending
 * on the value of wsdo->window.  Handles the common
 * framework of surface creation, such as ddInstance management,
 * and passes off the functionality of actual surface creation to
 * other functions.  Successful creation results in a return value
 * of TRUE.
 */
BOOL DDCreateSurface(Win32SDOps *wsdo, jboolean isVolatile)
{
    DTRACE_PRINTLN("DDCreateSurface");
    HMONITOR hMon;
    hMon = (HMONITOR)wsdo->device->GetMonitor();
    DDrawObjectStruct *tmpDdInstance = GetDDInstanceForDevice(hMon);

    wsdo->ddInstance = NULL; // default value in case of error
    wsdo->lpSurface = NULL; // default value in case of error

    if (wsdo->window) {
        if (tmpDdInstance && 
            tmpDdInstance->backBufferCount != wsdo->backBufferCount &&
            tmpDdInstance->hwndFullScreen == wsdo->window) {
            tmpDdInstance->context = CONTEXT_CHANGE_BUFFER_COUNT;
            tmpDdInstance->backBufferCount = wsdo->backBufferCount;
        }
	if (!tmpDdInstance || !tmpDdInstance->valid ||
            tmpDdInstance->context != CONTEXT_NORMAL
            ) {
	    // Only recreate dd object on primary create.  Given our current
	    // model of displayChange event propagation, we can only guarantee 
	    // that the system has been properly prepared for a recreate when
	    // we recreate a primary surface.  The offscreen surfaces may 
	    // be recreated at any time.
	    // Recreating ddraw at offscreen surface creation time has caused
	    // rendering artifacts as well as unexplainable hangs in ddraw
	    // calls.
	    ddInstanceLock.Enter();
	    BOOL success = DDCreatePrimary(wsdo);
	    ddInstanceLock.Leave();
	    if (!success) {
		return FALSE;
	    }
	    tmpDdInstance = GetDDInstanceForDevice(hMon);
	}
	// non-null window means onscreen surface.  Primary already 
	// exists, just need to cache a pointer to it in this wsdo
	wsdo->lpSurface = tmpDdInstance->primary;
    } else {
	if (!tmpDdInstance || !tmpDdInstance->valid) {
	    // Don't recreate the ddraw object here (see note above), but
	    // do fail this creation.  We will come back here eventually
	    // after an onscreen surface has been created (and the new 
	    // ddraw object to go along with it).
	    return FALSE;
	}
	if (!DDCreateOffScreenSurface(wsdo, isVolatile, tmpDdInstance)) {
	    DTRACE_PRINTLN("Failure in DDCreateSurface");
	    return FALSE;
	}
    }
    wsdo->ddInstance = tmpDdInstance;
    DTRACE_PRINTLN2("DDCreateSurface succeeded, ddInst, surf = 0x%x, 0x%x", 
		    tmpDdInstance, wsdo->lpSurface);
    return TRUE;
}


/**
 * Creates offscreen surface.  Examines the display mode information
 * for the current ddraw object and uses that to create this new
 * surface.
 */
BOOL DDCreateOffScreenSurface(Win32SDOps *wsdo,
			      jboolean isVolatile,
			      DDrawObjectStruct *ddInst)
{
    DTRACE_PRINTLN("DDCreateOffScreenSurface");

    wsdo->lpSurface = 
	ddInst->ddObject->CreateDDOffScreenSurface(wsdo->w, wsdo->h,
            wsdo->depth, isVolatile, wsdo->transparency, DDSCAPS_VIDEOMEMORY);
    if (!ddInst->primary || (ddInst->primary->IsLost() != DD_OK)) {
	if (wsdo->lpSurface) {
	    delete wsdo->lpSurface;
	    wsdo->lpSurface = NULL;
	}
	if (ddInst->primary) {
	    // attempt to restore primary
	    ddInst->primary->Restore();
	    if (ddInst->primary->IsLost() == DD_OK) {
		// Primary restored: create the offscreen surface again
		wsdo->lpSurface = 
		    ddInst->ddObject->CreateDDOffScreenSurface(wsdo->w, wsdo->h,
			wsdo->depth, isVolatile, wsdo->transparency, 
			DDSCAPS_VIDEOMEMORY);
		if (ddInst->primary->IsLost() != DD_OK) {
		    // doubtful, but possible that it is lost again
		    // If so, delete the surface and get out of here
		    if (wsdo->lpSurface) {
			delete wsdo->lpSurface;
			wsdo->lpSurface = NULL;
		    }
		}
	    }
	}
    }
    return (wsdo->lpSurface != NULL);
}


/**
 * Gets an attached surface, such as a back buffer, from a parent
 * surface.  Sets the lpSurface member of the wsdo supplied to
 * the attached surface.
 */
BOOL DDGetAttachedSurface(JNIEnv *env, Win32SDOps* wsdo_parent,
    Win32SDOps* wsdo)
{
    DTRACE_PRINTLN("DDGetAttachedSurface");
    HMONITOR hMon = (HMONITOR)wsdo_parent->device->GetMonitor();
    DDrawObjectStruct *tmpDdInstance = GetDDInstanceForDevice(hMon);

    wsdo->ddInstance = NULL; // default value in case of error
    wsdo->lpSurface = NULL; // default value in case of error

    if (!tmpDdInstance || !tmpDdInstance->valid) {
	return FALSE;
    }
    DDrawSurface* pNew = wsdo_parent->lpSurface->GetDDAttachedSurface();
    if (pNew == NULL) {
        return FALSE;
    }
    wsdo->lpSurface = pNew;
    wsdo->ddInstance = tmpDdInstance;
    DTRACE_PRINTLN1("DDGetAttachedSurface succeeded, surf = 0x%x",
        wsdo->lpSurface);
    return TRUE;
}


/**
 * Destroys resources associated with a surface
 */
void DDDestroySurface(Win32SDOps *wsdo)
{
    DTRACE_PRINTLN1("DDDestroySurface, wsdo->lpSurface = 0x%x", 
	wsdo->lpSurface);

    if (!wsdo->lpSurface) {
	// null surface means it was never created; simply return
	return;
    }
    if (!wsdo->window) {
	DTRACE_PRINTLN("DDDestroySurface: destroy offscreen surface");
	// offscreen surface
	delete wsdo->lpSurface;
	wsdo->lpSurface = NULL;
    }
    DTRACE_PRINTLN1("DDDestroySurface: ddInstance->refCount = %d",
	wsdo->ddInstance->refCount);
}

/**
 * Releases ddraw resources associated with a surface.  Note that
 * the DDrawSurface object is still valid, but the underlying
 * DirectDraw surface is released.
 */
void DDReleaseSurfaceMemory(DDrawSurface *lpSurface)
{
    DTRACE_PRINTLN1("DDReleaseSurfaceMemory, lpSurface = 0x%x", lpSurface);

    if (!lpSurface) {
	// null surface means it was never created; simply return
	return;
    }
    HRESULT ddResult = lpSurface->ReleaseSurface();
    if (ddResult != DD_OK) {
	DebugPrintDirectDrawError(ddResult, "DDReleaseSurfaceMemory");
    }
}

/*
 * This function returns whether or not surfaces should be replaced
 * in response to a WM_DISPLAYCHANGE message.  If we are a full-screen
 * application that has lost its surfaces, we do not want to replace
 * our surfaces in response to a WM_DISPLAYCHANGE.
 */
BOOL DDCanReplaceSurfaces(HWND hwnd)
{
    DTRACE_PRINTLN1("DDCanReplaceSurfaces hwnd=0x%x", hwnd);
    DDrawObjectStruct *tmpDdInstance = NULL;
    ddInstanceLock.Enter();
    for (int i = 0; i < currNumDevices; i++) {
        tmpDdInstance = ddInstance[i];
        if (tmpDdInstance) {
            DTRACE_PRINTLN2("DDInstance[%d] hwndFullScreen=0x%x",
                i, tmpDdInstance->hwndFullScreen);
            if (tmpDdInstance->hwndFullScreen != NULL &&
                tmpDdInstance->context == CONTEXT_NORMAL &&
                (tmpDdInstance->hwndFullScreen == hwnd || hwnd == NULL)) {
                ddInstanceLock.Leave();
                return FALSE;
            }
        }
    }
    ddInstanceLock.Leave();
    return TRUE;
}

/*
 * This function prints the DirectDraw error associated with
 * the given errNum
 */
void PrintDirectDrawError(DWORD errNum, char *message) 
{
    char buffer[255];

    GetDDErrorString(errNum, buffer);
    printf("%s:: %s\n", message, buffer);
}


/*
 * This function prints the DirectDraw error associated with
 * the given errNum
 */
void DebugPrintDirectDrawError(DWORD errNum, char *message) 
{
    char buffer[255];

    GetDDErrorString(errNum, buffer);
    DTRACE_PRINT2("%s:: %s\n", message, buffer);
}


/*
 * This function prints the error string into the given buffer
 */
void GetDDErrorString(DWORD errNum, char *buffer)
{
    switch (errNum) {
    case DDERR_ALREADYINITIALIZED:
	sprintf(buffer, "DirectDraw Error: DDERR_ALREADYINITIALIZED");
	break;
    case DDERR_CANNOTATTACHSURFACE:
	sprintf(buffer, "DirectDraw Error: DDERR_CANNOTATTACHSURFACE");
	break;
    case DDERR_CANNOTDETACHSURFACE:
	sprintf(buffer, "DirectDraw Error: DDERR_CANNOTDETACHSURFACE");
	break;
    case DDERR_CURRENTLYNOTAVAIL:
	sprintf(buffer, "DirectDraw Error: DDERR_CURRENTLYNOTAVAIL");
	break;
    case DDERR_EXCEPTION:
	sprintf(buffer, "DirectDraw Error: DDERR_EXCEPTION");
	break;
    case DDERR_GENERIC:
	sprintf(buffer, "DirectDraw Error: DDERR_GENERIC");
	break;
    case DDERR_HEIGHTALIGN:
	sprintf(buffer, "DirectDraw Error: DDERR_HEIGHTALIGN");
	break;
    case DDERR_INCOMPATIBLEPRIMARY:
	sprintf(buffer, "DirectDraw Error: DDERR_INCOMPATIBLEPRIMARY");
	break;
    case DDERR_INVALIDCAPS:
	sprintf(buffer, "DirectDraw Error: DDERR_INVALIDCAPS");
	break;
    case DDERR_INVALIDCLIPLIST:
	sprintf(buffer, "DirectDraw Error: DDERR_INVALIDCLIPLIST");
	break;
    case DDERR_INVALIDMODE:
	sprintf(buffer, "DirectDraw Error: DDERR_INVALIDMODE");
	break;
    case DDERR_INVALIDOBJECT:
	sprintf(buffer, "DirectDraw Error: DDERR_INVALIDOBJECT");
	break;
    case DDERR_INVALIDPARAMS:
	sprintf(buffer, "DirectDraw Error: DDERR_INVALIDPARAMS");
	break;
    case DDERR_INVALIDPIXELFORMAT:
	sprintf(buffer, "DirectDraw Error: DDERR_INVALIDPIXELFORMAT");
	break;
    case DDERR_INVALIDRECT:
	sprintf(buffer, "DirectDraw Error: DDERR_INVALIDRECT");
	break;
    case DDERR_LOCKEDSURFACES:
	sprintf(buffer, "DirectDraw Error: DDERR_LOCKEDSURFACES");
	break;
    case DDERR_NO3D:
	sprintf(buffer, "DirectDraw Error: DDERR_NO3D");
	break;
    case DDERR_NOALPHAHW:
	sprintf(buffer, "DirectDraw Error: DDERR_NOALPHAHW");
	break;
    case DDERR_NOCLIPLIST:
	sprintf(buffer, "DirectDraw Error: DDERR_NOCLIPLIST");
	break;
    case DDERR_NOCOLORCONVHW:
	sprintf(buffer, "DirectDraw Error: DDERR_NOCOLORCONVHW");
	break;
    case DDERR_NOCOOPERATIVELEVELSET:
	sprintf(buffer, "DirectDraw Error: DDERR_NOCOOPERATIVELEVELSET");
	break;
    case DDERR_NOCOLORKEY:
	sprintf(buffer, "DirectDraw Error: DDERR_NOCOLORKEY");
	break;
    case DDERR_NOCOLORKEYHW:
	sprintf(buffer, "DirectDraw Error: DDERR_NOCOLORKEYHW");
	break;
    case DDERR_NODIRECTDRAWSUPPORT:
	sprintf(buffer, "DirectDraw Error: DDERR_NODIRECTDRAWSUPPORT");
	break;
    case DDERR_NOEXCLUSIVEMODE:
	sprintf(buffer, "DirectDraw Error: DDERR_NOEXCLUSIVEMODE");
	break;
    case DDERR_NOFLIPHW:
	sprintf(buffer, "DirectDraw Error: DDERR_NOFLIPHW");
	break;
    case DDERR_NOGDI:
	sprintf(buffer, "DirectDraw Error: DDERR_NOGDI");
	break;
    case DDERR_NOMIRRORHW:
	sprintf(buffer, "DirectDraw Error: DDERR_NOMIRRORHW");
	break;
    case DDERR_NOTFOUND:
	sprintf(buffer, "DirectDraw Error: DDERR_NOTFOUND");
	break;
    case DDERR_NOOVERLAYHW:
	sprintf(buffer, "DirectDraw Error: DDERR_NOOVERLAYHW");
	break;
    case DDERR_NORASTEROPHW:
	sprintf(buffer, "DirectDraw Error: DDERR_NORASTEROPHW");
	break;
    case DDERR_NOROTATIONHW:
	sprintf(buffer, "DirectDraw Error: DDERR_NOROTATIONHW");
	break;
    case DDERR_NOSTRETCHHW:
	sprintf(buffer, "DirectDraw Error: DDERR_NOSTRETCHHW");
	break;
    case DDERR_NOT4BITCOLOR:
	sprintf(buffer, "DirectDraw Error: DDERR_NOT4BITCOLOR");
	break;
    case DDERR_NOT4BITCOLORINDEX:
	sprintf(buffer, "DirectDraw Error: DDERR_NOT4BITCOLORINDEX");
	break;
    case DDERR_NOT8BITCOLOR:
	sprintf(buffer, "DirectDraw Error: DDERR_NOT8BITCOLOR");
	break;
    case DDERR_NOTEXTUREHW:
	sprintf(buffer, "DirectDraw Error: DDERR_NOTEXTUREHW");
	break;
    case DDERR_NOVSYNCHW:
	sprintf(buffer, "DirectDraw Error: DDERR_NOVSYNCHW");
	break;
    case DDERR_NOZBUFFERHW:
	sprintf(buffer, "DirectDraw Error: DDERR_NOZBUFFERHW");
	break;
    case DDERR_NOZOVERLAYHW:
	sprintf(buffer, "DirectDraw Error: DDERR_NOZOVERLAYHW");
	break;
    case DDERR_OUTOFCAPS:
	sprintf(buffer, "DirectDraw Error: DDERR_OUTOFCAPS");
	break;
    case DDERR_OUTOFMEMORY:
	sprintf(buffer, "DirectDraw Error: DDERR_OUTOFMEMORY");
	break;
    case DDERR_OUTOFVIDEOMEMORY:
	sprintf(buffer, "DirectDraw Error: DDERR_OUTOFVIDEOMEMORY");
	break;
    case DDERR_OVERLAYCANTCLIP:
	sprintf(buffer, "DirectDraw Error: DDERR_OVERLAYCANTCLIP");
	break;
    case DDERR_OVERLAYCOLORKEYONLYONEACTIVE:
	sprintf(buffer, "DirectDraw Error: DDERR_OVERLAYCOLORKEYONLYONEACTIVE");
	break;
    case DDERR_PALETTEBUSY:
	sprintf(buffer, "DirectDraw Error: DDERR_PALETTEBUSY");
	break;
    case DDERR_COLORKEYNOTSET:
	sprintf(buffer, "DirectDraw Error: DDERR_COLORKEYNOTSET");
	break;
    case DDERR_SURFACEALREADYATTACHED:
	sprintf(buffer, "DirectDraw Error: DDERR_SURFACEALREADYATTACHED");
	break;
    case DDERR_SURFACEALREADYDEPENDENT:
	sprintf(buffer, "DirectDraw Error: DDERR_SURFACEALREADYDEPENDENT");
	break;
    case DDERR_SURFACEBUSY:
	sprintf(buffer, "DirectDraw Error: DDERR_SURFACEBUSY");
	break;
    case DDERR_CANTLOCKSURFACE:
	sprintf(buffer, "DirectDraw Error: DDERR_CANTLOCKSURFACE");
	break;
    case DDERR_SURFACEISOBSCURED:
	sprintf(buffer, "DirectDraw Error: DDERR_SURFACEISOBSCURED");
	break;
    case DDERR_SURFACELOST:
	sprintf(buffer, "DirectDraw Error: DDERR_SURFACELOST");
	break;
    case DDERR_SURFACENOTATTACHED:
	sprintf(buffer, "DirectDraw Error: DDERR_SURFACENOTATTACHED");
	break;
    case DDERR_TOOBIGHEIGHT:
	sprintf(buffer, "DirectDraw Error: DDERR_TOOBIGHEIGHT");
	break;
    case DDERR_TOOBIGSIZE:
	sprintf(buffer, "DirectDraw Error: DDERR_TOOBIGSIZE");
	break;
    case DDERR_TOOBIGWIDTH:
	sprintf(buffer, "DirectDraw Error: DDERR_TOOBIGWIDTH");
	break;
    case DDERR_UNSUPPORTED:
	sprintf(buffer, "DirectDraw Error: DDERR_UNSUPPORTED");
	break;
    case DDERR_UNSUPPORTEDFORMAT:
	sprintf(buffer, "DirectDraw Error: DDERR_UNSUPPORTEDFORMAT");
	break;
    case DDERR_UNSUPPORTEDMASK:
	sprintf(buffer, "DirectDraw Error: DDERR_UNSUPPORTEDMASK");
	break;
    case DDERR_VERTICALBLANKINPROGRESS:
	sprintf(buffer, "DirectDraw Error: DDERR_VERTICALBLANKINPROGRESS");
	break;
    case DDERR_WASSTILLDRAWING:
	sprintf(buffer, "DirectDraw Error: DDERR_WASSTILLDRAWING");
	break;
    case DDERR_XALIGN:
	sprintf(buffer, "DirectDraw Error: DDERR_XALIGN");
	break;
    case DDERR_INVALIDDIRECTDRAWGUID:
	sprintf(buffer, "DirectDraw Error: DDERR_INVALIDDIRECTDRAWGUID");
	break;
    case DDERR_DIRECTDRAWALREADYCREATED:
	sprintf(buffer, "DirectDraw Error: DDERR_DIRECTDRAWALREADYCREATED");
	break;
    case DDERR_NODIRECTDRAWHW:
	sprintf(buffer, "DirectDraw Error: DDERR_NODIRECTDRAWHW");
	break;
    case DDERR_PRIMARYSURFACEALREADYEXISTS:
	sprintf(buffer, "DirectDraw Error: DDERR_PRIMARYSURFACEALREADYEXISTS");
	break;
    case DDERR_NOEMULATION:
	sprintf(buffer, "DirectDraw Error: DDERR_NOEMULATION");
	break;
    case DDERR_REGIONTOOSMALL:
	sprintf(buffer, "DirectDraw Error: DDERR_REGIONTOOSMALL");
	break;
    case DDERR_CLIPPERISUSINGHWND:
	sprintf(buffer, "DirectDraw Error: DDERR_CLIPPERISUSINGHWND");
	break;
    case DDERR_NOCLIPPERATTACHED:
	sprintf(buffer, "DirectDraw Error: DDERR_NOCLIPPERATTACHED");
	break;
    case DDERR_NOHWND:
	sprintf(buffer, "DirectDraw Error: DDERR_NOHWND");
	break;
    case DDERR_HWNDSUBCLASSED:
	sprintf(buffer, "DirectDraw Error: DDERR_HWNDSUBCLASSED");
	break;
    case DDERR_HWNDALREADYSET:
	sprintf(buffer, "DirectDraw Error: DDERR_HWNDALREADYSET");
	break;
    case DDERR_NOPALETTEATTACHED:
	sprintf(buffer, "DirectDraw Error: DDERR_NOPALETTEATTACHED");
	break;
    case DDERR_NOPALETTEHW:
	sprintf(buffer, "DirectDraw Error: DDERR_NOPALETTEHW");
	break;
    case DDERR_BLTFASTCANTCLIP:
	sprintf(buffer, "DirectDraw Error: DDERR_BLTFASTCANTCLIP");
	break;
    case DDERR_NOBLTHW:
	sprintf(buffer, "DirectDraw Error: DDERR_NOBLTHW");
	break;
    case DDERR_NODDROPSHW:
	sprintf(buffer, "DirectDraw Error: DDERR_NODDROPSHW");
	break;
    case DDERR_OVERLAYNOTVISIBLE:
	sprintf(buffer, "DirectDraw Error: DDERR_OVERLAYNOTVISIBLE");
	break;
    case DDERR_NOOVERLAYDEST:
	sprintf(buffer, "DirectDraw Error: DDERR_NOOVERLAYDEST");
	break;
    case DDERR_INVALIDPOSITION:
	sprintf(buffer, "DirectDraw Error: DDERR_INVALIDPOSITION");
	break;
    case DDERR_NOTAOVERLAYSURFACE:
	sprintf(buffer, "DirectDraw Error: DDERR_NOTAOVERLAYSURFACE");
	break;
    case DDERR_EXCLUSIVEMODEALREADYSET:
	sprintf(buffer, "DirectDraw Error: DDERR_EXCLUSIVEMODEALREADYSET");
	break;
    case DDERR_NOTFLIPPABLE:
	sprintf(buffer, "DirectDraw Error: DDERR_NOTFLIPPABLE");
	break;
    case DDERR_CANTDUPLICATE:
	sprintf(buffer, "DirectDraw Error: DDERR_CANTDUPLICATE");
	break;
    case DDERR_NOTLOCKED:
	sprintf(buffer, "DirectDraw Error: DDERR_NOTLOCKED");
	break;
    case DDERR_CANTCREATEDC:
	sprintf(buffer, "DirectDraw Error: DDERR_CANTCREATEDC");
	break;
    case DDERR_NODC:
	sprintf(buffer, "DirectDraw Error: DDERR_NODC");
	break;
    case DDERR_WRONGMODE:
	sprintf(buffer, "DirectDraw Error: DDERR_WRONGMODE");
	break;
    case DDERR_IMPLICITLYCREATED:
	sprintf(buffer, "DirectDraw Error: DDERR_IMPLICITLYCREATED");
	break;
    case DDERR_NOTPALETTIZED:
	sprintf(buffer, "DirectDraw Error: DDERR_NOTPALETTIZED");
	break;
    case DDERR_UNSUPPORTEDMODE:
	sprintf(buffer, "DirectDraw Error: DDERR_UNSUPPORTEDMODE");
	break;
    case DDERR_NOMIPMAPHW:
	sprintf(buffer, "DirectDraw Error: DDERR_NOMIPMAPHW");
	break;
    case DDERR_INVALIDSURFACETYPE:
	sprintf(buffer, "DirectDraw Error: DDERR_INVALIDSURFACETYPE");
	break;
    case DDERR_DCALREADYCREATED:
	sprintf(buffer, "DirectDraw Error: DDERR_DCALREADYCREATED");
	break;
    case DDERR_CANTPAGELOCK:
	sprintf(buffer, "DirectDraw Error: DDERR_CANTPAGELOCK");
	break;
    case DDERR_CANTPAGEUNLOCK:
	sprintf(buffer, "DirectDraw Error: DDERR_CANTPAGEUNLOCK");
	break;
    case DDERR_NOTPAGELOCKED:
	sprintf(buffer, "DirectDraw Error: DDERR_NOTPAGELOCKED");
	break;
    case D3DERR_INVALID_DEVICE:
	sprintf(buffer, "Direct3D Error: D3DERR_INVALID_DEVICE");
	break;
    case D3DERR_INITFAILED:
	sprintf(buffer, "Direct3D Error: D3DERR_INITFAILED");
	break;
    case D3DERR_DEVICEAGGREGATED:
	sprintf(buffer, "Direct3D Error: D3DERR_DEVICEAGGREGATED");
	break;
    case D3DERR_EXECUTE_CREATE_FAILED:
	sprintf(buffer, "Direct3D Error: D3DERR_EXECUTE_CREATE_FAILED");
	break;
    case D3DERR_EXECUTE_DESTROY_FAILED:
	sprintf(buffer, "Direct3D Error: D3DERR_EXECUTE_DESTROY_FAILED");
	break;
    case D3DERR_EXECUTE_LOCK_FAILED:
	sprintf(buffer, "Direct3D Error: D3DERR_EXECUTE_LOCK_FAILED");
	break;
    case D3DERR_EXECUTE_UNLOCK_FAILED:
	sprintf(buffer, "Direct3D Error: D3DERR_EXECUTE_UNLOCK_FAILED");
	break;
    case D3DERR_EXECUTE_LOCKED:	
	sprintf(buffer, "Direct3D Error: D3DERR_EXECUTE_LOCKED");
	break;
    case D3DERR_EXECUTE_NOT_LOCKED:
	sprintf(buffer, "Direct3D Error: D3DERR_EXECUTE_NOT_LOCKED");
	break;
    case D3DERR_EXECUTE_FAILED:
	sprintf(buffer, "Direct3D Error: D3DERR_EXECUTE_FAILED");
	break;
    case D3DERR_EXECUTE_CLIPPED_FAILED:
	sprintf(buffer, "Direct3D Error: D3DERR_EXECUTE_CLIPPED_FAILED");
	break;
    case D3DERR_TEXTURE_NO_SUPPORT:
	sprintf(buffer, "Direct3D Error: D3DERR_TEXTURE_NO_SUPPORT");
	break;
    case D3DERR_TEXTURE_CREATE_FAILED:
	sprintf(buffer, "Direct3D Error: D3DERR_TEXTURE_CREATE_FAILED");
	break;
    case D3DERR_TEXTURE_DESTROY_FAILED:
	sprintf(buffer, "Direct3D Error: D3DERR_TEXTURE_DESTROY_FAILED");
	break;
    case D3DERR_TEXTURE_LOCK_FAILED:	
	sprintf(buffer, "Direct3D Error: D3DERR_TEXTURE_LOCK_FAILED");
	break;
    case D3DERR_TEXTURE_UNLOCK_FAILED:	
	sprintf(buffer, "Direct3D Error: D3DERR_TEXTURE_UNLOCK_FAILED");
	break;
    case D3DERR_TEXTURE_LOAD_FAILED:	
	sprintf(buffer, "Direct3D Error: D3DERR_TEXTURE_LOAD_FAILED");
	break;
    case D3DERR_TEXTURE_SWAP_FAILED:	
	sprintf(buffer, "Direct3D Error: D3DERR_TEXTURE_SWAP_FAILED");
	break;
    case D3DERR_TEXTURE_LOCKED:
	sprintf(buffer, "Direct3D Error: D3DERR_TEXTURE_LOCKED");
	break;
    case D3DERR_TEXTURE_NOT_LOCKED:
	sprintf(buffer, "Direct3D Error: D3DERR_TEXTURE_NOT_LOCKED");
	break;
    case D3DERR_TEXTURE_GETSURF_FAILED:
	sprintf(buffer, "Direct3D Error: D3DERR_TEXTURE_GETSURF_FAILED");
	break;
    case D3DERR_MATRIX_CREATE_FAILED:
	sprintf(buffer, "Direct3D Error: D3DERR_MATRIX_CREATE_FAILED");
	break;
    case D3DERR_MATRIX_DESTROY_FAILED:
	sprintf(buffer, "Direct3D Error: D3DERR_MATRIX_DESTROY_FAILED");
	break;
    case D3DERR_MATRIX_SETDATA_FAILED:
	sprintf(buffer, "Direct3D Error: D3DERR_MATRIX_SETDATA_FAILED");
	break;
    case D3DERR_MATRIX_GETDATA_FAILED:
	sprintf(buffer, "Direct3D Error: D3DERR_MATRIX_GETDATA_FAILED");
	break;
    case D3DERR_SETVIEWPORTDATA_FAILED:
	sprintf(buffer, "Direct3D Error: D3DERR_SETVIEWPORTDATA_FAILED");
	break;
    case D3DERR_INVALIDCURRENTVIEWPORT:
	sprintf(buffer, "Direct3D Error: D3DERR_INVALIDCURRENTVIEWPORT");
	break;
    case D3DERR_INVALIDPRIMITIVETYPE:
	sprintf(buffer, "Direct3D Error: D3DERR_INVALIDPRIMITIVETYPE");
	break;
    case D3DERR_INVALIDVERTEXTYPE:
	sprintf(buffer, "Direct3D Error: D3DERR_INVALIDVERTEXTYPE");
	break;
    case D3DERR_TEXTURE_BADSIZE:
	sprintf(buffer, "Direct3D Error: D3DERR_TEXTURE_BADSIZE");
	break;
    case D3DERR_INVALIDRAMPTEXTURE:	
	sprintf(buffer, "Direct3D Error: D3DERR_INVALIDRAMPTEXTURE");
	break;
    case D3DERR_MATERIAL_CREATE_FAILED:
	sprintf(buffer, "Direct3D Error: D3DERR_MATERIAL_CREATE_FAILED");
	break;
    case D3DERR_MATERIAL_DESTROY_FAILED:
	sprintf(buffer, "Direct3D Error: D3DERR_MATERIAL_DESTROY_FAILED");
	break;
    case D3DERR_MATERIAL_SETDATA_FAILED:
	sprintf(buffer, "Direct3D Error: D3DERR_MATERIAL_SETDATA_FAILED");
	break;
    case D3DERR_MATERIAL_GETDATA_FAILED:
	sprintf(buffer, "Direct3D Error: D3DERR_MATERIAL_GETDATA_FAILED");
	break;
    case D3DERR_INVALIDPALETTE:
	sprintf(buffer, "Direct3D Error: D3DERR_INVALIDPALETTE");
	break;
    case D3DERR_ZBUFF_NEEDS_SYSTEMMEMORY:
	sprintf(buffer, "Direct3D Error: D3DERR_ZBUFF_NEEDS_SYSTEMMEMORY");
	break;
    case D3DERR_ZBUFF_NEEDS_VIDEOMEMORY:
	sprintf(buffer, "Direct3D Error: D3DERR_ZBUFF_NEEDS_VIDEOMEMORY");
	break;
    case D3DERR_SURFACENOTINVIDMEM:
	sprintf(buffer, "Direct3D Error: D3DERR_SURFACENOTINVIDMEM");
	break;
    case D3DERR_LIGHT_SET_FAILED:
	sprintf(buffer, "Direct3D Error: D3DERR_LIGHT_SET_FAILED");
	break;
    case D3DERR_LIGHTHASVIEWPORT:
	sprintf(buffer, "Direct3D Error: D3DERR_LIGHTHASVIEWPORT");
	break;
    case D3DERR_LIGHTNOTINTHISVIEWPORT:
	sprintf(buffer, "Direct3D Error: D3DERR_LIGHTNOTINTHISVIEWPORT");
	break;
    case D3DERR_SCENE_IN_SCENE:
	sprintf(buffer, "Direct3D Error: D3DERR_SCENE_IN_SCENE");
	break;
    case D3DERR_SCENE_NOT_IN_SCENE:
	sprintf(buffer, "Direct3D Error: D3DERR_SCENE_NOT_IN_SCENE");
	break;
    case D3DERR_SCENE_BEGIN_FAILED:
	sprintf(buffer, "Direct3D Error: D3DERR_SCENE_BEGIN_FAILED");
	break;
    case D3DERR_SCENE_END_FAILED:	
	sprintf(buffer, "Direct3D Error: D3DERR_SCENE_END_FAILED");
	break;
    case D3DERR_INBEGIN:
	sprintf(buffer, "Direct3D Error: D3DERR_INBEGIN");
	break;
    case D3DERR_NOTINBEGIN:
	sprintf(buffer, "Direct3D Error: D3DERR_NOTINBEGIN");
	break;
    case D3DERR_NOVIEWPORTS:
	sprintf(buffer, "Direct3D Error: D3DERR_NOVIEWPORTS");
	break;
    case D3DERR_VIEWPORTDATANOTSET:
	sprintf(buffer, "Direct3D Error: D3DERR_VIEWPORTDATANOTSET");
	break;
    case D3DERR_VIEWPORTHASNODEVICE:
	sprintf(buffer, "Direct3D Error: D3DERR_VIEWPORTHASNODEVICE");
	break;
    case D3DERR_NOCURRENTVIEWPORT:	
	sprintf(buffer, "Direct3D Error: D3DERR_NOCURRENTVIEWPORT");
	break;
    default:
	sprintf(buffer, "DirectX Error Unknown 0x%x", errNum);
	break;
    }

}
