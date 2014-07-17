/*
 * @(#)awt_InputMethod.c	1.61 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifdef HEADLESS
    #error This file should not be included in headless library
#endif

#include <stdio.h>
#include <stdlib.h>
#include <X11/Xlib.h>
#include <Xm/Xm.h>
#include <Xm/RowColumn.h>
#include <Xm/MwmUtil.h>
#include <Xm/MenuShell.h>

#include "awt.h"
#include "awt_p.h"

#include <sun_awt_motif_MComponentPeer.h>
#include <sun_awt_motif_X11InputMethod.h>

#define THROW_OUT_OF_MEMORY_ERROR() \
	JNU_ThrowOutOfMemoryError((JNIEnv *)JNU_GetEnv(jvm, JNI_VERSION_1_2), NULL)
#define SETARG(name, value)	XtSetArg(args[argc], name, value); argc++

#define MCOMPONENTPEER_CLASS_NAME	"sun/awt/motif/MComponentPeer"

struct X11InputMethodIDs {
  jfieldID pData;
} x11InputMethodIDs;

static void PreeditStartCallback(XIC, XPointer, XPointer);
static void PreeditDoneCallback(XIC, XPointer, XPointer);
static void PreeditDrawCallback(XIC, XPointer,
				XIMPreeditDrawCallbackStruct *);
static void PreeditCaretCallback(XIC, XPointer,
				 XIMPreeditCaretCallbackStruct *);
#ifdef __linux__
static void StatusStartCallback(XIC, XPointer, XPointer);
static void StatusDoneCallback(XIC, XPointer, XPointer);
static void StatusDrawCallback(XIC, XPointer,
			       XIMStatusDrawCallbackStruct *);
#endif

#define ROOT_WINDOW_STYLES	(XIMPreeditNothing | XIMStatusNothing)
#define NO_STYLES		(XIMPreeditNone | XIMStatusNone)

#define PreeditStartIndex	0
#define PreeditDoneIndex	1
#define PreeditDrawIndex	2
#define PreeditCaretIndex	3
#ifdef __linux__
#define StatusStartIndex	4
#define StatusDoneIndex	        5
#define StatusDrawIndex	        6
#define NCALLBACKS		7
#else
#define NCALLBACKS		4
#endif

/*
 * Callback function pointers: the order has to match the *Index
 * values above.
 */
static XIMProc callback_funcs[NCALLBACKS] = {
    (XIMProc)PreeditStartCallback,
    (XIMProc)PreeditDoneCallback,
    (XIMProc)PreeditDrawCallback,
    (XIMProc)PreeditCaretCallback,
#ifdef __linux__
    (XIMProc)StatusStartCallback,
    (XIMProc)StatusDoneCallback,
    (XIMProc)StatusDrawCallback,
#endif
};

#ifdef __linux__
#define MAX_STATUS_LEN  100
typedef struct {
    Window   w;                /*status window id        */
    Window   root;             /*the root window id      */
    Widget   parent;           /*parent shell window     */
    int      x, y;             /*parent's upperleft position */
    int      width, height;    /*parent's width, height  */
    GC       lightGC;          /*gc for light border     */
    GC       dimGC;            /*gc for dim border       */
    GC       bgGC;             /*normal painting         */
    GC       fgGC;             /*normal painting         */
    int      statusW, statusH; /*status window's w, h    */  
    int      rootW, rootH;     /*root window's w, h    */  
    int      bWidth;           /*border width            */
    char     status[MAX_STATUS_LEN]; /*status text       */
    XFontSet fontset;           /*fontset for drawing    */
    int      off_x, off_y;
    Bool     on;                /*if the status window on*/
} StatusWindow;
#endif

/*
 * X11InputMethodData keeps per X11InputMethod instance information. A pointer
 * to this data structure is kept in an X11InputMethod object (pData).
 */
typedef struct _X11InputMethodData {
    XIC		current_ic;	/* current X Input Context */
    XIC		ic_active;	/* X Input Context for active clients */
    XIC		ic_passive;	/* X Input Context for passive clients */
    XIMCallback	*callbacks;	/* callback parameters */
    jobject	peer;		/* MComponentPeer of client Window */
    jobject	x11inputmethod;	/* global ref to X11InputMethod instance */
				/* associated with the XIC */
#ifdef __linux__
    StatusWindow *statusWindow; /* our own status window  */
#else
    Widget	statusWidget;	/* IM status window widget */
#endif
} X11InputMethodData;

jobject currentX11InputMethodInstance = NULL; /* reference to the current
						 X11InputMethod instance */
Window  currentFocusWindow = 0;  /* current window that has focus for input
				       method. (the best place to put this
				       information should be
				       currentX11InputMethodInstance's pData) */
static XIM X11im = NULL;
static jobject  mcompClass = NULL;
static jobject  awteventClass = NULL;

/*
 * Converts the wchar_t string to a multi-byte string calling wcstombs(). A
 * buffer is allocated by malloc() to store the multi-byte string. NULL is
 * returned if the given wchar_t string pointer is NULL or buffer allocation is
 * failed.
 */
static char *
wcstombsdmp(wchar_t *wcs, int len)
{
    size_t n;
    char *mbs;

    if (wcs == NULL)
	return NULL;

    n = len*MB_CUR_MAX + 1;

    mbs = (char *) malloc(n * sizeof(char));
    if (mbs == NULL) {
	THROW_OUT_OF_MEMORY_ERROR();
	return NULL;
    }

    /* TODO: check return values... Handle invalid characters properly...  */
    if (wcstombs(mbs, wcs, n) == (size_t)-1)
	return NULL;

    return mbs;
}

/*
 * Find a class for the given class name and return a global reference to the
 * class.
 */
static jobject
findClass(const char *className)
{
    JNIEnv *env = (JNIEnv *)JNU_GetEnv(jvm, JNI_VERSION_1_2);
    jclass classClass;
    jobject objectClass;

    classClass = (*env)->FindClass(env, className);
    objectClass = (*env)->NewGlobalRef(env,classClass);

    if (JNU_IsNull(env, objectClass)) {
        JNU_ThrowClassNotFoundException(env, className);
    }
    return objectClass;
}

static X11InputMethodData * getX11InputMethodData(JNIEnv * env, jobject imInstance) {
    return JNU_GetLongFieldAsPtr(env, imInstance, x11InputMethodIDs.pData);
}
    
