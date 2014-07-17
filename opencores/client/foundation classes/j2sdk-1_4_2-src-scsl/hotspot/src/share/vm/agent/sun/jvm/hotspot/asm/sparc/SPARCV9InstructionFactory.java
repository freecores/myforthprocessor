/*
 * @(#)SPARCV9InstructionFactory.java	1.2 03/01/23 11:19:20
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.asm.sparc;

import sun.jvm.hotspot.asm.*;

public interface SPARCV9InstructionFactory extends SPARCInstructionFactory {
    public SPARCInstruction newV9BranchInstruction(String name, PCRelativeAddress addr,
              boolean isAnnuled, int conditionCode, boolean predictTaken, int conditionFlag);
    public SPARCInstruction newV9RegisterBranchInstruction(String name, PCRelativeAddress addr,
                               boolean isAnnuled, int regConditionCode, SPARCRegister conditionRegister,
                               boolean predictTaken);
    public SPARCInstruction newV9CasInstruction(String name, SPARCRegisterIndirectAddress addr, 
                               SPARCRegister rs2, SPARCRegister rd, int dataType);
    public SPARCInstruction newV9PrefetchInstruction(String name, SPARCRegisterIndirectAddress addr, 
                               int prefetchFcn);
    public SPARCInstruction newV9FlushwInstruction();
    public SPARCInstruction newV9MOVccInstruction(String name, int conditionCode, int conditionFlag,
                                   ImmediateOrRegister source, SPARCRegister rd);
    public SPARCInstruction newV9MOVrInstruction(String name, SPARCRegister rs1,
                                   ImmediateOrRegister operand2, SPARCRegister rd,
                                   int regConditionCode);
    public SPARCInstruction newV9RdprInstruction(int regNum, SPARCRegister rd);
    public SPARCInstruction newV9WrprInstruction(SPARCRegister rs1, ImmediateOrRegister operand2, int regNum);
    public SPARCInstruction newV9PopcInstruction(ImmediateOrRegister source, SPARCRegister rd);
    public SPARCInstruction newV9DoneInstruction();
    public SPARCInstruction newV9RetryInstruction();
    public SPARCInstruction newV9SavedInstruction();
    public SPARCInstruction newV9RestoredInstruction();
    public SPARCInstruction newV9ReadInstruction(int specialRegNum, int asrRegNum, SPARCRegister rd);
    public SPARCInstruction newV9WriteInstruction(int specialRegNum, int asrRegNum, SPARCRegister rs1, 
                                                  ImmediateOrRegister operand2);
    public SPARCInstruction newV9MembarInstruction(int mmask, int cmask);
    public SPARCInstruction newV9SirInstruction();
    public SPARCInstruction newV9FMOVccInstruction(String name, int opf, int conditionCode,
                              int conditionFlag, SPARCFloatRegister rs,
                              SPARCFloatRegister rd);
    public SPARCInstruction newV9FMOVrInstruction(String name, int opf, 
                                   SPARCRegister rs1, SPARCFloatRegister rs2, 
                                   SPARCFloatRegister rd, int regConditionCode);
}
