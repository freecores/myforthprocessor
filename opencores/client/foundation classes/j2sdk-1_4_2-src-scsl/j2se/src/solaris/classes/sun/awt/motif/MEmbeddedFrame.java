/*
 * @(#)MEmbeddedFrame.java	1.11 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt.motif;

import java.awt.Component;
import java.awt.peer.FramePeer;
import sun.awt.EmbeddedFrame;
import java.awt.peer.ComponentPeer;
import sun.awt.*;
import java.awt.*;

public class MEmbeddedFrame extends EmbeddedFrame {

    long handle;

    public MEmbeddedFrame() {
    }

    // handle should be a valid Motif widget.
    public MEmbeddedFrame(long handle) {
       this.handle = handle;
       MToolkit toolkit = (MToolkit)Toolkit.getDefaultToolkit();
       setPeer(toolkit.createEmbeddedFrame(this));
       show();
    }
}
