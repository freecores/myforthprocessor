/*
 * @(#)unpack.cpp	1.18 03/01/23 
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

// -*- C++ -*-
// Program for unpacking specially compressed Java packages.
// John R. Rose

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

#include <limits.h>

#ifdef WIN32
#include <windows.h>
#include <winuser.h>
#endif
 

#include "defines.h"
#include "bytes.h"
#include "utils.h"
#include "coding.h"
#include "bands.h"

#include "constants.h"

#include "zip.h"

#include "unpack.h"



struct entry {
  byte tag;
  union {
    bytes b;
    int i;
    jlong l;
  }value;

  int outputIndex;
  int inord;   // cp.get(cp.tag_base+this.inord) == this
  int outord;  // order of appearance in classfile CPs
  int nrefs;
  entry* *refs;
  enum { INIT_REFS = 2 };
  entry* init_refs[INIT_REFS];

  void requestOutputIndex();  // return true if something changed
  int getOutputIndex() {
    assert(outputIndex > NOT_REQUESTED);
    return outputIndex;
  }

  entry* ref(int refnum) {
    assert((uint)refnum < nrefs);
    return refs[refnum];
  }

  const char* utf8String() {
    assert(tagMatches(CONSTANT_Utf8));
    assert(value.b.len == strlen((const char*)value.b.ptr));
    return (const char*)value.b.ptr;
  }

  entry* className() {
    assert(tagMatches(CONSTANT_Class));
    return ref(0);
  }

  entry* memberClass() {
    assert(tagMatches(CONSTANT_Member));
    return ref(0);
  }
  entry* memberDescr() {
    assert(tagMatches(CONSTANT_Member));
    return ref(1);
  }

  entry* descrName() {
    assert(tagMatches(CONSTANT_NameandType));
    return ref(0);
  }
  entry* descrType() {
    assert(tagMatches(CONSTANT_NameandType));
    return ref(1);
  }

  int typeSize();

  int compareTo(entry& e2);

  bytes& asUtf8();
  int    asInteger() { assert(tag == CONSTANT_Integer); return value.i; }

  bool isUtf8(bytes& b) { return tagMatches(CONSTANT_Utf8) && value.b.equals(b); }

  bool isDoubleWord() { return tag == CONSTANT_Double || tag == CONSTANT_Long; }

  bool tagMatches(byte tag2) {
    return (tag2 == tag)
      || (tag2 == CONSTANT_Utf8 && tag == CONSTANT_Signature)
      #ifndef PRODUCT
      || (tag2 == CONSTANT_Literal
	  && tag >= CONSTANT_Integer && tag <= CONSTANT_String && tag != CONSTANT_Class)
      || (tag2 == CONSTANT_Member
	  && tag >= CONSTANT_Fieldref && tag <= CONSTANT_InterfaceMethodref)
      #endif
      ;
  }

#ifdef PRODUCT
  char* string() { return 0; }
#else
  char* string();  // see far below
#endif
};

inline bytes& entry::asUtf8() {
  assert(tagMatches(CONSTANT_Utf8));
  return value.b;
}

int entry::typeSize() {
  assert(tagMatches(CONSTANT_Utf8));
  const char* sigp = (char*) value.b.ptr;
  switch (*sigp) {
  case '(': sigp++; break;  // skip opening '('
  case 'D':
  case 'J': return 2; // double field
  default:  return 1; // field
  }
  int siglen = 0;
  for (;;) {
    int ch = *sigp++;
    switch (ch) {
    case 'D': case 'J':
      siglen += 1;
      break;
    case '[':
      // Skip rest of array info.
      while (ch == '[') { ch = *sigp++; }
      if (ch != 'L')  break;
      // else fall through
    case 'L':
      sigp = strchr(sigp, ';')+1;
      break;
    case ')':  // closing ')'
      return siglen;
    }
    siglen += 1;
  }
}

inline entry** cpool::getFieldIndex(entry* classRef) {
  assert(classRef->tagMatches(CONSTANT_Class));
  assert((uint)classRef->inord < tag_count[CONSTANT_Class]);
  return field_indexes[classRef->inord];
}
inline entry** cpool::getMethodIndex(entry* classRef) {
  assert(classRef->tagMatches(CONSTANT_Class));
  assert((uint)classRef->inord < tag_count[CONSTANT_Class]);
  return method_indexes[classRef->inord];
}

struct attribute {
  entry* name;
  bytes b;
};

struct attrlist {
  int nattrs;
  attribute* attrs;

  attribute* alloc(int na) {
    return attrs = NEW(attribute, nattrs = na);
  }
};

struct handler {
  entry* klass;
  int    start;
  int    end;
  int    catsh;
};

struct fixup {
  int    off;
  entry* ref;
  void   init(int off_, entry* ref_) { off = off_; ref = ref_; }
  enum { u2_ref_marker = 0x02, u1_ref_marker = 0x01 };
  static void apply_list(int nfixups, fixup* fixups, byte* base);
};

struct code {
  member*  m;
  int      max_stack;
  int      max_locals;
  int      nhandlers;
  handler* handlers;
  bytes    b;
  attrlist a;
  int      nfixups;
  fixup*   fixups;
  byte*    wp;  // location of bytecodes in output buffer
};

struct member {
  klass* k;
  int flags;
  entry* descr;  // a CONSTANT_NameandType ref
  attrlist a;

  // hardwired attributes:
  entry* constantValue;
  int nexceptions;
  entry** exceptions;
  code* methodCode;

  entry* name() { return descr->descrName(); }
  entry* type() { return descr->descrType(); }

#ifndef PRODUCT
  bool isMethod();
#endif
};

struct klass {
  int flags;
  entry* name;
  entry* sup;

  int nintfs;
  entry** intfs;

  int nmems[2];
  member* mems[2];

  attrlist a;

  int ncodes;
  code* codes;

  int minver, majver;

  int est_size;  // estimated classfile size, less CP and ICs

  char* classFileName();
};

struct inner_klass {
  entry* inner;
  entry* outer;
  entry* name;
  int    flags;
};

// parsing state
struct {
  cpool cp;

  int default_class_minor_version;
  int default_class_major_version;

  int nklasses;
  klass* klasses;

  attrlist a;

  int nics;  // InnerClasses
  inner_klass* ics;

  int ncodes;
  code* codes;

  bytes   fixupbuf;
  fixup*  fixupptr;   // next fixup allocatable
  bytes   fixuparea;  // region where fixups can occur

  int max_klass_size;

  // input state
  char* pkgfilename;
  bytes pkgfile;

  // output state
  char* outputdir;
  bytes classfile_head;
  bytes classfile_tail;
  bool  jar_output;
} pkg;

struct { const char* name; int offset; } wkn_tab[] = {
  #define INIT_WKN(N) \
    { #N, (int) &((cpool*)null)->N },
  EACH_WK_NAME(INIT_WKN)
  #undef INIT_WKN
  { "<init>", (int) &((cpool*)null)->initName },
  { null, 0 }
};

#define CP_LOCAL \
  cpool& cp = pkg.cp

entry** getCPIndex(byte tag) {
  return pkg.cp.getIndex(tag);
}

static void init_pkg_fixups(int maxfixups, bytes& fixuparea) {
  pkg.fixuparea = fixuparea;
  fixup* fixups = (fixup*) pkg.fixupbuf.ptr;
  bytes& fixupbuf = pkg.fixupbuf;
  if (!fixupbuf.inBounds((byte*)(fixups + maxfixups)-1)) {
    fixupbuf.realloc(maxfixups * sizeof(fixup));
    fixups = (fixup*) fixupbuf.ptr;
  }
  pkg.fixupptr = fixups;
}

int total_cp_size[] = {0, 0};
int largest_cp_ref[] = {0, 0};
int hash_probes[] = {0, 0};

char* klass::classFileName() {
  bytes clsName = name->className()->asUtf8();
  const char* suffix = ".class";
  bytes str;
  str.malloc(clsName.len+strlen(suffix));
  return str.strcat(clsName).strcat(suffix).strval();
}

// the main routine

int nowrite = 0;
int strip_compile = 0;
int strip_debug = 0;
int files_written = 0;
int bytes_written = 0;
#ifndef PRODUCT
int skipfiles = 0;
#endif

extern byte* rp;
extern int deState;
extern byte* wp;


#if defined(WIN32) && defined(_MSC_VER) 
// The functions which follow is to satifsy InstallShield waiting
// for the process. Lifted from kgh's implementation in uncrunch.
#define OUR_CLASS_NAME "kgh-uncrunch"

HINSTANCE ourInstance;


// This function is called to process window messages
LRESULT CALLBACK window_proc(
	HWND window,
	UINT msg,
	WPARAM wParam,
	LPARAM lParam)
{
    // fprintf(errstrm, "window_proc: %d\n", msg);
     return 0;
}


// This function runs as a separate thread to manage our
// window state.
// We crate an invisible window simply so that InstallShield
// can detect when we exit.  Sigh.
// Lifted from uncrunch code.
DWORD WINAPI
window_thread(LPVOID arg) 
{
    WNDCLASSEX wex;
    ATOM our_class;
    HWND window;
    MSG msg;
    int rc;

    // fprintf(errstrm, "Window thread started\n");

    // Initialize our window state.
    wex.cbSize = sizeof(wex);
    wex.style = CS_NOCLOSE;
    wex.lpfnWndProc = window_proc;
    wex.cbClsExtra = 0;
    wex.cbWndExtra = 0;
    wex.hInstance = ourInstance;
    wex.hIcon = NULL;
    wex.hCursor = NULL;
    wex.hbrBackground = NULL;
    wex.lpszMenuName = NULL;
    wex.lpszClassName = OUR_CLASS_NAME;
    wex.hIconSm = NULL;

    our_class = RegisterClassEx(&wex);
    
    if (our_class == 0) {
	fprintf(errstrm, "RegisterClassEx error %d\n", GetLastError());
    }

    window = CreateWindow(OUR_CLASS_NAME,
		"Unpack.exe",
		WS_DLGFRAME,
		400, 300,	// Position
		300, 200,	//Sixe
		NULL,
		NULL,
		ourInstance,
		0);

    if (window == 0) {
	fprintf(errstrm, "CreateWindow error %d\n", GetLastError());
    }

    // Run the window message loop.
    while (GetMessage(&msg, NULL, 0, 0)) { 
        DispatchMessage(&msg);
    } 
    return 0;
}

struct strargs {
  int   argc   ;
  char  *argv[1024] ;
} st ;


void getArgs(char *s) {
  char *delim = {"\n\t "};
  char *p, *end;
  char *str = strdup(s);

  st.argc = 0 ;

  // Parameters could be quoted for FileNames with spaces.
  for (p = strtok(str, delim) ; p != NULL ;) {
    if (*p == '"') { 
      char *q = new char[PATH_MAX];
      strcpy(q, ++p) ; 
      if (q[strlen(p) - 1] == '"') { //Get rid of the last quote
        q[strlen(p) -1 ] = '\0';
      } else {  // If we dont have a quote look for more
        while ( (p = strtok(NULL, delim)) != NULL) {
          strcat(q, " ");
	  strcat(q, p);
	  char *lastq = strrchr(q,'"');
	  if (lastq != NULL) {
	    *lastq = '\0';
	    break;
	  }
        }
      }
      p = q ;
    }

    // WinMain will not set argv[0] to the program name
    // MSVC debugger will set it.
    if (st.argc == 0 ) {
      char *last_dot = strrchr(p,'.');
      if ( (*p == '-') || ((last_dot != NULL) && (strcasecmp(last_dot, ".exe") != 0)) ) {
       st.argv[st.argc] = strdup("unpack.exe");
       st.argc++;
      }
    }

    st.argv[st.argc] = strdup(p) ;
    p = strtok(NULL, delim);
    st.argc++;
  }
}


// Main entry point.
int APIENTRY
WinMain(HINSTANCE hInstance, 
	HINSTANCE hPrevInstance,
	LPSTR lpCmdLine,
	int nCmdShow)
{
    DWORD wthid;
    HANDLE wthd;
    ourInstance = hInstance;
    getArgs(lpCmdLine); 
    return run(st.argc, st.argv);
}
#else // For Sane OS, non Windows

int main(int argc, char **argv) {
    return run(argc, argv);
}

#endif  // End of WIN32 and _MSC_VER

int run(int argc, char **argv) {
  int ac = argc ;
  char **av = argv;
  char *logfilename = NULL;
  while (ac > 1 && *av[1] == '-') {
    if (false) { }
#ifndef PRODUCT
    else if (!strcmp("-v", av[1])) { incVerboseLevel(1); av++; ac--; }
    else if (!strcmp("-q", av[1])) { incVerboseLevel(-1); av++; ac--; }
    else if (!strcmp("-n", av[1])) { nowrite = 1; av++; ac--; }
    else if (!strcmp("-s", av[1])) { av++; av--; skipfiles = atol(av[1]); av++; ac--; }
#endif
    // PRODUCT flags:
    else if (!strcmp("-q", av[1])) { av++; ac--; }  /* make passes -q, does nothing for now till we fix make */
    else if (!strcmp("-c", av[1])) { strip_compile = 1; av++; ac--; }
    else if (!strcmp("-d", av[1])) { strip_debug = 1; av++; ac--; }
    else if (!strcmp("-l", av[1])) { logfilename = av[2] ; av +=2 ; ac -= 2; }
    else break;
  }  

  redirect_stdio(logfilename);  

  int i = 0;
  for (i = 0 ; i < argc ; i++) {
     fprintf(errstrm,"[%d] <%s>\n",i, argv[i]);
  }

  if (ac != 3) {
     syntax(argv[0]);
  }

  fprintf(errstrm,"Unpacking from %s to %s\n",av[1], av[2]);

  return unpack(av[1], av[2]);
}

