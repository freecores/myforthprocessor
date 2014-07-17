#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)psYoungGen.cpp	1.32 03/01/23 12:03:05 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_psYoungGen.cpp.incl"

PSYoungGen::PSYoungGen(ReservedSpace rs, 
                       size_t        initial_size,
                       size_t        min_size, 
                       size_t        max_size) :
  _init_gen_size(initial_size), 
  _min_gen_size(min_size),
  _max_gen_size(max_size), 
  _last_survivor_size(0),
  _last_survivor_size_valid(false)
{
  if (!_virtual_space.initialize(rs, initial_size)) {
    vm_exit_during_initialization("Could not reserve enough space for "
                                  "object heap");
  }

  _reserved = MemRegion((HeapWord*)_virtual_space.low_boundary(),
                        (HeapWord*)_virtual_space.high_boundary());

  MemRegion cmr((HeapWord*)_virtual_space.low(),
		(HeapWord*)_virtual_space.high());
  Universe::heap()->barrier_set()->resize_covered_region(cmr);

  _eden_space = new MutableSpace();
  _from_space = new MutableSpace();
  _to_space   = new MutableSpace();

  if (_eden_space == NULL || _from_space == NULL || _to_space == NULL) {
    vm_exit_during_initialization("Could not allocate a young gen space");
  }

  // Allocate the mark sweep views of spaces
  _eden_mark_sweep =
      new PSMarkSweepDecorator(_eden_space, NULL, MarkSweepDeadRatio);
  _from_mark_sweep =
      new PSMarkSweepDecorator(_from_space, NULL, MarkSweepDeadRatio);
  _to_mark_sweep =
      new PSMarkSweepDecorator(_to_space, NULL, MarkSweepDeadRatio);

  if (_eden_mark_sweep == NULL ||
      _from_mark_sweep == NULL ||
      _to_mark_sweep == NULL) {
    vm_exit_during_initialization("Could not complete allocation"
                                  " of the young generation");
  }

  // Generation Counters - generation 1, 3 subspaces
  _gen_counters = new GenerationCounters(PERF_GC, "new", 0, 3, &_virtual_space);

  // Compute sizes
  ParallelScavengeHeap* heap = (ParallelScavengeHeap*)Universe::heap();
  size_t alignment = heap->min_alignment();
  size_t size = _virtual_space.reserved_size();

  size_t max_survivor_size;

  if (UseAdaptiveSizePolicy) {
    max_survivor_size = size / MinSurvivorRatio;
  }
  else {
    max_survivor_size = size / InitialSurvivorRatio;
  }

  max_survivor_size = align_size_down(max_survivor_size, alignment);
  // ... but never less than an alignment
  max_survivor_size = MAX2(max_survivor_size, alignment);

  // Young generation is eden + 2 survivor spaces
  size_t max_eden_size = size - (2 * max_survivor_size);

  const char* ns = _gen_counters->name_space();

  _eden_counters = new SpaceCounters(ns, "eden", 0, max_eden_size, _eden_space);
  _from_counters = new SpaceCounters(ns, "s0", 1, max_survivor_size, _from_space);
  _to_counters = new SpaceCounters(ns, "s1", 2, max_survivor_size, _to_space);

  compute_initial_space_boundaries();
}

void PSYoungGen::compute_initial_space_boundaries() {
  ParallelScavengeHeap* heap = (ParallelScavengeHeap*)Universe::heap();
  assert(heap->kind() == CollectedHeap::ParallelScavengeHeap, "Sanity");

  // Compute sizes
  size_t alignment = heap->min_alignment();
  size_t size = _virtual_space.committed_size();

  size_t survivor_size = size / InitialSurvivorRatio;
  survivor_size = align_size_down(survivor_size, alignment);
  // ... but never less than an alignment
  survivor_size = MAX2(survivor_size, alignment);

  // Young generation is eden + 2 survivor spaces
  size_t eden_size = size - (2 * survivor_size);

  // Now go ahead and set 'em.
  set_space_boundaries(eden_size, survivor_size);
  space_invariants();

  if (UsePerfData) {
    _eden_counters->update_capacity();
    _from_counters->update_capacity();
    _to_counters->update_capacity();
  }
}

