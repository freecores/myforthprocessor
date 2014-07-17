#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)cardTableRS.cpp	1.24 03/01/23 12:07:11 JVM"
#endif
// 
// Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
// SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
// 

# include "incls/_precompiled.incl"
# include "incls/_cardTableRS.cpp.incl"

CardTableRS::CardTableRS(MemRegion whole_heap,
			 int max_covered_regions) :
  GenRemSet(&_ct_bs),
  _ct_bs(whole_heap, max_covered_regions),
  _cur_youngergen_card_val(youngergenP1_card)
{
  _last_cur_val_in_gen = new jbyte[GenCollectedHeap::max_gens + 1];
  if (_last_cur_val_in_gen == NULL) {
    vm_exit_during_initialization("Could not last_cur_val_in_gen array.");
  }
  for (int i = 0; i < GenCollectedHeap::max_gens + 1; i++) {
    _last_cur_val_in_gen[i] = clean_card_val();
  }
  _ct_bs.set_CTRS(this);
}

void CardTableRS::resize_covered_region(MemRegion new_region) {
  _ct_bs.resize_covered_region(new_region);
}

jbyte CardTableRS::find_unused_youngergenP_card_value() {
  GenCollectedHeap* gch = GenCollectedHeap::heap();
  for (jbyte v = youngergenP1_card;
       v < cur_youngergen_and_prev_nonclean_card;
       v++) {
    bool seen = false;
    for (int g = 0; g < gch->n_gens()+1; g++) {
      if (_last_cur_val_in_gen[g] == v) {
	seen = true;
	break;
      }
    }
    if (!seen) return v;
  }
  ShouldNotReachHere();
  return 0;
}

void CardTableRS::prepare_for_younger_refs_iterate(bool parallel) {
  // Parallel or sequential, we must always set the prev to equal the
  // last one written.
  if (parallel) {
    // Find a parallel value to be used next.
    jbyte next_val = find_unused_youngergenP_card_value();
    set_cur_youngergen_card_val(next_val);

  } else {
    // In an sequential traversal we will always write youngergen, so that
    // the inline barrier is  correct.
    set_cur_youngergen_card_val(youngergen_card);
  }
}

void CardTableRS::younger_refs_iterate(Generation* g,
				       OopsInGenClosure* blk) {
  _last_cur_val_in_gen[g->level()+1] = cur_youngergen_card_val();
  g->younger_refs_iterate(blk);
}

void CardTableRS::clear(MemRegion mr) {
  for (int i = 0; i < _ct_bs._cur_covered_regions; i++) {
    MemRegion mri = mr.intersection(_ct_bs._covered[i]);
    if (mri.byte_size() > 0) clear_MemRegion(mri);
  }
}

