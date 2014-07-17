/*
 * @(#)NullComponentPeer.java	1.14 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt;

import java.awt.AWTException;
import java.awt.BufferCapabilities;
import java.awt.Color;
import java.awt.Component;
import java.awt.Cursor;
import java.awt.Dimension;
import java.awt.Font;
import java.awt.FontMetrics;
import java.awt.Graphics;
import java.awt.GraphicsConfiguration;
import java.awt.Image;
import java.awt.Insets;
import java.awt.MenuBar;
import java.awt.Point;
import java.awt.Event;
import java.awt.event.PaintEvent;
import java.awt.image.ColorModel;
import java.awt.image.ImageObserver;
import java.awt.image.ImageProducer;
import java.awt.image.VolatileImage;
import java.awt.peer.CanvasPeer;
import java.awt.peer.LightweightPeer;
import java.awt.peer.PanelPeer;

/**
 * Implements the LightweightPeer interface for use in lightweight components
 * that have no native window associated with them.  This gets created by
 * default in Component so that Component and Container can be directly
 * extended to create useful components written entirely in java.  These
 * components must be hosted somewhere higher up in the component tree by a
 * native container (such as a Frame).
 *
 * This implementation provides no useful semantics and serves only as a
 * marker.  One could provide alternative implementations in java that do
 * something useful for some of the other peer interfaces to minimize the
 * native code.
 *
 * This was renamed from java.awt.LightweightPeer (a horrible and confusing
 * name) and moved from java.awt.Toolkit into sun.awt as a public class in
 * its own file.
 *
 * @author Timothy Prinzing
 * @author Michael Martak
 */

public class NullComponentPeer implements LightweightPeer,
    CanvasPeer, PanelPeer {
    
    public boolean isObscured() {
        return false;
    }

    public boolean canDetermineObscurity() {
        return false;
    }

    public boolean isFocusable() {
        return false;
    }

    public void setVisible(boolean b) {
    }

    public void show() {
    }

    public void hide() {
    }

    public void setEnabled(boolean b) {
    }

    public void enable() {
    }

    public void disable() {
    }

    public void paint(Graphics g) {
    }

    public void repaint(long tm, int x, int y, int width, int height) {
    }

    public void print(Graphics g) {
    }

    public void setBounds(int x, int y, int width, int height) {
    }

    public void reshape(int x, int y, int width, int height) {
    }

    public void coalescePaintEvent(PaintEvent e) {
    }

    public boolean handleEvent(Event e) {
        return false;
    }

    public void handleEvent(java.awt.AWTEvent arg0) {
    }

    public Dimension getPreferredSize() {
        return new Dimension(1,1);
    }

    public Dimension getMinimumSize() {
        return new Dimension(1,1);
    }

    public java.awt.Toolkit getToolkit() {
        return null;
    }

    public ColorModel getColorModel() {
        return null;
    }

    public Graphics getGraphics() {
        return null;
    }
        
    public GraphicsConfiguration getGraphicsConfiguration() {
        return null;
    }
        
    public FontMetrics	getFontMetrics(Font font) {
        return null;
    }

    public void dispose() {
    // no native code
    }

    public void setForeground(Color c) {
    }

    public void setBackground(Color c) {
    }

    public void setFont(Font f) {
    }

    public void updateCursorImmediately() {
    }

    public void setCursor(Cursor cursor) {
    }

    public boolean requestFocus
        (Component lightweightChild, boolean temporary,
         boolean focusedWindowChangeAllowed, long time) {
	return false;
    }

    public Image createImage(ImageProducer producer) {
        return null;
    }

    public Image createImage(int width, int height) {
        return null;
    }

    public boolean prepareImage(Image img, int w, int h, ImageObserver o) {
        return false;
    }

    public int	checkImage(Image img, int w, int h, ImageObserver o) {
        return 0;
    }

    public Dimension preferredSize() {
        return getPreferredSize();
    }

    public Dimension minimumSize() {
        return getMinimumSize();
    }

    public Point getLocationOnScreen() {
        return new Point(0,0);
    }

    public Insets getInsets() {
        return insets();
    }

    public void beginValidate() {
    }

    public void endValidate() {
    }

    public Insets insets() {
        return new Insets(0, 0, 0, 0);
    }
    
    public boolean isPaintPending() {
	return false;
    }

    public boolean handlesWheelScrolling() {
        return false;
    }

    public VolatileImage createVolatileImage(int width, int height) {
        return null;
    }

    public void beginLayout() {
    }

    public void endLayout() {
    }
    
    public void createBuffers(int numBuffers, BufferCapabilities caps)
        throws AWTException {
        throw new AWTException(
            "Page-flipping is not allowed on a lightweight component");
    }
    public Image getBackBuffer() {
        throw new IllegalStateException(
            "Page-flipping is not allowed on a lightweight component");
    }
    public void flip(BufferCapabilities.FlipContents flipAction) {
        throw new IllegalStateException(
            "Page-flipping is not allowed on a lightweight component");
    }
    public void destroyBuffers() {
    }
}