void syntax(char *progname) {
  static char message[1024];
  sprintf(message, "Usage: %s [-v -c -d -l logfile] pkgfile.package outputdir",progname);
  exit(-1);
}

int unpack(char* src, char *dst) {
  // Usage:  unpackage pkgfile outputdir
  // Output:  Writes all classfiles from the given pkgfile under outputdir.

  //Open the output file and set flag
  char* outputdir = pkg.outputdir = dst;
  if (outputdir == null || strlen(outputdir) == 0)  outputdir = (char*)".";
  if (strstr(outputdir,".jar") != null) {
    openJarFile(outputdir);
    pkg.jar_output = true;
  }
  
  // Open the input file.
  char* pkgfilename = pkg.pkgfilename = src;
  bytes& pkgfile = pkg.pkgfile;
  pkgfile.malloc(60 * 1024);

  FILE* fp = !strcmp(pkgfilename,"-") ? stdin : fopen(pkgfilename, "rb");
  if (!fp) {
     char *message = (char *) must_malloc(PATH_MAX);
     sprintf(message,"Error: Could not open input file: %s\n",pkgfilename);
     unpack_abort(message);
  }

  // Fully read the input file.
  int readptr = 0;
  for (;;) {
    if (readptr == pkgfile.len)  pkgfile.grow(2.0);
    int nr = fread(pkgfile.ptr + readptr, 1, pkgfile.len - readptr, fp);
    if (nr <= 0)  break;
    readptr += nr;
  }
  pkgfile.realloc(readptr+100);  // pad end with zeroes, for luck
  pkgfile.len = readptr;

  fclose(fp);

  printcr(1,"read %d bytes from %s", pkgfile.len, pkgfilename);


  if (pkgfile.ptr[0] == 'P' && pkgfile.ptr[1] == 'K') {
    //We simply simulate a copy command
    fprintf(errstrm,"Copy-mode\n");
    write2JarFile(pkgfile);
    closeJarFile(false);
  } else {
    band::initStatics();

    //Set up read pointer.
    rp = pkgfile.ptr;
    read_pkgfile();

    CP_LOCAL;
    int nklasses       = pkg.nklasses;
    klass* klasses     = pkg.klasses;
    int max_klass_size = pkg.max_klass_size;

    write_sidefiles();
    write_classfiles();

#ifndef PRODUCT
    printcr(0,"wrote grand total of %d CP entries, of total size %d",
	total_cp_size[0], total_cp_size[1]);
    printcr(0,"largest CP reference in input was %d, in output is %d",
	largest_cp_ref[0], largest_cp_ref[1]);
    printcr(1,"utf8 hash request count %d, probes needed were %d",
	hash_probes[0], hash_probes[1]);
#endif
    printcr(0,"wrote %d files into %s", files_written, outputdir);
    printcr(0,"unpacked %d bytes to %d bytes (%d%% expansion)",
	pkgfile.len, bytes_written, int(((bytes_written-pkgfile.len)*100.0+.5)/pkgfile.len));
    closeJarFile(true);
  }
  fprintf(errstrm,"Unpack completed OK\n");
  return 0;
}

void write_file(char* filename, int nsegs, bytes* segs) {
  int i;
  int len = 0;
  for (i = 0; i < nsegs; i++)
    len += segs[i].len;
  if (nowrite  NOT_PRODUCT(|| skipfiles-- > 0)) {
    printcr(2,"would write %d bytes to %s", len, filename);
    return;
  } else if (pkg.jar_output) {
    addJarEntry(filename, nsegs, segs);
    printcr(2,"wrote %d bytes to zip entry %s", len, filename);
  } else {
    bytes fullpath;
    char* outputdir = pkg.outputdir;
    fullpath.malloc(strlen(outputdir)+1+strlen(filename));
    fullpath.strcat(outputdir).strcat("/").strcat(filename);
    mkdirs(strlen(outputdir), fullpath.strval());
    FILE* cfp = fopen(fullpath.strval(), "wb");
    if (!cfp) {
	char *message = (char *) must_malloc(PATH_MAX);
	sprintf(message,"Error: Could not open output file/dir: %s\n",fullpath.strval());
	unpack_abort(message);
    }  
    for (i = 0; i < nsegs; i++)
      fwrite(segs[i].ptr, 1, segs[i].len, cfp);
    fclose(cfp);
    printcr(2,"wrote %d bytes to file %s", len, filename);
    fullpath.free();
  }
  bytes_written += len;
  files_written += 1;
}



static void read_magic_numbers() {
  BAND_LOCAL;

  package_magic.expectLength(4+2+2);  // magic, minor, major
  package_magic.readFrom(rp);
  rp = package_magic.rp;
  int magic = getu4(rp);  rp += 4;
  int minver = getu2(rp); rp += 2;
  int majver = getu2(rp); rp += 2;
  NOT_PRODUCT(package_magic.rp = rp);  // make asserts happy
  if (magic != JAVA_PACKAGE_MAGIC ||
      majver != JAVA_PACKAGE_MAJOR_VERSION ||
      minver != JAVA_PACKAGE_MINOR_VERSION) {
    fprintf(errstrm, "Bad file format, magic/ver = "
	    "%08X/%d.%d should be %08X/%d.%d\n",
	    magic, majver, minver, JAVA_PACKAGE_MAGIC,
	    JAVA_PACKAGE_MAJOR_VERSION, JAVA_PACKAGE_MINOR_VERSION);
    unpack_abort("Error: Bad file format");
  }
  package_magic.done();
}

static void read_header() {
  BAND_LOCAL;
  CP_LOCAL;

  int phlen = N_TAGS_IN_ORDER;
  // #minver, #majver, #classes, #innerClasses, #package_attrs
  phlen += 5;
  package_header.expectLength(phlen);
  package_header.readFrom(rp);

  // Fill-pointer for CP.
  int next_entry = 1;  // skip initial one because zero is special

  // Size the constant pool:
  for (int k = 0; k < N_TAGS_IN_ORDER; k++) {
    byte tag = TAGS_IN_ORDER[k];
    int  len = package_header.getUnsigned();
    cp.tag_count[tag] = len;
    cp.tag_base[tag] = next_entry;
    next_entry += len;
    switch (tag) {
    case CONSTANT_Utf8:
      cp_Utf8_prefix.expectLength(len);
      cp_Utf8_suffix.expectLength(len);
      break;
    case CONSTANT_Integer:
      cp_Int.expectLength(len);
      break;
    case CONSTANT_Float:
      cp_Float.expectLength(len);
      break;
    case CONSTANT_Long:
      cp_Long_hi.expectLength(len);
      cp_Long_lo.expectLength(len);
      break;
    case CONSTANT_Double:
      cp_Double_hi.expectLength(len);
      cp_Double_lo.expectLength(len);
      break;
    case CONSTANT_Class:
      cp_Class.expectLength(len);
      break;
    case CONSTANT_String:
      cp_String.expectLength(len);
      break;
    case CONSTANT_Fieldref:
      cp_Field_class.expectLength(len);
      cp_Field_desc.expectLength(len);
      break;
    case CONSTANT_Methodref:
      cp_Method_class.expectLength(len);
      cp_Method_desc.expectLength(len);
      break;
    case CONSTANT_InterfaceMethodref:
      cp_Imethod_class.expectLength(len);
      cp_Imethod_desc.expectLength(len);
      break;
    case CONSTANT_NameandType:
      cp_Descr_name.expectLength(len);
      cp_Descr_type.expectLength(len);
      break;
    case CONSTANT_Signature:
      cp_Signature_form.expectLength(len);
      cp_Signature_classes.expectLength(0);  // more later
      break;
    default:
      assert(false);
    }
  }

  // Close off the end of the CP:
  cp.nentries = next_entry;

  pkg.default_class_minor_version = package_header.getUnsigned();
  pkg.default_class_major_version = package_header.getUnsigned();
  int nklasses = package_header.getUnsigned();
  pkg.klasses = NEW(klass, pkg.nklasses = nklasses);
  int nics = package_header.getUnsigned();
  pkg.ics = NEW(inner_klass, pkg.nics = nics);
  //pkg.a.alloc(package_header.getUnsigned());
  pkg.a.nattrs = package_header.getUnsigned();
  class_flags.expectLength(nklasses);
  class_this.expectLength(nklasses);
  class_super.expectLength(nklasses);
  class_interface_count.expectLength(nklasses);
  class_field_count.expectLength(nklasses);
  class_method_count.expectLength(nklasses);
  ic_this_class.expectLength(nics);
  ic_flags.expectLength(nics);
  package_header.done();
}

static byte* store_Utf8_char(byte* cp, int ch) {
  if (ch >= 0x001 && ch <= 0x007F) {
    *cp++ = (byte) ch;
  } else if (ch <= 0x07FF) {
    *cp++ = (byte) (0xC0 | ((ch >>  6) & 0x1F));
    *cp++ = (byte) (0x80 | ((ch >>  0) & 0x3F));
  } else {
    *cp++ = (byte) (0xE0 | ((ch >> 12) & 0x0F));
    *cp++ = (byte) (0x80 | ((ch >>  6) & 0x3F));
    *cp++ = (byte) (0x80 | ((ch >>  0) & 0x3F));
  }
  return cp;
}

static byte* skip_Utf8_chars(byte* cp, int len) {
  for (;; cp++) {
    int ch = *cp & 0xFF;
    if ((ch & 0xC0) != 0x80) {
      if (len-- == 0)
	return cp;
      if (ch < 0x80 && len == 0)
	return cp+1;
    }
  }
}

static int compare_Utf8_chars(bytes& b1, bytes& b2) {
  int l1 = b1.len;
  int l2 = b2.len;
  int l0 = (l1 < l2) ? l1 : l2;
  byte* p1 = b1.ptr;
  byte* p2 = b2.ptr;
  int c0 = 0;
  for (int i = 0; i < l0; i++) {
    int c1 = p1[i] & 0xFF;
    int c2 = p2[i] & 0xFF;
    if (c1 != c2) {
      // Before returning the obvious answer,
      // check to see if c1 or c2 is part of a 0x0000,
      // which encodes as {0xC0,0x80}.  The 0x0000 is the
      // lowest-sorting Java char value, and yet it encodes
      // as if it were the first char after 0x7F, which causes
      // strings containing nulls to sort too high.  All other
      // comparisons are consistent between Utf8 and Java chars.
      if (c1 == 0xC0 && (p1[i+1] & 0xFF) == 0x80)  c1 = 0;
      if (c2 == 0xC0 && (p2[i+1] & 0xFF) == 0x80)  c2 = 0;
      if (c0 == 0xC0) {
	assert(((c1|c2) & 0xC0) == 0x80);  // c1 & c2 are extension chars
	if (c1 == 0x80)  c1 = 0;  // will sort below c2
	if (c2 == 0x80)  c2 = 0;  // will sort below c1
      }
      return c1 - c2;
    }
    c0 = c1;  // save away previous char
  }
  // common prefix is identical; return length difference if any
  return l1 - l2;
}

