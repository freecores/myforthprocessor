/*
 * @(#)awt_Clipboard.h	1.14 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef AWT_CLIPBOARD_H
#define AWT_CLIPBOARD_H

#include "awt.h"


/************************************************************************
 * AwtClipboard class
 */

class AwtClipboard {
private:
    static BOOL isGettingOwnership;

public:
    static jmethodID lostSelectionOwnershipMID;
    static jobject theCurrentClipboard;

    INLINE static void GetOwnership() {
	AwtClipboard::isGettingOwnership = TRUE;
	VERIFY(EmptyClipboard());
	AwtClipboard::isGettingOwnership = FALSE;
    }

    INLINE static BOOL IsGettingOwnership() {
	return isGettingOwnership;
    }

    static void LostOwnership(JNIEnv *env);
};

#endif /* AWT_CLIPBOARD_H */
