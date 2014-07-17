/*
 * @(#)check_format.c	1.49 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include <setjmp.h>
#include <stdlib.h>
#include <string.h>

#include "jni.h"
#include "jvm.h"

#define JAVA_CLASSFILE_MAGIC                 0xCafeBabe
#define JAVA_MIN_SUPPORTED_VERSION           45
#define JAVA_MAX_SUPPORTED_VERSION           48
#define JAVA_MAX_SUPPORTED_MINOR_VERSION     0

/* align byte code */
#ifndef ALIGN_UP
#define ALIGN_UP(n,align_grain) (((n) + ((align_grain) - 1)) & ~((align_grain)-1))
#endif /* ALIGN_UP */

typedef unsigned short unicode;

/* represent a UTF-8 string as appeared in class files. */
typedef struct {
    unsigned char len_h; /* unaligned */
    unsigned char len_l;
    char bytes[1];
} utf_str;

#define UTF_LEN(utf) \
  ((((unsigned int)(utf)->len_h) << 8) + (unsigned int)((utf)->len_l))

typedef union {
    jint i;
    utf_str *s;
} cp_type;

typedef struct {
    unsigned char *ptr;
    unsigned char *end_ptr;
    class_size_info *size;
    jmp_buf jump_buffer;

    cp_type *constant_pool;
    unsigned char *type_table;
    int nconstants;

    char *cp_buffer;
    jint err_code;
    char *class_name;
    char *msg;
    unsigned int msg_len;
    jboolean in_clinit;
   
    int major_version;
    int minor_version;

    jboolean measure_only;
    jboolean check_relaxed;
} CFcontext;

static int utfcmp(utf_str *ustr, char *cstr);
static char * utfncpy(char *buf, int buf_len, utf_str *ustr);

/*
 * The following routines are used to find duplicate name/signature of
 * interfaces, fields, and methods. It is based on heap sort algorithm.
 */
static int  uucmp2(utf_str *name1, utf_str *sig1, utf_str *name2, utf_str *sig2);
static void HeapAdjust(utf_str* array[], int first, int last, int init);
static void HeapSort(utf_str* array[], int length);
static int  FindDup(utf_str* array[], int length);

static utf_str *getAsciz(CFcontext *);
static utf_str *getAscizFromCP(CFcontext *, unsigned int i);

static unsigned int get1byte(CFcontext *);
static unsigned int get2bytes(CFcontext *);
static unsigned int get4bytes(CFcontext *);

static void skip2bytes(CFcontext *);
static void skip4bytes(CFcontext *);
static void skipNbytes(CFcontext *, size_t n);
static void skipAsciz(CFcontext *);

static utf_str *getUTF8String(CFcontext *);

static void ParseConstantPool(CFcontext *);
static void ParseCode(CFcontext *, utf_str *mb_name, unsigned int args_size);
static void ParseLineTable(CFcontext *, unsigned int code_length);
static void ParseExceptions(CFcontext *);
static void ParseLocalVars(CFcontext *, unsigned int code_length);

static void CFerror(CFcontext *context, char *fmt, ...);
static void CNerror(CFcontext *context, char *name);
static void UVerror(CFcontext *context);
static void CFnomem(CFcontext *context);

static void 
verify_static_constant(CFcontext *context, utf_str *sig, unsigned int index);

static void
verify_constant_pool(CFcontext *context, cp_type* constant_pool,
		     unsigned char * type_table, unsigned int nconstants);

static void 
verify_legal_class_modifiers(CFcontext *context, unsigned int access);
static void 
verify_legal_field_modifiers(CFcontext *context, unsigned int access, 
			     jboolean isIntf);
static void
verify_legal_method_modifiers(CFcontext *context, unsigned int access, 
			      jboolean isIntf, utf_str *name);

static void 
verify_legal_utf8(CFcontext *context, utf_str *str);

/* Argument for verify_legal_name */
enum { LegalClass, LegalField, LegalMethod, LegalVariable };
static void 
verify_legal_name(CFcontext *context, utf_str *name, int type);

static unsigned int
verify_legal_method_signature(CFcontext *context, utf_str *name,
			      utf_str *signature);
static void
verify_legal_field_signature(CFcontext *context, utf_str *name, 
			     utf_str *signature);

static void
verify_constant_entry(CFcontext *context, unsigned int index, int type, 
		      char *msg);

static unsigned int
verify_innerclasses_attribute(CFcontext *context);

static char *
skip_over_fieldname(char *name, jboolean slash_okay, 
		    unsigned int len);
static char *
skip_over_field_signature(char *name, jboolean void_okay,
			  unsigned int len);


/* RETURNS 
 * 0: on success
 * -1: out of memory
 * -2: class format error
 * -3: unsupported version error
 * -4: bad class name error
 *
 * Also fills in the class_size_info structure which contains size 
 * information of verious class components.
 */
#define CF_OK 0
#define CF_NOMEM (-1)
#define CF_ERR (-2)
#define CF_UVERR (-3)
#define CF_BADNAME (-4)
#define LOCAL_BUFFER_SIZE 1024

