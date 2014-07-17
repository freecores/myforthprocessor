#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)threadLocalAllocBuffer.inline.hpp	1.16 03/01/23 12:10:40 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// note: allocation size includes all necessary padding
inline size_t ThreadLocalAllocBuffer::compute_new_size(size_t alloc_size) {
  // Make sure there's enough room for a filler int[].
  const size_t filler_size =
    (size_t) align_object_size(typeArrayOopDesc::header_size(T_INT));
  const size_t alloc_filler_size = MAX2(alloc_size, filler_size);
  // Compute the size for the new TLAB.
  size_t new_tlab_size;
  if (ResizeTLAB) {
    // Double the old size, or make it as big as possible.
    // I don't expect to get TLAB sizes (in words) within a factor of 2
    // of max_uintx, but I don't want to just blindly double my way
    // into (and over) it.
    size_t doubled_size;
    if (size() <= (max_uintx / 2)) {
      doubled_size = size() * 2;
    } else {
      doubled_size = max_uintx;
    }
    // We'd like to allocate the larger of the doubled size
    // or the allocation request, but we can't ask for more than
    // the maximum allocation allowed.
    const size_t desired_size = MAX2(doubled_size, alloc_filler_size);
    adjust_max_size();
    new_tlab_size = MIN2(desired_size, max_size());
  } else {
    new_tlab_size = TLABSize / HeapWordSize;
  }
  // The "last" tlab may be smaller to reduce fragmentation.
  // unsafe_max_alloc is just a hint.
  // XXX: shouldn't this be unsafe_max_tlab_alloc()???
  const size_t available_size = 
    Universe::heap()->unsafe_max_alloc() / HeapWordSize;
  new_tlab_size = MIN2(available_size, new_tlab_size);
  if (new_tlab_size < alloc_filler_size) {
    // If there isn't enough room for the allocation, return failure.
    if (PrintTLAB) {
      tty->print_cr("ThreadLocalAllocBuffer::compute_new_size(" SIZE_FORMAT ")"
                    " returns failure",
                    alloc_size);
    }
    return 0;
  }
  if (PrintTLAB) {
    tty->print_cr("ThreadLocalAllocBuffer::compute_new_size(" SIZE_FORMAT ")"
                  " returns " SIZE_FORMAT,
                  alloc_size, new_tlab_size);
  }
  return new_tlab_size;
}
