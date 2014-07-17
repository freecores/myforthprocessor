/*
 * @(#)ByteEditor.java	1.11 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.beans.editors;

/**
 * Property editor for a java builtin "byte" type.
 *
 */

import java.beans.*;

public class ByteEditor extends NumberEditor {

    public String getJavaInitializationString() {
	return ("((byte)" + getValue() + ")");
    }

    public void setAsText(String text) throws IllegalArgumentException {
	setValue(Byte.valueOf(text));
    }

}