JNIEXPORT jint
VerifyFormat(char *class_name,
	     unsigned char *data,
	     unsigned int data_size,
	     class_size_info *size,
	     char *message_buffer,
	     jint buffer_length,
	     jboolean measure_only,
	     jboolean check_relaxed)
{
    CFcontext context_buf;
    CFcontext *context = &context_buf;
    unsigned int cb_access;
    unsigned int i;
    utf_str *cb_name;
    unsigned int attribute_count;

    unsigned int ii, total;
    utf_str *name_buffer[LOCAL_BUFFER_SIZE];
    utf_str ** volatile utf_names = name_buffer;

    memset(context, 0, sizeof(CFcontext));
    memset(size, 0, sizeof(class_size_info));
    
    /* Set up the context */
    context->ptr = data;
    context->end_ptr = data + data_size;

    context->class_name = class_name;
    context->size = size;
    context->msg = message_buffer;
    context->msg_len = buffer_length;
    context->msg[0] = 0;
    context->measure_only = measure_only;
    context->check_relaxed = check_relaxed;

    if (setjmp(context->jump_buffer)) {
        free(context->cp_buffer);
	if (utf_names != name_buffer) {
	    free(utf_names);
	}
	return context->err_code;
    }

    if (get4bytes(context) != JAVA_CLASSFILE_MAGIC) 
	CFerror(context, "Bad magic number");

    context->minor_version = get2bytes(context);
    context->major_version = get2bytes(context);
    if ((context->major_version < JAVA_MIN_SUPPORTED_VERSION) 
	 || (context->major_version > JAVA_MAX_SUPPORTED_VERSION)
	 || ((context->major_version == JAVA_MAX_SUPPORTED_VERSION)
	      && (context->minor_version > JAVA_MAX_SUPPORTED_MINOR_VERSION))) {
	UVerror(context);
    }

    ParseConstantPool(context);

    cb_access = get2bytes(context) & JVM_RECOGNIZED_CLASS_MODIFIERS;

    verify_legal_class_modifiers(context, cb_access);

    /* Get the name of the class */
    i = get2bytes(context);	/* index in constant pool of class */
    cb_name = getAscizFromCP(context, i);
    verify_legal_name(context, cb_name, LegalClass);
    if (cb_name->bytes[0] == JVM_SIGNATURE_ARRAY) {
        CFerror(context, "Bad name");
    }
    if (context->class_name != NULL && 
	utfcmp(cb_name, (char *)(context->class_name)) != 0) {
        char buf[256];
        utfncpy(buf, sizeof(buf), cb_name);
        CNerror(context, buf);
    }

    /* Get the super class name. */
    i = get2bytes(context);	/* index in constant pool of class */
    if (i > 0) {
        utf_str *super_name;
        verify_constant_entry(context, i, JVM_CONSTANT_Class, 
			      "Superclass name");
	super_name = getAscizFromCP(context, i);
	verify_legal_name(context, super_name, LegalClass);
	if (super_name->bytes[0] == JVM_SIGNATURE_ARRAY) {
	    CFerror(context, "Bad superclass name");
	}
    } else {
        if (utfcmp(cb_name, "java/lang/Object") != 0) 
	    CFerror(context, "Bad superclass index");
    }

    total = context->size->interfaces = get2bytes(context);
    total += total;
    if (total > LOCAL_BUFFER_SIZE) {
	utf_names = malloc(total * sizeof(utf_str*));
        if (!utf_names) {
	    CFnomem(context);
        }
    }
    for (i = 0; i < total; i += 2) {
        utf_str *intf_name;
        int intf_index = get2bytes(context);
	verify_constant_entry(context, intf_index, JVM_CONSTANT_Class, 
			      "Interface name");
	intf_name = getAscizFromCP(context, intf_index);
	verify_legal_name(context, intf_name, LegalClass);
	if (intf_name->bytes[0] == JVM_SIGNATURE_ARRAY) {
	    CFerror(context, "Bad interface name");
	}
        utf_names[i] = intf_name;
        utf_names[i+1] = 0;
    }
    if (FindDup(utf_names, total)) {
        CFerror(context, "Repetitive interface name");
    }
    if (utf_names != name_buffer) {
	free(utf_names);
	utf_names = name_buffer;
    }

    total = context->size->fields = get2bytes(context);
    total += total;
    if (total > LOCAL_BUFFER_SIZE) {
	utf_names = malloc(total * sizeof(utf_str*));
        if (!utf_names) {
	    CFnomem(context);
        }
    }
    for (i = 0; i < total; i += 2) {
        unsigned int j;
        jboolean hasSeenConstantValue = JNI_FALSE;
	unsigned int fb_access = 
	    get2bytes(context) & JVM_RECOGNIZED_FIELD_MODIFIERS;
	utf_str *fb_name = getAsciz(context);
	utf_str *fb_signature = getAsciz(context);

	verify_legal_field_modifiers(context, fb_access,
	(jboolean)((cb_access & JVM_ACC_INTERFACE) ? JNI_TRUE : JNI_FALSE));
	verify_legal_name(context, fb_name, LegalField);
	verify_legal_field_signature(context,  fb_name, fb_signature);

	attribute_count = get2bytes(context);
	for (j = 0; j < attribute_count; j++) {
	    utf_str *attr_name = getAsciz(context);
	    unsigned int length = get4bytes(context);
	    if ((fb_access & JVM_ACC_STATIC) 
		 && utfcmp(attr_name, "ConstantValue") == 0) {
		if (length != 2) 
		    CFerror(context, "Wrong size for VALUE attribute");
		if (hasSeenConstantValue)
		    CFerror(context, "Multiple ConstantValue attributes");
		else
		    hasSeenConstantValue = JNI_TRUE;
		if (fb_access & JVM_ACC_STATIC) {
		    verify_static_constant(context, 
					   fb_signature, 
					   get2bytes(context));
		}
	    } else {
		skipNbytes(context, length);
	    }
	}
	if ((fb_access & JVM_ACC_STATIC) &&
	    (fb_signature->bytes[0] == 'D' || 
	     fb_signature->bytes[0] == 'J')) {
	    context->size->fields2 += 1;
	}

        utf_names[i] = fb_name;
        utf_names[i+1] = fb_signature;
    }
    if (FindDup(utf_names, total)) {
        CFerror(context, "Repetitive field name/signature");
    }
    if (utf_names != name_buffer) {
	free(utf_names);
	utf_names = name_buffer;
    }

    total = context->size->methods = get2bytes(context);
    total += total;
    if (total > LOCAL_BUFFER_SIZE) {
	utf_names = malloc(total * sizeof(utf_str*));
        if (!utf_names) {
	    CFnomem(context);
        }
    }
    for (i = 0; i < total; i += 2) {
        unsigned int j;
        unsigned int args_size;
        jboolean hasSeenCodeAttribute = JNI_FALSE;
	jboolean hasSeenExceptionsAttribute = JNI_FALSE;
	unsigned int mb_access = 
	    get2bytes(context) & JVM_RECOGNIZED_METHOD_MODIFIERS;
	utf_str *mb_name = getAsciz(context);
	utf_str *mb_signature = getAsciz(context);

	if (utfcmp(mb_name, "<clinit>") == 0 &&
	    utfcmp(mb_signature, "()V") == 0) {
	    /* The VM ignores the access flags of <clinit>. We reset the
	     * access flags to avoid future errors in the VM */
	    mb_access = JVM_ACC_STATIC;
	    context->in_clinit = JNI_TRUE;
	} else {
	    verify_legal_method_modifiers(context, 
					  mb_access,
	    (jboolean)((cb_access & JVM_ACC_INTERFACE) ? JNI_TRUE : JNI_FALSE),
					  mb_name);
	}

	verify_legal_name(context, mb_name, LegalMethod);
	args_size = ((mb_access & JVM_ACC_STATIC) ? 0 : 1) + 
	    verify_legal_method_signature(context, mb_name, mb_signature);

	if (args_size > 255) {
	    CFerror(context, "Too many arguments in signature");
	}

        utf_names[i] = mb_name;
        utf_names[i+1] = mb_signature;

	attribute_count = get2bytes(context);
	for (j = 0; j < attribute_count; j++) {
	    utf_str *attr_name = getAsciz(context);
	    if (!utfcmp(attr_name, "Code")) {
	        if (mb_access & (JVM_ACC_NATIVE | JVM_ACC_ABSTRACT)) {
		    CFerror(context,
			    "Code attribute in native or abstract methods");
		}
		if (hasSeenCodeAttribute)
		    CFerror(context, "Multiple Code attributes");
		else
		    hasSeenCodeAttribute = JNI_TRUE;
		ParseCode(context, mb_name, args_size);
	    } else if (utfcmp(attr_name, "Exceptions") == 0) {
		if (hasSeenExceptionsAttribute)
		    CFerror(context, "Multiple Exceptions attributes");
		else
		    hasSeenExceptionsAttribute = JNI_TRUE;
	        ParseExceptions(context);
	    } else {
		unsigned int length = get4bytes(context);
		skipNbytes(context, length);
	    }
	}
        if (!context->in_clinit &&
            !(mb_access & (JVM_ACC_NATIVE | JVM_ACC_ABSTRACT)) &&
            !hasSeenCodeAttribute) {
            CFerror(context, "Code attribute is absent in method that is not abstract or native");
        }
	context->in_clinit = JNI_FALSE;
    }
    if (FindDup(utf_names, total)) {
        CFerror(context, "Repetitive method name/signature");
    }
    if (utf_names != name_buffer) {
	free(utf_names);
	utf_names = name_buffer;
    }

    /* See if there are class attributes */
    attribute_count = get2bytes(context); 
    {
	jboolean seenSourceAttribute = JNI_FALSE;
	jboolean seenInnerClassesAttribute = JNI_FALSE;
	for (i = 0; i < attribute_count; i++) {
	    utf_str *attr_name = getAsciz(context);
	    unsigned int length = get4bytes(context);
	    if (utfcmp(attr_name, "SourceFile") == 0) {
	        utf_str *source_name;
		if (length != 2) {
		    CFerror(context, "Wrong size for VALUE attribute");
		}
		if (seenSourceAttribute)
		    CFerror(context, "Multiple SourceFile attributes");
		source_name = getAsciz(context);
		seenSourceAttribute = JNI_TRUE;
	    } else if (utfcmp(attr_name, "InnerClasses") == 0) {
		if (seenInnerClassesAttribute) {
		    CFerror(context, "Multiple InnerClasses attributes");
		}
		seenInnerClassesAttribute = JNI_TRUE;
		context->size->innerclasses = 
		    verify_innerclasses_attribute(context);
	    } else {
	        skipNbytes(context, length);
	    }
	}
    }
    if (context->ptr != context->end_ptr && !context->check_relaxed)
        CFerror(context, "Extra bytes at the end of the class file");
    free(context->cp_buffer);
    return CF_OK;
}

/*
 * Verify that the "InnerClasses" attribute is well formed, and return
 * the number of inner class records.
 */
static unsigned int
verify_innerclasses_attribute(CFcontext *context) 
{
    unsigned char *s = context->ptr;
    unsigned number_of_classes = get2bytes(context);
    unsigned int i = 0;

    if (context->measure_only) {
	skipNbytes(context, number_of_classes * 8);
	return number_of_classes;
    }

    for (; i < number_of_classes; i++) {
	unsigned short ioff  = get2bytes(context); /* inner_class_info_index */
	unsigned short ooff  = get2bytes(context); /* outer_class_info_index */
	unsigned short inoff = get2bytes(context); /* inner_name_index       */
	unsigned short iacc  = get2bytes(context); /* inner_class_access_flg */
	int nconstants = context->size->constants; /* constant pool size     */
    
	verify_constant_entry(context, ioff, JVM_CONSTANT_Class,
			      "inner_class_info_index");
	
	/* Can be zero for non-members. */
	if (ooff)
	    verify_constant_entry(context, ooff, JVM_CONSTANT_Class,
				  "outer_class_info_index");
	
	/* Can be zero for anonymous. */
	if (inoff)
	    verify_constant_entry(context, inoff, JVM_CONSTANT_Utf8,
				  "inner_name_index");
	
	if (ooff == ioff) 
	    CFerror(context, "Class is both outer and inner class");
	
	verify_legal_class_modifiers(context, iacc);
    }
    return number_of_classes;
}


/* Initialize the static variables in a class structure */
static void
verify_static_constant(CFcontext *context, utf_str *sig, unsigned int index)
{
    unsigned char *type_table;

    if (context->measure_only)
        return;

    type_table = context->type_table;

    if (index <= 0 || index >= context->size->constants) {
        CFerror(context, "Bad initial value");
    }
    switch (sig->bytes[0]) { 
    case JVM_SIGNATURE_DOUBLE: 
        if (type_table[index] != JVM_CONSTANT_Double) 
	    CFerror(context, "Bad index into constant pool");
	break;
    case JVM_SIGNATURE_LONG: 
        if (type_table[index] != JVM_CONSTANT_Long)
	    CFerror(context, "Bad index into constant pool");
	break;
    case JVM_SIGNATURE_CLASS:
        if (utfcmp(sig, "Ljava/lang/String;") != 0 ||
	    type_table[index] != JVM_CONSTANT_String) {
	    CFerror(context, "Bad string initial value");
	}
	break;
    case JVM_SIGNATURE_BYTE: 
    case JVM_SIGNATURE_CHAR:
    case JVM_SIGNATURE_SHORT:
    case JVM_SIGNATURE_BOOLEAN:
    case JVM_SIGNATURE_INT:  
        if (type_table[index] != JVM_CONSTANT_Integer)
	    CFerror(context, "Bad index into constant pool");
	break;
    case JVM_SIGNATURE_FLOAT:
        if (type_table[index] != JVM_CONSTANT_Float)
	    CFerror(context, "Bad index into constant pool");
	break;
			
    default: 
        CFerror(context, "Unable to set initial value");
    }
}

