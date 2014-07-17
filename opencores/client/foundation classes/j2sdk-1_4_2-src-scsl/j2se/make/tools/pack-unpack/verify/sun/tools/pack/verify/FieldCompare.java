/*
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * @(#)FieldCompare.java	1.5 03/01/23
 */

package sun.tools.pack.verify;

import java.io.*;
import java.util.*;

import com.sun.classinfo.*;
import com.sun.classinfo.parser.*;
import com.sun.classinfo.validator.*;
import com.sun.classinfo.classfile.*;
import com.sun.classinfo.classfile.exceptions.*;

public class FieldCompare {
    
    private String getCompareString(Object o) {
	try { 
	    return AccessFlags.fieldFlagsToString(((Field)o).getAccessFlags()) + " " 
		+ ((Field)o).getType() + " " + ((Field)o).getName();
	} catch (ClassFileException cfe) {
	    cfe.printStackTrace();
	}
	return null;
    }
    
    class FieldComparator implements Comparator {
	public int compare (Object o1, Object o2) {
	    return getCompareString(o1).compareTo(getCompareString(o2));
	}
    }

    private VerifyTreeSet getVerifyTreeSet(ClassFile c) {
	VerifyTreeSet out = new VerifyTreeSet(new FieldComparator());
	try {
	    Fields fs = c.getFields();
	    for (Iterator iter = fs.getIterator(); iter.hasNext(); ) {
		out.add((Field)iter.next());
	    }
	} catch (Exception e) {
	    e.printStackTrace();
	}	
	return out;
    }
    private void reportContents(TreeSet s, String prepend) {
	for (Iterator is = s.iterator(); is.hasNext();) {
	    Globals.log(prepend + "Level2:Field:" + getCompareString(is.next()));
	}
    }
    
    public void doFieldDiffs(ClassFile c1, ClassFile c2) {
	VerifyTreeSet h1 = getVerifyTreeSet(c1);
	VerifyTreeSet h2 = getVerifyTreeSet(c2);

	TreeSet delta = h1.diff(h2);
	if (!delta.isEmpty()) reportContents(delta,"");
	
	delta = h2.diff(h1);
	if (!h2.isEmpty()) reportContents(delta,"\t");
    }
}

