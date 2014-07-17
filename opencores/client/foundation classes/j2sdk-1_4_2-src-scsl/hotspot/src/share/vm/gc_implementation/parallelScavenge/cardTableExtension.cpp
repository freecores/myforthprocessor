#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)cardTableExtension.cpp	1.19 03/01/23 12:01:49 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_cardTableExtension.cpp.incl"

// Checks an individual oop for missing precise marks. Mark
// may be either dirty or newgen.
class CheckForUnmarkedOops : public OopClosure {
  PSYoungGen* _young_gen;
  CardTableExtension* _card_table;
  HeapWord* _unmarked_addr;
  jbyte* _unmarked_card;

 public:
  CheckForUnmarkedOops( PSYoungGen* young_gen, CardTableExtension* card_table ) :
    _young_gen(young_gen), _card_table(card_table), _unmarked_addr(NULL) { }

  virtual void do_oop(oop* p) {
    if (_young_gen->is_in(*p) && !_card_table->addr_is_marked_imprecise(p)) {
      // Don't overwrite the first missing card mark
      if (_unmarked_addr == NULL) {
        _unmarked_addr = (HeapWord*)p;
        _unmarked_card = _card_table->byte_for(p);
      }
    }
  }

  bool has_unmarked_oop() {
    return _unmarked_addr != NULL;
  }
};

// Checks all objects for the existance of some type of mark,
// precise or imprecise, dirty or newgen.
class CheckForUnmarkedObjects : public ObjectClosure {
  PSYoungGen* _young_gen;
  CardTableExtension* _card_table;

 public:
  CheckForUnmarkedObjects() {
    ParallelScavengeHeap* heap = (ParallelScavengeHeap*)Universe::heap();
    assert(heap->kind() == CollectedHeap::ParallelScavengeHeap, "Sanity");
    
    _young_gen = heap->young_gen();
    _card_table = (CardTableExtension*)heap->barrier_set();
    // No point in asserting barrier set type here. Need to make CardTableExtension
    // a unique barrier set type.
  }

  // Card marks are not precise. The current system can leave us with
  // a mismash of precise marks and begining of object marks. This means
  // we test for missing precise marks first. If any are found, we don't
  // fail unless the object head is also unmarked.
  virtual void do_object(oop obj) {
    CheckForUnmarkedOops object_check( _young_gen, _card_table );
    obj->oop_iterate(&object_check);
    if (object_check.has_unmarked_oop()) {
      assert(_card_table->addr_is_marked_imprecise(obj), "Found unmarked young_gen object");
    }
  }
};

// Checks for precise marking of oops as newgen.
class CheckForPreciseMarks : public OopClosure {
  PSYoungGen* _young_gen;
  CardTableExtension* _card_table;

 public:
  CheckForPreciseMarks( PSYoungGen* young_gen, CardTableExtension* card_table ) :
    _young_gen(young_gen), _card_table(card_table) { }

  virtual void do_oop(oop* p) {
    if (_young_gen->is_in(*p)) {
      assert(_card_table->addr_is_marked_precise(p), "Found unmarked precise oop");
      _card_table->set_card_newgen(p);
    }
  }
};

// We get passed the space_top value to prevent us from traversing into
// the old_gen promotion labs, which cannot be safely parsed.
void CardTableExtension::scavenge_contents(ObjectStartArray* start_array,
                                           MutableSpace* sp,
                                           HeapWord* space_top,
                                           PSPromotionManager* pm) 
{
  assert(start_array != NULL && sp != NULL && pm != NULL, "Sanity");
  assert(start_array->covered_region().contains(sp->used_region()),
         "ObjectStartArray does not cover space");

  if (sp->not_empty()) {
    oop* sp_top = (oop*)space_top;
    oop* prev_top = NULL;
    jbyte* current_card = byte_for(sp->bottom());
    jbyte* end_card     = byte_for(sp_top - 1);    // sp_top is exclusive
    // scan card marking array
    while (current_card <= end_card) {
      jbyte value = *current_card;
      // skip clean cards
      if (card_is_clean(value)) {
        current_card++;
      } else {
        // we found a non-clean card
        jbyte* first_nonclean_card = current_card++;
        oop* bottom = (oop*)addr_for(first_nonclean_card);
        // find object starting on card
        oop* bottom_obj = (oop*)start_array->object_start((HeapWord*)bottom);
        // bottom_obj = (oop*)start_array->object_start((HeapWord*)bottom);
        assert(bottom_obj <= bottom, "just checking");
        // make sure we don't scan oops we already looked at
        if (bottom < prev_top) bottom = prev_top;
        // figure out when to stop scanning
        jbyte* first_clean_card;
        oop* top;
        bool restart_scanning;
        do {
          restart_scanning = false;
          // find a clean card
          while (current_card <= end_card) {
            value = *current_card;
            if (card_is_clean(value)) break;
            current_card++;
          }
          // check if we reached the end, if so we are done
          if (current_card >= end_card) {
            first_clean_card = end_card + 1;
            current_card++;
            top = sp_top;
          } else {
            // we have a clean card, find object starting on that card
            first_clean_card = current_card++;
            top = (oop*)addr_for(first_clean_card);
            oop* top_obj = (oop*)start_array->object_start((HeapWord*)top);
            // top_obj = (oop*)start_array->object_start((HeapWord*)top);
            assert(top_obj <= top, "just checking");
            if (oop(top_obj)->is_objArray() || oop(top_obj)->is_typeArray()) {
              // an arrayOop is starting on the clean card - since we do exact store
              // checks for objArrays we are done
            } else {
              // otherwise, it is possible that the object starting on the clean card
              // spans the entire card, and that the store happened on a later card.
              // figure out where the object ends
              top = top_obj + oop(top_obj)->size();
              jbyte* top_card = CardTableModRefBS::byte_for(top - 1);   // top is exclusive
              if (top_card > first_clean_card) {
                // object ends a different card
                current_card = top_card + 1;
                if (card_is_clean(*top_card)) {
                  // the ending card is clean, we are done
                  first_clean_card = top_card;
                } else {
                  // the ending card is not clean, continue scanning at start of do-while
                  restart_scanning = true;
                }
              } else {
                // object ends on the clean card, we are done.
                assert(first_clean_card == top_card, "just checking");
              }
            }
          }
        } while (restart_scanning);
        // we know which cards to scan, now clear them
        while (first_nonclean_card < first_clean_card) {
          *first_nonclean_card++ = clean_card;
        }
        // scan oops in objects
        do {
          oop(bottom_obj)->copy_contents(pm);
          bottom_obj += oop(bottom_obj)->size();
          assert(bottom_obj <= sp_top, "just checking");
        } while (bottom_obj < top);
        // remember top oop* scanned
        prev_top = top;
      }
    }
  }
}