void PSYoungGen::set_space_boundaries(size_t eden_size, size_t survivor_size) {
  assert(eden_size < _virtual_space.committed_size(), "just checking");
  assert(eden_size > 0  && survivor_size > 0, "just checking");

  // Initial layout is Eden, to, from. After swapping survivor spaces,
  // that leaves us with Eden, from, to, which is step one in our two
  // step resize-with-live-data procedure.
  char *eden_start = _virtual_space.low();
  char *to_start   = eden_start + eden_size;
  char *from_start = to_start   + survivor_size;
  char *from_end   = from_start + survivor_size;

  assert(from_end == _virtual_space.high(), "just checking");
  assert(is_object_aligned((intptr_t)eden_start), "checking alignment");
  assert(is_object_aligned((intptr_t)to_start),   "checking alignment");
  assert(is_object_aligned((intptr_t)from_start), "checking alignment");

  MemRegion eden_mr((HeapWord*)eden_start, (HeapWord*)to_start);
  MemRegion to_mr  ((HeapWord*)to_start, (HeapWord*)from_start);
  MemRegion from_mr((HeapWord*)from_start, (HeapWord*)from_end);

  eden_space()->initialize(eden_mr, true);
    to_space()->initialize(to_mr  , true);
  from_space()->initialize(from_mr, true);

  // Since we've totally changed the layout, 
  // clear any saved _last_survivor_size.
  unset_last_survivor_size();
}

#ifndef PRODUCT
void PSYoungGen::space_invariants() {
  // Currently, our eden size cannot shrink to zero
  guarantee(eden_space()->capacity_in_bytes() > 0, "eden size == 0");
  guarantee(from_space()->capacity_in_bytes() > 0, "from size == 0");
  guarantee(to_space()->capacity_in_bytes() > 0, "to size == 0");

  // Relationship of spaces to each other
  char* eden_start = (char*)eden_space()->bottom();
  char* eden_end   = (char*)eden_space()->end();   
  char* from_start = (char*)from_space()->bottom();
  char* from_end   = (char*)from_space()->end();
  char* to_start   = (char*)to_space()->bottom();
  char* to_end     = (char*)to_space()->end();

  guarantee(eden_start == _virtual_space.low(), "eden bottom");
  guarantee(eden_start < eden_end, "eden space consistency");
  guarantee(from_start < from_end, "from space consistency");
  guarantee(to_start < to_end, "to space consistency");

  // Check whether from space is below to space
  if (from_start < to_start) {
    // Eden, from, to
    guarantee(eden_end == from_start, "eden/from boundary");
    guarantee(from_end == to_start,   "from/to boundary");
    guarantee(to_end == _virtual_space.high(), "to end");
  } else {
    // Eden, to, from
    guarantee(eden_end == to_start, "eden/to boundary");
    guarantee(to_end == from_start, "to/from boundary");
    guarantee(from_end == _virtual_space.high(), "from end");
  }
}
#endif

