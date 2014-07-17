/*
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * @(#)MethodCompare.java	1.7 03/01/23
 */
package sun.tools.pack.verify;

import java.util.*;

import com.sun.classinfo.classfile.*;
import com.sun.classinfo.classfile.exceptions.*;
import com.sun.classinfo.classfile.attributes.*;
import com.sun.classinfo.classfile.constants.*;
/**
 * Compares the methods in the two classes we do the following:
 * 1. Compare the number of methods in each classes and
	A. Method Name
	B. Method Signature
	C. Method Access Flags.
	D. The Stack size of each method
	E. The number of locals.
	F. The code is compared.
 */

public class MethodCompare {
 
    private static final String LOG_ATTRCODE_PREAMBLE = "Level2:Method:AttrCode:";
    private static final String LOG_ATTREXC_PREAMBLE = "Level2:Method:Exception:";
    
    private static String getMethodExceptions(Method m) throws ClassFileException{
	SortedSet sort = new VerifyTreeSet();
	StringBuffer out = new StringBuffer("");
	AttrExceptions ae = m.getAttributes().getAttrExceptions();
	if (ae != null) {
	    out = out.append(" throws ");
	    for (Iterator aei = ae.getExceptionIterator(); aei.hasNext();) {
		ConstantClass cc = (ConstantClass)aei.next();
		sort.add(cc.getName());
	    }
	    
	    for (Iterator i = sort.iterator(); i.hasNext();) {
		out = out.append((String) i.next());
		if (i.hasNext()) out = out.append(" ,");
	    }
	}
	return new String(out);
    }
    private String getMethodCompareString(Object o) {
	try {
	    return (o instanceof Method) ?
		((Method) o).getName() + " " + ((Method)o).getSignatureString()
		+ " " + AccessFlags.fieldFlagsToString(((Method)o).getAccessFlags())
		+ " " + getMethodExceptions((Method)o) : null;
	    
	} catch (ClassFileException cfe) {
	    cfe.printStackTrace();
	}
	return null;
    }
    
    private class MethodComparator implements Comparator {
	public int compare (Object o1, Object o2) {
	    return getMethodCompareString(o1).compareTo(getMethodCompareString(o2)) ;
	}
    }
    
    private VerifyTreeSet getMethodsVTSet(ClassFile c) {
	VerifyTreeSet out = new VerifyTreeSet(new MethodComparator());
	try {
	    Methods ms = c.getMethods();
	    for (Iterator iter = ms.getIterator(); iter.hasNext(); ) {
		out.add((Method)iter.next());
	    }
	} catch (Exception e) {
	    e.printStackTrace();
	}
	return out;
    }
 
    private void reportMethodContents(TreeSet s, String prepend) {
	for (Iterator is = s.iterator(); is.hasNext();) {
	    Globals.log(prepend + "Level2:Method:" + getMethodCompareString((Method)is.next()));
	}
    }
    
    private void doAttrCodeDiff(Method[] marray1, Method[] marray2) {
	for (int i = 0 ; i < marray1.length ; i++) {
	    try {
		AttrCode ac1 = (marray1[i].getAttributes()).getAttrCode();
		AttrCode ac2 = (marray2[i].getAttributes()).getAttrCode();
		if (ac1 != null && ac2 != null) {
		    int ssize1 = ac1.getMaxStackSize();
		    int ssize2 = ac2.getMaxStackSize();
		    if (ssize1 != ssize2)
			Globals.log(LOG_ATTRCODE_PREAMBLE + "stack size:" +
					marray1[i].getName() + " " + ssize1 + "<>" + ssize2);
		     
		    int locals1 = ac1.getNumberOfLocals();
		    int locals2 = ac2.getNumberOfLocals();
		    if (locals1 != locals2) {
			Globals.log(LOG_ATTRCODE_PREAMBLE + "locals#:" +
					marray1[i].getName() + " " + locals1 + "<>" + locals2);
		    }
		}
	    } catch (ClassFileException cfe) {
		cfe.printStackTrace();
	    }
	}
    }
 
    public void doMethodDiffs(ClassFile c1, ClassFile c2) {
	
	// Basic Compare of signature, name etc.
	VerifyTreeSet m1 = getMethodsVTSet(c1);
	VerifyTreeSet m2 = getMethodsVTSet(c2);
	
	TreeSet delta1 = m1.diff(m2);
	if (!delta1.isEmpty()) reportMethodContents(delta1,"");
	
	TreeSet delta2 = m2.diff(m1);
	if (!delta2.isEmpty()) reportMethodContents(delta2,"\t");

	// Detailed Compare attributes, mainly AttrCode, and Exception
	// Ensure that there are no method diffs.
	if (delta1.isEmpty() && delta2.isEmpty()) {
	    Method[] marray1 = new Method[m1.size()];
	    Method[] marray2 = new Method[m2.size()];
	    m1.toArray(marray1);
	    m2.toArray(marray2);
	    
	    doAttrCodeDiff(marray1, marray2);
	    MethodCodeCompare mcc = new MethodCodeCompare();
	    mcc.doMethodCodeDiffs(marray1, marray2);
	    mcc.dumpMethod(marray1);
	}
    }
}


