/*
 * @(#)awt_util.h	1.4 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
#ifndef _AWT_UTIL_H_
#define _AWT_UTIL_H_

#include <Xm/VendorSEP.h>
#include <Xm/VendorSP.h>

int awt_util_focusIsOnMenu(Display *display);
int awt_util_sendButtonClick(Display *display, Window window);
void awt_util_dispatchButtonClick(Display *display, Window window);

Widget awt_util_createWarningWindow(Widget parent, char *warning);
void awt_util_show(Widget w);
void awt_util_hide(Widget w);
void awt_util_enable(Widget w);
void awt_util_disable(Widget w);
void awt_util_reshape(Widget w, long x, long y, long wd, long h);
void awt_util_mapChildren(Widget w, void (*func)(Widget,void *),
			  int applyToSelf, void *data);
int awt_util_setCursor(Widget w, Cursor c);
Widget awt_WidgetAtXY(Widget root, Position x, Position y);
int awt_util_getIMStatusHeight(Widget vw);
void awt_util_setMinMaxSizeProps(Widget shellW, Boolean set);
void awt_util_setShellResizable(Widget shellW, Boolean isMapped);
void awt_util_setShellNotResizable(Widget shellW, long width, long height, Boolean isMapped);
unsigned awt_util_runningWindowManager();
Cardinal awt_util_insertCallback(Widget w);
void awt_util_consumeAllXEvents(Widget widget);
void awt_util_cleanupBeforeDestroyWidget(Widget widget);

struct DPos {
    long x;
    long y;
    int mapped;
    void *data;
    void *peer;
    long echoC;
};

extern jobject awtJNI_GetCurrentThread(JNIEnv *env);
extern void awtJNI_ThreadYield(JNIEnv *env);
extern Widget prevWidget;

/*
 * Functions for accessing fields by name and signature
 */

JNIEXPORT jobject JNICALL
JNU_GetObjectField(JNIEnv *env, jobject self, const char *name,
		   const char *sig);

JNIEXPORT jboolean JNICALL
JNU_SetObjectField(JNIEnv *env, jobject self, const char *name,
		   const char *sig, jobject val);

JNIEXPORT jlong JNICALL
JNU_GetLongField(JNIEnv *env, jobject self, const char *name);

JNIEXPORT jint JNICALL
JNU_GetIntField(JNIEnv *env, jobject self, const char *name);

JNIEXPORT jboolean JNICALL
JNU_SetIntField(JNIEnv *env, jobject self, const char *name, jint val);

JNIEXPORT jboolean JNICALL
JNU_SetLongField(JNIEnv *env, jobject self, const char *name, jlong val);

JNIEXPORT jboolean JNICALL
JNU_GetBooleanField(JNIEnv *env, jobject self, const char *name);

JNIEXPORT jboolean JNICALL
JNU_SetBooleanField(JNIEnv *env, jobject self, const char *name, jboolean val);

JNIEXPORT jint JNICALL
JNU_GetCharField(JNIEnv *env, jobject self, const char *name);



int awt_util_getIMStatusHeight(Widget vw);
XVaNestedList awt_util_getXICStatusAreaList(Widget w);
Widget awt_util_getXICStatusAreaWindow(Widget w);

#endif           /* _AWT_UTIL_H_ */
