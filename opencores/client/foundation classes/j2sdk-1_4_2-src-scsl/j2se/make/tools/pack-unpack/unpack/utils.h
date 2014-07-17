/*
 * @(#)utils.h	1.9 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
 
//Definitions of our util functions
void setVerboseLevel(int n);
void incVerboseLevel(int n);
void* must_malloc(int size);

void setDisplayDialog(bool value);
bool getDisplayDialog();

extern FILE* errstrm;

void unpack_abort(const char *fmt,...);

#ifndef printcr
void printcr(int level, const char* fmt,...);
#endif

#ifdef PRODUCT
#define tagName(tag) (0)
#else
const char* tagName(uint tag);
#endif

// Support for standard classfile int formats:
uint getu2(byte* rp);
uint getu4(byte* rp);
static inline
void putu1At(byte* wp, int n) { *wp = n; }
void putu2At(byte* wp, int n);
void putu4At(byte* wp, int n);

// These take an implicit parameter of wp:
extern byte* wp;
void put_bytes(bytes& b);
void putu1(int n);
void putu2(int n);
void putu4(int n);
void putu8(jlong n);

void mkdirs(int oklen, char* path);

void remove_log();
void redirect_stdio(char *lfname);

struct ScanMarker {
  byte* &p;
  byte* pSaved;
  ScanMarker(byte* &p_) : p(p_) { pSaved = p; }
  ~ScanMarker()                 { p = pSaved; }
  void operator delete(void*) {}  // to avoid linkage to ::operator delete
};
