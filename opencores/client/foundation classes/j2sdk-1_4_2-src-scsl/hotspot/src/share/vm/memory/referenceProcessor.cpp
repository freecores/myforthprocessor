#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)referenceProcessor.cpp	1.13 03/01/23 12:09:48 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_referenceProcessor.cpp.incl"

oop  ReferenceProcessor::_sentinelRef = NULL;

void referenceProcessor_init() {
  ReferenceProcessor::init_statics();
}

void ReferenceProcessor::init_statics() {
  assert(_sentinelRef == NULL, "should be initialized precsiely once");
  EXCEPTION_MARK;
  _sentinelRef = instanceKlass::cast(
                   SystemDictionary::object_klass())->
                     allocate_permanent_instance(THREAD);

  // Initialize the master soft ref clock.
  java_lang_ref_SoftReference::set_clock(os::javaTimeMillis());

  if (HAS_PENDING_EXCEPTION) {
      Handle ex(THREAD, PENDING_EXCEPTION);
      vm_exit_during_initialization(ex);
  }
  assert(_sentinelRef != NULL && _sentinelRef->is_oop(),
         "Just constructed it!");
  guarantee(RefDiscoveryPolicy == ReferenceBasedDiscovery ||
            RefDiscoveryPolicy == ReferentBasedDiscovery,
            "Unrecongnized RefDiscoveryPolicy");
}

void ReferenceProcessor::initialize(bool atomic_discovery, bool mt_discovery) {
  _discovering_refs = false;
  assert(_sentinelRef != NULL, "_sentinelRef is NULL");
  _discoveredSoftRefs    = _sentinelRef;
  _discoveredWeakRefs    = _sentinelRef;
  _discoveredFinalRefs   = _sentinelRef;
  _discoveredPhantomRefs = _sentinelRef;
  _discovery_is_atomic   = atomic_discovery;
  _discovery_is_mt       = mt_discovery;
}

#ifndef PRODUCT
void ReferenceProcessor::verify_no_references_recorded() {
  guarantee(
    _discoveredSoftRefs    == _sentinelRef &&
    _discoveredWeakRefs    == _sentinelRef &&
    _discoveredFinalRefs   == _sentinelRef &&
    _discoveredPhantomRefs == _sentinelRef &&
    _discovering_refs      == false,
    "verify_no_references_recorded failed");
}
#endif

void ReferenceProcessor::oops_do(OopClosure* f) {
  f->do_oop((oop*)&_discoveredSoftRefs);
  f->do_oop((oop*)&_discoveredWeakRefs);
  f->do_oop((oop*)&_discoveredFinalRefs);
  f->do_oop((oop*)&_discoveredPhantomRefs);
}

void ReferenceProcessor::oops_do_statics(OopClosure* f) {
  f->do_oop((oop*)&_sentinelRef);
}

