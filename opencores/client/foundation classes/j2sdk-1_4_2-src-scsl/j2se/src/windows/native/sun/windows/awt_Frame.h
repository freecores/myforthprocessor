/*
 * @(#)awt_Frame.h	1.58 03/04/28
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef AWT_FRAME_H
#define AWT_FRAME_H

#include "awt_Window.h"
#include "awt_MenuBar.h" //add for multifont
#include "awt_Toolkit.h"

#include "java_awt_Frame.h"
#include "sun_awt_windows_WFramePeer.h"


/************************************************************************
 * AwtFrame class
 */

#define AWT_FRAME_WINDOW_CLASS_NAME "SunAwtFrame"


class AwtFrame : public AwtWindow {
public:
    enum FrameExecIds {
	FRAME_SETMENUBAR
    };

    /* int handle field for sun.awt.windows.WEmbeddedFrame */
    static jfieldID handleID;

    /* int state field for java.awt.Frame */
    static jfieldID stateID;

    /* boolean undecorated field for java.awt.Frame */
    static jfieldID undecoratedID;

    AwtFrame();
    virtual ~AwtFrame();

    virtual const char* GetClassName();
    virtual LPCWSTR GetClassNameW();
    
    /* Create a new AwtFrame.  This must be run on the main thread. */
    static AwtFrame* Create(jobject self, jobject parent);

    /* Returns whether this frame is embedded in an external native frame. */
    INLINE BOOL IsEmbedded() { return m_isEmbedded; }

    /* Returns whether this window is in iconified state. */
    INLINE BOOL isIconic() { return m_iconic; }
    INLINE void setIconic(BOOL b) { m_iconic = b; }

    /* Returns whether this window is in zoomed state. */
    INLINE BOOL isZoomed() { return m_zoomed; }
    INLINE void setZoomed(BOOL b) { m_zoomed = b; }

    void SendWindowStateEvent(int oldState, int newState);

    void Reshape(int x, int y, int width, int height);
    void Show();

    INLINE void DrawMenuBar() { VERIFY(::DrawMenuBar(GetHWnd())); }

    void MakeSetIcon(JNIEnv *env, jintArray intRasterData,
                     jbyteArray byteMaskData, int ss, int w, int h);

    /*for WmDrawItem and WmMeasureItem method */
    AwtMenuBar* GetMenuBar();
    void SetMenuBar(AwtMenuBar*);

    MsgRouting WmGetMinMaxInfo(LPMINMAXINFO lpmmi);
    MsgRouting WmSize(UINT type, int w, int h);
    MsgRouting WmActivate(UINT nState, BOOL fMinimized, HWND opposite);
    MsgRouting WmDrawItem(UINT ctrlId, DRAWITEMSTRUCT& drawInfo);
    MsgRouting WmMeasureItem(UINT ctrlId, MEASUREITEMSTRUCT& measureInfo);
    MsgRouting WmEnterMenuLoop(BOOL isTrackPopupMenu);
    MsgRouting WmExitMenuLoop(BOOL isTrackPopupMenu);
    MsgRouting WmMouseUp(UINT flags, int x, int y, int button);
    MsgRouting WmMouseMove(UINT flags, int x, int y);
    MsgRouting WmNcMouseDown(WPARAM hitTest, int x, int y, int button);
    void PushModality();
    void PopModality();

    virtual MsgRouting WmSysCommand(UINT uCmdType, int xPos, int yPos);

    LRESULT WinThreadExecProc(ExecuteArgs * args);

    INLINE BOOL IsUndecorated() { return m_isUndecorated; }

    INLINE HWND GetProxyFocusOwner() {
	DASSERT(AwtToolkit::MainThread() == ::GetCurrentThreadId());
	if (m_proxyFocusOwner == NULL) {
	    CreateProxyFocusOwner();
	}
	return m_proxyFocusOwner;
    }

    void SetMaximizedBounds(int x, int y, int w, int h);
    void ClearMaximizedBounds();

protected:
    /* The frame is undecorated. */
    BOOL m_isUndecorated;

private:
    static BOOL CALLBACK OwnedSetIcon(HWND hWnd, LPARAM lParam);
    static LRESULT CALLBACK ProxyWindowProc(HWND hwnd, UINT message,
					    WPARAM wParam, LPARAM lParam);
    void CreateProxyFocusOwner();
    void DestroyProxyFocusOwner();

  /* The frame's icon and its size.  If NULL, just paint the HotJava
       logo. */
    HICON m_hIcon;

    /* The frame's embedding parent (if any) */
    HWND m_parentWnd;

    /* The frame's menubar. */
    AwtMenuBar* menuBar;

    /* The frame is an EmbeddedFrame. */
    BOOL m_isEmbedded;

    /* used so that calls to ::MoveWindow in SetMenuBar don't propogate
       because they are immediately followed by calls to Component.resize */
    BOOL m_ignoreWmSize;

    /* tracks whether or not menu on this frame is dropped down */
    BOOL m_isMenuDropped;

    /* The frame is an InputMethodWindow */
    BOOL m_isInputMethodWindow;

    char* m_windowClassName;
    LPWSTR m_windowClassNameW;

    /* Receives all keyboard input when an AwtWindow which is not an AwtFrame
       or an AwtDialog (or one of its children) has the logical input focus. */
    HWND m_proxyFocusOwner;
    
    /* The original, default WndProc for m_proxyFocusOwner. */
    WNDPROC m_proxyDefWindowProc;

    BOOL m_iconic;          /* are we in an iconic state */
    BOOL m_zoomed;          /* are we in a zoomed state */

    BOOL  m_maxBoundsSet;
    POINT m_maxPos;
    POINT m_maxSize;

    BOOL isInManualMoveOrSize;
    WPARAM grabbedHitTest;
    POINT savedMousePos;

};

#endif /* AWT_FRAME_H */
