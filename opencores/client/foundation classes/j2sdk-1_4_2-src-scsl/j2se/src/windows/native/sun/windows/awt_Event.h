/*
 * @(#)awt_Event.h	1.7 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/************************************************************************
 * AwtEvent class
 */

#ifndef AWT_EVENT_H
#define AWT_EVENT_H

#include <jni.h>
#include <jni_util.h>

class AwtEvent {
public:

    /* java.awt.Event field ids */
    static jfieldID targetID;
    static jfieldID xID;
    static jfieldID yID;


};

#endif // AWT_EVENT_H

