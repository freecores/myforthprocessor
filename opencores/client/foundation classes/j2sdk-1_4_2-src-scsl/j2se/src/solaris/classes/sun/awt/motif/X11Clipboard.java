/*
 * @(#)X11Clipboard.java	1.24 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt.motif;

import java.awt.datatransfer.ClipboardOwner;
import java.awt.datatransfer.Transferable;

import sun.awt.datatransfer.SunClipboard;
import sun.awt.datatransfer.TransferableProxy;


/**
 * A class which interfaces with the X11 selection service in order to support
 * data transfer via Clipboard operations. Most of the work is provided by
 * sun.awt.datatransfer.DataTransferer.
 *
 * @author Amy Fowler
 * @author Roger Brinkley
 * @author Danila Sinopalnikov
 * @version 1.24, 01/23/03
 *
 * @since JDK1.1
 */
public class X11Clipboard extends SunClipboard implements X11SelectionHolder {

    private final X11Selection clipboardSelection;

    /**
     * Creates a system clipboard object.
     */
    public X11Clipboard(String name, String selectionName) {
        super(name);
        clipboardSelection = new X11Selection(selectionName);
    }

    protected void setContentsNative(Transferable contents) {
        if (!clipboardSelection.getSelectionOwnership(contents, this)) {
            // Need to figure out how to inform owner the request failed...
            this.owner = null;
            this.contents = null;
        }
    }

    public long getID() {
        return clipboardSelection.atom;
    }

    // NOTE: This method may be called by privileged threads.
    //       DO NOT INVOKE CLIENT CODE ON THIS THREAD!
    public void lostSelectionOwnership() {
        lostOwnershipImpl();
    }

    protected void clearNativeContext() {
        clipboardSelection.clearNativeContext();
    }
}
