// 
// Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
// SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
// @(#)atomic_solaris_sparc.il	1.9 03/01/29 13:16:02
// 

// Get the raw thread ID from %g7
     .inline  _raw_thread_id,0
     .register  %g7,#scratch
     mov        %g7, %o0
     .end


  // Support for jint atomic::exchange(jint exchange_value, jint* dest).
  //
  // Arguments :
  //
  //      exchange_value: O0
  //      dest:           O1
  //
  // Results:
  //
  //     O0: the value previously stored in dest
  //

      .inline           _atomic_exchange_v9,2
      swap              [%o1],%o0
      .end


  // Support for jint atomic::compare_and_exchange(jint exchange_value, 
  //                                               jint* dest, 
  //                                               jint compare_value)
  //
  // Arguments :
  //
  //      exchange_value: O0
  //      dest:           O1
  //      compare_value:  O2
  //
  // Results:
  //
  //     O0: the value previously stored in dest
  //
      .inline        _atomic_cas_v9,3
      cas            [%o1], %o2, %o0
      .end


  // Support for jint atomic::compare_and_exchange_ptr(jint exchange_value, 
  //                                                   jint* dest, 
  //                                                   jint compare_value)
  //
  // Arguments :
  //
  //      exchange_value: O0
  //      dest:           O1
  //      compare_value:  O2
  //
  // Results:
  //
  //     O0: the value previously stored in dest
  //
      .inline        _atomic_casx_v9,3
      casx           [%o1], %o2, %o0
      .end


  // Support for jlong atomic::compare_and_exchange_long(jlong exchange_value, 
  //                                                     jlong *dest, 
  //                                                     jlong compare_value)
  //
  // Arguments :
  //
  //      exchange_value: O1:O0
  //      dest:           O2
  //      compare_value:  O4:O3
  //
  // Results:
  //
  //     O1:O0: the value previously stored in dest
  //
      .inline  _atomic_cas_long_v9,3
      sllx    %o0, 32, %o0
      srl     %o1, 0, %o1
      or      %o0,%o1,%o0
      sllx    %o3, 32, %o3
      srl     %o4, 0, %o4
      or      %o3,%o4,%o3
      casx    [%o2], %o3, %o0
      srl     %o0, 0, %o1
      srlx    %o0, 32, %o0
      .end

  // Support for jint atomic::add(jint inc, jint* dest).
  //
  // Arguments :
  //
  //      inc: O0   (e.g., +1 or -1)
  //      loc: O1
  //
  // Results:
  //
  //     O0: the new value stored in dest
  //
  // Overwrites (v9): O3

      .inline _atomic_add_v9,2
  1:
      ld   [%o1], %o2
      add  %o0, %o2, %o3
      cas  [%o1], %o2, %o3
      cmp  %o2, %o3
      bne  1b
       nop
      add  %o0, %o2, %o0
      .end

  // Support for jint atomic::add_ptr(jint inc, jint* dest) for 64 bit
  //
  // Arguments :
  //
  //      inc: O0   (e.g., +1 or -1)
  //      loc: O1
  //
  // Results:
  //
  //     O0: the new value stored in dest
  //
  // Overwrites (v9): O3

     .inline _atomic_add_ptr_v9,2
  2:

      ldx   [%o1], %o2
      add   %o0, %o2, %o3
      casx  [%o1], %o2, %o3
      cmp   %o2, %o3
      bne   %xcc, 2b
       nop
      add   %o0, %o2, %o0
      .end

  // Support for intptr_t atomic::exchange_ptr(intptr_t exchange_value, intptr_t * dest).
  //
  // Arguments :
  //
  //      exchange_value: O0
  //      dest:           O1
  //
  // Results:
  //
  //     O0: the value previously stored in dest
  //
      .inline         _atomic_exchange_ptr_v9,2
  3:
      mov   %o0, %o3
      ldx   [%o1], %o2
      casx  [%o1], %o2, %o3
      cmp   %o2, %o3
      bne   %xcc, 3b
       nop
      mov   %o2, %o0
      .end

   // Support for atomic::membar()
      .inline         _atomic_membar_v9,0
      membar          #StoreLoad
      .end

   // Support for atomic::prefetch_read(void *loc, intx interval)
      .inline         _atomic_prefetch_read,2
      prefetch        [%o0+%o1], 0
      .end


   // Support for atomic::prefetch_write(void *loc, intx interval)
      .inline         _atomic_prefetch_write,2
      prefetch        [%o0+%o1], 2
      .end
