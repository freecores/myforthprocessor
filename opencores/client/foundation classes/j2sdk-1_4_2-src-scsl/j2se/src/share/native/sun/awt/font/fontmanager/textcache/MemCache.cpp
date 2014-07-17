/*
 * @(#)MemCache.cpp	1.4 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "MemCache.h"

struct MemEntry {
  UInt32 fOwnerID; // id of owner of this memory
  void*  fMemPtr;  // memory owned
  UInt32 fMemSize; // size of memory allocated

  MemEntry() : fOwnerID(0), fMemPtr(0), fMemSize(0) {}

  ~MemEntry() { 
    assert(fMemPtr == 0);
    fMemPtr = 0; 
  }

  // dangerous calls, make sure you don't reset fMemPtr by accident
  void set(UInt32 ownerID, void* memPtr, UInt32 memSize) {
    fOwnerID = ownerID;
    fMemPtr = memPtr;
    fMemSize = memSize;
  }

  void move(MemEntry& rhs) {
    set(rhs.fOwnerID, rhs.fMemPtr, rhs.fMemSize);
    rhs.reset();
  }

  void reset() {
    set(0, 0, 0);
  }

private:
  MemEntry& operator=(MemEntry&) { return *this; }
};

// ---------------------------------------------------------------------------

const UInt16 MemCache::kNoIndex = 0xffffu;

MemCache::MemCache(UInt16 numEntries, UInt16 nearLimit, UInt32 maxMem, Boolean purgeOn)
  : fNumEntries(0)
  , fMaxEntries(numEntries)
  , fMem(0)
  , fMaxMem(maxMem)
  , fNearLimit(nearLimit)
  , fTop(numEntries - 1)
  , fEntries(new MemEntry[numEntries])
  , fPurge(purgeOn)
{
}

void 
MemCache::releaseEntry(MemEntry& e, Boolean* purged) {
  if (e.fMemPtr) {
    fNumEntries--; 
    fMem -= e.fMemSize;
    if (e.fMemPtr) {
      disposeMemPtr(e.fMemPtr);
    }
    if (purged) {
      *purged = true;
    }
    e.reset();
  }
} 

void 
MemCache::setEntry(MemEntry& e, UInt32 ownerID, void* memPtr, UInt32 memSize, Boolean* purged) {
  releaseEntry(e, purged);
  e.set(ownerID, memPtr, memSize);
  if (memPtr) {
    fNumEntries++;
    fMem += memSize;
  }
}

void 
MemCache::moveEntry(MemEntry& lhs, MemEntry& rhs, Boolean* purged) {
  if (&lhs != &rhs) {
    releaseEntry(lhs, purged);
    lhs.move(rhs);
  }
}

MemCache::~MemCache() {
  releaseAll();
  delete[] fEntries;
  fEntries = 0;
}

UInt16
MemCache::assignMemory(UInt32 ownerID, UInt32 memSize, void* mem, Boolean* purged) {
  if (mem) {
    // try to ensure that after allocation mem is still < maxmem
    if (fPurge && (fMem + memSize > fMaxMem)) {
      UInt16 bottom = fTop;
      UInt32 memLimit = fMaxMem > memSize ? fMaxMem - memSize : 0;
      while (fMem > memLimit && fNumEntries > 0) {
        bottom = next(bottom);
        releaseEntry(fEntries[bottom], purged); // adjusts fMem, fNumEntries
      }
    }

    fTop = next(fTop);
    if (!fPurge && fNumEntries < fMaxEntries) {
      while (fEntries[fTop].fMemPtr) {
        fTop = next(fTop);
      }
    }

    setEntry(fEntries[fTop], ownerID, mem, memSize, purged);

    return fTop;
  }

  return kNoIndex;
}

const void*
MemCache::getMemory(UInt32 ownerID, UInt16& index, Boolean* purged) {

  if (index < fMaxEntries) {
    MemEntry& p = fEntries[index];
    if (p.fMemPtr && p.fOwnerID == ownerID) { // cache hit
      void* mem = p.fMemPtr;

      if (fPurge) {
        UInt16 dist = (fTop < index ? fTop + fMaxEntries : fTop) - index;
        if (dist > fNearLimit) { // move entry to top
          fTop = next(fTop);

          index = fTop; // set index
          moveEntry(fEntries[index], p, purged); // test in move avoids copy to self
        }
      }

      return mem;
    }
  }

  // cache miss
  index = kNoIndex;
  return 0;
}

void*
MemCache::refMemory(UInt32 ownerID, UInt16 index) {
  if (index < fMaxEntries) {
    MemEntry& p = fEntries[index];
    if (p.fOwnerID == ownerID) {
      return p.fMemPtr; // if null that's ok
    }
  }

  return 0;
}

void
MemCache::releaseMemory(UInt32 ownerID, UInt16 index, Boolean* purged) {
  if (index < fMaxEntries) {
    MemEntry& p = fEntries[index];
    if (p.fOwnerID == ownerID) {
      releaseEntry(p, purged);
    }
  }
}

void 
MemCache::releaseAll() {
  for (UInt16 i = 0; i < fMaxEntries; ++i) {
    releaseEntry(fEntries[i], 0);
  }
}

void
MemCache::reserveSpace(UInt16 numEntries, UInt32 memSize, Boolean* purged) {
  UInt16 bottom = fTop;
  UInt16 entryLimit = fMaxEntries > numEntries ? fMaxEntries - numEntries : 0;
  while (fNumEntries > entryLimit) {
    bottom = next(bottom);
    releaseEntry(fEntries[bottom], purged); // adjusts fMem, fNumEntries
  }

  UInt32 memLimit = fMaxMem > memSize ? fMaxMem - memSize : 0;
  while (fMem > memLimit && fNumEntries > 0) {
    bottom = next(bottom);
    releaseEntry(fEntries[bottom], purged); // adjusts fMem, fNumEntries
  }
}
