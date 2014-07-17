/*
 * @(#)REException.java	1.11 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.misc;

/**
 * A class to signal exception from the RegexpPool class.
 * @author  James Gosling
 */

public class REException extends Exception {
    REException (String s) {
	super(s);
    }
}
