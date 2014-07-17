/*
 * @(#)DoubleEditor.java	1.10 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.beans.editors;

/**
 * Property editor for a java builtin "double" type.
 *
 */

import java.beans.*;

public class DoubleEditor extends NumberEditor {

    public void setAsText(String text) throws IllegalArgumentException {
	setValue(Double.valueOf(text));
    }

}

