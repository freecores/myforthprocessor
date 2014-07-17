/*
 * @(#)Win32OffScreenSurfaceData.cpp	1.41 03/04/28
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "sun_awt_windows_Win32OffScreenSurfaceData.h"

#include "Win32SurfaceData.h"

#include "awt_Component.h"
#include "debug_trace.h"
#include "ddrawUtils.h"
#include "awt_Win32GraphicsDevice.h"

#include "jni_util.h"

/**
 * This source file contains support code for loops using the
 * SurfaceData interface to talk to a Win32 drawable from native
 * code.
 */

static LockFunc Win32OSSD_Lock;
static GetRasInfoFunc Win32OSSD_GetRasInfo;
static UnlockFunc Win32OSSD_Unlock;
static DisposeFunc Win32OSSD_Dispose;
static GetDCFunc Win32OSSD_GetDC;
static ReleaseDCFunc Win32OSSD_ReleaseDC;
static InvalidateSDFunc Win32OSSD_InvalidateSD;

jfieldID localD3dEnabledID; // non-static becaused shared by Win32BBSD
jfieldID d3dClippingEnabledID; // non-static becaused shared by Win32BBSD
jfieldID ddSurfacePuntedID;
static HBRUSH	nullbrush;
static HPEN	nullpen;

extern MTSafeArray *devices;
extern BOOL useD3D;
extern BOOL forceD3DUsage;
jboolean g_offscreenSharing = FALSE;