static void setX11InputMethodData(JNIEnv * env, jobject imInstance, X11InputMethodData *pX11IMData) {
    JNU_SetLongFieldFromPtr(env, imInstance, x11InputMethodIDs.pData, pX11IMData);
}

/* this function should be called within AWT_LOCK() */
static void
destroyX11InputMethodData(JNIEnv *env, X11InputMethodData *pX11IMData)
{
    if (pX11IMData->callbacks)
	free((void *)pX11IMData->callbacks);
    /*
     * Destroy XICs
     */
#ifdef __linux__
    if (pX11IMData->statusWindow != NULL){
        StatusWindow *sw = pX11IMData->statusWindow;
        XFreeGC(awt_display, sw->lightGC);
        XFreeGC(awt_display, sw->dimGC);
        XFreeGC(awt_display, sw->bgGC);
        XFreeGC(awt_display, sw->fgGC);
        XFreeFontSet(awt_display, sw->fontset);
        XDestroyWindow(awt_display, sw->w);
	free((void*)sw);
    }
#endif
    if (pX11IMData->ic_active != (XIC)0) {
#if 0
	/*
	 * This XmbResetIC call may be necessary to avoid a hang of
	 * XIM. However, at this point the client component may be
	 * gone, which causes another hang. This part disabled until
	 * the synchronization problem between the client component and
	 * the input method is resolved.
	 */
	char *committedString;

	if ((committedString = XmbResetIC(pX11IMData->ic_active)) != NULL)
	    XFree((void *)committedString);
#endif
        XUnsetICFocus(pX11IMData->ic_active);
	XDestroyIC(pX11IMData->ic_active);
	if (pX11IMData->ic_active != pX11IMData->ic_passive) {
#if 0
	    if ((committedString = XmbResetIC(pX11IMData->ic_passive)) != NULL)
		XFree((void *)committedString);
#endif
	    if (pX11IMData->ic_passive != (XIC)0) {
		XUnsetICFocus(pX11IMData->ic_passive);
		XDestroyIC(pX11IMData->ic_passive);
	    }
	    pX11IMData->ic_passive = (XIC)0;
	    pX11IMData->current_ic = (XIC)0;
	}
    }
    if (env) {
	(*env)->DeleteGlobalRef(env, pX11IMData->peer);
	(*env)->DeleteGlobalRef(env, pX11IMData->x11inputmethod);
    }
    free((void *)pX11IMData);
}

/*
 * Sets or unsets the focus to the given XIC.
 */
static void
setXICFocus(XIC ic, unsigned short req)
{
    if (ic == NULL) {
        (void)fprintf(stderr, "Couldn't find X Input Context\n");
        return;
    }
    if (req == 1)
        XSetICFocus(ic);
    else
        XUnsetICFocus(ic);
}

/*
 * Sets the focus window to the given XIC.
 */
static void
setXICWindowFocus(XIC ic, Window w)
{
    if (ic == NULL) {
        (void)fprintf(stderr, "Couldn't find X Input Context\n");
        return;
    }
    (void) XSetICValues(ic, XNFocusWindow, w, NULL);
}

/*
 * Invokes XmbLookupString() to get something from the XIM. It invokes
 * X11InputMethod.dispatchCommittedText() if XmbLookupString() returns
 * committed text.  This function is called from handleKeyEvent in canvas.c and
 * it's under the Motif event loop thread context.
 *
 * Buffer usage: The function uses the local buffer first. If it's too small to
 * get the committed text, it allocates memory for the buffer. Note that
 * XmbLookupString() sometimes produces a non-null-terminated string.
 *
 * Returns True when there is a keysym value to be handled.
 */
#define LOOKUP_BUF_SIZE 64

Bool
awt_x11inputmethod_lookupString(XKeyPressedEvent *event, KeySym *keysymp)
{
    JNIEnv *env = (JNIEnv *)JNU_GetEnv(jvm, JNI_VERSION_1_2);
    X11InputMethodData *pX11IMData;
    int buf_len = LOOKUP_BUF_SIZE;
    char mbbuf[LOOKUP_BUF_SIZE];
    char *buf;
    KeySym keysym = NoSymbol;
    Status status;
    int mblen;
    jstring javastr;
    XIC ic;
    Bool result = True;
    static Bool composing = False;

    /*
      printf("lookupString: entering...\n");
     */

    pX11IMData = getX11InputMethodData(env, currentX11InputMethodInstance);

    if (pX11IMData == NULL) {
        (void)fprintf(stderr, "Couldn't find X Input method Context\n");
#ifdef __linux__
        return False;
#else
        return result;
#endif
    }

    if ((ic = pX11IMData->current_ic) == (XIC)0){
	return result;
    }

    buf = mbbuf;
    mblen = XmbLookupString(ic, event, buf, buf_len - 1, &keysym, &status);

    /*
     * In case of overflow, a buffer is allocated and it retries
     * XmbLookupString().
     */
    if (status == XBufferOverflow) {
        buf_len = mblen + 1;
	buf = (char *)malloc(buf_len);
	if (buf == NULL) {
	    THROW_OUT_OF_MEMORY_ERROR();
	    return result;
	}
        mblen = XmbLookupString(ic, event, buf, buf_len, &keysym, &status);
    }
    buf[mblen] = 0;

    /* Get keysym without taking modifiers into account first to map
     * to AWT keyCode table.
     */
    if (((event->state & ShiftMask) ||
	(event->state & LockMask)) && 
	 keysym >= 'A' && keysym <= 'Z')
    {
        keysym = XLookupKeysym(event, 0);
    }

    switch (status) {
    case XLookupBoth:
	if (!composing) {
	    if (keysym < 128 || ((keysym & 0xff00) == 0xff00)) {
		*keysymp = keysym;
		result = False;
		break;
	    }
	}
	composing = False;
	/*FALLTHRU*/ 
    case XLookupChars:
    /*
     printf("lookupString: status=XLookupChars, type=%d, state=%x, keycode=%x, keysym=%x\n",
       event->type, event->state, event->keycode, keysym);
    */
        javastr = JNU_NewStringPlatform(env, (const char *)buf);
        if (javastr != NULL) {
            JNU_CallMethodByName(env, NULL,
				 currentX11InputMethodInstance,
				 "dispatchCommittedText",
				 "(Ljava/lang/String;J)V",
				 javastr,
                                 awt_util_nowMillisUTC_offset(event->time));
        }
	break;

    case XLookupKeySym:
    /*
     printf("lookupString: status=XLookupKeySym, type=%d, state=%x, keycode=%x, keysym=%x\n",
       event->type, event->state, event->keycode, keysym);
    */
 	if (keysym == XK_Multi_key)
	    composing = True;
 	if (! composing) {
 	    *keysymp = keysym;
 	    result = False;
 	}
	break;

    case XLookupNone:
    /*
     printf("lookupString: status=XLookupNone, type=%d, state=%x, keycode=%x, keysym=%x\n",
        event->type, event->state, event->keycode, keysym);
    */
	break;
    }

    if (buf != mbbuf)
	free((void *)buf);

    return result;
}

