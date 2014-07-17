/*
 * @(#)awt_Unicode.h	1.15 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * Unicode to ANSI string conversion macros, based on a slide from a
 * presentation by Asmus Freytag.  These must be macros, since the 
 * alloca() has to be in the caller's stack space.
 */

#ifndef AWT_UNICODE_H
#define AWT_UNICODE_H

#include <malloc.h>

// <XXX> <!-- TEMPORARY HACK TO TEST AGAINST OLD VC INLCUDES -->
#if !defined(__int3264)
#define SetWindowLongPtrA SetWindowLongA
#define SetWindowLongPtrW SetWindowLongW
typedef __int32 LONG_PTR;
#endif // __int3264
// </XXX>

// Get a Unicode string copy of a Java String object (Java String aren't
// null-terminated).
extern LPWSTR J2WHelper(LPWSTR lpw, LPWSTR lpj, int nChars);
extern LPWSTR J2WHelper1(LPWSTR lpw, LPWSTR lpj, int offset, int nChars);

extern LPWSTR JNI_J2WHelper1(JNIEnv *env, LPWSTR lpw, jstring jstr);

#define TO_WSTRING(jstr) \
   ((jstr == NULL) ? NULL : \
     (JNI_J2WHelper1(env, (LPWSTR) alloca((env->GetStringLength(jstr)+1)*2), \
		     jstr) \
    ))

// Translate an ANSI string into a Unicode one.
extern LPWSTR A2WHelper(LPWSTR lpw, LPCSTR lpa, int nChars);
#define A2W(lpa) \
    ((NULL == lpa) ? NULL : \
     (A2WHelper((LPWSTR) alloca((strlen(lpa)+1)*2), lpa, strlen(lpa)+1)))

// Translate a Unicode string into an ANSI one.
extern LPSTR W2AHelper(LPSTR lpa, LPCWSTR lpw, int nChars);
#define W2A(lpw) \
    ((NULL == (LPCWSTR)lpw) ? NULL : \
     (W2AHelper((LPSTR) alloca((wcslen(lpw)+1)*2), lpw, (wcslen(lpw)+1)*2)))


// Function pointer definitions
typedef BOOL (CALLBACK* GETCLASSINFOPROC)(HINSTANCE, CONST VOID *, VOID *);
typedef ATOM (CALLBACK* REGISTERCLASSPROC)(CONST VOID *);
typedef BOOL (CALLBACK* UNREGISTERCLASSPROC)(CONST VOID *, HINSTANCE);
typedef LRESULT (CALLBACK* DEFWINDOWPROCPROC)(HWND, UINT, WPARAM, LPARAM);
typedef LRESULT (CALLBACK* CALLWINDOWPROCPROC)(WNDPROC, HWND, UINT, WPARAM, LPARAM);
typedef LONG_PTR (CALLBACK* SETWINDOWLONGPTRPROC)(HWND, int, LONG_PTR);
typedef BOOL (CALLBACK* WINPEEKMESSAGEPROC)(LPMSG, HWND, UINT, UINT, UINT);
typedef LRESULT (CALLBACK* DISPATCHMESSAGEPROC)(CONST MSG *);

extern void initializeUnicodeFunctions(void);

// Function pointers
extern GETCLASSINFOPROC		lpFuncGetClassInfo;
extern REGISTERCLASSPROC	lpFuncRegisterClass;
extern UNREGISTERCLASSPROC	lpFuncUnregisterClass;
extern DEFWINDOWPROCPROC	lpFuncDefWindowProc;
extern CALLWINDOWPROCPROC	lpFuncCallWindowProc;
extern SETWINDOWLONGPTRPROC	lpFuncSetWindowLongPtr;
extern WINPEEKMESSAGEPROC	lpFuncPeekMessage;
extern DISPATCHMESSAGEPROC	lpFuncDispatchMessage;
#endif // AWT_UNICODE_H
