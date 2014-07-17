#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)parallelScavengeHeap.hpp	1.19 03/01/23 12:02:15 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

class AdaptiveSizePolicy;

class ParallelScavengeHeap : public CollectedHeap {
 private:
  static PSYoungGen* _young_gen;
  static PSOldGen*   _old_gen;
  static PSPermGen*  _perm_gen;

  // Sizing policy for entire heap
  static AdaptiveSizePolicy* _size_policy;
  static GCPolicyCounters*   _gc_policy_counters;

 protected:
  HeapWord* allocate_new_tlab(size_t size);
  void fill_all_tlabs();

 public:
  // For use by VM operations
  enum CollectionType {
    Scavenge,
    MarkSweep
  };

  ParallelScavengeHeap::Name kind()  { return CollectedHeap::ParallelScavengeHeap; }

  static PSYoungGen* young_gen()     { return _young_gen; }
  static PSOldGen* old_gen()         { return _old_gen; }
  static PSPermGen* perm_gen()       { return _perm_gen; }

  static AdaptiveSizePolicy* size_policy() { return _size_policy; }

  static GCPolicyCounters* gc_policy_counters() { return _gc_policy_counters; }

  void initialize();
  void post_initialize();
  void update_counters();

  // The generations need a reasonable minimum alignment.
  const size_t min_alignment() { return 64 * K; }

  size_t capacity() const;
  size_t used() const;
  
  size_t permanent_capacity() const;
  size_t permanent_used() const;

  size_t max_capacity() const;

  bool is_in(const void* p) const;
  bool is_in_permanent(const void *p) const {
    return perm_gen()->reserved().contains(p);
  }

  static bool is_in_young(oop *p);
  static bool is_in_old_or_perm(oop *p);

  // Memory allocation
  HeapWord* mem_allocate(size_t size, bool is_noref, bool is_tlab);
  HeapWord* failed_mem_allocate(bool& notify_ref_lock,
                                size_t size,
                                bool is_noref,
                                bool is_tlab);

  HeapWord* permanent_mem_allocate(size_t size);
  HeapWord* failed_permanent_mem_allocate(bool& notify_ref_lock, size_t size);

  // Support for System.gc()
  void collect(GCCause::Cause cause);

  size_t large_typearray_limit() { return FastAllocateSizeLimit; }

  bool supports_inline_contig_alloc() const { return true; }
  HeapWord** top_addr() const { return young_gen()->top_addr(); }
  HeapWord** end_addr() const { return young_gen()->end_addr(); }

  void ensure_parseability();

  size_t unsafe_max_alloc();

  bool supports_tlab_allocation() const { return true; }

  size_t tlab_capacity() const;

  void oop_iterate(OopClosure* cl);
  void object_iterate(ObjectClosure* cl);
  void permanent_oop_iterate(OopClosure* cl);
  void permanent_object_iterate(ObjectClosure* cl);

  HeapWord* block_start(const void* addr) const;
  size_t block_size(const HeapWord* addr) const;
  bool block_is_obj(const HeapWord* addr) const;

  jlong millis_since_last_gc();

  void prepare_for_verify();
  void print() const;
  void print_on(outputStream* st) const;

#ifndef PRODUCT
  void verify(bool allow_dirty, bool silent);
#endif

  void print_heap_change(size_t prev_used);

  int addr_to_arena_id(void* addr);
};

