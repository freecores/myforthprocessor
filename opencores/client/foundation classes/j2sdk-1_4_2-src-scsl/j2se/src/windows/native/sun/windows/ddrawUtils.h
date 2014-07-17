/*
 * @(#)ddrawUtils.h	1.24 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef DDRAWUTILS_H
#define DDRAWUTILS_H


#include <ddraw.h>
#include <jni.h>
#include <windows.h>
#include "Win32SurfaceData.h"
#include "ddrawObject.h"

// J2dTrace
// Trace utility used by our DirectX code.  Uses a "level"
// parameter that allows user to specify how much detail
// they want traced at runtime.  Tracing is only enabled
// in debug mode, to avoid overhead running release build.

#define J2D_TRACE_OFF 		0
#define J2D_TRACE_ERROR 	1
#define J2D_TRACE_WARNING 	2
#define J2D_TRACE_INFO 		3
#define J2D_TRACE_VERBOSE 	4

void J2dTraceImpl(int level, BOOL cr, const char *string, ...);

#ifndef DEBUG
#define J2dTrace(level, string)
#define J2dTrace1(level, string, arg1)
#define J2dTrace2(level, string, arg1, arg2)
#define J2dTrace3(level, string, arg1, arg2, arg3)
#define J2dTrace4(level, string, arg1, arg2, arg3, arg4)
#define J2dTrace5(level, string, arg1, arg2, arg3, arg4, arg5)
#define J2dTraceLn(level, string)
#define J2dTraceLn1(level, string, arg1)
#define J2dTraceLn2(level, string, arg1, arg2)
#define J2dTraceLn3(level, string, arg1, arg2, arg3)
#define J2dTraceLn4(level, string, arg1, arg2, arg3, arg4)
#define J2dTraceLn5(level, string, arg1, arg2, arg3, arg4, arg5)
#else // DEBUG
#define J2dTrace(level, string) { \
	    J2dTraceImpl(level, FALSE, string); \
	    DTRACE_PRINT(string); \
	}
#define J2dTrace1(level, string, arg1) { \
	    J2dTraceImpl(level, FALSE, string, arg1); \
	    DTRACE_PRINT1(string, arg1); \
	}
#define J2dTrace2(level, string, arg1, arg2) { \
	    J2dTraceImpl(level, FALSE, string, arg1, arg2); \
	    DTRACE_PRINT2(string, arg1, arg2); \
	}
#define J2dTrace3(level, string, arg1, arg2, arg3) { \
	    J2dTraceImpl(level, FALSE, string, arg1, arg2, arg3); \
	    DTRACE_PRINT3(string, arg1, arg2, arg3); \
	}
#define J2dTrace4(level, string, arg1, arg2, arg3, arg4) { \
	    J2dTraceImpl(level, FALSE, string, arg1, arg2, arg3, arg4); \
	    DTRACE_PRINT4(string, arg1, arg2, arg3, arg4); \
	}
#define J2dTrace5(level, string, arg1, arg2, arg3, arg4, arg5) { \
	    J2dTraceImpl(level, FALSE, string, arg1, arg2, arg3, arg4, arg5); \
	    DTRACE_PRINT5(string, arg1, arg2, arg3, arg4, arg5); \
	}
#define J2dTraceLn(level, string) { \
	    J2dTraceImpl(level, TRUE, string); \
	    DTRACE_PRINTLN(string); \
	}
#define J2dTraceLn1(level, string, arg1) { \
	    J2dTraceImpl(level, TRUE, string, arg1); \
	    DTRACE_PRINTLN1(string, arg1); \
	}
#define J2dTraceLn2(level, string, arg1, arg2) { \
	    J2dTraceImpl(level, TRUE, string, arg1, arg2); \
	    DTRACE_PRINTLN2(string, arg1, arg2); \
	}
#define J2dTraceLn3(level, string, arg1, arg2, arg3) { \
	    J2dTraceImpl(level, TRUE, string, arg1, arg2, arg3); \
	    DTRACE_PRINTLN3(string, arg1, arg2, arg3); \
	}
#define J2dTraceLn4(level, string, arg1, arg2, arg3, arg4) { \
	    J2dTraceImpl(level, TRUE, string, arg1, arg2, arg3, arg4); \
	    DTRACE_PRINTLN4(string, arg1, arg2, arg3, arg4); \
	}
#define J2dTraceLn5(level, string, arg1, arg2, arg3, arg4, arg5) { \
	    J2dTraceImpl(level, TRUE, string, arg1, arg2, arg3, arg4, arg5); \
	    DTRACE_PRINTLN5(string, arg1, arg2, arg3, arg4, arg5); \
	}
#endif // DEBUG

/**
 * Direct Draw utility functions
 */

