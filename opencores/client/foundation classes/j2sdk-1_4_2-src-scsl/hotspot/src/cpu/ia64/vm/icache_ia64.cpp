#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)icache_ia64.cpp	1.9 03/01/23 10:57:29 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_icache_ia64.cpp.incl"

extern "C" {
    typedef int (*_flush_icache_stub_t)(address addr, int lines, int magic);
}

static _flush_icache_stub_t flush_icache_stub = NULL; 

static address stubCode_entry; // The flush stub function descriptor address

#define __ _masm->

static const uint line_size      = 32;
static const uint log2_line_size = 5;

class ICacheStubGenerator: public StubCodeGenerator {
 public:
  ICacheStubGenerator(CodeBuffer *c) : StubCodeGenerator(c) {}

  // Issue the instruction to flush the icache.
  // There is an interesting issue--since we cannot flush the cache when this stub is
  // generated, it must be generated first, and just to be sure, we will do extra work
  // to allow a check that these instructions got executed.

  address generate_icache_flush() {
    StubCodeMark mark(this, "ICache", "flush_icache_stub");

    address entry = __ emit_fd();

    const Register addr  = GR_I0;
    const Register lines = GR_I1;
    const Register magic = GR_I2;

    const Register save_LC  = GR35;
    const Register save_PFS = GR36;

    const PredicateRegister flush    = PR15_SCRATCH;
    const PredicateRegister no_flush = PR14_SCRATCH;

    __ alloc(save_PFS, 3, 2, 0, 0);
    __ mov(save_LC, AR_LC);
    __ cmp4(no_flush, flush, 0, lines, Assembler::equal);

    __ mov(no_flush, AR_PFS, save_PFS);
    __ sub(lines, lines, 1);

    __ mov(flush, AR_LC, lines);
    __ mov(no_flush, GR_RET, magic);
    __ ret(no_flush);

    Label flush_line;
    __ bind(flush_line);

    __ fc(addr);

    __ add(addr, addr, line_size);
    __ cloop(flush_line, Assembler::sptk, Assembler::few);

    __ synci();

    __ srlzi();

    __ mov(AR_LC, save_LC);
    __ mov(AR_PFS, save_PFS);
    __ mov(GR_RET, magic);     // Handshake with caller to make sure it happened!
    __ ret();

    stubCode_entry = entry;
    return entry;
  };
};


void ICache::initialize() {
  // Making this stub must be FIRST use of assembler 
  ResourceMark rm;

  const int stub_size = 200; // Big enough...

  BufferBlob* b = BufferBlob::create(stub_size, "flush_icache_stub");
  CodeBuffer* c = new CodeBuffer(b->instructions_begin(), b->instructions_size());

  ICacheStubGenerator g(c);
  flush_icache_stub = CAST_TO_FN_PTR(_flush_icache_stub_t, g.generate_icache_flush());

  // The first use of flush_icache_stub must apply it to itself.
  ICache::invalidate_range(CAST_FROM_FN_PTR(address, flush_icache_stub), c->code_size());
}


// The business with the magic number is just a little security.
// We cannot call the flush stub when generating the flush stub
// because it isn't there yet.  So, the stub also copies GR_I2 to GR_RET.
// This is a cheap check that the stub was really executed. 
static void call_flush_stub(u_char* start, int lines) {
  static int magic = 0xbaadbabe;

  int auto_magic = magic;      // Make a local copy to avoid race condition
  int r = (*flush_icache_stub)(start, lines, auto_magic);
  guarantee(r == auto_magic, "flush stub routine did not execute");
  ++magic;
}


void ICache::invalidate_word(u_char* addr) {
  // Because this is called for instruction patching on the fly,
  // long after bootstrapping, we execute the stub directly.
  // Assume the word is entirely contained in one cache line.
  (*flush_icache_stub)(addr, 1, 0);
}


void ICache::invalidate_range(u_char* start, int nBytes) {
  static bool firstTime = true;
  if (firstTime) {
    guarantee(start == stubCode_entry, "first flush should be for flush stub");
    firstTime = false;
    return;
  }

  if (nBytes == 0) {
    return;
  }
  const uint line_offset = mask_address_bits(start, line_size-1);
  if (line_offset != 0) {
    start -= line_offset;
    nBytes += line_offset;
  }
  call_flush_stub(start, round_to(nBytes, line_size) >> log2_line_size);
}
