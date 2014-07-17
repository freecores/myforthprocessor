/*
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * @(#)ClassAttributeCompare.java	1.5 03/01/23
 */

package sun.tools.pack.verify;

import java.util.*;

import com.sun.classinfo.classfile.*;
import com.sun.classinfo.classfile.exceptions.*;
import com.sun.classinfo.classfile.attributes.*;
import com.sun.classinfo.classfile.constants .*;


/**
 * Compares the Class Attributes of two classes.
 * The Class Attributes we interested in.
 * 	Fixed size attributes:
 *		AttrDeprecated
 * 		AttrSourceFile
 * 		AttrSynthetic
 *	Variable Length:
 *		AttrInnerClasses
 **/

public class ClassAttributeCompare {
    CPoolRef _ref;
    
    ClassAttributeCompare(CPoolRef ref) {
	_ref = ref;
    }

    private String getCompareString(Object o) {
	try {
	    if (o instanceof Attribute) {
		switch (((Attribute)o).getType()) {
		    case Attribute.ctInt_Attr_Deprecated:
			return ((AttrDeprecated)o).getTypeName();
		    case Attribute.ctInt_Attr_SourceFile:
			return ((AttrSourceFile)o).getTypeName() + " " + ((AttrSourceFile)o).getFilename();
		    case Attribute.ctInt_Attr_Synthetic:
			return ((AttrSynthetic)o).getTypeName();
		}
	    } else if (o instanceof InnerClassInfo) {
		return "InnerClass:" + ((InnerClassInfo) o).getInnerClassConstant().getName() + " " 
		    + AccessFlags.innerClassFlagsToString(((InnerClassInfo) o).getAccessFlags());
	    }
	} catch (ClassFileException cfe) {
	    cfe.printStackTrace();
	}
	return null;
    }
    
    class ClassAttributeComparator implements Comparator {
	public int compare (Object o1, Object o2) {
	    return getCompareString(o1).compareTo(getCompareString(o2));
	}
    }
    
    class ClassAttributeVisitor extends AttributeVisitor {
	VerifyTreeSet _fixedSet;

	ClassAttributeVisitor() {
	    super();
	}
	
	public void init() {
	    _fixedSet = new VerifyTreeSet(new ClassAttributeComparator());	    
	}
	
	public  void visitAttrSourceFile(AttrSourceFile sourceFileAttrib) throws ClassFileException {
	    if (!Globals.ignoreKnownAttributes()) {
	    	_fixedSet.add(sourceFileAttrib);
	    }
    	}
	
	public void visitAttrDeprecated(AttrDeprecated deprecatedAttrib) throws ClassFileException {
	    if (!Globals.ignoreKnownAttributes()) {
	    	_fixedSet.add(deprecatedAttrib);
	    }
	}

	public void visitAttrSynthetic(AttrSynthetic syntheticAttrib) throws ClassFileException {
	    if(!Globals.ignoreKnownAttributes()) {
	    	_fixedSet.add(syntheticAttrib);
	    }
	}
	
	public void visitAttrInnerClasses(AttrInnerClasses innerclassesAttrib) throws ClassFileException {
	    //System.err.println("InnerClasses#="+innerclassesAttrib.getCount());
	    for (Iterator is = innerclassesAttrib.getIterator(); is.hasNext();) {
		InnerClassInfo ici = (InnerClassInfo) is.next();
		try {
		    // HACK: force exceptions here, because there maybe
		    // some entries we cannot resolve we ignore these.
		    ici.getInnerClassName(); 
		    _fixedSet.add(ici);
		} catch (ClassFileException cfe) {
		    //cannot resolve the CPOOL we ignore these.
		}
	    }
	}
	
	VerifyTreeSet getFixedAttrSet() {
	    return _fixedSet;
	}

    }
    
    private void doClassAttributeSets(ClassFile c, ClassAttributeVisitor cav) {
	cav.init();
	try {
	    for (Iterator i = (c.getAttributes()).getIterator(); i.hasNext(); ) {
		Attribute a = (Attribute) i.next();
		cav.visit(a);
	    }
	} catch (ClassFileException cfe) {
	    cfe.printStackTrace();
	}
    }
    
    public void doAttributeDiffs(ClassFile c1, ClassFile c2) {
   
	ClassAttributeVisitor cav = new ClassAttributeVisitor();
	
	doClassAttributeSets(c1, cav);
	VerifyTreeSet c1_fixed = cav.getFixedAttrSet();

	
	doClassAttributeSets(c2, cav);
	VerifyTreeSet c2_fixed = cav.getFixedAttrSet();

	
	TreeSet delta;
	delta = c1_fixed.diff(c2_fixed);
	if (!delta.isEmpty()) {
	    reportFixedContents(delta,"");
	}
	delta = c2_fixed.diff(c1_fixed);
	if (!delta.isEmpty()) {
	    reportFixedContents(delta,"\t");
	}
	return ;
    }	
    
    private void reportFixedContents(TreeSet s, String prepend) {
	/* 
	 * We have to process this in reverse by doing so we process in
	 * natural ordering of the InnerClasses see fixed-point issue in
	 * ClassWriter.cpp ex: A$B$C we have to process C and then B.
	 */
	TreeSet tmp = (TreeSet) s.clone();
	InnerClassInfo ici = (InnerClassInfo) tmp.last();
	while (ici != null) {
	    try {
		CPIndex cpi = ici.getInnerClassConstant().getCPIndex();
		if ( (_ref._classfile.getThisClass() == ici.getOuterClassConstant()) ||
		   _ref.getRefCount(cpi) > 1 ) {
		    Globals.log(prepend + "Level2:Class:Attribute:" + getCompareString(ici));
		    Globals.log("RefCount = " + _ref.getRefCount(cpi));
		    Globals.log("Constant Class" + cpi + " " + cpi.getConstant() );
		    Globals.log(((InnerClassInfo)ici).toString());
		} else {
		    _ref._usedCPRefs.remove(ici.getInnerClassIndex());
		    _ref._usedCPRefs.remove(ici.getInnerClassNameIndex());
		    _ref._usedCPRefs.remove(ici.getOuterClassIndex());   
		}
	    } catch (ClassFileException cfe) {
		cfe.printStackTrace();
	    }
	    tmp.remove(ici);
	    ici = (tmp.size() >0) ? (InnerClassInfo) tmp.last():null;
	}
    }
}