#ifdef __linux__
static StatusWindow *createStatusWindow(Widget parent){
    StatusWindow *statusWindow;
    XSetWindowAttributes attrib;
    unsigned long attribmask;
    Window containerWindow;
    Window status;
    Window child;
    XWindowAttributes xwa;
    XWindowAttributes xxwa;
    /*dummy variable for XCreateFontSet()*/
    char **mclr;
    int  mccr;
    char *dsr;
    Pixel bg, fg, light, dim;
    int x, y, w, h, bw, depth, off_x, off_y, xx, yy;
    XGCValues values;
    unsigned long valuemask = 0;  /*ignore XGCvalue and use defaults*/
    int screen = 0;
    int i;
    AwtGraphicsConfigDataPtr adata;
    extern int awt_numScreens;
    /*hardcode the size right now, should get the size base on font*/
    int   width=80, height=22; 
    Window rootWindow;
    Window *ignoreWindowPtr;
    unsigned int ignoreUnit;

    attrib.override_redirect = True;
    attribmask = CWOverrideRedirect;
    while (!XtIsShell(parent)){
	parent = XtParent(parent);
    }
    for (i = 0; i < awt_numScreens; i++) {
        if (ScreenOfDisplay(awt_display, i) == XtScreen(parent)) {
            screen = i;
            break;
        }
    }
    adata = getDefaultConfig(screen);
    bg    = adata->AwtColorMatch(255, 255, 255, adata);
    fg    = adata->AwtColorMatch(0, 0, 0, adata);
    light = adata->AwtColorMatch(195, 195, 195, adata);
    dim   = adata->AwtColorMatch(128, 128, 128, adata);

    XGetWindowAttributes(awt_display, XtWindow(parent), &xwa);
    bw = 2; /*xwa.border_width does not have the correct value*/

    /*compare the size difference between parent container
      and shell widget, the diff should be the border frame
      and title bar height (?)*/

    XQueryTree( awt_display,
		XtWindow(parent),
		&rootWindow, 
		&containerWindow, 
		&ignoreWindowPtr,
		&ignoreUnit);
    XGetWindowAttributes(awt_display, containerWindow, &xxwa);

    off_x = (xxwa.width - xwa.width) / 2; 
    off_y = xxwa.height - xwa.height - off_x; /*it's magic:-) */
    
    /*get the size of root window*/
    XGetWindowAttributes(awt_display, rootWindow, &xxwa);

    XTranslateCoordinates(awt_display, 
			  XtWindow(parent), xwa.root,
			  xwa.x, xwa.y,
                          &x, &y, 
			  &child);
    xx = x - off_x;
    yy = y + xwa.height - off_y;
    if (xx < 0 ){
        xx = 0;
    }
    if (xx + width > xxwa.width){
        xx = xxwa.width - width;
    } 
    if (yy + height > xxwa.height){
        yy = xxwa.height - height;
    }

    status =  XCreateWindow(awt_display, 
			    xwa.root,
			    xx, yy,
			    width, height,
			    0, 
			    xwa.depth,
			    InputOutput, 
			    adata->awt_visInfo.visual,
			    attribmask, &attrib);
    XSelectInput(awt_display, status, 
		 ExposureMask | StructureNotifyMask | EnterWindowMask |
		 LeaveWindowMask | VisibilityChangeMask);
    statusWindow = (StatusWindow*) calloc(1, sizeof(StatusWindow));
    if (statusWindow == NULL){
	THROW_OUT_OF_MEMORY_ERROR();
	return NULL;
    }
    statusWindow->w = status;
    //12-point font
    statusWindow->fontset = XCreateFontSet(awt_display, 
                                           "-*-*-medium-r-normal--*-120-*-*-*-*",
					   &mclr, &mccr, &dsr);
    statusWindow->parent = parent;
    statusWindow->on  = False;     
    statusWindow->x = x;
    statusWindow->y = y;
    statusWindow->width = xwa.width;
    statusWindow->height = xwa.height;
    statusWindow->off_x = off_x;
    statusWindow->off_y = off_y;
    statusWindow->bWidth  = bw; 
    statusWindow->statusH = height;
    statusWindow->statusW = width;
    statusWindow->rootH = xxwa.height;
    statusWindow->rootW = xxwa.width;
    statusWindow->lightGC = XCreateGC(awt_display, status, valuemask, &values);
    XSetForeground(awt_display, statusWindow->lightGC, light);
    statusWindow->dimGC = XCreateGC(awt_display, status, valuemask, &values);
    XSetForeground(awt_display, statusWindow->dimGC, dim);
    statusWindow->fgGC = XCreateGC(awt_display, status, valuemask, &values);
    XSetForeground(awt_display, statusWindow->fgGC, fg);
    statusWindow->bgGC = XCreateGC(awt_display, status, valuemask, &values);
    XSetForeground(awt_display, statusWindow->bgGC, bg);
    return statusWindow;
}

static void onoffStatusWindow(StatusWindow *statusWindow, Widget parent, Bool ON){
    XWindowAttributes xwa;
    Window child;
    int x, y;

    if (statusWindow == NULL){
        return;
    }
    if (ON == False){
        XUnmapWindow(awt_display, statusWindow->w);
        return;
    }
    while (!XtIsShell(parent)){
	parent = XtParent(parent);
    }
    if (statusWindow->parent != parent){
        statusWindow->parent = parent;
    }
    XGetWindowAttributes(awt_display, XtWindow(parent), &xwa);
    XTranslateCoordinates(awt_display, 
			  XtWindow(parent), xwa.root,
			  xwa.x, xwa.y,
			  &x, &y, 
			  &child);
    if (statusWindow->x != x 
	|| statusWindow->y != y
        || statusWindow->height != xwa.height){
        statusWindow->x = x;
	statusWindow->y = y;
	statusWindow->height = xwa.height;
	x = statusWindow->x - statusWindow->off_x;
	y = statusWindow->y + statusWindow->height - statusWindow->off_y;
	if (x < 0 ){
	    x = 0;
	}
        if (x + statusWindow->statusW > statusWindow->rootW){
	    x = statusWindow->rootW - statusWindow->statusW;
	} 
	if (y + statusWindow->statusH > statusWindow->rootH){
	    y = statusWindow->rootH - statusWindow->statusH;
	}
	XMoveWindow(awt_display, statusWindow->w, x, y);
    }
    XMapWindow(awt_display, statusWindow->w);
}

