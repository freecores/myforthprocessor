#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)psYoungGen.hpp	1.21 03/01/23 12:03:08 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

class PSMarkSweepDecorator;

class PSYoungGen : public CHeapObj {
 protected:
  MemRegion      _reserved;
  VirtualSpace   _virtual_space;

  // Spaces
  MutableSpace* _eden_space;
  MutableSpace* _from_space;
  MutableSpace* _to_space;

  // MarkSweep Decorators
  PSMarkSweepDecorator* _eden_mark_sweep;
  PSMarkSweepDecorator* _from_mark_sweep;
  PSMarkSweepDecorator* _to_mark_sweep;

  // AdaptiveSizePolicy support
  //
  // After a minor collection, we update the eden and survivor size
  // in two steps, starting when the heap is configured Eden, from, to,
  // and finishing at the next GC when the heap is Eden, to, from.
  // We cache the survivor size used to start the resize process to
  // ensure we're using the same value the second time.
  bool   _last_survivor_size_valid;
  size_t _last_survivor_size;

  // Sizing information, in bytes, set in constructor
  const size_t _init_gen_size;
  const size_t _min_gen_size;
  const size_t _max_gen_size;

  // Performance counters
  GenerationCounters*       _gen_counters;
  SpaceCounters*            _eden_counters;
  SpaceCounters*            _from_counters;
  SpaceCounters*            _to_counters;

  // Initialize the space boundaries 
  void compute_initial_space_boundaries();

  // Space boundary helper
  void set_space_boundaries(size_t eden_size, size_t survivor_size);

  // Space boundary invariant checker
  void space_invariants() PRODUCT_RETURN;

  // Resize this generation to the desired size, but don't allow shrinking
  // below shrink_boundary.
  // Return true if the generation size changed.
  bool resize_helper(size_t desired_size, char* shrink_boundary);

  // Resize the generation when there is live data in it. This
  // requires that we use much care to ensure the live data isn't
  // disrupted.
  void resize_with_live(size_t eden_size, size_t survivor_size);

  HeapWord* expand_and_allocate(size_t word_size,
                                bool   is_large_noref,
                                bool   is_tlab);

  // Accessors for _last_survivor_size, 
  // to make sure each setting is only used once.
  void unset_last_survivor_size();
  void set_last_survivor_size(size_t value);
  size_t get_and_unset_last_survivor_size();
  bool last_survivor_size_valid() const;

 public:
  // Initialize the generation.
  PSYoungGen(ReservedSpace rs,
             size_t        initial_byte_size, 
             size_t        minimum_byte_size,
             size_t        maximum_byte_size);

  MemRegion reserved()              { return _reserved; }

  bool is_in(const void* p) const   {
      return _virtual_space.contains((void *)p);
  }

  MutableSpace* eden_space() const  { return _eden_space; }
  MutableSpace* from_space() const  { return _from_space; }
  MutableSpace* to_space() const    { return _to_space; }

  // MarkSweep support
  PSMarkSweepDecorator* eden_mark_sweep() const    { return _eden_mark_sweep; }
  PSMarkSweepDecorator* from_mark_sweep() const    { return _from_mark_sweep; }
  PSMarkSweepDecorator* to_mark_sweep() const      { return _to_mark_sweep;   }

  void precompact();
  void adjust_pointers();
  void compact();

  // Called during/after gc
  void swap_spaces();

  // Resize generation using suggested free space size and survivor size
  // NOTE:  "eden_size" and "survivor_size" are suggestions only. Current
  //        heap layout (particularly, live objects in from space) might
  //        not allow us to use these values.
  void resize(size_t eden_size, size_t survivor_size);

  // Size info
  size_t capacity_in_bytes() const;
  size_t used_in_bytes() const;
  size_t free_in_bytes() const;

  size_t capacity_in_words() const;
  size_t used_in_words() const;
  size_t free_in_words() const;

  // The max this generation can grow to
  size_t max_size() const       { return _max_gen_size;   }

  // Allocation
  HeapWord* allocate(size_t word_size, bool is_large_noref, bool is_tlab) {
    HeapWord* result = eden_space()->cas_allocate(word_size);
    return result;
  }

  HeapWord** top_addr() const   { return eden_space()->top_addr(); }
  HeapWord** end_addr() const   { return eden_space()->end_addr(); }

  // Iteration.
  void oop_iterate(OopClosure* cl);
  void object_iterate(ObjectClosure* cl);

  // Performance Counter support
  void update_counters();

  // Debugging - do not use for time critical operations
  void print() const;
  void print_on(outputStream* st) const;
 
  void verify(bool allow_dirty) PRODUCT_RETURN;
};

