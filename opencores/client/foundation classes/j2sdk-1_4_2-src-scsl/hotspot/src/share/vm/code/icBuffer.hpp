#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)icBuffer.hpp	1.22 03/01/23 11:59:51 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

//
// For CompiledIC's:
//
// In cases where we do not have MT-safe state transformation,
// we go to a transition state, using ICStubs. At a safepoint,
// the inline caches are transferred from the transitional code:
//
//    instruction_address --> 01 set xxx_oop, Ginline_cache_klass
//                            23 jump_to Gtemp, yyyy
//                            4  nop

class ICStub;

class InlineCacheBuffer: public AllStatic {
 private:
  // friends  
  friend class ICStub;

  static const int ic_stub_code_size;

  static StubQueue* _buffer;
  static ICStub*    _next_stub;

  static StubQueue* buffer()                         { return _buffer;         }
  static void       set_next_stub(ICStub* next_stub) { _next_stub = next_stub; }
  static ICStub*    get_next_stub()                  { return _next_stub;      }

  static void       init_next_stub();

  static ICStub* new_ic_stub();
 

  // Machine-dependent implementation of ICBuffer
  static void    assemble_ic_buffer_code(address code_begin, oop cached_oop, address entry_point);
  static address ic_buffer_entry_point  (address code_begin); 
  static oop     ic_buffer_cached_oop   (address code_begin);

 public:
 
    // Initialization; must be called before first usage
  static void initialize();

  // Access
  static bool contains(address instruction_address);

    // removes the ICStubs after backpatching
  static void update_inline_caches();

  // for debugging
  static bool is_empty();


  // New interface
  static void    create_transition_stub(CompiledIC *ic, oop cached_oop, address entry);
  static address ic_destination_for(CompiledIC *ic);
  static oop     cached_oop_for(CompiledIC *ic);
};