void paintStatusWindow(StatusWindow *statusWindow){
    Display *dpy = awt_display;
    Window  win  = statusWindow->w;
    GC  lightgc = statusWindow->lightGC;
    GC  dimgc = statusWindow->dimGC;
    GC  bggc = statusWindow->bgGC;
    GC  fggc = statusWindow->fgGC;

    int width = statusWindow->statusW;
    int height = statusWindow->statusH;
    int bwidth = statusWindow->bWidth;
    XFillRectangle(dpy, win, bggc, 0, 0, width, height);
    /* draw border */
    XDrawLine(dpy, win, fggc, 0, 0, width, 0);
    XDrawLine(dpy, win, fggc, 0, height-1, width-1, height-1);
    XDrawLine(dpy, win, fggc, 0, 0, 0, height-1);
    XDrawLine(dpy, win, fggc, width-1, 0, width-1, height-1);

    XDrawLine(dpy, win, lightgc, 1, 1, width-bwidth, 1);
    XDrawLine(dpy, win, lightgc, 1, 1, 1, height-2);
    XDrawLine(dpy, win, lightgc, 1, height-2, width-bwidth, height-2);
    XDrawLine(dpy, win, lightgc, width-bwidth-1, 1, width-bwidth-1, height-2);

    XDrawLine(dpy, win, dimgc, 2, 2, 2, height-3);
    XDrawLine(dpy, win, dimgc, 2, height-3, width-bwidth-1, height-3);
    XDrawLine(dpy, win, dimgc, 2, 2, width-bwidth-2, 2);
    XDrawLine(dpy, win, dimgc, width-bwidth, 2, width-bwidth, height-3);
    if (statusWindow->fontset){
        XmbDrawString(dpy, win, statusWindow->fontset, fggc, 
		      bwidth + 2, height - bwidth - 4,
		      statusWindow->status,
		      strlen(statusWindow->status));
    }
    else{
        /*too bad we failed to create a fontset for this locale*/
        XDrawString(dpy, win, fggc, bwidth + 2, height - bwidth - 4,
		    "[InputMethod ON]", strlen("[InputMethod ON]"));
    }
}

void statusWindowEventHandler(XEvent event){
    JNIEnv *env = (JNIEnv *)JNU_GetEnv(jvm, JNI_VERSION_1_2);
    X11InputMethodData *pX11IMData;
    StatusWindow *statusWindow;
    if (NULL == currentX11InputMethodInstance
        || NULL == (pX11IMData = getX11InputMethodData(env, currentX11InputMethodInstance))
	|| NULL == (statusWindow = pX11IMData->statusWindow)
        || statusWindow->w != event.xany.window){
	return;
    }

    switch (event.type){
    case Expose:
        paintStatusWindow(statusWindow);
        break;
    case MapNotify:
    case ConfigureNotify:
        {
	  /*need to reset the stackMode...*/
            XWindowChanges xwc;
	    int value_make = CWStackMode; 
	    xwc.stack_mode = TopIf;
	    XConfigureWindow(awt_display, statusWindow->w, value_make, &xwc); 
        }
        break;
	/*
    case UnmapNotify:
    case VisibilityNotify:
        break;
	*/
    default:
        break;
  }
}

void adjustStatusWindow(Widget shell){
    JNIEnv *env = (JNIEnv *)JNU_GetEnv(jvm, JNI_VERSION_1_2);
    X11InputMethodData *pX11IMData;
    StatusWindow *statusWindow;
    if (NULL == currentX11InputMethodInstance
        || NULL == (pX11IMData = getX11InputMethodData(env, currentX11InputMethodInstance))
	|| NULL == (statusWindow = pX11IMData->statusWindow)){
	return;
    }
    if (statusWindow->parent == shell){
        XWindowAttributes xwa;
        int x, y;
        Window child;
	XGetWindowAttributes(awt_display, XtWindow(shell), &xwa);
	XTranslateCoordinates(awt_display, 
			      XtWindow(shell), xwa.root,
			      xwa.x, xwa.y,
			      &x, &y, 
			      &child);
	if (statusWindow->x != x 
	    || statusWindow->y != y
	    || statusWindow->height != xwa.height){
	  statusWindow->x = x;
	  statusWindow->y = y;
	  statusWindow->height = xwa.height;

          x = statusWindow->x - statusWindow->off_x;
	  y = statusWindow->y + statusWindow->height - statusWindow->off_y;
          if (x < 0 ){
              x = 0;
	  }
	  if (x + statusWindow->statusW > statusWindow->rootW){
	      x = statusWindow->rootW - statusWindow->statusW;
	  } 
	  if (y + statusWindow->statusH > statusWindow->rootH){
              y = statusWindow->rootH - statusWindow->statusH;
	  }
	  XMoveWindow(awt_display, statusWindow->w, x, y);
	} 
    }
}
#endif  /*__linux__*/
/*
 * Creates two XICs, one for active clients and the other for passive
 * clients. All information on those XICs are stored in the
 * X11InputMethodData given by the pX11IMData parameter.
 * 
 * For active clients: Try to use preedit callback to support
 * on-the-spot. If tc is not null, the XIC to be created will
 * share the Status Area with Motif widgets (TextComponents). If the
 * preferable styles can't be used, fallback to root-window styles. If
 * root-window styles failed, fallback to None styles.
 * 
 * For passive clients: Try to use root-window styles. If failed,
 * fallback to None styles.
 */
