/*
 * @(#)ToolkitThreadBlockedHandler.java	1.3 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt.datatransfer;

public interface ToolkitThreadBlockedHandler {
    public void lock();
    public void unlock();
    public void enter();
    public void exit();
}
