/*
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * @(#)MethodCodeCompare.java	1.5 03/01/23
 */
package sun.tools.pack.verify;

import java.util.*;

import com.sun.classinfo.classfile.*;
import com.sun.classinfo.classfile.exceptions.*;
import com.sun.classinfo.classfile.attributes.*;
import com.sun.classinfo.classfile.instructions.*;
import com.sun.classinfo.classfile.constants.*;

/**
 * Here we compare details of Method Code and Exceptions.
 */

public class MethodCodeCompare {
    
    static final String LOG_PREAMBLE = "Level3:Method:Code:";
    static final String LOG_EXC_PREAMBLE = "Level3:Method:Exception:";
    
    private HashSet _usedCP1 = new HashSet();
    private HashSet _usedCP2 = new HashSet();
    
    // Inner classe to help with doing the diffs of exceptions
    class VerifyExceptionVector extends Vector {
	Method _m;
	
	void initVerifyExceptionVector(Method m) {
	    _m = m;
	    this.clear();
	}
	
	boolean addExceptionHandlerInfo(ExceptionHandlers es, HashSet cp_usage) {
	    int i = 0;
	    for (Iterator is_iter = es.getIterator(); is_iter.hasNext();) {
		try {
		    ExceptionHandlerInfo ehi = (ExceptionHandlerInfo)is_iter.next();
		    // Finalizers may be set to 0 ignore the names of these.
		    // However we do want to count them
		    if ((ehi.getCatchType()).getRenderedIndex() == 0) {
			this.add("Finalizer." + i);
			i++;
		    } else {
			cp_usage.add(new Integer((ehi.getCatchType()).getRenderedIndex()));
			this.add(ehi.getCatchTypeName());
		    }
		} catch (ClassFileException cfe) {
		    cfe.printStackTrace();
		}
	    }
	    return false;
	}
	
	void reportDetailExceptionHandlerDiff(Vector that) {
	    try {
		if (this.size() == that.size()) {
		    if (this.size() > 0) {
			for (int i = 0 ; i < this.size() ; i++) {
			    if (((String)this.get(i)).compareTo((String)that.get(i))!=0) {
				Globals.log(LOG_EXC_PREAMBLE + "Method:" + _m.getName());
				Globals.log((String)this.get(i));
				Globals.log("\t->" + (String)this.get(i));
			    }
			}
		    }
		} else {
		    Globals.log(_m.getName() + ":Exception differ: " + this.size() + "<>" + that.size());
		}
	    } catch (ClassFileException cfe) {
		cfe.printStackTrace();
	    }
	}
    }
   
    /**
     * Mostly used for debugg to dump an Entire Method
     */
    public void dumpMethod(Method[] marray) {
	try {
	    for (int i = 0 ; i < marray.length ; i++) {
		Globals.println("Method " + marray[i].getSignatureString() + " " + marray[i].getName());
		AttrCode ac = marray[i].getAttributes().getAttrCode();
		if (ac == null) continue ;
		Instructions instrs = ac.getInstructions();
		instrs.resolveJumpTargets();
		for (Iterator insIter = instrs.getIterator(); insIter.hasNext();) {
		    Instruction ins = (Instruction) insIter.next();
		    Globals.println("\t" + ins.getInstrIndex() + " " + ins.toString().trim());
		}
	    }
	} catch (ClassFileException cfe) {
	    cfe.printStackTrace();
	}
    }

    /**
     * Given two identical arrays of methods, we do a diff on the
     * instructions and print it to the logger.
     */
    
    public void doMethodCodeDiffs(Method[] marray1, Method[] marray2) {
	VerifyExceptionVector vev1 = new VerifyExceptionVector();
	VerifyExceptionVector vev2 = new VerifyExceptionVector();
	try {
	    for (int i = 0 ; i < marray1.length ; i++) {
		AttrCode ac1 = marray1[i].getAttributes().getAttrCode();
		AttrCode ac2 = marray2[i].getAttributes().getAttrCode();
		
		if (ac1 != null && ac2 != null) {
		    Instructions instrs1 = ac1.getInstructions();
		    Instructions instrs2 = ac2.getInstructions();
		    
		    instrs1.resolveJumpTargets();
		    instrs2.resolveJumpTargets();
		    
		    Instruction[] iarray2 = new Instruction[instrs2.getCount()];
		    
		    int count = 0 ;
		    for (Iterator insIter = instrs2.getIterator(); insIter.hasNext(); ) {
			iarray2[count] = (Instruction)insIter.next();
			count++;
		    }
		    
		    count = 0;
		    MethodInstructionVisitor miv = new MethodInstructionVisitor(marray1[i],iarray2, instrs1, instrs2);
		    for (Iterator insIter = instrs1.getIterator(); insIter.hasNext(); count++) {
			Instruction ins = (Instruction) insIter.next();
			miv.visitInstruction(ins, count);
		    }
		    
		    // Exceptions for each method
		    vev1.initVerifyExceptionVector(marray1[i]);
		    vev2.initVerifyExceptionVector(marray1[i]);

		    vev1.addExceptionHandlerInfo(ac1.getExceptionHandlers(), _usedCP1);
		    vev2.addExceptionHandlerInfo(ac2.getExceptionHandlers(), _usedCP2);
		    
		    vev1.reportDetailExceptionHandlerDiff(vev2);
		}
	    }
	} catch (ClassFileException cfe) {
	    cfe.printStackTrace();
	}
    }
    
    /**
     * Gets a HashSet of all the used Constant Pool Entries
     * of Jar 1.
     */
    HashSet getUsedCPoolIndices1() {
	return _usedCP1;
    }
    
     /**
     * Gets a HashSet of all the used Constant Pool Entries
     * of Jar 2.
     */
    HashSet getUsedCPoolIndices2() {
	return _usedCP2;
    }
}