void PSYoungGen::resize(size_t eden_size, size_t survivor_size) {
  // If there's no data in the generation, we can do a very simple
  // resize. Otherwise, we call resize_with_live, which has a bunch
  // more logic to resize things without messing up live data.
  // If there's any live data, we'll do the more elaborate resizing

  guarantee ((eden_size + 2*survivor_size)  <= _max_gen_size, 
                                         "incorrect input arguments");

  // Used for printing below
  const size_t size_before = _virtual_space.committed_size();

  if (!eden_space()->is_empty() ||
      !to_space()->is_empty()   ||
      !from_space()->is_empty()) {

    if (UseAdaptiveSizePolicy) {
      resize_with_live(eden_size, survivor_size);
    }
  } else {

    // We can do a simple resize of the generation, using the sizes
    // passed in for eden and the survivor spaces.
    char *eden_start = _virtual_space.low();
    resize_helper(eden_size + (2 * survivor_size), eden_start);
    
    // Recalculate eden size. We might have changed the expected
    // size while deciding the correct generation size.
    eden_size = _virtual_space.committed_size() - (2*survivor_size);
    set_space_boundaries(eden_size, survivor_size);
  }

  space_invariants();

  if (PrintAdaptiveSizePolicy) {
    ParallelScavengeHeap* heap = (ParallelScavengeHeap*)Universe::heap();
    assert(heap->kind() == CollectedHeap::ParallelScavengeHeap, "Sanity");

    const size_t size_after = _virtual_space.committed_size();
    // Don't print anything if the size didn't change
    if (size_before != size_after) {

      tty->print_cr("AdaptiveSizePolicy::young generation size: "
                    "collection: %d "
                    "(" SIZE_FORMAT ") -> (" SIZE_FORMAT ") ",
                    heap->total_collections(),
                    size_before, size_after);
    }
  }
}

bool PSYoungGen::resize_helper(size_t desired_size, char* shrink_bound) {
  bool changed = false;

  size_t new_size_before = _virtual_space.committed_size();

  assert(_max_gen_size == reserved().byte_size(), "max gen size problem?");
  assert(_min_gen_size <= new_size_before &&
         new_size_before <= _max_gen_size, "just checking");

  ParallelScavengeHeap* heap = (ParallelScavengeHeap*)Universe::heap();
  assert(heap->kind() == CollectedHeap::ParallelScavengeHeap, "Sanity");
  const size_t alignment = heap->min_alignment();

  int threads_count = Threads::number_of_non_daemon_threads();
  size_t thread_increase_size = threads_count * NewSizeThreadIncrease;
  desired_size = align_size_up(desired_size + thread_increase_size, alignment);

  // Adjust new generation size
  desired_size = MAX2(MIN2(desired_size, _max_gen_size), _min_gen_size);
  assert(desired_size <= _max_gen_size, "just checking");

  if (desired_size > new_size_before) {
    // Grow the generation
    size_t change = desired_size - new_size_before;
    assert(change % alignment == 0, "just checking");
    changed = _virtual_space.expand_by(change);
  }
  // We're probably shrinking;  don't allow shrinking past the limit
  // sent in if non-NULL. This limit might cause us to not shrink at all.
  if (shrink_bound != NULL) {
    assert (shrink_bound >= _virtual_space.low(), "shrink boundary too low");
    size_t limit = pointer_delta(shrink_bound, 
                                 _virtual_space.low(),
                                 sizeof(char));
    limit = align_size_up(limit, alignment);
    desired_size = MAX2(desired_size, limit);
  }

  if (desired_size < new_size_before) {
    // Shrink the generation
    size_t change = new_size_before - desired_size;
    assert(change % alignment == 0, "just checking");
    _virtual_space.shrink_by(change);
    changed = true;
  }

  if (changed) {
    // Update the barrier set
    MemRegion cmr((HeapWord*)_virtual_space.low(), 
                  (HeapWord*)_virtual_space.high());
    Universe::heap()->barrier_set()->resize_covered_region(cmr);

    if (Verbose && PrintGC) {
      size_t new_size_after  = _virtual_space.committed_size();
      gclog_or_tty->print("New generation size changed: " SIZE_FORMAT "K->" 
                                                 SIZE_FORMAT "K",
                    new_size_before/K, new_size_after/K);
      if (WizardMode) {
        gclog_or_tty->print("[allowed " SIZE_FORMAT "K extra for %d threads]", 
                      thread_increase_size/K, threads_count);
      }
      tty->cr();
    }
  }
  return changed;
}

