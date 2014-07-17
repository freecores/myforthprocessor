/*
 * @(#)awt_Unicode.cpp	1.10 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "awt.h"
#define AWT_UNICODE_CPP
#include "awt_Unicode.h"
#undef AWT_UNICODE_CPP

LPWSTR J2WHelper1(LPWSTR lpw, LPWSTR lpj, int offset, int nChars) {
    memcpy(lpw, lpj + offset, nChars*2);
    lpw[nChars] = '\0';
    return lpw;
}

LPWSTR JNI_J2WHelper1(JNIEnv *env, LPWSTR lpwstr, jstring jstr) {

    int len = env->GetStringLength(jstr);

    env->GetStringRegion(jstr, 0, len, lpwstr);
    lpwstr[len] = '\0';

    return lpwstr;
}

LPWSTR J2WHelper(LPWSTR lpw, LPWSTR lpj,  int nChars) {
    return J2WHelper1(lpw, lpj, 0, nChars);
}

// Translate an ANSI string into a Unicode one.
LPWSTR A2WHelper(LPWSTR lpw, LPCSTR lpa, int nChars) {
    lpw[0] = '\0';
    VERIFY(::MultiByteToWideChar(CP_ACP, 0, lpa, -1, lpw, nChars));
    return lpw;
}

// Translate a Unicode string into an ANSI one.
LPSTR W2AHelper(LPSTR lpa, LPCWSTR lpw, int nChars) {
    lpa[0] = '\0';
    VERIFY(::WideCharToMultiByte(CP_ACP, 0, lpw, -1, lpa, nChars, NULL, NULL));
    return lpa;
}

// Function pointers
GETCLASSINFOPROC	lpFuncGetClassInfo = NULL;
REGISTERCLASSPROC	lpFuncRegisterClass = NULL;
UNREGISTERCLASSPROC	lpFuncUnregisterClass = NULL;
DEFWINDOWPROCPROC	lpFuncDefWindowProc = NULL;
CALLWINDOWPROCPROC	lpFuncCallWindowProc = NULL;
SETWINDOWLONGPTRPROC	lpFuncSetWindowLongPtr = NULL;
WINPEEKMESSAGEPROC	lpFuncPeekMessage = NULL;
DISPATCHMESSAGEPROC	lpFuncDispatchMessage = NULL;

void initializeUnicodeFunctions() {
    if (IS_NT) {
	lpFuncGetClassInfo = (GETCLASSINFOPROC)::GetClassInfoW;
	lpFuncRegisterClass = (REGISTERCLASSPROC)::RegisterClassW;
	lpFuncUnregisterClass = (UNREGISTERCLASSPROC)::UnregisterClassW;
	lpFuncDefWindowProc = ::DefWindowProcW;
	lpFuncCallWindowProc = ::CallWindowProcW;
	lpFuncSetWindowLongPtr = (SETWINDOWLONGPTRPROC)::SetWindowLongPtrW;
	lpFuncPeekMessage = ::PeekMessageW;
	lpFuncDispatchMessage = ::DispatchMessageW;
    } else {
	lpFuncGetClassInfo = (GETCLASSINFOPROC)::GetClassInfoA;
	lpFuncRegisterClass = (REGISTERCLASSPROC)::RegisterClassA;
	lpFuncUnregisterClass = (UNREGISTERCLASSPROC)::UnregisterClassA;
	lpFuncDefWindowProc = ::DefWindowProcA;
	lpFuncCallWindowProc = ::CallWindowProcA;
	lpFuncSetWindowLongPtr = (SETWINDOWLONGPTRPROC)::SetWindowLongPtrA;
	lpFuncPeekMessage = ::PeekMessageA;
	lpFuncDispatchMessage = ::DispatchMessageA;
    }
}
