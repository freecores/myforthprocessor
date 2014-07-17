/*
 * @(#)convert.c	1.3 03/01/23
 * 
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * Expand out a crunched JAR file.
 *
 *							KGH Nov 98
 */

#include <stdio.h>
#include "crunch.h"
#include "opcodes.h"

static void convert_attributes(ushort flags);
static void convert_attribute();
static void convert_small_code_attribute();
static void convert_code_attribute();
static void convert_code();

// Pointers to the inbuffer and outbuffer.
static uchar *inp;
static uchar *outp;

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
put1(uchar value)
{
    *outp++ = value;
}

static void
put2(ushort value)
{
    *outp++ = (value >> 8) & 0xFF;
    *outp++ = value & 0xFF;
}

static void
put4(uint value)
{
    *outp++ = (value >> 24) & 0xFF;
    *outp++ = (value >> 16) & 0xFF;
    *outp++ = (value >> 8) & 0xFF;
    *outp++ = value & 0xFF;
}

static void
put4at(uint value, uchar *ptr)
{
    *ptr++ = (value >> 24) & 0xFF;
    *ptr++ = (value >> 16) & 0xFF;
    *ptr++ = (value >> 8) & 0xFF;
    *ptr++ = value & 0xFF;
}

uint
convert(uchar *inp_arg, uchar *outp_arg, int length)
{
    ushort len, i, rix;

    inp = inp_arg;  
    outp = outp_arg;  

    // We are only concerned with the data after the local
    // constant pool.  The magic header, versions, and
    // constant pool are handled elsewhere.

    // Copy over the flags.
    put2(get2());

    // Convert the this-class pointer.
    put2(localize_class(get2()));

    // Convert the super-class pointer.
    put2(localize_class(get2()));

    // Convert the interface pointers.
    len = get1();
    put2(len);
    for (i = 0 ; i < len; i++) {
        put2(localize_class(get2()));
    }

    // Convert the field descriptors.
    len = get2();
    put2(len);
    for (i = 0; i < len; i++) {
	// Convert the flags
	ushort flags = get2();
	put2((ushort)(flags & 0xFFF));
	if (flags & 0x4000) {
	    // Get a reference index and write the name and type.
	    rix = get2();
	    put2(localize_ref_name(rix));
	    put2(localize_ref_type(rix));
	} else {
	    // Convert name and type indexes.
	    put2(localize_utf(get2()));
	    put2(localize_type(get2()));
	}
	// Convert the attributes.
	convert_attributes(flags);
    }

    // Process the method descriptors.
    len =get1();
    put2(len);
    for (i = 0; i < len; i++) {
	// Convert the flags, name-index, descriptor-index.
	ushort flags = get2();
	put2((ushort)(flags & 0xFFF));
	if (flags & 0x4000) {
	    // Get a reference index and write the name and type.
	    rix = get2();
	    put2(localize_ref_name(rix));
	    put2(localize_ref_type(rix));
	} else {
	    // Convert name and type indexes.
	    put2(localize_utf(get2()));
	    put2(localize_type(get2()));
	}
	// Convert the attributes.
	convert_attributes(flags);
    }

    // Process the class level attributes
    convert_attributes(0x8000);

    // Check that the sizes came out right.
    if ((inp_arg+length) != inp) {
	fprintf(stderr, "botched convert\n");
	exit(53);
    }

    return (outp - outp_arg);

}

// convert attribute table
void
convert_attributes(ushort flags)
{
    ushort i, len, tlen;
   
    if (flags & 0x8000) {
	len = get1();
    } else {
	len = 0;
    }
    tlen = len;
    if (flags & 0x2000) {
        tlen++;
    }
    put2(tlen);
    for (i = 0; i < len; i++) {
	convert_attribute();
    }
    if (flags & 0x2000) {
	convert_small_code_attribute();
    }
}

// Convert an attribute.
void
convert_attribute()
{
    ushort len, i;
    uchar kind = get1();

    // Write the attribute name index.
    put2(localize_utf(kind));

    switch (kind) {
      case ATTRIBUTE_CODE:
	convert_code_attribute();
	break;
      case ATTRIBUTE_VALUE:
	// Convert value pointer.
	put4(2);
	put2(localize_value(get2()));
	break;
      case ATTRIBUTE_EXCEPTIONS:
	len = get1();
	put4(2 + (2*len)); 
	put2(len);
	for (i = 0; i < len; i++) {
	    put2(localize_class(get2()));
	}
	break;
      case ATTRIBUTE_INNERCLASSES:
	len = get1();
	put4(2 + (8*len)); 
	put2(len);
	for (i = 0; i < len; i++) {
	    put2(localize_class(get2()));
	    put2(localize_class(get2()));
	    put2(localize_utf(get2()));
	    put2(get2());
	}
	break;
      case ATTRIBUTE_DEPRECATED:
      case ATTRIBUTE_SYNTHETIC:
	put4(0); 
	break;
      default:
	fprintf(stderr, "Unexpected attribute %d\n", kind);
	exit(51);
    }
}

