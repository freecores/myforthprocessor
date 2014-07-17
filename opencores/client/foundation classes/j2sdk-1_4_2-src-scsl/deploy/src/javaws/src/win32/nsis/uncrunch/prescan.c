/*
 * @(#)prescan.c	1.3 03/01/23
 * 
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * Support code to prescan a classffile and locate and instantiate all
 * one-byte constant references from "ldc" instructions.
 *
 * This prescan is necessary to ensure that all ldc constants are put
 * at the front of the constant table where they can be reached with
 * a one-byte index.
 *
 *							KGH Nov 98
 */

#include <stdio.h>
#include "crunch.h"
#include "opcodes.h"

// Pointer to the inbuffer
static uchar *inp;

static void skip_attribute(uchar kind);
static void prescan_small_code_attribute();
static void prescan_code_attribute();
static void prescan_code();

static uchar 
get1()
{
    return *inp++;
}

static ushort
get2()
{
    ushort val = (inp[0] << 8) | inp[1];
    inp += 2;
    return val;
}

static uint
get4()
{
    uint val = (inp[0] << 24) | (inp[1] << 16) | (inp[2] << 8) | inp[3];
    inp += 4;
    return val;
}

static void
skip(int x)
{
    inp += x;
}


void
prescan(uchar *inp_arg, int length)
{
    char *start = inp_arg;
    int i, j, len, alen;

    inp = inp_arg;

    // Skip the flags, this-class, and super-class.
    skip(6);

    // Skip the interface pointers.
    len = get1();
    skip(len*2);

    // Skip the field descriptors.
    len = get2();
    for (i = 0; i < len; i++) {
	ushort flags = get2();
	if (flags & 0x4000) {
	    // Skip the ref-index.
	    skip(2);
	} else {
	    // Skip name and type indexes.
	    skip(4);
	}
	if (flags & 0x8000) {
	    alen = get1();
	    for (j = 0; j < alen; j++) {
	        uchar kind = get1();
	        skip_attribute(kind);
	    }
	}
    }

    // Process the method descriptors.
    len = get1();
    for (i = 0; i < len; i++) {
	ushort flags = get2();
	if (flags & 0x4000) {
	    // Skip the ref-index.
	    skip(2);
	} else {
	    // Skip name and type indexes.
	    skip(4);
	}
	if (flags & 0x8000) {
	    alen = get1();
	    for (j = 0; j < alen; j++) {
	        uchar kind = get1();
	        if (kind == ATTRIBUTE_CODE) {
		    prescan_code_attribute();
	        } else {
	            skip_attribute(kind);
		}
	    }
	}
	if (flags & 0x2000) {
	    // there is a small code attribute.
	    prescan_small_code_attribute();	
	}
    }

    // Process the class level attributes
    len = get1();
    for (i = 0; i < len; i++) {
        uchar kind = get1();
	skip_attribute(kind);
    }

    if ((start+length) != inp) {
	fprintf(stderr, "botched prescan\n");
	exit(53);
    }
}

// Skip over attributes.
void
skip_attribute(uchar kind)
{
    int alen, i;

    switch (kind) {
      case ATTRIBUTE_CODE:
	fprintf(stderr, "Trying to skip code attribute???\n");
	exit(50);
	break;
      case ATTRIBUTE_VALUE:
	// Skip value pointer.
	skip(2);
	break;
      case ATTRIBUTE_EXCEPTIONS:
	alen = get1();
	skip(2*alen);
	break;
      case ATTRIBUTE_INNERCLASSES:
	alen = get1();
	skip(8*alen);
	break;
      case ATTRIBUTE_DEPRECATED:
      case ATTRIBUTE_SYNTHETIC:
	// Nothing to do
	break;
      default:
	fprintf(stderr, "prescan unexpected attribute %d\n", kind);
	exit(51);
    }
}

void
prescan_small_code_attribute()
{
    // Skip maxStack and maxLocals.
    skip(1);

    // prescan the bytecodes.
    prescan_code();
}

void
prescan_code_attribute()
{
   ushort elen;    

    // Skip maxStack and maxLocals.
    skip(4);

    // prescan the bytecodes.
    prescan_code();

    // Skip the exception table.
    elen = get2();
    skip(8 * elen);
}

void
prescan_code()
{
    char *start = inp;
    ushort index, fix;
    int dflt, low, high, slots, i;

    for (;;) {
	int opcode = get1();
	if (opcode >= 252) {
	   break;
	}

        switch (opcode) {
          case opc_ldc:
	    // This is what we are looking for!!!
	    // Pointer to integer/string/float/long/double constant.
	    index = get2();
	    prescan_value(index);
	    break;

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
          case opc_ireturn:
          case opc_lreturn:
          case opc_freturn:
          case opc_dreturn:
          case opc_areturn:
          case opc_return:
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
	    skip(1);
    	    break;

          case opc_sipush:
          case opc_iinc:
    	    // Opcode has two boring byte arguments.
	    skip(2);
    	    break;

          case opc_wide:
    	    // adds 2 or 4 boring byte to normal opcode
    	    opcode = get1();
	    skip(2);
    	    if (opcode == opc_iinc) {
	        skip(2);
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
    	    // Branches.  
	    skip(2);
    	    break;
    	     	      
          case opc_goto_w:
          case opc_jsr_w:
    	    // 32 bit branches.
	    skip(4);
    	    break;

          case opc_getstatic:
          case opc_putstatic:
          case opc_getfield:
          case opc_putfield:
          case opc_invokevirtual:
          case opc_invokespecial:
          case opc_invokestatic:
            // Field/method reference.
	    skip(2);
	    break;

          case opc_invokeinterface:
            // Method reference, plus two boring bytes.
	    skip(4);
    	    break;

          case opc_new:
          case opc_anewarray:
          case opc_checkcast:
          case opc_instanceof:	
    	    //  Class reference
	    skip(2);
    	    break;

          case opc_multianewarray:
    	    //  Class reference + boring byte.
	    skip(3);
    	    break;

          case opc_ldc_w:
          case opc_ldc2_w:
	    skip(2);
    	    break;

          case opc_tableswitch:
    	    // This is complicated, but all the bytes are boring.
	    // Note: there is no padding in our input.
    	    dflt = get4();
    	    low = get4();
    	    high = get4();
    	    slots = high - low + 1;
	    skip(4*slots);
    	    break;

          case opc_lookupswitch:
    	    // This is complicated, but all the bytes are boring.
	    // Note: there is no padding in our input.
    	    dflt = get4();
    	    slots = get4();
	    skip(8*slots);
    	    break;

          default:
    	    fprintf(stderr, "Unexpected opcode %d\n", opcode);
	    exit(57);
        }
    }
}

