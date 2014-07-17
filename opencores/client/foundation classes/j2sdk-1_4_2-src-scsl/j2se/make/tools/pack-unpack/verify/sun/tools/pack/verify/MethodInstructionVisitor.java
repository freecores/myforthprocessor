/*
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * @(#)MethodInstructionVisitor.java	1.8 03/01/23
 */
package sun.tools.pack.verify;

import java.util.*;

import com.sun.classinfo.classfile.*;
import com.sun.classinfo.classfile.exceptions.*;
import com.sun.classinfo.classfile.attributes.*;
import com.sun.classinfo.classfile.instructions.*;
import com.sun.classinfo.classfile.constants.*;

/**
 * This class does most of the work of Comparing methods
 */

class MethodInstructionVisitor extends InstructionVisitor {
    private Method _m;
    private Instruction _thatInstructions[];
    private int _instructionIndex;
    private Instructions _is1;
    private Instructions _is2;
    
    public MethodInstructionVisitor(Method m, Instruction[] instrs, Instructions is1, Instructions is2) {
	super();
	_m = m;
	_thatInstructions = instrs;
	_is1 = is1;
	_is2 = is2;
    }
  
    
    public void visitInstruction  (Instruction instruction, int index) throws ClassFileException {
	_instructionIndex = index ;
	visit(instruction);
    }
    
    public void visitInstrByteValue(InstrByteValue ins) throws ClassFileException {
	InstrByteValue thatIns = (InstrByteValue)_thatInstructions[_instructionIndex];
	if (mnemonicCompare(ins,thatIns)) {
	    if (ins.getByteValue() == thatIns.getByteValue()) return ;
	}
	logDiff(ins, thatIns);
    }
    
    public void visitInstrByteValue2(InstrByteValue2 ins) throws ClassFileException {
	Globals.println("InstrByteValue2:"+ins.getMnemonic());
	InstrByteValue2 thatIns = (InstrByteValue2)_thatInstructions[_instructionIndex];
	if (mnemonicCompare(ins, thatIns)) {
	    if ( (ins.getFirstValue() == thatIns.getFirstValue()) &&
		    (ins.getSecondValue() == thatIns.getSecondValue()) ) {
		return ;
	    }
	}
	logDiff(ins, thatIns);
    }
    
    public void visitInstrCpByteRef(InstrCpByteRef ins) throws ClassFileException {
	Globals.println("InstrCpByteRef:"+ins.getMnemonic());
	Instruction thatIns = _thatInstructions[_instructionIndex];
	String s1 = CPoolCompare.getCPConstantCompareString(ins.getConstant());
	String s2 = null;
	// We treat ldc here and ldc_w there differently
	if (ins.getMnemonic().compareTo("ldc") == 0 &&  thatIns.getMnemonic().compareTo("ldc_w") == 0) {

	    s2 = CPoolCompare.getCPConstantCompareString(((InstrCpShortRef)thatIns).getConstant());
	    if (s1.compareTo(s2) == 0)  return;
	    
	} else if (mnemonicCompare(ins, thatIns)) {
	    s2 = CPoolCompare.getCPConstantCompareString(((InstrCpByteRef)thatIns).getConstant());
	    if (s1.compareTo(s2) == 0)  return;
	}
	logDiff(ins, thatIns, s1, s2);
    }
    
    
    public void visitInstrCpShortRef(InstrCpShortRef ins) throws ClassFileException {
	Globals.println("InstrCpShortRef:"+ins.getMnemonic());
	InstrCpShortRef thatIns = (InstrCpShortRef) _thatInstructions[_instructionIndex];
	if (mnemonicCompare(ins, thatIns)) {
	    String s1 = CPoolCompare.getCPConstantCompareString(((InstrCpShortRef)ins).getConstant());
	    String s2 = CPoolCompare.getCPConstantCompareString(((InstrCpShortRef)thatIns).getConstant());
	    if (s1.compareTo(s2) == 0) return;
	}
	logDiff(ins,thatIns);
    }
    
    public void visitInstrFieldRef(InstrFieldRef ins) throws ClassFileException {
	Globals.println("InstrFieldRef:"+ins.getMnemonic());
	InstrFieldRef thatIns = (InstrFieldRef) _thatInstructions[_instructionIndex];
	if (mnemonicCompare(ins, thatIns)) {
	    String s1 = ((InstrFieldRef)ins).getClassName() + " " + ((InstrFieldRef)ins).getTypeString()
		+ " " + ((InstrFieldRef)ins).getFieldName();
	    String s2 = ((InstrFieldRef)thatIns).getClassName() + " " + ((InstrFieldRef)thatIns).getTypeString()
		+ " " + ((InstrFieldRef)thatIns).getFieldName();
	    if (s1.compareTo(s2) == 0) return;
	}
	logDiff(ins,thatIns);
    }
    