static void ParseConstantPool(CFcontext *context) 
{
    unsigned int nconstants = get2bytes(context);
    unsigned int cp_request;
    unsigned char *type_table;
    cp_type *constant_pool;
    unsigned int i;

    if (nconstants < 1) {
	CFerror(context, "Illegal constant pool size");
    }

    /*
     * Measure how much space we need for the constant pool
     * in memory: both the type table and the items.
     */
    cp_request = nconstants * (sizeof(char) + sizeof(cp_type));
    /* Allocate the space */
    context->cp_buffer = malloc(cp_request);
    if (context->cp_buffer == NULL) {
	CFnomem(context);
    }
    memset(context->cp_buffer, 0, cp_request);
    /* Divide up the space. */
    constant_pool = (cp_type *) context->cp_buffer;
    type_table = (unsigned char *)
        (context->cp_buffer + (nconstants * sizeof(cp_type)));

    context->constant_pool = constant_pool;
    context->type_table = type_table;
    context->size->constants = nconstants;

    /* Read in the constant pool */
    for (i = 1; i < nconstants; i++) {
	int type = get1byte(context);
	type_table[i] = type;
	switch (type) {
	  case JVM_CONSTANT_Utf8: {
	      constant_pool[i].s = getUTF8String(context);
	      break;
	  }
	
	  case JVM_CONSTANT_Class:
	  case JVM_CONSTANT_String:
	      constant_pool[i].i = get2bytes(context);
	      break;

	  case JVM_CONSTANT_Fieldref:
	  case JVM_CONSTANT_Methodref:
	  case JVM_CONSTANT_InterfaceMethodref:
	  case JVM_CONSTANT_NameAndType:
	      constant_pool[i].i = get4bytes(context);
	      break;
	    
	  case JVM_CONSTANT_Integer:
	  case JVM_CONSTANT_Float:
	      get4bytes(context);
	      break;

	  case JVM_CONSTANT_Long:
	  case JVM_CONSTANT_Double: {
	      get4bytes(context);
	      get4bytes(context);
	      i++;		/* increment i for loop, too */
	      /* A mangled type might cause you to overrun allocated memory */
	      if (i >= nconstants) {
		  CFerror(context, "Invalid constant pool entry");
	      }
	      break;
	  }

	  default:
	      CFerror(context, "Illegal constant pool type");
	}
    }
    verify_constant_pool(context, constant_pool, type_table, nconstants);
}

static void ParseCode(CFcontext *context, utf_str *mb_name, 
		      unsigned int args_size)
{
    unsigned int attribute_length = get4bytes(context);
    unsigned char *end_ptr = context->ptr + attribute_length;
    unsigned int attribute_count;
    unsigned int code_length;
    unsigned int i;
    unsigned int maxstack;
    unsigned int nlocals;

    if (context->major_version == 45 && context->minor_version <= 2) { 
	maxstack = get1byte(context);
	nlocals = get1byte(context);
	code_length = get2bytes(context);
    } else { 
	maxstack = get2bytes(context);
	nlocals = get2bytes(context);
	code_length = get4bytes(context);
    }

    if (context->in_clinit) {
        context->size->clinit.code += ALIGN_UP(code_length,sizeof(int));
    } else {
        context->size->main.code += ALIGN_UP(code_length,sizeof(int));
    }

    if (code_length > 65535) {
        CFerror(context, "Code of a method longer than 65535 bytes");
    }

    if (code_length == 0) {
        CFerror(context, "Code of a method has length 0"); 
    }

    if (nlocals < args_size)
        CFerror(context, "Arguments can't fit into locals");

    skipNbytes(context, code_length);
    i = get2bytes(context);
    if (context->in_clinit) {
        context->size->clinit.etab += i;
    } else {
        context->size->main.etab += i;
    }
    for (; i > 0; i--) {
        get2bytes(context); /* start_pc */
	get2bytes(context); /* end_pc */	
	get2bytes(context); /* handler_pc */
	get2bytes(context); /* CatchType */
    }

    attribute_count = get2bytes(context);
    for (i = 0; i < attribute_count; i++) {
        utf_str *attr_name = getAsciz(context);
	if (utfcmp(attr_name, "LineNumberTable") == 0) {
	    ParseLineTable(context, code_length);
	} else if (utfcmp(attr_name, "LocalVariableTable") == 0) {
	    ParseLocalVars(context, code_length);
	} else {
	    unsigned int length = get4bytes(context);
	    skipNbytes(context, length);
	}
    }

    if (context->ptr != end_ptr) 
	CFerror(context, "Code segment has wrong length");
}

static void
ParseLineTable(CFcontext *context, unsigned int code_length)
{
    unsigned int attribute_length = get4bytes(context);
    unsigned char *end_ptr = context->ptr  + attribute_length;
    unsigned int i, table_length;
    table_length = get2bytes(context);
    if (context->in_clinit) {
        context->size->clinit.lnum += table_length;
    } else {
        context->size->main.lnum += table_length;
    }
    for (i = 0; i < table_length; i++) {
        int pc = get2bytes(context);
	int line_number = get2bytes(context);
	if (pc >= code_length)
	    CFerror(context, "Invalid pc in line number table");
    }
    if (context->ptr != end_ptr)
	CFerror(context, "Line number table has wrong length");
    return;
}	

static void
ParseLocalVars(CFcontext *context, unsigned int code_length)
{
    unsigned int attribute_length = get4bytes(context);
    unsigned char *end_ptr = context->ptr  + attribute_length;
    unsigned int i, table_length;
    table_length = get2bytes(context);
    if (context->in_clinit) {
        context->size->clinit.lvar += table_length;
    } else {
        context->size->main.lvar += table_length;
    }
    for (i = 0; i < table_length; i++) {
        unsigned int pc0 = get2bytes(context);
	unsigned int length = get2bytes(context);
	unsigned int nameoff = get2bytes(context);
	unsigned int sigoff = get2bytes(context);
	unsigned int slot = get2bytes(context);
       	utf_str *name, *sig;
	/* Assign to a unsigned long to avoid overflow */
	unsigned long end_pc = (unsigned int)pc0 + (unsigned int)length;

	/* We do not check that pc0 indeed points to the 
	 * beginning of an instruction, or slot in fact holds
	 * this variable. That would require changes in the byte 
	 * code verifier.
	 */
	if (pc0 >= code_length || end_pc > code_length)
	    CFerror(context, "Invalid start_pc/length in local var table");

	verify_constant_entry(context, nameoff, JVM_CONSTANT_Utf8, 
			      "Local variable name");
	verify_constant_entry(context, sigoff, JVM_CONSTANT_Utf8,
			      "Local variable signature");

	name = context->constant_pool[nameoff].s;
	sig = context->constant_pool[sigoff].s;
	verify_legal_name(context, name, LegalVariable);
	verify_legal_field_signature(context, name, sig);
    }
    if (context->ptr != end_ptr)
	CFerror(context, "Local variables table has wrong length");
    return;
}	

static void
ParseExceptions(CFcontext *context)
{
    unsigned int attribute_length = get4bytes(context);
    unsigned char *end_ptr = context->ptr + attribute_length;
    unsigned int nexceptions = get2bytes(context);
    int i;

    if (context->in_clinit) {
        context->size->clinit.excs += nexceptions;
    } else {
        context->size->main.excs += nexceptions;
    }
    for (i = 0; i < nexceptions; i++) {
        unsigned int index = get2bytes(context);
	verify_constant_entry(context, index, JVM_CONSTANT_Class,
			      "Exception name");
    }
    if (context->ptr != end_ptr)
	CFerror(context, "Exceptions attribute has wrong length");
}
	
static unsigned int get1byte(CFcontext *context)
{
    unsigned char *ptr = context->ptr;
    unsigned int value;
    if (context->end_ptr - ptr < 1) {
	CFerror(context, "Truncated class file");
    }
    value = ptr[0];
    (context->ptr) += 1;
    return value;
}

static unsigned int get2bytes(CFcontext *context)
{
    unsigned char *ptr = context->ptr;
    unsigned int value;
    if (context->end_ptr - ptr < 2) {
	CFerror(context, "Truncated class file");
    }
    value = (ptr[0] << 8) + ptr[1];
    (context->ptr) += 2;
    return value;
}

static unsigned int get4bytes(CFcontext *context)
{
    unsigned char *ptr = context->ptr;
    unsigned int value;
    if (context->end_ptr - ptr < 4) {
	CFerror(context, "Truncated class file");
    }
    value = (ptr[0] << 24) + (ptr[1] << 16) + (ptr[2] << 8) + ptr[3];
    (context->ptr) += 4;
    return value;
}

static utf_str *getAsciz(CFcontext *context) 
{
    cp_type *constant_pool = context->constant_pool;
    unsigned char *type_table = context->type_table;
    int nconstants = context->size->constants;

    int value = get2bytes(context);
    if ((value == 0) || 
	(value >= nconstants) || 
	(type_table[value] != JVM_CONSTANT_Utf8))
	CFerror(context, "Illegal constant pool index");
    return constant_pool[value].s;
}

