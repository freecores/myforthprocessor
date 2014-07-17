/*
 * @(#)constants.h	1.5 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

// classfile constants
#define JAVA_MAGIC 0xCAFEBABE
#define JAVA_MIN_MAJOR_VERSION 45
#define JAVA_MIN_MINOR_VERSION 3
#define JAVA_MAX_MAJOR_VERSION 50
#define JAVA_MAX_MINOR_VERSION 0

// package file constants
#define JAVA_PACKAGE_MAGIC 0xCAFED00D
#define JAVA_PACKAGE_MAJOR_VERSION 146
// %%% for now, this number is bumped on each output-changing putback:
#define JAVA_PACKAGE_MINOR_VERSION 3

enum {
  CODE_SIZE_LIMIT  = 0x10000
};

enum {
    CONSTANT_None,
    CONSTANT_Utf8,
    CONSTANT_Unicode,		/* unused */
    CONSTANT_Integer,
    CONSTANT_Float,
    CONSTANT_Long,      
    CONSTANT_Double,
    CONSTANT_Class,
    CONSTANT_String,
    CONSTANT_Fieldref,
    CONSTANT_Methodref,
    CONSTANT_InterfaceMethodref,
    CONSTANT_NameandType,

    CONSTANT_Signature = 13,
    CONSTANT_Member = 14,
    CONSTANT_Limit  = 18,

    CONSTANT_Literal   //pseudo-tag for debugging
};

enum {
  ACC_STATIC         = 0x00000008,
  ACC_NATIVE         = 0x00000100,
  ACC_ABSTRACT       = 0x00000400,
  ACC_CONSTANT_VALUE = 0x00000020, // ACC_SYNCHRONIZED, for fields
  ACC_EXCEPTIONS     = 0x00000040, // ACC_VOLATILE, for methods
  ACC_IC_LONG_FORM   = 0x00000040, // ACC_VOLATILE, for ic_flags
  ACC_HAS_ATTRIBUTES = 0x00001000
};

// Bytecode goo.