void PSYoungGen::resize_with_live(size_t eden_size, size_t survivor_size) {
  assert(UseAdaptiveSizePolicy, "sanity check");

  // We require eden and to space to be empty
  if ((!eden_space()->is_empty()) || (!to_space()->is_empty())) {
    return;
  }

  if (PrintAdaptiveSizePolicy && Verbose) {
    tty->print_cr("PSYoungGen::resize_with_live(eden_size: " SIZE_FORMAT 
                  ", survivor_size: " SIZE_FORMAT ")",
                  eden_size, survivor_size);
    tty->print_cr("    eden: [" PTR_FORMAT ".." PTR_FORMAT ") " SIZE_FORMAT, 
                  eden_space()->bottom(), 
                  eden_space()->end(), 
                  pointer_delta(eden_space()->end(),
                                eden_space()->bottom(),
                                sizeof(char)));
    tty->print_cr("    from: [" PTR_FORMAT ".." PTR_FORMAT ") " SIZE_FORMAT, 
                  from_space()->bottom(), 
                  from_space()->end(), 
                  pointer_delta(from_space()->end(),
                                from_space()->bottom(),
                                sizeof(char)));
    tty->print_cr("      to: [" PTR_FORMAT ".." PTR_FORMAT ") " SIZE_FORMAT, 
                  to_space()->bottom(),   
                  to_space()->end(), 
                  pointer_delta(  to_space()->end(),
                                  to_space()->bottom(),
                                  sizeof(char)));
  }
  char* eden_start = (char*)eden_space()->bottom();
  char* eden_end   = (char*)eden_space()->end();   
  char* from_start = (char*)from_space()->bottom();
  char* from_end   = (char*)from_space()->end();
  char* to_start   = (char*)to_space()->bottom();
  char* to_end     = (char*)to_space()->end();

  // Check whether from space is below to space
  if (from_start < to_start) {
    // Eden, from, to
    if (PrintAdaptiveSizePolicy && Verbose) {
      tty->print_cr("  Eden, from, to:");
    }

    // Cache the value used in the computation;  we'll use it
    // to finish the resize at the next GC.
    set_last_survivor_size(survivor_size);

    // There's nothing to do if the new sizes are the same as the current
    if (survivor_size == to_space()->capacity_in_bytes() && 
        survivor_size == from_space()->capacity_in_bytes() &&
        eden_size == eden_space()->capacity_in_bytes()) {
      if (PrintAdaptiveSizePolicy && Verbose) {
        tty->print_cr("    capacities are the right sizes, returning");
      }
      return;
    }

    // We don't want to shrink the heap into from space, but
    // we want to ensure that there's at least some to space left
    // as well. 

    const size_t desired_new_size = eden_size + (2 * survivor_size);
    
    // Don't let the to space shrink a great deal all at once
    const double limit_factor = 15.0/16.0;
    const size_t survivor_limit = survivor_size * limit_factor;
    const bool new_changed =
        resize_helper(desired_new_size, to_start + survivor_limit);

    // We can shrink Eden, and we can grow from space.
    assert(eden_size > 0  && survivor_size > 0, "just checking");

    to_end   = (char*)_virtual_space.high();
    eden_end = from_start = MIN2(eden_start + eden_size, from_start);

    // Leave the largest block in from space -- it'll be to space
    // at the next GC, and we'll be able to adjust the Eden/to space bound.
    from_end = to_start   = MAX2((char*)pointer_delta(to_end,
                                                      (char*)survivor_size,
                                                      sizeof(char)),
                                 from_end);

    guarantee(to_start != to_end, "to space is zero sized");
      
    if (PrintAdaptiveSizePolicy && Verbose) {
      tty->print_cr("            desired_new_size: " SIZE_FORMAT, 
                    desired_new_size);
      tty->print_cr("                limit_factor: %f",
                    limit_factor);
      tty->print_cr("              survivor_limit: " SIZE_FORMAT, 
                    survivor_limit);
      tty->print_cr("                 new_changed: %s",
                    new_changed ? "true" : "false");
      tty->print_cr("         _last_survivor_size: " SIZE_FORMAT, 
                    _last_survivor_size);
      tty->print_cr("    [eden_start .. eden_end): "
                    "[" PTR_FORMAT " .. " PTR_FORMAT ") " SIZE_FORMAT, 
                    eden_start, 
                    eden_end, 
                    pointer_delta(eden_end, eden_start, sizeof(char)));
      tty->print_cr("    [from_start .. from_end): "
                    "[" PTR_FORMAT " .. " PTR_FORMAT ") " SIZE_FORMAT, 
                    from_start, 
                    from_end, 
                    pointer_delta(from_end, from_start, sizeof(char)));
      tty->print_cr("    [  to_start ..   to_end): "
                    "[" PTR_FORMAT " .. " PTR_FORMAT ") " SIZE_FORMAT, 
                    to_start,   
                    to_end, 
                    pointer_delta(  to_end,   to_start, sizeof(char)));
    }
  } else {
    // Eden, to, from
    //
    // Finish up the work started above, using the survivor size
    // from the last GC. We probably got one of the Eden/to or to/from
    // boundaries correct above (depending on if we were shrinking or
    // growing); this pass will correct the other boundary.
    if (PrintAdaptiveSizePolicy && Verbose) {
      tty->print_cr("  Eden, to, from:");
    }

    if (!last_survivor_size_valid()) {
      if (PrintAdaptiveSizePolicy && Verbose) {
        tty->print_cr("    no last_survivor_size, returning");
      }
      return;
    }
      
    const size_t younggen_size = _virtual_space.committed_size();
    const size_t last_survivor_size = get_and_unset_last_survivor_size();

    // We'll recompute eden size
    eden_size = younggen_size - (2*last_survivor_size);
    assert(eden_size > 0  && survivor_size > 0, "just checking");

    // There's nothing to do if the new sizes are the same as the current
    if (last_survivor_size == to_space()->capacity_in_bytes() && 
        last_survivor_size == from_space()->capacity_in_bytes() &&
        eden_size == eden_space()->capacity_in_bytes()) {
      if (PrintAdaptiveSizePolicy && Verbose) {
        tty->print_cr("    capacities are the right sizes, returning");
      }
      return;
    }

    eden_end = to_start   = eden_start + eden_size;
    to_end   = from_start = MIN2(to_start + last_survivor_size, from_start);
    from_end = (char*)_virtual_space.high();
    if (PrintAdaptiveSizePolicy && Verbose) {
      tty->print_cr("               younggen_size: " SIZE_FORMAT, 
                    younggen_size);
      tty->print_cr("          last_survivor_size: " SIZE_FORMAT, 
                    last_survivor_size);
      tty->print_cr("                   eden_size: " SIZE_FORMAT, 
                    eden_size);
      tty->print_cr("    [eden_start .. eden_end): "
                    "[" PTR_FORMAT " .. " PTR_FORMAT ") " SIZE_FORMAT, 
                    eden_start, 
                    eden_end, 
                    pointer_delta(eden_end, eden_start, sizeof(char)));
      tty->print_cr("    [  to_start ..   to_end): " 
                    "[" PTR_FORMAT " .. " PTR_FORMAT ") " SIZE_FORMAT, 
                    to_start,   
                    to_end, 
                    pointer_delta(  to_end,   to_start, sizeof(char)));
      tty->print_cr("    [from_start .. from_end): " 
                    "[" PTR_FORMAT " .. " PTR_FORMAT ") " SIZE_FORMAT, 
                    from_start, 
                    from_end, 
                    pointer_delta(from_end, from_start, sizeof(char)));
    }
  }

  guarantee((HeapWord*)from_start <= from_space()->bottom(), 
            "from start moved to the right");
  guarantee((HeapWord*)from_end >= from_space()->end(),
            "from end moved to the left");
  assert(is_object_aligned((intptr_t)eden_start), "checking alignment");
  assert(is_object_aligned((intptr_t)from_start), "checking alignment");
  assert(is_object_aligned((intptr_t)to_start), "checking alignment");

  MemRegion edenMR((HeapWord*)eden_start, (HeapWord*)eden_end);
  MemRegion toMR  ((HeapWord*)to_start,   (HeapWord*)to_end);
  MemRegion fromMR((HeapWord*)from_start, (HeapWord*)from_end);

  // The call to initialize will reset "bottom"
  HeapWord* start_of_from_objects = from_space()->bottom();
  // Let's make sure the call to initialize doesn't reset "top"!
  HeapWord* old_from_top = from_space()->top();

  // For PrintAdaptiveSizePolicy block  below
  size_t old_from = from_space()->capacity_in_bytes();
  size_t old_to   = to_space()->capacity_in_bytes();

  eden_space()->initialize(edenMR, true);
    to_space()->initialize(toMR  , true);
  from_space()->initialize(fromMR, false);     // Note, not cleared!

  assert(from_space()->top() == old_from_top, "from top changed!");

  // Used for PrintAdaptiveSizePolicy code below
  int loop_trips = -1;
  size_t array_length = 0;

  // If from space isn't empty, and the bottom boundary changed,
  // we need to insert a filler object up to the old boundary.
  if (!from_space()->is_empty() &&
      from_space()->bottom() != start_of_from_objects) {
    HeapWord* from_bottom = from_space()->bottom();
    assert(from_bottom < start_of_from_objects,
           "from bottom is not below the start of from objects");
    
    // Check that the filler object isn't bigger than we can represent
    //
    // Due to the way arrays calculate their size, they cannot be larger
    // than 2GB in size. To avoid problems with overflow, we cap them at 1GB.
    const size_t max_filler_len = ((size_t)G / (size_t)sizeof(jint)) - 
                                   typeArrayOopDesc::header_size(T_INT);
    
    while (from_bottom < start_of_from_objects) {
      size_t desired_filler_len =
        pointer_delta(start_of_from_objects, from_bottom) -
        typeArrayOopDesc::header_size(T_INT);

      // We have to consider the case where desired == max + 1 heapword!
      if (desired_filler_len > max_filler_len) {
        desired_filler_len = max_filler_len;
        desired_filler_len -= typeArrayOopDesc::header_size(T_INT);
      }      
      typeArrayOop filler_oop = (typeArrayOop) from_bottom;
      filler_oop->set_mark();
      filler_oop->set_klass(Universe::intArrayKlassObj());
      array_length = desired_filler_len;
      loop_trips++;
      assert((array_length * (HeapWordSize/sizeof(jint))) < max_jint, 
             "array_length too big for ArrayOop");
      filler_oop->set_length((int)(array_length * (HeapWordSize/sizeof(jint))));
      
#ifdef ASSERT
      HeapWord* elt_words =
        (((HeapWord*)filler_oop) + typeArrayOopDesc::header_size(T_INT));
      Memory::set_words(elt_words, array_length, 0xDEAFBABE);
#endif
      
      from_bottom = from_bottom + filler_oop->size();

      assert(from_bottom <= start_of_from_objects, "Sanity");
    }
  }

  if (PrintAdaptiveSizePolicy) {
    ParallelScavengeHeap* heap = (ParallelScavengeHeap*)Universe::heap();
    assert(heap->kind() == CollectedHeap::ParallelScavengeHeap, "Sanity");

    tty->print("AdaptiveSizePolicy::survivor space sizes: "
                  "collection: %d "
                  "(" SIZE_FORMAT ", " SIZE_FORMAT ") -> "
                  "(" SIZE_FORMAT ", " SIZE_FORMAT ") "
                  "filler object size: " SIZE_FORMAT,
                  heap->total_collections(),
                  old_from, old_to,
                  from_space()->capacity_in_bytes(),
                  to_space()->capacity_in_bytes(),
                  array_length);
    if (loop_trips > 0) {
      tty->print(" (%d) ", loop_trips);
    }
    tty->cr();
  }
}