void
ReferenceProcessor::process_discovered_references(ReferencePolicy *policy,
                                           BoolObjectClosure* is_alive,
                                           OopClosure* keep_alive,
                                           VoidClosure* complete_gc) {
  NOT_PRODUCT(verify_ok_to_handle_reflists());

  assert(!enqueuing_is_done(), "If here enqueuing should not be complete");
  // Stop treating discovered references specially.
  disable_discovery();

  // Soft references
  if (TraceReferenceGC) {
    gclog_or_tty->print_cr("Processing discovered soft references");
  }
  process_discovered_reflist(&_discoveredSoftRefs, policy, true, is_alive,
                             keep_alive, complete_gc);

  {
    // Update (advance) the soft ref master clock field. This must be done
    // after processing the soft ref list.
    jlong now = os::javaTimeMillis();
    jlong clock = java_lang_ref_SoftReference::clock();
    NOT_PRODUCT(
      if (now < clock) {
        warning("time warp: %d to %d", clock, now);
      }
    )
    // In product mode, protect ourselves from system time being adjusted
    // externally and going backward; see note in the implementation of
    // GenCollectedHeap::time_since_last_gc() for the right way to fix
    // this uniformly throughout the VM; see bug-id 4741166. XXX
    if (now > clock) {
      java_lang_ref_SoftReference::set_clock(now);
    }
    // Else leave clock stalled at its old value until time progresses
    // past clock value.
  }
  
  // Weak references
  if (TraceReferenceGC) {
    gclog_or_tty->print_cr("Processing discovered weak references");
  }
  process_discovered_reflist(&_discoveredWeakRefs, NULL, true, is_alive,
                             keep_alive, complete_gc);

  // Final references
  if (TraceReferenceGC) {
    gclog_or_tty->print_cr("Processing discovered final references");
  }
  process_discovered_reflist(&_discoveredFinalRefs, NULL, false, is_alive,
                             keep_alive, complete_gc);

  // Phantom references
  if (TraceReferenceGC) {
    gclog_or_tty->print_cr("Processing discovered phantom references");
  }
  process_discovered_reflist(&_discoveredPhantomRefs, NULL, false, is_alive,
                             keep_alive, complete_gc);

  // Weak global JNI references. It would make more sense (semantically) to
  // traverse these simultaneously with the regular weak references above, but
  // that is not how the JDK1.2 specification is. See #4126360. Native code can
  // thus use JNI weak references to circumvent the phantom references and
  // resurrect a "post-mortem" object.
  if (TraceReferenceGC) {
    gclog_or_tty->print_cr("Processing JNI weak references");
  }
  JNIHandles::weak_oops_do(is_alive, keep_alive);

  // Finally remember to keep sentinel around
  keep_alive->do_oop(&_sentinelRef);
  complete_gc->do_void();
};

bool ReferenceProcessor::enqueue_discovered_references() {
  NOT_PRODUCT(verify_ok_to_handle_reflists());
  // Remember old value of pending references list
  oop* pending_list_addr = java_lang_ref_Reference::pending_list_addr();
  oop old_pending_list_value = *pending_list_addr;

  // Enqueue references that are not made active again
  enqueue_discovered_reflist(_discoveredSoftRefs, pending_list_addr);
  enqueue_discovered_reflist(_discoveredWeakRefs, pending_list_addr);
  enqueue_discovered_reflist(_discoveredFinalRefs, pending_list_addr);
  enqueue_discovered_reflist(_discoveredPhantomRefs, pending_list_addr);

  // Clear lists for next GC
  _discoveredSoftRefs    = _sentinelRef;
  _discoveredWeakRefs    = _sentinelRef;
  _discoveredFinalRefs   = _sentinelRef;
  _discoveredPhantomRefs = _sentinelRef;

  // Stop treating discovered references specially.
  disable_discovery();

  // Return true if new pending references were added
  return old_pending_list_value != *pending_list_addr;
};

