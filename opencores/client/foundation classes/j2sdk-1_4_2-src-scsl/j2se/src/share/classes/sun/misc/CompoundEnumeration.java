/*
 * @(#)CompoundEnumeration.java	1.6 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.misc;

import java.util.Enumeration;
import java.util.NoSuchElementException;

/*
 * A useful utility class that will enumerate over an array of
 * enumerations.
 */
public class CompoundEnumeration implements Enumeration {
    private Enumeration[] enums;
    private int index = 0;

    public CompoundEnumeration(Enumeration[] enums) {
	this.enums = enums;
    }

    private boolean next() {
	while (index < enums.length) {
	    if (enums[index] != null && enums[index].hasMoreElements()) {
		return true;
	    }
	    index++;
	}
	return false;
    }

    public boolean hasMoreElements() {
	return next();
    }

    public Object nextElement() {
	if (!next()) {
	    throw new NoSuchElementException();
	}
	return enums[index].nextElement();
    }
}