static void read_Utf8_values(entry* cpMap, int len) {
  BAND_LOCAL;

  int i;

  cp_Utf8_prefix.readFrom(rp);
  int* prefixes = cp_Utf8_prefix.getValueArray();
  cp_Utf8_prefix.done();

  cp_Utf8_suffix.readFrom(rp);
  int* suffixes = cp_Utf8_suffix.getValueArray();
  cp_Utf8_suffix.done();

  for (i = 0; i < len; i++) {
    cp_Utf8_chars.expectMoreLength(suffixes[i]);
  }

  bytes* suffixChars = NEW(bytes, len);
  cp_Utf8_chars.readFrom(rp);
  for (i = 0; i < len; i++) {
    int suffix = suffixes[i];
    if (suffix == 0) {
      // chars are packed in cp_Utf8_big_chars
      cp_Utf8_big_length.expectMoreLength(1);
      continue;
    }
    bytes& chars = suffixChars[i];
    chars.malloc(suffix * 3);  // max Utf8 length
    byte* cp = chars.ptr;
    for (int j = 0; j < suffix; j++) {
      cp = store_Utf8_char(cp, cp_Utf8_chars.getUnsigned());
    }
    // shrink to fit:
    chars.realloc(cp - chars.ptr);
  }
  cp_Utf8_chars.done();

  int maxChars = 0;

  // Handle specially-packed strings.
  cp_Utf8_big_length.readFrom(rp);
  for (i = 0; i < len; i++) {
    int suffix = suffixes[i];
    if (suffix == 0) {
      suffixes[i] = suffix = cp_Utf8_big_length.getUnsigned();
    }
    if (maxChars < prefixes[i] + suffixes[i])
      maxChars = prefixes[i] + suffixes[i];
  }
  cp_Utf8_big_length.done();

  for (i = 0; i < len; i++) {
    bytes& chars = suffixChars[i];
    if (chars.ptr != null)  continue;  // already input
    int suffix = suffixes[i];
    chars.malloc(suffix * 3);  // max Utf8 length
    byte* cp = chars.ptr;
    band saved_band = cp_Utf8_big_chars;
    cp_Utf8_big_chars.expectLength(suffix);
    cp_Utf8_big_chars.readFrom(rp);
    for (int j = 0; j < suffix; j++) {
      cp = store_Utf8_char(cp, cp_Utf8_big_chars.getUnsigned());
    }
    chars.realloc(cp - chars.ptr);
    cp_Utf8_big_chars.done();
    cp_Utf8_big_chars = saved_band;  // reset the band for the next string
  }
  cp_Utf8_big_chars.readFrom(rp);  // zero chars
  cp_Utf8_big_chars.done();

  // Finally, sew together all the prefixes and suffixes.
  bytes bigbuf;
  bigbuf.malloc(maxChars * 3 + 1);  // max Utf8 length, plus slop for null
  for (i = 0; i < len; i++) {
    int prefix = prefixes[i];
    int suffix = suffixes[i];
    bytes& chars = suffixChars[i];

    byte* startp = bigbuf.ptr;
    byte* fillp = startp;
    // by induction, the buffer is already filled with the prefix
    fillp = skip_Utf8_chars(startp, prefix);
    // copy the suffix into the same buffer:
    fillp = chars.writeTo(fillp);
    assert(bigbuf.inBounds(fillp));
    *fillp = 0;  // bigbuf must contain a well-formed Utf8 string
    chars.free();  // tidy
    cpMap[i].value.b.saveFrom(startp, fillp - startp);
  }
}

static void read_single_words(band& cp_band, entry* cpMap, int len) {
  cp_band.readFrom(rp);
  for (int i = 0; i < len; i++) {
    cpMap[i].value.i = cp_band.getValue();  // coding handles signs OK
  }
  cp_band.done();
}

static void read_double_words(band& cp_bands, entry* cpMap, int len) {
  band& cp_band_hi = cp_bands;
  band& cp_band_lo = *cp_bands.nextBand();
  cp_band_hi.readFrom(rp);
  cp_band_lo.readFrom(rp);
  for (int i = 0; i < len; i++) {
    int hi = cp_band_hi.getValue();
    int lo = cp_band_lo.getValue();
    cpMap[i].value.l = ((julong)hi << 32) + (uint)lo;
  }
  cp_band_hi.done();
  cp_band_lo.done();
}

static void read_single_refs(band& cp_band, byte refTag, entry* cpMap, int len) {
  cp_band.setIndexByTag(refTag);
  cp_band.readFrom(rp);
  for (int i = 0; i < len; i++) {
    entry& e = cpMap[i];
    e.nrefs = 1;
    e.refs = e.init_refs;
    e.refs[0] = cp_band.getRef();
  }
  cp_band.done();
}

static void read_double_refs(band& cp_band, byte ref1Tag, byte ref2Tag,
			     entry* cpMap, int len) {
  band& cp_band1 = cp_band;
  band& cp_band2 = *cp_band.nextBand();
  cp_band1.setIndexByTag(ref1Tag);
  cp_band2.setIndexByTag(ref2Tag);
  cp_band1.readFrom(rp);
  cp_band2.readFrom(rp);
  for (int i = 0; i < len; i++) {
    entry& e = cpMap[i];
    e.nrefs = 2;
    e.refs = e.init_refs;
    e.refs[0] = cp_band1.getRef();
    e.refs[1] = cp_band2.getRef();
  }
  cp_band1.done();
  cp_band2.done();
}

static void read_cp() {
  BAND_LOCAL;
  CP_LOCAL;

  byte* rp0 = rp;
  int k, i;

  // place a limit on future CP growth:
  int cp_size_limit = 0;
  for (k = 0; k < N_TAGS_IN_ORDER; k++) {
    byte tag = TAGS_IN_ORDER[k];
    int  len = cp.tag_count[tag];
    cp_size_limit += len;
  }
  cp_size_limit += pkg.nics * 3; // name, outer, outer.utf8
  cp_size_limit += 20;  // WKUs, misc

  // Note that this CP does not include "empty" entries
  // for longs and doubles.  Those are introduced when
  // the entries are renumbered for classfile output.

  cp.entries = NEW(entry, cp_size_limit);
  cp.entryRefs = NEW(entry*, cp_size_limit);
  for (i = 0; i < cp_size_limit; i++)
    cp.entryRefs[i] = &cp.entries[i];
  {
    int pow2 = 1;
    while (pow2 < cp_size_limit)  pow2 <<= 1;
    cp.indexes = NEW(entry*, cp.nindexes = pow2);
  }

  // fixed overhead is magic, minvar, majver, ncp
  int outputSize = 0;
  outputSize = outputSize += 4 + 3*2;

  for (k = 0; k < N_TAGS_IN_ORDER; k++) {
    byte tag = TAGS_IN_ORDER[k];
    int  len = cp.tag_count[tag];
    int base = cp.tag_base[tag];

    outputSize += len;

    printcr(1,"Reading %d %s entries...", len, tagName(tag));
    entry* cpMap = &cp.entries[base];
    for (i = 0; i < len; i++) {
      cpMap[i].tag = tag;
      cpMap[i].inord = i;
    }

    switch (tag) {
    case CONSTANT_Utf8:
      read_Utf8_values(cpMap, len);
      // Maintain cross-reference:
      for (i = 0; i < len; i++) {
	entry& e = cpMap[i];
	cp.findIndex(CONSTANT_Utf8, e.value.b) = &e;
	outputSize += 2 + e.value.b.len;
      }
      break;
    case CONSTANT_Integer:
      read_single_words(cp_Int, cpMap, len);
      outputSize += 4*len;
      break;
    case CONSTANT_Float:
      read_single_words(cp_Float, cpMap, len);
      outputSize += 4*len;
      break;
    case CONSTANT_Long:
      read_double_words(cp_Long_hi /*& cp_Long_lo*/, cpMap, len);
      outputSize += 8*len;
      break;
    case CONSTANT_Double:
      read_double_words(cp_Double_hi /*& cp_Double_lo*/, cpMap, len);
      outputSize += 8*len;
      break;
    case CONSTANT_String:
      read_single_refs(cp_String, CONSTANT_Utf8, cpMap, len);
      outputSize += 2*len;
      break;
    case CONSTANT_Class:
      read_single_refs(cp_Class, CONSTANT_Utf8, cpMap, len);
      // Maintain cross-reference:
      for (i = 0; i < len; i++) {
	entry& e = cpMap[i];
	// Copy the print-name of the class into the entry.
	// This makes entry::compareTo simpler.
	e.value.b = e.refs[0]->value.b;
	if (cp.maxClass < e.value.b.len)
	  cp.maxClass = e.value.b.len;
	cp.findIndex(CONSTANT_Class, e.value.b) = &e;
      }
      outputSize += 2*len;
      break;
    case CONSTANT_Signature:
      {
	cp_Signature_form.setIndexByTag(CONSTANT_Utf8);
	cp_Signature_form.readFrom(rp);
	for (i = 0; i < len; i++) {
	  entry& form = *cp_Signature_form.getRef();
	  int nc = 0;
	  for (const char* ncp = form.utf8String(); *ncp; ncp++) {
	    if (*ncp == 'L')  nc++;
	  }
	  if (cp.maxSignatureForm < form.value.b.len)
	    cp.maxSignatureForm = form.value.b.len;
	  if (cp.maxSignatureFormLen < nc)
	    cp.maxSignatureFormLen = nc;
	  cpMap[i].nrefs = 1 + nc;
	  // Reallocate cpMap[i].refs if needed.
	  if (cpMap[i].nrefs > cpMap[i].INIT_REFS) {
	    cpMap[i].refs = NEW(entry*, cpMap[i].nrefs);
	  } else {
	    cpMap[i].refs = cpMap[i].init_refs;
	  }
	  cp_Signature_classes.expectMoreLength(nc);
	  cpMap[i].refs[0] = &form;
	}
	cp_Signature_form.done();
	cp_Signature_classes.setIndexByTag(CONSTANT_Class);
	cp_Signature_classes.readFrom(rp);
	for (i = 0; i < len; i++) {
	  for (int j = 1; j < cpMap[i].nrefs; j++) {
	    cpMap[i].refs[j] = cp_Signature_classes.getRef();
	  }
	}
	cp_Signature_classes.done();
	break;
      }
    case CONSTANT_NameandType:
      read_double_refs(cp_Descr_name /*& cp_Descr_type*/,
		       CONSTANT_Utf8, CONSTANT_Signature,
		       cpMap, len);
      outputSize += 4*len;
      break;
    case CONSTANT_Fieldref:
      read_double_refs(cp_Field_class /*& cp_Field_desc*/,
		       CONSTANT_Class, CONSTANT_NameandType,
		       cpMap, len);
      outputSize += 4*len;
      break;
    case CONSTANT_Methodref:
      read_double_refs(cp_Method_class /*& cp_Method_desc*/,
		       CONSTANT_Class, CONSTANT_NameandType,
		       cpMap, len);
      outputSize += 4*len;
      break;
    case CONSTANT_InterfaceMethodref:
      read_double_refs(cp_Imethod_class /*& cp_Imethod_desc*/,
		       CONSTANT_Class, CONSTANT_NameandType,
		       cpMap, len);
      outputSize += 4*len;
      break;
    default:
      assert(false);
      break;
    }
  }

  cp.outputSize = outputSize;

  cp.postProcess();
  printcr(1,"parsed %d constant pool entries in %d bytes", cp.nentries, (rp - rp0));

  band::initIndexes();
#ifndef PRODUCT
  // check hand-initialization of TAG_ORDER
  for (i = 0; i < N_TAGS_IN_ORDER; i++) {
    byte tag = TAGS_IN_ORDER[i];
    assert(TAG_ORDER[tag] == i+1);
  }
#endif
}

#define NO_ENTRY_YET ((entry*)-1)

static bool isDigitString(bytes& x, int beg, int end) {
  if (beg == end)  return false;  // null string
  byte* xptr = x.ptr;
  for (int i = beg; i < end; i++) {
    char ch = xptr[i];
    if (!(ch >= '0' && ch <= '9'))  return false;
  }
  return true;
}

