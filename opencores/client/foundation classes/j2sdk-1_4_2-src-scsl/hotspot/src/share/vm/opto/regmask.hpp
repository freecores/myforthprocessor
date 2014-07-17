#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)regmask.hpp	1.54 03/01/23 12:18:57 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// Some fun naming (textual) substitutions:
//
// RegMask::get_low_elem() ==> RegMask::find_first_elem()
// RegMask::Special        ==> RegMask::Empty
// RegMask::_flags         ==> RegMask::is_AllStack()
// RegMask::operator<<=()  ==> RegMask::Insert()
// RegMask::operator>>=()  ==> RegMask::Remove()
// RegMask::Union()        ==> RegMask::OR
// RegMask::Inter()        ==> RegMask::AND
//
// OptoRegister::RegName   ==> OptoReg::Name
//
// SharedInfo::stack0      ==> _last_Mach_Reg  or ZERO in core version
//
// numregs in chaitin      ==> proper degree in chaitin


//------------------------------RegMask----------------------------------------
// The ADL file describes how to print the machine-specific registers, as well
// as any notion of register classes.  We provide a register mask, which is
// just a collection of Register numbers.  

// The ADLC defines 2 macros, RM_SIZE and FORALL_BODY.
// RM_SIZE is the size of a register mask in words.
// FORALL_BODY replicates a BODY macro once per word in the register mask.
// The usage is somewhat clumsy and limited to the regmask.[h,c]pp files.
// However, it means the ADLC can redefine the unroll macro and all loops
// over register masks will be unrolled by the correct amount.

class RegMask VALUE_OBJ_CLASS_SPEC {
  union {
    double _dummy_force_double_alignment[RM_SIZE>>1];
    // Array of Register Mask bits.  This array is large enough to cover 
    // all the machine registers and all parameters that need to be passed
    // on the stack (stack registers) up to some interesting limit.  Methods
    // that need more parameters will NOT be compiled.  On Intel, the limit
    // is something like 90+ parameters.
    int _A[RM_SIZE];
  };

  enum {
    _WordBits    = BitsPerInt,
    _LogWordBits = LogBitsPerInt,
    _RM_SIZE     = RM_SIZE   // local constant, imported, then hidden by #undef
  };

public:
  enum { CHUNK_SIZE = RM_SIZE*_WordBits };

  // SlotsPerLong is 2, since slots are 32 bits and longs are 64 bits.
  // Also, consider the maximum alignment size for a normally allocated
  // value.  Since we allocate register pairs but not register quads (at
  // present), this alignment is SlotsPerLong (== 2).  A normally
  // aligned allocated register is either a single register, or a pair
  // of adjacent registers, the lower-numbered being even.
  // See also is_aligned_Pairs() below, and the padding added before
  // Matcher::_new_SP to keep allocated pairs aligned properly.
  // If we ever go to quad-word allocations, SlotsPerQuad will become
  // the controlling alignment constraint.  Note that this alignment
  // requirement is internal to the allocator, and independent of any
  // particular platform.
  enum { SlotsPerLong = 2 };

  // A constructor only used by the ADLC output.  All mask fields are filled
  // in directly.  Calls to this look something like RM(1,2,3,4);
  RegMask( 
#   define BODY(I) int a##I,
    FORALL_BODY   
#   undef BODY
    int dummy = 0 ) {
#   define BODY(I) _A[I] = a##I;
    FORALL_BODY
#   undef BODY
  }

  // Handy copying constructor
  RegMask( RegMask *rm ) {
#   define BODY(I) _A[I] = rm->_A[I];
    FORALL_BODY
#   undef BODY
  }

  // Construct an empty mask
  RegMask( ) { Clear(); }

  // Construct a mask with a single bit
  RegMask( OptoReg::Name reg ) { Clear(); Insert(reg); }

  // Check for register being in mask
  int Member( OptoReg::Name reg ) const {
    assert( reg < CHUNK_SIZE, "" );
    return _A[reg>>_LogWordBits] & (1<<(reg&(_WordBits-1)));
  }

