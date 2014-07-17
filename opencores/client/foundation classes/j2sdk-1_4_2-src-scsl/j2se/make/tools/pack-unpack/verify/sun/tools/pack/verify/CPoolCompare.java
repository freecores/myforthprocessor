/*
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * @(#)CPoolCompare.java	1.4 03/01/23
 */
package sun.tools.pack.verify;

import java.io.*;
import java.util.*;

import com.sun.classinfo.*;
import com.sun.classinfo.classfile.*;
import com.sun.classinfo.classfile.exceptions.*;
import com.sun.classinfo.classfile.constants.*;

public class CPoolCompare {
        
    public static String getCPConstantCompareString(Object o) {
	try {
	    switch (((Constant)o).getConstantType()) {
		
		case Constant.ctInt_Const_Class:
		    return "CPool:Const_Class:" + (String)((ConstantClass)o).getName();
		    
		case Constant.ctInt_Const_Double:
		    return "CPool:Const_Double:" + Double.toString(((ConstantDouble)o).getValue());
		    
		case Constant.ctInt_Const_FieldRef:
		    return "CPool:Const_FieldRef:" + ((ConstantFieldRef)o).getFieldName() + " " +((ConstantFieldRef)o).getTypeString();
		    
		case Constant.ctInt_Const_Float:
		    return "CPool:Const_Float:" + Float.toString(((ConstantFloat)o).getValue());
		    
		case Constant.ctInt_Const_Integer:
		    return "CPool:Const_Integer:" + Integer.toString(((ConstantInteger)o).getValue());
		    
		case Constant.ctInt_Const_InterfaceMethodRef:
		    return "CPool:Const_InterfaceMethodRef:" +  ((ConstantInterfaceMethodRef)o).getSignatureString() + " " + ((ConstantInterfaceMethodRef)o).getInterfaceName() ;
		    
		case Constant.ctInt_Const_Long:
		    return "CPool:Const_Long:" + Long.toString(((ConstantLong)o).getValue());
		    
		case Constant.ctInt_Const_MethodRef:
		    return "CPool:Const_MethodRef:" + ((ConstantMethodRef)o).getSignatureString() + " " + ((ConstantMethodRef)o).getMethodName();
		    
		case Constant.ctInt_Const_NameAndType:
		    return "CPool:Const_NameAndType:" + ((ConstantNameAndType)o).getSignatureString() + " " + ((ConstantNameAndType)o).getName();
		    
		case Constant.ctInt_Const_String:
		    return "CPool:Const_String:" + ((ConstantString)o).getValue();
		    
		case Constant.ctInt_Const_Utf8:
		    return "CPool:Const_Utf8:" + ((ConstantUtf8)o).getValue();
		    
		default:
		    Globals.log("CPool:Error:Unknown class constant type");
		    return "Error!!!";
	    }
	} catch (ClassFileException cfe) {
	    cfe.printStackTrace();
	}
	return null;
    }
    
    class CPoolComparator implements Comparator {
	public int compare (Object o1, Object o2) {
	    return getCPConstantCompareString(o1).compareTo(getCPConstantCompareString(o2));
	}
    }
    

    private VerifyTreeSet getCPoolVTSet(ClassFile c) {
	VerifyTreeSet _vts = new VerifyTreeSet(new CPoolComparator());
	try {
	    for (Iterator i = (c.getConstantPool()).getConstantPoolIterator(); i.hasNext(); ) {
		_vts.add((Constant) i.next());
	    }
	} catch (ClassFileException cfe) {
	    cfe.printStackTrace();
	}
	return _vts;
    }
    
    private void reportContents(TreeSet s, String prepend) {
	for (Iterator is = s.iterator(); is.hasNext();) {
	    Globals.log(prepend + "Level2:" + getCPConstantCompareString((Constant)is.next()));
	}
    }
    
    public void cpoolDump(ClassFile c) {
	try {
	    ConstantPool cp = c.getConstantPool();
	    int cpool_entries = cp.getCount();
	    for (int i = 1 ; i < cpool_entries ; i++) {
		System.out.print("cpool["+i+"] = ");
	        Constant ct = cp.get(i);
		System.out.println(getCPConstantCompareString(ct));
	    }
	} catch (ClassFileException cfe) {
	    cfe.printStackTrace();
	}

    }
    
    public void doCPoolDiffs(ClassFile c1, ClassFile c2) {
	
	VerifyTreeSet c1_set = getCPoolVTSet(c1);
	VerifyTreeSet c2_set = getCPoolVTSet(c2);
	
	TreeSet delta;
	delta = c1_set.diff(c2_set);
	if (!delta.isEmpty()) reportContents(delta,"");

	delta = c2_set.diff(c1_set);
	if (!c2_set.isEmpty()) reportContents(delta,"\t");

	return ;
    }	
}

