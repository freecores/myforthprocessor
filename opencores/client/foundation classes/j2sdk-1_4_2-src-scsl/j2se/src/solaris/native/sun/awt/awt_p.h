/*
 * @(#)awt_p.h	1.113 03/04/28
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

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#ifndef HEADLESS
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
#endif /* !HEADLESS */
#include "awt.h"
#include "awt_util.h"
#include "color.h"
#include "colordata.h"
#include "gdefs.h"

#include <sun_awt_motif_MComponentPeer.h>


#ifndef HEADLESS
#include "awt_motif.h"

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
    jlong   	       event_mask;
    struct WidgetInfo* next;
};
#endif /* !HEADLESS */

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

#ifndef HEADLESS
typedef struct _AwtGraphicsConfigData  {
    int         awt_depth;
    Colormap    awt_cmap;
    XVisualInfo awt_visInfo;
    int         awt_num_colors;
    awtImageData *awtImage;
    int         (*AwtColorMatch)(int, int, int,
                                 struct _AwtGraphicsConfigData *);
    XImage	*savedXImage;	 /* Used in awt_Graphics.c */
    XImage	*monoImage;
    Pixmap      monoPixmap;      /* Used in awt_Graphics.c */
    int         monoPixmapWidth; /* Used in awt_Graphics.c */
    int         monoPixmapHeight;/* Used in awt_Graphics.c */
    GC          monoPixmapGC;    /* Used in awt_Graphics.c */
    int	        bytesPerChannel; /* Used in awt_Graphics.c */
    int         pixelStride;     /* Used in awt_Graphics.c */   
    int	        primitiveType;   /* Used in awt_Graphics.c */
    int         byteSwapped;     /* Used in awt_Graphics.c */
    ColorData      *color_data;
} AwtGraphicsConfigData;

typedef AwtGraphicsConfigData* AwtGraphicsConfigDataPtr;

typedef struct _AwtScreenData {
    int numConfigs;
    Window root;
    unsigned long whitepixel;
    unsigned long blackpixel;
    AwtGraphicsConfigDataPtr defaultConfig;
    AwtGraphicsConfigDataPtr *configs;
} AwtScreenData;

typedef AwtScreenData* AwtScreenDataPtr;

struct ComponentData {
    Widget	widget;
    int 	repaintPending;
    DamageRect  repaintRect;
    DamageRect  exposeRect;
    DropSitePtr dsi;
};

struct MessageDialogData {
    struct ComponentData	comp;
    int			isModal;
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
    AwtGraphicsConfigDataPtr	awtData;
    XImage                      *monoImage;
    void                        *dgaDev; /* pointer to DGA Device handle */
    void                        *pScreen;
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

struct FrameData {
    struct CanvasData	winData;
    int			isModal;
    Widget		mainWindow;
    Widget              focusProxy;     /* for key events targeted at Windows */
    Widget		menuBar;
    Widget		warningWindow;
    int			top;		/* these four are the insets...	*/
    int			bottom;
    int			left;
    int			right;
    int                 topGuess;       /* these four are the guessed insets */
    int                 bottomGuess;
    int                 leftGuess;
    int                 rightGuess;
    int			mbHeight;	/* height of the menubar window	*/
    int			wwHeight;	/* height of the warning window	*/
    jint		state;		/* java.awt.Frame.state bits	*/
    Boolean		reparented;
    Boolean		shellResized;	/* frame shell has been resized	*/
    Boolean		canvasResized;	/* frame inner canvas resized	*/
    Boolean		menuBarReset;	/* frame menu bar added/removed	*/
    Boolean		isResizable;	/* is this window resizable ?	*/
    Boolean		isFixedSizeSet;	/* is fixed size already set ?	*/
    Boolean		isShowing;	/* is this window now showing ?	*/
    Boolean		hasTextComponentNative;
    Boolean		need_reshape;
    Boolean             callbacksAdded; /* needed for fix for 4078176   */
    Pixmap              iconPixmap;     /* Pixmap to hold icon image    */
    int                 iconWidth;
    int                 iconHeight;
    int	                imHeight;       /* imStatusBar's height         */ 
    Boolean             imRemove;       /* ImStatusBar is being removed */
    Boolean             fixInsets;      /* [jk] REMINDER: remove if possible */
    int			decor;		/* type of native decorations */
    Boolean		initialFocus;	/* does Window take focus initially */
    Boolean		isInputMethodWindow;