  // The last bit in the register mask indicates that the mask should repeat
  // indefinitely with ONE bits.  Returns TRUE if mask is infinite or 
  // unbounded in size.  Returns FALSE if mask is finite size.
  int is_AllStack() const { return _A[RM_SIZE-1] >> (_WordBits-1); }

  // Work around an -xO3 optimization problme in WS6U1. The old way:
  //   void set_AllStack() { _A[RM_SIZE-1] |= (1<<(_WordBits-1)); }
  // will cause _A[RM_SIZE-1] to be clobbered, not updated when set_AllStack()
  // follows an Insert() loop, like the one found in init_spill_mask(). Using
  // Insert() instead works because the index into _A in computed instead of
  // constant.  See bug 4665841.
  void set_AllStack() { Insert(OptoReg::Name(CHUNK_SIZE-1)); }

  // Test for being a not-empty mask.
  int is_NotEmpty( ) const {
    int tmp = 0;
#   define BODY(I) tmp |= _A[I];
    FORALL_BODY
#   undef BODY
    return tmp;
  }

  // Find lowest-numbered register from mask, or BAD if mask is empty.
  OptoReg::Name find_first_elem() const { 
    int base, bits;
#   define BODY(I) if( (bits = _A[I]) != 0 ) base = I<<_LogWordBits; else
    FORALL_BODY
#   undef BODY
      { base = OptoReg::Bad; bits = 1<<0; }
    return OptoReg::Name(base + ff1(bits)); 
  }
  // Get highest-numbered register from mask, or BAD if mask is empty.
  OptoReg::Name find_last_elem() const { 
    int base, bits;
#   define BODY(I) if( (bits = _A[RM_SIZE-1-I]) != 0 ) base = (RM_SIZE-1-I)<<_LogWordBits; else
    FORALL_BODY
#   undef BODY
      { base = OptoReg::Bad; bits = 1<<0; }
    return OptoReg::Name(base + fh1(bits)); 
  }

  // Find the lowest-numbered register pair in the mask.  Return the
  // HIGHEST register number in the pair, or BAD if no pairs.
  // Assert that the mask contains only bit pairs.
  OptoReg::Name find_first_pair() const;

  // Clear out partial bits; leave only aligned adjacent bit pairs.
  void ClearToPairs();
  // Smear out partial bits; leave only aligned adjacent bit pairs.
  void SmearToPairs();
  // Verify that the mask contains only aligned adjacent bit pairs
  void VerifyPairs() const { assert( is_aligned_Pairs(), "mask is not aligned, adjacent pairs" ); }
  // Test that the mask contains only aligned adjacent bit pairs
  bool is_aligned_Pairs() const;

  // Test for single register
  int is_bound1() const;
  // Test for a single adjacent pair
  int is_bound2() const;

  // Fast overlap test.  Non-zero if any registers in common.
  int overlap( const RegMask &rm ) const {
    return 
#   define BODY(I) (_A[I] & rm._A[I]) |
    FORALL_BODY
#   undef BODY
    0 ;
  }

  // Special test for register pressure based splitting
  // UP means register only, Register plus stack, or stack only is DOWN
  bool is_UP() const;

  // Clear a register mask
  void Clear( ) {
#   define BODY(I) _A[I] = 0;
    FORALL_BODY
#   undef BODY
  }

  // Fill a register mask with 1's
  void Set_All( ) {
#   define BODY(I) _A[I] = -1;
    FORALL_BODY
#   undef BODY
  }

  // Insert register into mask
  void Insert( OptoReg::Name reg ) {
    assert( reg < CHUNK_SIZE, "" );
    _A[reg>>_LogWordBits] |= (1<<(reg&(_WordBits-1)));
  }

  // Remove register from mask
  void Remove( OptoReg::Name reg ) {
    assert( reg < CHUNK_SIZE, "" );
    _A[reg>>_LogWordBits] &= ~(1<<(reg&(_WordBits-1)));
  }

