/**
 * @(#)ByteCodes.java	1.13 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.tools.javac.v8.code;

/**
 * Bytecode instruction codes, as well as typecodes used as
 *  instruction modifiers.
 */
public interface ByteCodes {

    /**
     * Byte code instruction codes.
     */
    int illegal = -1;

    /**
     * Byte code instruction codes.
     */
    int nop = 0;

    /**
     * Byte code instruction codes.
     */
    int aconst_null = 1;

    /**
     * Byte code instruction codes.
     */
    int iconst_m1 = 2;

    /**
     * Byte code instruction codes.
     */
    int iconst_0 = 3;

    /**
     * Byte code instruction codes.
     */
    int iconst_1 = 4;

    /**
     * Byte code instruction codes.
     */
    int iconst_2 = 5;

    /**
     * Byte code instruction codes.
     */
    int iconst_3 = 6;

    /**
     * Byte code instruction codes.
     */
    int iconst_4 = 7;

    /**
     * Byte code instruction codes.
     */
    int iconst_5 = 8;

    /**
     * Byte code instruction codes.
     */
    int lconst_0 = 9;

    /**
     * Byte code instruction codes.
     */
    int lconst_1 = 10;

    /**
     * Byte code instruction codes.
     */
    int fconst_0 = 11;

    /**
     * Byte code instruction codes.
     */
    int fconst_1 = 12;

    /**
     * Byte code instruction codes.
     */
    int fconst_2 = 13;

    /**
     * Byte code instruction codes.
     */
    int dconst_0 = 14;

    /**
     * Byte code instruction codes.
     */
    int dconst_1 = 15;

    /**
     * Byte code instruction codes.
     */
    int bipush = 16;

    /**
     * Byte code instruction codes.
     */
    int sipush = 17;

    /**
     * Byte code instruction codes.
     */
    int ldc1 = 18;

    /**
     * Byte code instruction codes.
     */
    int ldc2 = 19;

    /**
     * Byte code instruction codes.
     */
    int ldc2w = 20;

    /**
     * Byte code instruction codes.
     */
    int iload = 21;

    /**
     * Byte code instruction codes.
     */
    int lload = 22;

    /**
     * Byte code instruction codes.
     */
    int fload = 23;

    /**
     * Byte code instruction codes.
     */
    int dload = 24;

    /**
     * Byte code instruction codes.
     */
    int aload = 25;

    /**
     * Byte code instruction codes.
     */
    int iload_0 = 26;

    /**
     * Byte code instruction codes.
     */
    int iload_1 = 27;

    /**
     * Byte code instruction codes.
     */
    int iload_2 = 28;

    /**
     * Byte code instruction codes.
     */
    int iload_3 = 29;

    /**
     * Byte code instruction codes.
     */
    int lload_0 = 30;

    /**
     * Byte code instruction codes.
     */
    int lload_1 = 31;

    /**
     * Byte code instruction codes.
     */
    int lload_2 = 32;

    /**
     * Byte code instruction codes.
     */
    int lload_3 = 33;

    /**
     * Byte code instruction codes.
     */
    int fload_0 = 34;

    /**
     * Byte code instruction codes.
     */
    int fload_1 = 35;

    /**
     * Byte code instruction codes.
     */
    int fload_2 = 36;

    /**
     * Byte code instruction codes.
     */
    int fload_3 = 37;

    /**
     * Byte code instruction codes.
     */
    int dload_0 = 38;

    /**
     * Byte code instruction codes.
     */
    int dload_1 = 39;

    /**
     * Byte code instruction codes.
     */
    int dload_2 = 40;

    /**
     * Byte code instruction codes.
     */
    int dload_3 = 41;

    /**
     * Byte code instruction codes.
     */
    int aload_0 = 42;

    /**
     * Byte code instruction codes.
     */
    int aload_1 = 43;

    /**
     * Byte code instruction codes.
     */
    int aload_2 = 44;

    /**
     * Byte code instruction codes.
     */
    int aload_3 = 45;

    /**
     * Byte code instruction codes.
     */
    int iaload = 46;

    /**
     * Byte code instruction codes.
     */
    int laload = 47;

    /**
     * Byte code instruction codes.
     */
    int faload = 48;

    /**
     * Byte code instruction codes.
     */
    int daload = 49;

    /**
     * Byte code instruction codes.
     */
    int aaload = 50;

    /**
     * Byte code instruction codes.
     */
    int baload = 51;

    /**
     * Byte code instruction codes.
     */
    int caload = 52;

    /**
     * Byte code instruction codes.
     */
    int saload = 53;

    /**
     * Byte code instruction codes.
     */
    int istore = 54;

    /**
     * Byte code instruction codes.
     */
    int lstore = 55;

