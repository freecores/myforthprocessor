/*
 * @(#)awt.h	1.4 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * Common AWT definitions
 */

#ifndef _AWT_
#define _AWT_

#include "jni_util.h"
#include "jvm.h"

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Intrinsic.h>

#define JNI_AWT_LOCK

#ifdef NETSCAPE
#include "prmon.h"

extern PRMonitor *_pr_rusty_lock;
extern void PR_XWait(int ms);
extern void PR_XNotify(void);
extern void PR_XNotifyAll(void);

#define AWT_LOCK() PR_XLock()

#define AWT_NOFLUSH_UNLOCK() PR_XUnlock()

#define AWT_UNLOCK() awt_output_flush(); PR_XUnlock()

#define AWT_FLUSH_UNLOCK() awt_output_flush(); PR_XUnlock()

#define AWT_WAIT(tm) PR_XWait(tm)

#define AWT_NOTIFY() PR_XNotify()

#define AWT_NOTIFY_ALL() PR_XNotifyAll()

#else

#ifdef DEBUG
#define DEBUG_AWT_LOCK
#endif

#ifdef DEBUG_AWT_LOCK

extern int awt_locked;
extern char *lastF;
extern int lastL;

#ifdef JNI_AWT_LOCK

#define AWT_LOCK()\
if (awt_lock == 0) {\
jio_fprintf(stderr, "AWT lock error, awt_lock is null\n");\
}\
if (awt_locked < 0) {\
jio_fprintf(stderr, "AWT lock error (%s,%d) (last held by %s,%d) %d\n",\
__FILE__, __LINE__,lastF,lastL,awt_locked);\
}\
lastF=__FILE__;\
lastL=__LINE__;\
(*env)->MonitorEnter(env,awt_lock);awt_locked++

#define AWT_NOFLUSH_UNLOCK()\
lastF=""; lastL=-1;\
if (awt_locked < 1) {\
jio_fprintf(stderr, "AWT unlock error (%s,%d,%d)\n", __FILE__,__LINE__,awt_locked);\
}\
awt_locked--;(*env)->MonitorExit(env,awt_lock)

#define AWT_UNLOCK()\
awt_output_flush(); AWT_NOFLUSH_UNLOCK()

#define AWT_FLUSH_UNLOCK()\
awt_output_flush(); AWT_NOFLUSH_UNLOCK()

#define AWT_WAIT(tm)\
if (awt_locked < 1) {\
jio_fprintf(stderr, "AWT wait error (%s,%d,%d)\n", __FILE__,__LINE__,awt_locked);\
}\
{ int old_lockcount = awt_locked; awt_locked = 0; \
JNU_MonitorWait(env,awt_lock,(tm)); \
awt_locked = old_lockcount; }

#define AWT_NOTIFY()\
if (awt_locked < 1) {\
jio_fprintf(stderr, "AWT notify error (%s,%d,%d)\n", __FILE__,__LINE__,awt_locked);\
}\
JNU_Notify(env,awt_lock)

#define AWT_NOTIFY_ALL()\
if (awt_locked < 1) {\
jio_fprintf(stderr, "AWT notify all error (%s,%d,%d)\n", __FILE__,__LINE__,awt_locked);\
}\
JNU_NotifyAll(env,awt_lock)

#else /* Not JNI Locks */

#define AWT_LOCK()\
if (awt_lock == 0) {\
jio_fprintf(stderr, "AWT lock error, awt_lock is null\n");\
}\
if (awt_locked < 0) {\
jio_fprintf(stderr, "AWT lock error (%s,%d) (last held by %s,%d) %d\n",\
__FILE__, __LINE__,lastF,lastL,awt_locked);\
}\
lastF=__FILE__;\
lastL=__LINE__;\
monitorEnter(obj_monitor(awt_lock));awt_locked++

#define AWT_NOFLUSH_UNLOCK()\
lastF=""; lastL=-1;\
if (awt_locked < 1) {\
jio_fprintf(stderr, "AWT unlock error (%s,%d,%d)\n", __FILE__,__LINE__,awt_locked);\
}\
awt_locked--;monitorExit(obj_monitor(awt_lock))

