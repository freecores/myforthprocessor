/*
 * @(#)InputMethodJFrame.java	1.3 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt.im;

import javax.swing.JFrame;

/**
 * Implements a Swing based input method window that provides the minimal
 * functionality as specified in
 * {@link java.awt.im.spi.InputMethodContext#createInputMethodJFrame}.
 *
 * @version 1.3 01/23/03
 */
public class InputMethodJFrame
        extends JFrame
        implements InputMethodWindow {

    InputContext inputContext = null;

    /**
     * Constructs a Swing based input method window.
     */
    public InputMethodJFrame(String title, InputContext context) {
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