void PSYoungGen::swap_spaces() {
  MutableSpace* s    = from_space();
  _from_space        = to_space();
  _to_space          = s;

  // Now update the decorators.
  PSMarkSweepDecorator* md = from_mark_sweep();
  _from_mark_sweep           = to_mark_sweep();
  _to_mark_sweep             = md;

  assert(from_mark_sweep()->space() == from_space(), "Sanity");
  assert(to_mark_sweep()->space() == to_space(), "Sanity");
}

size_t PSYoungGen::capacity_in_bytes() const {
  return eden_space()->capacity_in_bytes()
       + from_space()->capacity_in_bytes();  // to_space() is only used during scavenge
}


size_t PSYoungGen::used_in_bytes() const {
  return eden_space()->used_in_bytes()
       + from_space()->used_in_bytes();      // to_space() is only used during scavenge
}


size_t PSYoungGen::free_in_bytes() const {
  return eden_space()->free_in_bytes()
       + from_space()->free_in_bytes();      // to_space() is only used during scavenge
}

size_t PSYoungGen::capacity_in_words() const {
  return eden_space()->capacity_in_words()
       + from_space()->capacity_in_words();  // to_space() is only used during scavenge
}


size_t PSYoungGen::used_in_words() const {
  return eden_space()->used_in_words()
       + from_space()->used_in_words();      // to_space() is only used during scavenge
}