extern "C" {


/*
 * Class:     sun_awt_windows_Win32OffScreenSurfaceData
 * Method:    initIDs
 * Signature: ()V
 */
JNIEXPORT void JNICALL
Java_sun_awt_windows_Win32OffScreenSurfaceData_initIDs(JNIEnv *env, jclass wsd)
{
    DTRACE_PRINTLN("Java_sun_awt_windows_Win32OffScreenSurfaceData_initIDs");
    localD3dEnabledID = env->GetFieldID(wsd, "localD3dEnabled", "Z");
    d3dClippingEnabledID = env->GetFieldID(wsd, "d3dClippingEnabled", "Z");
    ddSurfacePuntedID = env->GetFieldID(wsd, "ddSurfacePunted", "Z");
    nullbrush = (HBRUSH) ::GetStockObject(NULL_BRUSH);
    nullpen = (HPEN) ::GetStockObject(NULL_PEN);
}

void Win32OSSD_RestoreSurface(JNIEnv *env, Win32SDOps *wsdo) 
{
    DTRACE_PRINTLN("Win32OSSD_RestoreSurface: throwing exception");
    wsdo->surfaceLost = TRUE;
    SurfaceData_ThrowInvalidPipeException(env, "Lost DDraw Surface");
}

/*
 * Class:     sun_awt_windows_Win32OffScreenSurfaceData
 * Method:    initDDraw
 * Signature: (Z)V
 */
JNIEXPORT void JNICALL
Java_sun_awt_windows_Win32OffScreenSurfaceData_initDDraw(JNIEnv *env, 
							 jclass wsd,
							 jboolean sharing)
{
    g_offscreenSharing = sharing;	// external sharing of
    					// native offscreen surfaces
    DDInit();
}


BOOL VideoCardD3dFriendly()
{
    typedef BOOL (WINAPI *FnEnumDisplayDevicesFunc) (VOID*, DWORD,
						     DISPLAY_DEVICE*,
						     DWORD);
    HINSTANCE hLibUser32 = LoadLibrary("user32.dll");
    char *badDeviceStrings[] = {
	"Trident Video Accelerator",
    };
    int numBadDevices = 1;
    
    //char *deviceString;
    if (!hLibUser32) {
	// failure situation; shouldn't get here, but just
	// NOOP if we do
	return TRUE;
    }
    DWORD deviceNum = 0;
    DISPLAY_DEVICE displayDevice;
    displayDevice.cb = sizeof(displayDevice);
    FnEnumDisplayDevicesFunc enumFunc = 
	(FnEnumDisplayDevicesFunc)::GetProcAddress(hLibUser32, 
						   "EnumDisplayDevicesA");
    if (enumFunc) {
	// This function only exists on win98, win2000, and later
	// There is apparently a bug in NT4 sp6 where it actually does
	// have some implementation of this function, but with a different
	// calling convention.  But our calling function avoids calling
	// us if running on NT4 (see the IS_NT check at the start of
	// enableD3D), so we do not have to handle the buggy NT4 case.
	while (enumFunc(NULL, deviceNum, &displayDevice, 0) && 
	       deviceNum < 20) // avoid infinite loop with buggy drivers
	{
	    if (displayDevice.StateFlags & 
		DISPLAY_DEVICE_ATTACHED_TO_DESKTOP) 
	    {
		// We only care about actual display devices.  Devices without
		// this flag could be virtual devices such as NetMeeting.
		for (int i = 0; i < numBadDevices; ++i) {
		    if (strncmp(badDeviceStrings[i], 
			       (char*)displayDevice.DeviceString,
			       strlen(badDeviceStrings[i])) == 0)
		    {
			J2dTraceLn1(J2D_TRACE_INFO, 
				    "Suspect video card (%s): disable d3d",
				    badDeviceStrings[i]);
			return FALSE;
		    }
		}
	    }
	    deviceNum++;
	}
    }
    // No match against problem devices; return TRUE to allow
    // d3d to function.
    return TRUE;
}

/*
 * Class:     sun_awt_windows_Win32OffScreenSurfaceData
 * Method:    enableD3D
 * Signature: (Z)Z
 */
JNIEXPORT jboolean JNICALL
Java_sun_awt_windows_Win32OffScreenSurfaceData_enableD3D(JNIEnv *env, 
							 jclass wsd, 
							 jboolean forceD3DOn) 
{
    DTRACE_PRINTLN("enableD3D");

    if (IS_NT && !(IS_WIN2000)) {
	// Do not enable d3d on NT4; d3d is only supported through
	// software on that platform
	return FALSE;
    }
    if (IS_WIN64 && !forceD3DOn) {
	// Only enable d3d on Itanium if user forces it on.
	// D3d was not functioning on initial XP Itanium releases
	// so we do not want it suddenly enabled in the field without
	// having tested that codepath first.
	return FALSE;
    }
    forceD3DUsage = forceD3DOn;
    if (!forceD3DUsage) {
	// Only check environment variable if user did not set Java
	// command-line parameter; values of sun.java2d.d3d override
	// any setting of J2D_D3D environment variable.
	char *d3dEnv = getenv("J2D_D3D");
	if (d3dEnv) {
	    if (strcmp(d3dEnv, "false") == 0) {
		printf("Java2D Direct3D usage disabled by J2D_D3D env\n");
		return FALSE;
	    } else if (strcmp(d3dEnv, "true") == 0) {
		printf("Java2D Direct3D usage forced on by J2D_D3D env\n");
		forceD3DUsage = TRUE;
	    }
	}
    }
    if (!forceD3DUsage && !VideoCardD3dFriendly()) {
	// If any video card in this system has major problems with d3d, then
	// disable d3d unless the user tells us not to.  Note that this 
	// global disabling of d3d is overkill; we should disable acceleration
	// just on the problem device(s).
	return FALSE;
    }
    useD3D = TRUE;
    return TRUE;
}


void disposeOSSD_WSDO(JNIEnv* env, Win32SDOps* wsdo)
{
    wsdo->device->Release();
    delete wsdo->surfaceLock;
}

void initOSSD_WSDO(JNIEnv* env, Win32SDOps* wsdo, jint width, jint height,
    jint screen, jint transparency)
{
    wsdo->device = 
	(AwtWin32GraphicsDevice*)devices->GetElementReference(screen);

    wsdo->transparency = transparency;
    // fix the size to be the next power of two as required by d3d
    // REMIND: we should query if the device driver supports arbitrary
    // texture sizes prior to changing.
    if (transparency == TR_TRANSLUCENT) {
	for (wsdo->w = 1; width  > wsdo->w; wsdo->w <<=1);
	for (wsdo->h = 1; height > wsdo->h; wsdo->h <<=1);
    } else {
	wsdo->w = width;
	wsdo->h = height;
    }
    wsdo->surfacePuntData.disablePunts = TRUE;
}

JNIEXPORT void JNICALL 
Java_sun_awt_windows_Win32OffScreenSurfaceData_initSurface(JNIEnv *env, 
							   jobject sData, 
							   jint depth, 
							   jint width,
							   jint height,
							   jint screen,
							   jboolean isVolatile,
                                                           jint transparency,
							   jboolean disablePunts)
{
    Win32SDOps *wsdo = (Win32SDOps *)SurfaceData_GetOps(env, sData);

    DTRACE_PRINTLN("Win32OSSD_initSurface");
    initOSSD_WSDO(env, wsdo, width, height, screen, transparency);
    if (!DDCreateSurface(wsdo, isVolatile)) {
	DTRACE_PRINTLN1("Win32OSD.initSurface: Can't create offsc surf tr=%d\n",
			transparency);
	SurfaceData_ThrowInvalidPipeException(env, 
					      "Can't create offscreen surf");
    } else {
	wsdo->surfacePuntData.lpSurfaceVram = wsdo->lpSurface;
	if (!D3DEnabled(wsdo)) {
	    // d3d enabled by default for each surface - disable if necessary
	    env->SetBooleanField(sData, localD3dEnabledID, JNI_FALSE);
	} else {
	    env->SetBooleanField(sData, d3dClippingEnabledID, 
	    			 (wsdo->ddInstance && 
	    			  wsdo->ddInstance->canClipD3dLines));
	}
    }

    // disable punts for accelerated surfaces
    disablePunts = isVolatile ? disablePunts : TRUE;

    // 8 is somewhat arbitrary; we want the threshhold to represent a
    // significant portion of the surface area in order to avoid
    // punting for occasional, small reads
    wsdo->surfacePuntData.pixelsReadThreshold = width * height / 8;
    wsdo->surfacePuntData.disablePunts = disablePunts;
}

/*
 * Class:     sun_awt_windows_Win32OffScreenSurfaceData
 * Method:    restoreSurface
 * Signature: ()V
 */
JNIEXPORT void JNICALL
Java_sun_awt_windows_Win32OffScreenSurfaceData_restoreSurface(JNIEnv *env, 
							      jobject sData)
{
    DTRACE_PRINTLN("native method Win32OSSD_RestoreSurface: restoring offscreen");
    Win32SDOps *wsdo = (Win32SDOps *)SurfaceData_GetOps(env, sData);

    // Might have gotten here by some default action.  Make sure that the
    // surface is marked as lost before bothering to try to restore it.
    if (!wsdo->surfaceLost) {
	return;
    }

    // Attempt to restore and lock the surface (to make sure the restore worked)
    if (DDRestoreSurface(wsdo) && DDLock(env, wsdo, NULL, NULL)) {
	DDUnlock(env, wsdo);
	wsdo->surfaceLost = FALSE;
    } else {
	// Failure - throw exception
	DTRACE_PRINTLN("Win32OSSD_restoreSurface: problems restoring");
	SurfaceData_ThrowInvalidPipeException(env, "RestoreSurface failure");
    }
}

    
/*
 * Class:     sun_awt_windows_Win32OffScreenSurfaceData
 * Method:    initOps
 * Signature: (Ljava/lang/Object;)V
 */
JNIEXPORT void JNICALL
Java_sun_awt_windows_Win32OffScreenSurfaceData_initOps(JNIEnv *env, jobject wsd,
						       jint depth,
						       jint transparency)
{
    Win32SDOps *wsdo = (Win32SDOps *)SurfaceData_InitOps(env, wsd, sizeof(Win32SDOps));
    wsdo->sdOps.Lock = Win32OSSD_Lock;
    wsdo->sdOps.GetRasInfo = Win32OSSD_GetRasInfo;
    wsdo->sdOps.Unlock = Win32OSSD_Unlock;
    wsdo->sdOps.Dispose = Win32OSSD_Dispose;
    wsdo->RestoreSurface = Win32OSSD_RestoreSurface;
    wsdo->GetDC = Win32OSSD_GetDC;
    wsdo->ReleaseDC = Win32OSSD_ReleaseDC;
    wsdo->InvalidateSD = Win32OSSD_InvalidateSD;
    wsdo->invalid = JNI_FALSE;
    wsdo->lockType = WIN32SD_LOCK_UNLOCKED;
    wsdo->window = NULL;
    wsdo->backBufferCount = 0;
    wsdo->depth = depth;
    switch (depth) {
	case 8:
	    wsdo->pixelStride = 1;
	    break;
	case 15: //555
	    wsdo->pixelStride = 2;
	    wsdo->pixelMasks[0] = 0x1f << 10;
	    wsdo->pixelMasks[1] = 0x1f << 5;
	    wsdo->pixelMasks[2] = 0x1f;
	    break;
	case 16: //565
	    wsdo->pixelStride = 2;
	    wsdo->pixelMasks[0] = 0x1f << 11;
	    wsdo->pixelMasks[1] = 0x3f << 5;
	    wsdo->pixelMasks[2] = 0x1f;
	    break;
	case 24:
	    wsdo->pixelStride = 3;
	    break;
	case 32: //888
	    wsdo->pixelStride = 4;
 	    wsdo->pixelMasks[0] = 0xff0000;
	    wsdo->pixelMasks[1] = 0x00ff00;
	    wsdo->pixelMasks[2] = 0x0000ff;
	    break;
    }
    wsdo->surfaceLock = new CriticalSection();
    wsdo->surfaceLost = FALSE;
    wsdo->transparency = transparency;
    wsdo->surfacePuntData.usingDDSystem = FALSE;
    wsdo->surfacePuntData.lpSurfaceSystem = NULL;
    wsdo->surfacePuntData.lpSurfaceVram = NULL;
    wsdo->surfacePuntData.numBltsSinceRead = 0;
    wsdo->surfacePuntData.pixelsReadSinceBlt = 0;
    wsdo->surfacePuntData.numBltsThreshold = 2;
}

JNIEXPORT Win32SDOps * JNICALL
Win32OffScreenSurfaceData_GetOps(JNIEnv *env, jobject sData)
{
    SurfaceDataOps *ops = SurfaceData_GetOps(env, sData);
    if (ops == NULL) {
	JNU_ThrowNullPointerException(env, "SurfaceData native ops");
    } else if (ops->Lock != Win32OSSD_Lock) {
	SurfaceData_ThrowInvalidPipeException(env, "not a Win32 SurfaceData");
	ops = NULL;
    }
    return (Win32SDOps *) ops;
}

} /* extern "C" */