static void read_ics() {
  BAND_LOCAL;
  int i;
  int nics = pkg.nics;
  inner_klass* ics = pkg.ics;
  ic_this_class.readFrom(rp);
  ic_flags.readFrom(rp);
  for (i = 0; i < nics; i++) {
    int flags = ic_flags.getValue();  // may be long form!
    bool longForm = (flags & ACC_IC_LONG_FORM) != 0;
    flags &= ~ACC_IC_LONG_FORM;
    if (longForm) {
      ic_outer_class.expectMoreLength(1);
      ic_name.expectMoreLength(1);
      ics[i].name = NO_ENTRY_YET;
    }
    ics[i].inner = ic_this_class.getRef();
    ics[i].flags = flags;
  }
  ic_this_class.done();
  ic_flags.done();

  ic_outer_class.readFrom(rp);
  ic_name.readFrom(rp);
  for (i = 0; i < nics; i++) {
    if (ics[i].name == NO_ENTRY_YET) {
      // Long form.
      ics[i].outer = ic_outer_class.getRef();
      ics[i].name  = ic_name.getRef();
    } else {
      // Fill in outer and name based on inner.
      bytes& n = ics[i].inner->value.b;
      bytes pkgOuter;
      bytes number;
      bytes name;
      // Parse n into pkgOuter and name (and number).
      printcr(5, "parse short IC name %s", n.ptr);
      int dollar1, dollar2;  // pointers to $ in the pattern
      // parse n = (<pkg>/)*<outer>($<number>)?($<name>)?
      int nlen = n.len;
      int pkglen = n.lastIndexOf('/')+1;
      dollar2 = n.lastIndexOf('$');
      assert(dollar2 >= pkglen);
      if (isDigitString(n, dollar2+1, nlen)) {
	// n = (<pkg>/)*<outer>$<number>
	number = n.slice(dollar2+1, nlen);
	name.set(null,0);
	dollar1 = dollar2;
      } else if ((dollar1 = n.lastIndexOf('$', dollar2-1)) > pkglen
		   && isDigitString(n, dollar1+1, dollar2)) {
	// n = (<pkg>/)*<outer>$<number>$<name>
	number = n.slice(dollar1+1, dollar2);
	name = n.slice(dollar2+1, nlen);
      } else {
	// n = (<pkg>/)*<outer>$<name>
	dollar1 = dollar2;
	number.set(null,0);
	name = n.slice(dollar2+1, nlen);
      }
      if (number.ptr == null)
	pkgOuter = n.slice(0, dollar1);
      else
	pkgOuter.set(null,0);
      printcr(5,"=> %s$ 0%s $%s",
	      pkgOuter.string(), number.string(), name.string());

      if (pkgOuter.ptr != null)
	ics[i].outer = pkg.cp.ensureClass(pkgOuter);

      if (name.ptr != null)
	ics[i].name = pkg.cp.ensureUtf8(name);
    }
  }
  ic_outer_class.done();
  ic_name.done();
}

static void read_members() {
  BAND_LOCAL;
  int i;

  int nklasses = pkg.nklasses;
  class_field_count.readFrom(rp);
  class_method_count.readFrom(rp);
  for (i = 0; i < nklasses; i++) {
    klass& k = pkg.klasses[i];
    int nf = class_field_count.getUnsigned();
    int nm = class_method_count.getUnsigned();
    k.mems[0] = NEW(member, k.nmems[0] = nf);
    k.mems[1] = NEW(member, k.nmems[1] = nm);
    field_flags.expectMoreLength(nf);
    field_descr.expectMoreLength(nf);
    method_flags.expectMoreLength(nm);
    method_descr.expectMoreLength(nm);
    // per-member fixed overhead is flags, name, type, na
    k.est_size += 4*2 * (nf + nm);
  }
  class_field_count.done();
  class_method_count.done();

  field_flags.readFrom(rp);
  method_flags.readFrom(rp);
  field_descr.readFrom(rp);
  method_descr.readFrom(rp);

  // list of clients for field_constant_value:
  ptrlist conFields;
  conFields.init();

  // list of clients for method_exceptions:
  ptrlist excMethods;
  excMethods.init();

  // list of clients for the code attribute:
  ptrlist codeMethods;
  codeMethods.init();

  for (i = 0; i < nklasses; i++) {
    klass& k = pkg.klasses[i];
    int     nf   = k.nmems[0];
    int     nm   = k.nmems[1];
    member* fptr = k.mems[0];
    member* mptr = k.mems[1];
    int codes_so_far = codeMethods.len;
    int j;
    for (j = 0; j < nf; j++) {
      int     flags    = field_flags.getFlags();
      bool    hasAttrs = (flags & ACC_HAS_ATTRIBUTES) != 0;
      flags &= ~ACC_HAS_ATTRIBUTES;
      bool    hasCon   = (flags & ACC_CONSTANT_VALUE) != 0;
      flags &= ~ACC_CONSTANT_VALUE;
      member& f = fptr[j];
      f.k     = &k;
      f.flags = flags;
      f.descr = field_descr.getRef();
      if (hasAttrs)  field_attrs.expectClient(&f);
      if (hasCon) {
	field_constant_value.expectMoreLength(1);
	conFields.add(&f);
      }
    }
    for (j = 0; j < nm; j++) {
      int     flags    = method_flags.getFlags();
      bool    hasAttrs = (flags & ACC_HAS_ATTRIBUTES) != 0;
      flags &= ~ACC_HAS_ATTRIBUTES;
      bool    hasExc   = (flags & ACC_EXCEPTIONS) != 0;
      flags &= ~ACC_EXCEPTIONS;
      bool    hasCode  = (flags & (ACC_NATIVE|ACC_ABSTRACT)) == 0;
      member& m = mptr[j];
      m.k     = &k;
      m.flags = flags;
      m.descr = method_descr.getRef();
      if (hasAttrs)  method_attrs.expectClient(&m);
      if (hasCode)   codeMethods.add(&m);
      if (hasExc) {
	method_exception_count.expectMoreLength(1);
	excMethods.add(&m);
      }
    }
    k.ncodes = codeMethods.len - codes_so_far;
  }
  field_flags.done();
  field_descr.done();
  method_flags.done();
  method_descr.done();

  // Read field ConstantValue attributes:
  field_constant_value.readFrom(rp);
  for (i = 0; i < conFields.len; i++) {
    member& f = *(member*) conFields.get(i);
    entry* ftype = f.type();
    byte tag;
    switch (ftype->value.b.ptr[0]) {
    case 'L': tag = CONSTANT_String;   break;
    case 'I': tag = CONSTANT_Integer;  break;
    case 'J': tag = CONSTANT_Long;     break;
    case 'F': tag = CONSTANT_Float;    break;
    case 'D': tag = CONSTANT_Double;   break;
    case 'B': case 'S': case 'C':
    case 'Z': tag = CONSTANT_Integer;  break;
    default:  assert(false);
    }
    f.constantValue = field_constant_value.getRefUsing(getCPIndex(tag));
  }
  field_constant_value.done();

  // Read method Exceptions attributes:
  method_exception_count.readFrom(rp);
  for (i = 0; i < excMethods.len; i++) {
    member& m = *(member*) excMethods.get(i);
    int ne = method_exception_count.getUnsigned();
    assert(ne > 0);
    m.exceptions = NEW(entry*, m.nexceptions = ne);
    method_exceptions.expectMoreLength(ne);
  }
  method_exception_count.done();
  method_exceptions.readFrom(rp);
  for (i = 0; i < excMethods.len; i++) {
    member& m = *(member*) excMethods.get(i);
    int ne = m.nexceptions;
    for (int j = 0; j < ne; j++) {
      m.exceptions[j] = method_exceptions.getRef();
    }
  }
  method_exceptions.done();

  // Get ready to read code attributes:
  code* codes = pkg.codes = NEW(code, pkg.ncodes = codeMethods.len);
  for (i = 0; i < codeMethods.len; i++) {
    member& m = *(member*) codeMethods.get(i);
    m.methodCode = &codes[i];
    codes[i].m = &m;
  }
  codeMethods.free();
  for (i = 0; i < nklasses; i++) {
    klass& k = pkg.klasses[i];
    k.codes = codes;
    codes += k.ncodes;
  }
}

static void read_klasses() {
  BAND_LOCAL;
  int i;
  int minver = pkg.default_class_minor_version;
  int majver = pkg.default_class_major_version;
  int nklasses = pkg.nklasses;
  klass* klasses = pkg.klasses;
  printcr(1,"  ...building %d classes...",pkg.nklasses);
  class_flags.readFrom(rp);
  class_this.readFrom(rp);
  class_super.readFrom(rp);
  class_interface_count.readFrom(rp);
  for (i = 0; i < nklasses; i++) {
    klass& k = klasses[i];
    int    flags      = class_flags.getFlags();
    bool   hasAttrs   = (flags & ACC_HAS_ATTRIBUTES) != 0;
    flags &= ~ACC_HAS_ATTRIBUTES;

    k.flags = flags;
    k.name = class_this.getRef();
    k.sup  = class_super.getRef();
    printcr(3, "klass name = %s", k.name->string());
    printcr(4, "sup = %s", k.sup == null ? "<none>" : k.sup->string());
    int nintfs = class_interface_count.getUnsigned();
    k.intfs = NEW(entry*, k.nintfs = nintfs);
    // fill in the interfaces in a moment
    class_interface.expectMoreLength(nintfs);
    if (hasAttrs)  class_attrs.expectClient(&k);
    // fixed overhead is flags, name, sup, flags, nintf, nf, nm, na
    k.est_size = 8*2 + nintfs*2;
    k.minver = minver; // update later
    k.majver = majver; // update later
  }
  class_flags.done();
  class_this.done();
  class_super.done();
  class_interface_count.done();

  // Read the class_interface band.
  class_interface.readFrom(rp);
  for (i = 0; i < nklasses; i++) {
    int    nintfs = klasses[i].nintfs;
    entry** intfs = klasses[i].intfs;
    for (int j = 0; j < nintfs; j++) {
      intfs[j] = class_interface.getRef();
      printcr(4, "klass %s intfs[%d] = %s",
	      klasses[i].name->string(), j, intfs[j]->string());
    }
  }
  class_interface.done();
  read_ics();
  read_members();
  printcr(1,"parsed %d classes and %d inner classes", nklasses, pkg.nics);
}

static void read_code_headers() {
  BAND_LOCAL;
  int i;
  code* codes = pkg.codes;
  int  ncodes = pkg.ncodes;
  code_headers.expectLength(ncodes);
  code_headers.readFrom(rp);
  int nhandlersTotal = 0;
  ptrlist longCodes;
  longCodes.init();
  for (i = 0; i < ncodes; i++) {
    code& c = codes[i];
    int sc = code_headers.getUnsigned();
    assert(sc == (sc & 0xFF));
    printcr(5,"codeHeader 0x%p = %d",&c,sc);
    if (sc == 0) {
      code_max_stack.expectMoreLength(1);
      code_max_locals.expectMoreLength(1);
      code_handler_count.expectMoreLength(1);
      code_attrs.expectClient(&c);
      longCodes.add(&c);
      continue;
    }
    // Short code header is the usual case:
    int nh;
    int mod;
    if (sc < 1 + 12*12) {
      sc -= 1;
      nh = 0;
      mod = 12;
    } else if (sc < 1 + 12*12 + 8*8) {
      sc -= 1 + 12*12;
      nh = 1;
      mod = 8;
    } else {
      assert(sc < 1 + 12*12 + 8*8 + 7*7);
      sc -= 1 + 12*12 + 8*8;
      nh = 2;
      mod = 7;
    }
    member& m = *c.m;
    int siglen = m.type()->typeSize();
    if ((m.flags & ACC_STATIC) == 0)  siglen++;
    c.max_stack  = sc % mod;
    c.max_locals = sc / mod + siglen;
    c.handlers = NEW(handler, c.nhandlers = nh);
    nhandlersTotal += nh;
  }
  code_headers.done();

  // Do the long headers now.
  code_max_stack.readFrom(rp);
  code_max_locals.readFrom(rp);
  code_handler_count.readFrom(rp);
  for (i = 0; i < longCodes.len; i++) {
    code& c = *(code*) longCodes.get(i);
    c.max_stack  = code_max_stack.getUnsigned();
    c.max_locals = code_max_locals.getUnsigned();
    int nh = code_handler_count.getUnsigned();
    c.handlers = NEW(handler, c.nhandlers = nh);
    nhandlersTotal += nh;
  }
  code_max_stack.done();
  code_max_locals.done();
  code_handler_count.done();
  // Read handler specifications.
  code_handler_start.expectLength(nhandlersTotal);
  code_handler_end.expectLength(nhandlersTotal);
  code_handler_catch.expectLength(nhandlersTotal);
  code_handler_class.expectLength(nhandlersTotal);
  code_handler_start.readFrom(rp);
  code_handler_end.readFrom(rp);
  code_handler_catch.readFrom(rp);
  code_handler_class.readFrom(rp);
  for (i = 0; i < ncodes; i++) {
    code& c = codes[i];
    int sum;
    int nh = c.nhandlers;
    for (int j = 0; j < nh; j++) {
      handler& h = c.handlers[j];
      h.klass = code_handler_class.getRef();  // null OK
      sum = code_handler_start.getUnsigned();
      h.start = sum;
      sum += code_handler_end.getUnsigned();
      h.end   = sum;
      sum += code_handler_catch.getUnsigned();
      h.catsh = sum;
      // (+) Note:  Sums may overflow code length,
      // in order to express negative offsets via
      // unsigned numbers.  These are fixed up
      // later in postprocessing, in expand_bc_ops.
    }
  }
  code_handler_start.done();
  code_handler_end.done();
  code_handler_catch.done();
  code_handler_class.done();
}

