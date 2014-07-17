/*
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * @(#)CPoolRef.java	1.5 03/01/23
 */
package sun.tools.pack.verify;

import java.io.*;
import java.util.*;
import java.util.jar.*;

import com.sun.classinfo.*;
import com.sun.classinfo.classfile.*;
import com.sun.classinfo.classfile.exceptions.*;
import com.sun.classinfo.classfile.constants.*;
import com.sun.classinfo.classfile.attributes.*;
import com.sun.classinfo.classfile.instructions.*;


public class CPoolRef {
    CPHashtable _usedCPRefs ;
    ClassFile _classfile;
    
    CPoolRef(ClassFile c) {
	_usedCPRefs = new CPHashtable();
	_classfile = c;
    }
    
    class CPHashtable extends Hashtable {
	
	class CPoolIndexRef {
	    CPIndex cpidx;
	    int     refCount;
	    
	    CPoolIndexRef(CPIndex c) {
		cpidx = c;
		refCount = 1;
	    }
	}
	
	CPHashtable() {
	    super();
	}
	
	public boolean add(CPIndex cpi) {
	    try {
		// CPI Can be null constant and if innerclass is not a member
		if (cpi.getRenderedIndex() != 0) { 
		    CPoolIndexRef centry = (CPoolIndexRef) _usedCPRefs.get(cpi.toString());
		    if (centry == null) {
			CPoolIndexRef cir = new CPoolIndexRef(cpi);
			put(cir.cpidx.toString(),cir);
			return true;
		    } else {
			centry.refCount++;
			put(centry.cpidx.toString(), centry);
			return false;
		    }    
		}
	    }catch (ClassFileException cfe) {
		cfe.printStackTrace();
	    }
	    return false;
	}
	
	public void remove(CPIndex cpi) {
	    try {
		// CPI Can be null constant and if innerclass is not a member
		if (cpi.getRenderedIndex() != 0) { 
		    CPoolIndexRef centry = (CPoolIndexRef) _usedCPRefs.get(cpi.toString());
		    if (centry != null) {
			if (centry.refCount > 1) {
			    centry.refCount--;
			} else {
			    remove(centry.cpidx.toString());
			}
		    }
		}
	    }catch (ClassFileException cfe) {
		cfe.printStackTrace();
	    }	    
	}
	
	public int getRefCount(CPIndex cpi) {
	    CPoolIndexRef cir = (CPoolIndexRef) get(cpi.toString());
	    return cir.refCount;
	}
	
	class CPoolIndexComparator implements Comparator {
	    public int compare (Object o1, Object o2) {
		try {
		    return ((CPoolIndexRef)o1).cpidx.getRenderedIndex() -
			((CPoolIndexRef)o2).cpidx.getRenderedIndex();
		} catch (ClassFileException cfe) {
		    cfe.printStackTrace();
		    return -1;
		}
	    }
	}
	
	public String toString() {
	    ConstantPool cp;
	    String out = new String();
	    try {
		cp = _classfile.getConstantPool();
		TreeSet s = new TreeSet(new CPoolIndexComparator());
		s.addAll(_usedCPRefs.values());
		for (Iterator i = s.iterator();i.hasNext();) {
		    CPoolIndexRef cpr = ((CPoolIndexRef)i.next());
		    CPIndex cpi = cpr.cpidx;
		    out = out.concat("[" + cpi.getRenderedIndex() + " ]" + " ");
		    out = out.concat("[ " + ((CPoolIndexRef)_usedCPRefs.get(cpi.toString())).refCount + " ]\n");
		    out = out.concat(cpi.getConstant() + "\n");
		 
		}
	    } catch (ClassFileException cfe) {
		cfe.printStackTrace();
	    }
	    return out;
	}
    }	
    
