/*
 * @(#)awt_p.h	1.4 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * Motif-specific data structures for AWT Java objects.
 *
 */
#ifndef _AWT_P_H_
#define _AWT_P_H_

/* turn on to do event filtering */
#define NEW_EVENT_MODEL
/* turn on to only filter keyboard events */
#define KEYBOARD_ONLY_EVENTS

#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <X11/Intrinsic.h>
#include <X11/IntrinsicP.h>
#include <X11/Shell.h>
#include <X11/StringDefs.h>
#include <X11/Xatom.h>
#include <X11/keysym.h>
#include <X11/keysymdef.h>
#include <Xm/CascadeB.h>
#include <Xm/DrawingA.h>
#include <Xm/FileSB.h>
#include <Xm/BulletinB.h>
#include <Xm/Form.h>
#include <Xm/Frame.h>
#include <Xm/Label.h>
#include <Xm/PushB.h>
#include <Xm/PushBG.h>
#include <Xm/RowColumn.h>
#include <Xm/ScrollBar.h>
#include <Xm/ScrolledW.h>
#include <Xm/SelectioB.h>
#include <Xm/SeparatoG.h>
#include <Xm/ToggleB.h>
#include <Xm/TextF.h>
#include <Xm/Text.h>
#include <Xm/List.h>
#include <Xm/Xm.h>
#include <Xm/MainW.h>
#include "awt.h"
#include "awt_util.h"
#include "color.h"
#include "colordata.h"

#ifndef min
#define min(a,b) ((a) <= (b)? (a):(b))
#endif
#ifndef max
#define max(a,b) ((a) >= (b)? (a):(b))
#endif

extern Pixel awt_pixel_by_name(Display *dpy, char *color, char *defaultColor);

typedef struct DropSiteInfo* DropSitePtr;

struct WidgetInfo {
    Widget	       widget;
    Widget	       origin;
    void*  	       peer;
    long   	       event_mask;
    struct WidgetInfo* next;
};

#define RepaintPending_NONE 	0
#define RepaintPending_REPAINT  (1 << 0)
#define RepaintPending_EXPOSE   (1 << 1)
#define LOOKUPSIZE 32

typedef struct _DamageRect {
    int x1;
    int y1;
    int x2;
    int y2;
} DamageRect;

typedef struct _AwtData  {
    int         awt_depth;
    Colormap    awt_cmap;
    XVisualInfo awt_visInfo;
    int         awt_num_colors;
    awtImageData *awtImage;
    int         (*AwtColorMatch)(int, int, int, struct _AwtData *);
    XImage	*savedXImage;	 /* Used in awt_Graphics.c */
    long	bytesPerChannel; /* Used in awt_Graphics.c */
    long        pixelStride;     /* Used in awt_Graphics.c */
    long	primitiveType;   /* Used in awt_Graphics.c */
    int         byteSwapped;     /* Used in awt_Graphics.c */
    ColorData   *color_data;
} AwtData;

typedef AwtData* AwtDataPtr;

struct ComponentData {
    Widget	widget;
    int 	repaintPending;
    DamageRect  repaintRect;
    DamageRect  exposeRect;
    Cursor	cursor;
    Boolean     customCursor;
    DropSitePtr dsi;
    AwtDataPtr  awtData;
    /*
     * Fix for BugTraq ID 4186663 - Pural PopupMenus appear at the same time.
     * This field keeps the pointer to the currently showing popup.
     */
    Widget      activePopup;    
};

struct MessageDialogData {
    struct ComponentData	comp;
    long			isModal;
};

struct CanvasData {
    struct ComponentData	comp;
    Widget			shell;
    int				flags;
};

struct GraphicsData {
    Drawable	                drawable;
    GC		                gc;
    XRectangle	                cliprect;
    int				originX;
    int				originY;
    Pixel	                fgpixel;
    Pixel	                xorpixel;
    char	                clipset;
    char	                xormode;
    AwtDataPtr			awtData;
};

struct MenuItemData {
    struct ComponentData	comp;
    int				index;
};

struct MenuData {
    struct ComponentData	comp;
    struct MenuItemData		itemData;
};


#define W_GRAVITY_INITIALIZED 1
#define W_IS_EMBEDDED 2

/* Window Manager (WM). See runningWM() in awt_util.c, awt_Frame.c      */
#define WM_YET_TO_BE_DETERMINED		0
#define CDE_WM				1
#define MOTIF_WM			2
#define OPENLOOK_WM			3
#define KDE_WM                          50
#define ENLIGHTENMENT_WM                51
#define NO_WM				98
#define OTHER_WM			99