static Bool
createXIC(Widget w, X11InputMethodData *pX11IMData,
	  jobject tc, jobject peer)
{
    XIC active_ic, passive_ic;
    XVaNestedList preedit = NULL;
    XVaNestedList status = NULL;
    XIMStyle on_the_spot_styles = XIMPreeditCallbacks,
	     active_styles = 0,
	     passive_styles = 0,
	     no_styles = 0;
    XIMCallback *callbacks;
    unsigned short i;
    XIMStyles *im_styles;
    char *ret;

    if (X11im == NULL) {
        (void)fprintf(stderr, "Couldn't find X Input method\n");
        return False;
    }

    /*
     * If the parent window has one or more TextComponents, the status
     * area of Motif will be shared with the created XIC. Otherwise,
     * root-window style status is used.
     */
#ifdef __linux__
    /*kinput does not support XIMPreeditCallbacks and XIMStatusArea
      at the same time, so use StatusCallback to draw the status
      ourself
    */
    on_the_spot_styles |= XIMStatusCallbacks;
#else
    /*
     * If the parent window has one or more TextComponents, the status
     * area of Motif will be shared with the created XIC. Otherwise,
     * root-window style status is used.
     */
    if (tc != NULL){
        XVaNestedList status = NULL;        
        status = awt_motif_getXICStatusAreaList(w, tc);
        if (status != NULL){
            on_the_spot_styles |=  XIMStatusArea;
            XFree(status);
        }
        else
            on_the_spot_styles |= XIMStatusNothing;
    }
    else
        on_the_spot_styles |= XIMStatusNothing;

#endif
    ret = XGetIMValues(X11im, XNQueryInputStyle, &im_styles, NULL);

    if (ret != NULL) {
        jio_fprintf(stderr,"XGetIMValues: %s\n",ret);
        return FALSE ;
    }


    for (i = 0; i < im_styles->count_styles; i++) {
        active_styles |= im_styles->supported_styles[i] & on_the_spot_styles;
	passive_styles |= im_styles->supported_styles[i] & ROOT_WINDOW_STYLES;
	no_styles |= im_styles->supported_styles[i] & NO_STYLES;
    }

    XFree(im_styles);

    if (active_styles != on_the_spot_styles) {
	if (passive_styles == ROOT_WINDOW_STYLES)
	    active_styles = passive_styles;
	else {
	    if (no_styles == NO_STYLES)
		active_styles = passive_styles = NO_STYLES;
	    else
		active_styles = passive_styles = 0;
	}
    } else {
	if (passive_styles != ROOT_WINDOW_STYLES) {
	    if (no_styles == NO_STYLES)
		active_styles = passive_styles = NO_STYLES;
	    else
		active_styles = passive_styles = 0;
	}
    }

    if (active_styles == on_the_spot_styles) {
	callbacks = (XIMCallback *)malloc(sizeof(XIMCallback) * NCALLBACKS);
	if (callbacks == (XIMCallback *)NULL)
	    return False;
	pX11IMData->callbacks = callbacks;

	for (i = 0; i < NCALLBACKS; i++, callbacks++) {
	    callbacks->client_data = (XPointer) pX11IMData->x11inputmethod;
	    callbacks->callback = callback_funcs[i];
	}

	callbacks = pX11IMData->callbacks;
	preedit = (XVaNestedList)XVaCreateNestedList(0,
			XNPreeditStartCallback, &callbacks[PreeditStartIndex],
			XNPreeditDoneCallback,  &callbacks[PreeditDoneIndex],
			XNPreeditDrawCallback,  &callbacks[PreeditDrawIndex],
			XNPreeditCaretCallback, &callbacks[PreeditCaretIndex],
			NULL);
	if (preedit == (XVaNestedList)NULL)
	    goto err;
#ifdef __linux__
        /*always try XIMStatusCallbacks for active client...*/
	{
            status = (XVaNestedList)XVaCreateNestedList(0,
			XNStatusStartCallback, &callbacks[StatusStartIndex],
			XNStatusDoneCallback,  &callbacks[StatusDoneIndex],
			XNStatusDrawCallback, &callbacks[StatusDrawIndex],
			NULL);

	    if (status == NULL)
		goto err;
	    pX11IMData->statusWindow = createStatusWindow(w);
	    pX11IMData->ic_active = XCreateIC(X11im,
					      XNClientWindow, XtWindow(w),
					      XNFocusWindow, XtWindow(w), 
					      XNInputStyle, active_styles,
					      XNPreeditAttributes, preedit,
					      XNStatusAttributes, status,
					      NULL);
	    XFree((void *)status);
	    XFree((void *)preedit);
	}
#else
	if (on_the_spot_styles & XIMStatusArea) {
	    Widget parent;
	    status = awt_motif_getXICStatusAreaList(w, tc);
	    if (status == NULL)
		goto err;
	    pX11IMData->statusWidget = awt_util_getXICStatusAreaWindow(w);
	    pX11IMData->ic_active = XCreateIC(X11im,
					      XNClientWindow, XtWindow(pX11IMData->statusWidget),
					      XNFocusWindow, XtWindow(w), 
					      XNInputStyle, active_styles,
					      XNPreeditAttributes, preedit,
					      XNStatusAttributes, status,
					      NULL);
	    XFree((void *)status);
	} else {
	    pX11IMData->ic_active = XCreateIC(X11im,
					      XNClientWindow, XtWindow(w),
					      XNFocusWindow, XtWindow(w), 
					      XNInputStyle, active_styles,
					      XNPreeditAttributes, preedit,
					      NULL);
	}
	XFree((void *)preedit);
#endif
	pX11IMData->ic_passive = XCreateIC(X11im,
					   XNClientWindow, XtWindow(w),
					   XNFocusWindow, XtWindow(w), 
					   XNInputStyle, passive_styles,
					   NULL);

    } else {
	pX11IMData->ic_active = XCreateIC(X11im,
					  XNClientWindow, XtWindow(w),
					  XNFocusWindow, XtWindow(w),
					  XNInputStyle, active_styles,
					  NULL);
	pX11IMData->ic_passive = pX11IMData->ic_active;
    }

    if (pX11IMData->ic_active == (XIC)0
	|| pX11IMData->ic_passive == (XIC)0) {
	return False;
    }
    return True;

 err:
    if (preedit)
	XFree((void *)preedit);
    THROW_OUT_OF_MEMORY_ERROR();
    return False;
}

static void
PreeditStartCallback(XIC ic, XPointer client_data, XPointer call_data)
{
    /*ARGSUSED*/
    /* printf("Native: PreeditCaretCallback\n"); */
}

