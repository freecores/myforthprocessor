/*
 * @(#)WPrintDialog.java	1.8 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt.windows;

import java.awt.*;
import java.awt.peer.*;

import sun.awt.print.AwtPrintControl;

public class WPrintDialog extends Dialog {
    static {
        initIDs();
    }

    private PrintJob job;
    private AwtPrintControl control;

    public WPrintDialog(Frame parent, AwtPrintControl control) {
        super(parent, true);
	this.control = control;
	setLayout(null);
    }

    // Use native code to circumvent access restrictions on Component.peer
    private native void setPeer(ComponentPeer peer);

    public void addNotify() {
        synchronized(getTreeLock()) {
	    Container parent = getParent();
            if (parent != null && parent.getPeer() == null) {
                parent.addNotify();
            }

	    if (getPeer() == null) {
	        ComponentPeer peer = ((WToolkit)Toolkit.getDefaultToolkit()).
		    createWPrintDialog(this);
		setPeer(peer);
	    }
	    super.addNotify();
	}
    }

    private boolean retval = false;
    
    public void setRetVal(boolean ret) {
	retval = ret;
    }

    public boolean getRetVal() {
	return retval;
    }

    /**
     * Initialize JNI field and method ids
     */
    private static native void initIDs();
}
