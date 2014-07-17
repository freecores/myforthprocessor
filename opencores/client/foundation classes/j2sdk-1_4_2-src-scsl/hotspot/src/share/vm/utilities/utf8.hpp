#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)utf8.hpp	1.13 03/01/23 12:29:11 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// Low-level interface for UTF8 strings

class UTF8 : AllStatic {
 public:
  // returns the unicode length of a 0-terminated uft8 string
  static int unicode_length(const char* uft8_str);

  // returns the unicode length of a non-0-terminated uft8 string
  static int unicode_length(const char* uft8_str, int len);

  // converts a uft8 string to a unicode string
  static void convert_to_unicode(const char* utf8_str, jchar* unicode_buffer, int unicode_length); 

  // decodes the current utf8 character, stores the result in value,
  // and returns the end of the current uft8 chararacter.
  static char* next(const char* str, jchar* value);

  // Utility methods
  static jbyte* strrchr(jbyte* base, int length, jbyte c);
  static bool   equal(jbyte* base1, int length1, jbyte* base2, int length2);
};


// Low-level interface for UNICODE strings

class UNICODE : AllStatic {
 public:
  // returns the utf8 size of a unicode character
  static int utf8_size(jchar c);

  // returns the utf8 length of a unicode string
  static int utf8_length(jchar* base, int length);

  // converts a unicode string to utf8 string
  static void convert_to_utf8(const jchar* base, int length, char* utf8_buffer);

  // converts a unicode string to a utf8 string and
  // returns the result as a resource allocated string
  static char* as_utf8(jchar* base, int length);  
};