static utf_str *getAscizFromCP(CFcontext *context, unsigned int value) 
{
    cp_type *constant_pool = context->constant_pool;
    unsigned char *type_table = context->type_table;
    int nconstants = context->size->constants;

    if ((value <= 0) || (value >= nconstants) 
	|| (type_table[value] != JVM_CONSTANT_Class))
	CFerror(context, "Illegal constant pool index");
    value = constant_pool[value].i;
    if ((value <= 0) || (value >= nconstants) ||
	 (type_table[value] != JVM_CONSTANT_Utf8))
	CFerror(context, "Illegal constant pool index");
    return constant_pool[value].s;
}

static void skip2bytes(CFcontext *context)
{
    if (context->end_ptr - context->ptr < 2) {
	CFerror(context, "Truncated class file");
    }
    (context)->ptr += 2;
}

static void skip4bytes(CFcontext *context)
{
    if (context->end_ptr - context->ptr < 4) {
	CFerror(context, "Truncated class file");
    }
    context->ptr += 4;
}

static void skipNbytes(CFcontext *context, size_t n)
{
    if (context->end_ptr - context->ptr < n) {
	CFerror(context, "Truncated class file");
    }
    context->ptr += n;
}

static void skipAsciz(CFcontext *context)
{
    skip2bytes(context);
}

static utf_str * getUTF8String(CFcontext* context)
{
    char buf[256];
    utf_str *result = (utf_str *)context->ptr;
    skip2bytes(context);
    skipNbytes(context, UTF_LEN(result));
    verify_legal_utf8(context, result);
    return result;
}

static void
verify_constant_pool(CFcontext *context, 
		     cp_type *cp, 
		     unsigned char *type_table,
		     unsigned int cp_count)
{
    unsigned int i, type;
    
    if (context->measure_only)
        return;

    /* Let's make two quick passes over the constant pool. The first one 
     * checks that everything is of the right type. 
     */
    for (i = 1; i < cp_count; i++) {
        switch(type = type_table[i]) {
	case JVM_CONSTANT_String:
	case JVM_CONSTANT_Class: {
	    unsigned int index = cp[i].i;
	    if ((index <= 0) ||
		(index >= cp_count) ||
		(type_table[index] != JVM_CONSTANT_Utf8)) {
	        CFerror(context, "Bad index in constant pool #%d", i);
	    }
	    break;
	}
	
	case JVM_CONSTANT_Fieldref:
	case JVM_CONSTANT_Methodref:
	case JVM_CONSTANT_InterfaceMethodref: 
	case JVM_CONSTANT_NameAndType: {
	    unsigned int index = (unsigned)(cp[i].i);
	    unsigned int key1 = index >> 16;
	    unsigned int key2 = index & 0xFFFF;
	    if (key1 <= 0 || key1 >= cp_count 
		|| key2 <= 0 || key2 >= cp_count) {
	        CFerror(context, "Bad index in constant pool #%d", i);
	    }
	    if (type == JVM_CONSTANT_NameAndType) {
	        if ((type_table[key1] != JVM_CONSTANT_Utf8) ||
		    (type_table[key2] != JVM_CONSTANT_Utf8)) {
		    CFerror(context, "Bad index in constant pool.");
		}
	    } else {
	        if ((type_table[key1] != JVM_CONSTANT_Class) ||
		    (type_table[key2] != JVM_CONSTANT_NameAndType)) {
		    CFerror(context, "Bad index in constant pool #%d", i);
		}
	    }
	    break;
	}
	case JVM_CONSTANT_Utf8:
	case JVM_CONSTANT_Integer:
	case JVM_CONSTANT_Float:
	    break;
	    
	case JVM_CONSTANT_Long:
	case JVM_CONSTANT_Double:
	    if ((i + 1 >= cp_count) || 
		(type_table[i + 1] != 0)) {
	        CFerror(context, 
			"Improper constant pool long/double #%d", i);
	    } else {
	        i++;	
		break;	    
	    }
	    
	default:
	    CFerror(context, "Illegal constant pool type at #%d", i);
	}
    }
    /* The second pass checks the strings are of the right format */
    for (i = 1; i < cp_count; i++) {
	switch(type = type_table[i]) {
	case JVM_CONSTANT_Class: {
	    unsigned int index = cp[i].i;
	    verify_legal_name(context, cp[index].s, LegalClass); 
	    break;
	}
	      
	case JVM_CONSTANT_Fieldref:
	case JVM_CONSTANT_Methodref:
	case JVM_CONSTANT_InterfaceMethodref: {
	    unsigned int index = (unsigned)(cp[i].i);
	    unsigned int name_type_index = index & 0xFFFF;
	    unsigned int name_type_key = cp[name_type_index].i;
	    unsigned int name_index = name_type_key >> 16;
	    unsigned int signature_index = name_type_key & 0xFFFF;
	    utf_str *name = cp[name_index].s;
	    utf_str *signature = cp[signature_index].s;

	    if (type == JVM_CONSTANT_Fieldref) {
	        verify_legal_name(context, name, LegalField);
		verify_legal_field_signature(context, name, signature);
	    } else {
	        verify_legal_name(context, name, LegalMethod);
		verify_legal_method_signature(context, name, signature);

		/* If a method name begins with '<', it must be "<init>". */
		if (type == JVM_CONSTANT_Methodref) {
		    int utf_len = UTF_LEN(name);
		    if (utf_len > 0 && name->bytes[0] == '<') {
			if (utf_len != 6 || utfcmp(name, "<init>") != 0) {
			    CFerror(context,
				    "Bad method name in constant pool.");
			}
		    }
		}
	    }
	    break;
	}
	}
    }
}

static void 
verify_legal_class_modifiers(CFcontext *context, unsigned int access)
{
    if (context->measure_only)
        return;

    /* This is a workaround for a javac bug. Interfaces are not
     * marked as ABSTRACT.
     */
    if (access & JVM_ACC_INTERFACE) {
        access |= JVM_ACC_ABSTRACT;
    }

    if (access & JVM_ACC_INTERFACE) {
        if ((access & JVM_ACC_ABSTRACT) == 0)
	    goto failed;
	if (access & JVM_ACC_FINAL)
	    goto failed;
    } else {
        if ((access & JVM_ACC_FINAL) && (access & JVM_ACC_ABSTRACT))
	    goto failed;
    }
    return;

 failed:
    CFerror(context, "Illegal class modifiers: 0x%X", access);
}

static void 
verify_legal_field_modifiers(CFcontext *context, unsigned int access,
			     jboolean isIntf)
{
    if (context->measure_only)
        return;

    if (!isIntf) {
        /* class fields */
        if ((access & JVM_ACC_PUBLIC) && 
	    ((access & JVM_ACC_PROTECTED) || (access & JVM_ACC_PRIVATE)))
	    goto failed;
	if ((access & JVM_ACC_PROTECTED) && 
	    ((access & JVM_ACC_PUBLIC) || (access & JVM_ACC_PRIVATE)))
	    goto failed;
	if ((access & JVM_ACC_PRIVATE) && 
	    ((access & JVM_ACC_PROTECTED) || (access & JVM_ACC_PUBLIC)))
	    goto failed;
	if ((access & JVM_ACC_FINAL) && (access & JVM_ACC_VOLATILE))
	    goto failed;
    } else {
        /* interface fields */
        if (access & ~(JVM_ACC_STATIC | JVM_ACC_FINAL | JVM_ACC_PUBLIC))
	    goto failed;
        if (!(access & JVM_ACC_STATIC) ||
	    !(access & JVM_ACC_FINAL) ||
	    !(access & JVM_ACC_PUBLIC))
	    goto failed;
    }
    return;

 failed:
    CFerror(context, "Illegal field modifiers: 0x%X", access);
}

static void 
verify_legal_method_modifiers(CFcontext *context, unsigned int access, 
			      jboolean isIntf, utf_str *name)
{
    if (context->measure_only)
        return;

    if (!isIntf) {
        /* class or instance methods */
        if (utfcmp(name, "<init>") == 0) {
	    /* The STRICT bit is new as of 1.2. */
	    if (access & ~(JVM_ACC_PUBLIC
                            | JVM_ACC_PROTECTED
                            | JVM_ACC_PRIVATE
                            | JVM_ACC_STRICT))
	        goto failed;
	} else {
	    if (access & JVM_ACC_ABSTRACT) {
	        if ((access & JVM_ACC_FINAL) ||
	 	    (access & JVM_ACC_NATIVE) /* || 
	 	     This is commented out until after javac is fixed so that it 
		     rejects abstract synchronized methods.
		      (access & JVM_ACC_SYNCHRONIZED)*/)
		     goto failed;
	    }
	    if ((access & JVM_ACC_PRIVATE) && (access & JVM_ACC_ABSTRACT))
	        goto failed;
	    if ((access & JVM_ACC_STATIC) && (access & JVM_ACC_ABSTRACT))
	        goto failed;
	}		
    } else {
        /* interface methods */
        if (!(access & JVM_ACC_ABSTRACT) ||
	    !(access & JVM_ACC_PUBLIC) ||
            (access & JVM_ACC_STATIC) || 
            (access & JVM_ACC_FINAL) ||
            (access & JVM_ACC_NATIVE)) 
	    goto failed;	        
    }
   
    if ((access & JVM_ACC_PUBLIC) && 
	((access & JVM_ACC_PROTECTED) || (access & JVM_ACC_PRIVATE)))
        goto failed;
    if ((access & JVM_ACC_PROTECTED) && 
	((access & JVM_ACC_PUBLIC) || (access & JVM_ACC_PRIVATE)))
        goto failed;
    if ((access & JVM_ACC_PRIVATE) && 
	((access & JVM_ACC_PROTECTED) || (access & JVM_ACC_PUBLIC)))
        goto failed;
    return;

 failed:
    CFerror(context, "Illegal method modifiers: 0x%X", access);
}

