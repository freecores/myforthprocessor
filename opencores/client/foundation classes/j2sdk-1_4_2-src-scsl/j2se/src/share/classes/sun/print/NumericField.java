/*
 * @(#)NumericField.java	1.6 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.print;

import java.awt.*;

class NumericField extends TextField {
    public NumericField(int chars) {
        super(chars);
    }
    public boolean keyDown(Event evt, int key) {
        if (Character.isDigit((char)key) == false && key != '\b') {
            Toolkit.getDefaultToolkit().beep();
            return true;
        }       
        return false;   
    }
    public void setValue(int value) {
        Integer intVal = new Integer(value);
        setText(intVal.toString());
    }
    public int getValue() throws NumberFormatException {
        Integer intVal;
        String valStr = getText();

        intVal = new Integer(valStr);
        return intVal.intValue();
    }   
}