class ClearNoncleanCardWrapper: public MemRegionClosure {
  MemRegionClosure* _dirty_card_closure;
  CardTableRS* _ct;
  bool _is_par;
private:
  // Clears the given card, return true if the corresponding card should be 
  // processed.
  bool clear_card(jbyte* entry) {
    if (_is_par) {
      while (true) {
	// In the parallel case, we may have to do this several times.
	jbyte entry_val = *entry;
	assert(entry_val != CardTableRS::clean_card_val(),
	       "We shouldn't be looking at clean cards, and this should "
	       "be the only place they get cleaned.");
	if (CardTableRS::card_is_dirty_wrt_gen_iter(entry_val)
	    || _ct->is_prev_youngergen_card_val(entry_val)) {
	  jbyte res =
	    atomic::compare_and_exchange_byte(CardTableRS::clean_card_val(),
					      entry, 
					      entry_val);
	  if (res == entry_val) {
	    break;
	  } else {
	    assert(res == CardTableRS::cur_youngergen_and_prev_nonclean_card,
		   "The CAS above should only fail if another thread did "
		   "a GC write barrier.");
	  }
	} else if (entry_val ==
		   CardTableRS::cur_youngergen_and_prev_nonclean_card) {
	  // Parallelism shouldn't matter in this case.  Only the thread
	  // assigned to scan the card should change this value.
	  *entry = _ct->cur_youngergen_card_val();
	  break;
	} else {
	  assert(entry_val == _ct->cur_youngergen_card_val(),
		 "Should be the only possibility.");
	  // In this case, the card was clean before, and become
	  // cur_youngergen only because of processing of a promoted object.
	  // We don't have to look at the card.
	  return false;
	}
      }
      return true;
    } else {
      jbyte entry_val = *entry;
      assert(entry_val != CardTableRS::clean_card_val(),
	     "We shouldn't be looking at clean cards, and this should "
	     "be the only place they get cleaned.");
      assert(entry_val != CardTableRS::cur_youngergen_and_prev_nonclean_card,
	     "This should be possible in the sequential case.");
      *entry = CardTableRS::clean_card_val();
      return true;
    }
  }

public:
  ClearNoncleanCardWrapper(MemRegionClosure* dirty_card_closure,
			   CardTableRS* ct) :
    _dirty_card_closure(dirty_card_closure), _ct(ct) {
    _is_par = (SharedHeap::heap()->n_par_threads() > 0);
  }
  void do_MemRegion(MemRegion mr) {
    HeapWord* end_of_non_clean = mr.end();
    HeapWord* start_of_non_clean = end_of_non_clean;
    jbyte* entry = _ct->byte_for(mr.last());
    HeapWord* cur = _ct->addr_for(entry);
    while (mr.contains(cur)) {
      jbyte entry_val = *entry;
      if (!clear_card(entry)) {
	if (start_of_non_clean < end_of_non_clean) {
	  MemRegion mr2(start_of_non_clean, end_of_non_clean);
	  _dirty_card_closure->do_MemRegion(mr2);
	}
	end_of_non_clean = cur;
	start_of_non_clean = end_of_non_clean;
      }
      entry--;
      start_of_non_clean = cur;
      cur = _ct->addr_for(entry);
    }
    if (start_of_non_clean < end_of_non_clean) {
      MemRegion mr2(start_of_non_clean, end_of_non_clean);
      _dirty_card_closure->do_MemRegion(mr2);
    }
  }
};
// clean (by dirty->clean before) ==> cur_younger_gen
// dirty                          ==> cur_youngergen_and_prev_nonclean_card
// precleaned			  ==> cur_youngergen_and_prev_nonclean_card
// prev-younger-gen               ==> cur_youngergen_and_prev_nonclean_card
// cur-younger-gen                ==> cur_younger_gen
// cur_youngergen_and_prev_nonclean_card ==> no change.
void CardTableRS::write_ref_field_gc_par(oop* field, oop new_val) {
  jbyte* entry = ct_bs()->byte_for(field);
  do {
    jbyte entry_val = *entry;
    // We put this first because it's probably the most common case.
    if (entry_val == clean_card_val()) {
      // No threat of contention with cleaning threads.
      *entry = cur_youngergen_card_val();
      return;
    } else if (card_is_dirty_wrt_gen_iter(entry_val)
	       || is_prev_youngergen_card_val(entry_val)) {
      // Mark it as both cur and prev youngergen; card cleaning thread will
      // eventually remove the previous stuff.
      jbyte new_val = cur_youngergen_and_prev_nonclean_card;
      jbyte res = atomic::compare_and_exchange_byte(new_val, entry,
						    entry_val);
      // Did the CAS succeed?
      if (res == entry_val) return;
      // Otherwise, retry, to see the new value.
      continue;
    } else {
      assert(entry_val == cur_youngergen_and_prev_nonclean_card
	     || entry_val == cur_youngergen_card_val(),
	     "should be only possibilities.");
      return;
    }
  } while (true);
}

void CardTableRS::younger_refs_in_space_iterate(Space* sp, 
						OopsInGenClosure* cl) {
  DirtyCardToOopClosure* dcto_cl = sp->new_dcto_cl(cl, _ct_bs.precision(),
                                                   cl->gen_boundary());
  ClearNoncleanCardWrapper clear_cl(dcto_cl, this);

  _ct_bs.non_clean_card_iterate(sp, sp->used_region_at_save_marks(),
                                dcto_cl, &clear_cl, false);
}

void CardTableRS::clear_MemRegion(MemRegion mr) {
  jbyte* cur  = byte_for(mr.start());
  jbyte* last = byte_after(mr.last());
  assert(addr_for(cur) == mr.start(), "region must be card aligned");
  while (cur < last) {
    *cur = CardTableModRefBS::clean_card;
    cur++;
  }
}

void CardTableRS::clear_into_gen_and_younger(Generation* gen) {
  GenCollectedHeap* gch = GenCollectedHeap::heap();
  // Gen and all younger have been evacuated. We can clear
  // remembered set entries for the next highest generation
  // (we know that it has no pointers to younger gens) and
  // those below.
  Generation* g = gch->next_gen(gen);
  while (g != NULL) {
    clear(g->reserved());
    g = gch->prev_gen(gen);
  }
}

#ifndef PRODUCT

class VerifyCleanCardClosure: public OopClosure {
  HeapWord* boundary;
  HeapWord* begin; HeapWord* end;
public:
  void do_oop(oop* p) {
    HeapWord* jp = (HeapWord*)p;
    if (jp >= begin && jp < end) {
      guarantee(*p == NULL || (HeapWord*)p < boundary
		|| (HeapWord*)(*p) >= boundary,
		"pointer on clean card crosses boundary");
    }
  }
  VerifyCleanCardClosure(HeapWord* b, HeapWord* _begin, HeapWord* _end) :
    boundary(b), begin(_begin), end(_end) {}
};