static void
verify_legal_utf8(CFcontext *context, utf_str *str)
{
    unsigned int i;
    unsigned int length;
    unsigned char *bytes;

    if (context->measure_only)
        return;

    length = UTF_LEN(str);
    bytes = (unsigned char*)str->bytes;

    for (i=0; i<length; i++) {
        unsigned short c; 
        if (bytes[i] == 0) /* no embedded zeros */
	    goto failed;
	if (bytes[i] < 128)
	    continue;
        switch (bytes[i] >> 4) {
        default:
	    break;

	case 0x8: case 0x9: case 0xA: case 0xB: case 0xF:
	    goto failed;

	case 0xC: case 0xD:	
	    /* 110xxxxx  10xxxxxx */
	    c = (bytes[i] & 0x1F) << 6;
	    i++;
	    if (i >= length)
	        goto failed;
	    if ((bytes[i] & 0xC0) == 0x80) {
		c += bytes[i] & 0x3F;
                if (context->major_version <= 47 || c == 0 || c >= 0x80) {
		    /* verify c is null or a charater in its shortest form */
	            break;
 		}
	    }
	    goto failed;

	case 0xE:
	    /* 1110xxxx 10xxxxxx 10xxxxxx */
            c = (bytes[i] & 0xF) << 12;
	    i += 2;
	    if (i >= length)
	        goto failed;
	    if (((bytes[i-1] & 0xC0) == 0x80) &&
		((bytes[i] & 0xC0) == 0x80)) {
		c += ((bytes[i-1] & 0x3F) << 6) + (bytes[i] & 0x3F);
		if (context->major_version <= 47 || c >= 0x800) { 
		    /* verify c is in its shortest form */
	            break;
		}
	    }
	    goto failed;
	} /* end of switch */
    }
    return;
 failed:
    CFerror(context, "Illegal UTF8 string in constant pool");
}

/* Check if the entire second argument consists of a legal fieldname 
 * (or classname, if the third argument is LegalClass). 
 */

static void
verify_legal_name(CFcontext *context, utf_str *name, int type)
{
    jboolean result;
    unsigned int length;
    char *bytes;

    if (context->measure_only || context->check_relaxed)
        return;

    length = UTF_LEN(name);
    bytes = name->bytes;

    if (length > 0) {
        if (bytes[0] == '<') {
	    result = (type == LegalMethod) && 
	        ((utfcmp(name, "<init>") == 0) || 
		 (utfcmp(name, "<clinit>") == 0));
	} else {
	    char *p;
	    if (type == LegalClass && bytes[0] == JVM_SIGNATURE_ARRAY) {
	        p = skip_over_field_signature(bytes, JNI_FALSE, length);
	    } else {
	        p = skip_over_fieldname(bytes, 
		    (jboolean)(type == LegalClass ? JNI_TRUE : JNI_FALSE), 
					length);
	    }
	    result = (p != NULL) && (p - bytes == length);
	}
    } else {
        result = JNI_FALSE;
    }
    if (!result) {
        char buf[100];
	char *thing = (type == LegalField) ? "Field" 
	            : (type == LegalMethod) ? "Method" 
	            : (type == LegalVariable) ? "Variable" : "Class";
	utfncpy(buf, sizeof(buf), name);
	CFerror(context, "Illegal %s name \"%s\"", thing, buf);
    } 
}

/* Check if the entire string consists of a legal field signature */
static void
verify_legal_field_signature(CFcontext *context, utf_str *fieldname, 
			     utf_str *signature) 
{
    char *bytes;
    unsigned int length;
    char *p;

    if (context->measure_only)
        return;

    bytes = signature->bytes;
    length = UTF_LEN(signature);
    p = skip_over_field_signature(bytes, JNI_FALSE, length);

    if (p == NULL || p - bytes != length) {
        char buf1[100], buf2[100];
	utfncpy(buf1, sizeof(buf1), fieldname);
	utfncpy(buf2, sizeof(buf2), signature);
	CFerror(context, "Field \"%s\" has illegal signature \"%s\"", 
		buf1, buf2);
    }
}


static unsigned int
verify_legal_method_signature(CFcontext *context, utf_str *methodname, 
			      utf_str *signature)
{
    unsigned int args_size;
    char *p;
    unsigned length;
    char *next_p;

    if (context->measure_only)
        return 0;  /* REMIND: we return 0 as the args_size when no checking
		    * is necessary. Not a problem currently. May become a 
		    * problem when the caller starts to make other uses
		    * of args_size.
		    */

    args_size = 0;
    p = signature->bytes;
    length = UTF_LEN(signature);

    /* The first character must be a '(' */
    if ((length > 0) && (*p++ == JVM_SIGNATURE_FUNC)) {
        length--;
	/* Skip over however many legal field signatures there are */
	while ((length > 0) &&
	       (next_p = skip_over_field_signature(p, JNI_FALSE, length))) {
	    args_size++;
	    if (p[0] == 'J' || p[0] == 'D')
	        args_size++;
	    length -= (next_p - p);
	    p = next_p;
	}
	/* The first non-signature thing better be a ')' */
	if ((length > 0) && (*p++ == JVM_SIGNATURE_ENDFUNC)) {
	    length --;
	    if (UTF_LEN(methodname) > 0 && methodname->bytes[0] == '<') {
		/* All internal methods must return void */
		if ((length == 1) && (p[0] == JVM_SIGNATURE_VOID))
		    return args_size;
	    } else {
		/* Now, we better just have a return value. */
		next_p =  skip_over_field_signature(p, JNI_TRUE, length);
		if (next_p && (length == next_p - p))
		    return args_size;
	    }
	}
    }
    {
        char buf1[100], buf2[100];
	utfncpy(buf1, sizeof(buf1), methodname);
	utfncpy(buf2, sizeof(buf2), signature);
	CFerror(context, "Method \"%s\" has illegal signature \"%s\"", 
		buf1, buf2);
	return 0; /* never reached */
    }
}

