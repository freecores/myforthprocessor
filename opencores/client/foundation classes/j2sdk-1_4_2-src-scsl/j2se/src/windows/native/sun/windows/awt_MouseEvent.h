/*
 * @(#)awt_MouseEvent.h	1.8 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/************************************************************************
 * AwtMouseEvent class
 */

#ifndef AWT_MOUSEEVENT_H
#define AWT_MOUSEEVENT_H

#include <jni.h>
#include <jni_util.h>

class AwtMouseEvent {
public:

    /* java.awt.MouseEvent field ids */
    static jfieldID xID;
    static jfieldID yID;
    static jfieldID buttonID;

};

#endif // AWT_MOUSEEVENT_H

