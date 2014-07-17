#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)parGCAllocBuffer.cpp	1.12 03/01/23 12:09:27 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_parGCAllocBuffer.cpp.incl"

ParGCAllocBuffer::ParGCAllocBuffer(size_t word_sz) :
  _word_sz(word_sz), _bottom(NULL), _top(NULL),
  _end(NULL), _hard_end(NULL),
  _retained(false), _retained_filler()
{}

const size_t ParGCAllocBuffer::FillerHeaderSize =
             align_object_size(arrayOopDesc::header_size(T_INT));

const size_t ParGCAllocBuffer::AlignmentReserve =
             ALIGN_ALL_OBJECTS ? 0 : FillerHeaderSize;

void ParGCAllocBuffer::retire(bool end_of_gc, bool retain) {
  assert(!retain || end_of_gc, "Can only retain at GC end.");
  if (_retained) {
    // If the buffer had been retained shorten the previous filler object.
    assert(_retained_filler.end() <= _top, "INVARIANT");
    SharedHeap::fill_region_with_object(_retained_filler);
    _retained = false;
  }
  assert(!end_of_gc || !_retained, "At this point, end_of_gc ==> !_retained.");
  if (_top < _hard_end) {
    SharedHeap::fill_region_with_object(MemRegion(_top, _hard_end));
    if (!retain) {
      invalidate();
    } else {
      // Is there wasted space we'd like to retain for the next GC?
      if (pointer_delta(_end, _top) > FillerHeaderSize) {
	_retained = true;
	_retained_filler = MemRegion(_top, FillerHeaderSize);
	_top = _top + FillerHeaderSize;
      } else {
        invalidate();
      }
    }
  }
}

#ifndef PRODUCT
void ParGCAllocBuffer::print() {
  tty->print("parGCAllocBuffer: _bottom: %p  _top: %p  _end: %p  _hard_end: %p"
             "_retained: %c _retained_filler: [%p,%p)\n",
             _bottom, _top, _end, _hard_end,
             "FT"[_retained], _retained_filler.start(), _retained_filler.end());
}
#endif // !PRODUCT