    /**
     * Byte code instruction codes.
     */
    int fstore = 56;

    /**
     * Byte code instruction codes.
     */
    int dstore = 57;

    /**
     * Byte code instruction codes.
     */
    int astore = 58;

    /**
     * Byte code instruction codes.
     */
    int istore_0 = 59;

    /**
     * Byte code instruction codes.
     */
    int istore_1 = 60;

    /**
     * Byte code instruction codes.
     */
    int istore_2 = 61;

    /**
     * Byte code instruction codes.
     */
    int istore_3 = 62;

    /**
     * Byte code instruction codes.
     */
    int lstore_0 = 63;

    /**
     * Byte code instruction codes.
     */
    int lstore_1 = 64;

    /**
     * Byte code instruction codes.
     */
    int lstore_2 = 65;

    /**
     * Byte code instruction codes.
     */
    int lstore_3 = 66;

    /**
     * Byte code instruction codes.
     */
    int fstore_0 = 67;

    /**
     * Byte code instruction codes.
     */
    int fstore_1 = 68;

    /**
     * Byte code instruction codes.
     */
    int fstore_2 = 69;

    /**
     * Byte code instruction codes.
     */
    int fstore_3 = 70;

    /**
     * Byte code instruction codes.
     */
    int dstore_0 = 71;

    /**
     * Byte code instruction codes.
     */
    int dstore_1 = 72;

    /**
     * Byte code instruction codes.
     */
    int dstore_2 = 73;

    /**
     * Byte code instruction codes.
     */
    int dstore_3 = 74;

    /**
     * Byte code instruction codes.
     */
    int astore_0 = 75;

    /**
     * Byte code instruction codes.
     */
    int astore_1 = 76;

    /**
     * Byte code instruction codes.
     */
    int astore_2 = 77;

    /**
     * Byte code instruction codes.
     */
    int astore_3 = 78;

    /**
     * Byte code instruction codes.
     */
    int iastore = 79;

    /**
     * Byte code instruction codes.
     */
    int lastore = 80;

    /**
     * Byte code instruction codes.
     */
    int fastore = 81;

    /**
     * Byte code instruction codes.
     */
    int dastore = 82;

    /**
     * Byte code instruction codes.
     */
    int aastore = 83;

    /**
     * Byte code instruction codes.
     */
    int bastore = 84;

    /**
     * Byte code instruction codes.
     */
    int castore = 85;

    /**
     * Byte code instruction codes.
     */
    int sastore = 86;

    /**
     * Byte code instruction codes.
     */
    int pop = 87;

    /**
     * Byte code instruction codes.
     */
    int pop2 = 88;

    /**
     * Byte code instruction codes.
     */
    int dup = 89;

    /**
     * Byte code instruction codes.
     */
    int dup_x1 = 90;

    /**
     * Byte code instruction codes.
     */
    int dup_x2 = 91;

    /**
     * Byte code instruction codes.
     */
    int dup2 = 92;

    /**
     * Byte code instruction codes.
     */
    int dup2_x1 = 93;

    /**
     * Byte code instruction codes.
     */
    int dup2_x2 = 94;

    /**
     * Byte code instruction codes.
     */
    int swap = 95;

    /**
     * Byte code instruction codes.
     */
    int iadd = 96;

    /**
     * Byte code instruction codes.
     */
    int ladd = 97;

    /**
     * Byte code instruction codes.
     */
    int fadd = 98;

    /**
     * Byte code instruction codes.
     */
    int dadd = 99;

    /**
     * Byte code instruction codes.
     */
    int isub = 100;

    /**
     * Byte code instruction codes.
     */
    int lsub = 101;

    /**
     * Byte code instruction codes.
     */
    int fsub = 102;

    /**
     * Byte code instruction codes.
     */
    int dsub = 103;

    /**
     * Byte code instruction codes.
     */
    int imul = 104;

    /**
     * Byte code instruction codes.
     */
    int lmul = 105;

    /**
     * Byte code instruction codes.
     */
    int fmul = 106;

    /**
     * Byte code instruction codes.
     */
    int dmul = 107;

    /**
     * Byte code instruction codes.
     */
    int idiv = 108;

    /**
     * Byte code instruction codes.
     */
    int ldiv = 109;

    /**
     * Byte code instruction codes.
     */
    int fdiv = 110;

    /**
     * Byte code instruction codes.
     */
    int ddiv = 111;

    /**
     * Byte code instruction codes.
     */
    int imod = 112;

    /**
     * Byte code instruction codes.
     */
    int lmod = 113;

    /**
     * Byte code instruction codes.
     */
    int fmod = 114;

    /**
     * Byte code instruction codes.
     */
    int dmod = 115;

