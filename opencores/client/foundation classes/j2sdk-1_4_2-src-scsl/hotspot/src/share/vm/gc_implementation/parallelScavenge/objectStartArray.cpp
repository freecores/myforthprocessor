#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)objectStartArray.cpp	1.5 03/01/23 12:02:07 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_objectStartArray.cpp.incl"

void ObjectStartArray::initialize(MemRegion mr) {
  // The region of memory we're providing start info for
  _covered_region = mr;

  // We're based on the assumption that we use the same
  // size blocks as the card table.
  assert(block_size == CardTableModRefBS::card_size, "Sanity");

  HeapWord* low_bound  = _covered_region.start();
  HeapWord* high_bound = _covered_region.end();
  assert((uintptr_t(low_bound)  & (block_size - 1))  == 0, "heap must start at block boundary");
  assert((uintptr_t(high_bound) & (block_size - 1))  == 0, "heap must end at block boundary");

  assert(block_size <= 512, "block_size must be less than 512");
  size_t heap_size_in_words = _covered_region.word_size();
  size_t raw_data_size = ReservedSpace::page_align_size_up((heap_size_in_words / 
                                                            block_size_in_words));

  // FIX ME! We should be using ReservedSpace and VirtualSpace here. Clean up.
  _raw_base = NEW_C_HEAP_ARRAY(jbyte, raw_data_size);
  if (_raw_base == NULL)
    vm_exit_during_initialization("Could not allocate object start array");

  _offset_base = _raw_base - (size_t(low_bound) >> block_shift);

  _blocks_region.set_start((HeapWord*)_raw_base);
  _blocks_region.set_word_size(raw_data_size / sizeof(HeapWord));

  assert(block_for_addr(low_bound) == &_raw_base[0], "Checking start of map");
  assert(block_for_addr(high_bound-1) <= &_raw_base[raw_data_size-1], "Checking end of map");

  reset();
}

// This initializes the ObjectStartArray with a preallocated backing store.
// If the base_size_in_bytes isn't large enough to cover the requested MemRegion,
// a fatal error will result. 
void ObjectStartArray::initialize_preallocated(MemRegion mr, jbyte *base,
                                               size_t base_size_in_bytes,
                                               bool should_clear) {
  guarantee(base != NULL, "Cannot initialize ObjectStartArray with null base");

  // The region of memory we're providing start info for
  _covered_region = mr;

  // We're based on the assumption that we use the same
  // size blocks as the card table.
  assert(block_size == CardTableModRefBS::card_size, "Sanity");

  HeapWord* low_bound  = _covered_region.start();
  HeapWord* high_bound = _covered_region.end();
  assert((uintptr_t(low_bound)  & (block_size - 1))  == 0, "heap must start at block boundary");
  assert((uintptr_t(high_bound) & (block_size - 1))  == 0, "heap must end at block boundary");

  assert(block_size <= 512, "block_size must be less than 512");
  size_t heap_size_in_words = _covered_region.word_size();
  size_t raw_data_size = ReservedSpace::page_align_size_up((heap_size_in_words / 
                                                            block_size_in_words));

  _raw_base = base;
  if ( raw_data_size > base_size_in_bytes ) {
    fatal("Unable to initialize ObjectStartArray");
  }

  _offset_base = _raw_base - (size_t(low_bound) >> block_shift);

  _blocks_region.set_start((HeapWord*)_raw_base);
  _blocks_region.set_word_size(raw_data_size / sizeof(HeapWord));

  assert(block_for_addr(low_bound) == &_raw_base[0], "Checking start of map");
  assert(block_for_addr(high_bound-1) <= &_raw_base[raw_data_size-1], "Checking end of map");

  if (should_clear)
    reset();
}

void ObjectStartArray::reset() {
  memset(_blocks_region.start(), clean_block, _blocks_region.byte_size());
}


