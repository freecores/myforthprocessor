#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)tenuredGeneration.hpp	1.8 03/01/23 12:10:32 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// TenuredGeneration models the heap containing old (promoted/tenured) objects.

class ParGCAllocBufferWithBOT;

class TenuredGeneration: public OneContigSpaceCardGeneration {
  friend class VMStructs;
 protected:
  // current shrinking effect: this damps shrinking when the heap gets empty.
  size_t _shrink_factor;
  // Some statistics from before gc started.
  // These are gathered in the gc_prologue (and should_collect) 
  // to control growing/shrinking policy in spite of promotions.
  size_t _capacity_at_prologue;

  // To support parallel promotion: an array of parallel allocation
  // buffers, one per thread, initially NULL.
  ParGCAllocBufferWithBOT** _alloc_buffers;

  // Retire all alloc buffers before a full GC, so that they will be
  // re-allocated at the start of the next young GC.
  void retire_alloc_buffers_before_full_gc();

  GenerationCounters*   _gen_counters;
  CSpaceCounters*       _space_counters;
  
 public:
  TenuredGeneration(ReservedSpace rs, size_t initial_byte_size, int level,
		    GenRemSet* remset);
  
  Generation::Name kind() { return Generation::MarkSweepCompact; }
  
  // Printing
  const char* name() const;
  const char* short_name() const { return "Tenured"; }
  bool must_be_youngest() const { return false; }
  bool must_be_oldest() const { return true; }

  // override 
  int addr_to_arena_id(void* addr); 
  bool full_collects_younger_generations() const { return true; }

  // Mark sweep support
  void compute_new_size();
  int allowed_dead_ratio() const;

  virtual void gc_prologue(bool full);
  bool should_collect(bool   full,
                      size_t word_size,
                      bool   is_large_noref,
                      bool   is_tlab);

  virtual void collect(bool full,
                       bool clear_all_soft_refs,
                       size_t size, 
		       bool is_large_noref,
                       bool is_tlab);

  // Overrides.
  virtual oop par_promote(int thread_num,
			  oop obj, markOop m, size_t word_sz);
  virtual void par_promote_alloc_undo(int thread_num,
				      HeapWord* obj, size_t word_sz);
  virtual void par_promote_alloc_done(int thread_num);

  // Performance Counter support
  void update_counters();
};
