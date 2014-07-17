/*
 *  @(#)native.c	1.46 03/04/25
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


/*
 * Native Methods to support Java Plug-in on Un*x systems.
 *
 * This gets compiled with -DJDK11 for 1.1 and -DJDK12 for 1.2
 *
 *						    KGH Dec 97
 */

#include "sun_plugin_navig_motif_Plugin.h"
#include "sun_plugin_navig_motif_Worker.h"
#include "sun_plugin_viewer_MNetscapePluginContext.h"
#include "sun_plugin_JavaRunTime.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#if 0
#include <sys/socket.h>
#else
#include <stropts.h>
#endif
#include <X11/Xatom.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Vendor.h>
#include <X11/Shell.h>
#include <X11/IntrinsicP.h>
#include "plugin_defs.h"
#include <dlfcn.h>

/* This is the one and only AWT header file allowed. */
/*#include "awt_Plugin.h"*/

/* AWT interface functions. */
static void (* LockIt)(JNIEnv *) = NULL;
static void (* UnLockIt)(JNIEnv *) = NULL;
static void (* NoFlushUnlockIt)(JNIEnv *) = NULL;
static void *res = NULL;
static initialized_lock = 0;
static Display *display;


static int trace_native = 1;
/*
 * Create a Java FileDescriptor object for a given Unix descriptor number
 *      fd is the file descriptor number
 */

#ifdef DO_TRACE
#define TRACE(i) fprintf(stderr, "%s\n", i);
#else
#define TRACE(i) 
#endif

/*
 * Returns a new Java string object for the specified platform string.
 */
static jstring
NewPlatformString(JNIEnv *env, char *s) {
    
    int len = (int)strlen(s);
    jclass cls;
    jmethodID mid;
    jbyteArray ary;

    if ((cls = (*env)->FindClass(env, "java/lang/String")) == NULL)
      return 0;

    if ((mid = (*env)->GetMethodID(env, cls, "<init>", "([B)V")) == NULL)
      return 0;

    ary = (*env)->NewByteArray(env, len);

    if (ary != 0) {
	jstring str = 0;
	(*env)->SetByteArrayRegion(env, ary, 0, len, (jbyte *)s);
	if (!(*env)->ExceptionOccurred(env)) {
	    str = (*env)->NewObject(env, cls, mid, ary);
	}
	(*env)->DeleteLocalRef(env, ary);
	return str;
    }
    return 0;
}
  
static void *awtHandle = NULL;

static void initAwtHandle() {
    char awtPath[MAXPATHLEN];

    sprintf(awtPath, "%s/lib/" LIBARCH "/libawt.so",
	    getenv("JAVA_HOME"));

    awtHandle = dlopen(awtPath, RTLD_LAZY);
    if (awtHandle == NULL) {
	fprintf(stderr,"reflect - bad awtHandle.\n");
	exit(123);
    }
    return;
}