static void read_attributes() {
  BAND_LOCAL;
  ATTR_BAND_LOCAL;
  int n;
  package_attrs.expectClient(NOT_PRODUCT(&pkg)+0);
  // xxx_attrs.expectClient() calls have set up all the counts.
  assert(package_attrs.count->length = 1);
  for (n = 0; n < ATTR_BAND_LIMIT; n++) {
    if (&all_attr_bands[n] != &package_attrs)
      all_attr_bands[n].readCountsFrom(rp);
    else {
      // do this special one manually:
      byte* fake_rp = NEW(byte, 4);
      int tem = pkg.a.nattrs;
      fake_rp[0] = (byte) tem; tem >>= 8;
      fake_rp[1] = (byte) tem; tem >>= 8;
      fake_rp[2] = (byte) tem; tem >>= 8;
      fake_rp[3] = (byte) tem;
      all_attr_bands[n].readCountsFrom(fake_rp);
    }
  }

  for (n = 0; n < ATTR_BAND_LIMIT; n++) {
    all_attr_bands[n].readNamesFrom(rp);
  }

  for (n = 0; n < ATTR_BAND_LIMIT; n++) {
    all_attr_bands[n].readSizesFrom(rp);
  }

  // Got the raw bits.  Now parse the attributes.
  for (n = 0; n < ATTR_BAND_LIMIT; n++) {
    attr_bands& which_attrs = all_attr_bands[n];
    char        what_kind   = ATTR_BAND_CLIENT_KINDS[n];
    klass*      which_klass = null;
    which_attrs.setBitsInputFrom(rp);
    for (int i = 0; i < which_attrs.clients.len; i++) {
      void*      client = which_attrs.clients.get(i);
      attrlist*  client_attrs;
      switch (what_kind) {
      case 'f': case 'm':
	{
	  member& m = *(member*) client;
	  client_attrs = &m.a;
	  which_klass = m.k;
	  break;
	}
      case 'k':
	{
	  klass& k = *(klass*) client;
	  client_attrs = &k.a;
	  which_klass = &k;
	  break;
	}
      case 'c':
	{
	  code& c = *(code*) client;
	  client_attrs = &c.a;
	  which_klass = c.m->k;
	  break;
	}
      case 'p':
	{
	  client_attrs = &pkg.a;
	  assert(client == &pkg);
	  break;
	}
      default:   assert(false);
      }
      int        nattrs = which_attrs.count->getUnsigned();
      attribute* attrs  = client_attrs->alloc(nattrs);
      int	 asize  = nattrs * 6;  // fixed overhead per attr
      for (int j = 0; j < nattrs; j++) {
	attrs[j].name  = which_attrs.name->getRef();
	int      len   = which_attrs.size->getUnsigned();
	attrs[j].b     = which_attrs.bits->getBytes(len);
	asize += 2 * (len | 1);
      }
      if (which_klass != null)
	which_klass->est_size += asize;
    }
    byte* rp0 = rp;
    which_attrs.done(rp);
    if (&which_attrs == &package_attrs)
      printcr(1,"parsed %d package attributes, of total size %d",
	      pkg.a.nattrs, (int)(rp - rp0));
  }
}

static void putrefAt(byte* wp, entry* e) {
  if (e == null) {
    putu2At(wp, 0);
    return;
  }
#ifndef PRODUCT
  if (largest_cp_ref[0] < e->inord)  largest_cp_ref[0] = e->inord;
#endif
  e->requestOutputIndex();
  assert(pkg.fixuparea.inBounds(wp));
  fixup* fixupptr = pkg.fixupptr++;
  byte* ptr = wp;
  fixupptr->init(ptr - pkg.fixuparea.ptr, e);
  ptr[0] = fixup::u2_ref_marker;
  NOT_PRODUCT(ptr[1] = 0x22);  // to make it easier to eyeball
  wp = ptr + 2;
}
static void putref(entry* e) {
  putrefAt((wp += 2)-2, e);
}

static inline bool is_in_range(uint n, uint min, uint max) {
  return n - min <= max - min;  // unsigned arithmetic!
}
static inline bool isFieldOp(int bc) {
  return is_in_range(bc, bc_getstatic, bc_putfield);
}
static inline bool isInvokeInitOp(int bc) {
  return is_in_range(bc, _invokeinit_op, _invokeinit_limit-1);
}
static inline bool isSelfLinkerOp(int bc) {
  return is_in_range(bc, _self_linker_op, _self_linker_limit-1);
}
static bool isBranchOp(int bc) {
  return is_in_range(bc, bc_ifeq,   bc_jsr)
      || is_in_range(bc, bc_ifnull, bc_jsr_w);
}
static bool isLocalSlotOp(int bc) {
  return is_in_range(bc, bc_iload,  bc_aload)
      || is_in_range(bc, bc_istore, bc_astore)
      || bc == bc_iinc || bc == bc_ret;
}
static band* getCPRefOpBand(int bc) {
  BAND_LOCAL;
  switch (bc) {
  case _ildc:
  case _ildc_w:
    return &bc_intref;
  case _fldc:
  case _fldc_w:
    return &bc_floatref;
  case _lldc2_w:
    return &bc_longref;
  case _dldc2_w:
    return &bc_doubleref;
  case _aldc:
  case _aldc_w:
    return &bc_stringref;

  case bc_getstatic:
  case bc_putstatic:
  case bc_getfield:
  case bc_putfield:
    return &bc_fieldref;

  case bc_invokevirtual:
  case bc_invokespecial:
  case bc_invokestatic:
    return &bc_methodref;
  case bc_invokeinterface:
    return &bc_imethodref;

  case bc_new:
  case bc_anewarray:
  case bc_checkcast:
  case bc_instanceof:
  case bc_multianewarray:
    return &bc_classref;
  }
  return null;
}
static band* getSelfOpRefBand(int bc, bool& isAloadVar, int& origBCVar) {
  BAND_LOCAL;
  if (!isSelfLinkerOp(bc))  return null;
  int idx = (bc - _self_linker_op);
  bool isSuper = (idx >= _self_linker_super_flag);
  if (isSuper)  idx -= _self_linker_super_flag;
  bool isAload = (idx >= _self_linker_aload_flag);
  if (isAload)  idx -= _self_linker_aload_flag;
  int origBC = _first_linker_op + idx;
  bool isField = isFieldOp(origBC);
  isAloadVar = isAload;
  origBCVar  = _first_linker_op + idx;
  if (!isSuper)
    return isField? &bc_thisfield: &bc_thismethod;
  else
    return isField? &bc_superfield: &bc_supermethod;
}

static void read_bc_ops(int* &allSwitchLengths) {
  ptrlist allSwitchTypes;
  allSwitchTypes.init();

  BAND_LOCAL;
  band* bc_which;
  code* codes = pkg.codes;
  int  ncodes = pkg.ncodes;
  bc_codes.setInputFrom(rp);
  byte* opptr = bc_codes.rp;
  int   oplen;
  bool  isAload;  // passed by ref and then ignored
  int   junkBC;   // passed by ref and then ignored
  for (int k = 0; k < ncodes; k++) {
    // Scan one method:
    code& c = codes[k];
    c.b.ptr = opptr;
    for (;;) {
      int bc = *opptr++ & 0xFF;
      bool isWide = false;
      if (bc == bc_wide) {
	bc = *opptr++ & 0xFF;
	isWide = true;
      }
      // Adjust expectations of various band sizes.
      switch (bc) {
      case bc_tableswitch:
      case bc_lookupswitch:
	bc_case_count.expectMoreLength(1);
	allSwitchTypes.add((void*)bc);
	break;
      case bc_iinc:
	{
	  bc_local.expectMoreLength(1);
	  bc_which = isWide ? &bc_short : &bc_byte;
	  bc_which->expectMoreLength(1);
	}
      break;
      case bc_sipush:
	bc_short.expectMoreLength(1);
	break;
      case bc_bipush:
	bc_byte.expectMoreLength(1);
	break;
      case bc_newarray:
	bc_byte.expectMoreLength(1);
	break;
      case bc_multianewarray:
	assert(getCPRefOpBand(bc) == &bc_classref);
	bc_classref.expectMoreLength(1);
	bc_byte.expectMoreLength(1);
	break;
      default:
	if (isInvokeInitOp(bc)) {
	  bc_initref.expectMoreLength(1);
	  break;
	}
	bc_which = getSelfOpRefBand(bc, isAload, junkBC);
	if (bc_which != null) {
	  bc_which->expectMoreLength(1);
	  break;
	}
	if (isBranchOp(bc)) {
	  bc_label.expectMoreLength(1);
	  break;
	}
	bc_which = getCPRefOpBand(bc);
	if (bc_which != null) {
	  bc_which->expectMoreLength(1);
	  assert(bc != bc_multianewarray);  // handled elsewhere
	  break;
	}
	if (isLocalSlotOp(bc)) {
	  bc_local.expectMoreLength(1);
	  break;
	}
	break;
      case _end_marker:
	{
	  int oplen = (opptr-1) - c.b.ptr;
	  c.b.len = oplen;  // just temporary
	  goto doneScanningMethod;
	}
      }
    }
  doneScanningMethod:{}
  }
  rp = bc_codes.rp = opptr;  // copy out final read position
  bc_codes.done();

  // To size instruction bands correctly, we need info on switches:
  bc_case_count.readFrom(rp);
  allSwitchLengths = bc_case_count.getValueArray();
  bc_case_count.done();
  for (int i = 0; i < allSwitchTypes.len; i++) {
    int caseCount = allSwitchLengths[i];
    int bc        = (int) allSwitchTypes.get(i);
    bc_label.expectMoreLength(1+caseCount); // default label + cases
    bc_case_value.expectMoreLength(bc == bc_tableswitch ? 1 : caseCount);
  }

  allSwitchTypes.free();
}