static void
PreeditDoneCallback(XIC ic, XPointer client_data, XPointer call_data)
{
    /*ARGSUSED*/
    /* printf("Native: StatusStartCallback\n"); */
}

/*
 * Translate the preedit draw callback items to Java values and invoke
 * X11InputMethod.dispatchComposedText().
 *
 * client_data: X11InputMethod object
 */
static void
PreeditDrawCallback(XIC ic, XPointer client_data,
		    XIMPreeditDrawCallbackStruct *pre_draw)
{
    JNIEnv *env = (JNIEnv *)JNU_GetEnv(jvm, JNI_VERSION_1_2);
    X11InputMethodData *pX11IMData;
    jmethodID x11imMethodID;

    XIMText *text;
    jstring javastr = NULL;
    jintArray style = NULL;

    /*    printf("Native: PreeditDrawCallback() \n"); */
    
    pX11IMData = getX11InputMethodData(env, (jobject)client_data);
    if (pX11IMData == NULL) {
        return;
    }

    if (pre_draw == NULL)
        return;

    if ((text = pre_draw->text) != NULL) {
	if (text->string.multi_byte != NULL) {
	    if (pre_draw->text->encoding_is_wchar == False) {
		javastr = JNU_NewStringPlatform(env, (const char *)text->string.multi_byte);
	    } else {
		char *mbstr = wcstombsdmp(text->string.wide_char, text->length);
		if (mbstr == NULL)
		    return;
		javastr = JNU_NewStringPlatform(env, (const char *)mbstr);
		free(mbstr);
	    }
	}
	if (text->feedback != NULL) {
            int cnt;
            jint *tmpstyle;

            style = (*env)->NewIntArray(env, text->length);
            if (JNU_IsNull(env, style)) {
                THROW_OUT_OF_MEMORY_ERROR();
                return;
            }

	    if (sizeof(XIMFeedback) == sizeof(jint)) {
		/*
		 * Optimization to avoid copying the array
		 */
		(*env)->SetIntArrayRegion(env, style, 0,
					  text->length, (jint *)text->feedback);
	    } else {
		tmpstyle  = (jint *)malloc(sizeof(jint)*(text->length));
		if (tmpstyle == (jint *) NULL) {
		    THROW_OUT_OF_MEMORY_ERROR();
		    return;
		}
		for (cnt = 0; cnt < (int)text->length; cnt++)
			tmpstyle[cnt] = text->feedback[cnt];
		(*env)->SetIntArrayRegion(env, style, 0,
					  text->length, (jint *)tmpstyle);
	    }
        }
    }
    JNU_CallMethodByName(env, NULL, pX11IMData->x11inputmethod,
			 "dispatchComposedText",
			 "(Ljava/lang/String;[IIIIJ)V",
			 javastr,
			 style,
			 (jint)pre_draw->chg_first,
			 (jint)pre_draw->chg_length,
			 (jint)pre_draw->caret,
                         awt_util_nowMillisUTC());
}

static void
PreeditCaretCallback(XIC ic, XPointer client_data,
		     XIMPreeditCaretCallbackStruct *pre_caret)
{
    /*ARGSUSED*/
    /* printf("Native: PreeditCaretCallback\n"); */

}

#ifdef __linux__
static void
StatusStartCallback(XIC ic, XPointer client_data, XPointer call_data)
{
    /*ARGSUSED*/
    /*printf("StatusStartCallback:\n");  */

}

static void
StatusDoneCallback(XIC ic, XPointer client_data, XPointer call_data)
{
    /*ARGSUSED*/
    /*printf("StatusDoneCallback:\n"); */

}

static void
StatusDrawCallback(XIC ic, XPointer client_data,
		     XIMStatusDrawCallbackStruct *status_draw)
{
    /*ARGSUSED*/
    /*printf("StatusDrawCallback:\n"); */
    JNIEnv *env = (JNIEnv *)JNU_GetEnv(jvm, JNI_VERSION_1_2);
    X11InputMethodData *pX11IMData;
    StatusWindow *statusWindow;
    if (NULL == (pX11IMData = getX11InputMethodData(env, (jobject)client_data))
	|| NULL == (statusWindow = pX11IMData->statusWindow)){
	return;
    }

    if (status_draw->type == XIMTextType){
        XIMText *text = (status_draw->data).text;
        if (text != NULL){
	  if (text->string.multi_byte != NULL){
              strcpy(statusWindow->status, text->string.multi_byte);
	  }
	  else{
	      char *mbstr = wcstombsdmp(text->string.wide_char, text->length);
              strcpy(statusWindow->status, mbstr);
	  }
	  statusWindow->on = True;
	  onoffStatusWindow(statusWindow, statusWindow->parent, True); 
	  paintStatusWindow(statusWindow);
	}
	else { 
	    statusWindow->on = False;
            /*just turnoff the status window
	    paintStatusWindow(statusWindow);
	    */
	    onoffStatusWindow(statusWindow, NULL, False); 
	}
    }
}
#endif /*__linux__*/

/*
 * Class:     java_sun_awt_motif_X11InputMethod
 * Method:    initIDs
 * Signature: ()V
 */

/* This function gets called from the static initializer for
   X11InputMethod.java
   to initialize the fieldIDs for fields that may be accessed from C */
JNIEXPORT void JNICALL
Java_sun_awt_motif_X11InputMethod_initIDs(JNIEnv *env, jclass cls)
{
    x11InputMethodIDs.pData = (*env)->GetFieldID(env, cls, "pData", "J");
}


JNIEXPORT jboolean JNICALL
Java_sun_awt_motif_X11InputMethod_openXIM(JNIEnv *env,
					  jobject this)
{
    extern int xerror_handler();

    AWT_LOCK();

    X11im = XOpenIM(awt_display, NULL, NULL, NULL);

    /* Workaround for Solaris 2.6 bug 4097754. We're affected by this problem 
     * because Motif also calls XOpenIM for us. Re-registering the error handler
     * that MToolkit has registered already after calling XOpenIM avoids the
     * problem.
     */
    XSetErrorHandler(xerror_handler);

    AWT_UNLOCK();

    return (X11im != NULL) ? JNI_TRUE : JNI_FALSE;
}

