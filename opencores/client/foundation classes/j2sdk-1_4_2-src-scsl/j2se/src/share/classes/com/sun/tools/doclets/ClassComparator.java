/*
 * @(#)ClassComparator.java	1.11 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.tools.doclets;

import com.sun.javadoc.*;
import java.util.*;

/**
 * Comparator to be used to compare the class names.
 */
public class ClassComparator implements Comparator {

    /**
     * Compare the classnames of classes represented by two
     * {@link com.sun.javadoc.ClassDoc} objects.
     *
     * @param   o1 ClassDoc object1
     * @param   o2 ClassDoc object2
     * @return  Return the value returned by {@link java.lang.String#compareTo}
     *          method from java.lang.String class.
     */
    public int compare(Object o1, Object o2) {
        return (((ClassDoc)o1).name())
            .compareTo(((ClassDoc)o2).name());
    }
}
