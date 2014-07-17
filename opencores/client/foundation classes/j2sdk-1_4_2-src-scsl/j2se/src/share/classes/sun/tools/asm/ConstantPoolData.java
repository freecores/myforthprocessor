/*
 * @(#)ConstantPoolData.java	1.18 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.tools.asm;

import sun.tools.java.*;
import java.io.IOException;
import java.io.DataOutputStream;

/**
 * Base constant data class. Every constant pool data item
 * is derived from this class.
 */

abstract class  ConstantPoolData implements RuntimeConstants {
    int index;

    /**
     * Write the constant to the output stream
     */
    abstract void write(Environment env, DataOutputStream out, ConstantPool tab) throws IOException;

    /**
     * Return the order of the constant
     */
    int order() {
	return 0;
    }

    /**
     * Return the number of entries that it takes up in the constant pool
     */
    int width() {
	return 1;
    }
}
