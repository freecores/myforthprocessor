/*
 * @(#)WScrollPanePeer.java	1.26 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package sun.awt.windows;

import java.awt.*;
import java.awt.event.AdjustmentEvent;
import java.awt.peer.ScrollPanePeer;
import sun.awt.DebugHelper;
import sun.awt.PeerEvent;

class WScrollPanePeer extends WPanelPeer implements ScrollPanePeer {
    private static final DebugHelper dbg
	= DebugHelper.create(WScrollPanePeer.class);
 
    int scrollbarWidth;
    int scrollbarHeight;
    int prevx;
    int prevy;

    static {
	initIDs();
    }

    static native void initIDs();
    native void create(WComponentPeer parent);	    
    native int getOffset(int orient);

    WScrollPanePeer(Component target) {
	super(target);
        scrollbarWidth = _getVScrollbarWidth();
        scrollbarHeight = _getHScrollbarHeight();
    }

    void initialize() {
        super.initialize();
        setInsets();
	Insets i = getInsets();
	setScrollPosition(-i.left,-i.top);
    }

    public void setUnitIncrement(Adjustable adj, int p) {
        // The unitIncrement is grabbed from the target as needed.
    }

    public Insets insets() {
        return getInsets();
    }
    private native void setInsets();

    public native synchronized void setScrollPosition(int x, int y);

    public int getHScrollbarHeight() {
        return scrollbarHeight;
    }
    private native int _getHScrollbarHeight();

    public int getVScrollbarWidth() {
        return scrollbarWidth;
    }
    private native int _getVScrollbarWidth();

    public Point getScrollOffset() {
	int x = getOffset(Adjustable.HORIZONTAL);
	int y = getOffset(Adjustable.VERTICAL);
	return new Point(x, y);
    }

    /**
     * The child component has been resized.  The scrollbars must be
     * updated with the new sizes.  At the native level the sizes of
     * the actual windows may not have changed yet, so the size
     * information from the java-level is passed down and used.
     */
    public void childResized(int width, int height) {
	ScrollPane sp = (ScrollPane)target;
	Dimension vs = sp.getSize();
	setSpans(vs.width, vs.height, width, height);
        setInsets();
    }

    native synchronized void setSpans(int viewWidth, int viewHeight, 
                                      int childWidth, int childHeight);

    /**
     * Called by ScrollPane's internal observer of the scrollpane's adjustables.
     * This is called whenever a scroll position is changed in one
     * of adjustables, whether it was modified externally or from the
     * native scrollbars themselves.  
     */
    public void setValue(Adjustable adj, int v) {
	Component c = getScrollChild();
	Point p = c.getLocation();
	switch(adj.getOrientation()) {
	case Adjustable.VERTICAL:
	    setScrollPosition(-(p.x), v);
	    break;
	case Adjustable.HORIZONTAL:
	    setScrollPosition(v, -(p.y));
	    break;
	}
    }
	    
    private native Component getScrollChild();

    /*
     * Called from Windows in response to WM_VSCROLL/WM_HSCROLL message
     */
    private void postScrollEvent(int orient, int type,
				 int pos, boolean isAdjusting)
    {
	Runnable adjustor = new Adjustor(orient, type, pos, isAdjusting);
	WToolkit.executeOnEventHandlerThread(new ScrollEvent(target, adjustor));
    }

    /*
     * Event that executes on the Java dispatch thread to move the
     * scroll bar thumbs and paint the exposed area in one synchronous
     * operation.
     */
    class ScrollEvent extends PeerEvent {
	ScrollEvent(Object source, Runnable runnable) {
	    super(source, runnable, 0L);
	}

	public PeerEvent coalesceEvents(PeerEvent newEvent) {
	    if (dbg.on) dbg.println("ScrollEvent coalesced "+newEvent);
	    if (newEvent instanceof ScrollEvent) {
		return newEvent;
	    }
	    return null;
	}
    }

    /*
     * Runnable for the ScrollEvent that performs the adjustment.
     */
    class Adjustor implements Runnable {
	int orient;		// selects scrollbar
	int type;		// adjustment type
	int pos;		// new position (only used for absolute)
	boolean isAdjusting;	// isAdjusting status

	Adjustor(int orient, int type, int pos, boolean isAdjusting) {
	    this.orient = orient;
	    this.type = type;
	    this.pos = pos;
	    this.isAdjusting = isAdjusting;
	}

	public void run() {
	    ScrollPane sp = (ScrollPane)WScrollPanePeer.this.target;
	    ScrollPaneAdjustable adj = null;

	    // ScrollPaneAdjustable made public in 1.4, but
	    // get[HV]Adjustable can't be declared to return
	    // ScrollPaneAdjustable because it would break backward
	    // compatibility -- hence the cast

	    if (orient == Adjustable.VERTICAL) {
		adj = (ScrollPaneAdjustable)sp.getVAdjustable();
	    } else if (orient == Adjustable.HORIZONTAL) {
		adj = (ScrollPaneAdjustable)sp.getHAdjustable();
	    } else {
		if (dbg.on) dbg.assertion(false);
	    }

	    if (adj == null) {
		return;
	    }

	    int newpos = adj.getValue();
	    switch (type) {
	      case AdjustmentEvent.UNIT_DECREMENT:
		  newpos -= adj.getUnitIncrement();
		  break;
	      case AdjustmentEvent.UNIT_INCREMENT:
		  newpos += adj.getUnitIncrement();
		  break;
	      case AdjustmentEvent.BLOCK_DECREMENT:
		  newpos -= adj.getBlockIncrement();
		  break;
	      case AdjustmentEvent.BLOCK_INCREMENT:
		  newpos += adj.getBlockIncrement();
		  break;
	      case AdjustmentEvent.TRACK:
		  newpos = this.pos;
		  break;
	      default:
		  if (dbg.on) dbg.assertion(false);
		  return;
	    }

	    // keep scroll position in acceptable range
	    newpos = Math.max(adj.getMinimum(), newpos);
	    newpos = Math.min(adj.getMaximum(), newpos);

	    // set value, this will synchronously fire an AdjustmentEvent
	    adj.setValueIsAdjusting(isAdjusting);
	    adj.setValue(newpos);

	    // Paint the exposed area right away.  To do this - find
	    // the heavyweight ancestor of the scroll child.
	    Component hwAncestor = getScrollChild();
	    while (hwAncestor != null
		   && !(hwAncestor.getPeer() instanceof WComponentPeer))
	    {
		hwAncestor = hwAncestor.getParent();
	    }
	    if (dbg.on) {
		dbg.assertion(hwAncestor != null,
		    "couldn't find heavyweight ancestor of scroll pane child");
	    }
	    WComponentPeer hwPeer = (WComponentPeer)hwAncestor.getPeer();
	    hwPeer.paintDamagedAreaImmediately();
	}
    }
}
