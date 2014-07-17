/*
 * @(#)crunch.h	1.3 03/01/23
 * 
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


/*
 * Constants to do with Java Crunch file format.
 */

#define uchar unsigned char
#define ushort unsigned short
#define uint unsigned int

// Magic value at the start off each Crunched file:
#define CRUNCH_MAGIC 1262962738

// Current file-format version number
#define CRUNCH_VERSION 19

// Tags for Java constant pool entries.
#define CONSTANT_UTF8		     1
#define CONSTANT_INTEGER	     3
#define CONSTANT_FLOAT		     4
#define CONSTANT_LONG		     5
#define CONSTANT_DOUBLE		     6
#define CONSTANT_CLASS 		     7
#define CONSTANT_STRING		     8
#define CONSTANT_FIELDREF	     9
#define CONSTANT_METHODREF	    10
#define CONSTANT_INTERFACEMETHODREF 11
#define CONSTANT_NAMEANDTYPE	    12

// Tags for attributes.
#define ATTRIBUTE_CODE		1
#define ATTRIBUTE_VALUE		2
#define ATTRIBUTE_EXCEPTIONS	3
#define ATTRIBUTE_SYNTHETIC	4
#define ATTRIBUTE_INNERCLASSES	5
#define ATTRIBUTE_DEPRECATED	6

// Functions to read from our input file.
extern void read_fully(void *buff, int len);
extern ushort read_swapped_short();
extern uint read_swapped_int();

// Utility functions.
extern ushort get_swapped_short(uchar *raw);
extern uint get_swapped_int(uchar *raw);
extern void set_swapped_short(uchar *raw, ushort x);
extern void set_swapped_int(uchar *raw, uint x);

extern void write_data(void *buff, int len);

extern void prescan(uchar *bp, int length);
extern uint convert(uchar *inp, uchar *outp, int length);

// Constant pool functions.
extern void read_global_pools();
void reset_local_pool();
extern void prescan_value(ushort ix);
extern void fixup_prescan_values();
extern ushort localize_value(ushort ix);
extern ushort localize_class(ushort ix);
extern ushort localize_utf(ushort ix);
extern ushort localize_type(ushort ix);
extern ushort localize_field_ref(ushort ix);
extern ushort localize_method_ref(ushort ix);
extern ushort localize_interface_method_ref(ushort ix);
extern ushort localize_ref_name(ushort ix);
extern ushort localize_ref_type(ushort ix);
extern uint get_local_pool_length();