static void
verify_constant_entry(CFcontext *context, unsigned int index, int type,
		      char *msg)
{
    cp_type *constant_pool;
    unsigned char *type_table;
    int nconstants;

    if (context->measure_only)
        return;

    constant_pool = context->constant_pool;
    type_table = context->type_table;
    nconstants = context->size->constants;

    if (index <= 0 || index >= nconstants) 
        CFerror(context, "%s has bad constant pool index", msg);

    if (type_table[index] != type)
        CFerror(context, "%s has bad constant type", msg);
}

  /* The following tables and code generated using: */
  /* java GenerateCharacter -verbose -identifiers -c -spec UnicodeData.txt -template check_class.c.template -o check_class.c [8 4 4] */
  /* The X table has 256 entries for a total of 256 bytes. */

  static unsigned char X[256] = {
      0,   1,   2,   3,   4,   5,   6,   7,  /* 0x0000 */
      8,   9,  10,  11,  12,  13,  14,  15,  /* 0x0800 */
     16,  17,  18,  19,  20,   1,  21,  22,  /* 0x1000 */
     23,   8,   8,   8,   8,   8,  24,  25,  /* 0x1800 */
     26,  27,   8,   8,   8,   8,   8,   8,  /* 0x2000 */
      8,   8,   8,   8,   8,   8,   8,   8,  /* 0x2800 */
     28,  29,   8,   8,   1,   1,   1,   1,  /* 0x3000 */
      1,   1,   1,   1,   1,   1,   1,   1,  /* 0x3800 */
      1,   1,   1,   1,   1,   1,   1,   1,  /* 0x4000 */
      1,   1,   1,   1,   1,  30,   1,   1,  /* 0x4800 */
      1,   1,   1,   1,   1,   1,   1,   1,  /* 0x5000 */
      1,   1,   1,   1,   1,   1,   1,   1,  /* 0x5800 */
      1,   1,   1,   1,   1,   1,   1,   1,  /* 0x6000 */
      1,   1,   1,   1,   1,   1,   1,   1,  /* 0x6800 */
      1,   1,   1,   1,   1,   1,   1,   1,  /* 0x7000 */
      1,   1,   1,   1,   1,   1,   1,   1,  /* 0x7800 */
      1,   1,   1,   1,   1,   1,   1,   1,  /* 0x8000 */
      1,   1,   1,   1,   1,   1,   1,   1,  /* 0x8800 */
      1,   1,   1,   1,   1,   1,   1,   1,  /* 0x9000 */
      1,   1,   1,   1,   1,   1,   1,  31,  /* 0x9800 */
      1,   1,   1,   1,  32,   8,   8,   8,  /* 0xA000 */
      8,   8,   8,   8,   1,   1,   1,   1,  /* 0xA800 */
      1,   1,   1,   1,   1,   1,   1,   1,  /* 0xB000 */
      1,   1,   1,   1,   1,   1,   1,   1,  /* 0xB800 */
      1,   1,   1,   1,   1,   1,   1,   1,  /* 0xC000 */
      1,   1,   1,   1,   1,   1,   1,   1,  /* 0xC800 */
      1,   1,   1,   1,   1,   1,   1,  33,  /* 0xD000 */
      8,   8,   8,   8,   8,   8,   8,   8,  /* 0xD800 */
      8,   8,   8,   8,   8,   8,   8,   8,  /* 0xE000 */
      8,   8,   8,   8,   8,   8,   8,   8,  /* 0xE800 */
      8,   8,   8,   8,   8,   8,   8,   8,  /* 0xF000 */
      8,   1,  34,  35,   1,  36,  37,  38   /* 0xF800 */
  };

  /* The Y table has 624 entries for a total of 624 bytes. */

  static unsigned char Y[624] = {
      0,   0,   1,   2,   3,   4,   3,   5,  /*   0 */
      0,   0,   6,   7,   8,   9,   8,   9,  /*   0 */
      8,   8,   8,   8,   8,   8,   8,   8,  /*   1 */
      8,   8,   8,   8,   8,   8,   8,   8,  /*   1 */
      8,   8,  10,  11,   0,   8,   8,   8,  /*   2 */
      8,   8,  12,  13,  14,  14,  15,   0,  /*   2 */
     16,  16,  16,  16,  17,   0,  18,  19,  /*   3 */
     20,   8,  21,   8,  22,  23,   8,  11,  /*   3 */
      8,   8,   8,   8,   8,   8,   8,   8,  /*   4 */
     24,   8,   8,   8,  25,   8,   8,  26,  /*   4 */
      0,   0,   0,   3,   8,  27,   3,   8,  /*   5 */
     28,  29,  30,  31,  32,   8,   5,  33,  /*   5 */
      0,   0,   3,   5,  34,  35,   2,  36,  /*   6 */
      8,   8,   8,   8,   8,  37,  38,  39,  /*   6 */
      0,  40,  41,  16,  42,   0,   0,   0,  /*   7 */
      8,   8,  43,  44,   0,   0,   0,   0,  /*   7 */
      0,   0,   0,   0,   0,   0,   0,   0,  /*   8 */
      0,   0,   0,   0,   0,   0,   0,   0,  /*   8 */
     45,   8,   8,  46,  47,  48,  49,   0,  /*   9 */
     50,  51,  52,  53,  54,  55,  49,  11,  /*   9 */
     56,  51,  52,  57,  58,  59,  60,  61,  /*  10 */
     62,  21,  52,  63,  64,  65,  66,   0,  /*  10 */
     50,  51,  52,  67,  68,  69,  70,   0,  /*  11 */
     71,  72,  73,  74,  75,  76,  77,   0,  /*  11 */
     78,  79,  52,  80,  81,  82,  70,   0,  /*  12 */
     83,  79,  52,  80,  81,  84,  70,   0,  /*  12 */
     83,  79,  52,  85,  86,  76,  70,   0,  /*  13 */
     87,  88,   8,  89,  90,  91,   0,  92,  /*  13 */
      3,   8,   8,  93,  94,   2,   0,   0,  /*  14 */
     95,  96,  97,  98,  99, 100,   0,   0,  /*  14 */
     65, 101,   2, 102, 103,   8,   5,  29,  /*  15 */
    104, 105,  16, 106, 107,   0,   0,   0,  /*  15 */
      8,   8, 108, 109,   2, 110,   0,   0,  /*  16 */
      0,   0,   8,   8, 111,   8,   8, 112,  /*  16 */
      8,   8,   8,   8,   8, 113,   8,   8,  /*  17 */
      8,   8, 114,   8,   8,   8,   8, 115,  /*  17 */
      9,   8,   8,   8, 116, 116,   8,   8,  /*  18 */
    116,   8,  22, 117, 117,   9,  22,   8,  /*  18 */
     22, 117,   8,   8,   9,   5, 118, 119,  /*  19 */
      0,   0,   8,   8,   8,   8,   8, 120,  /*  19 */
      3,   8,   8,   8,   8,   8,   8,   8,  /*  20 */
      8,   8,   8,   8,   8,   8,   8,   8,  /*  20 */
      8,   8,   8,   8,   8,   8, 121, 112,  /*  21 */
      3,   5,   8,   8,   8,   8,   5,   0,  /*  21 */
      0,   0,   0,   0,   0,   0,   0,   0,  /*  22 */
      8,   8,   8, 122,  16, 123,   2,   0,  /*  22 */
      0,   2,   8,   8,   8,   8,   8,  28,  /*  23 */
      8,   8, 124,   0,   0,   0,   0,   0,  /*  23 */
      8,   8,   8,   8,   8,   8,   8,   8,  /*  24 */
      8, 125,   8,   8,   8,   8,   8, 115,  /*  24 */
      8, 126,   8,   8, 126, 127,   8,  12,  /*  25 */
      8,   8,   8, 128, 129, 130,  41, 129,  /*  25 */
      0,   0,   0, 131,  65,   0,   0, 131,  /*  26 */
      0,   0,   8,   0,   0, 106, 132,   0,  /*  26 */
    133, 134, 135, 136,   0,   0,   8,   8,  /*  27 */
     11,   0,   0,   0,   0,   0,   0,   0,  /*  27 */
    137,   0, 138, 139,   3,   8,   8,   8,  /*  28 */
      8, 140,   3,   8,   8,   8,   8,  22,  /*  28 */
    141,   8,  41,   3,   8,   8,   8,   8,  /*  29 */
     22,   0,   8,  28,   0,   0,   0,   0,  /*  29 */
      8,   8,   8,   8,   8,   8,   8,   8,  /*  30 */
      8,   8,   8, 111,   0,   0,   0,   0,  /*  30 */
      8,   8,   8,   8,   8,   8,   8,   8,  /*  31 */
      8,   8, 111,   0,   0,   0,   0,   0,  /*  31 */
      8,   8,   8,   8,   8,   8,   8,   8,  /*  32 */
     41,   0,   0,   0,   0,   0,   0,   0,  /*  32 */
      8,   8,   8,   8,   8,   8,   8,   8,  /*  33 */
      8,   8,  11,   0,   0,   0,   0,   0,  /*  33 */
      8,   8,  12,   0,   0,   0,   0,   0,  /*  34 */
      0,   0,   0,   0,   0,   0,   0,   0,  /*  34 */
    112, 142,  52, 143, 144,   8,   8,   8,  /*  35 */
      8,   8,   8,  14,   0, 145,   8,   8,  /*  35 */
      8,   8,   8,  12,   0,   8,   8,   8,  /*  36 */
      8,  10,   8,   8,  28,   0,   0, 125,  /*  36 */
      0,   0, 146, 147, 148,   0, 149, 150,  /*  37 */
      8,   8,   8,   8,   8,   8,   8,  41,  /*  37 */
      1,   2,   3,   4,   3,   5, 141,   8,  /*  38 */
      8,   8,   8,  22, 151, 152, 153,   0   /*  38 */
  };

  /* The A table has 2464 entries for a total of 616 bytes. */

  static unsigned int A[154] = {
    0x00000000,  /*   0 */
    0x00000300,  /*   1 */
    0x00055555,  /*   2 */
    0xFFFFFFFC,  /*   3 */
    0xC03FFFFF,  /*   4 */
    0x003FFFFF,  /*   5 */
    0x00300FF0,  /*   6 */
    0x00300C00,  /*   7 */
    0xFFFFFFFF,  /*   8 */
    0xFFFF3FFF,  /*   9 */
    0xFFFFFFF0,  /*  10 */
    0x000000FF,  /*  11 */
    0x0FFFFFFF,  /*  12 */
    0xFFC3FFFF,  /*  13 */
    0x0000000F,  /*  14 */
    0x300003FF,  /*  15 */
    0x55555555,  /*  16 */
    0x15555555,  /*  17 */
    0x00000015,  /*  18 */
    0x00300000,  /*  19 */
    0xF33F3000,  /*  20 */
    0xFFFFFFCF,  /*  21 */
    0x3FFFFFFF,  /*  22 */
    0xFFF0FFFF,  /*  23 */
    0xFF00154F,  /*  24 */
    0x03C3C3FF,  /*  25 */
    0x000F0FFF,  /*  26 */
    0x000C3FFF,  /*  27 */
    0x0000FFFF,  /*  28 */
    0x55555554,  /*  29 */
    0x55555545,  /*  30 */
    0x45455555,  /*  31 */
    0x00000114,  /*  32 */
    0x0000003F,  /*  33 */
    0x557FFFFF,  /*  34 */
    0x00000555,  /*  35 */
    0xFFFFFFFD,  /*  36 */
    0x41555CFF,  /*  37 */
    0x05517D55,  /*  38 */
    0x03F55555,  /*  39 */
    0xFFFFFFF7,  /*  40 */
    0x03FFFFFF,  /*  41 */
    0x00155555,  /*  42 */
    0x55555FFF,  /*  43 */
    0x00000001,  /*  44 */
    0xFFFFFC54,  /*  45 */
    0x5D0FFFFF,  /*  46 */
    0x05555555,  /*  47 */
    0xFFFF0157,  /*  48 */
    0x5555505F,  /*  49 */
    0xC3FFFC54,  /*  50 */
    0xFFFFFFC3,  /*  51 */
    0xFFF3FFFF,  /*  52 */
    0x510FF033,  /*  53 */
    0x05414155,  /*  54 */
    0xCF004000,  /*  55 */
    0xC03FFC10,  /*  56 */
    0x510F3CF3,  /*  57 */
    0x05414015,  /*  58 */
    0x33FC0000,  /*  59 */
    0x55555000,  /*  60 */
    0x000003F5,  /*  61 */
    0xCCFFFC54,  /*  62 */
    0x5D0FFCF3,  /*  63 */
    0x05454555,  /*  64 */
    0x00000003,  /*  65 */
    0x55555003,  /*  66 */
    0x5D0FF0F3,  /*  67 */
    0x05414055,  /*  68 */
    0xCF005000,  /*  69 */
    0x5555500F,  /*  70 */
    0xF03FFC50,  /*  71 */
    0xF33C0FF3,  /*  72 */
    0xF03F03C0,  /*  73 */
    0x500FCFFF,  /*  74 */
    0x05515015,  /*  75 */
    0x00004000,  /*  76 */
    0x55554000,  /*  77 */
    0xF3FFFC54,  /*  78 */
    0xFFFFFFF3,  /*  79 */
    0x500FFCFF,  /*  80 */
    0x05515155,  /*  81 */
    0x00001400,  /*  82 */
    0xF3FFFC50,  /*  83 */
    0x30001400,  /*  84 */
    0x500FFFFF,  /*  85 */
    0x05515055,  /*  86 */
    0xFFFFFC50,  /*  87 */
    0xFFF03FFF,  /*  88 */
    0x0CFFFFCF,  /*  89 */
    0x40103FFF,  /*  90 */
    0x55551155,  /*  91 */
    0x00000050,  /*  92 */
    0xC01555F7,  /*  93 */
    0x15557FFF,  /*  94 */
    0x0C33C33C,  /*  95 */
    0xFFFCFF00,  /*  96 */
    0xFCF0CCFC,  /*  97 */
    0x0D4555F7,  /*  98 */
    0x055533FF,  /*  99 */
    0x0F055555,  /* 100 */
    0x00050000,  /* 101 */
    0x50044400,  /* 102 */
    0xFFFCFFFF,  /* 103 */
    0x00FF5155,  /* 104 */
    0x55545555,  /* 105 */
    0x01555555,  /* 106 */
    0x00001000,  /* 107 */
    0x553CFFCF,  /* 108 */
    0x00055015,  /* 109 */
    0x00055FFF,  /* 110 */
    0x00000FFF,  /* 111 */
    0x00003FFF,  /* 112 */
    0xC00FFFFF,  /* 113 */
    0xFFFF003F,  /* 114 */
    0x000FFFFF,  /* 115 */
    0x0FF33FFF,  /* 116 */
    0x3FFF0FF3,  /* 117 */
    0x55540000,  /* 118 */
    0x00000005,  /* 119 */
    0x000003FF,  /* 120 */
    0xC3FFFFFF,  /* 121 */
    0x555555FF,  /* 122 */
    0x00C00055,  /* 123 */
    0x0007FFFF,  /* 124 */
    0x00FFFFFF,  /* 125 */
    0x0FFF0FFF,  /* 126 */
    0xCCCCFFFF,  /* 127 */
    0x33FFF3FF,  /* 128 */
    0x03FFF3F0,  /* 129 */
    0x00FFF0FF,  /* 130 */
    0xC0000000,  /* 131 */
    0x00000004,  /* 132 */
    0xFFF0C030,  /* 133 */
    0x0FFC0CFF,  /* 134 */
    0xCFF33300,  /* 135 */
    0x000FFFCF,  /* 136 */
    0x0000FC00,  /* 137 */
    0x555FFFFC,  /* 138 */
    0x003F0FFC,  /* 139 */
    0x3C1403FF,  /* 140 */
    0xFFFFFC00,  /* 141 */
    0xDC00FFC0,  /* 142 */
    0x33FF3FFF,  /* 143 */
    0xFFFFF3CF,  /* 144 */
    0xFFFFFFC0,  /* 145 */
    0x00000055,  /* 146 */
    0x000003C0,  /* 147 */
    0xFC000000,  /* 148 */
    0x000C0000,  /* 149 */
    0xFFFFF33F,  /* 150 */
    0xFFF0FFF0,  /* 151 */
    0x03F0FFF0,  /* 152 */
    0x00003C0F   /* 153 */
  };

  /* In all, the character property tables require 1496 bytes. */

