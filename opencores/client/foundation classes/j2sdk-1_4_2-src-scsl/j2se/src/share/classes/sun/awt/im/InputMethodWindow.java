/*
 * @(#)InputMethodWindow.java	1.7 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt.im;

/**
 * Interface for input method windows that need special handling
 * by input method window.
 *
 * @version 1.7 01/23/03
 */
public interface InputMethodWindow {
    
    /**
     * Sets the input context that this input method window is attached to,
     * null to unattach the window.
     * @see java.awt.im.spi.InputMethodContext#createInputMethodWindow
     */
    public void setInputContext(InputContext inputContext);

}