#define REFLECT_VOID_FUNCTION(name, arglist, paramlist)			\
typedef name##_type arglist;						\
static void name arglist						\
{									\
    static name##_type *name##_ptr = NULL;				\
    if (name##_ptr == NULL) {						\
        if (awtHandle == NULL) {					\
	    initAwtHandle();						\
	}								\
	name##_ptr = (name##_type *) dlsym(awtHandle, #name);		\
	if (name##_ptr == NULL) {					\
	    fprintf(stderr,"reflect failed to find " #name ".\n");	\
	    exit(123);							\
	    return;							\
	}								\
    }									\
    (*name##_ptr)paramlist;						\
}

#define REFLECT_FUNCTION(return_type, name, arglist, paramlist)		\
typedef return_type name##_type arglist;				\
static return_type name arglist						\
{									\
    static name##_type *name##_ptr = NULL;				\
    if (name##_ptr == NULL) {						\
        if (awtHandle == NULL) {					\
	    initAwtHandle();						\
	}								\
	name##_ptr = (name##_type *) dlsym(awtHandle, #name);		\
	if (name##_ptr == NULL) {					\
	    fprintf(stderr,"reflect failed to find " #name ".\n");	\
	    exit(123);							\
	    return;							\
	}								\
    }									\
    return (*name##_ptr)paramlist;					\
}


/*
 * These entry points must remain in libawt.so ***for Java Plugin ONLY***
 * Reflect this call over to the correct libmawt.so.
 */

REFLECT_VOID_FUNCTION(getAwtLockFunctions,
		      (void (**AwtLock)(JNIEnv *), void (**AwtUnlock)(JNIEnv *),
		       void (**AwtNoFlushUnlock)(JNIEnv *), void *reserved), 
		      (AwtLock, AwtUnlock, AwtNoFlushUnlock, reserved))

REFLECT_VOID_FUNCTION(getAwtData,
		      (int *awt_depth, Colormap *awt_cmap, Visual **awt_visual,
		       int *awt_num_colors, void *pReserved),
		      (awt_depth, awt_cmap, awt_visual,
		       awt_num_colors, pReserved))

REFLECT_FUNCTION(Display *, getAwtDisplay, (void), ())


/* Event Handler to correct for Shell position */
static void
checkPos(Widget w, XtPointer data, XEvent *event)
{
        /* this is heinous, but necessary as we need to update
        ** the X,Y position of the shell if netscape has moved.
        ** we have to do this so that XtTranslateCoords used by
        ** popups and the like get the proper screen positions
        ** Additionally we can use XtSet/ XtMove/ConfigureWidget
        ** As the widget code will think the the shell has moved
        ** and generate a XConfigure which WILL move the window
        ** We are only trying to correct for the reparent hack.
        ** sigh.
        */

        w->core.x = event->xcrossing.x_root - event->xcrossing.x;
        w->core.y = event->xcrossing.y_root - event->xcrossing.y;
}
/* Event Handler to correct for Shell position */
static void
propertyHandler(Widget w, XtPointer data, XEvent *event)
{
        /* this is heinous, but necessary as we need to update
        ** the X,Y position of the shell is changed to wrong value.
        ** we have to do this so that XtTranslateCoords used by
        ** popups and the like get the proper screen positions
        ** Additionally we can use XtSet/ XtMove/ConfigureWidget
        ** 
        */
  int px, py;
  Window dummy;
  
  XTranslateCoordinates(display, XtWindow(w), DefaultRootWindow(display), 0,0, &px, &py, &dummy);
  
  w->core.x=px;
  w->core.y=py;
     
}

/*
 * Create a local managed widget inside a given X window.
 * We allocate a top-level shell and then reparent it into the
 * given window id.
 *
 * This is used to take the X11 window ID that has been passed
 * to us by our parent Navigator plugin and return a widget
 * that can be used as the base for our Java EmbeddeFrame.
 *
 * Note that the ordering of the various calls is tricky here as
 * we have to cope with the variations between 1.1.3, 1.1.6,
 * and 1.2.
 */
JNIEXPORT jint JNICALL
Java_sun_plugin_viewer_MNetscapePluginContext_getWidget(
		JNIEnv *env, jclass clz,
		jint winid, jint width, jint height, jint x, jint y)
{
    Arg args[40];
    int argc;
    Widget w;
    Window child, parent;
    Visual *visual;
    Colormap cmap;
    int depth;
    int ncolors;
    Display **awt_display_ptr;

    /* Initialize access to AWT lock functions. */
    if (initialized_lock == 0) {
	getAwtLockFunctions(&LockIt, &UnLockIt, &NoFlushUnlockIt, NULL);
	initialized_lock = 1 ;
    }

    /*
     * Create a top-level shell.  Note that we need to use the
     * AWT's own awt_display to initialize the widget.  If we
     * try to create a second X11 display connection the Java
     * runtimes get very confused.
     */
    (*LockIt)(env);
    argc = 0;
    XtSetArg(args[argc], XtNsaveUnder, False); argc++;
    XtSetArg(args[argc], XtNallowShellResize, False); argc++;

    /* the awt initialization should be done by now (awt_GraphicsEnv.c) */

    getAwtData(&depth,&cmap,&visual,&ncolors,NULL);

    awt_display_ptr = (Display **) dlsym(awtHandle, "awt_display");
    if (awt_display_ptr == NULL)
	display = getAwtDisplay();
    else
	display = *awt_display_ptr;

    XtSetArg(args[argc], XtNvisual, visual); argc++;
    XtSetArg(args[argc], XtNdepth, depth); argc++;
    XtSetArg(args[argc], XtNcolormap, cmap); argc++;

    XtSetArg(args[argc], XtNwidth, width); argc++;
    XtSetArg(args[argc], XtNheight, height); argc++;
    /* The shell has to have relative coords of O,0? */
    XtSetArg(args[argc], XtNx, 0); argc++;
    XtSetArg(args[argc], XtNy, 0); argc++;

    /* The shell widget starts out as a top level widget.
     * Without intervention, it will be managed by the window
     * manager and will be its own widow. So, until it is reparented,
     *  we don't map it. 
     */
    XtSetArg(args[argc], XtNmappedWhenManaged, False); argc++;

    w = XtAppCreateShell("AWTapp","XApplication",
				    vendorShellWidgetClass,
				    display,
				    args,
				    argc);
    XtRealizeWidget(w);

    /* 
     * i think the following 2 lines wont be needed because of fix of 4419207
     * the function checkPos and propertyHandler can also be deleted
     * please let me know if testing shows otherwise
     * see awt_addEmbeddedFrame in awt_util.c
     * tao.ma@eng
     */
    /*
    XtAddEventHandler(w, EnterWindowMask, FALSE,(XtEventHandler) checkPos, 0);
    XtAddEventHandler(w, PropertyChangeMask , FALSE,(XtEventHandler) propertyHandler, 0);
    */
    /*
     * Now reparent our new Widget into our Navigator window
     */
    parent = (Window) winid;
    child = XtWindow(w);
    XReparentWindow(display, child, parent, 0, 0);
    XFlush(display);
    XSync(display, False); 
    XtVaSetValues(w, XtNx, 0, XtNy, 0, NULL);
    XFlush(display);
    XSync(display, False);

    (*UnLockIt)(env);	

    return (int)w;
}

/*
 * Make sure the given widget is mapped.
 *
 * This isn't necessary on JDK 1.1.5 but is needed on JDK 1.1.4
 */
JNIEXPORT jint JNICALL
Java_sun_plugin_viewer_MNetscapePluginContext_mapWidget
(JNIEnv *env, jclass clz, jint widget, 
 jint wd, jint ht, jint x, jint y) 
{
    Widget w = (Widget)widget;
    /*
     * this is what JDK 1.1.5 does in MFramePeer.pShow.
     */
    (*LockIt)(env);
    XtSetMappedWhenManaged(w, True);
    XtPopup(w, XtGrabNone);
    (*UnLockIt)(env);
    return (jint) 1;
}


/**
 * Return true if our parent process is still alive.
 */
JNIEXPORT jboolean JNICALL
Java_sun_plugin_navig_motif_Plugin_parentAlive(JNIEnv *env, jclass clz)
{
    int ppid = getppid();

    if (ppid >= 0 && ppid < 4) {
	return JNI_FALSE;
    } else {
        return JNI_TRUE;
    }
}

/*
 * Retrieve an Environment variable.
 */
JNIEXPORT jstring JNICALL
Java_sun_plugin_navig_motif_Plugin_getenv(
		JNIEnv *env, jclass clz, jstring nameString)
{
    const char *name;  
    const char *res;
    jstring resString = 0;

    if (nameString == 0) {
	return NULL;
    }
    name = (*env)->GetStringUTFChars(env, nameString, 0);

    res = getenv(name);

    if (res != 0) {
      resString = NewPlatformString(env, res);
    }

    (*env)->ReleaseStringUTFChars(env, nameString, name);

    return resString;
}

JNIEXPORT jstring JNICALL
Java_sun_plugin_javascript_navig_JSObject_evalScript(JNIEnv *env, jobject obj,
						   jint instance, jstring s) {
    jclass clz = (*env)->FindClass(env, "sun/plugin/navig/motif/Plugin");
    jmethodID meth;
    meth = (*env)->GetStaticMethodID(env, clz, "evalString",
				     "(ILjava/lang/String;)Ljava/lang/String;");
    return (*env)->CallStaticObjectMethod(env, clz, meth, instance, s);
}

JNIEXPORT jobject JNICALL
Java_sun_plugin_navig_motif_Plugin_getPipe(JNIEnv *env, jclass clz, jint fd)
{
    jobject result;
    jclass fd_clz = (*env)->FindClass(env, "java/io/FileDescriptor");
    jfieldID field = (*env)->GetFieldID(env, fd_clz, "fd", "I");

    result = (*env)->AllocObject(env, fd_clz);

    /* On JDK1.2 the fd field gets set to fd */
    (*env)->SetIntField(env, result, field, fd);

    return result;
}

/* Dump all the thread stacks by calling JVM function */
JNIEXPORT jstring JNICALL Java_sun_plugin_JavaRunTime_dumpAllStacks (JNIEnv * env, jclass clazz)
{
    jstring str = NULL;
    char* pszConsoleOutput = NULL;
    char* tmpFile = NULL;
    struct stat buf;
    int sfildes, tfildes;
    int len;

    /* Open a tmp file to record thread info */
    tmpFile = tmpnam(NULL);
    tfildes = open(tmpFile, O_CREAT|O_RDWR, 0666);
    unlink(tmpFile);

    /* Duplicate a standard file descriptor */
    sfildes = dup(1);

    /* file descriptor 1 point to tfildes */
    dup2(tfildes, 1);

    /* Trigger thread dump */
    JVM_DumpAllStacks(env, NULL);

    /* file descriptor 1 point back to sfildes */
    dup2(sfildes, 1);

    /* Move the file pointer to the begining */
    lseek(tfildes, 0, SEEK_SET);

    /* Get the file size */
    fstat(tfildes, &buf);
    len = buf.st_size;

    /* Read the content in tmp file into output buffer */
    if (len > 0)
    {
       pszConsoleOutput = (char *) malloc(len * sizeof(char));
       read(tfildes, pszConsoleOutput, len);
       pszConsoleOutput[len] = '\0';
    }

    /* Display the thread output in Java Console */
    if (pszConsoleOutput != NULL)
       str = (*env)->NewStringUTF(env, pszConsoleOutput);

    /* Release memory */
    free(pszConsoleOutput);

    return str;
}
