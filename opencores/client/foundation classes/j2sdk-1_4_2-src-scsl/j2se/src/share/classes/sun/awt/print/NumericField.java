/*
 * @(#)NumericField.java	1.5 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt.print;

import java.awt.*;
import java.awt.event.*;
import javax.swing.*;

class NumericField extends JTextField {
    protected void processEvent(AWTEvent evt) {
        int id = evt.getID();
        if (id >= KeyEvent.KEY_FIRST && id <= KeyEvent.KEY_LAST) {
	    KeyEvent kevt = (KeyEvent) evt;
	    char c = kevt.getKeyChar();
	    if (c >= 0x20 && c <= 0x7F && !Character.isDigit(c)) {
	        if (id == KeyEvent.KEY_PRESSED) {
		    Toolkit.getDefaultToolkit().beep();
		} 
		kevt.consume();
		return;
	    }
	}
	super.processEvent(evt);
    }
    public void setValue(int value) {
        setText("" + value);
    }
    public int getValue() throws NumberFormatException {
	return Integer.parseInt(getText());
    }   
}