enum {
  bc_nop                  =   0, // 0x00
  bc_aconst_null          =   1, // 0x01
  bc_iconst_m1            =   2, // 0x02
  bc_iconst_0             =   3, // 0x03
  bc_iconst_1             =   4, // 0x04
  bc_iconst_2             =   5, // 0x05
  bc_iconst_3             =   6, // 0x06
  bc_iconst_4             =   7, // 0x07
  bc_iconst_5             =   8, // 0x08
  bc_lconst_0             =   9, // 0x09
  bc_lconst_1             =  10, // 0x0a
  bc_fconst_0             =  11, // 0x0b
  bc_fconst_1             =  12, // 0x0c
  bc_fconst_2             =  13, // 0x0d
  bc_dconst_0             =  14, // 0x0e
  bc_dconst_1             =  15, // 0x0f
  bc_bipush               =  16, // 0x10
  bc_sipush               =  17, // 0x11
  bc_ldc                  =  18, // 0x12
  bc_ldc_w                =  19, // 0x13
  bc_ldc2_w               =  20, // 0x14
  bc_iload                =  21, // 0x15
  bc_lload                =  22, // 0x16
  bc_fload                =  23, // 0x17
  bc_dload                =  24, // 0x18
  bc_aload                =  25, // 0x19
  bc_iload_0              =  26, // 0x1a
  bc_iload_1              =  27, // 0x1b
  bc_iload_2              =  28, // 0x1c
  bc_iload_3              =  29, // 0x1d
  bc_lload_0              =  30, // 0x1e
  bc_lload_1              =  31, // 0x1f
  bc_lload_2              =  32, // 0x20
  bc_lload_3              =  33, // 0x21
  bc_fload_0              =  34, // 0x22
  bc_fload_1              =  35, // 0x23
  bc_fload_2              =  36, // 0x24
  bc_fload_3              =  37, // 0x25
  bc_dload_0              =  38, // 0x26
  bc_dload_1              =  39, // 0x27
  bc_dload_2              =  40, // 0x28
  bc_dload_3              =  41, // 0x29
  bc_aload_0              =  42, // 0x2a
  bc_aload_1              =  43, // 0x2b
  bc_aload_2              =  44, // 0x2c
  bc_aload_3              =  45, // 0x2d
  bc_iaload               =  46, // 0x2e
  bc_laload               =  47, // 0x2f
  bc_faload               =  48, // 0x30
  bc_daload               =  49, // 0x31
  bc_aaload               =  50, // 0x32
  bc_baload               =  51, // 0x33
  bc_caload               =  52, // 0x34
  bc_saload               =  53, // 0x35
  bc_istore               =  54, // 0x36
  bc_lstore               =  55, // 0x37
  bc_fstore               =  56, // 0x38
  bc_dstore               =  57, // 0x39
  bc_astore               =  58, // 0x3a
  bc_istore_0             =  59, // 0x3b
  bc_istore_1             =  60, // 0x3c
  bc_istore_2             =  61, // 0x3d
  bc_istore_3             =  62, // 0x3e
  bc_lstore_0             =  63, // 0x3f
  bc_lstore_1             =  64, // 0x40
  bc_lstore_2             =  65, // 0x41
  bc_lstore_3             =  66, // 0x42
  bc_fstore_0             =  67, // 0x43
  bc_fstore_1             =  68, // 0x44
  bc_fstore_2             =  69, // 0x45
  bc_fstore_3             =  70, // 0x46
  bc_dstore_0             =  71, // 0x47
  bc_dstore_1             =  72, // 0x48
  bc_dstore_2             =  73, // 0x49
  bc_dstore_3             =  74, // 0x4a
  bc_astore_0             =  75, // 0x4b
  bc_astore_1             =  76, // 0x4c
  bc_astore_2             =  77, // 0x4d
  bc_astore_3             =  78, // 0x4e
  bc_iastore              =  79, // 0x4f
  bc_lastore              =  80, // 0x50
  bc_fastore              =  81, // 0x51
  bc_dastore              =  82, // 0x52
  bc_aastore              =  83, // 0x53
  bc_bastore              =  84, // 0x54
  bc_castore              =  85, // 0x55
  bc_sastore              =  86, // 0x56
  bc_pop                  =  87, // 0x57
  bc_pop2                 =  88, // 0x58
  bc_dup                  =  89, // 0x59 
  bc_dup_x1               =  90, // 0x5a
  bc_dup_x2               =  91, // 0x5b
  bc_dup2                 =  92, // 0x5c
  bc_dup2_x1              =  93, // 0x5d
  bc_dup2_x2              =  94, // 0x5e
  bc_swap                 =  95, // 0x5f
  bc_iadd                 =  96, // 0x60
  bc_ladd                 =  97, // 0x61
  bc_fadd                 =  98, // 0x62
  bc_dadd                 =  99, // 0x63
  bc_isub                 = 100, // 0x64
  bc_lsub                 = 101, // 0x65
  bc_fsub                 = 102, // 0x66
  bc_dsub                 = 103, // 0x67
  bc_imul                 = 104, // 0x68
  bc_lmul                 = 105, // 0x69
  bc_fmul                 = 106, // 0x6a
  bc_dmul                 = 107, // 0x6b
  bc_idiv                 = 108, // 0x6c
  bc_ldiv                 = 109, // 0x6d
  bc_fdiv                 = 110, // 0x6e
  bc_ddiv                 = 111, // 0x6f
  bc_irem                 = 112, // 0x70
  bc_lrem                 = 113, // 0x71
  bc_frem                 = 114, // 0x72
  bc_drem                 = 115, // 0x73
  bc_ineg                 = 116, // 0x74
  bc_lneg                 = 117, // 0x75
  bc_fneg                 = 118, // 0x76
  bc_dneg                 = 119, // 0x77
  bc_ishl                 = 120, // 0x78
  bc_lshl                 = 121, // 0x79
  bc_ishr                 = 122, // 0x7a
  bc_lshr                 = 123, // 0x7b
  bc_iushr                = 124, // 0x7c
  bc_lushr                = 125, // 0x7d
  bc_iand                 = 126, // 0x7e
  bc_land                 = 127, // 0x7f
  bc_ior                  = 128, // 0x80
  bc_lor                  = 129, // 0x81
  bc_ixor                 = 130, // 0x82
  bc_lxor                 = 131, // 0x83
  bc_iinc                 = 132, // 0x84
  bc_i2l                  = 133, // 0x85
  bc_i2f                  = 134, // 0x86
  bc_i2d                  = 135, // 0x87
  bc_l2i                  = 136, // 0x88
  bc_l2f                  = 137, // 0x89
  bc_l2d                  = 138, // 0x8a
  bc_f2i                  = 139, // 0x8b
  bc_f2l                  = 140, // 0x8c
  bc_f2d                  = 141, // 0x8d
  bc_d2i                  = 142, // 0x8e
  bc_d2l                  = 143, // 0x8f
  bc_d2f                  = 144, // 0x90
  bc_i2b                  = 145, // 0x91
  bc_i2c                  = 146, // 0x92
  bc_i2s                  = 147, // 0x93
  bc_lcmp                 = 148, // 0x94
  bc_fcmpl                = 149, // 0x95
  bc_fcmpg                = 150, // 0x96
  bc_dcmpl                = 151, // 0x97
  bc_dcmpg                = 152, // 0x98
  bc_ifeq                 = 153, // 0x99
  bc_ifne                 = 154, // 0x9a
  bc_iflt                 = 155, // 0x9b
  bc_ifge                 = 156, // 0x9c
  bc_ifgt                 = 157, // 0x9d
  bc_ifle                 = 158, // 0x9e
  bc_if_icmpeq            = 159, // 0x9f
  bc_if_icmpne            = 160, // 0xa0
  bc_if_icmplt            = 161, // 0xa1
  bc_if_icmpge            = 162, // 0xa2
  bc_if_icmpgt            = 163, // 0xa3
  bc_if_icmple            = 164, // 0xa4
  bc_if_acmpeq            = 165, // 0xa5
  bc_if_acmpne            = 166, // 0xa6
  bc_goto                 = 167, // 0xa7
  bc_jsr                  = 168, // 0xa8
  bc_ret                  = 169, // 0xa9
  bc_tableswitch          = 170, // 0xaa
  bc_lookupswitch         = 171, // 0xab
  bc_ireturn              = 172, // 0xac
  bc_lreturn              = 173, // 0xad
  bc_freturn              = 174, // 0xae
  bc_dreturn              = 175, // 0xaf
  bc_areturn              = 176, // 0xb0
  bc_return               = 177, // 0xb1
  bc_getstatic            = 178, // 0xb2
  bc_putstatic            = 179, // 0xb3
  bc_getfield             = 180, // 0xb4
  bc_putfield             = 181, // 0xb5
  bc_invokevirtual        = 182, // 0xb6
  bc_invokespecial        = 183, // 0xb7
  bc_invokestatic         = 184, // 0xb8
  bc_invokeinterface      = 185, // 0xb9
  bc_xxxunusedxxx         = 186, // 0xba
  bc_new                  = 187, // 0xbb
  bc_newarray             = 188, // 0xbc
  bc_anewarray            = 189, // 0xbd
  bc_arraylength          = 190, // 0xbe
  bc_athrow               = 191, // 0xbf
  bc_checkcast            = 192, // 0xc0
  bc_instanceof           = 193, // 0xc1
  bc_monitorenter         = 194, // 0xc2
  bc_monitorexit          = 195, // 0xc3
  bc_wide                 = 196, // 0xc4
  bc_multianewarray       = 197, // 0xc5
  bc_ifnull               = 198, // 0xc6
  bc_ifnonnull            = 199, // 0xc7
  bc_goto_w               = 200, // 0xc8
  bc_jsr_w                = 201, // 0xc9
  bc_bytecode_limit       = 202  // 0xca
};

enum {
  _end_marker = 255,

  _first_linker_op = bc_getstatic,
  _last_linker_op  = bc_invokestatic,
  _num_linker_ops  = (_last_linker_op - _first_linker_op) + 1,
  _self_linker_op  = bc_bytecode_limit,
  _self_linker_aload_flag = 1*_num_linker_ops,
  _self_linker_super_flag = 2*_num_linker_ops,
  _self_linker_limit = _self_linker_op + 4*_num_linker_ops,

  _invokeinit_op = _self_linker_limit,
  _invokeinit_self_option = 0,
  _invokeinit_super_option = 1,
  _invokeinit_new_option = 2,
  _invokeinit_limit = _invokeinit_op+3,

  _xldc_op = _invokeinit_limit,
  _aldc = bc_ldc,
  _ildc = _xldc_op+0,
  _fldc = _xldc_op+1,
  _aldc_w = bc_ldc_w,
  _ildc_w = _xldc_op+2,
  _fldc_w = _xldc_op+3,
  _lldc2_w = bc_ldc2_w,
  _dldc2_w = _xldc_op+4,
  _xldc_limit = _xldc_op+5,

  _xxx_end
};





