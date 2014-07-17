/*
 * @(#)WClipboard.java	1.23 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt.windows;

import java.awt.datatransfer.ClipboardOwner;
import java.awt.datatransfer.DataFlavor;
import java.awt.datatransfer.Transferable;

import java.io.IOException;

import java.util.Iterator;
import java.util.Map;

import sun.awt.AppContext;
import sun.awt.datatransfer.SunClipboard;
import sun.awt.datatransfer.TransferableProxy;


/**
 * A class which interfaces with the Windows clipboard in order to support
 * data transfer via Clipboard operations. Most of the work is provided by
 * sun.awt.datatransfer.DataTransferer.
 *
 * @author Tom Ball
 * @author David Mendenhall
 * @author Danila Sinopalnikov
 * @version 1.23, 01/23/03
 *
 * @since JDK1.1
 */
public class WClipboard extends SunClipboard {

    public WClipboard() {
        super("System");
    }

    public long getID() {
        return 0;
    }

    protected void setContentsNative(Transferable contents) {

        // Don't use delayed Clipboard rendering for the Transferable's data.
        // If we did that, we would call Transferable.getTransferData on
        // the Toolkit thread, which is a security hole.
        //
        // Get all of the target formats into which the Transferable can be
        // translated. Then, for each format, translate the data and post
        // it to the Clipboard.
        Map formatMap = WDataTransferer.getInstance().
            getFormatsForTransferable(contents, flavorMap);

        openClipboard(this);

        try {
            for (Iterator iter = formatMap.keySet().iterator();
                 iter.hasNext(); ) {
                Long lFormat = (Long)iter.next();
                long format = lFormat.longValue();
                DataFlavor flavor = (DataFlavor)formatMap.get(lFormat);
                
                try {
                    byte[] bytes = WDataTransferer.getInstance().
                        translateTransferable(contents, flavor, format);
                    publishClipboardData(format, bytes);
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
        } finally {
            closeClipboard();
        }
    }

    private void lostSelectionOwnershipImpl() {
        lostOwnershipImpl();
    }
          
    /**
     * Currently delayed data rendering is not used for the Windows clipboard,
     * so there is no native context to clear.
     */
    protected void clearNativeContext() {}

    /**
     * Call the Win32 OpenClipboard function. If newOwner is non-null,
     * we also call EmptyClipboard and take ownership.
     */
    public native void openClipboard(SunClipboard newOwner);
    /**
     * Call the Win32 CloseClipboard function.
     */
    public native void closeClipboard();
    /**
     * Call the Win32 SetClipboardData function.
     */
    private native void publishClipboardData(long format, byte[] bytes);

    private static native void init();
    static {
        init();
    }
}