JNIEXPORT jboolean JNICALL
Java_sun_awt_motif_X11InputMethod_createXICNative(JNIEnv *env,
						  jobject this,
						  jobject comp,
						  jobject tc)
{
    struct ComponentData *cdata;
    X11InputMethodData *pX11IMData;
    jobject globalRef;
    jfieldID mcompPDataID;
    XIC ic;

    AWT_LOCK();

    if (JNU_IsNull(env, comp)) {
        JNU_ThrowNullPointerException(env, "NullPointerException");
        AWT_UNLOCK();
        return JNI_FALSE;
    }

    pX11IMData = (X11InputMethodData *) calloc(1, sizeof(X11InputMethodData));
    if (pX11IMData == NULL) {
	THROW_OUT_OF_MEMORY_ERROR();
	AWT_UNLOCK();
        return JNI_FALSE;
    }

    if (mcompClass == NULL)
	mcompClass = findClass(MCOMPONENTPEER_CLASS_NAME);

    mcompPDataID = (*env)->GetFieldID(env, mcompClass, "pData", "J");
    cdata = (struct ComponentData *) JNU_GetLongFieldAsPtr(env,comp,mcompPDataID);

    if (cdata == 0) {
	free((void *)pX11IMData);
        JNU_ThrowNullPointerException(env, "createXIC");
        AWT_UNLOCK();
        return JNI_FALSE;
    }

    globalRef = (*env)->NewGlobalRef(env, this);
    pX11IMData->peer = (*env)->NewGlobalRef(env, comp);
    pX11IMData->x11inputmethod = globalRef;
#ifdef __linux__
    pX11IMData->statusWindow = NULL;
#else
    pX11IMData->statusWidget = (Widget) NULL;
#endif

    if (createXIC(cdata->widget, pX11IMData, tc, comp)
	== False) {
	destroyX11InputMethodData((JNIEnv *) NULL, pX11IMData);
	pX11IMData = (X11InputMethodData *) NULL;
    }

    setX11InputMethodData(env, this, pX11IMData);
    
    AWT_UNLOCK();
    return (pX11IMData != NULL);
}

JNIEXPORT void JNICALL
Java_sun_awt_motif_X11InputMethod_reconfigureXICNative(JNIEnv *env,
						       jobject this,
						       jobject comp,
						       jobject tc)
{
    X11InputMethodData *pX11IMData;

    AWT_LOCK();

    pX11IMData = getX11InputMethodData(env, this);
    if (pX11IMData == NULL) {
        AWT_UNLOCK();
        return;
    }

    if (pX11IMData->current_ic == (XIC)0) {
	destroyX11InputMethodData(env, pX11IMData);
	pX11IMData = (X11InputMethodData *)NULL;
    } else {
	Bool active;
	struct ComponentData *cdata;
	jfieldID mcompPDataID;

	active = pX11IMData->current_ic == pX11IMData->ic_active;
	mcompPDataID = (*env)->GetFieldID(env, mcompClass, "pData", "J");
	cdata = (struct ComponentData *) JNU_GetLongFieldAsPtr(env,comp,mcompPDataID);
	if (cdata == 0) {
	    JNU_ThrowNullPointerException(env, "reconfigureXICNative");
	    destroyX11InputMethodData(env, pX11IMData);
	    pX11IMData = (X11InputMethodData *)NULL;
	}
	XDestroyIC(pX11IMData->ic_active);
	if (pX11IMData->ic_active != pX11IMData->ic_passive)
	    XDestroyIC(pX11IMData->ic_passive);
	pX11IMData->current_ic = (XIC)0;
	pX11IMData->ic_active = (XIC)0;
	pX11IMData->ic_passive = (XIC)0;
	if (createXIC(cdata->widget, pX11IMData, tc, comp)) {
	    pX11IMData->current_ic = active ?
			pX11IMData->ic_active : pX11IMData->ic_passive;
	    /*
	     * On Solaris2.6, setXICWindowFocus() has to be invoked
	     * before setting focus.
	     */
	    setXICWindowFocus(pX11IMData->current_ic, XtWindow(cdata->widget));
	    setXICFocus(pX11IMData->current_ic, True);
	} else {
	    destroyX11InputMethodData((JNIEnv *) NULL, pX11IMData);
	    pX11IMData = (X11InputMethodData *)NULL;
	}
    }

    setX11InputMethodData(env, this, pX11IMData);

    AWT_UNLOCK();
}

JNIEXPORT void JNICALL
Java_sun_awt_motif_X11InputMethod_setXICFocus(JNIEnv *env,
					      jobject this,
					      jobject comp,
					      jboolean req,
					      jboolean active)
{
    X11InputMethodData *pX11IMData;
    jfieldID mcompPDataID;
    struct ComponentData *cdata;

    AWT_LOCK();
    pX11IMData = getX11InputMethodData(env, this);
    if (pX11IMData == NULL) {
        AWT_UNLOCK();
        return;
    }

    if (req) {
        struct ComponentData *cdata;
        jfieldID mcompPDataID;

	if (JNU_IsNull(env, comp)) {
	    AWT_UNLOCK();
	    return;
	}
	if (mcompClass == NULL)
	    mcompClass = findClass(MCOMPONENTPEER_CLASS_NAME);
        mcompPDataID = (*env)->GetFieldID(env, mcompClass, "pData", "J");
        cdata = (struct ComponentData *)JNU_GetLongFieldAsPtr(env, comp,
							      mcompPDataID);
        if (cdata == 0) {
            JNU_ThrowNullPointerException(env, "setXICFocus pData");
            AWT_UNLOCK();
            return;
        }

	pX11IMData->current_ic = active ?
			pX11IMData->ic_active : pX11IMData->ic_passive;
	/*
	 * On Solaris2.6, setXICWindowFocus() has to be invoked
	 * before setting focus.
	 */
        setXICWindowFocus(pX11IMData->current_ic, XtWindow(cdata->widget));
        setXICFocus(pX11IMData->current_ic, req);
        currentX11InputMethodInstance = pX11IMData->x11inputmethod;
        currentFocusWindow =  XtWindow(cdata->widget);
#ifdef __linux__
	if (active && pX11IMData->statusWindow && pX11IMData->statusWindow->on)
  	    onoffStatusWindow(pX11IMData->statusWindow, cdata->widget, True);
#endif
    } else {
	currentX11InputMethodInstance = NULL;
        currentFocusWindow = 0;
#ifdef __linux__
        onoffStatusWindow(pX11IMData->statusWindow, NULL, False);
        if (pX11IMData->current_ic != NULL)
#endif
        setXICFocus(pX11IMData->current_ic, req);

	pX11IMData->current_ic = (XIC)0;
    }

    XSync(awt_display, False);
    AWT_UNLOCK();
}