void
ReferenceProcessor::process_discovered_reflist(oop* refs_list_addr,
			                ReferencePolicy *policy,
				        bool clear_referent,
				        BoolObjectClosure* is_alive,
				        OopClosure* keep_alive,
				        VoidClosure* complete_gc) {
  debug_only(oop first_seen = NULL;) // Used for cyclic linked list check
    
  bool kept_some_alive = false;
  oop* prev_next = refs_list_addr;
  oop obj = *refs_list_addr;
  debug_only(first_seen = obj;) // cyclic linked list check

  // (SoftReferences only) Traverse the list and remove any SoftReferences whose
  // referents are not alive, but that should be kept alive for policy reasons.
  // Keep alive the transitive closure of all such referents.
  if (policy != NULL) {
    // Decide which softly reachable refs should be kept alive.
    while (obj != _sentinelRef) {
      oop* discovered_addr = java_lang_ref_Reference::discovered_addr(obj);
      assert(discovered_addr || (*discovered_addr)->is_oop_or_null(),
        "discovered field is bad");
      oop next = *discovered_addr;
      oop* referent_addr = java_lang_ref_Reference::referent_addr(obj);
      oop referent = *referent_addr;
      assert(Universe::heap()->is_in_or_null(referent),
	"Wrong oop found in java.lang.Reference object");
      bool referent_is_dead = !is_alive->do_object_b(referent);
      if (referent_is_dead && !policy->should_clear_reference(obj)) {
        // Make the Reference object active again
        java_lang_ref_Reference::set_next(obj, NULL);
        keep_alive->do_oop(referent_addr);    // keep the referent around
        kept_some_alive = true; 
        if (TraceReferenceGC) {
          gclog_or_tty->print_cr("Dropping reference (0x%x) by policy", obj);
        }
        // Remove Reference object from list
	*discovered_addr = NULL;
        *prev_next = next;
      } else {
        prev_next = discovered_addr;
      }
      obj = next;
      assert(obj != first_seen, "cyclic ref_list found");
    }
    if (kept_some_alive) {
      complete_gc->do_void();
      kept_some_alive = false;
    }
    // Prepare for next pass.
    prev_next = refs_list_addr;
    obj = *refs_list_addr;
  }

  // Traverse the list and remove any refs whose referents are alive.
  while (obj != _sentinelRef) {
    oop* discovered_addr = java_lang_ref_Reference::discovered_addr(obj);
    assert(discovered_addr || (*discovered_addr)->is_oop_or_null(),
      "discovered field is bad");
    oop next = *discovered_addr;
    oop* referent_addr = java_lang_ref_Reference::referent_addr(obj);
    oop referent = *referent_addr;
    assert(Universe::heap()->is_in_or_null(referent),
      "Wrong oop found in java.lang.Reference object");
    if (is_alive->do_object_b(referent)) {
      // The referent is reachable after all, make the Reference object active again
      java_lang_ref_Reference::set_next(obj, NULL);
      keep_alive->do_oop(referent_addr);    // keep the referent around
      // Note that we do not have to set kept_some_alive to true here, since the
      // referent must already be traversed
      if (TraceReferenceGC) {
      gclog_or_tty->print_cr("Dropping strongly reachable reference (" INTPTR_FORMAT ")", obj);
      }
      // Remove Reference object from list
      *prev_next = next;
      // Clear the discovered_addr field so that the object doe
      // not look like it has been discovered.
      *discovered_addr = NULL;
    } else {
      prev_next = discovered_addr;
    }
    obj = next;
    assert(obj != first_seen, "cyclic ref_list found");
  }

  // Now traverse list and process list pointers themselves
  prev_next = refs_list_addr;
  obj = *refs_list_addr;
  debug_only(first_seen = obj;) // cyclic linked list check
  while (obj != _sentinelRef) {
    keep_alive->do_oop(prev_next);
    prev_next = java_lang_ref_Reference::discovered_addr(obj);
    assert(prev_next || (*prev_next)->is_oop_or_null(),
      "discovered field is bad");
    oop* referent_addr = java_lang_ref_Reference::referent_addr(obj);
    oop referent = *referent_addr;
    assert(Universe::heap()->is_in_or_null(referent),
           "Wrong oop found in java.lang.Reference object");
    if (clear_referent) {
      // NULL out referent pointer
      *referent_addr = NULL;
    } else {
      // keep the referent around
      keep_alive->do_oop(referent_addr);
      kept_some_alive = true;
    }
    if (TraceReferenceGC) {
      gclog_or_tty->print_cr("Adding %sreference (" INTPTR_FORMAT ") as pending", 
        clear_referent ? "cleared " : "", obj);
    }
    assert(obj->is_oop(), "Adding a bad reference");
    assert(referent->is_oop(), "Adding a bad referent");
    obj = *prev_next;
    assert(obj != first_seen, "cyclic ref_list found");
  }
  // Remember to keep sentinel pointer around
  keep_alive->do_oop(prev_next);
  if (kept_some_alive) {
    complete_gc->do_void();
  }
}

