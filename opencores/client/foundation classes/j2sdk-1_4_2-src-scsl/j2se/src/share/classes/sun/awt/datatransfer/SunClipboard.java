/*
 * @(#)SunClipboard.java	1.11 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt.datatransfer;

import java.awt.datatransfer.Clipboard;
import java.awt.datatransfer.FlavorTable;
import java.awt.datatransfer.SystemFlavorMap;
import java.awt.datatransfer.Transferable;
import java.awt.datatransfer.ClipboardOwner;

import java.beans.PropertyChangeEvent;
import java.beans.PropertyChangeListener;

import sun.awt.AppContext;
import sun.awt.PeerEvent;
import sun.awt.SunToolkit;


/**
 * Serves as a common, helper superclass for the Win32 and X11 system
 * Clipboards.
 *
 * @author Danila Sinopalnikov
 * @version 1.11, 01/23/03
 *
 * @since 1.3
 */
public abstract class SunClipboard extends Clipboard 
    implements PropertyChangeListener {

    public static final FlavorTable flavorMap =
	(FlavorTable)SystemFlavorMap.getDefaultFlavorMap();

    private AppContext contentsContext = null;

    public SunClipboard(String name) {
        super(name);
    }

    public synchronized void setContents(Transferable contents,
                                         ClipboardOwner owner) {
        // 4378007 : Toolkit.getSystemClipboard().setContents(null, null)
        // should throw NPE
        if (contents == null) {
            throw new NullPointerException("contents");
        }

        initContext();

        final ClipboardOwner oldOwner = this.owner;
        final Transferable oldContents = this.contents;

        try {
            this.owner = owner;
            this.contents = new TransferableProxy(contents, true);

            setContentsNative(contents);

        } finally {
            if (oldOwner != null && oldOwner != owner) {
                oldOwner.lostOwnership(this, oldContents);
            }
        }
    }

    private synchronized void initContext() {
        final AppContext context = AppContext.getAppContext();

        if (contentsContext != context) {
            // Need to synchronize on the AppContext to guarantee that it cannot
            // be disposed after the check, but before the listener is added.
            synchronized (context) {
                if (context.isDisposed()) {
                    throw new IllegalStateException("Can't set contents from disposed AppContext");
                }
                context.addPropertyChangeListener
                    (AppContext.DISPOSED_PROPERTY_NAME, this);
            }
            if (contentsContext != null) {
                contentsContext.removePropertyChangeListener
                    (AppContext.DISPOSED_PROPERTY_NAME, this);
            }
            contentsContext = context;
        }
    }

    public synchronized Transferable getContents(Object requestor) {
        if (contents != null) {
            return contents;
        }
        return new ClipboardTransferable(this);
    }

    public void openClipboard(SunClipboard newOwner) {}
    public void closeClipboard() {}    

    public abstract long getID();

    public void propertyChange(PropertyChangeEvent evt) {
        if (AppContext.DISPOSED_PROPERTY_NAME.equals(evt.getPropertyName()) &&
            Boolean.TRUE.equals(evt.getNewValue())) {
            final AppContext disposedContext = (AppContext)evt.getSource();
            lostOwnershipLater(disposedContext);
        }
    }

    protected void lostOwnershipImpl() {
        lostOwnershipLater(null);
    }   

    /**
     * Clears the clipboard state (contents, owner and contents context) and
     * notifies the current owner that ownership is lost. Does nothing if the
     * argument is not <code>null</code> and is not equal to the current
     * contents context.
     *
     * @param disposedContext the AppContext that is disposed or
     *        <code>null</code> if the ownership is lost because another
     *        application acquired ownership.
     */
    protected void lostOwnershipLater(final AppContext disposedContext) {
        final AppContext context = this.contentsContext;
        if (context == null) {
            return;
        }
        
        final Runnable runnable = new Runnable() {
                public void run() {
                    final SunClipboard sunClipboard = SunClipboard.this;
                    ClipboardOwner owner = null;
                    Transferable contents = null;
                    
                    synchronized (sunClipboard) {
                        final AppContext context = sunClipboard.contentsContext;
                        
                        if (context == null) {
                            return;
                        }
                        
                        if (disposedContext == null || context == disposedContext) { 
                            owner = sunClipboard.owner;
                            contents = sunClipboard.contents;
                            sunClipboard.contentsContext = null;
                            sunClipboard.owner = null;
                            sunClipboard.contents = null;
                            sunClipboard.clearNativeContext();
                            context.removePropertyChangeListener
                                (AppContext.DISPOSED_PROPERTY_NAME, sunClipboard);
                        } else {
                            return;
                        }
                    }
                    if (owner != null) {
                        owner.lostOwnership(sunClipboard, contents);
                    }
                }                
            };

        SunToolkit.postEvent(context, new PeerEvent(this, runnable,
                                                    PeerEvent.PRIORITY_EVENT));
    }

    protected abstract void clearNativeContext();

    protected abstract void setContentsNative(Transferable contents);
}