static void Win32OSSD_LockByDD(JNIEnv *env, Win32SDOps *wsdo, jint lockflags,
			       SurfaceDataRasInfo *pRasInfo)
{
    DTRACE_PRINTLN("Win32OSSD_LockByDD");
    
    if ((lockflags & SD_LOCK_READ) &&
	!wsdo->surfacePuntData.disablePunts) 
    {
	wsdo->surfacePuntData.numBltsSinceRead = 0;
	if (!wsdo->surfacePuntData.usingDDSystem) {
	    int w = pRasInfo->bounds.x2 - pRasInfo->bounds.x1;
	    int h = pRasInfo->bounds.y2 - pRasInfo->bounds.y1;
	    wsdo->surfacePuntData.pixelsReadSinceBlt += w * h;
	    // Note that basing this decision on the bounds is somewhat
	    // incorrect because locks of type FASTEST will simply send
	    // in bounds that equal the area of the entire surface.
	    // To do this correctly, we would need to return 
	    // SLOWLOCK and recalculate the punt data in GetRasInfo()
	    if (wsdo->surfacePuntData.pixelsReadSinceBlt >
		wsdo->surfacePuntData.pixelsReadThreshold) 
	    {
		// Create the system surface if it doesn't exist
		if (!wsdo->surfacePuntData.lpSurfaceSystem) {
		    wsdo->surfacePuntData.lpSurfaceSystem = 
			wsdo->ddInstance->ddObject->CreateDDOffScreenSurface(
			wsdo->w, wsdo->h, wsdo->depth, TRUE/*isVolatile*/, 
			wsdo->transparency, DDSCAPS_SYSTEMMEMORY);
		    if (wsdo->surfacePuntData.lpSurfaceSystem) {
			DDCOLORKEY ddck;
			HRESULT ddResult = 
			    wsdo->surfacePuntData.lpSurfaceVram->GetColorKey(
			    DDCKEY_SRCBLT, &ddck);
			if (ddResult == DD_OK) {
			    // Vram surface has colorkey - use same colorkey on sys
			    ddResult = 
				wsdo->surfacePuntData.lpSurfaceSystem->SetColorKey(
				DDCKEY_SRCBLT, &ddck);
			}
		    }
		}
		// Assuming no errors in system creation, copy contents
		if (wsdo->surfacePuntData.lpSurfaceSystem) {
		    if (wsdo->surfacePuntData.lpSurfaceSystem->Blt(NULL, 
			    wsdo->surfacePuntData.lpSurfaceVram, NULL, 
			    DDBLT_WAIT, NULL) == DD_OK) 
		    {
			J2dTraceLn2(J2D_TRACE_INFO, 
				    "Win32OSSD: punting VRAM to sys: 0x%x -> 0x%x\n",
				    wsdo->surfacePuntData.lpSurfaceSystem,
				    wsdo->surfacePuntData.lpSurfaceVram);
			wsdo->lpSurface = wsdo->surfacePuntData.lpSurfaceSystem;
			wsdo->surfacePuntData.usingDDSystem = TRUE;
			// Notify the Java level that this surface has
			// been punted to avoid performance penalties from
			// copying from VRAM cached versions of other images
			// when we should use system memory versions instead.
			jobject sdObject =
			    env->NewLocalRef(wsdo->sdOps.sdObject);
			if (sdObject) {
			    // Only bother with this optimization if the
			    // reference is still valid
			    env->SetBooleanField(sdObject, ddSurfacePuntedID,
						 JNI_TRUE);
			    env->DeleteLocalRef(sdObject);
			}
		    }
		}
	    }
	}
    }

    if (!DDLock(env, wsdo, NULL, pRasInfo))
	return;

    wsdo->lockType = WIN32SD_LOCK_BY_DDRAW;
}


