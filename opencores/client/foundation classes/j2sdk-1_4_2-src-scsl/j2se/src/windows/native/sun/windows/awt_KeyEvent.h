/*
 * @(#)awt_KeyEvent.h	1.7 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/************************************************************************
 * AwtKeyEvent class
 */

#ifndef AWT_KEYEVENT_H
#define AWT_KEYEVENT_H

#include <jni.h>
#include <jni_util.h>

class AwtKeyEvent {
public:

    /* java.awt.KeyEvent field ids */
    static jfieldID keyCodeID;
    static jfieldID keyCharID;

};

#endif // AWT_KEYEVENT_H

