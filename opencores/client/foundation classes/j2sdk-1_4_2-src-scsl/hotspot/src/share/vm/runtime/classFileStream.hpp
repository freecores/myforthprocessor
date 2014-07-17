#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)classFileStream.hpp	1.23 03/01/23 12:21:36 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// Input stream for reading .class file
//
// The entire input stream is present in a buffer allocated by the caller.
// The caller is responsible for deallocating the buffer and for using
// ResourceMarks appropriately when constructing streams.

class ClassFileStream: public ResourceObj {
 private:
  u1*   _buffer_start; // Buffer bottom
  u1*   _buffer_end;   // Buffer top (one past last element)
  u1*   _current;      // Current buffer position
  char* _source;       // Source of stream (directory name, ZIP/JAR archive name)

 public:
  // Constructor
  ClassFileStream(u1* buffer, int length, char* source);

  // Buffer access
  u1* buffer() const           { return _buffer_start; }
  int length() const           { return _buffer_end - _buffer_start; }
  char* source() const         { return _source; }
  
  // Read u1 from stream
  u1 get_u1() {
    assert(_current + 1 <= _buffer_end, "buffer overflow");
    return *_current++;
  }

  // Read u2 from stream
  u2 get_u2() {
    assert(_current + 2 <= _buffer_end, "buffer overflow");
    u1* tmp = _current;
    _current += 2;
    return Bytes::get_Java_u2(tmp);
  }

  // Read u4 from stream
  u4 get_u4() {
    assert(_current + 4 <= _buffer_end, "buffer overflow");
    u1* tmp = _current;
    _current += 4;
    return Bytes::get_Java_u4(tmp);
  }

  // Read u8 from stream
  u8 get_u8() {
    assert(_current + 8 <= _buffer_end, "buffer overflow");
    u1* tmp = _current;
    _current += 8;
    return Bytes::get_Java_u8(tmp);
  }

  // Get direct pointer into stream at current position. 
  // Returns NULL if length elements are not remaining. The caller is 
  // responsible for calling skip below if buffer contents is used.
  u1* get_u1_buffer(int length) {
    return _current;
  }

  u2* get_u2_buffer(int length) {
    return (u2*) _current;
  }

  // Skip length u1 or u2 elements from stream
  void skip_u1(int length) {
    _current += length;
  }

  void skip_u2(int length) {
    _current += length * 2;
  }

  // Tells whether eos is reached
  bool at_eos() const          { return _current == _buffer_end; }
};