/*
 * This code mirrors Character.isJavaIdentifierStart.  It determines whether
 * the specified character is a legal start of a Java identifier as per JLS.
 *
 * The parameter ch is the character to be tested; return 1 if the
 * character is a letter, 0 otherwise.
 */
#define isJavaIdentifierStart(ch) (((A[Y[(X[ch>>8]<<4)|((ch>>4)&0xF)]]>>((ch&0xF)<<1))&3) & 0x2)

/*
 * This code mirrors Character.isJavaIdentifierPart.  It determines whether
 * the specified character is a legal part of a Java identifier as per JLS.
 *
 * The parameter ch is the character to be tested; return 1 if the
 * character is a digit, 0 otherwise.
 */
#define isJavaIdentifierPart(ch) (((A[Y[(X[ch>>8]<<4)|((ch>>4)&0xF)]]>>((ch&0xF)<<1))&3) & 0x1)

static unicode 
next_utf2unicode(char **utfstring_ptr) 
{
    unsigned char *ptr = (unsigned char *)(*utfstring_ptr);
    unsigned char ch, ch2, ch3;
    int length = 1;		/* default length */
    unicode result = 0x80;	/* default bad result; */
    switch ((ch = ptr[0]) >> 4) {
        default:
	    result = ch;
	    break;

	case 0x8: case 0x9: case 0xA: case 0xB: case 0xF:
	    /* Shouldn't happen. */
	    break;

	case 0xC: case 0xD:	
	    /* 110xxxxx  10xxxxxx */
	    if (((ch2 = ptr[1]) & 0xC0) == 0x80) {
		unsigned char high_five = ch & 0x1F;
		unsigned char low_six = ch2 & 0x3F;
		result = (high_five << 6) + low_six;
		length = 2;
	    } 
	    break;

	case 0xE:
	    /* 1110xxxx 10xxxxxx 10xxxxxx */
	    if (((ch2 = ptr[1]) & 0xC0) == 0x80) {
		if (((ch3 = ptr[2]) & 0xC0) == 0x80) {
		    unsigned char high_four = ch & 0x0f;
		    unsigned char mid_six = ch2 & 0x3f;
		    unsigned char low_six = ch3 & 0x3f;
		    result = (((high_four << 6) + mid_six) << 6) + low_six;
		    length = 3;
		} else {
		    length = 2;
		}
	    }
	    break;
	} /* end of switch */

    *utfstring_ptr = (char *)(ptr + length);
    return result;
}

/* Take pointer to a string.  Skip over the longest part of the string that
 * could be taken as a fieldname.  Allow '/' if slash_okay is JNI_TRUE.
 *
 * Return a pointer to just past the fieldname.  Return NULL if no fieldname
 * at all was found, or in the case of slash_okay being true, we saw
 * consecutive slashes (meaning we were looking for a qualified path but
 * found something that was badly-formed). 
 */
static char *
skip_over_fieldname(char *name, jboolean slash_okay, 
		    unsigned int length)
{
    char *p;
    unicode ch;
    unicode last_ch = 0;
    /* last_ch == 0 implies we are looking at the first char. */
    for (p = name; p != name + length; last_ch = ch) {
	char *old_p = p;
	ch = *p;
	if (ch < 128) {
            p++;
	    /* quick check for ascii */
	    if ((ch >= 'a' && ch <= 'z') ||
		(ch >= 'A' && ch <= 'Z') ||
		(last_ch && ch >= '0' && ch <= '9')) {
	        continue;
	    }
	} else {
	    char *tmp_p = p;
	    ch = next_utf2unicode(&tmp_p);
	    p = tmp_p;
	    if (isJavaIdentifierStart(ch) || (last_ch && isJavaIdentifierPart(ch))) {
			continue;
	    }
	}

	if (slash_okay && ch == '/' && last_ch) {
	    if (last_ch == '/') {
		return 0;	/* Don't permit consecutive slashes */
	    }
	} else if (ch == '_' || ch == '$') {
	} else {
	    return last_ch ? old_p : 0;
	}
    }
    return last_ch ? p : 0;
}

/* Take pointer to a string.  Skip over the longest part of the string that
 * could be taken as a field signature.  Allow "void" if void_okay.
 *
 * Return a pointer to just past the signature.  Return NULL if no legal
 * signature is found.
 */

static char *
skip_over_field_signature(char *name, jboolean void_okay, 
			  unsigned int length)
{
    for (;length > 0;) {
	switch (name[0]) {
            case JVM_SIGNATURE_VOID:
		if (!void_okay) return 0;
		/* FALL THROUGH */
            case JVM_SIGNATURE_BOOLEAN:
            case JVM_SIGNATURE_BYTE:
            case JVM_SIGNATURE_CHAR:
            case JVM_SIGNATURE_SHORT:
            case JVM_SIGNATURE_INT:
            case JVM_SIGNATURE_FLOAT:
            case JVM_SIGNATURE_LONG:
            case JVM_SIGNATURE_DOUBLE:
		return name + 1;

	    case JVM_SIGNATURE_CLASS: {
		/* Skip over the classname, if one is there. */
		char *p = 
		    skip_over_fieldname(name + 1, JNI_TRUE, --length);
		/* The next character better be a semicolon. */
		if (p && p - name - 1 > 0 && p[0] == ';') 
		    return p + 1;
		return 0;
	    }
	    
	    case JVM_SIGNATURE_ARRAY: 
		/* The rest of what's there better be a legal signature.  */
		name++;
		length--;
		void_okay = JNI_FALSE;
		break;

	    default:
		return 0;
	}
    }
    return 0;
}

