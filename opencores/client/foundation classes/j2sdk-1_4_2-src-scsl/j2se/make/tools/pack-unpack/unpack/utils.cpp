/*
 * @(#)utils.cpp	1.11 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
 
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include <sys/stat.h>

#ifdef _MSC_VER 
#include <direct.h>
#include <io.h>
#include <process.h>
#else
#include <unistd.h>
#endif

#include "constants.h"
#include "defines.h"
#include "bytes.h"
#include "utils.h"

FILE* errstrm;

//Globals
byte* rp;  // read pointer
byte* wp;  // write pointer

static char *log_file_name;

#ifndef PRODUCT
const char* tagNames[] = {
  "None",
  "Utf8",
  "Unicode",		/* unused */
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
  "Signature",

  "Literal", //pseudo-tag
};

static int verboseLevel = 1;

void setVerboseLevel(int n) {
  verboseLevel = n;
}

void incVerboseLevel(int n) {
  verboseLevel += n;
}

void printcr(int level, const char* fmt ...) {
  if (verboseLevel < level)  return;
  va_list vl;
  va_start(vl, fmt);
  char fmtbuf[9999];
  strcpy(fmtbuf+100, fmt);
  strcat(fmtbuf+100, "\n");
  char* fmt2 = fmtbuf+100;
  while (level-- > 0)  *--fmt2 = ' ';
  vfprintf(errstrm, fmt2, vl);
}

const char* tagName(uint tag) {
  if (tag <= CONSTANT_Limit && tagNames[tag] != null)
    return tagNames[tag];
  static char buf[20];
  sprintf(buf, "Tag#%d", tag);
  return buf;
}
#endif  //PRODUCT



void* must_malloc(int size) {
  void* ptr = malloc(size);
  if (ptr == null)  unpack_abort("ERROR: Internal Error");
  memset(ptr, 0, size);
  return ptr;
}


// Standard classfile u1, u2, and u4 formats.
uint getu2(byte* rp) {
  return ((rp[0] & 0xFF) << 8)
       | ((rp[1] & 0xFF) << 0);
}

uint getu4(byte* rp) {
  return ((rp[0] & 0xFF) << 24)
       | ((rp[1] & 0xFF) << 16)
       | ((rp[2] & 0xFF) << 8)
       | ((rp[3] & 0xFF) << 0);
}


// unparsing routines

extern byte* wp;

void putu1(int b) {
  *wp++ = b;
}

void put_bytes(bytes& b) {
  wp = b.writeTo(wp);
}

#define STOREU2(wp, x) { \
  wp[0] = (n) >> 8; \
  wp[1] = (n) >> 0; }

void putu2At(byte* wp, int n) {
  if (n != (unsigned short)n) {
#ifndef PRODUCT
    printcr(0, "Fatal Error:  Constant pool index overflow (%d)", n);
#endif
    unpack_abort("ERROR: Internal Error");
  }
  STOREU2(wp, n);
}

void putu2(int n) {
  putu2At((wp += 2) - 2, n);
}

void putu4At(byte* wp, int n) {
  putu2At(wp+0, (unsigned short)(n >> 16));
  putu2At(wp+2, (unsigned short)(n >>  0));
}

void putu4(int n) {
  putu4At((wp += 4) - 4, n);
}

void putu8(jlong n) {
  putu4(n >> 32);
  putu4(n >>  0);
}


void mkdirs(int oklen, char* path) {

  if (strlen(path) <= oklen)  return;
  char dir[PATH_MAX];

  strcpy(dir, path);
  char* slash = strrchr(dir, '/');
  if (slash == 0)  return;
  *slash = 0;
  mkdirs(oklen, dir);
  MKDIR(dir);
}




// We have our own abort since we need to suppress dialogs
// on windows.
void unpack_abort(const char *fmt ...) {
  va_list vl;
  va_start(vl, fmt);
  vfprintf(errstrm, fmt, vl);

#ifdef _DEBUG
    abort();
#else
    exit(-1);
#endif
}

// Remove the log file
void remove_log() {
   fprintf(errstrm,"log_file_name=%s",log_file_name);
   fclose(errstrm);
   if (log_file_name) remove(log_file_name);
}

void redirect_stdio(char *lfname) {


    if (lfname != NULL && (*lfname != '\0' && (errstrm = fopen(lfname,"a+")) != NULL)) { 
      log_file_name = lfname;
      return ;
    }
    log_file_name = (char *)must_malloc(PATH_MAX);
    char tmpdir[PATH_MAX];
#ifdef WIN32
    
    int n = GetTempPath(PATH_MAX,tmpdir); //API returns with trailing '\'
    if ( n < 1  || n > PATH_MAX) {
        sprintf(tmpdir,"C:\\");
    }
    sprintf(log_file_name,"%sunpack.log",tmpdir); 
#else
    sprintf(tmpdir,"/tmp");
    sprintf(log_file_name, "/tmp/unpack.log");
#endif
    if ( (errstrm = fopen(log_file_name,"a+")) != NULL) {
	return ;
    }
    char *tname = tempnam(tmpdir,"#upkg");	
    sprintf(log_file_name,tname);
    if ( (errstrm = fopen(log_file_name,"a+")) != NULL) {
	return ;
    }
#ifndef WIN32
    //On windows most likely it will fail.
    if ( (errstrm = fopen("/dev/null","a+")) != NULL) {
	return ;
    }
#endif
    //Last resort
    errstrm = stderr;  
   
}

#ifndef PRODUCT
void breakpoint() { }  // hook for debugger
void assert_failed(const char* p) {
  fprintf(errstrm, "failed assert(%s)\n", p);
  breakpoint();
}
#endif