    public void visitInstrInterfaceRef(InstrInterfaceRef ins) throws ClassFileException {
	Globals.println("InstrInterfaceRef:"+ins.getMnemonic());
	InstrInterfaceRef thatIns = (InstrInterfaceRef) _thatInstructions[_instructionIndex];
	if (mnemonicCompare(ins, thatIns)) {
	    String s1 = ((InstrInterfaceRef)ins).getClassName() + " " +
		((InstrInterfaceRef)ins).getInterfaceName() + " " +
		((InstrInterfaceRef)ins).getSignatureString();
		
	    String s2 = ((InstrInterfaceRef)thatIns).getClassName() + " " +
		((InstrInterfaceRef)thatIns).getInterfaceName() + " " +
		((InstrInterfaceRef)thatIns).getSignatureString();

	    int n1 = ((InstrInterfaceRef)ins).getNumberOfArguments();
	    int n2 = ((InstrInterfaceRef)thatIns).getNumberOfArguments();
	    
	    boolean printLogDiff = false;
	    
	    if (s1.compareTo(s2) != 0) {
		printLogDiff = true;
	    }

	    if (n1 != n2) {
		Globals.log("InstrInterfaceRef: args count does not match " + n1 + "<>" + n2);
		printLogDiff = true;
	    }
	    
	    if (printLogDiff) {
		logDiff(ins, thatIns);
	    }
	}
    }
    
    public void visitInstrIntLabel(InstrIntLabel ins) throws ClassFileException {
	// Since Pack sometimes replaces ldc with ldc_w therefore the jump target
	// changes, so we resolve the target and compare the instruction mnemonic.
	// This may not be 100% fool proof.
	Globals.println("InstrIntLabel:"+ins.getMnemonic());
	InstrIntLabel thatIns = (InstrIntLabel)_thatInstructions[_instructionIndex];
	if (mnemonicCompare(ins, thatIns)) {
	    InstrIndex idx1 = ins.getJumpTarget();
	    InstrIndex idx2 = thatIns.getJumpTarget();
	    if (_is1.get(idx1).getMnemonic().compareTo(_is2.get(idx1).getMnemonic()) == 0) return;
	}
	logDiff(ins,thatIns);
    }
    
    public void visitInstrLookupSwitch(InstrLookupSwitch ins) throws ClassFileException {
	/*
	 1. Compare the default case.
	 2. compare the number of cases.
	 3. compare the values of the cases
	 4. compare the jumptarget of each of the cases to the instruction jumped to.
	 (Index will not match!)
	 */
	Globals.println("InstrLookupSwitch:"+ins.getMnemonic());
	InstrLookupSwitch thatIns = (InstrLookupSwitch)_thatInstructions[_instructionIndex];
	
	int nCaseValues1[] = ins.getCaseValues();
	int nCaseValues2[] = thatIns.getCaseValues();
	
	Instruction i1 = _is1.get(ins.getDefaultIndex());
	Instruction i2 = _is2.get(thatIns.getDefaultIndex());
	
	boolean printLogDiff = false;
	
	if (!mnemonicCompare(i1, i2)) {  // Default cases match
	    Globals.log("Default case: jump instruction does not match");
	    printLogDiff = true;
	}
	
	if (nCaseValues1.length != nCaseValues2.length) {
	    Globals.log("Number of Cases different:" + nCaseValues1.length + " <> " + nCaseValues2.length);
	    printLogDiff = true;
	}

	for (int i = 0 ; i < nCaseValues1.length ; i++) {
	    if (nCaseValues1[i] != nCaseValues2[i]) {
		printLogDiff = true;
		Globals.log("Case value does not match:" +
				nCaseValues1[i] + " <> " + nCaseValues2[i]);
	    }
	    i1 = _is1.get(ins.getCaseIndex(i));
	    i2 = _is1.get(thatIns.getCaseIndex(i));
	    if (!mnemonicCompare(i1,i2)) {
		printLogDiff = true;
		Globals.log("Case jump instruction does not match for " + nCaseValues1[i] +
				" " + i1.getMnemonic() + " <> " + i2.getMnemonic());
	    }
	    // Nothing more to do or fall through.

	    if (i == nCaseValues1.length -1 && printLogDiff == false) return;
	}
	if (printLogDiff) logDiff(ins,thatIns);
    }
    
    public void visitInstrMethodRef(InstrMethodRef ins) throws ClassFileException {
	Globals.println("InstrMethodRef:"+ins.getMnemonic());
	InstrMethodRef thatIns = (InstrMethodRef) _thatInstructions[_instructionIndex];
	if (mnemonicCompare(ins, thatIns)) {
	    String s1 = ((InstrMethodRef)ins).getClassName() + " " +
		((InstrMethodRef)ins).getMethodName() + " " +
		((InstrMethodRef)ins).getSignatureString();
	    String s2 = ((InstrMethodRef)thatIns).getClassName() + " " +
		((InstrMethodRef)thatIns).getMethodName() + " " +
		((InstrMethodRef)thatIns).getSignatureString();
	    if (s1.compareTo(s2) == 0) return;
	}
	logDiff(ins,thatIns);
    }
    
    public void visitInstrMultiArray(InstrMultiArray ins) throws ClassFileException {
	Globals.println("InstrMultiArray:"+ins.getMnemonic());
	InstrMultiArray thatIns = (InstrMultiArray)_thatInstructions[_instructionIndex];
	if (mnemonicCompare(ins,thatIns)) {
	    if (ins.getDimensions() == thatIns.getDimensions()) return ;
	}
	logDiff(ins, thatIns);
    }
    