void
convert_small_code_attribute()
{
    ushort i;
    uchar *attribute_length_ptr;
    uchar *code_length_ptr;
    uint xlen;
    uchar bits;

    // Write the attribute name index.
    put2(localize_utf(ATTRIBUTE_CODE));

    // Reserve space for the attribute length:
    attribute_length_ptr = outp;
    put4(0);

    // Copy maxStack and maxLocals.
    bits = get1();
    put2((ushort)(bits >> 4));
    put2((ushort)(bits & 0xF));

    // Reserve space for the code length
    code_length_ptr = outp;
    put4(0);

    // Convert the actual byte codes.
    convert_code();

    // Update the code length.
    xlen = outp - (code_length_ptr+4);
    put4at(xlen, code_length_ptr);

    // We don't have an exception table.
    put2(0);

    // There are no nested attributes
    put2(0);

    // Update the attribute length.
    xlen = outp - (attribute_length_ptr+4);
    put4at(xlen, attribute_length_ptr);
}

void
convert_code_attribute()
{
    ushort elen, i;
    uchar *attribute_length_ptr;
    uchar *code_length_ptr;
    uint xlen;

    // Reserve space for the attribute length:
    attribute_length_ptr = outp;
    put4(0);

    // Copy maxStack and maxLocals.
    put2(get2());
    put2(get2());

    // Reserve space for the code length
    code_length_ptr = outp;
    put4(0);

    // Convert the actual byte codes.
    convert_code();

    // Update the code length.
    xlen = outp - (code_length_ptr+4);
    put4at(xlen, code_length_ptr);

    // Copy over the exception table.
    elen = get2();
    put2(elen);
    for (i = 0; i < elen; i++) {
	put2(get2());
	put2(get2());
	put2(get2());
	put2(localize_class(get2()));
    }

    // There are no nested attributes
    put2(0);

    // Update the attribute length.
    xlen = outp - (attribute_length_ptr+4);
    put4at(xlen, attribute_length_ptr);
}

void
convert_code()
{
    char *start = outp;
    ushort index, fix;
    int dflt, low, high, slots, i;

    for (;;) {
	uchar opcode = get1();

	// printf("%d : %d\n", (outp-start-1), opcode);

	// Check for terminators first.
        switch (opcode) {
          case 255:
	    return;
          case 254:
	    put1(opc_return);
	    return;
          case 253:
	    put1(opc_areturn);
	    return;
          case 252:
	    put1(opc_ireturn);
	    return;
	  default:
	    break;
	}

        put1(opcode);

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
	    put1(get1());
    	    break;

          case opc_sipush:
          case opc_iinc:
    	    // Opcode has two boring byte arguments.
	    put2(get2());
    	    break;

          case opc_wide:
    	    // adds 2 or 4 boring byte to normal opcode
    	    opcode = get1();
	    put1(opcode);
	    put2(get2());
    	    if (opcode == opc_iinc) {
	        put2(get2());
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
	    put2(get2());
    	    break;
    	     	      
          case opc_goto_w:
          case opc_jsr_w:
    	    // 32 bit branches.
	    put4(get4());
    	    break;

          case opc_getstatic:
          case opc_putstatic:
          case opc_getfield:
          case opc_putfield:
            // Field reference.
	    put2(localize_field_ref(get2()));
	    break;

          case opc_invokevirtual:
          case opc_invokespecial:
          case opc_invokestatic:
            // method reference.
	    put2(localize_method_ref(get2()));
	    break;

          case opc_invokeinterface:
            // interface method reference, plus two boring bytes.
	    put2(localize_interface_method_ref(get2()));
	    put2(get2());
    	    break;

          case opc_new:
          case opc_anewarray:
          case opc_checkcast:
          case opc_instanceof:	
    	    //  Class reference
	    put2(localize_class(get2()));
    	    break;

          case opc_multianewarray:
    	    //  Class reference + boring byte.
	    put2(localize_class(get2()));
	    put1(get1());
    	    break;

          case opc_ldc:
	    // This one needs special handling.
	    // We currently have a two byte value index and need
	    // to replace it with a one byte local index.
	    // We allocated the value constant during our prescan so 
	    // the index should fit in a byte.
	    index = get2();
	    fix = localize_value(index);
	    if (fix > 255) {
	        fprintf(stderr, "convert_code got long index for ldc constant???");
		exit(71);
	    }
	    put1((uchar)fix);
	    break;

          case opc_ldc_w:
          case opc_ldc2_w:
	    // value constant.
	    put2(localize_value(get2()));
    	    break;

          case opc_tableswitch:
    	    // This is complicated, but all the bytes are boring.
	    // Note: there is no padding in our input.
	    // Pad our output to a long boundary.
	    while ((outp-start)&3) {
		put1(0);
	    }
    	    dflt = get4();
	    put4(dflt);
    	    low = get4();
	    put4(low);
    	    high = get4();
	    put4(high);
    	    slots = high - low + 1;
	    // printf("tableswitch %d %d %d %d\n", dflt, low, high, slots);
    	    for (i = 0; i < slots; i++) {
		put4(get4());
    	    }
    	    break;

          case opc_lookupswitch:
    	    // This is complicated, but all the bytes are boring.
	    // Note: there is no padding in our input.
	    // Pad our output to a long boundary.
	    while ((outp-start)&3) {
		put1(0);
	    }
    	    dflt = get4();
	    put4(dflt);
    	    slots = get4();
	    put4(slots);
    	    for (i = 0; i < slots; i++) {
		put4(get4());
		put4(get4());
    	    }
    	    break;

          default:
    	    fprintf(stderr, "Unexpected opcode %d\n", opcode);
	    exit(57);
        }
    }

}

