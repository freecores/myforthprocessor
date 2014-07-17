/*
 * @(#)SimpleInputMethodWindow.java	1.9 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt.im;

import java.awt.Frame;

/**
 * Implements a simple input method window that provides the minimal
 * functionality as specified in
 * {@link java.awt.im.spi.InputMethodContext#createInputMethodWindow}.
 *
 * @version 1.9 01/23/03
 */
public class SimpleInputMethodWindow
        extends Frame
        implements InputMethodWindow {

    InputContext inputContext = null;

    /**
     * Constructs a simple input method window.
     */
    public SimpleInputMethodWindow(String title, InputContext context) {
        super(title);
        if (context != null) {
            this.inputContext = context;
        }
	setFocusableWindowState(false);
    }
    
    public void setInputContext(InputContext inputContext) {
        this.inputContext = inputContext;
    }

    public java.awt.im.InputContext getInputContext() {
        if (inputContext != null) {
            return inputContext;
        } else {
            return super.getInputContext();
        }
    }
}