struct FrameData {
    struct CanvasData	winData;
    long		isModal;
    long		mappedOnce;
    Widget		mainWindow;
    Widget		contentWindow;
    Widget		menuBar;
    Widget		warningWindow;
    long		top;		/* these four are the insets...	*/
    long		bottom;
    long		left;
    long		right;
    long		mbHeight;	/* height of the menubar window	*/
    long		wwHeight;	/* height of the warning window	*/
    Boolean		reparented;
    Boolean		shellResized;	/* frame shell has been resized	*/
    Boolean		canvasResized;	/* frame inner canvas resized	*/
    Boolean		menuBarReset;	/* frame menu bar added/removed	*/
    Boolean		isResizable;	/* is this window resizable ?	*/
    Boolean		isIconic;	/* is this window iconified ?	*/
    Boolean		isFixedSizeSet;	/* is fixed size already set ?	*/
    Boolean		isShowing;	/* is this window now showing ?	*/
    Boolean		hasTextComponentNative;
    Boolean		need_reshape;
    Boolean             callbacksAdded; /* needed for fix for 4078176   */
    Pixmap              iconPixmap;     /* Pixmap to hold icon image    */
    int                 iconWidth;
    int                 iconHeight;
    long                imHeight;       /* imStatusBar's height         */ 
    Boolean             imRemove;       /* ImStatusBar is being removed */

    Boolean             fixInsets;      /* [jk] REMINDER: remove if possible */
};

struct ListData {
    struct ComponentData comp;
    Widget		 list;
};

struct TextAreaData {
    struct ComponentData comp;
    Widget 		 txt;
};

struct TextFieldData {
    struct ComponentData comp;
    int                  echoContextID;
    Boolean              echoContextIDInit;
};

struct FileDialogData {
    struct ComponentData comp;
    char	*file;
};

typedef struct awtFontList {
    char *xlfd;
    int index_length;
    int load;
    char *charset_name;
    XFontStruct *xfont;
} awtFontList;

struct FontData {
    int charset_num;
    awtFontList *flist;
    XFontSet xfs; 	/* for TextField & TextArea */
    XFontStruct *xfont;	/* Latin1 font */
};

struct ChoiceData {
    struct ComponentData comp;
    Widget		 menu;
    Widget		 *items;
    int			 maxitems;
    int			 n_items;
};

struct ImageData {
    Pixmap	xpixmap;
    Pixmap	xmask;
};

extern int awt_init_gc(JNIEnv *env,Display *display, struct GraphicsData *gdata,jobject this);

extern struct FontData *awtJNI_GetFontData(JNIEnv *env,jobject font, char **errmsg);
extern GC awt_getImageGC(Pixmap pixmap);

extern XtAppContext	awt_appContext;
extern Pixel		awt_defaultBg;
extern Pixel		awt_defaultFg;
extern AwtData		*defaultConfig;

/* allocated and initialize a structure */
#define ZALLOC(T)	((struct T *)calloc(1, sizeof(struct T)))

#define XDISPLAY awt_display;

extern Boolean awt_currentThreadIsPrivileged(JNIEnv *env);

extern void awt_put_back_event(JNIEnv *env, XEvent *event);
extern void awt_MToolkit_modalWait(int (*terminateFn)(void *data), void *data);
extern void awt_Frame_guessInsets(struct FrameData *fdata);
extern void awt_output_flush(void);

extern void awt_addWidget(Widget w, Widget origin, void *peer, long event_mask);
extern void awt_delWidget(Widget w);
extern void awt_enableWidgetEvents(Widget w, long event_mask);
extern void awt_disableWidgetEvents(Widget w, long event_mask);

extern int awt_allocate_colors(AwtData *);
extern int awtJNI_GetColorForVis (JNIEnv *, jobject, AwtData *);
extern jobject awtJNI_GetColorModel(JNIEnv *, AwtData *);
extern void awtJNI_CreateColorData (JNIEnv *, AwtData *, int);
extern void awt_allocate_systemcolors(XColor *, int, AwtData *);
extern void awt_allocate_systemrgbcolors(jint *, int, AwtData *);
extern int getTopGuess();
extern int getSideGuess();
 
void awtJNI_DeleteGlobalRef(JNIEnv *env,jobject this);
void awtJNI_DeleteGlobalMenuRef(JNIEnv *env,jobject this);
jobject awtJNI_CreateAndSetGlobalRef(JNIEnv *env,jobject this);
void awtJNI_CleanupGlobalRefs(void);

/* XXX: Motif internals. Need to fix 4090493. */
#define MOTIF_XmINVALID_DIMENSION	((Dimension) 0xFFFF)
#define MOTIF_XmDEFAULT_INDICATOR_DIM	((Dimension) 9)

extern Dimension awt_computeIndicatorSize(struct FontData *fdata);
extern Dimension awt_adjustIndicatorSizeForMenu(Dimension indSize);

#endif           /* _AWT_P_H_ */
