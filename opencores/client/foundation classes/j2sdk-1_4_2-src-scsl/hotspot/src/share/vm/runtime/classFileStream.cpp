#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)classFileStream.cpp	1.31 03/01/23 12:21:33 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_classFileStream.cpp.incl"

ClassFileStream::ClassFileStream(u1* buffer, int length, char* source) {
  _buffer_start = buffer;
  _buffer_end   = buffer + length;
  _current      = buffer;
  _source       = source;
}

