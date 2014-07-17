/*
 * @(#)AttributeNameEnumeration.java	1.9 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
 
package sun.security.x509;

import java.util.Vector;
import java.util.Enumeration;

/**
 * <p>This class provides the Enumeration implementation used
 * by all the X509 certificate attributes to return the attribute
 * names contained within them.
 *
 * @author Amit Kapoor
 * @author Hemma Prafullchandra
 * @version 1.9
 */
public class AttributeNameEnumeration extends Vector {
    /**
     * The default constructor for this class.
     */
    public AttributeNameEnumeration() {
        super(1,1);
    }
}
