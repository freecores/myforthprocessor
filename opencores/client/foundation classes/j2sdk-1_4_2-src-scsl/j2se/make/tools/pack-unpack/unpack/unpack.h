/*
 * @(#)unpack.h	1.3 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
 
// Global Structures
struct cpool;
struct entry;
struct package;
struct klass;
struct member;
struct code;
struct attribute;


// Prototypes of functions
void read_pkgfile();
void write_file(char* classfilename, int nsegs, bytes* segs);
void write_classfiles();
void write_classfile_head(klass& k);
void write_classfile_tail(klass& k);
void write_sidefiles();

void mkdirs(int oklen, char* path);

void syntax(char *);
int unpack(char *, char*);
void getArgs();
int run(int argc, char **argv);

// tags, in canonical order:
static const byte TAGS_IN_ORDER[] = {
  CONSTANT_Utf8,
  CONSTANT_Integer,
  CONSTANT_Float,
  CONSTANT_Long,
  CONSTANT_Double,
  CONSTANT_String,
  CONSTANT_Class,
  CONSTANT_Signature,
  CONSTANT_NameandType,
  CONSTANT_Fieldref,
  CONSTANT_Methodref,
  CONSTANT_InterfaceMethodref
};
#define N_TAGS_IN_ORDER (sizeof TAGS_IN_ORDER)
static const byte TAG_ORDER[CONSTANT_Limit] = {
  0, 1, 0, 2, 3, 4, 5, 7, 6, 10, 11, 12, 9, 8
};

#ifndef PRODUCT
static const char* TAG_NAME[] = {
  "*None",
  "Utf8",
  "*Unicode",
  "Integer",
  "Float",
  "Long",
  "Double",
  "Class",
  "String",
  "Fieldref",
  "Methodref",
  "InterfaceMethodref",
  "NameandType",
  "*Signature",
  0
};
#endif

// well-known attr names & misc. names:
#define EACH_WK_ATTR_NAME(F) \
	F(ClassFileVersion) \
IF_FULL(F(Deprecated)) \
IF_FULL(F(LineNumberTable)) \
IF_FULL(F(LocalVariableTable)) \
IF_FULL(F(Signature)) \
IF_FULL(F(SourceFile)) \
IF_FULL(F(Synthetic)) \
// End EACH_WK_ATTR_NAME.

#define EACH_WK_NAME(F) \
	F(Code) \
	F(ConstantValue) \
	F(Exceptions) \
	F(InnerClasses) \
	EACH_WK_ATTR_NAME(F)


struct cpool {
  int nentries;
  entry* entries;
  entry** entryRefs;  // copy of entries, as refs

  // Position and size of each homogeneous subrange:
  int tag_count[CONSTANT_Limit];
  int tag_base[CONSTANT_Limit];

  int getCount(byte tag) {
    assert((uint)tag < CONSTANT_Limit);
    return tag_count[tag];
  }
  entry** getIndex(byte tag) {
    assert((uint)tag < CONSTANT_Limit);
    return &entryRefs[tag_base[tag]];
  }

  entry*** field_indexes;   // indexed by CONSTANT_Class.ord
  entry*** method_indexes;  // indexed by CONSTANT_Class.ord
  entry** getFieldIndex(entry* classRef);
  entry** getMethodIndex(entry* classRef);

  // some handy maxima:
  int maxClass;
  int maxSignatureForm;
  int maxSignatureFormLen;

  int outputSize;  // adjusted estimate of maximum output size (after signature expansion)

  int outputOrderLimit;  // outord limit after computeOutputOrder
  int outputIndexLimit;  // index limit after renumbering
  ptrlist outputEntries; // list of entry* needing output idx assigned

  void postProcess() {
    expandSignatures();
    initWellKnownNames();
    initMemberIndexes();
  }
  void expandSignatures();
  void initWellKnownNames();
  void initMemberIndexes();

  void computeOutputOrder();
  void computeOutputIndexes();
  void resetOutputIndexes();
  void suppressOutputIndexes() { outputIndexLimit = -1; }

  int nindexes;
  entry** indexes;
  void makeIndexes();
  entry*& findIndex(byte tag, bytes& b);
  entry* ensureUtf8(bytes& b);
  entry* ensureClass(bytes& b);
  

  #define DECL_WKN(N) entry* N;
  EACH_WK_NAME(DECL_WKN)
  #undef DECL_WKN

  entry* initName;  // utf8 for "<init>"
};


enum { NOT_REQUESTED = 0, REQUESTED = -1, REQUESTED_LDC = -2 };

/* 
struct entry {
  byte tag;
  union {
    bytes b;
    int i;
    jlong l;
  } value;
*/