  // OR 'rm' into 'this'
  void OR( const RegMask &rm ) {
#   define BODY(I) this->_A[I] |= rm._A[I];
    FORALL_BODY
#   undef BODY
  }

  // AND 'rm' into 'this'
  void AND( const RegMask &rm ) {
#   define BODY(I) this->_A[I] &= rm._A[I];
    FORALL_BODY
#   undef BODY
  }

  // Subtract 'rm' from 'this'
  void SUBTRACT( const RegMask &rm ) {
#   define BODY(I) _A[I] &= ~rm._A[I];
    FORALL_BODY
#   undef BODY
  }

  // Compute size of register mask: number of bits
  uint Size() const;

#ifndef PRODUCT
  void print() const { dump(); }
  void dump() const;            // Print a mask
#endif

  static const RegMask Empty;   // Common empty mask

  static bool can_represent(OptoReg::Name reg) {
    // NOTE: -1 in computation reflects the usage of the last
    //       bit of the regmask as an infinite stack flag.
    return (int)reg < (int)(CHUNK_SIZE-1);
  }
};

// Do not use this constant directly in client code!
#undef RM_SIZE

/*
    // Difference
  void operator-= (const RegMask &rm) { 
    // Exclude on a best-efforts basis, but do not bother
    // to drop bits if rm is in a different chunk.
    // Any caller who needs more precision is responsible
    // for matching this->_chunk to rm._chunk explicitly.
    if( _chunk == rm._chunk )
      _regs &= ~rm._regs;
    _flags &= ~rm._flags;
  }

  // Get highest-numbered register from mask
  OptoReg::Name get_high_elem() const { 
    assert(_regs, "Empty mask"); 
    assert(!_flags, "Infinite mask"); 
    return OptoReg::Name((_chunk<<_LogWordBits) + fh1(_regs)); 
  }
  // Test for an even/odd pair
  int is_evenodd(const RegMask &rm) const;
  // Test whether it contains any non-stack regs.
  int has_nonstack() const {
    return (int)_chunk < (SharedInfo::stack0>>_LogWordBits) && _regs;
  }
  // Test whether it contains any stack slots.
  int has_stack() const {
    return _flags || (int)_chunk >= (SharedInfo::stack0>>_LogWordBits) && _regs;
  }
  // Return single bound register or Bad
  OptoReg::Name get_elem() const { 
    return is_bound1() ? get_low_elem() : OptoReg::Bad; }

  // Fast overlap test.  Non-zero if any registers in common.
  // Does not count _flags in the overlap computation.
  uint32 overlap( const RegMask &rm ) const {
    if( _flags && (int)_chunk >= (SharedInfo::stack0>>_LogWordBits) )
      return 0;
    if( _chunk == rm._chunk ) {
      int x = _regs & rm._regs;
      return x;
    }
    return 0;
  }
  // Fast overlap test.  Non-zero if any registers in common.
  // Does count _flags in the overlap computation.
  uint32 overlap_stack( const RegMask &rm ) const;

  // Fast subsumption test.  Non-zero if rm is a subset of this mask.
  uint32 contains_all( const RegMask &rm ) const {
    if( (rm._flags & ~_flags ) != 0 )
      return 0;
    else if( _chunk == rm._chunk || rm._regs == 0 )
      return (rm._regs & ~_regs) == 0;
    else if( _chunk < rm._chunk )
      return _flags && (int)(rm._chunk) >= (SharedInfo::stack0>>_LogWordBits);
    else
      return 0;
  }

  // Convert the RegMask rm to a infinite VectorSet and union the
  // bits into the VectorSet.
  friend VectorSet &operator <<= (VectorSet &set, const RegMask &rm );

  // Convenience built-in masks
  static const RegMask Empty;	// Empty register mask
  static const RegMask Special;	// The unallocated register
  static const RegMask Bad;	// The not-a-register mask
  static const RegMask Physical;// All physical registers


  void dump() const;	// Print a mask
#endif
  */


