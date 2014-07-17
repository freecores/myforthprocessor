/*
 * @(#)MToolkitThreadBlockedHandler.java	1.3 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt.motif;

import sun.awt.datatransfer.ToolkitThreadBlockedHandler;

final class MToolkitThreadBlockedHandler implements 
                                 ToolkitThreadBlockedHandler {
    private static ToolkitThreadBlockedHandler priveleged_lock = null;
    static {
        priveleged_lock = new MToolkitThreadBlockedHandler();    
    }
    private MToolkitThreadBlockedHandler() {}
    public static ToolkitThreadBlockedHandler getToolkitThreadBlockedHandler() {
        return priveleged_lock;
    }
    public void lock() {
        AWTLockAccess.awtLock();
    }
    public void unlock() {
        AWTLockAccess.awtUnlock();
    }
    public native void enter();
    public native void exit();
}