class VerifyCTSpaceClosure: public SpaceClosure {
  CardTableRS* _ct;
  HeapWord* _boundary;
public:
  VerifyCTSpaceClosure(CardTableRS* ct, HeapWord* boundary) :
    _ct(ct), _boundary(boundary) {}
  void do_space(Space* s) { _ct->verify_space(s, _boundary); }
};

class VerifyCTGenClosure: public GenCollectedHeap::GenClosure {
  CardTableRS* _ct;
public:
  VerifyCTGenClosure(CardTableRS* ct) : _ct(ct) {}
  void do_generation(Generation* gen) {
    // Skip the youngest generation.
    if (gen->level() == 0) return;
    // Normally, we're interested in pointers to younger generations.
    VerifyCTSpaceClosure blk(_ct, gen->reserved().start());
    gen->space_iterate(&blk, true);
  }
};

void CardTableRS::verify_space(Space* s, HeapWord* gen_boundary) {
  // We don't need to do young-gen spaces.
  if (s->end() <= gen_boundary) return;
  MemRegion used = s->used_region();

  jbyte* cur_entry = byte_for(used.start());
  jbyte* limit = byte_after(used.last());
  while (cur_entry < limit) {
    if (*cur_entry == CardTableModRefBS::clean_card) {
      jbyte* first_dirty = cur_entry+1;
      while (first_dirty < limit &&
	     *first_dirty == CardTableModRefBS::clean_card)
	first_dirty++;
      // If the first object is a regular object, and it has a
      // young-to-old field, that would mark the previous card.
      HeapWord* boundary = addr_for(cur_entry);
      HeapWord* end = addr_for(first_dirty);
      HeapWord* boundary_block = s->block_start(boundary);
      HeapWord* begin = boundary;             // Until proven otherwise.
      HeapWord* start_block = boundary_block; // Until proven otherwise.
      if (boundary_block < boundary) {
	if (s->block_is_obj(boundary_block)) {
	  oop boundary_obj = oop(boundary_block);
	  if (!boundary_obj->is_objArray() &&
	      !boundary_obj->is_typeArray()) {
	    guarantee(cur_entry > byte_for(used.start()),
		      "else boundary would be boundary_block");
	    if (*byte_for(boundary_block) != CardTableModRefBS::clean_card) {
	      begin = boundary_block + s->block_size(boundary_block);
	      start_block = begin;
	    }
	  }
	}
      }
      // Now traverse objects until end.
      HeapWord* cur = start_block;
      VerifyCleanCardClosure verify_blk(gen_boundary, begin, end);
      while (cur < end) {
	if (s->block_is_obj(cur)) {
	  oop(cur)->oop_iterate(&verify_blk);
	}
	cur += s->block_size(cur);
      }
      cur_entry = first_dirty;
    } else {
      guarantee(*cur_entry != cur_youngergen_and_prev_nonclean_card,
		"Illegal CT value");
      // If we're in the parallel case, the cur and prev values are
      // different, and we can't have left a prev in the table.
      guarantee(cur_youngergen_card_val() == youngergen_card
		|| !is_prev_youngergen_card_val(*cur_entry),
		"Illegal CT value");
      cur_entry++;
    }
  }
}

void CardTableRS::verify() {
  // At present, we only know how to verify the card table RS for
  // generational heaps.
  VerifyCTGenClosure blk(this);
  CollectedHeap* ch = Universe::heap();
  // We will do the perm-gen portion of the card table, too.
  Generation* pg = SharedHeap::heap()->perm_gen();
  HeapWord* pg_boundary = pg->reserved().start();

  if (ch->kind() == CollectedHeap::GenCollectedHeap) {
    GenCollectedHeap::heap()->generation_iterate(&blk, false);
    _ct_bs.verify();

    // If the old gen collections also collect perm, then we are only
    // interested in perm-to-young pointers, not perm-to-old pointers.
    GenCollectedHeap* gch = GenCollectedHeap::heap();
    CollectorPolicy* cp = gch->collector_policy();
    if (cp->is_mark_sweep_policy() || cp->is_concurrent_mark_sweep_policy()) {
      pg_boundary = gch->get_gen(1)->reserved().start();
    }
  }
  VerifyCTSpaceClosure perm_space_blk(this, pg_boundary);
  SharedHeap::heap()->perm_gen()->space_iterate(&perm_space_blk, true);
}
#endif
