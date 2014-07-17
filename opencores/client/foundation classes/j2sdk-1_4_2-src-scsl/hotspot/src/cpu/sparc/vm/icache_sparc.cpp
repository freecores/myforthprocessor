#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)icache_sparc.cpp	1.19 03/01/23 11:01:14 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_icache_sparc.cpp.incl"

static jlong stubCode[10]; // want doubleword alignment
extern "C" {
    typedef int (*_flush_icache_stub_t)(address addr, int dwords, int magic);
}
static _flush_icache_stub_t flush_icache_stub = NULL; 

static address stubCode_start; // the actual start of the flush stub code

class ICacheStubGenerator: public StubCodeGenerator {
 public:

  ICacheStubGenerator(CodeBuffer *c) : StubCodeGenerator(c) {}

  // issue the intruction to flush the icache
  // There is an interesting issue--since we cannot flush the cache when this stub is
  // generated, it must be generated first, and just to be sure, we will do extra work
  // to allow a check that these instructions got executed.
  // Pass address to flush in o0, value to return in o1

  address generate_icache_flush() {
    assert( (intptr_t)stubCode % ((sizeof (jint)) * 2) == 0,  "must be doubleword aligned");
    StubCodeMark mark(this, "ICache", "flush_icache_stub");
#   define __ _masm->
    address start = __ pc();

    Label L;
    __ bind(L);
    __ flush( O0, G0 );
    __ deccc( O1 );
    __ br(Assembler::positive, false, Assembler::pn, L);
    __ delayed()->inc( O0, 8 );
    __ retl();
    __ delayed()->mov( O2, O0 ); // handshake with caller to make sure it happened!
#   undef __
  
    stubCode_start = start;
    return start;
  };
};


void ICache::initialize() {
  ResourceMark rm;
  // Making this stub must be FIRST use of assembler 
  CodeBuffer* c = new CodeBuffer(address(stubCode), sizeof(stubCode));
  ICacheStubGenerator g(c);
  flush_icache_stub = CAST_TO_FN_PTR(_flush_icache_stub_t, g.generate_icache_flush());
  // The first use of flush_icache_stub must apply it to itself:
  ICache::invalidate_range(CAST_FROM_FN_PTR(address, flush_icache_stub), c->code_size());
}

// The business with the magic number is just a little security.
// We cannot call the flush stub when generating the flush stub
// because it isn't there yet. So, the stub also copies o1 to o0.
// This is a cheap check that the stub was really executed. 
static void call_flush_stub(address addr, int dwords) {
  static int magic = 0xbaadbabe;

  int    auto_magic = magic;	// make a local copy to avoid race condition
  int r = (*flush_icache_stub)(addr, dwords, auto_magic);
  guarantee( r == auto_magic, "flush stub routine did not execute");
  ++magic;
}


void ICache::invalidate_word(u_char* addr) {
  // Because this is called for instruction patching on the fly,
  // long after bootstrapping, we execute the stub directly.
  (*flush_icache_stub)(addr, 1, 0);
}

void ICache::invalidate_range(u_char* addr, int size) { 
  static bool firstTime = true;

  if (firstTime) {
    guarantee( addr == stubCode_start,
               "first flush should be for flush stub");
    firstTime = false;
    return;
  }
  u_char* a = addr;
  int     s = size;
  // flush instruction does doublewords
  if ((intptr_t)a & BytesPerInt) {
    a -= BytesPerInt;
    s += BytesPerInt;
  }
  call_flush_stub(a, (s + BytesPerInt*2-1) / (BytesPerInt*2));
}



