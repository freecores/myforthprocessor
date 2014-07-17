/*
 * @(#)CodeAttribute.java	1.3 03/01/23
 * 
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.tools.crunch;

/**
 * Descriptor for a "Code" attribute.
 *
 * @author Graham Hamilton
 */

import java.io.*;

// We implement the sun.tools.java.RuntimeConstants interface to
// pick up the java bytecode constant definitions.

public class CodeAttribute extends Attribute
	implements sun.tools.java.RuntimeConstants {

    // Statistics
    private static int count;
    private static int totalExceptionBytes;
    private static int totalCodeLength;
    private static int expandedBytecodes;
    private static int skippedPadding;
    private static int mungedReturns;
    private static int fullWrites;
    private static int smallWrites;

    private ClassFile cfile;
    private short maxStack;
    private short maxLocals;
    private byte code[];

    // Information on exceptions.
    private short exStart[];
    private short exEnd[];
    private short exHandler[];
    private short exClass[];

    public static Attribute read(ClassFile cf, DataInputStream in) throws IOException {
	return new CodeAttribute(cf, in);
    }

    private CodeAttribute(ClassFile cf, DataInputStream in) throws IOException {
	count++;

	cfile = cf;

	// We skip the overall attribute length as we don't need it.
	in.readInt();

	maxStack = in.readShort();
	maxLocals = in.readShort();

	// This is the code length:
	int length = in.readInt();
	if (length >= 0x10000) {
	   throw new Error("Code block too long in " + cfile.getName());
	}
	totalCodeLength += length;

	// Read the code
	code = new byte[length];
	int read = 0;
	while (read < length) {
	    int rc = in.read(code, read, length-read);
	    if (rc <= 0) {
		throw new IOException("Couldn't read code");
	    }
	    read += rc;
	}

	short exceptionCount = in.readShort();
	exStart = new short[exceptionCount];
	exEnd = new short[exceptionCount];
	exHandler = new short[exceptionCount];
	exClass = new short[exceptionCount];
	for (int i = 0; i < exceptionCount; i++) {
	    exStart[i] = in.readShort();
	    exEnd[i] = in.readShort();
	    exHandler[i] = in.readShort();
	    exClass[i] = in.readShort();
	}
	totalExceptionBytes += (8 * exceptionCount);

	// We don't expect or want any code sub-attributes.
	Attribute attributes[] = Attribute.readAttributes(cf, in);
	if (attributes.length != 0) {
	    throw new Error("Unexpected code sub-attribute");
	}
    }

    // Report on whether we can be written using writeSmall.
    public boolean isSmall() {
	// Note that we have to check the non-converted code size.
	if (maxStack <= 15 && maxLocals <= 15 && exStart.length == 0) {
	    return true;
	}
	return false;
    }

    public void write(DataOutputStream out) throws IOException {
	fullWrites++;
	out.writeByte(GlobalTables.codeAttribute);
	out.writeShort(maxStack);
	out.writeShort(maxLocals);
	writeCode(out);
	writeExceptions(out);	
    }

    public void writeSmall(DataOutputStream out) throws IOException {
	smallWrites++;
	// We can pack maxStack and maxLocals into a single byte.
	int bits = (maxStack << 4) | maxLocals;
	out.writeByte(bits);
	writeCode(out);
    }

    private short shortAt(byte data[], int index) {
	return ((short)((code[index] << 8) | (code[index+1] & 0xFF)));
    }

    private int intAt(byte data[], int index) {
	return ((code[index] << 24) | 
		((code[index+1] << 16) & 0xFF0000) |
		((code[index+2] << 8) & 0xFF00) |
		(code[index+3] & 0xFF));
    }

    /**
     * Write out the bytecodes, replacing all local constant pool references
     * with corresponding global constant pool references.
     */

    private void writeCode(DataOutputStream out) throws IOException {
	int length = code.length;
	int offset = 0;
	int opcode = 0;
	Constant con;
	short fix;
	Object value;
	int dflt, low, high, slots;

	while (offset < length) {
	    opcode = ((int)code[offset++]) & 0xFF;
	    // System.out.println("" + offset + "  opcode = " + opcode);

	    // We special case the last opcode if it is a return.
	    if (offset == length) {
	        switch (opcode) {
	          case opc_return:
		    mungedReturns++;
		    out.writeByte(254);
		    return;
	          case opc_areturn:
		    mungedReturns++;
		    out.writeByte(253);
		    return;
	          case opc_ireturn:
		    mungedReturns++;
		    out.writeByte(252);
		    return;
		  default:
		   break;
		}
	    }

	    out.writeByte(opcode);

	    switch (opcode) {
	      case opc_nop:
	      case opc_iconst_m1:
	      case opc_iconst_0:
	      case opc_iconst_1:
	      case opc_iconst_2:
	      case opc_iconst_3:
	      case opc_iconst_4:
	      case opc_iconst_5:
	      case opc_lconst_0:
	      case opc_lconst_1:
	      case opc_fconst_0:
	      case opc_fconst_1:
	      case opc_fconst_2:
	      case opc_dconst_0:
	      case opc_dconst_1:
	      case opc_aconst_null:
	      case opc_iload_0:
	      case opc_iload_1:
	      case opc_iload_2:
	      case opc_iload_3:
	      case opc_lload_0:
	      case opc_lload_1:
	      case opc_lload_2:
	      case opc_lload_3:
	      case opc_fload_0:
	      case opc_fload_1:
	      case opc_fload_2:
	      case opc_fload_3:
	      case opc_dload_0:
	      case opc_dload_1:
	      case opc_dload_2:
	      case opc_dload_3:
	      case opc_aload_0:
	      case opc_aload_1:
	      case opc_aload_2:
	      case opc_aload_3:
	      case opc_iaload:
	      case opc_laload:
	      case opc_faload:
	      case opc_daload:
	      case opc_aaload:
	      case opc_baload:
	      case opc_caload:
	      case opc_saload:
	      case opc_istore_0:
	      case opc_istore_1:
	      case opc_istore_2:
	      case opc_istore_3:
	      case opc_lstore_0:
	      case opc_lstore_1:
	      case opc_lstore_2:
	      case opc_lstore_3:
	      case opc_fstore_0:
	      case opc_fstore_1:
	      case opc_fstore_2:
	      case opc_fstore_3:
	      case opc_dstore_0:
	      case opc_dstore_1:
	      case opc_dstore_2:
	      case opc_dstore_3:
	      case opc_astore_0:
	      case opc_astore_1:
	      case opc_astore_2:
	      case opc_astore_3:
	      case opc_iastore:
	      case opc_lastore:
	      case opc_fastore:
	      case opc_dastore:
	      case opc_aastore:
	      case opc_bastore:
	      case opc_castore:
	      case opc_sastore:
	      case opc_pop:
	      case opc_pop2:
	      case opc_dup:
	      case opc_dup_x1:
	      case opc_dup_x2:
	      case opc_dup2:
	      case opc_dup2_x1:
	      case opc_dup2_x2:
	      case opc_swap:
	      case opc_iadd:
	      case opc_ladd:
	      case opc_fadd:
	      case opc_dadd:
	      case opc_isub:
	      case opc_lsub:
	      case opc_fsub:
	      case opc_dsub:
	      case opc_imul:
	      case opc_lmul:
	      case opc_fmul:
	      case opc_dmul:
	      case opc_idiv:
	      case opc_ldiv:
	      case opc_fdiv:
	      case opc_ddiv:
	      case opc_irem:
	      case opc_lrem:
	      case opc_frem:
	      case opc_drem:
	      case opc_ineg:
	      case opc_lneg:
	      case opc_fneg:
	      case opc_dneg:
	      case opc_ishl:
	      case opc_lshl:
	      case opc_ishr:
	      case opc_lshr:
	      case opc_iushr:
	      case opc_lushr:
	      case opc_iand:
	      case opc_land:
	      case opc_ior:
	      case opc_lor:
	      case opc_ixor:
	      case opc_lxor:
	      case opc_i2l:
	      case opc_i2f:
	      case opc_i2d:
	      case opc_l2i:
	      case opc_l2f:
	      case opc_l2d:
	      case opc_f2i:
	      case opc_f2l:
	      case opc_f2d:
	      case opc_d2i:
	      case opc_d2l:
	      case opc_d2f:
	      case opc_i2b:
	      case opc_i2c:
	      case opc_i2s:
	      case opc_lcmp:
	      case opc_fcmpl:
	      case opc_fcmpg:
	      case opc_dcmpl:
	      case opc_dcmpg:
	      case opc_lreturn:
	      case opc_freturn:
	      case opc_dreturn:
	      case opc_return:
	      case opc_areturn:
	      case opc_ireturn:
	      case opc_arraylength:
	      case opc_athrow:
	      case opc_monitorenter:
	      case opc_monitorexit:
		// Opcode has no arguments.  Continue.
		break;

	      case opc_bipush:
	      case opc_iload:
	      case opc_lload:
	      case opc_fload:
	      case opc_dload:
	      case opc_aload:
	      case opc_istore:
	      case opc_lstore:
	      case opc_fstore:
	      case opc_dstore:
	      case opc_astore:
	      case opc_ret:
	      case opc_newarray:
		// Opcode has one boring byte argument.
		// Write the byte and continue.
	        out.writeByte(code[offset++]);
		break;

	      case opc_sipush:
	      case opc_iinc:
		// Opcode has two boring byte arguments.
		// Write the bytes and continue.
	        out.writeByte(code[offset++]);
	        out.writeByte(code[offset++]);
		break;

	      case opc_wide:
		// adds 2 or 4 boring byte to normal opcode
		opcode = ((int)code[offset]) & 0xFF;
	        out.writeByte(code[offset++]);
	        out.writeByte(code[offset++]);
	        out.writeByte(code[offset++]);
		if (opcode == opc_iinc) {
	            out.writeByte(code[offset++]);
	            out.writeByte(code[offset++]);
		}
		break;
		
	      case opc_ifeq:
	      case opc_ifne:
	      case opc_iflt:
	      case opc_ifge:
	      case opc_ifgt:
	      case opc_ifle:
	      case opc_if_icmpeq:
	      case opc_if_icmpne:
	      case opc_if_icmplt:
	      case opc_if_icmpge:
	      case opc_if_icmpgt:
	      case opc_if_icmple:
	      case opc_if_acmpeq:
	      case opc_if_acmpne:
	      case opc_goto:
	      case opc_jsr:
	      case opc_ifnull:
	      case opc_ifnonnull:
		// 16 bit branches.
	        out.writeByte(code[offset++]);
	        out.writeByte(code[offset++]);
		break;
		     	      
	      case opc_goto_w:
	      case opc_jsr_w:
		// 32 bit branches.
	        out.writeByte(code[offset++]);
	        out.writeByte(code[offset++]);
	        out.writeByte(code[offset++]);
	        out.writeByte(code[offset++]);
		break;

	      case opc_getstatic:
	      case opc_putstatic:
	      case opc_getfield:
	      case opc_putfield:
	      case opc_invokevirtual:
	      case opc_invokespecial:
	      case opc_invokestatic:
	        // Field/method reference.
		// Get a short and map it.
		fix = shortAt(code, offset);
		offset += 2;
		fix = cfile.mapRef(fix);
		out.writeShort(fix);
		break;

	      case opc_invokeinterface:
	        // Method reference, plus two boring bytes.
	        // Get a short and map it.
		fix = shortAt(code, offset);
		offset += 2;
		fix = cfile.mapRef(fix);
		out.writeShort(fix);
	        out.writeByte(code[offset++]);
	        out.writeByte(code[offset++]);
		break;

	      case opc_new:
	      case opc_anewarray:
	      case opc_checkcast:
	      case opc_instanceof:
		//  Class reference
		fix = shortAt(code, offset);
		offset += 2;
	        fix = cfile.mapClass(fix);
	        out.writeShort(fix);
		break;

	      case opc_multianewarray:
		//  Class reference + boring byte.
		fix = shortAt(code, offset);
		offset += 2;
	        fix = cfile.mapClass(fix);
	        out.writeShort(fix);
	        out.writeByte(code[offset++]);
		break;

	      case opc_ldc:
		// Pointer to integer/string/float/long/double constant.
		// We expand the single byte argument into a pair of bytes.
		// We will do the reverse  transformation at decode time.
		fix = (short)(((int)code[offset++]) & 0xFF);
		con = cfile.constants[fix];
		value =  con.getValue();
		fix = GlobalTables.findValue(value);
		out.writeShort(fix);
		expandedBytecodes++;
		break;

	      case opc_ldc_w:
	      case opc_ldc2_w:
		// pointer to integer/string/float/long/double constant.
		fix = shortAt(code, offset);
		offset += 2;
		con = cfile.constants[fix];
		value =  con.getValue();
		fix = GlobalTables.findValue(value);
		out.writeShort(fix);
		break;

	      case opc_tableswitch:
		// This is complicated, but all the bytes are boring.
		// Note: we don't write the padding.
		while ((offset&3) != 0) {
		    skippedPadding++;
	            offset++;
		}
		dflt = intAt(code, offset);
		low = intAt(code, offset+4);
		high = intAt(code, offset+8);
		for (int i = 0; i < 12; i++) {
	            out.writeByte(code[offset++]);
		}
		slots = high - low + 1;
		for (int i = 0; i < (4*slots); i++) {
	            out.writeByte(code[offset++]);
		}
		break;

	      case opc_lookupswitch:
		// This is complicated, but all the bytes are boring.
		// Note: we don't write the padding.
		while ((offset&3) != 0) {
		    skippedPadding++;
	            offset++;
		}
		dflt = intAt(code, offset);
		slots = intAt(code, offset+4);
		for (int i = 0; i < 8; i++) {
	            out.writeByte(code[offset++]);
		}
		for (int i = 0; i < (8*slots); i++) {
	            out.writeByte(code[offset++]);
		}
		break;

	      default:
		throw new Error("Unexpected opcode " + opcode);
	    }
	}

	// And now write the terminator.
	out.writeByte(255);

    }

    private void writeExceptions(DataOutputStream out) throws IOException {
	int length = exStart.length;
	out.writeShort((short)length);
	for (int i = 0; i < length; i++) {
	    out.writeShort(exStart[i]);
	    out.writeShort(exEnd[i]);
	    out.writeShort(exHandler[i]);
	    // Transfom the class index to a global index.
	    short cix = exClass[i];
	    if (cix != 0) {
	        cix = cfile.mapClass(cix);
	    }
	    out.writeShort(cix);
	}
    }

    public static void summarize() {
	System.out.println("    " + totalExceptionBytes/8 +
			" exception handlers, total length = " + totalExceptionBytes + " bytes");
	System.out.println("    " + count + " code attributes, total bytecode length = "
			+ totalCodeLength + " bytes");
	System.out.println("    total expanded ldc bytecodes = " + expandedBytecodes);
	System.out.println("    total skipped padding = " + (-skippedPadding));
	System.out.println("    total munged returns = " + mungedReturns);
	System.out.println("    code writes => " + fullWrites + " full, " +
						smallWrites + " small");
    }
}
