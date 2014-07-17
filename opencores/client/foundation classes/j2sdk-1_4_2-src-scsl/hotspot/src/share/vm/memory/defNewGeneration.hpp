#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)defNewGeneration.hpp	1.14 03/01/23 12:07:52 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

class EdenSpace;
class ContiguousSpace;

// DefNewGeneration is a young generation containing eden, from- and
// to-space.

class DefNewGeneration: public Generation {
  friend class VMStructs;

protected:
  Generation* _next_gen;
  int         _tenuring_threshold;   // Tenuring threshold for next collection.
  ageTable    _age_table;
  // Size of object to pretenure in words; command line provides bytes
  size_t        _pretenure_size_threshold_words; 

  ageTable*   age_table() { return &_age_table; }

  // Decide if there's enough space to promote all of the DefNewGeneration
  bool   full_promotion_would_succeed();

  // Performance Counters
  GenerationCounters*  _gen_counters;
  CSpaceCounters*      _eden_counters;
  CSpaceCounters*      _from_counters;
  CSpaceCounters*      _to_counters;

 protected:
  // Spaces
  EdenSpace*       _eden_space;
  ContiguousSpace* _from_space;
  ContiguousSpace* _to_space;

  enum SomeProtectedConstants {
    // Generations are GenGrain-aligned and have size that are multiples of
    // GenGrain.
    MinFreeScratchWords = 100
  };

  // Return the size of a survivor space if this generation were of size
  // gen_size.
  size_t compute_survivor_size(size_t gen_size, size_t alignment) const {
    size_t n = gen_size / (SurvivorRatio + 2);
    return n > alignment ? align_size_down(n, alignment) : alignment;
  }

 public:  // was "protected" but caused compile error on win32
  class IsAliveClosure: public BoolObjectClosure {
    Generation* _g;
  public:
    IsAliveClosure(Generation* g);
    void do_object(oop p);
    bool do_object_b(oop p);
  };

  class KeepAliveClosure: public OopClosure {
  protected:
    ScanWeakRefClosure* _cl;
    CardTableRS* _rs;
  public:
    KeepAliveClosure(ScanWeakRefClosure* cl);
    void do_oop(oop* p);
  };

  class FastKeepAliveClosure: public KeepAliveClosure {
  protected:
    HeapWord* _boundary;
  public:
    FastKeepAliveClosure(DefNewGeneration* g, ScanWeakRefClosure* cl);
    void do_oop(oop* p);
  };

  class TrainPolicyKeepAliveClosure: public KeepAliveClosure {
    TrainGeneration* _tg;
  public:
    TrainPolicyKeepAliveClosure(TrainGeneration* tg, ScanWeakRefClosure* cl);
    void do_oop(oop* p);
  };


  class EvacuateFollowersClosure: public VoidClosure {
    GenCollectedHeap* _gch;
    int _level;
    ScanClosure* _scan_cur_or_nonheap;
    ScanClosure* _scan_older;
  public:
    EvacuateFollowersClosure(GenCollectedHeap* gch, int level,
			     ScanClosure* cur, ScanClosure* older);
    void do_void();
  };

  class FastEvacuateFollowersClosure: public VoidClosure {
    GenCollectedHeap* _gch;
    int _level;
    FastScanClosure* _scan_cur_or_nonheap;
    FastScanClosure* _scan_older;
  public:
    FastEvacuateFollowersClosure(GenCollectedHeap* gch, int level,
				 FastScanClosure* cur,
				 FastScanClosure* older);
    void do_void();
  };

 public:
  DefNewGeneration(ReservedSpace rs, size_t initial_byte_size, int level,
                   const char* policy="Copy");

  virtual Generation::Name kind() { return Generation::DefNew; }

  // Accessing spaces
  EdenSpace*       eden() const           { return _eden_space; }
  ContiguousSpace* from() const           { return _from_space;  }
  ContiguousSpace* to()   const           { return _to_space;    }

  inline CompactibleSpace* first_compaction_space() const;

  // Space enquiries
  size_t capacity() const;
  size_t used() const;
  size_t free() const;
  size_t max_capacity() const;
  size_t capacity_before_gc() const;
  size_t unsafe_max_alloc_nogc() const;
  size_t contiguous_available() const;

  bool supports_inline_contig_alloc() const { return true; }
  HeapWord** top_addr() const;
  HeapWord** end_addr() const;

  // Thread-local allocation buffers
  bool supports_tlab_allocation() const { return true; }
  inline size_t tlab_capacity() const;

  // Iteration
  void object_iterate(ObjectClosure* blk);
  void object_iterate_since_last_GC(ObjectClosure* cl);

  void younger_refs_iterate(OopsInGenClosure* cl);

  void space_iterate(SpaceClosure* blk, bool usedOnly = false);

  virtual bool should_allocate(size_t word_size,
                               bool   is_large_noref,
                               bool   is_tlab) {
    assert(UseTLAB || !is_tlab, "Should not allocate tlab");

    const bool non_zero      = word_size > 0;
    const bool check_too_big = _pretenure_size_threshold_words > 0;
    const bool not_too_big   = word_size < _pretenure_size_threshold_words;
    const bool size_ok       = is_tlab || !check_too_big || not_too_big;
    const bool not_jvmpi     = !Universe::jvmpi_slow_allocation();
      
    bool result = non_zero   && 
                  size_ok    &&
                  not_jvmpi;

    return result;
  }

  inline HeapWord* allocate(size_t word_size, bool is_large_noref, bool is_tlab);
  HeapWord* allocate_from_space(size_t word_size);

  inline HeapWord* par_allocate(size_t word_size, bool is_large_noref,
				bool is_tlab);

  inline virtual void gc_prologue(bool full);
  virtual void gc_epilogue(bool full);

  // Accessing marks
  void save_marks();
  void reset_saved_marks();
  bool no_allocs_since_save_marks();

  // Need to declare the full complement of closures, whether we'll
  // override them or not, or get message from the compiler:
  //   oop_since_save_marks_iterate_nv hides virtual function...
#define DefNew_SINCE_SAVE_MARKS_DECL(OopClosureType, nv_suffix)	\
  void oop_since_save_marks_iterate##nv_suffix(OopClosureType* cl);

  ALL_SINCE_SAVE_MARKS_CLOSURES(DefNew_SINCE_SAVE_MARKS_DECL)

#undef DefNew_SINCE_SAVE_MARKS_DECL
  
  // For non-youngest collection, the DefNewGeneration can contribute
  // "to-space".
  void contribute_scratch(ScratchBlock*& list, Generation* requestor,
			  size_t max_alloc_words);

  // GC support
  void compute_new_size();
  virtual void collect(bool   full,
                       bool   clear_all_soft_refs,
                       size_t size, 
		       bool   is_large_noref,
                       bool   is_tlab);
  HeapWord* expand_and_allocate(size_t size,
				bool is_large_noref, bool is_tlab,
				bool parallel = false);

  oop copy_to_survivor_space(oop old, oop* from);
  int tenuring_threshold() { return _tenuring_threshold; }

  // Performance Counter support
  void update_counters();

  // Printing
  virtual const char* name() const;
  virtual const char* short_name() const { return "DefNew"; }

  bool must_be_youngest() const { return true; }
  bool must_be_oldest() const { return false; }

  // PrintHeapAtGC support.
  void print_on(outputStream* st) const;

  void verify(bool allow_dirty) PRODUCT_RETURN;

  int addr_to_arena_id(void* addr);

 protected:
  void compute_space_boundaries(uintx minimum_eden_size);
  // Scavenge support
  void swap_spaces();
  
  // JVMPI support
  void fill_newgen();

};