static void expand_bc_ops(int* &allSwitchLengths) {
  BAND_LOCAL;
  CP_LOCAL;
  band* bc_which;
  code* codes = pkg.codes;
  int  ncodes = pkg.ncodes;

  bytes codebuf;
  codebuf.malloc(CODE_SIZE_LIMIT);
  init_pkg_fixups(CODE_SIZE_LIMIT/2, codebuf);
  cp.suppressOutputIndexes();  // avoid overhead of OI generation

  fixup* fixupbase = pkg.fixupptr;
  byte*  codebase  = codebuf.ptr;

  int nsw = 0;  // index into allSwitchLengths
  for (int n = 0; n < ncodes; n++) {
    // Scan one method:
    code& c = codes[n];
    byte*  opptr   = c.b.ptr;
    byte*  oplimit = c.b.limit();

    // reset global fill pointers:
    wp           = codebase;
    pkg.fixupptr = fixupbase;

    bool   isAload;  // copy-out result
    int    origBC;

    klass& k = *c.m->k;

    entry* thisClass  = k.name;
    entry* superClass = k.sup;
    entry* newClass   = null;  // class of last _new opcode

    // overwrite any prior index on these bands; it changes w/ current class:
    bc_thisfield.setIndex(    cp.getFieldIndex( thisClass));
    bc_thismethod.setIndex(   cp.getMethodIndex(thisClass));
    if (superClass != null) {
      bc_superfield.setIndex( cp.getFieldIndex( superClass));
      bc_supermethod.setIndex(cp.getMethodIndex(superClass));
    } else {
      NOT_PRODUCT(bc_superfield.setIndex(null));
      NOT_PRODUCT(bc_supermethod.setIndex(null));
    }

    while (opptr < oplimit) {
      assert(codebuf.inBounds(wp));
      assert(pkg.fixupbuf.inBounds(pkg.fixupptr));
      int bc = *opptr++ & 0xFF;
      putu1(bc);
      bool isWide = false;
      if (bc == bc_wide) {
	bc = *opptr++ & 0xFF;
	putu1(bc);
	isWide = true;
      }
      switch (bc) {
      case bc_tableswitch: // apc:  (df, lo, hi, (hi-lo+1)*(label))
      case bc_lookupswitch: // apc:  (df, nc, nc*(case, label))
	{
	  int caseCount = allSwitchLengths[nsw++];
	  while (((wp - codebase) % 4) != 0)  putu1(0);
	  int df = bc_label.getValue();
	  putu4(df);
	  if (bc == bc_tableswitch) {
	    int lo = bc_case_value.getValue();
	    int hi = lo + caseCount-1;
	    putu4(lo);
	    putu4(hi);
	    for (int j = 0; j < caseCount; j++) {
	      int lVal = bc_label.getValue();
	      //int cVal = lo + j;
	      putu4(lVal);
	    }
	  } else {
	    putu4(caseCount);
	    for (int j = 0; j < caseCount; j++) {
	      int cVal = bc_case_value.getValue();
	      int lVal = bc_label.getValue();
	      putu4(cVal);
	      putu4(lVal);
	    }
	  }
	  continue;
	}
      case bc_iinc:
	{
	  int local = bc_local.getUnsigned();
	  int delta = (isWide ? bc_short : bc_byte).getValue();
	  if (isWide) {
	    putu2(local);
	    putu2(delta);
	  } else {
	    putu1(local);
	    putu1(delta);
	  }
	  continue;
	}
      case bc_sipush:
	{
	  int val = bc_short.getValue();
	  putu2(val);
	  continue;
	}
      case bc_bipush:
      case bc_newarray:
	{
	  int val = bc_byte.getValue();
	  putu1(val);
	  continue;
	}
      default:
	if (isInvokeInitOp(bc)) {
	  origBC = bc_invokespecial;
	  entry* classRef;
	  switch (bc - _invokeinit_op) {
	  case _invokeinit_self_option:
	    classRef = thisClass; break;
	  case _invokeinit_super_option:
	    classRef = superClass; break;
	  case _invokeinit_new_option:
	  default:
	    assert(bc == _invokeinit_op+_invokeinit_new_option);
	    classRef = newClass; break;
	  }
	  wp[-1] = origBC;  // overwrite with origBC
	  int coding = bc_initref.getUnsigned();
	  // Find the nth overloading of <init> in classRef.
	  entry* ref;
	  entry** ix = cp.getMethodIndex(classRef);
	  for (int j = 0; ; j++) {
	    ref = ix[j];
	    assert(ref != null && ref->tag == CONSTANT_Methodref);
	    if (ref->memberDescr()->descrName() == cp.initName) {
	      if (coding == 0)  break;
	      --coding;
	    }
	  }
	  putref(ref);
	  continue;
	}
	bc_which = getSelfOpRefBand(bc, isAload, origBC);
	if (bc_which != null) {
	  if (!isAload) {
	    wp[-1] = origBC;  // overwrite with origBC
	  } else {
	    wp[-1] = bc_aload_0;  // overwrite with _aload_0
	    putu1(origBC);
	  }
	  entry* ref = bc_which->getRef();
	  putref(ref);
	  continue;
	}
	if (isBranchOp(bc)) {
	  int lVal = bc_label.getValue();
	  if (bc < bc_goto_w) {
	    assert(lVal == (short)lVal);
	    putu2(lVal & 0xFFFF);
	  } else {
	    assert(bc <= bc_jsr_w);
	    putu4(lVal);
	  }
	  continue;
	}
	bc_which = getCPRefOpBand(bc);
	if (bc_which != null) {
	  entry* ref = bc_which->getRef();
	  if (ref == null) {
	    assert(bc_which == &bc_classref);
	    // Shorthand for class self-references.
	    ref = thisClass;
	  }
	  origBC = bc;
	  switch (bc) {
	  case _ildc:
	  case _fldc:
	  case _aldc:
	    origBC = bc_ldc;
	    break;
	  case _ildc_w:
	  case _fldc_w:
	  case _aldc_w:
	    origBC = bc_ldc_w;
	    break;
	  case _lldc2_w:
	  case _dldc2_w:
	    origBC = bc_ldc2_w;
	    break;
	  case bc_new:
	    newClass = ref;
	    break;
	  }
	  wp[-1] = origBC;  // overwrite with origBC
	  putref(ref);
	  if (origBC == bc_ldc) {
	    (--wp)[-1] = fixup::u1_ref_marker;  // back up over 2-byte ref
	  }
	  if (origBC == bc_multianewarray) {
	    // Copy the trailing byte also.
	    int val = bc_byte.getValue();
	    putu1(val);
	  } else if (origBC == bc_invokeinterface) {
	    int argSize = ref->memberDescr()->descrType()->typeSize();
	    putu1(1 + argSize);
	    putu1(0);
	  }
	  continue;
	}
	if (isLocalSlotOp(bc)) {
	  int local = bc_local.getUnsigned();
	  if (isWide) {
	    putu2(local);
	    if (bc == bc_iinc) {
	      int iVal = bc_short.getValue();
	      putu2(iVal);
	    }
	  } else {
	    putu1(local);
	    if (bc == bc_iinc) {
	      int iVal = bc_byte.getValue();
	      putu1(iVal);
	    }
	  }
	  continue;
	}
	// Random bytecode.  Just copy it.
	assert(bc < bc_bytecode_limit);
      }
    }
    // now make a permanent copy of the bytecodes
    int codelen = wp - codebase;
    int nfixups = pkg.fixupptr - fixupbase;
    c.b.saveFrom(codebuf.ptr, codelen);
    bytes fixups;
    fixups.saveFrom(fixupbase, nfixups * sizeof(fixup));
    c.nfixups = nfixups;
    c.fixups = (fixup*) fixups.ptr;

    // (+) given codelen, fix handler offset (cf. PRCode.adjustBCOffsets)
    for (int j = 0; j < c.nhandlers; j++) {
      handler& h = c.handlers[j];
      assert(h.start < codelen);
      h.end   %= codelen;
      h.catsh %= codelen;
    }

    // update estimated size
    // fixed code overhead is aname, alen[2], stk, loc, clen[2], nh, na
    k.est_size += 9*2 + c.nhandlers*2*4 + codelen;
  }
#undef CODEPTR
#undef CODEIDX

  codebuf.free();
  NOT_PRODUCT(pkg.fixuparea.set(null, 0));  // nuke dangling ref
}

static void read_bcs() {
  BAND_LOCAL;
  int i;
  code* codes = pkg.codes;
  int  ncodes = pkg.ncodes;

  printcr(3, "reading compressed bytecodes and operands for %d codes...",
	  ncodes);
  byte* rp0 = rp;
  // reads from bc_codes and bc_case_count
  int* allSwitchLengths = 0;
  read_bc_ops(allSwitchLengths);

  for (i = e_bc_case_value; i <= e_bc_initref; i++) {
    all_bands[i].readFrom(rp);
  }
  printcr(3, "expanding %d compressed bytecodes for %d codes...",
	  (int)(bc_codes.rp - rp0), ncodes);
  expand_bc_ops(allSwitchLengths);
  for (i = e_bc_case_value; i <= e_bc_initref; i++) {
    all_bands[i].done();
  }

  printcr(1,"parsed %d code segments of total size %d", ncodes,
	  (int)(rp - rp0));
}

static void post_process() {
  int i;

  int nklasses = pkg.nklasses;
  klass* klasses = pkg.klasses;
  int max_klass_size = 0;
  for (i = 0; i < nklasses; i++) {
    klass& k = klasses[i];
    if (max_klass_size < k.est_size)
      max_klass_size = k.est_size;
  }
  // maybe add an InnerClasses attribute:
  max_klass_size += 6 + 2 + pkg.nics*4*2;
  pkg.max_klass_size = max_klass_size;

  pkg.cp.computeOutputOrder();
}

void read_pkgfile() {
  byte* rp0 = rp;
  int i;

  read_magic_numbers();
  read_header();
  read_cp();
  read_klasses();
  read_code_headers();
  read_attributes();
  read_bcs();

  // Done reading.
  assert(rp - rp0 == pkg.pkgfile.len);

  post_process();
}

// CP routines

entry*& cpool::findIndex(byte tag, bytes& b) {
  unsigned hash1 = b.len;
  for (int i = 0; i < b.len; i++) {
    hash1 = hash1 * 31 + (0xFF & b.ptr[i]);
  }
  unsigned hash2 = ((hash1 % 113) | 1);  // relatively prime to nutf8Indexes
  entry** ix = indexes;
  int    nix = nindexes;
  hash1 %= (nix-1);
  hash2 %= nix;
  unsigned u = hash1;
  int probes = 0;
  while (ix[u] != null) {
    entry& e = *ix[u];
    if (e.tag == tag && e.value.b.equals(b))
      break;
    u += hash2;
    if (u >= nix)  u -= nix;
    assert(u < nix);
    assert(++probes < nix);
  }
  #ifndef PRODUCT
  hash_probes[0] += 1;
  hash_probes[1] += probes;
  #endif
  return ix[u];
}

// install all well-known names in the cpool:
void cpool::initWellKnownNames() {
  for (int j = 0; ; j++) {
    const char* name   = wkn_tab[j].name;
    int         offset = wkn_tab[j].offset;
    if (name == null)  break;
    entry*     &cpref  = *(entry**)( (byte*)this + offset );
    bytes nb;
    nb.set(name);
    cpref = ensureUtf8(nb);
  }
}

entry* cpool::ensureUtf8(bytes& b) {
  entry*& ix = findIndex(CONSTANT_Utf8, b);
  if (ix != null)  return ix;
  // Make one.
  entry& e = entries[nentries++];
  e.tag = CONSTANT_Utf8;
  e.value.b.saveFrom(b);
  outputSize += 3+b.len;
  printcr(4,"ensureUtf8 miss %s", e.string());
  return ix = &e;
}

entry* cpool::ensureClass(bytes& b) {
  entry*& ix = findIndex(CONSTANT_Class, b);
  if (ix != null)  return ix;
  // Make one.
  entry& e = entries[nentries++];
  e.tag = CONSTANT_Class;
  e.nrefs = 1;
  e.refs = e.init_refs;
  ix = &e;  // hold my spot in the index
  entry* utf = ensureUtf8(b);
  e.refs[0] = utf;
  e.value.b = utf->value.b;
  outputSize += 3;
  printcr(4,"ensureClass miss %s", e.string());
  return &e;
}

void cpool::expandSignatures() {
  int i;
  bytes buf;
  int nsigs = 0;
  int nreused = 0;
  int first_sig = tag_base[CONSTANT_Signature];
  int sig_limit = tag_count[CONSTANT_Signature] + first_sig;
  buf.malloc(maxSignatureForm + (maxSignatureFormLen * maxClass)+100);
  int addedOutputSize = 0;
  for (i = first_sig; i < sig_limit; i++) {
    entry& e = entries[i];
    assert(e.tag == CONSTANT_Signature);
    int refnum = 0;
    bytes form = e.refs[refnum++]->asUtf8();
    byte* bp = buf.ptr;
    for (int j = 0; j < form.len; j++) {
      int c = form.ptr[j];
      assert(buf.inBounds(bp));
      *bp++ = c;
      if (c != 'L')  continue;
      entry* cls = e.refs[refnum++];
      bytes clsName = cls->className()->asUtf8();
      bp = clsName.writeTo(bp);
      assert(buf.inBounds(bp));
    }
    assert(refnum == e.nrefs);
    bytes sig;
    sig.saveFrom(buf.ptr, bp - buf.ptr);
    printcr(5,"signature %d %s -> %s", e.inord, form.ptr, sig.ptr);
    bool freeSig = false;
#ifdef REPLACE_SIGS
    // try to find a pre-existing Utf8:
    entry* &e2 = findIndex(CONSTANT_Utf8, sig);
    if (e2 != null) {
      assert(e2->isUtf8(sig));
      e.value.b = e2->value.b;
      e.refs[0] = e2;
      e.nrefs = 1;
      printcr(5,"signature replaced %d => %s", i, e.string());
      nreused++;
      freeSig = true;
    } else {
      // there is no other replacement; reuse this CP entry as a Utf8
      e.value.b = sig;
      e.tag = CONSTANT_Utf8;
      e.nrefs = 0;
      addedOutputSize += 3+sig.len;
      e2 = &e;
      printcr(5,"signature changed %d => %s", e.inord, e.string());
    }
    #ifndef PRODUCT
    if (nentries < 2500) {
      for (int i3 = 0; i3 < nentries; i3++) {
	entry& e3 = entries[i3];
	if (e3.isUtf8(e.value.b)) {
	  assert(&e3 == e2);
	}
      }
    }
    #endif
#else // REPLACE_SIGS
    // from now on, this entry acts like a Utf8, but may alias another Utf8
    e.value.b = sig;
    addedOutputSize += 3+sig.len;
#endif // REPLACE_SIGS
    nsigs++;
    if (freeSig)
      sig.free();
  }
  outputSize += addedOutputSize;
  printcr(1,"expanded %d signatures (reused %d utfs) adding total size %d", nsigs, nreused, addedOutputSize);

#ifdef REPLACE_SIGS
  // go expunge all references to remaining signatures:
  for (i = 0; i < nentries; i++) {
    entry& e = entries[i];
    for (int j = 0; j < e.nrefs; j++) {
      entry*& e2 = e.refs[j];
      if (e2->tag == CONSTANT_Signature)
	e2 = e2->refs[0];
    }
  }
#endif
}