#define AWT_UNLOCK()\
awt_output_flush(); AWT_NOFLUSH_UNLOCK()

#define AWT_FLUSH_UNLOCK()\
awt_output_flush(); AWT_NOFLUSH_UNLOCK()

#define AWT_WAIT(tm)\
if (awt_locked < 1) {\
jio_fprintf(stderr, "AWT wait error (%s,%d,%d)\n", __FILE__,__LINE__,awt_locked);\
}\
{ int old_lockcount = awt_locked; awt_locked = 0; \
monitorWait(obj_monitor(awt_lock), (tm));
awt_locked = old_lockcount; }

#define AWT_NOTIFY()\
if (awt_locked < 1) {\
jio_fprintf(stderr, "AWT notify error (%s,%d,%d)\n", __FILE__,__LINE__,awt_locked);\
}\
monitorNotify(obj_monitor(awt_lock))

#define AWT_NOTIFY_ALL()\
if (awt_locked < 1) {\
jio_fprintf(stderr, "AWT notify all error (%s,%d,%d)\n", __FILE__,__LINE__,awt_locked);\
}\
monitorNotifyAll(obj_monitor(awt_lock))

#endif /* JNI_AWT_LOCK */

#else /* else not debug locks */

#ifdef JNI_AWT_LOCK

#define AWT_LOCK()\
(*env)->MonitorEnter(env,awt_lock)

#define AWT_NOFLUSH_UNLOCK()\
(*env)->MonitorExit(env,awt_lock)

#define AWT_UNLOCK()\
awt_output_flush(); AWT_NOFLUSH_UNLOCK()

#define AWT_FLUSH_UNLOCK()\
awt_output_flush(); AWT_NOFLUSH_UNLOCK()

#define AWT_WAIT(tm)\
JNU_MonitorWait(env,awt_lock,(tm))

#define AWT_NOTIFY()\
JNU_Notify(env,awt_lock)

#define AWT_NOTIFY_ALL()\
JNU_NotifyAll(env,awt_lock)

#else /* Not JNI Locks */

#define AWT_LOCK()\
monitorEnter(obj_monitor(awt_lock))

#define AWT_NOFLUSH_UNLOCK()\
monitorExit(obj_monitor(awt_lock))

#define AWT_UNLOCK()\
awt_output_flush(); AWT_NOFLUSH_UNLOCK()

#define AWT_FLUSH_UNLOCK()\
awt_output_flush(); AWT_NOFLUSH_UNLOCK()

#define AWT_WAIT(tm)\
monitorWait(obj_monitor(awt_lock), (tm))

#define AWT_NOTIFY()\
monitorNotify(obj_monitor(awt_lock))

#define AWT_NOTIFY_ALL()\
monitorNotifyAll(obj_monitor(awt_lock))

#endif /* JNI Locks */

#endif				/* DEBUG_AWT_LOCK */

#endif /* NETSCAPE */

    /* We have to leave these in here until the Tiny Awt is ported. */

#define JAVA_UPCALL(args)\
execute_java_dynamic_method args;\
if (exceptionOccurred(EE())) {\
exceptionDescribe(EE());\
exceptionClear(EE());\
}

#define JAVA_UPCALL_UNLOCKED(args)\
execute_java_dynamic_method args;\
if (exceptionOccurred(EE())) {\
exceptionDescribe(EE());\
exceptionClear(EE());\
}

extern Display		*awt_display;
extern Window		awt_root;
extern long		awt_screen;
extern int		awt_whitepixel;
extern int		awt_blackpixel;
extern int		awt_multiclick_time;
extern int              awt_multiclick_smudge;
extern int		awt_MetaMask;
extern int		awt_AltMask;
extern int              awt_NumLockMask;
extern Cursor           awt_scrollCursor;

extern Boolean awt_isSelectionOwner(char *sel_str);
extern void awt_notifySelectionLost(char *sel_str);

extern jobject awt_lock;

extern int awtJNI_GetColor(JNIEnv *env,jobject thisObj);

extern Boolean awtJNI_isSelectionOwner(JNIEnv *env,char *sel_str);
extern void awtJNI_notifySelectionLost(JNIEnv *env,char *sel_str);

/* The JVM instance */
extern JavaVM *jvm;
#endif
