/*
 * @(#)X11Selection.java	1.30 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt.motif;

import java.awt.Toolkit;

import java.awt.datatransfer.Transferable;
import java.awt.datatransfer.FlavorMap;
import java.awt.datatransfer.DataFlavor;
import java.awt.datatransfer.SystemFlavorMap;

import java.util.Map;
import java.util.Vector;

import sun.awt.AppContext;
import sun.awt.SunToolkit;

import sun.awt.datatransfer.DataTransferer;

/*
 * Implements a general interface to the X11 selection mechanism.
 *
 * @author Amy Fowler
 * @author Roger Brinkley
 * @author Danila Sinopalnikov
 * @version 1.30, 01/23/03
 *
 * @since JDK1.1
 */
public class X11Selection {

    static FlavorMap flavorMap = SystemFlavorMap.getDefaultFlavorMap();

    static Vector selections;

    long atom;

    private X11SelectionHolder holder;
    private Transferable contents;

    private boolean disposed = false;
    private byte[] data = null;
    private boolean dataAvailable = false;
    private static final Object source = new Object();

    static {
	// 4154170:  Need to ensure the the toolkit is initialized prior
	// to executing this initializer
	Toolkit toolkit = Toolkit.getDefaultToolkit();

	selections = new Vector();

        initIDs();
	init();

    }

    private static native void initIDs();
    static native void init();

    public X11Selection(String name) {
	atom = ((MDataTransferer)DataTransferer.getInstance()).getAtomForTarget(name);
        selections.addElement(this);
    }

    private static Object[] getSelectionsArray() {
	return selections.toArray();
    }

   /*
    * methods for acting as selection provider
    */
    native boolean pGetSelectionOwnership(Object source,
                                          Transferable transferable,
                                          long[] formats,
                                          Map formatMap,
                                          X11SelectionHolder holder);

    boolean getSelectionOwnership(Transferable contents,
				  X11SelectionHolder holder) {
        Map formatMap = 
            DataTransferer.getInstance().getFormatsForTransferable
	        (contents, DataTransferer.adaptFlavorMap(flavorMap));
        long[] formats = 
            DataTransferer.getInstance().keysToLongArray(formatMap);
        SunToolkit.insertTargetMapping(source, AppContext.getAppContext());

        /*
         * Update 'contents' and 'holder' fields in the native code under
         * AWTLock protection to prevent race with lostSelectionOwnership().
         */        
        return pGetSelectionOwnership(source, contents, formats, formatMap,
                                      holder);
    }

    // NOTE: This method may be called by privileged threads.
    //       DO NOT INVOKE CLIENT CODE ON THIS THREAD!
    private void lostSelectionOwnership() {
        if (holder != null) {
            holder.lostSelectionOwnership();
            holder = null;
        }
	contents = null;
    }

    native void clearNativeContext();

    /*
     * Subclasses should override disposeImpl() instead of dispose(). Client
     * code should always invoke dispose(), never disposeImpl().
     */
    protected void disposeImpl() {
        selections.removeElement(this);
    }

    public final void dispose() {
        boolean call_disposeImpl = false;

        if (!disposed) {
            synchronized (this) {
                if (!disposed) {
                    disposed = call_disposeImpl = true;
                }
            }
        }

        if (call_disposeImpl) {
            disposeImpl();
        }
    }
}
