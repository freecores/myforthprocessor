/*
 * @(#)WindowClosingSupport.java	1.6 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt;

import java.awt.Window;
import java.awt.event.WindowEvent;

/**
 * Interface for identifying and casting toolkits that support
 * WindowClosingListeners.
 */
public interface WindowClosingSupport {
    WindowClosingListener getWindowClosingListener();
    void setWindowClosingListener(WindowClosingListener wcl);
}
