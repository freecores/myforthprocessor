/*
 * @(#)SPARCV9RegisterBranchInstruction.java	1.3 03/01/23 11:19:50
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.asm.sparc;

import sun.jvm.hotspot.asm.*;

public class SPARCV9RegisterBranchInstruction extends SPARCInstruction
    implements SPARCV9Instruction, BranchInstruction {
    final protected PCRelativeAddress addr;
    final protected boolean isAnnuled;
    final protected int regConditionCode;
    final protected SPARCRegister conditionRegister;
    final protected boolean predictTaken;

    public SPARCV9RegisterBranchInstruction(String name, PCRelativeAddress addr, 
                               boolean isAnnuled, int regConditionCode, 
                               SPARCRegister conditionRegister, boolean predictTaken) {
        super(name);
        this.addr = addr;
        this.isAnnuled = isAnnuled;
        this.regConditionCode = regConditionCode;
        this.conditionRegister = conditionRegister;
        this.predictTaken = predictTaken;
    }

    public String asString(long currentPc, SymbolFinder symFinder) {
        long address = addr.getDisplacement() + currentPc;
        StringBuffer buf = new StringBuffer();
        buf.append(getName());
        buf.append(spaces);
        buf.append(symFinder.getSymbolFor(address));
        return buf.toString();
    }

    public boolean isBranch() {
        return true;
    }

    public Address getBranchDestination() {
        return addr;
    }

    public boolean isAnnuledBranch() {
        return isAnnuled;
    }

    public boolean isConditional() {
        return true;
    }

    public int getRegisterConditionCode() {
        return regConditionCode;
    }

    public SPARCRegister getConditionRegister() {
        return conditionRegister;
    }

    public boolean getPredictTaken() {
        return predictTaken;
    }
}