void cpool::initMemberIndexes() {
  // This function does NOT refer to the klass schema.
  // It is totally internal to the cpool.
  int i, j;

  // Get the pre-existing indexes:
  int   nclasses = tag_count[CONSTANT_Class];
  entry* classes = tag_base[CONSTANT_Class] + entries;
  int   nfields  = tag_count[CONSTANT_Fieldref];
  entry* fields  = tag_base[CONSTANT_Fieldref] + entries;
  int   nmethods = tag_count[CONSTANT_Methodref];
  entry* methods = tag_base[CONSTANT_Methodref] + entries;

  int* field_counts   = NEW(int, nclasses);
  int* method_counts  = NEW(int, nclasses);
  entry*** field_ixs  = NEW(entry**, nclasses);
  entry*** method_ixs = NEW(entry**, nclasses);
  entry**  field_ix   = NEW(entry*, nfields+nclasses);
  entry**  method_ix  = NEW(entry*, nmethods+nclasses);
  NOT_PRODUCT(memset(field_ix,  -1, (nfields+nclasses)*sizeof(entry*)));
  NOT_PRODUCT(memset(method_ix, -1, (nmethods+nclasses)*sizeof(entry*)));

  for (j = 0; j < nfields; j++) {
    entry& f = fields[j];
    i = f.memberClass()->inord;
    field_counts[i]++;
  }
  for (j = 0; j < nmethods; j++) {
    entry& m = methods[j];
    i = m.memberClass()->inord;
    method_counts[i]++;
  }
  int fbase = 0, mbase = 0;
  for (i = 0; i < nclasses; i++) {
    field_ixs[i]  = field_ix+fbase;
    method_ixs[i] = method_ix+mbase;
    int fc = field_counts[i];
    int mc = method_counts[i];
    // reuse field_counts and member_counts as fill pointers:
    field_counts[i] = fbase;
    method_counts[i] = mbase;
    fbase += fc+1;
    mbase += mc+1;
    // (the +1 leaves a space between every subarray)
    NOT_PRODUCT(field_ix[fbase-1]  = null);
    NOT_PRODUCT(method_ix[mbase-1] = null);
  }
  assert(fbase == nfields+nclasses);
  assert(mbase == nmethods+nclasses);

  for (j = 0; j < nfields; j++) {
    entry& f = fields[j];
    i = f.memberClass()->inord;
    field_ix[field_counts[i]++] = &f;
  }
  for (j = 0; j < nmethods; j++) {
    entry& m = methods[j];
    i = m.memberClass()->inord;
    method_ix[method_counts[i]++] = &m;
  }

  free(method_counts);
  free(field_counts);

  field_indexes = field_ixs;
  method_indexes = method_ixs;

#ifndef PRODUCT
  int fvisited = 0, mvisited = 0;
  int prevord;
  for (i = 0; i < nclasses; i++) {
    entry*  cls = &classes[i];
    entry** fix = getFieldIndex(cls);
    prevord = -1;
    for (j = 0; fix[j] != null; j++) {
      entry* f = fix[j];
      assert(f->memberClass() == cls);
      assert(prevord < f->inord);
      prevord = f->inord;
      fvisited++;
    }
    entry** mix = getMethodIndex(cls);
    prevord = -1;
    for (j = 0; mix[j] != null; j++) {
      entry* m = mix[j];
      assert(m->memberClass() == cls);
      assert(prevord < m->inord);
      prevord = m->inord;
      mvisited++;
    }
  }
  assert(fvisited == nfields);
  assert(mvisited == nmethods);
#endif  
}

void entry::requestOutputIndex() {
  assert(outputIndex <= NOT_REQUESTED);  // must not have assigned indexes yet
  if (outputIndex != NOT_REQUESTED)  return;  // already done
  CP_LOCAL;
  if (cp.outputIndexLimit < 0)  return; // effect of suppressOutputIndexes
  outputIndex = REQUESTED;
  assert(!cp.outputEntries.contains(this));
  cp.outputEntries.add(this);
  if (tag != CONSTANT_Signature) {
    for (int j = 0; j < nrefs; j++) {
      refs[j]->requestOutputIndex();
    }
  }
}

void cpool::resetOutputIndexes() {
  int i;
  int   noes = outputEntries.len;
  void** oes = outputEntries.base();
  for (i = 0; i < noes; i++) {
    entry& e = *(entry*) oes[i];
    e.outputIndex = NOT_REQUESTED;
  }
  outputIndexLimit = 0;
  outputEntries.clear();
#ifndef PRODUCT
  // they must all be clear now
  for (i = 0; i < nentries; i++)
    assert(entries[i].outputIndex == NOT_REQUESTED);
#endif
}

// compare symbolically (ignoring outputIndex)
inline
int entry::compareTo(entry& e2) {
  entry& e1 = *this;
  if (e1.outord != e2.outord) {
    // use a precomputed order, for efficiency
    assert(pkg.cp.outputOrderLimit == pkg.cp.nentries);
    return e1.outord - e2.outord;
  } else if (e1.tag != e2.tag) {
    // tag is major key
    return TAG_ORDER[e1.tag] - TAG_ORDER[e2.tag];
  } else if (tag == CONSTANT_Utf8 || tag == CONSTANT_Class) {
    // since new Utf8s and Classes can be created,
    // do an explicit value comparison.
    // Use a slight variant of bytes::compareTo (memcmp).
    return compare_Utf8_chars(e1.value.b, e2.value.b);
  } else {
    return e1.inord - e2.inord;
  }
}

extern "C"
int outputEntry_cmp(const void* e1p, const void* e2p) {
  entry& e1 = *(entry*) *(void**) e1p;
  entry& e2 = *(entry*) *(void**) e2p;
  // ldc guys (OI==-2) come before all others (OI=-1):
  if (e1.outputIndex != e2.outputIndex)
    return e1.outputIndex - e2.outputIndex;
  else
    return e1.compareTo(e2);
}

void cpool::computeOutputOrder() {
  // assign outord for all elements
  int ne = nentries;
  outputOrderLimit = ne;
  printcr(3, "computing output order for %d CP entries...", ne);
  bytes buf;
  buf.saveFrom(entryRefs, ne * sizeof(entry*));
  qsort(buf.ptr, ne, sizeof(entry*), outputEntry_cmp);
  printcr(3, "done computing output order");
  entry** base = (entry**) buf.ptr;
  printcr(4, "output CP order:");
  for (int i = 0; i < ne; i++) {
    base[i]->outord = i;
    printcr(4, "%d: %s %s", i, tagName(base[i]->tag), base[i]->string());
  }
  buf.free();
}

void cpool::computeOutputIndexes() {
  int i;

#ifndef PRODUCT
  // Selected subset of ICs must have referential integrity.
  for (i = 0; i < pkg.nics; i++) {
    inner_klass& ic = pkg.ics[i];
    if (ic.inner->outputIndex != NOT_REQUESTED) {
      if (ic.outer != null)  assert(ic.outer->outputIndex < NOT_REQUESTED);
      if (ic.name != null)   assert(ic.name->outputIndex  < NOT_REQUESTED);
    }
  }

  // outputEntries must be a complete list of those requested:
  int checkLimit = nentries;
  if (checkLimit > 1000)  checkLimit = 1000;
  for (i = 0; i < checkLimit; i++) {
    entry& e = entries[i];
    if (e.outputIndex != NOT_REQUESTED) {
      assert(outputEntries.contains(&e));
    } else {
      assert(!outputEntries.contains(&e));
    }
  }
#endif

  // Sort the output constant pool.
  qsort(outputEntries.base(), outputEntries.len,
	sizeof(void*), outputEntry_cmp);
  
  // Allocate a new index for each entry that needs one.
  int   noes = outputEntries.len;
  void** oes = outputEntries.base();
  int nextIndex = 1;  // always skip index #0 in output cpool
  for (i = 0; i < noes; i++) {
    entry& e = *(entry*) oes[i];
    assert(e.outputIndex < NOT_REQUESTED);  // no aliasing tricks!
    e.outputIndex = nextIndex++;
    if (e.isDoubleWord())  nextIndex++;  // do not use the next index
  }
  outputIndexLimit = nextIndex;
  printcr(3,"renumbering CP to %d entries", outputIndexLimit);
}

void write_classfiles() {
  CP_LOCAL;
  const int SLOP = 5000;
  bytes& classfile_head = pkg.classfile_head;
  bytes& classfile_tail = pkg.classfile_tail;
  int est_cpool_size = 10 + cp.outputSize;
  int est_klass_size = pkg.max_klass_size;
  classfile_head.malloc(est_cpool_size + SLOP);
  classfile_tail.malloc(est_klass_size + SLOP);

  bytes segs[2]; // head & tail segments, clipped to actual length

  for (int i = 0; i < pkg.nklasses; i++) {
    klass& k = pkg.klasses[i];

    printcr(3, "writing %s...", k.name->string());

    // Reset the CP and fixup pointers:
    cp.resetOutputIndexes();
    init_pkg_fixups(est_klass_size/2, classfile_tail);

    // Write the klass part first.
    wp = classfile_tail.ptr;
    write_classfile_tail(k);
    assert(classfile_tail.inBounds(wp + SLOP-1));
    cp.computeOutputIndexes();

    fixup* fixups = (fixup*) pkg.fixupbuf.ptr;
    int   nfixups = pkg.fixupptr - fixups;
    assert(pkg.fixupbuf.inBounds(pkg.fixupptr));
    fixup::apply_list(nfixups, fixups, classfile_tail.ptr);
    printcr(3,"fixed up %d CP references outside codes", nfixups);
    NOT_PRODUCT(int ncfixups = 0);
    for (int j = 0; j < k.ncodes; j++) {
      code& c = k.codes[j];
      fixup::apply_list(c.nfixups, c.fixups, c.wp);
      NOT_PRODUCT(ncfixups += c.nfixups);
    }
#ifndef PRODUCT
    printcr(3,"fixed up %d CP references in %d codes", ncfixups, k.ncodes);
    pkg.fixuparea.set(null,0);
#endif

    segs[1] = classfile_tail.slice(0, (wp - classfile_tail.ptr));

    // Now we can write the CP, after seeing all the indexes.
    wp = classfile_head.ptr;
    write_classfile_head(k);
    assert(classfile_head.inBounds(wp + SLOP-1));

    segs[0] = classfile_head.slice(0, (wp - classfile_head.ptr));
    
    char* classfilename = k.classFileName();
    write_file(classfilename, 2, segs);
    free(classfilename);
  }
  classfile_head.free();
  classfile_tail.free();
}

void write_sidefiles() {
  int i;
  for (i = 0; i < pkg.a.nattrs; i++) {
    bytes name = pkg.a.attrs[i].name->asUtf8();
    if (name.contains('/')) {
      char* pname = name.strval();
      if (pname[0] == '.' && pname[1] == '/' ) pname = &pname[2]; 
      write_file(pname, 1, &pkg.a.attrs[i].b);
    } else {
      printcr(0, "Skipping unrecognized package attribute %s", name.strval());
    }
  }
}

static int getu() {
  static coding* coding_5_64_0;  // standard coding inside attributes
  coding* c = coding_5_64_0;
  if (c == null)
    coding_5_64_0 = c = coding::findBySpec(CODING_SPEC(5,64,0,0));
  return c->readFrom(rp, (int*) null);
}

static int copyu2() {
  int n = getu();
  putu2(n);
  return n;
}

static entry* getref(byte tag) {
  entry** ix = getCPIndex(tag);
  int n = getu();
  return (n == 0) ? null : ix[n-1];
}

static void copyref(byte tag) {
  putref(getref(tag));
}

static void copy_bytes(int len) {
  bytes b;
  b.set(rp, len);
  rp = b.limit();
  put_bytes(b);
}

