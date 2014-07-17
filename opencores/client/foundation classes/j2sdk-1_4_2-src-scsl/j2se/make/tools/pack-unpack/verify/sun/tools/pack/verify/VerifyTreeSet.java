/*
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * @(#)VerifyTreeSet.java	1.4 03/01/23
 */
package sun.tools.pack.verify;

import java.util.*;

public class VerifyTreeSet extends java.util.TreeSet {
    
    public VerifyTreeSet() {
	super();
    }
    
    public void addWithIterator (Iterator i) {
	while (i.hasNext()) {
	    add(i.next());
	}
    }
    
    public VerifyTreeSet(Comparator c) {
	super(c);
    }
    
    public TreeSet diff(TreeSet in) {
	TreeSet delta = (TreeSet) this.clone();
	delta.removeAll(in);
	return delta;
    }
}