BOOL	DDInit();

void	DDRelease();

void	DDReleaseSurfaceMemory(DDrawSurface *lpSurface);

BOOL	D3DTest(DDrawObjectStruct *tmpDdInstance);

BOOL	DDSetupDevice(DDrawObjectStruct *tmpDdInstance);

BOOL    DDCreateObject();

BOOL    DDCreatePrimary(Win32SDOps *wsdo);

void	DDSync();

BOOL	DDCanCreatePrimary(HMONITOR hMon);

BOOL	DDCanBlt(Win32SDOps *wsdo);

BOOL	DDUseDDraw(Win32SDOps *wsdo);

BOOL	D3DEnabled(Win32SDOps *wsdo);

void	DDInvalidateDDInstance(DDrawObjectStruct *ddInst);

void	ReleaseDDInstance(DDrawObjectStruct *ddInst);

BOOL    DDEnterFullScreen(HMONITOR hMon, HWND hwnd);

BOOL    DDExitFullScreen(HMONITOR hMon, HWND hwnd);

BOOL    DDGetDisplayMode(HMONITOR hMon, DDrawDisplayMode& displayMode);

BOOL    DDSetDisplayMode(HMONITOR hMon, DDrawDisplayMode& displayMode);

BOOL    DDEnumDisplayModes(HMONITOR hMon, DDrawDisplayMode* constraint,
                           DDrawDisplayMode::Callback callback, void* context);

BOOL	DDClipCheck(Win32SDOps *wsdo, RECT *operationRect);

BOOL	DDLock(JNIEnv *env, Win32SDOps *wsdo, RECT *lockRect, 
	       SurfaceDataRasInfo *pRasInfo);

void	DDUnlock(JNIEnv *env, Win32SDOps *wsdo);

BOOL	DDColorFill(JNIEnv *env, jobject sData, Win32SDOps *wsdo, 
		    RECT *fillRect, jint color);

BOOL	DDBlt(JNIEnv *env, Win32SDOps *wsdoSrc, Win32SDOps *wsdoDst,
	      RECT *rDst, RECT *rSrc, CompositeInfo *compInfo = NULL);

void    DDSetColorKey(JNIEnv *env, Win32SDOps *wsdo, jint color);

BOOL	D3DLine(JNIEnv *env, Win32SDOps *wsdo, int x1, int y1,
		int x2, int y2, int clipX1, int clipY1, int clipX2, int clipY2, 
		jint color);

BOOL	D3DRect(JNIEnv *env, Win32SDOps *wsdo, int x, int y, 
		int width, int height, int clipX1, int clipY1, 
		int clipX2, int clipY2, jint color);

BOOL    DDFlip(JNIEnv *env, Win32SDOps *src, Win32SDOps *dest);

BOOL	DDRestoreSurface(Win32SDOps *wsdo);

jint	DDGetAvailableMemory(HMONITOR hMon);

BOOL	DDCreateSurface(Win32SDOps *wsdo, jboolean isVolatile);

BOOL	DDCreateOffScreenSurface(Win32SDOps *wsdo, jboolean isVolatile, 
				 DDrawObjectStruct *ddInst);

BOOL    DDGetAttachedSurface(JNIEnv *env, Win32SDOps* wsdo_parent, Win32SDOps* wsdo);

void	DDDestroySurface(Win32SDOps *wsdo);

BOOL	DDCanReplaceSurfaces(HWND hwnd);

void	PrintDirectDrawError(DWORD errNum, char *message);

void	DebugPrintDirectDrawError(DWORD errNum, char *message);

void	GetDDErrorString(DWORD errNum, char *buffer);


#endif DDRAWUTILS_H
