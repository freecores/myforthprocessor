#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)threadLocalAllocBuffer.hpp	1.17 03/01/23 12:10:38 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// ThreadLocalAllocBuffer: a descriptor for thread-local storage used by
// the threads for allocation.
//            It is thread-private at any time, but maybe multiplexed over
//            time across multiple threads. The park()/unpark() pair is
//            used to make it avaiable for such multiplexing.
class ThreadLocalAllocBuffer: public CHeapObj {
  friend class VMStructs;
private:
  Thread*   _thread;
  HeapWord* _start;
  HeapWord* _top;
  HeapWord* _end;
  size_t    _size;                               // full size (including padding)
  bool _has_terminated;                          // true if the host thread has terminated
       
  static size_t _log2_fragmentation_ratio;       // log2 of the portion of (alive) TLAB that can remain unused
  static size_t _max_size;                       // max allowed tlab size in words (used with ResizeTLAB)  


public:
  ThreadLocalAllocBuffer() {
    initialize(NULL,                    // thread
               NULL,                    // start
               NULL,                    // top
               NULL,                    // end
               0);                      // size
  }
  ThreadLocalAllocBuffer(Thread*   thread,
                         HeapWord* start,
                         HeapWord* top,
			 HeapWord* end,
                         size_t    size) {
    initialize(thread, start, top, end, size);
  }
  Thread* thread() const                         { return _thread; }
  void set_thread(Thread* thread)                { _thread = thread; }
  HeapWord* start() const                        { return _start; }
  void set_start(HeapWord* start)                { _start = start; }
  HeapWord* end() const                          { return _end; }
  void set_end(HeapWord* end)                    { _end = end; }
  HeapWord* top() const                          { return _top; }
  void set_top(HeapWord* top)                    { _top = top; }
  size_t size() const                            { return _size; }
  void set_size(size_t size)                     { _size = size; }
  bool has_terminated()                          { return _has_terminated; }
  void set_has_terminated(bool b)                { _has_terminated = b; }
  
  size_t available_size()                        {
    // in words
    return pointer_delta(_end, _top);
  }

  static size_t log2_fragmentation_ratio()       { return _log2_fragmentation_ratio; }
  static void set_log2_fragmentation_ratio(size_t ratio) { _log2_fragmentation_ratio = ratio; }
  static void initialize_fragmentation_ratio()   { _log2_fragmentation_ratio = exact_log2(TLABFragmentationRatio); }
  static size_t max_size()                       { return _max_size; }
  static void set_max_size(size_t size)          { _max_size = size; }
  static void adjust_max_size();

  void reset_original_size()                     { _size = TLABSize / HeapWordSize; }
  void initialize(Thread*   thread,
                  HeapWord* start,
                  HeapWord* top,
                  HeapWord* end,
                  size_t    size) {
    set_thread(thread);
    set_start(start);
    set_top(top);
    set_end(end);
    set_size(size);
    set_has_terminated(false);
    assert(invariants(), "corrupt TLAB");
    if (PrintTLAB) {
      tty->print("initialize ");
      print();
    }
  }
  void reset();
  ThreadLocalAllocBuffer* clone() {
    ThreadLocalAllocBuffer* tlab =
      new ThreadLocalAllocBuffer(_thread, _start, _top, _end, _size);
    tlab->set_has_terminated(true); // no currently associated thread
    return tlab;
  }
  bool invariants() const { return (top() >= start() && top() <= end()); }

  void print();

  // Code generation support
  static ByteSize start_offset()                 { return byte_offset_of(ThreadLocalAllocBuffer, _start); }
  static ByteSize end_offset()                   { return byte_offset_of(ThreadLocalAllocBuffer, _end  ); }
  static ByteSize top_offset()                   { return byte_offset_of(ThreadLocalAllocBuffer, _top  ); }
  static ByteSize size_offset()                  { return byte_offset_of(ThreadLocalAllocBuffer, _size ); }  

  void clear() {
    initialize(NULL,                    // thread
               NULL,                    // start
               NULL,                    // top
               NULL,                    // end
               0);                      // size
    reset_original_size();
  }

  inline size_t compute_new_size(size_t allocation_size);
  void verify();
};