static jint Win32OSSD_Lock(JNIEnv *env,
			   SurfaceDataOps *ops,
			   SurfaceDataRasInfo *pRasInfo,
			   jint lockflags)
{
    Win32SDOps *wsdo = (Win32SDOps *) ops;
    DTRACE_PRINTLN1("W32OSSD::Win32OSSD_Lock, lockflags = 0x%x", lockflags);
    wsdo->surfaceLock->Enter();
    if (wsdo->invalid) {
	wsdo->surfaceLock->Leave();
	SurfaceData_ThrowInvalidPipeException(env, "invalid sd");
	return SD_FAILURE;
    }

    if (wsdo->lockType != WIN32SD_LOCK_UNLOCKED) {
	wsdo->surfaceLock->Leave();
	JNU_ThrowInternalError(env, "Win32OSSD_Lock cannot nest locks");
	return SD_FAILURE;
    }

    if (lockflags & SD_LOCK_RD_WR) {
	if (pRasInfo->bounds.x1 < 0) pRasInfo->bounds.x1 = 0;
	if (pRasInfo->bounds.y1 < 0) pRasInfo->bounds.y1 = 0;
	if (pRasInfo->bounds.x2 > wsdo->w) pRasInfo->bounds.x2 = wsdo->w;
	if (pRasInfo->bounds.y2 > wsdo->h) pRasInfo->bounds.y2 = wsdo->h;
	if (DDUseDDraw(wsdo)) {
	    Win32OSSD_LockByDD(env, wsdo, lockflags, pRasInfo);
	}
	if (wsdo->lockType == WIN32SD_LOCK_UNLOCKED) {
	    wsdo->lockFlags = lockflags;
	    wsdo->surfaceLock->Leave();
	    return SD_FAILURE;
	}
    } else {
	// They didn't ask for a lock, so they don't get one
	wsdo->lockType = WIN32SD_LOCK_BY_NULL;
    }
    wsdo->lockFlags = lockflags;
    DTRACE_PRINTLN2("Win32OSSD_Lock: flags, type = 0x%x, %d\n", 
	wsdo->lockFlags, wsdo->lockType);
    return 0;
}

