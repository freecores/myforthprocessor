/*
 * @(#)awt_Insets.h	1.7 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/************************************************************************
 * AwtInsets class
 */

#ifndef AWT_INSETS_H
#define AWT_INSETS_H

#include <jni.h>
#include <jni_util.h>

class AwtInsets {
public:

    /* java.awt.Insets field ids */
    static jfieldID leftID;
    static jfieldID rightID;
    static jfieldID topID;
    static jfieldID bottomID;

};

#endif // AWT_INSETS_H