size_t PSYoungGen::free_in_words() const {
  return eden_space()->free_in_words()
       + from_space()->free_in_words();      // to_space() is only used during scavenge
}

void PSYoungGen::object_iterate(ObjectClosure* blk) {
  eden_space()->object_iterate(blk);
  from_space()->object_iterate(blk);
  to_space()->object_iterate(blk);
}

void PSYoungGen::precompact() {
  eden_mark_sweep()->precompact();
  from_mark_sweep()->precompact();
  to_mark_sweep()->precompact();
}

void PSYoungGen::adjust_pointers() {
  eden_mark_sweep()->adjust_pointers();
  from_mark_sweep()->adjust_pointers();
  to_mark_sweep()->adjust_pointers();
}

void PSYoungGen::compact() {
  eden_mark_sweep()->compact(ZapUnusedHeapArea);
  from_mark_sweep()->compact(ZapUnusedHeapArea);
  // Mark sweep stores preserved markOops in to space, don't disturb!
  to_mark_sweep()->compact(false);
}

HeapWord* PSYoungGen::expand_and_allocate(size_t size,
                                          bool   is_large_noref,
                                          bool   is_tlab) {
  // We don't attempt to expand the young generation.
  return NULL;
}

void PSYoungGen::print() const { print_on(tty); }
void PSYoungGen::print_on(outputStream* st) const {
  st->print(" %-15s", "PSYoungGen");
  st->print(" total " SIZE_FORMAT "K, used " SIZE_FORMAT "K",
             capacity_in_bytes()/K, used_in_bytes()/K);
  st->print_cr(" [" INTPTR_FORMAT ", " INTPTR_FORMAT ", " INTPTR_FORMAT ")",
              _virtual_space.low_boundary(),
              _virtual_space.high(),
              _virtual_space.high_boundary());
  
  st->print("  eden"); eden_space()->print_on(st);
  st->print("  from"); from_space()->print_on(st);
  st->print("  to  "); to_space()->print_on(st);
}

void PSYoungGen::unset_last_survivor_size() {
  _last_survivor_size = 0;
  _last_survivor_size_valid = false;
}

void PSYoungGen::set_last_survivor_size(size_t value) {
  _last_survivor_size = value;
  _last_survivor_size_valid = true;
}

size_t PSYoungGen::get_and_unset_last_survivor_size() {
  guarantee(last_survivor_size_valid(),
            "Shouldn't be reading _last_survivor_size");
  size_t result = _last_survivor_size;
  unset_last_survivor_size();
  return result;
}

bool PSYoungGen::last_survivor_size_valid() const {
  return _last_survivor_size_valid;
}

void PSYoungGen::update_counters() {
  if (UsePerfData) {
    _eden_counters->update_all();
    _from_counters->update_all();
    _to_counters->update_all();
    _gen_counters->update_all();
  }
}

#ifndef PRODUCT

void PSYoungGen::verify(bool allow_dirty) {
  eden_space()->verify(allow_dirty);
  from_space()->verify(allow_dirty);
  to_space()->verify(allow_dirty);
}

#endif // !PRODUCT