static void Win32OSSD_GetRasInfo(JNIEnv *env,
			       SurfaceDataOps *ops,
			       SurfaceDataRasInfo *pRasInfo)
{
    Win32SDOps *wsdo = (Win32SDOps *) ops;
    jint lockflags = wsdo->lockFlags;

    DTRACE_PRINTLN("WOSD::Win32OSSD_GetRasInfo");

    if (wsdo->lockType == WIN32SD_LOCK_UNLOCKED) {
	DTRACE_PRINTLN("  lockType == UNLOCKED");
	memset(pRasInfo, 0, sizeof(*pRasInfo));
	return;
    }

    if (wsdo->lockType != WIN32SD_LOCK_BY_DDRAW) {
	/* They didn't lock for anything - we won't give them anything */
	pRasInfo->rasBase = NULL;
	pRasInfo->pixelStride = 0;
	pRasInfo->scanStride = 0;
    }
    if (wsdo->lockFlags & SD_LOCK_LUT) {
	pRasInfo->lutBase = 
	    (long *) wsdo->device->GetSystemPaletteEntries();
	pRasInfo->lutSize = 256;
    } else {
	pRasInfo->lutBase = NULL;
	pRasInfo->lutSize = 0;
    }
    if (wsdo->lockFlags & SD_LOCK_INVCOLOR) {
	pRasInfo->invColorTable = wsdo->device->GetSystemInverseLUT();
	ColorData *cData = wsdo->device->GetColorData();
	pRasInfo->redErrTable = cData->img_oda_red;
	pRasInfo->grnErrTable = cData->img_oda_green;
	pRasInfo->bluErrTable = cData->img_oda_blue;
    } else {
	pRasInfo->invColorTable = NULL;
	pRasInfo->redErrTable = NULL;
	pRasInfo->grnErrTable = NULL;
	pRasInfo->bluErrTable = NULL;
    }
    if (wsdo->lockFlags & SD_LOCK_INVGRAY) {
	pRasInfo->invGrayTable = 
	    wsdo->device->GetColorData()->pGrayInverseLutData;
    } else {
	pRasInfo->invGrayTable = NULL;
    }
}