void CardTableExtension::scavenge_contents_parallel(ObjectStartArray* start_array,
                                                    MutableSpace* sp,
                                                    HeapWord* space_top,
                                                    PSPromotionManager* pm,
                                                    uint stripe_number) {
  int ssize = 128;      // Naked constant!  Work unit = 64k.
  int dirty_card_count = 0;

  oop* sp_top = (oop*)space_top;
  jbyte* start_card = byte_for(sp->bottom());
  jbyte* end_card   = byte_for(sp_top - 1) + 1;
  oop* last_scanned = NULL; // Prevent scanning objects more than once
  for (jbyte* slice = start_card; slice < end_card; slice += ssize*ParallelGCThreads) {
    jbyte* worker_start_card = slice + stripe_number * ssize;
    if (worker_start_card >= end_card)
      return; // We're done.

    jbyte* worker_end_card = worker_start_card + ssize;
    if (worker_end_card > end_card)
      worker_end_card = end_card;

    // We do not want to scan objects more than once. In order to accomplish
    // this, we assert that any object with an object head inside our 'slice'
    // belongs to us. We may need to extend the range of scanned cards if the
    // last object continues into the next 'slice'.
    //
    // Note! ending cards are exclusive!
    HeapWord* slice_start = addr_for(worker_start_card);
    HeapWord* slice_end = addr_for(worker_end_card);

    // Update our begining addr
    HeapWord* first_object = start_array->object_start(slice_start);
    if (first_object < slice_start) {
      last_scanned = (oop*)(first_object + oop(first_object)->size());

      // It is possible that a single object completely spans this slice.
      // We do not want to scan this slice if that is the case.
      if ((HeapWord*)last_scanned >= slice_end) {
        continue; 
      }

      worker_start_card = byte_for(last_scanned);
    }

    // Update the ending addr
    if (slice_end >= (HeapWord*)sp_top) {
      slice_end = (HeapWord*)sp_top;
    } else {
      // The subtraction is important! An object may start precisely at slice_end.
      HeapWord* last_object = start_array->object_start(slice_end - 1);
      slice_end = last_object + oop(last_object)->size();
      // worker_end_card is exclusive, so bump it one past the end of last_object's
      // covered span.
      worker_end_card = byte_for(slice_end) + 1;

      if (worker_end_card > end_card)
        worker_end_card = end_card;
    }

    assert(slice_end <= (HeapWord*)sp_top, "Last object in slice crosses space boundary");
    assert(is_valid_card_address(worker_start_card), "Invalid worker start card");
    assert(is_valid_card_address(worker_end_card), "Invalid worker end card");
    // Note that worker_start_card >= worker_end_card is legal, and happens when
    // an object spans an entire slice.
    assert(worker_start_card <= end_card, "worker start card beyond end card");
    assert(worker_end_card <= end_card, "worker end card beyond end card");

    jbyte* current_card = worker_start_card;
    while (current_card < worker_end_card) {
      // Find an unclean card.
      while (current_card < worker_end_card && card_is_clean(*current_card)) {
        current_card++;
      }
      jbyte* first_unclean_card = current_card;

      // Find the end of a run of contiguous unclean cards
      while (current_card < worker_end_card && !card_is_clean(*current_card)) {
        while (current_card < worker_end_card && !card_is_clean(*current_card)) {
          current_card++;
        }

        if (current_card < worker_end_card) {
          // Some objects may be large enough to span several cards. If such
          // an object has more than one dirty card, separated by a clean card,
          // we will attempt to scan it twice. The test against "last_scanned"
          // prevents the redundant object scan, but it does not prevent newly
          // marked cards from being cleaned.
          HeapWord* last_object_in_dirty_region = start_array->object_start(addr_for(current_card)-1);
          size_t size_of_last_object = oop(last_object_in_dirty_region)->size();
          HeapWord* end_of_last_object = last_object_in_dirty_region + size_of_last_object;
          jbyte* ending_card_of_last_object = byte_for(end_of_last_object);
          assert(ending_card_of_last_object <= worker_end_card, "ending_card_of_last_object is greater than worker_end_card");
          if (ending_card_of_last_object > (current_card+1)) {
            // This means the object spans the next complete card.
            // We need to bump the current_card to ending_card_of_last_object
            current_card = ending_card_of_last_object;
          }
        }
      }
      jbyte* following_clean_card = current_card;

      if (first_unclean_card < worker_end_card) {
	oop* p = (oop*) start_array->object_start(addr_for(first_unclean_card));
	assert((HeapWord*)p <= addr_for(first_unclean_card), "checking");
	if (p < last_scanned) {   
	  // Avoid scanning more than once; this can happen because
	  // newgen cards set by GC may a different set than the
	  // originally dirty set
	  p = last_scanned;
	}
	oop* to = (oop*)addr_for(following_clean_card);

        // Test slice_end first!
        if ((HeapWord*)to > slice_end) {
          to = (oop*)slice_end;
        } else if (to > sp_top) {
	  to = sp_top;
	} 

        // we know which cards to scan, now clear them
        if (first_unclean_card <= worker_start_card+1)
          first_unclean_card = worker_start_card+1;
        if (following_clean_card >= worker_end_card-1)
          following_clean_card = worker_end_card-1;

        while (first_unclean_card < following_clean_card) {
          *first_unclean_card++ = clean_card;
        }

	const int interval = PrefetchScanIntervalInBytes;
	// scan all objects in the range
	if (interval != 0) {
	  while (p < to) {
	    atomic::prefetch_write(p, interval);
	    oop m = oop(p);
	    assert(m->is_oop_or_null(), "check for header");
	    m->copy_contents(pm);
            p += m->size();
          }
	} else {
	  while (p < to) {
	    oop m = oop(p);
	    assert(m->is_oop_or_null(), "check for header");
	    m->copy_contents(pm);
            p += m->size();
          }
	}
	last_scanned = p;
      }
    }
  }
}