    /**
     * Byte code instruction codes.
     */
    int ineg = 116;

    /**
     * Byte code instruction codes.
     */
    int lneg = 117;

    /**
     * Byte code instruction codes.
     */
    int fneg = 118;

    /**
     * Byte code instruction codes.
     */
    int dneg = 119;

    /**
     * Byte code instruction codes.
     */
    int ishl = 120;

    /**
     * Byte code instruction codes.
     */
    int lshl = 121;

    /**
     * Byte code instruction codes.
     */
    int ishr = 122;

    /**
     * Byte code instruction codes.
     */
    int lshr = 123;

    /**
     * Byte code instruction codes.
     */
    int iushr = 124;

    /**
     * Byte code instruction codes.
     */
    int lushr = 125;

    /**
     * Byte code instruction codes.
     */
    int iand = 126;

    /**
     * Byte code instruction codes.
     */
    int land = 127;

    /**
     * Byte code instruction codes.
     */
    int ior = 128;

    /**
     * Byte code instruction codes.
     */
    int lor = 129;

    /**
     * Byte code instruction codes.
     */
    int ixor = 130;

    /**
     * Byte code instruction codes.
     */
    int lxor = 131;

    /**
     * Byte code instruction codes.
     */
    int iinc = 132;

    /**
     * Byte code instruction codes.
     */
    int i2l = 133;

    /**
     * Byte code instruction codes.
     */
    int i2f = 134;

    /**
     * Byte code instruction codes.
     */
    int i2d = 135;

    /**
     * Byte code instruction codes.
     */
    int l2i = 136;

    /**
     * Byte code instruction codes.
     */
    int l2f = 137;

    /**
     * Byte code instruction codes.
     */
    int l2d = 138;

    /**
     * Byte code instruction codes.
     */
    int f2i = 139;

    /**
     * Byte code instruction codes.
     */
    int f2l = 140;

    /**
     * Byte code instruction codes.
     */
    int f2d = 141;

    /**
     * Byte code instruction codes.
     */
    int d2i = 142;

    /**
     * Byte code instruction codes.
     */
    int d2l = 143;

    /**
     * Byte code instruction codes.
     */
    int d2f = 144;

    /**
     * Byte code instruction codes.
     */
    int int2byte = 145;

    /**
     * Byte code instruction codes.
     */
    int int2char = 146;

    /**
     * Byte code instruction codes.
     */
    int int2short = 147;

    /**
     * Byte code instruction codes.
     */
    int lcmp = 148;

    /**
     * Byte code instruction codes.
     */
    int fcmpl = 149;

    /**
     * Byte code instruction codes.
     */
    int fcmpg = 150;

    /**
     * Byte code instruction codes.
     */
    int dcmpl = 151;

    /**
     * Byte code instruction codes.
     */
    int dcmpg = 152;

    /**
     * Byte code instruction codes.
     */
    int ifeq = 153;

    /**
     * Byte code instruction codes.
     */
    int ifne = 154;

    /**
     * Byte code instruction codes.
     */
    int iflt = 155;

    /**
     * Byte code instruction codes.
     */
    int ifge = 156;

    /**
     * Byte code instruction codes.
     */
    int ifgt = 157;

    /**
     * Byte code instruction codes.
     */
    int ifle = 158;

    /**
     * Byte code instruction codes.
     */
    int if_icmpeq = 159;

    /**
     * Byte code instruction codes.
     */
    int if_icmpne = 160;

    /**
     * Byte code instruction codes.
     */
    int if_icmplt = 161;

    /**
     * Byte code instruction codes.
     */
    int if_icmpge = 162;

    /**
     * Byte code instruction codes.
     */
    int if_icmpgt = 163;

    /**
     * Byte code instruction codes.
     */
    int if_icmple = 164;

    /**
     * Byte code instruction codes.
     */
    int if_acmpeq = 165;

    /**
     * Byte code instruction codes.
     */
    int if_acmpne = 166;

    /**
     * Byte code instruction codes.
     */
    int goto_ = 167;

    /**
     * Byte code instruction codes.
     */
    int jsr = 168;

    /**
     * Byte code instruction codes.
     */
    int ret = 169;

    /**
     * Byte code instruction codes.
     */
    int tableswitch = 170;

    /**
     * Byte code instruction codes.
     */
    int lookupswitch = 171;

    /**
     * Byte code instruction codes.
     */
    int ireturn = 172;

    /**
     * Byte code instruction codes.
     */
    int lreturn = 173;

    /**
     * Byte code instruction codes.
     */
    int freturn = 174;

    /**
     * Byte code instruction codes.
     */
    int dreturn = 175;

    /**
     * Byte code instruction codes.
     */
    int areturn = 176;

