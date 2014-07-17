/*
 * @(#)awt_Window.h	1.10 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include <jni_util.h>

/* fieldIDs for Window fields that may be accessed from C */
struct WindowIDs {
    jfieldID warningString;
    jmethodID resetGCMID;
    jmethodID focusableWindowMID;
    jfieldID focusableWindowState;
};

/* fieldIDs for MWindowPeer fields that may be accessed from C */
struct MWindowPeerIDs {
    jfieldID insets;
    jfieldID iconWidth;
    jfieldID iconHeight;
    jfieldID winAttr;
    jmethodID handleWindowFocusIn;
    jmethodID handleWindowFocusOut;
    jmethodID handleIconify;
    jmethodID handleDeiconify;
    jmethodID handleStateChange;
    jmethodID draggedToScreenMID;
};