// This should be called before a scavenge.
void CardTableExtension::verify_all_young_refs_imprecise() {
  CheckForUnmarkedObjects check;
  
  ParallelScavengeHeap* heap = (ParallelScavengeHeap*)Universe::heap();
  assert(heap->kind() == CollectedHeap::ParallelScavengeHeap, "Sanity");

  PSOldGen* old_gen = heap->old_gen();
  PSPermGen* perm_gen = heap->perm_gen();

  old_gen->object_iterate(&check);
  perm_gen->object_iterate(&check);
}

// This should be called immediately after a scavenge, before mutators resume.
void CardTableExtension::verify_all_young_refs_precise() {
  ParallelScavengeHeap* heap = (ParallelScavengeHeap*)Universe::heap();
  assert(heap->kind() == CollectedHeap::ParallelScavengeHeap, "Sanity");

  PSOldGen* old_gen = heap->old_gen();
  PSPermGen* perm_gen = heap->perm_gen();

  CheckForPreciseMarks check(heap->young_gen(), (CardTableExtension*)heap->barrier_set());

  old_gen->oop_iterate(&check);
  perm_gen->oop_iterate(&check);

  verify_all_young_refs_precise_helper(old_gen->object_space()->used_region());
  verify_all_young_refs_precise_helper(perm_gen->object_space()->used_region());
}

void CardTableExtension::verify_all_young_refs_precise_helper(MemRegion mr) {
  CardTableExtension* card_table = (CardTableExtension*)Universe::heap()->barrier_set();
  // FIX ME ASSERT HERE

  jbyte* bot = card_table->byte_for(mr.start());
  jbyte* top = card_table->byte_for(mr.end());
  while(bot <= top) {
    assert(*bot == clean_card || *bot == verify_card, "Found unwanted or unknown card mark");
    if (*bot == verify_card)
      *bot = youngergen_card;
    bot++;
  }
}

bool CardTableExtension::addr_is_marked_imprecise(void *addr) {
  jbyte* p = byte_for(addr);
  jbyte val = *p;

  if (card_is_dirty(val))
    return true;

  if (card_is_newgen(val))
    return true;

  if (card_is_clean(val))
    return false;

  assert(false, "Found unhandled card mark type");

  return false;
}

// Also includes verify_card
bool CardTableExtension::addr_is_marked_precise(void *addr) {
  jbyte* p = byte_for(addr);
  jbyte val = *p;

  if (card_is_newgen(val))
    return true;

  if (card_is_verify(val))
    return true;

  if (card_is_clean(val))
    return false;

  if (card_is_dirty(val))
    return false;

  assert(false, "Found unhandled card mark type");

  return false;
}

