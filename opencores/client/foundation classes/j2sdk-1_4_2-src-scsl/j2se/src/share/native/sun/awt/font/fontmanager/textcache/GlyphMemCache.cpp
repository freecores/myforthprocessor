/*
 * @(#)GlyphMemCache.cpp	1.3 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "GlyphMemCache.h"
#include "hsMemory.h"

UInt16 
GlyphMemCache::assignGlyphImage(UInt32 ownerID, hsGGlyph& glyph, Boolean *purged) {
  return assignMemory(ownerID, glyph.fHeight * glyph.fRowBytes, (void*)glyph.fImage, purged);
}

const void* 
GlyphMemCache::getGlyphImage(UInt32 ownerID, UInt16& index, Boolean *purged) {
  return getMemory(ownerID, index, purged);
}

const void* 
GlyphMemCache::refGlyphImage(UInt32 ownerID, UInt16 index) {
  return refMemory(ownerID, index);
}

void 
GlyphMemCache::reserveGlyphs(UInt16 numGlyphs, UInt32 memSize, Boolean *purged) {
  reserveSpace(numGlyphs, memSize, purged);
}

void 
GlyphMemCache::releaseAllGlyphs() {
  releaseAll();
}
  
void 
GlyphMemCache::disposeMemPtr(void* mem) {
  HSMemory::Delete(mem);
}