JNIEXPORT void JNICALL
Java_sun_awt_motif_X11InputMethod_turnoffStatusWindow(JNIEnv *env,
						      jobject this)
{
#ifdef __linux__
    X11InputMethodData *pX11IMData;
    StatusWindow *statusWindow;

    AWT_LOCK();

    if (NULL == currentX11InputMethodInstance
        || NULL == (pX11IMData = getX11InputMethodData(env, currentX11InputMethodInstance))
	|| NULL == (statusWindow = pX11IMData->statusWindow)){
	AWT_UNLOCK();
	return;
    }
    onoffStatusWindow(statusWindow, NULL, False);
    
    AWT_UNLOCK();
#endif
}

JNIEXPORT void JNICALL
Java_sun_awt_motif_X11InputMethod_disposeXIC(JNIEnv *env,
					     jobject this)
{
    X11InputMethodData *pX11IMData;

    AWT_LOCK();
    pX11IMData = getX11InputMethodData(env, this);
    if (pX11IMData == NULL) {
        AWT_UNLOCK();
        return;
    }
    
    setX11InputMethodData(env, this, NULL);

    if (pX11IMData->x11inputmethod == currentX11InputMethodInstance) {
	currentX11InputMethodInstance = NULL;
	currentFocusWindow = 0;
    }
    destroyX11InputMethodData(env, pX11IMData);
    AWT_UNLOCK();
}

JNIEXPORT void JNICALL
Java_sun_awt_motif_X11InputMethod_closeXIM(JNIEnv *env,
					   jobject this)
{
    AWT_LOCK();

    if (X11im != NULL) {
        XCloseIM(X11im);
        X11im = NULL;
    }
    AWT_UNLOCK();
}

JNIEXPORT jstring JNICALL
Java_sun_awt_motif_X11InputMethod_resetXIC(JNIEnv *env,
					   jobject this)
{
    X11InputMethodData *pX11IMData;
    char *xText;
    jstring jText = (jstring)0;

    AWT_LOCK();
    pX11IMData = getX11InputMethodData(env, this);
    if (pX11IMData == NULL) {
        AWT_UNLOCK();
        return jText;
    }

    if (pX11IMData->current_ic)
	xText = XmbResetIC(pX11IMData->current_ic);
    else {
	/*
	 * If there is no reference to the current XIC, try to reset both XICs.
	 */
	xText = XmbResetIC(pX11IMData->ic_active);
        /*it may also means that the real client component does 
          not have focus -- has been deactivated... its xic should 
          not have the focus, bug#4284651 showes reset XIC for htt 
          may bring the focus back, so de-focus it again.
	*/
        setXICFocus(pX11IMData->ic_active, FALSE);
	if (pX11IMData->ic_active != pX11IMData->ic_passive) {
	    char *tmpText = XmbResetIC(pX11IMData->ic_passive);
            setXICFocus(pX11IMData->ic_passive, FALSE);
	    if (xText == (char *)NULL && tmpText)
		xText = tmpText;
	}

    }
    if (xText != NULL) {
	jText = JNU_NewStringPlatform(env, (const char *)xText);
	XFree((void *)xText);
    }

    AWT_UNLOCK();
    return jText;
}

JNIEXPORT void JNICALL
Java_sun_awt_motif_X11InputMethod_configureStatusAreaNative(JNIEnv *env,
							    jobject this,
							    jobject tc)
{
    X11InputMethodData *pX11IMData;
    XVaNestedList status;

#ifdef __linux__
      /*do nothing for linux? */
#else
    AWT_LOCK();
    pX11IMData = getX11InputMethodData(env, this);

    if ((pX11IMData == NULL) || (pX11IMData->ic_active == (XIC)0)) {
        AWT_UNLOCK();
	return;
    }

    if (pX11IMData->statusWidget) {
        status = awt_motif_getXICStatusAreaList(pX11IMData->statusWidget, tc);
        if (status != (XVaNestedList)NULL) {
            XSetICValues(pX11IMData->ic_active,
                         XNStatusAttributes, status,
                         NULL);
            XFree((void *)status);
        }
    }
    AWT_UNLOCK();
#endif
}

/*
 * Class:     sun_awt_motif_X11InputMethod
 * Method:    setCompositionEnabledNative
 * Signature: (ZJ)V
 */
JNIEXPORT void JNICALL Java_sun_awt_motif_X11InputMethod_setCompositionEnabledNative
  (JNIEnv *env, jobject this, jboolean enable)
{
    X11InputMethodData *pX11IMData;
    char * ret;

    AWT_LOCK();
    pX11IMData = getX11InputMethodData(env, this);

    if ((pX11IMData == NULL) || (pX11IMData->current_ic == NULL)) {
        AWT_UNLOCK();
        return;
    }

    ret = XSetICValues(pX11IMData->current_ic, XNPreeditState, 
                       (enable ? XIMPreeditEnable : XIMPreeditDisable), NULL);
    AWT_UNLOCK();
    
    if ((ret != 0) && (strcmp(ret, XNPreeditState) == 0)) {
        JNU_ThrowByName(env, "java/lang/UnsupportedOperationException", "");
    }
}

/*
 * Class:     sun_awt_motif_X11InputMethod
 * Method:    isCompositionEnabledNative
 * Signature: (J)Z
 */
JNIEXPORT jboolean JNICALL Java_sun_awt_motif_X11InputMethod_isCompositionEnabledNative
  (JNIEnv *env, jobject this)
{
    X11InputMethodData *pX11IMData;
    char * ret;
    XIMPreeditState state;

    AWT_LOCK();
    pX11IMData = getX11InputMethodData(env, this);

    if ((pX11IMData == NULL) || (pX11IMData->current_ic == NULL)) {
        AWT_UNLOCK();
        return JNI_FALSE;
    }

    ret = XGetICValues(pX11IMData->current_ic, XNPreeditState, &state, NULL);
    AWT_UNLOCK();
    
    if ((ret != 0) && (strcmp(ret, XNPreeditState) == 0)) {
        JNU_ThrowByName(env, "java/lang/UnsupportedOperationException", "");
        return JNI_FALSE;
    }

    return (jboolean)(state == XIMPreeditEnable);
}
