#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)onStackReplacement.cpp	1.35 03/01/23 12:00:06 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_onStackReplacement.cpp.incl"


#ifdef COMPILER2
// Implementation of OSRAdapter

OSRAdapter::OSRAdapter(CodeBuffer *cb, OopMapSet *oop_maps, int size, int frame_size)
  :CodeBlob(cb,                  // Code buffer
            sizeof(OSRAdapter),  // Header size            
            size,                // Size
            frame_size,          // Frame size
            oop_maps) {
}


OSRAdapter* OSRAdapter::new_osr_adapter(CodeBuffer* cb,	OopMapSet *oop_maps, int frame_size) {
  unsigned int size = allocation_size(cb, sizeof(OSRAdapter));
  OSRAdapter* osr_adapter = NULL;
  {
    MutexLockerEx mu(CodeCache_lock, Mutex::_no_safepoint_check_flag);
    osr_adapter = new (size) OSRAdapter(cb, oop_maps, size, frame_size);
  }

  // We do not need to hold the CodeCache lock during name formatting.
  if (osr_adapter != NULL) {
    char blob_id[256];
    jio_snprintf(blob_id, sizeof(blob_id), "OSRAdapter@" PTR_FORMAT, osr_adapter->instructions_begin());
    VTune::register_stub(blob_id, osr_adapter->instructions_begin(), osr_adapter->instructions_end());
  }

  return osr_adapter;
}


void* OSRAdapter::operator new(size_t s, unsigned size) {
  void* p = CodeCache::allocate(size);  
  return p;
}


#ifndef PRODUCT
void OSRAdapter::verify() {
  // Nothing to verify for now
}


void OSRAdapter::print() const {
  tty->print_cr("[OSRAdapter. Framesize: %d]", frame_size());
}


void OSRAdapter::print_value_on(outputStream* st) const {
  tty->print_cr("[OSRAdapter]");
}
#endif // PRODUCT
#endif // COMPILER2


// Implementation of OnStackReplacement

#ifdef COMPILER2
GrowableArray<OSRAdapter*>* OnStackReplacement::_osr_adapters_other;
GrowableArray<OSRAdapter*>* OnStackReplacement::_osr_adapters_fp;

void OnStackReplacement::initialize() {
  if (!UseOnStackReplacement) return;
  // Initialize vector of OSRAdapter. The common used sizes gets precomputed
  // to limit fragmentation of the code cache.
  _osr_adapters_other = new(ResourceObj::C_HEAP)GrowableArray<OSRAdapter*>(InitialAdapterVectorSize, true);
  _osr_adapters_fp    = new(ResourceObj::C_HEAP)GrowableArray<OSRAdapter*>(InitialAdapterVectorSize, true);
  // Precompute common cases
  assert(MaxTypicalAdapterSize < InitialAdapterVectorSize, "wrong setup parameters");
  for(int framesize = MaxTypicalAdapterSize; framesize >= MinTypicalAdapterSize; framesize--) {
    OSRAdapter* osr = OptoRuntime::generate_osr_blob(framesize,false);
    assert(osr != NULL, "must generate an adapter blob");
    _osr_adapters_other->at_put_grow(framesize, osr);
  }
}


// The generator method is implemented in runtime_<cpu>.cpp
OSRAdapter* OnStackReplacement::get_osr_adapter(int framesize, bool returning_fp) {
  assert(UseOnStackReplacement, "on-stack replacement not used");

  // Adapters on Intel need special treatment of the FPU stack if they
  // return a float or double.
  GrowableArray<OSRAdapter*>*osr_ary = returning_fp 
    ? _osr_adapters_fp
    : _osr_adapters_other;

  OSRAdapter* osr = NULL;  
  { MutexLocker mu(AdapterCache_lock);
    // This will potentially grow the array. 
    osr = osr_ary->at_grow(framesize);
  }

  // Need to create OSR adapter and update list.
  if (osr == NULL) {
    osr = OptoRuntime::generate_osr_blob(framesize, returning_fp);
    if (osr == NULL) {
       //CodeCache is probably full
       return NULL;
    }

    OSRAdapter* check = NULL;
    { MutexLocker mu(AdapterCache_lock);
      // Grap lock and update cache. Check if someone else already updated the list
      check = osr_ary->at(framesize);
      if (check == NULL) {
        osr_ary->at_put(framesize, osr);    
      } 
    }

    // The adapter already existed
    if (check != NULL) {
      MutexLockerEx mu(CodeCache_lock, Mutex::_no_safepoint_check_flag);
      CodeCache::free(osr);     
      osr = check;                
    }      
  }
  return osr;
}
#endif // COMPILER2


void onStackReplacement_init() {
  // Note: C1 doesn't need any of the osr adapter machinery
  //       in this file as there is only one osr adapter and
  //       the corresponding code is generated via c1_Runtime1
  //       => no initialization code needed for now
#ifdef COMPILER2
  OnStackReplacement::initialize();
#endif // COMPILER2
}
