/*
 * @(#)awt_MenuComponent.c	1.10 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifdef HEADLESS
    #error This file should not be included in headless library
#endif

#include "java_awt_MenuComponent.h"
#include "jni_util.h"

#include "awt_MenuComponent.h"

struct MenuComponentIDs menuComponentIDs;


JNIEXPORT void JNICALL
Java_java_awt_MenuComponent_initIDs(JNIEnv *env, jclass cls)
{
    menuComponentIDs.font =
      (*env)->GetFieldID(env, cls, "font", "Ljava/awt/Font;");
    menuComponentIDs.privateKey =
      (*env)->GetFieldID(env, cls, "privateKey", "Ljava/lang/Object;");
    menuComponentIDs.getParent =
      (*env)->GetMethodID(
	  env, cls, "getParent_NoClientCode", "()Ljava/awt/MenuContainer;");
}
