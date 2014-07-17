/*
 * @(#)awt_InputEvent.cpp	1.7 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "awt_InputEvent.h"
#include "awt.h"

/************************************************************************
 * AwtInputEvent fields
 */

jfieldID AwtInputEvent::modifiersID;

/************************************************************************
 * AwtInputEvent native methods
 */

extern "C" {

JNIEXPORT void JNICALL 
Java_java_awt_event_InputEvent_initIDs(JNIEnv *env, jclass cls) {
    AwtInputEvent::modifiersID = env->GetFieldID(cls, "modifiers", "I");

    DASSERT(AwtInputEvent::modifiersID != NULL);
}

} /* extern "C" */