enum {
  ATTR_UNRECOGNIZED = 0,
  ATTR_OK = 1,
  ATTR_NULLIFIED = -1
};

static int write_ClassFileVersion(attribute& at, char client_kind, void* client) {
  if (client_kind != 'k')  return ATTR_UNRECOGNIZED;
  klass& k = *(klass*) client;
  uint version = getref(CONSTANT_Integer)->asInteger();
  k.minver = version & 0xFFFF;
  k.majver = version >> 16;
  return ATTR_NULLIFIED;
}

#if FULL // support all attributes

static int write_Deprecated(attribute& at, char client_kind, void* client) {
  // no data
  if (strip_compile)  return ATTR_NULLIFIED;
  return ATTR_OK;
}

static int write_LineNumberTable(attribute& at, char client_kind, void* client) {
  if (client_kind != 'c')  return ATTR_UNRECOGNIZED;
  if (strip_debug)         return ATTR_NULLIFIED;
  int count = copyu2();
  while (count-- > 0) {
    copyu2(); // pc
    copyu2(); // line
  }
  return ATTR_OK;
}

static int write_LocalVariableTable(attribute& at, char client_kind, void* client) {
  if (client_kind != 'c')  return ATTR_UNRECOGNIZED;
  if (strip_debug)         return ATTR_NULLIFIED;
  int count = copyu2();
  while (count-- > 0) {
    copyu2();                // start_bci
    copyu2();                // length
    copyref(CONSTANT_Utf8);  // name
    copyref(CONSTANT_Utf8);  // signature
    copyu2();                // slot
  }
  return ATTR_OK;
}

static int write_Signature(attribute& at, char client_kind, void* client) {
  if (client_kind != 'm')  return ATTR_UNRECOGNIZED;
  copyref(CONSTANT_Utf8);
  return ATTR_OK;
}

static int write_SourceFile(attribute& at, char client_kind, void* client) {
  if (client_kind != 'k')  return ATTR_UNRECOGNIZED;
  if (strip_debug)         return ATTR_NULLIFIED;
  copyref(CONSTANT_Utf8);
  return ATTR_OK;
}

static int write_Synthetic(attribute& at, char client_kind, void* client) {
  // no data
  if (strip_compile)  return ATTR_NULLIFIED;
  return ATTR_OK;
}

#endif

// write part of an attr list, returning number of attrs emitted
// caller is responsible for patching the attr count (using putu2at wp-2)
int write_attrs(attrlist& a, char client_kind, void* client) {
  CP_LOCAL;
  int na = 0;

  for (int i = 0; i < a.nattrs; i++) {
    attribute& at = a.attrs[i];
    entry* at_name = at.name;
    byte* at_wp = (wp += 6); // placeholder for at_name, at_length
    fixup* at_fixupptr = pkg.fixupptr;
    //putref(at_name);
    //putu4(at_length);

    rp = at.b.ptr;  // we will read from the attribute for a bit

    int ok = ATTR_UNRECOGNIZED;

    if (false) ;

    #define CASE_WKA(N) \
    else if (at_name == cp.N) \
      ok = write_##N(at, client_kind, client);
    EACH_WK_ATTR_NAME(CASE_WKA)
    #undef CASE_WKA

    if (ok == ATTR_UNRECOGNIZED) {
      // unknown attribute !?
      printcr(0, "unknown attribute: %s [%d]", at_name->string(), at.b.len);
      put_bytes(at.b);
#ifdef PRODUCT
      ok = ATTR_NULLIFIED;  // nullify unrecognized attrs!
#endif
    }

    assert(ok == ATTR_NULLIFIED || rp == at.b.limit());
    rp = at.b.limit();

    if (ok == ATTR_NULLIFIED) {
      printcr(3, "nullifying attribute: %s", at_name->string());
      wp = at_wp-6;  // back up!
      assert(pkg.fixupptr == at_fixupptr);  // do not putref
      continue;
    }

    // patch in the name and size, now that we know we need it
    putrefAt(at_wp-6, at_name);
    putu4At( at_wp-4, wp - at_wp);
    na++;
  }

  return na;
}

// constant pool fixups

void fixup::apply_list(int nfixups, fixup* fixups, byte* base) {
  for (int i = 0; i < nfixups; i++) {
    uint  idx = fixups[i].ref->getOutputIndex();
    byte* wp  = fixups[i].off + base;
    assert(pkg.fixuparea.inBounds(wp));
    if (*wp == u1_ref_marker) {
      putu1At(wp, idx);
    } else {
      assert(*wp == u2_ref_marker);
      putu2At(wp, idx);
    }
  }
}

// classfile writing

void write_classfile_tail(klass& k) {
  CP_LOCAL;
  int i, j;

  putu2(k.flags);
  putref(k.name);
  putref(k.sup);
  putu2(k.nintfs);
  for (i = 0; i < k.nintfs; i++) {
    putref(k.intfs[i]);
  }

  // write fields
  for (int isM = (int)false; isM <= (int)true; isM++) {
    int nmems = k.nmems[isM];
    putu2(nmems);
    for (i = 0; i < nmems; i++) {
      member& m = k.mems[isM][i];
      assert((bool)isM == m.isMethod());
      putu2(m.flags);
      putref(m.name());
      putref(m.type());
      byte* na_wp = (wp += 2)-2;
      int na = write_attrs(m.a, isM ? 'm' : 'f', &m);
      if (m.constantValue != null) {
	assert(!isM);
	putref(cp.ConstantValue);
	putu4(2);
	putref(m.constantValue);
	na++;
      }
      int ne = m.nexceptions;
      if (ne != 0) {
	assert(isM);
	putref(cp.Exceptions);
	putu4(2 + 2*ne);
	putu2(ne);
	for (j = 0; j < ne; j++)
	  putref(m.exceptions[j]);
	na++;
      }
      if (m.methodCode != null) {
	assert(isM);
	code& c = *m.methodCode;
	putref(cp.Code);
	byte* at_wp = (wp += 4); // placeholder for at_length
	putu2(c.max_stack);
	putu2(c.max_locals);
	int   len = c.b.len;
	putu4(len);
	byte* outptr = c.wp = wp;  // for later execution of fixups
	wp = c.b.writeTo(outptr);
	// For each embedded ref, ask for an outputIndex.
	// Take care to mark the "ldc" references specially.
	int nf = c.nfixups;
	for (j = 0; j < nf; j++) {
	  entry* ref = c.fixups[j].ref;
	  byte*  loc = c.fixups[j].off + outptr;
	  ref->requestOutputIndex();
	  assert(ref->outputIndex < NOT_REQUESTED);
	  if (*loc == fixup::u1_ref_marker)
	    ref->outputIndex = REQUESTED_LDC;
	  else
	    assert(*loc == fixup::u2_ref_marker);
	}
	int nh = c.nhandlers;
	putu2(nh);
	for (j = 0; j < nh; j++) {
	  handler& h = c.handlers[j];
	  putu2(h.start);
	  putu2(h.end);
	  putu2(h.catsh);
	  putref(h.klass);
	}
	byte* nca_wp = (wp += 2)-2;
	int nca = write_attrs(c.a, 'c', &c);
	putu2At(nca_wp, nca);
	putu4At(at_wp-4, wp - at_wp);  // size of code attr
	na++;
      }
      putu2At(na_wp, na);
    }
  }

  byte* na_wp = (wp += 2)-2;
  int na = write_attrs(k.a, 'k', &k);

  // at the very last, choose which inner classes (if any) pertain to k:
  int         nics = pkg.nics;
  inner_klass* ics = pkg.ics;
  int    local_ics = 0;
  // decide on innerclasses, and add the relevant references:
  for (i = nics-1; i >= 0; --i) {
    // work in reverse order, to visit inners before outers
    inner_klass& ic = ics[i];
    if (ic.outer == k.name)
      // Include all members of the current class.
      ic.inner->requestOutputIndex();
    if (ic.inner->outputIndex != NOT_REQUESTED) {
      local_ics += 1;
      if (ic.outer != null)  ic.outer->requestOutputIndex();
      if (ic.name != null)   ic.name->requestOutputIndex();
    }
  }
  if (local_ics > 0) {
    // append the new attribute:
    putref(cp.InnerClasses);
    putu4(2 + 2*4*local_ics);
    putu2(local_ics);
    for (i = 0; i < nics; i++) {
      inner_klass& ic = ics[i];
      if (ic.inner->outputIndex != NOT_REQUESTED) {
	putref(ic.inner);
	putref(ic.outer);
	putref(ic.name);
	putu2(ic.flags);
	NOT_PRODUCT(local_ics--);
      }
    }
    assert(local_ics == 0);  // must balance
    na++;
  }
  putu2At(na_wp, na);
}

void write_classfile_head(klass& k) {
  CP_LOCAL;

  putu4(JAVA_MAGIC);
  putu2(k.minver);
  putu2(k.majver);

  byte* wp0 = wp;
  putu2(cp.outputIndexLimit);
  int   noes = cp.outputEntries.len;
  void** oes = cp.outputEntries.base();
  for (int i = 0; i < noes; i++) {
    entry& e = *(entry*) oes[i];
    assert(e.outputIndex > NOT_REQUESTED);
    byte tag = e.tag;
    if (tag == CONSTANT_Signature)
      tag = CONSTANT_Utf8;
    putu1(tag);
    switch (tag) {
    case CONSTANT_Utf8:
    case CONSTANT_Signature:
      putu2(e.value.b.len);
      put_bytes(e.value.b);
      break;
    case CONSTANT_Integer:
    case CONSTANT_Float:
      putu4(e.value.i);
      break;
    case CONSTANT_Long:
    case CONSTANT_Double:
      putu8(e.value.l);
      break;
    case CONSTANT_Class:
    case CONSTANT_String:
      // just write the ref
      putu2(e.refs[0]->getOutputIndex());
      break;
    case CONSTANT_Fieldref:
    case CONSTANT_Methodref:
    case CONSTANT_InterfaceMethodref:
    case CONSTANT_NameandType:
      putu2(e.refs[0]->getOutputIndex());
      putu2(e.refs[1]->getOutputIndex());
      break;
    default:
      unpack_abort("Error: Internal error");
    }
  }
#ifndef PRODUCT
  total_cp_size[0] += cp.outputIndexLimit;
  total_cp_size[1] += (wp - wp0);
#endif
}


#ifndef PRODUCT
// debugging goo

bool member::isMethod() {
  return type()->asUtf8().ptr[0] == '(';
}

static bytes& getbuf(int len) {  // for debugging only!
  static int bn = 0;
  static bytes bufs[8] = { 0 };
  bytes& buf = bufs[bn++ & 7];
  while (buf.len < len+10)
    buf.realloc(buf.len ? buf.len * 2 : 1000);
  buf.ptr[0] = 0;  // for the sake of strcat
  return buf;
}

char* entry::string() {
  bytes buf;
  switch (tag) {
  case CONSTANT_None:
    return (char*)"<empty>";
  case CONSTANT_Signature:
    if (value.b.ptr == null)
      return ref(0)->string();
    // else fall through:
  case CONSTANT_Utf8:
    buf = value.b;
    break;
  case CONSTANT_Integer:
  case CONSTANT_Float:
    buf = getbuf(12);
    sprintf((char*)buf.ptr, "0x%08x", value.i);
    break;
  case CONSTANT_Long:
  case CONSTANT_Double:
    buf = getbuf(24);
    sprintf((char*)buf.ptr, "0x%016llx", value.l);
    break;
  default:
    if (nrefs == 0) {
      buf = getbuf(20);
      sprintf((char*)buf.ptr, "<tag=%d>", tag);
    } else if (nrefs == 1) {
      return refs[0]->string();
    } else {
      char* s1 = refs[0]->string();
      char* s2 = refs[1]->string();
      buf = getbuf(strlen(s1) + 1 + strlen(s2) + 4 + 1);
      buf.strcat(s1).strcat(" ").strcat(s2);
      if (nrefs > 2)  buf.strcat(" ...");
    }
  }
  return (char*)buf.ptr;
}

void print_cp_entry(int i) {
  CP_LOCAL;
  entry& e = cp.entries[i];
  printf(" %d\t%s %s\n", i, tagName(e.tag), e.string());
}

void print_cp_entries(int beg, int end) {
  CP_LOCAL;
  for (int i = beg; i < end; i++)
    print_cp_entry(i);
}

void print_cp() {
  CP_LOCAL;
  print_cp_entries(0, cp.nentries);
}

#endif