static void Win32OSSD_Unlock(JNIEnv *env,
			     SurfaceDataOps *ops,
			     SurfaceDataRasInfo *pRasInfo)
{
    Win32SDOps *wsdo = (Win32SDOps *) ops;

    DTRACE_PRINTLN("WOSD::Win32OSSD_Unlock");

    if (wsdo->lockType == WIN32SD_LOCK_UNLOCKED) {
	JNU_ThrowInternalError(env, "Unmatched unlock on Win32OS SurfaceData");
	return;
    }

    if (wsdo->lockType == WIN32SD_LOCK_BY_DDRAW) {
	DTRACE_PRINTLN("Unlocking ddraw surface");
	DDUnlock(env, wsdo);
    }
    wsdo->lockType = WIN32SD_LOCK_UNLOCKED;
    wsdo->surfaceLock->Leave();
}

static void
GetClipFromRegion(JNIEnv *env, jobject clip, RECT &r)
{
    SurfaceDataBounds bounds;
    SurfaceData_GetBoundsFromRegion(env, clip, &bounds);
    r.left = bounds.x1;
    r.top = bounds.y1;
    r.right = bounds.x2;
    r.bottom = bounds.y2;
}

/*
 * REMIND: This mechanism is just a prototype of a way to manage a
 * small cache of DC objects.  It is incomplete in the following ways:
 *
 * - It is not thread-safe!  It needs appropriate locking and release calls
 *   (perhaps the AutoDC mechanisms from Kestrel)
 * - It does hardly any error checking (What if GetDCEx returns NULL?)
 * - It cannot handle printer DCs, their resolution, or Embedded DCs
 * - It always selects a clip region, even if the clip is the window bounds
 * - There is very little error checking (null DC returned from GetDCEx, etc)
 * - It should probably "live" in the native SurfaceData object to allow
 *   alternate implementations for printing and embedding
 * - It doesn't handle XOR
 * - It caches the client bounds to determine if clipping is really needed
 *   (no way to invalidate the cached bounds and there is probably a better
 *    way to manage clip validation in any case)
 */