    /**
     * Byte code instruction codes.
     */
    int return_ = 177;

    /**
     * Byte code instruction codes.
     */
    int getstatic = 178;

    /**
     * Byte code instruction codes.
     */
    int putstatic = 179;

    /**
     * Byte code instruction codes.
     */
    int getfield = 180;

    /**
     * Byte code instruction codes.
     */
    int putfield = 181;

    /**
     * Byte code instruction codes.
     */
    int invokevirtual = 182;

    /**
     * Byte code instruction codes.
     */
    int invokespecial = 183;

    /**
     * Byte code instruction codes.
     */
    int invokestatic = 184;

    /**
     * Byte code instruction codes.
     */
    int invokeinterface = 185;

    /**
     * Byte code instruction codes.
     */
    int newfromname = 186;

    /**
     * Byte code instruction codes.
     */
    int new_ = 187;

    /**
     * Byte code instruction codes.
     */
    int newarray = 188;

    /**
     * Byte code instruction codes.
     */
    int anewarray = 189;

    /**
     * Byte code instruction codes.
     */
    int arraylength = 190;

    /**
     * Byte code instruction codes.
     */
    int athrow = 191;

    /**
     * Byte code instruction codes.
     */
    int checkcast = 192;

    /**
     * Byte code instruction codes.
     */
    int instanceof_ = 193;

    /**
     * Byte code instruction codes.
     */
    int monitorenter = 194;

    /**
     * Byte code instruction codes.
     */
    int monitorexit = 195;

    /**
     * Byte code instruction codes.
     */
    int wide = 196;

    /**
     * Byte code instruction codes.
     */
    int multianewarray = 197;

    /**
     * Byte code instruction codes.
     */
    int if_acmp_null = 198;

    /**
     * Byte code instruction codes.
     */
    int if_acmp_nonnull = 199;

    /**
     * Byte code instruction codes.
     */
    int goto_w = 200;

    /**
     * Byte code instruction codes.
     */
    int jsr_w = 201;

    /**
     * Byte code instruction codes.
     */
    int breakpoint = 202;

    /**
     * Byte code instruction codes.
     */
    int ByteCodeCount = 203;

    /**
     * Virtual instruction codes; used for constant folding.
     */
    int string_add = 256;

    /**
     * Virtual instruction codes; used for constant folding.
     */
    int bool_not = 257;

    /**
     * Virtual instruction codes; used for constant folding.
     */
    int bool_and = 258;

    /**
     * Virtual instruction codes; used for constant folding.
     */
    int bool_or = 259;

    /**
     * Virtual opcodes; used for shifts with long shiftcount
     */
    int ishll = 270;

    /**
     * Virtual opcodes; used for shifts with long shiftcount
     */
    int lshll = 271;

    /**
     * Virtual opcodes; used for shifts with long shiftcount
     */
    int ishrl = 272;

    /**
     * Virtual opcodes; used for shifts with long shiftcount
     */
    int lshrl = 273;

    /**
     * Virtual opcodes; used for shifts with long shiftcount
     */
    int iushrl = 274;

    /**
     * Virtual opcodes; used for shifts with long shiftcount
     */
    int lushrl = 275;

    /**
     * Virtual opcode for null reference checks
     */
    int nullchk = 276;

    /**
     * Virtual opcode for disallowed operations.
     */
    int error = 277;

    /**
     * All conditional jumps come in pairs. To streamline the
     *  treatment of jumps, we also introduce a negation of an
     *  unconditional jump. That opcode happens to be jsr.
     */
    int dontgoto = jsr;

    /**
     * Shift and mask constants for shifting prefix instructions.
     *  a pair of instruction codes such as LCMP ; IFEQ is encoded
     *  in Symtab as (LCMP << preShift) + IFEQ.
     */
    int preShift = 9;
    int preMask = (1 << preShift)
                  - 1;

    /**
     * Type codes.
     */
    int INTcode = 0;

    /**
     * Type codes.
     */
    int LONGcode = 1;

    /**
     * Type codes.
     */
    int FLOATcode = 2;

    /**
     * Type codes.
     */
    int DOUBLEcode = 3;

    /**
     * Type codes.
     */
    int OBJECTcode = 4;

    /**
     * Type codes.
     */
    int BYTEcode = 5;

    /**
     * Type codes.
     */
    int CHARcode = 6;

    /**
     * Type codes.
     */
    int SHORTcode = 7;

    /**
     * Type codes.
     */
    int VOIDcode = 8;

    /**
     * Type codes.
     */
    int TypeCodeCount = 9;
    static final String[] typecodeNames = {"int", "long", "float", "double", "object",
    "byte", "char", "short", "void", "oops"};
}