    /* 
     * Fix for BugTraq ID 4060975. 
     * firstShellEH() stores to this field handle of the widget that had 
     * focus before the shell was resized so that we can later restore it.
     */
    Widget              focusWidget;   
    int screenNum;      /* Which screen this Window is on.  Xinerama-aware. */
    Boolean             isDisposeScheduled;
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

extern XmFontList getMotifFontList(void);
extern XFontSet getMotifFontSet(void);
extern XFontStruct *getMotifFontStruct(void);
extern Boolean awt_isAwtWidget(Widget widget);

struct ChoiceData {
    struct ComponentData comp;
    Widget		 menu;
    Widget		 *items;
    int			 maxitems;
    int			 n_items;
    short                n_columns;
/* Bug 4255631 Solaris: Size returned by Choice.getSize() does not match
 * actual size
 * y and height which Choice takes in pReshape
*/
    jint                 bounds_y;
    jint                 bounds_height;
};

struct ImageData {
    Pixmap	xpixmap;
    Pixmap	xmask;
};

extern int awt_init_gc(JNIEnv *env,Display *display, struct GraphicsData *gdata,jobject thisgraphics);

extern struct FontData *awtJNI_GetFontData(JNIEnv *env,jobject font, char **errmsg);
extern GC awt_getImageGC(Pixmap pixmap);

extern AwtGraphicsConfigDataPtr getDefaultConfig(int screen);
extern AwtScreenDataPtr getScreenData(int screen);
#endif /* !HEADLESS */

/* allocated and initialize a structure */
#define ZALLOC(T)	((struct T *)calloc(1, sizeof(struct T)))

#ifndef HEADLESS
#define XDISPLAY awt_display;

extern Boolean awt_currentThreadIsPrivileged(JNIEnv *env);

extern void awt_put_back_event(JNIEnv *env, XEvent *event);
extern void awt_MToolkit_modalWait(int (*terminateFn)(void *data), void *data);
extern void awt_Frame_guessInsets(struct FrameData *fdata);

extern void awt_addWidget(Widget w, Widget origin, void *peer, jlong event_mask);
extern void awt_delWidget(Widget w);

extern int awt_allocate_colors(AwtGraphicsConfigDataPtr);
extern void awt_allocate_systemcolors(XColor *, int, AwtGraphicsConfigDataPtr);
extern void awt_allocate_systemrgbcolors(jint *, int, AwtGraphicsConfigDataPtr);

extern int awtJNI_GetColor(JNIEnv *, jobject);
extern int awtJNI_GetColorForVis (JNIEnv *, jobject, AwtGraphicsConfigDataPtr);
extern jobject awtJNI_GetColorModel(JNIEnv *, AwtGraphicsConfigDataPtr);
extern void awtJNI_CreateColorData (JNIEnv *, AwtGraphicsConfigDataPtr, int lock);

extern Boolean awtJNI_isSelectionOwner(JNIEnv *env, char *sel_str);
extern void awtJNI_notifySelectionLost(JNIEnv *env, char *sel_str);
extern void removePopupMenus();
extern Boolean awtMenuIsActive();
#endif /* !HEADLESS */
 
extern void awtJNI_DeleteGlobalRef(JNIEnv *env,jobject thiscomp);
extern void awtJNI_DeleteGlobalMenuRef(JNIEnv *env,jobject thismenu);
extern jobject awtJNI_CreateAndSetGlobalRef(JNIEnv *env,jobject thiscomp);
extern void awtJNI_CleanupGlobalRefs(void);

#ifndef HEADLESS
/* XXX: Motif internals. Need to fix 4090493. */
#define MOTIF_XmINVALID_DIMENSION	((Dimension) 0xFFFF)
#define MOTIF_XmDEFAULT_INDICATOR_DIM	((Dimension) 9)

extern Dimension awt_computeIndicatorSize(struct FontData *fdata);
extern Dimension awt_adjustIndicatorSizeForMenu(Dimension indSize);
#endif /* !HEADLESS */

#endif           /* _AWT_P_H_ */
