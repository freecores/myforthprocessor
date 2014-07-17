#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)psOldGen.hpp	1.14 03/01/23 12:02:32 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

class PSMarkSweepDecorator;

class PSOldGen : public CHeapObj {
  friend class PSPromotionManager; // Uses the cas_allocate methods

 protected:
  MemRegion                _reserved;          // Used for simple containment tests
  VirtualSpace             _virtual_space;     // Controls mapping and unmapping of virtual mem
  ObjectStartArray         _start_array;       // Keeps track of where objects start in a 512b block
  MutableSpace*            _object_space;      // Where all the objects live
  PSMarkSweepDecorator*    _object_mark_sweep; // The mark sweep view of _object_space

  // Performance Counters
  GenerationCounters*      _gen_counters;
  SpaceCounters*           _space_counters;
  
  // Sizing information, in bytes, set in constructor
  const size_t _init_gen_size;
  const size_t _min_gen_size;
  const size_t _max_gen_size;

  HeapWord* allocate_noexpand(size_t word_size, bool is_large_noref, bool is_tlab) {
    // We assume the heap lock is held here.
    assert(!is_tlab, "Does not support TLAB allocation");
    assert_locked_or_safepoint(Heap_lock);
    HeapWord* res = object_space()->allocate(word_size);
    if (res != NULL) {
      _start_array.allocate_block(res);
    }
    return res;
  }

  // Support for MT garbage collection. CAS allocation is lower overhead than grabbing
  // and releasing the heap lock, which is held during gc's anyway. This method is not
  // safe for use at the same time as allocate_noexpand()!
  HeapWord* cas_allocate_noexpand(size_t word_size) {
    assert(SafepointSynchronize::is_at_safepoint(), "Must only be called at safepoint")
    HeapWord* res = object_space()->cas_allocate(word_size);
    if (res != NULL) {
      _start_array.allocate_block(res);
    }
    return res;
  }

  // Support for MT garbage collection. See above comment.
  HeapWord* cas_allocate(size_t word_size) {
    HeapWord* res = cas_allocate_noexpand(word_size);
    return (res == NULL) ? expand_and_cas_allocate(word_size) : res;
  }

  HeapWord* expand_and_allocate(size_t word_size, bool is_large_noref, bool is_tlab);
  HeapWord* expand_and_cas_allocate(size_t word_size);
  void expand(size_t bytes);
  bool expand_by(size_t bytes);
  void expand_to_reserved();

  void shrink(size_t bytes);

  void post_resize();

 public:
  // Initialize the generation.
  PSOldGen(ReservedSpace rs, size_t initial_byte_size,
           size_t minimum_byte_size, size_t maximum_byte_size,
           const char* gen_name, int level);

  MemRegion reserved()                      { return _reserved; }

  bool is_in(const void* p) const           { return _virtual_space.contains((void *)p); }

  MutableSpace* object_space() const        { return _object_space; }
  PSMarkSweepDecorator* object_mark_sweep() { return _object_mark_sweep; }
  ObjectStartArray* start_array()           { return &_start_array; }

  // MarkSweep methods
  virtual void precompact();
  void adjust_pointers();
  void compact();

  // Size info
  size_t capacity_in_bytes() const        { return object_space()->capacity_in_bytes(); }
  size_t used_in_bytes() const            { return object_space()->used_in_bytes(); }
  size_t free_in_bytes() const            { return object_space()->free_in_bytes(); }

  size_t capacity_in_words() const        { return object_space()->capacity_in_words(); }
  size_t used_in_words() const            { return object_space()->used_in_words(); }
  size_t free_in_words() const            { return object_space()->free_in_words(); }

  // Includes uncommitted memory
  size_t contiguous_available() const;

  // Calculating new sizes
  void resize(size_t desired_free_space);

  // Allocation. We report all successful allocations to the size policy
  // Note that the perm gen does not use this method, and should not!
  HeapWord* allocate(size_t word_size, bool is_large_noref, bool is_tlab);

  // Iteration.
  void oop_iterate(OopClosure* cl) { object_space()->oop_iterate(cl); }
  void object_iterate(ObjectClosure* cl) { object_space()->object_iterate(cl); }

  // Debugging - do not use for time critical operations
  virtual void print() const;
  virtual void print_on(outputStream* st) const;
  
  void verify(bool allow_dirty) PRODUCT_RETURN;
  void verify_object_start_array() PRODUCT_RETURN;

  // Performace Counter support
  void update_counters();

  // Printing support
  virtual const char* name() const { return "PSOldGen"; }
};