    InstructionVisitor iVisitor = new InstructionVisitor() {
	
	public void visitInstrCpByteRef(InstrCpByteRef instr) throws ClassFileException {
	    _usedCPRefs.add(instr.getCPIndex());
	}
	
	public void visitInstrCpShortRef(InstrCpShortRef instr) throws ClassFileException {
	    _usedCPRefs.add(instr.getCPIndex());
	}
	
	public void visitInstrFieldRef(InstrFieldRef instr) throws ClassFileException {
	    _usedCPRefs.add(instr.getCPIndex());
	}
	
	public void visitInstrInterfaceRef(InstrInterfaceRef instr) throws ClassFileException {
	    _usedCPRefs.add(instr.getCPIndex());
	}	
	
	public void visitInstrMethodRef(InstrMethodRef instr) throws ClassFileException {
	    _usedCPRefs.add(instr.getCPIndex());
	}	
    };
    
    
    AttributeVisitor aVisitor = new AttributeVisitor() {
  
	public void visitAttrConstantValue(AttrConstantValue attrConstantValue) throws ClassFileException {
	    _usedCPRefs.add(attrConstantValue.getValueIndex());
	}
	
	public void visitAttrExceptions(AttrExceptions attrExceptions) throws ClassFileException {
	    for (Iterator eitr = attrExceptions.getIterator();eitr.hasNext();) {
		_usedCPRefs.add((CPIndex)eitr.next());
	    }
	}
	
	public void visitAttrLocalVariableTable(AttrLocalVariableTable attrLocalVariableTable) throws ClassFileException {
	    for (Iterator lvitr = attrLocalVariableTable.getIterator() ; lvitr.hasNext(); ) {
		LocalVariable lv = (LocalVariable) lvitr.next();
		_usedCPRefs.add(lv.getNameIndex());
		_usedCPRefs.add(lv.getTypeIndex());
	    }
	}
	
	public void visitAttrInnerClasses(AttrInnerClasses attrInnerClasses) throws ClassFileException {
	    for (Iterator icitr = attrInnerClasses.getIterator(); icitr.hasNext();) {
		InnerClassInfo ici = (InnerClassInfo) icitr.next();
		_usedCPRefs.add(ici.getInnerClassIndex());
		_usedCPRefs.add(ici.getInnerClassNameIndex());
		_usedCPRefs.add(ici.getOuterClassIndex());
	    }
	}
	
	public void visitAttrCode(AttrCode ac) throws ClassFileException {
	    com.sun.classinfo.classfile.attributes.Attributes as = ac.getAttributes();
	    for (Iterator ias = as.getIterator(); ias.hasNext(); ) {
		aVisitor.visit((Attribute)ias.next());
	    }
	    
	    for (Iterator isitr = ac.getInstructions().getIterator() ; isitr.hasNext(); ) {
		Instruction i = (Instruction)isitr.next();
		iVisitor.visit(i);
	    }
	    
	    for (Iterator i = ac.getExceptionHandlers().getIterator(); i.hasNext();) {
		ExceptionHandlerInfo eif = (ExceptionHandlerInfo) i.next();
		_usedCPRefs.add(eif.getCatchType());
	    }
	}
    };
    
    ConstantVisitor cVisitor = new ConstantVisitor() {
	
	public void visitConstantString(ConstantString c) throws ClassFileException {
	    _usedCPRefs.add(c.getIndex());  // Index to UTF8 string
	}
	
	public void visitConstantMethodRef(ConstantMethodRef c) throws ClassFileException {
	    _usedCPRefs.add(c.getClassIndex());	    // Index to Constant Class
	    visitConstantClass(c.getClassConstant());	    
	    visitConstantNameAndType(c.getNameAndTypeConstant());
	}
	
	public void visitConstantInterfaceMethodRef(ConstantInterfaceMethodRef c) throws ClassFileException {
	    _usedCPRefs.add(c.getClassIndex()); 	    // Index to Constant Class
	    visitConstantClass(c.getClassConstant());
	    visitConstantNameAndType(c.getNameAndTypeConstant());	
	}
	
	public void visitConstantFieldRef(ConstantFieldRef c) throws ClassFileException {
	    _usedCPRefs.add(c.getClassIndex());	    // Index to Constant Class
	    visitConstantClass(c.getClassConstant());	    
	    visitConstantNameAndType(c.getNameAndTypeConstant());
	}
	
	public void visitConstantClass(ConstantClass c) throws ClassFileException {
	    _usedCPRefs.add(c.getNameIndex());	    // Index to UTF8 string
	}
	
	public void visitConstantNameAndType(ConstantNameAndType c) throws ClassFileException {
	    _usedCPRefs.add(c.getNameIndex());		// Index to UTF8 string
	    _usedCPRefs.add(c.getSignatureIndex());	// Index to UTF8 string
	}
    };
    
    
    
    
    public void getAllCPReferences(ClassFile c) {
	try {
	    _usedCPRefs.add(c.getThisClassIndex());
	    _usedCPRefs.add(c.getSuperClassIndex());
	    
	    for (Iterator citr = c.getConstantPool().getConstantPoolIterator();citr.hasNext();) {
		cVisitor.visit( (Constant) citr.next());
	    }
	    
	    Interfaces ifs = c.getInterfaces();
	    for (Iterator iitr = ifs.getIterator(); iitr.hasNext(); ){
		Interface i = (Interface) iitr.next();
		_usedCPRefs.add(i.getClassIndex());		
	    }
	    
	    Fields fs = c.getFields();
	    for (Iterator fitr = fs.getIterator(); fitr.hasNext(); ) {
		Field f = (Field) fitr.next();
		_usedCPRefs.add(f.getNameIndex());
		_usedCPRefs.add(f.getTypeIndex());
		for (Iterator fasitr = f.getAttributes().getIterator(); fasitr.hasNext();){
		    aVisitor.visit( (Attribute) fasitr.next());
		}
	    }
	       
	    Methods ms = c.getMethods();
	    for (Iterator msitr = ms.getIterator(); msitr.hasNext(); ) {
		Method m = (Method) msitr.next();
		_usedCPRefs.add(m.getNameIndex());
		_usedCPRefs.add(m.getSignatureIndex());

		for (Iterator maitr = m.getAttributes().getIterator(); maitr.hasNext();){
		    Attribute a = (Attribute)maitr.next();
		    aVisitor.visit(a);
		}
	    }
	    
	    for (Iterator aitr = c.getAttributes().getIterator(); aitr.hasNext(); ){
		aVisitor.visit((Attribute)aitr.next());
	    }
	    
	} catch (ClassFileException cfe) {
	    cfe.printStackTrace();
	}
    }
  
    public int getRefCount(CPIndex cpi) {
	return _usedCPRefs.getRefCount(cpi);
    }
}

