/*
 * @(#)awt_KeyboardFocusManager.h	1.6 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "jni.h"

struct KeyboardFocusManagerIDs {
    jclass keyboardFocusManagerCls;
    jmethodID shouldNativelyFocusHeavyweightMID;
    jmethodID heavyweightButtonDownMID;
    jmethodID heavyweightButtonDownZMID;
    jmethodID markClearGlobalFocusOwnerMID;
    jmethodID processSynchronousTransferMID;
    jfieldID isProxyActive;
};
