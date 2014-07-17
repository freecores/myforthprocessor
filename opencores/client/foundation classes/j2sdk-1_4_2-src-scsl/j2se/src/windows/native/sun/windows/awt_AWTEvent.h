/*
 * @(#)awt_AWTEvent.h	1.8 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
/************************************************************************
 * AwtAWTEvent class
 */

#ifndef AWT_AWTEVENT_H
#define AWT_AWTEVENT_H

#include <jni.h>
#include <jni_util.h>
#include "awt.h"

class AwtAWTEvent {
public:
    /* java.awt.AWTEvent field ids */
    static jfieldID bdataID;
    static jfieldID idID;
    static jfieldID consumedID;
    /* Copy msg to jbyteArray and save it in bdata */
    static void saveMSG(JNIEnv *env, MSG *pMsg, jobject jevent);
};

#endif // AWT_AWTEVENT_H

