/*
 * @(#)SPARCDisassembler.java	1.3 03/01/23 11:17:04
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.asm.sparc;

import sun.jvm.hotspot.asm.*;
import java.io.*;
import java.util.*;

public abstract class SPARCDisassembler extends Disassembler 
                    implements /* imports */ SPARCOpcodes, RTLDataTypes, RTLOperations {

    // instruction cache - Map<Integer, Instruction>.
    protected static Map instructionCache = new HashMap();
    protected final SPARCInstructionFactory factory;

    public SPARCDisassembler(long startPc, byte[] code, SPARCInstructionFactory factory) {
        super(startPc, code);
        this.factory = factory;
    }

    protected static InstructionDecoder illegalDecoder = new IllegalInstructionDecoder();
    protected static InstructionDecoder callDecoder = new CallDecoder();

    // direct call instruction
    protected Instruction decodeFormat1Instruction(int instruction) {
        return callDecoder.decode(instruction, factory);
    }

    protected abstract InstructionDecoder getFormat2Decoder(int op2);

    protected Instruction decodeFormat2Instruction(int instruction) {
        int op2 = (instruction & OP_2_MASK) >>> OP_2_START_BIT;
        InstructionDecoder decoder = getFormat2Decoder(op2);
        return decoder.decode(instruction, factory);
    }

    // "op3" - used in format 3 & 3A instructions - 6 bits width

    protected static int getOp3(int instruction) {
        return (instruction & OP_3_MASK) >>> OP_3_START_BIT;
    }

    // op3 opcodes is broken up into column and row. MSB 2 bits form column.
    // LSB 4 bits form row number.

    protected static int getOp3Row(int op3) {
        return op3 & 0xF;
    }

    protected static int getOp3Column(int op3) {
        return (op3 >>> 4) & 0x3;
    }

    protected abstract InstructionDecoder getFormat3Decoder(int row, int column);

    // memory instructions
    protected Instruction decodeFormat3Instruction(int instruction) {
        int op3 = getOp3(instruction);
        int row = getOp3Row(op3);
        int column = getOp3Column(op3);
        return getFormat3Decoder(row, column).decode(instruction, factory);
    }

    protected abstract InstructionDecoder getFormat3ADecoder(int row, int column);

    // arithmetic, logic, shift and the rest
    protected Instruction decodeFormat3AInstruction(int instruction) {
        int op3 = getOp3(instruction);
        int row = getOp3Row(op3);
        int column = getOp3Column(op3);
        return getFormat3ADecoder(row, column).decode(instruction, factory);
    }

    public void decode(InstructionVisitor visitor) {
        visitor.prologue();
        try {
            DataInputStream dis = new DataInputStream(new ByteArrayInputStream(code));
            int instruction = -1;
            int format = -1;
            Instruction instr = null;
            int len = 0;

            while (len < code.length) {
                instr = null;
                instruction =  dis.readInt();
                // check whether we have this in cache.
                instr = (Instruction) instructionCache.get(new Integer(instruction));
                if (instr == null) {
                    format = (instruction & FORMAT_MASK) >>> FORMAT_START_BIT;

                    switch (format) {
                        case FORMAT_2:  // 0
                            instr = decodeFormat2Instruction(instruction);
                            break;

                        case FORMAT_1:  // 1
                            instr = decodeFormat1Instruction(instruction);
                            break;

                        case FORMAT_3A: // 2
                            instr = decodeFormat3AInstruction(instruction);
                            break;

                        case FORMAT_3:  // 3
                            instr = decodeFormat3Instruction(instruction);
                            break;
                    }

                    // add the new instruction to cache.
                    instructionCache.put(new Integer(instruction), instr);
                }

                visitor.visit(startPc + len, instr);
                len += 4;
            } 
        } catch (IOException ioExp) {
            // ignore, can't happen
        } finally {
            visitor.epilogue();
        }
    }
}
