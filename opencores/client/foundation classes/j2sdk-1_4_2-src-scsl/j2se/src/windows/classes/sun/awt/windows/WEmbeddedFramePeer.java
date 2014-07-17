/*
 * @(#)WEmbeddedFramePeer.java	1.12 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt.windows;

import java.awt.Component;
import java.awt.Graphics;
import sun.awt.EmbeddedFrame;

public class WEmbeddedFramePeer extends WFramePeer {

    public WEmbeddedFramePeer(EmbeddedFrame target) {
        super(target);
    }

    // force AWT into modal state
    native void pushModality();

    // release AWT from modal state
    native  void popModality();

    native void create(WComponentPeer parent);

    // suppress printing of an embedded frame.
    public void print(Graphics g) {}
}
