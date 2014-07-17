/*
 * @(#)CPX.java	1.8 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


package sun.tools.javap;

/**
 * Stores constant pool entry information with one field.
 *
 * @author  Sucheta Dambalkar (Adopted code from jdis)
 */
class CPX {
    int cpx;
    
    CPX (int cpx) {
	this.cpx=cpx;
    }
}
