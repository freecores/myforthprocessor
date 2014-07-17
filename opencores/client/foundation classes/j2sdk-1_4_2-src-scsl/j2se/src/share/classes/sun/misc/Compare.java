/*
 * @(#)Compare.java	1.11 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/**
 * Compare: an interface to enable users to define the result of
 * 	    a comparison of two objects.
 *
 * @version 1.11, 01/23/03
 * @author Sunita Mani
 */

package sun.misc;

public interface Compare {

    /**
     * doCompare
     *
     * @param  obj1 first object to compare.
     * @param  obj2 second object to compare.
     * @return -1 if obj1 < obj2, 0 if obj1 == obj2, 1 if obj1 > obj2.
     */
    public int doCompare(Object obj1, Object obj2);

}
