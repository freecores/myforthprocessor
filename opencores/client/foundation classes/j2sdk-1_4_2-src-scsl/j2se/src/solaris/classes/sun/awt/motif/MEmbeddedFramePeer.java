/*
 * @(#)MEmbeddedFramePeer.java	1.15 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt.motif;

import sun.awt.EmbeddedFrame;

public class MEmbeddedFramePeer extends MFramePeer {

    public MEmbeddedFramePeer(EmbeddedFrame target) {
        super(target);
    }

    void create(MComponentPeer parent) {
	NEFcreate(parent, ((MEmbeddedFrame)target).handle);
    }
    native void NEFcreate(MComponentPeer parent, long handle);
    native void pShow();
}