static void
CFerror(CFcontext *context, char *format, ...)
{
    va_list args;
    int n = 0;
    va_start(args, format);
    if (context->class_name)
        n = jio_snprintf(context->msg, context->msg_len, "%s (", 
			 context->class_name);
    /* 4417214: make sure buffer size is positive */
    if (n >= 0 && n < context->msg_len) {
      n += jio_vsnprintf(context->msg + n, context->msg_len - n, format, args);
    }
    if (context->class_name && n >= 0 && n < context->msg_len)
        jio_snprintf(context->msg + n, context->msg_len - n, ")");
    va_end(args);
    context->err_code = CF_ERR;
    longjmp(context->jump_buffer, 1);
}

static void
CNerror(CFcontext *context, char *name)
{
    jio_snprintf(context->msg, context->msg_len, "%s (wrong name: %s)", 
		 context->class_name, name);
    context->err_code = CF_BADNAME;
    longjmp(context->jump_buffer, 1);
}

static void
UVerror(CFcontext *context)
{
    int n = 0;
    if (context->class_name)
        n = jio_snprintf(context->msg, context->msg_len, "%s (", 
			 context->class_name);
    /* 4417214: make sure buffer size is positive */
    if (n >= 0 && n < context->msg_len) {
        n += jio_snprintf(context->msg + n, context->msg_len - n, "Unsupported major.minor version %d.%d", context->major_version, context->minor_version);
    }
    if (context->class_name && n >= 0 && n < context->msg_len)
        jio_snprintf(context->msg + n, context->msg_len - n, ")");
    context->err_code = CF_UVERR;
    longjmp(context->jump_buffer, 1);
}

static void
CFnomem(CFcontext *context)
{
    context->err_code = CF_NOMEM;
    longjmp(context->jump_buffer, 1);
}

/* Used in java/lang/Class.c */
/* Determine if the specified name is legal
 * UTF name for a classname.
 *
 * Note that this routine expects the internal form of qualified classes:
 * the dots should have been replaced by slashes.
 */
JNIEXPORT jboolean
VerifyClassname(char *name, jboolean allowArrayClass) 
{ 
    unsigned int length = strlen(name);
    char *p;
    if (length > 0 && name[0] == JVM_SIGNATURE_ARRAY) {
	if (!allowArrayClass) {
	    return JNI_FALSE;
	} else { 
	    /* Everything that's left better be a field signature */
	    p = skip_over_field_signature(name, JNI_FALSE, length);
	}
    } else {
	/* skip over the fieldname.  Slashes are okay */
	p = skip_over_fieldname(name, JNI_TRUE, length);
    }
    return (p != 0 && p - name == length);
}

/*
 * Translates '.' to '/'.  Returns JNI_TRUE is any / were present.
 */
JNIEXPORT jboolean
VerifyFixClassname(char *name)
{
    char *p = name;
    jboolean slashesFound = JNI_FALSE;
    
    while (*p != '\0') {
	if (*p == '/') {
	    slashesFound = JNI_TRUE;
	    p++;
	} else if (*p == '.') {
	    *p++ = '/';
	} else {
            next_utf2unicode(&p);
        }
    }
    return slashesFound;
}

static int utfcmp(utf_str *ustr, char *cstr)
{
    int i;
    int length = UTF_LEN(ustr);
    char *bytes = ustr->bytes;
    for (i=0; i < length; i++) {
        int c1 = bytes[i];
	int c2 = cstr[i];
	if (c1 > c2) /* this handles the case when c2 is zero. */
	    return 1;
	if (c1 < c2)
	    return -1;
    }
    if (cstr[i] == 0)
        return 0;
    return -1;
}

static char * utfncpy(char *buf, int buf_len, utf_str *ustr)
{
    int i;
    int length = UTF_LEN(ustr);
    char *bytes = ustr->bytes;
    for (i = 0; i < length && i < buf_len - 1; i++) {
	buf[i] = bytes[i];
    }
    buf[i] = 0;
    return buf;
}

/*
 * this routine compare two name/signature pairs.
 */
static int uucmp2(utf_str *name1, utf_str *signature1,
                  utf_str *name2, utf_str *signature2)
{
    int len1 = UTF_LEN(name1);
    int len2 = UTF_LEN(name2);
    int result;

    if (len1 < len2) {
        return -1;
    }
    if (len1 > len2) {
        return 1;
    }

    result = strncmp(name1->bytes, name2->bytes, len1);
    if (result) {
        return result;
    }

    if (signature1 == 0 || signature2 == 0) {
        return 0;
    }

    len1 = UTF_LEN(signature1);
    len2 = UTF_LEN(signature2);

    if (len1 < len2) {
        return -1;
    }
    if (len1 > len2) {
        return 1;
    }

    return strncmp(signature1->bytes, signature2->bytes, len1);
}

/*
 * This is classic heap adjustment routine used for
 * sort the name/signature array.
 * Calculate the first and second child. It assumes the 
 * classic array representation of binary tree.
 *
 * Create heap out of the name/signature array. In place.
 */
static void HeapAdjust(utf_str* array[], int first, int last, int init)
{
    int firstChild = (first << 1) + 2;
    int secondChild = firstChild + 2;
    /* Read the current node to locals, for performance. */
    utf_str* a0 = array[first];
    utf_str* a1 = array[first+1];
    utf_str* b0;
    utf_str* b1;
    utf_str* c0;
    utf_str* c1;
    /* Test if it has left child node */
    if (firstChild < last) {
        if (init) {
            /* Recursive make heap out of left subtree. */
            HeapAdjust(array, firstChild, last, 1);
        }
        /* Read current and left node to local for better performance */
        b0 = array[firstChild];
        b1 = array[firstChild+1];
        /* Test if it has right child node */
        if (secondChild < last) {
            if (init) {
                /* Recursieve make heap out of right subtree */
                HeapAdjust(array, secondChild, last, 1);
            }
            /* Read right node to local for better performance */
            c0 = array[secondChild];
            c1 = array[secondChild+1];
            if (uucmp2(b0, b1, a0, a1) < 0) {
                /* the left cild is less than current */
                if (uucmp2(b0, b1, c0, c1) < 0) {
                    /* left child is the least, swap it */
                    array[first] = b0;
                    array[first+1] = b1;
                    array[firstChild] = a0;
                    array[firstChild+1] = a1;
                    /* adjust heap after any swap */
                    HeapAdjust(array, firstChild, last, 0);
                } else {
                    /* right child is less than left */
                    /* right is the least, swap it with current */
                    array[first] = c0;
                    array[first+1] = c1;
                    array[secondChild] = a0;
                    array[secondChild+1] = a1;
                    /* adjust heap after any swap */
                    HeapAdjust(array, secondChild, last, 0);
                }
            } else if (uucmp2(c0, c1, a0, a1) < 0) {
                /* If right is less than current, swap it */
                array[first] = c0;
                array[first+1] = c1;
                array[secondChild] = a0;
                array[secondChild+1] = a1;
                /* adjust heap after any swap */
                HeapAdjust(array, secondChild, last, 0);
            } /* current is the least, it is done */
        } else if (uucmp2(b0, b1, a0, a1) < 0) {
            /* there is only left child, no right one */
            /* If left is less than current, swap it */
            array[first] = b0;
            array[first+1] = b1;
            array[firstChild] = a0;
            array[firstChild+1] = a1;
            /* adjust heap after any swap */
            HeapAdjust(array, firstChild, last, 0);
        } /* current is less than left, no right, DONE. */
    } /* No children at all, DONE. */
}

/*
 * This is a customized heap sort for sorting the
 * array of name/signature pair. It first create
 * the heap, then pop the root and put it at the
 * end of heap, and readjust the heap. Repeat 
 * untill there is only one element left. The heap
 * order assume the root is the first in lexical order.
 * After sorting, the order is decrement.
 */
static void HeapSort(utf_str* array[], int length)
{
    utf_str* tmp0;
    utf_str* tmp1;
    /* Create the heap for name/signature array. */
    HeapAdjust(array, 0, length, 1);
    /* Sort the heap */
    while (length > 2) {
        /* decrement heap size */
        length -= 2;
        /* Pop the root */
        tmp0 = array[0];
        tmp1 = array[1];
        /* Move the last element to root */
        array[0] = array[length];
        array[1] = array[length+1];
        /* Put the old root BEYOND the end of heap */
        array[length] = tmp0;
        array[length+1] = tmp1;
        /* Readjust the heap */
        HeapAdjust(array, 0, length, 0);
    }
}

/*
 * The function return 1 if it finds any duplications,
 * 0 otherwise. The algorithm used here is heap sort.
 * After sorting the name/signature array, it will do
 * comparison of adjacent entry looking for duplication.
 * The sorting is done in place, and complexity is O(n log n);
 * Note:
 *   For interface, the signature will just be null pointer.
 */
static int FindDup(utf_str* array[], int length)
{
    int i;
    HeapSort(array, length);
    for (i = 0; i < length - 2; i += 2) {
        if (uucmp2(array[i], array[i+1], array[i+2], array[i+3]) == 0) {
            return 1;
        }
    }
    return 0;
}