void ReferenceProcessor::enqueue_discovered_reflist(oop refs_list,
  oop* pending_list_addr) {
  // Move objects from a discovered list to the pending list.  The
  // discovered list uses the "discovered" field in a java.lang.ref.Reference
  // whereas the pending list uses the "next" field.
#ifdef ASSERT
  if(TraceReferenceGC && PrintGCDetails) {
    gclog_or_tty->print_cr("instanceRefKlass::enqueue_discovered_reflist list " INTPTR_FORMAT, 
      refs_list);
  }
#endif
  oop obj = refs_list;
  debug_only(oop prev_obj = NULL);
  while (obj != _sentinelRef) {
    assert(obj->is_instanceRef(), "should be reference object");
    oop next = java_lang_ref_Reference::discovered(obj);
#ifdef ASSERT
    if(TraceReferenceGC && PrintGCDetails) {
      gclog_or_tty->print_cr("	obj " INTPTR_FORMAT "/next " INTPTR_FORMAT, obj, next);
    }
#endif
    // Hook up to pending list
    oop pending_list = *pending_list_addr;
    if (pending_list == NULL) {
      // Make "self-reference" end of list marker
      pending_list = obj;
    }
    assert(*java_lang_ref_Reference::next_addr(obj) == NULL, 
      "The reference should not be enqueued");
    java_lang_ref_Reference::set_next(obj, pending_list);
    oop_store(pending_list_addr, obj);
    // Should be finished with the discovered list at this point.
    java_lang_ref_Reference::set_discovered(obj, (oop) NULL);
    debug_only(prev_obj = obj);
    obj = next;
  }
}

