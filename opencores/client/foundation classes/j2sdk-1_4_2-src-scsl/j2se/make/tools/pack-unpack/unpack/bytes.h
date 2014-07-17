/*
 * @(#)bytes.h	1.4 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifdef WIN32_LEAN_AND_MEAN
typedef signed char byte ;
#endif

struct bytes {
  byte* ptr;
  int len;
  byte* limit() { return ptr+len; }

  void set(byte* ptr_, int len_) { ptr = ptr_; len = len_; }
  void set(const char* str)      { ptr = (byte*)str; len = strlen(str); }
  bool inBounds(const void* p);  // p in [ptr, limit)
  void malloc(int len_);
  void realloc(int len_);
  void grow(double factor) { realloc(int(factor * len)); }
  void free();
  void copyFrom(const void* ptr_, int len_, int offset = 0);
  void saveFrom(const void* ptr_, int len_);
  void copyFrom(bytes& other, int offset = 0) {
    copyFrom(other.ptr, other.len, offset);
  }
  void saveFrom(bytes& other) {
    saveFrom(other.ptr, other.len);
  }
  void clear() { memset(ptr, 0, len); }
  byte* writeTo(byte* bp);
  bool equals(bytes& other) { return 0 == compareTo(other); }
  int compareTo(bytes& other);
  bool contains(byte c) { return indexOf(c) >= 0; }
  int indexOf(byte c);
  int lastIndexOf(byte c);
  int lastIndexOf(byte c, int lastPos) { return slice(0, lastPos+1).lastIndexOf(c); }
  // substrings:
  bytes slice(int beg, int end) {
    bytes res;
    res.ptr = ptr + beg;
    res.len = end - beg;
    assert(res.len == 0 || inBounds(res.ptr) && inBounds(res.limit()-1));
    return res;
  }
  // building C strings inside byte buffers:
  bytes& strcat(const char* str) { ::strcat((char*)ptr, str); return *this; }
  bytes& strcat(bytes& other) { ::strncat((char*)ptr, (char*)other.ptr, other.len); return *this; }
  char* strval() { assert(strlen((char*)ptr) == len); return (char*) ptr; }
#ifdef PRODUCT
  const char* string() { return 0; }
#else
  const char* string();
#endif
};

struct ptrlist {
  bytes b;
  int len;

  void** base()     { return (void**)b.ptr; }
  void*  get(int i) { return base()[i]; }
  void** limit()    { return base() + len; }
  void   init()     { b.set(null, 0); len = 0; }
  void   clear()    { len = 0; }
  void add(void* p);
  bool contains(void* p);
  void free();
};