// REMIND: should use MT-safe way of storing these values
static AwtBrush	*lastBrush;
static AwtPen	*lastPen;
extern COLORREF CheckGrayColor(Win32SDOps *wsdo, int c);

static HDC Win32OSSD_GetDC(JNIEnv *env, Win32SDOps *wsdo,
                           jint type, jint *patrop,
                           jobject clip, jobject comp, jint color)
{
    // REMIND: Should lock around all accesses to "last<mumble>"
    DTRACE_PRINTLN1("Win32OSSD.GetDC, color = 0x%x", color);

    if (wsdo->invalid) {
	SurfaceData_ThrowInvalidPipeException(env, "invalid sd");
	return (HDC) NULL;
    }

    // REMIND: put this in ddrawUtils and catch errors
    HDC hdc = wsdo->lpSurface->GetDC();
    if (hdc == NULL) {
	// Note: DDrawSurface::GetDC() releases its surfaceLock
	// when it returns an error here, so do not call ReleaseDC()
	// to force the release of surfaceLock
	SurfaceData_ThrowInvalidPipeException(env, "invalid sd");
	return (HDC) NULL;
    }
    lastBrush = NULL;
    lastPen = NULL;
    
    // Initialize DC.  Assume nothing about the DC since ddraw DC's are
    // created from scratch every time

    // Since we can't get here in XOR mode (ISCOPY only), we will ignore the
    // comp and force the patrop to PATCOPY if necessary.
    if (patrop != NULL) {
        *patrop = PATCOPY;
    }

    // Setup clipping for DC
    if (clip == NULL) {
	::SelectClipRgn(hdc, (HRGN) NULL);
    } else {
	RECT r;
	GetClipFromRegion(env, clip, r);
	// Only bother setting clip if it's smaller than our window
	if ((r.left > 0) || (r.top > 0) ||
	    (r.right < wsdo->w) || (r.bottom < wsdo->h)) {
	    DTRACE_PRINTLN4("  W32OSSD::GetDC - clipRect = %d, %d, %d, %d",
		r.left, r.top, r.right, r.bottom);
	    //Make the window-relative rect a client-relative one for Windows
	    ::OffsetRect(&r, -wsdo->insets.left, -wsdo->insets.top);
	    HRGN hrgn = ::CreateRectRgnIndirect(&r);
	    ::SelectClipRgn(hdc, hrgn);
	    ::DeleteObject(hrgn);
	}
    }
    if (type & BRUSH) {
	// Setup BRUSH for DC
	AwtBrush *newBrush = AwtBrush::Get(CheckGrayColor(wsdo, color));
	::SelectObject(hdc, newBrush->GetHandle());
	lastBrush = newBrush;
    } else if (type & NOBRUSH) {
	::SelectObject(hdc, nullbrush);
    }
    if (type & PEN) {
	AwtPen *newPen = AwtPen::Get(CheckGrayColor(wsdo, color));
	::SelectObject(hdc, newPen->GetHandle());
	lastPen = newPen;
    } else if (type & NOPEN) {
	::SelectObject(hdc, nullpen);
    }
    return hdc;
}