// We mention two of several possible choices here:
// #0: if the reference object is not in the "originating generation"
//     (or part of the heap being collected, indicated by our "span"
//     we don't treat it specially (i.e. we scan it as we would
//     a normal oop, treating its references as strong references).
//     This means that references can't be enqueued unless their
//     referent is also in the same span. This is the simplest,
//     most "local" and most conservative approach, albeit one
//     that may cause weak references to be enqueued least promptly.
//     We call this choice the "ReferenceBasedDiscovery" policy.
// #1: the reference object may be in any generation (span), but if
//     the referent is in the generation (span) being currently collected
//     then we can discover the reference object, provided
//     the object has not already been discovered by
//     a different concurrently running collector (as may be the
//     case, for instance, if the reference object is in CMS and
//     the referent in DefNewGeneration), and provided the processing
//     of this reference object by the current collector will
//     appear atomic to every other collector in the system.
//     (Thus, for instance, a concurrent collector may not
//     discover references in other generations even if the
//     referent is in its own generation). This policy may,
//     in certain cases, enqueue references somewhat sooner than
//     might Policy #0 above, but at marginally increased cost
//     and complexity in processing these references.
//     We call this choice the "RefeferentBasedDiscovery" policy.
bool ReferenceProcessor::record_and_enqueue_reference(oop obj,
                                                      ReferenceType rt) {
  // We enqueue references only if we are discovering refs
  // (rather than processing discovered refs).
  if (!_discovering_refs || !RegisterReferences) {
    return false;
  }
  // We only enqueue active references.
  oop* next_addr = java_lang_ref_Reference::next_addr(obj);
  if (*next_addr != NULL) {
    return false;
  }

  HeapWord* obj_addr = (HeapWord*)obj;
  if (RefDiscoveryPolicy == ReferenceBasedDiscovery &&
      !_span.contains(obj_addr)) {
    // Reference is not in the originating generation;
    // don't treat it specially (i.e. we want to scan it as a normal
    // object with strong references).
    return false;
  }

  oop* discovered_addr = java_lang_ref_Reference::discovered_addr(obj);
  assert(discovered_addr != NULL && (*discovered_addr)->is_oop_or_null(),
         "bad discovered field");
  if (*discovered_addr != NULL) {
    // The reference has already been discovered...
    if (TraceReferenceGC) {
      gclog_or_tty->print_cr("Already enqueued reference (" INTPTR_FORMAT ")", obj);
    }
    if (RefDiscoveryPolicy == ReferentBasedDiscovery) {
      // assumes that an object is not processed twice;
      // if it's been already discovered it must be on another
      // generation's discovered list; so we won't discover it.
      return false;
    } else {
      assert(RefDiscoveryPolicy == ReferenceBasedDiscovery,
             "Unrecognized policy");
      // check assumption that an object is not potentially
      // discovered twice except by collectors whose discovery is
      // not atomic wrt other collectors in the configuration (for
      // instance, a concurrent collector).
      assert(!discovery_is_atomic(),
             "Only non-atomic collectors are allowed to even "
             "attempt rediscovery in their own span");
      return true;
    }
  }

  if (RefDiscoveryPolicy == ReferentBasedDiscovery) {
    oop referent = java_lang_ref_Reference::referent(obj);
    assert(referent->is_oop(), "bad referent");
    // enqueue if and only if either:
    // reference is in our span or
    // we are an atomic collector and referent is in our span
    if (_span.contains(obj_addr) ||
        (discovery_is_atomic() && _span.contains(referent))) {
      // should_enqueue = true;
    } else {
      return false;
    }
  } else {
    assert(RefDiscoveryPolicy == ReferenceBasedDiscovery &&
           _span.contains(obj_addr), "code inconsistency");
  }
  // Get the discovered queue to which we will add
  oop* list;
  switch (rt) {
    case REF_OTHER:
      return false;   // Unknown reference type, no special treatment
    case REF_SOFT:
      list = &_discoveredSoftRefs;
      break;
    case REF_WEAK:
      list = &_discoveredWeakRefs;
      break;
    case REF_FINAL:
      list = &_discoveredFinalRefs;
      break;
    case REF_PHANTOM:
      list = &_discoveredPhantomRefs;
      break;
    case REF_NONE:
      // we should not reach here if we are an instanceRefKlass
    default:
      ShouldNotReachHere();
  }
  // link reference object to appropriate list
  assert(*discovered_addr == NULL || _discovery_is_mt,
         "tautology -- we eliminated the already-discovered case above");
  assert(*list != NULL, "cannot set next to NULL -- will look "
                        "like non-enqueued reference");

  // We do a raw store here, the field will be visited later when
  // processing the discovered references. 
  // It may be simpler to just always do the CAS, but this is safer for now.
  if (_discovery_is_mt) {
    // First we must make sure this object is only enqueued once. CAS in a non null
    // discovered_addr.
    oop retest = (oop)atomic::compare_and_exchange_ptr((intptr_t)*list,
                                                       (intptr_t*)discovered_addr,
                                                       (intptr_t)NULL);
    if (retest == NULL) {
      oop current_head;
      do {
        current_head = *list;
        *discovered_addr = current_head;
      } while (atomic::compare_and_exchange_ptr((intptr_t)obj,
                                                (intptr_t*)list,
                                                (intptr_t)current_head) != (intptr_t)current_head);
    } else {
      // If retest was non NULL, another thread already
      // enqueued the object. Do the right thing.
      
      // The reference has already been discovered...
      if (TraceReferenceGC) {
        gclog_or_tty->print_cr("Already enqueued reference (" INTPTR_FORMAT ")", obj);
      }
      if (RefDiscoveryPolicy == ReferentBasedDiscovery) {
        // assumes that an object is not processed twice;
        // if it's been already discovered it must be on another
        // generation's discovered list; so we won't discover it.
        return false;
      } else {
        assert(RefDiscoveryPolicy == ReferenceBasedDiscovery,
               "Unrecognized policy");
        // check assumption that an object is not potentially
        // discovered twice except by collectors whose discovery is
        // not atomic wrt other collectors in the configuration (for
        // instance, a concurrent collector).
        assert(!discovery_is_atomic(),
               "Only non-atomic collectors are allowed to even "
               "attempt rediscovery in their own span");
        return true;
      }
    }
  } else {
    *discovered_addr = *list;
    *list = obj;
  }

  if (TraceReferenceGC) {
    oop referent = java_lang_ref_Reference::referent(obj);
    if(PrintGCDetails) {
      gclog_or_tty->print_cr("Enqueued reference (" INTPTR_FORMAT ")", obj);
    }
    assert(referent->is_oop(), "Enqueuing a bad referent");
  }
  assert(obj->is_oop(), "Enqueuing a bad reference");
  return true;
}

#ifndef PRODUCT
void ReferenceProcessor::verify_ok_to_handle_reflists() {
  // empty for now
}
#endif PRODUCT