    public void visitInstrShortLabel(InstrShortLabel ins) throws ClassFileException {
	Globals.println("InstrShortLabel:"+ins.getMnemonic());
	// Since Pack sometimes replaces ldc with ldc_w the jump target
	// changes, so we resolve the target and compare the instruction mnemonic.
	// This may not 100% fool proof.
	InstrShortLabel thatIns = (InstrShortLabel)_thatInstructions[_instructionIndex];
	if (mnemonicCompare(ins, thatIns)) {
	    InstrIndex idx1 = ins.getJumpTarget();
	    InstrIndex idx2 = thatIns.getJumpTarget();
	    if (_is1.get(idx1).getMnemonic().compareTo(_is2.get(idx1).getMnemonic()) == 0) return;
	}
	logDiff(ins,thatIns);
    }
    
    public void visitInstrShortValue(InstrShortValue ins) throws ClassFileException {
	Globals.println("InstrShortValue:"+ins.getMnemonic());
	InstrShortValue thatIns = (InstrShortValue)_thatInstructions[_instructionIndex];
	if (mnemonicCompare(ins,thatIns)) {
	    if (ins.getShortValue() == thatIns.getShortValue()) return ;
	}
	logDiff(ins, thatIns);
    }
    
    public void visitInstrTableSwitch(InstrTableSwitch ins) throws ClassFileException {
	Globals.println("InstrTableSwitch:"+ins.getMnemonic());
	/*
	 1. Compare the default case.
	 2. compare the number of cases.
	 3. compare the values of the cases
	 4. compare the jumptarget of each of the cases to the instruction jumped to.
	 (Index will not match!)
	 */
	Globals.println("InstrTableSwitch:"+ins.getMnemonic());
	InstrTableSwitch thatIns = (InstrTableSwitch)_thatInstructions[_instructionIndex];
	
	InstrIndex[] indexs1 = ins.getIndexes();
	InstrIndex[] indexs2 = thatIns.getIndexes();
	
	Instruction i1 = _is1.get(ins.getDefaultIndex());
	Instruction i2 = _is2.get(thatIns.getDefaultIndex());
	
	boolean printLogDiff = false;
	
	if (!mnemonicCompare(i1, i2)) {  // Default cases match
	    Globals.log("Default case: jump instruction does not match" +
			    i1.getMnemonic() + " <> " + i2.getMnemonic());
	    printLogDiff = true;
	}
	
	if (indexs1.length != indexs2.length) {
	    Globals.log("Index numbers different:" + indexs1.length + " <> " + indexs2.length);
	    printLogDiff = true;
	}
	
	for (int i = 0 ; i < indexs1.length ; i++) {
	    i1 = _is1.get(indexs1[i]);
	    i2 = _is1.get(indexs2[i]);
	    if (!mnemonicCompare(i1,i2)) {
		printLogDiff = true;
		Globals.log("Index jump instruction does not match for " + indexs1[i] +
				" " + i1.getMnemonic() + " <> " + i2.getMnemonic());
	    }
	    // Nothing more to do return or fall through to print diffs.
	    if (i == indexs1.length -1 && printLogDiff == false) return;
	}
	logDiff(ins,thatIns);
    }
    
    public void visitInstrWide(InstrWide ins) throws ClassFileException {
	Globals.println("InstrWide:"+ins.getMnemonic());
	InstrWide thatIns = (InstrWide)_thatInstructions[_instructionIndex];
	if (mnemonicCompare(ins,thatIns)) {
	    if ((ins.getConstant() == thatIns.getConstant()) &&
		    (ins.getIndex() == thatIns.getIndex()) &&
		    (ins.getModifiedInstruction() == thatIns.getModifiedInstruction()))
		return;
	}
	logDiff(ins, thatIns);
    }
    
    private void logDiff(Instruction i1, Instruction i2, String s1, String s2) throws ClassFileException {
	logDiff(i1, i2);
	Globals.log("Diff encountered at: " + s1.compareTo(s2));
	Globals.diffCharData(s1,s2);
	Globals.dumpToHex(s1);
	Globals.dumpToHex(s2);
    }
    
    
    private void logDiff(Instruction i1, Instruction i2) throws ClassFileException {
	Globals.log(MethodCodeCompare.LOG_PREAMBLE + ":" + _m.getSignatureString() + " " +
			_m.getName() + ":" + i1.getInstrIndex() + " " + i1.toString().trim()
			+ " <> " + i2.getInstrIndex() + " " +  i2.toString().trim() );
    }
    
    private boolean mnemonicCompare(Instruction i1, Instruction i2) throws ClassFileException {
	String iMStr1 = i1.getMnemonic();
	String iMStr2 = i2.getMnemonic();
	return (iMStr1.compareTo("ldc") == 0) ?
	    (((iMStr1.compareTo(iMStr2) == 0) || (iMStr2.compareTo("ldc_w") == 0))) :
	    iMStr1.compareTo(iMStr2) == 0 ;
    }
}