static void Win32OSSD_ReleaseDC(JNIEnv *env, Win32SDOps *wsdo, HDC hdc)
{
    DTRACE_PRINTLN("W32OSSD::Win32OSSD_ReleaseDC");

    // Release brush and pen, if appropriate.  
    // REMIND: find MT-safe way of doing this.
    // REMIND: AwtGdiObject is set up to Release objects every time
    // instead of preserving the cache - is that what we want?  We're going
    // to thrash using ddraw DC's because we'll keep creating/releasing the
    // same kind of brush/pen every time.
    /*
    ::SelectObject(hdc, nullbrush);
    ::SelectObject(hdc, nullpen);
    if (lastBrush != NULL) {
	lastBrush->Release();
    }
    if (lastPen != NULL) {
	lastPen->Release();
    }
    */
    wsdo->lpSurface->ReleaseDC(hdc);
}

static void Win32OSSD_InvalidateSD(JNIEnv *env, Win32SDOps *wsdo)
{
    DTRACE_PRINTLN("Win32OSSD_InvalidateSD");
    wsdo->invalid = JNI_TRUE;
}

/*
 * Class:     sun_awt_windows_Win32OffScreenSurfaceData
 * Method:    invalidateSD
 * Signature: ()V
 */
JNIEXPORT void JNICALL 
Java_sun_awt_windows_Win32OffScreenSurfaceData_nativeInvalidate(JNIEnv *env, 
								jobject wsd)
{
    Win32SDOps *wsdo = Win32SurfaceData_GetOps(env, wsd);
    DTRACE_PRINTLN("Win32OffScreenSurfaceData_nativeInvalidate");
    if (wsdo != NULL) {
	wsdo->InvalidateSD(env, wsdo);
    }
}

/*
 * Method:    Win32OSSD_Dispose
 */
static void
Win32OSSD_Dispose(JNIEnv *env, SurfaceDataOps *ops)
{
    DTRACE_PRINTLN("Win32OSSD_Dispose");
    // REMIND: Need to delete a lot of other things here as well, starting
    // with the offscreen surface

    // ops is assumed non-null as it is checked in SurfaceData_DisposeOps
    Win32SDOps *wsdo = (Win32SDOps*)ops;
    if (wsdo->surfacePuntData.lpSurfaceVram) {
	delete wsdo->surfacePuntData.lpSurfaceVram;
    }
    if (wsdo->surfacePuntData.lpSurfaceSystem) {
	delete wsdo->surfacePuntData.lpSurfaceSystem;
    }
    wsdo->lpSurface = NULL;
    disposeOSSD_WSDO(env, wsdo);
}

JNIEXPORT void JNICALL 
Java_sun_awt_windows_Win32OffScreenSurfaceData_setTransparentPixel(JNIEnv *env,
                                                                   jobject wsd,
                                                                   jint pixel)
{
    Win32SDOps *wsdo = (Win32SDOps *)SurfaceData_GetOps(env, wsd);
    DDSetColorKey(env, wsdo, pixel);
}

/*
 * Class:     sun_awt_windows_Win32OffScreenSurfaceData
 * Method:    flush
 * Signature: ()V
 */
JNIEXPORT void JNICALL 
Java_sun_awt_windows_Win32OffScreenSurfaceData_flush(JNIEnv *env, 
						     jobject wsd)
{
    DTRACE_PRINTLN("Win32OffScreenSurfaceData_flush");
    Win32SDOps *wsdo = (Win32SDOps *)SurfaceData_GetOps(env, wsd);
    if (wsdo != NULL) {
	// Note that wsdo may be null if there was some error during
	// construction, such as a surface depth we could not handle
	DDReleaseSurfaceMemory(wsdo->surfacePuntData.lpSurfaceSystem);
	DDReleaseSurfaceMemory(wsdo->surfacePuntData.lpSurfaceVram);
    }
}

