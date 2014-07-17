#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)referenceProcessor.hpp	1.12 03/01/23 12:09:50 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// ReferenceProcessor class encapsulates the per-"collector" processing
// of "weak" references for GC. The interface is useful for supporting
// a generational abstraction, in particular when there are multiple
// generations that are being independently collected -- possibly
// concurrently and/or incrementally.  Note, however, that the
// ReferenceProcessor class abstracts away from a generational setting
// by using only a heap interval (called "span" below), thus allowing
// its use in a straightforward manner in a general, non-generational
// setting.
//
// The basic idea is that each ReferenceProcessor object concerns
// itself with ("weak") reference processing in a specific "span"
// of the heap of interest to a specific collector. Currently,
// the span is a convex interval of the heap, but, efficiency
// apart, there seems to be no reason it couldn't be extended
// (with appropriate modifications) to any "non-convex interval".

// forward references
class ReferencePolicy;

class ReferenceProcessor : public CHeapObj {
  // End of list marker
  static oop  _sentinelRef;
  MemRegion   _span; // (right-open) interval of heap
                     // subject to wkref discovery
  oop         _discoveredSoftRefs;
  oop         _discoveredWeakRefs;
  oop         _discoveredFinalRefs;
  oop         _discoveredPhantomRefs;
  bool        _discovering_refs;      // true when discovery enabled
  bool        _discovery_is_atomic;   // if discovery is atomic wrt
				      // other collectors in configuration
  bool        _discovery_is_mt;       // true if reference queueing is MT.
  bool        _notify_ref_lock;       // used only when !_discovery_is_atomic
  bool	      _enqueuing_is_done;    // true if all weak references enqueued

 private:
  // Process references with a certain reachability level.
  void process_discovered_reflist(oop* refs_list_addr,
                                  ReferencePolicy *policy,
                                  bool clear_referent,
                                  BoolObjectClosure* is_alive,
                                  OopClosure* keep_alive,
                                  VoidClosure* complete_gc);

  // Enqueue references with a certain reachability level
  void enqueue_discovered_reflist(oop refs_list,
                                  oop* pending_list_addr);

  void verify_ok_to_handle_reflists() PRODUCT_RETURN;

 public:
  // constructor
  ReferenceProcessor() :
   _span((HeapWord*)NULL, (HeapWord*)NULL),
   _discoveredSoftRefs(NULL),  _discoveredWeakRefs(NULL),
   _discoveredFinalRefs(NULL), _discoveredPhantomRefs(NULL),
   _discovering_refs(false),
   _discovery_is_atomic(true),
   _enqueuing_is_done(false),
   _discovery_is_mt(false),
    _notify_ref_lock(false)
  {}

  // RefDiscoveryPolicy values
  enum {
    ReferenceBasedDiscovery = 0,
    ReferentBasedDiscovery  = 1
  };

  static void init_statics();

  void initialize(bool atomic_discovery, bool mt_discovery);

  // get and set span
  MemRegion span()                   { return _span; }
  void      set_span(MemRegion span) { _span = span; }

  // start and stop weak ref discovery
  void enable_discovery()   { _discovering_refs = true;  }
  void disable_discovery()  { _discovering_refs = false; }
  bool discovery_enabled()  { return _discovering_refs;  }

  // whether discovery is atomic wrt other collectors
  bool discovery_is_atomic() const { return _discovery_is_atomic; }
  // whether discovery is done by multiple threads same-old-timeously
  bool discovery_is_mt() const { return _discovery_is_mt; }
  void set_mt_discovery(bool mt) { _discovery_is_mt = mt; }

  // whether all enqueuing of weak references is complete
  bool enqueuing_is_done()  { return _enqueuing_is_done; }
  void set_enqueuing_is_done(bool v) { _enqueuing_is_done = v; }

  // In the case of non-atomic discovery we need a way of
  // exchanging info regarding notification on the ref lock (PLL).
  bool notify_ref_lock()           { return _notify_ref_lock; }
  bool read_and_reset_notify_ref_lock() {
    assert(!discovery_is_atomic(), "Else why use this method?");
    if (_notify_ref_lock) {
      _notify_ref_lock = false;
      return true;
    }
    return false;
  }
  void set_notify_ref_lock(bool b) { _notify_ref_lock = b; }

  // iterate over oops
  void oops_do(OopClosure* f);
  static void oops_do_statics(OopClosure* f);

  // Check whether reference obj should be enqueued
  bool record_and_enqueue_reference(oop obj, ReferenceType rt);

  // Process references found during GC (called by the garbage collector)
  void process_discovered_references(ReferencePolicy *policy,
                                     BoolObjectClosure* is_alive,
                                     OopClosure* keep_alive,
                                     VoidClosure* complete_gc);

  // Enqueue references at end of GC (called by the garbage collector)
  bool enqueue_discovered_references();

  // debugging
  void verify_no_references_recorded() PRODUCT_RETURN;
};


// A utility class to disable reference discovery in
// the scope which contains it, for given ReferenceProcessor.
class NoRefDiscovery: StackObj {
 private:
  ReferenceProcessor* _rp;
  bool _was_discovering_refs;
 public:
  NoRefDiscovery(ReferenceProcessor* rp) : _rp(rp) {
    if (_was_discovering_refs = _rp->discovery_enabled()) {
      _rp->disable_discovery();
    }
  }

  ~NoRefDiscovery() {
    if (_was_discovering_refs) {
      _rp->enable_discovery();
    }
  }
};


// A utility class to temporarily mutate the span of the
// given ReferenceProcessor in the scope that contains it.
class ReferenceProcessorSpanMutator: StackObj {
 private:
  ReferenceProcessor* _rp;
  MemRegion           _saved_span;

 public:
  ReferenceProcessorSpanMutator(ReferenceProcessor* rp,
                                MemRegion span):
    _rp(rp) {
    _saved_span = _rp->span();
    _rp->set_span(span);
  }

  ~ReferenceProcessorSpanMutator() {
    _rp->set_span(_saved_span);
  }
};

// A utility class to temporarily change the MT'ness of
// reference discovery for the given ReferenceProcessor
// in the scope that contains it.
class ReferenceProcessorMTMutator: StackObj {
 private:
  ReferenceProcessor* _rp;
  bool                _saved_mt;

 public:
  ReferenceProcessorMTMutator(ReferenceProcessor* rp,
                                bool mt):
    _rp(rp) {
    _saved_mt = _rp->discovery_is_mt();
    _rp->set_mt_discovery(mt);
  }

  ~ReferenceProcessorMTMutator() {
    _rp->set_mt_discovery(_saved_mt);
  }
};
