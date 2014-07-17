/*
 * @(#)hsGGlyphCache.cpp	1.33 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * Copyright (C) 1996-1997 all rights reserved by HeadSpin Technology Inc. Chapel Hill, NC USA
 *
 * This software is the property of HeadSpin Technology Inc. and it is furnished
 * under a license and may be used and copied only in accordance with the
 * terms of such license and with the inclusion of the above copyright notice.
 * This software or any other copies thereof may not be provided or otherwise
 * made available to any other person or entity except as allowed under license.
 * No title to and ownership of the software or intellectual property
 * therewithin is hereby transferred.
 *
 * HEADSPIN MAKES NO REPRESENTATIONS OR WARRANTIES ABOUT THE SUITABILITY
 * OF THE SOFTWARE, EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED
 * TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
 * PARTICULAR PURPOSE, OR NON-INFRINGEMENT. SUN SHALL NOT BE LIABLE FOR
 * ANY DAMAGES SUFFERED BY LICENSEE AS A RESULT OF USING, MODIFYING OR
 * DISTRIBUTING THIS SOFTWARE OR ITS DERIVATIVES.
 *
 * This information in this software is subject to change without notice
*/

#if !defined(HS_BUILD_FOR_MAC)
#include <new.h>        // For _set_new_handler
#endif
#include <assert.h>
#include <string.h> // memset
#include "hsGGlyphCache.h"
#include "hsTemplates.h"
#include "hsGFontScaler.h"
#include "hsGeometry.h"

#include "GlyphMemCache.h"

///////////////////////////////////////////////////////////////////////////////////
#define HS_BUILD_FOR_WIN32 1
typedef void (*NewHandlerProc)();
typedef void (*new_handler)(); 
// new_handler set_new_handler(new_handler); 

#define MAX_GLYPHS 768
#define MOVE_LIMIT 512
#define MAX_MEMORY (MAX_GLYPHS * 256)
#define PURGE_OFF 0
#define PURGE_ON 1

class hsGGlyphCache {
        static void             New_Handler();
        static NewHandlerProc   gPrevHandler;
        static hsGGlyphCache*   gCache;

        hsDynamicArray<hsGGlyphStrike*> fStrikeList;
        GlyphMemCache           fMemCache;

        Boolean                 Purge();

                                hsGGlyphCache(): fStrikeList(), 
				  fMemCache(MAX_GLYPHS, MOVE_LIMIT, MAX_MEMORY, PURGE_OFF) {};

public:
        virtual                 ~hsGGlyphCache();
        hsGGlyphStrike* FindStrike(hsConstDescriptor desc);
        void                            AddStrike(hsGGlyphStrike* strike);

        GlyphMemCache&          getMemCache() { return fMemCache; }

        void resetStrikeIDs();

        static void             Kill();
        static hsGGlyphCache* GetGlobalCache()
        {
                if (gCache == nil)
                {       gCache = new hsGGlyphCache();
        #if HS_BUILD_FOR_WIN32
                //      gPrevHandler = _set_new_handler(hsGGlyphCache::New_Handler);
        #else
//                              gPrevHandler = set_new_handler(hsGGlyphCache::New_Handler);
        #endif
                }
                return gCache;
        }
};

NewHandlerProc hsGGlyphCache::gPrevHandler;
hsGGlyphCache* hsGGlyphCache::gCache = nil;

///////////////////////////////////////////////////////////////////////////////////

struct hsGGlyphStrikeEntry {
        hsGGlyph                fGlyph;
        hsFixedPoint2   fAdvance;
        hsRect          fOutlineBounds;
        UInt16 fCacheIndex; // used for GlyphMemCache
};

#define kNoMetrics_Width                0xFFFF  // marks the fWidth field to indicate that there are no valid metrics yet
#define kNoImage_Image          (void*)-1L              // marks the fImage field to indicate that there is no valid image yet
#define kNoBounds_Left (hsScalar)0xFEEDBEEF

// if font has more than this number of glyphs, use two-stage lookup array and initialize on demand
#define kEntryLengthLimit 1024

#define kExtendedChunkLengthBits 7
#define kExtendedChunkLengthMask 0x7f
#define kExtendedChunkLength (1<<kExtendedChunkLengthBits)
#define kExtendedChunkCountBits (16 - kExtendedChunkLengthBits)
#define kExtendedChunkCountMask 0x1ff
#define kExtendedChunkCount (1<<kExtendedChunkCountBits)

UInt32 hsGGlyphStrike::gID = 0; // strike ID needs synchronized access

hsGGlyphStrike::hsGGlyphStrike(hsConstDescriptor desc) 
  : fEntries(0)
  , fExtendedEntries(0)
{
        fDesc = hsDescriptor_Copy(desc);
        fID = ++gID << 16;
        fSizeHint = 10;

        UInt32 scalerID = hsDescriptor_Find32(desc, kScalerID_FontScalerDesc);
        hsGFontScaler*  scaler = hsGFontScaler::Find(scalerID);
        hsThrowIfNilParam(scaler);

        fScalerContext  = scaler->CreateContext(desc);
        fGlyphCount     = fScalerContext->CountGlyphs();
	
	if (fGlyphCount >= kEntryLengthLimit) {
	  fExtendedEntries = new hsGGlyphStrikeEntry*[kExtendedChunkCount];
	  memset(fExtendedEntries, 0, sizeof(hsGGlyphStrikeEntry*) * kExtendedChunkCount);
	} else if (fGlyphCount) {
	  fEntries = new hsGGlyphStrikeEntry[fGlyphCount];
	  initEntries(fEntries, fGlyphCount);
	}

        if (fGlyphCount) {
            fScalerContext->GetLineHeight(&fAscent, &fDescent, &fBaseline,
                                          &fLeading, &fMaxAdvance);
	} else {
	  fAscent.fX = 0;
	  fAscent.fY = 0;
	  fDescent.fX = 0;
	  fDescent.fY = 0;
	  fBaseline.fX = 0;
	  fBaseline.fY = 0;
	  fLeading.fX = 0;
	  fLeading.fY = 0;
	  fMaxAdvance.fX = 0;
	  fMaxAdvance.fY = 0;
        }
}

void 
hsGGlyphStrike::initEntries(hsGGlyphStrikeEntry* entries, int count) 
{
  for (hsGGlyphStrikeEntry* e = entries + count; e-- != entries;) {
    e->fGlyph.fWidth = kNoMetrics_Width;
    e->fGlyph.fImage = kNoImage_Image;
    e->fOutlineBounds.fLeft = kNoBounds_Left;
    e->fCacheIndex = MemCache::kNoIndex;
  }
}

hsGGlyphStrikeEntry* 
hsGGlyphStrike::getEntry(int index) 
{
  if (fExtendedEntries) {
    return getExtendedEntry(index);
  } else {
    return &fEntries[index];
  }
}

hsGGlyphStrikeEntry*
hsGGlyphStrike::getExtendedEntry(int index) {
  int chunkIndex = (index >> kExtendedChunkLengthBits) & kExtendedChunkCountMask;
  int lengthIndex = index & kExtendedChunkLengthMask;
  if (fExtendedEntries[chunkIndex] == 0) {
    fExtendedEntries[chunkIndex] = new hsGGlyphStrikeEntry[kExtendedChunkLength];
    initEntries(fExtendedEntries[chunkIndex], kExtendedChunkLength);
  }
  return &fExtendedEntries[chunkIndex][lengthIndex];
}

hsGGlyphStrike::~hsGGlyphStrike()
{
  /* Don't need anymore, cache manages memory now.
        for (unsigned i = 0; i < fGlyphCount; i++)
                if (fEntries[i].fGlyph.fImage != kNoImage_Image)
                        HSMemory::Delete((void*)fEntries[i].fGlyph.fImage);
                        */

  if (fExtendedEntries) {
    for (int i = 0; i < kExtendedChunkCount; ++i) {
      delete[] fExtendedEntries[i];
    }
    delete[] fExtendedEntries;
  } else {
    delete[] fEntries;
  }

  delete fScalerContext;
  hsDescriptor_Delete(fDesc);
}

void hsGGlyphStrike::resetID() {
  fID = ++gID << 16;
}

Boolean hsGGlyphStrike::canRotate ()
{
        return fScalerContext->canRotate ();
}

hsPathSpline::Type hsGGlyphStrike::GetPathType() const
{
        return fScalerContext->GetPathType();
}

CharToGlyphMapper *
hsGGlyphStrike::getMapper()
{
    return fScalerContext->getMapper();
}

void hsGGlyphStrike::GetOutlineBounds(UInt16 index, hsRect* bounds)
{
  if (!bounds)
    return;

  hsAssert(index < fGlyphCount, "bad index");
  if (index >= fGlyphCount) {
    memset(bounds, 0, sizeof(*bounds));
    return;
  }

  hsGGlyphStrikeEntry* entry = getEntry(index);
        
  if (entry->fOutlineBounds.fLeft == kNoBounds_Left) {
    hsPathSpline path;
    path.Init(&path);
    entry->fOutlineBounds.Set(0, 0, 0, 0); // ComputeBounds does not set!
    fScalerContext->GeneratePath(index, &path, nil);
    // both cubic and quadratic use same contour data, use bounding box of points since
    // we don't have an implementation for cubic and it's not clear we need to do better

    ((hsQuadraticSpline*)&path)->ComputeBounds(&entry->fOutlineBounds, (Boolean)false);
            
    hsAssert(entry->fOutlineBounds.fLeft != kNoBounds_Left, "bad bounds flag");
  }

  *bounds = entry->fOutlineBounds;
}

void hsGGlyphStrike::GetMetrics(UInt16 index, hsGGlyph* glyph, hsFixedPoint2* advance)
{
	hsAssert(index < fGlyphCount, "bad index");
        if (index >= fGlyphCount) {
            if (glyph) {
                memset(glyph, 0, sizeof(*glyph));
            }
            if (advance) {
                memset(advance, 0, sizeof(*advance));
            }
            return;
        }

	hsGGlyphStrikeEntry* entry = getEntry(index);
	
	if (entry->fGlyph.fWidth == kNoMetrics_Width) {
            fScalerContext->GenerateMetrics(index, &entry->fGlyph, &entry->fAdvance);
            hsAssert(entry->fGlyph.fWidth != kNoMetrics_Width, "bad width flag");
	}
	if (glyph)
		*glyph = entry->fGlyph;
	if (advance)
		*advance = entry->fAdvance;
}

void hsGGlyphStrike::getEntryImage(hsGGlyphStrikeEntry* entry, UInt16 glyphIndex, Boolean* purged) {
  UInt32 ownerID = fID | glyphIndex;
  void* image = (void*)hsGGlyphCache::GetGlobalCache()->getMemCache().getGlyphImage(ownerID, entry->fCacheIndex, purged);

  // signal that the image is not available in the cache
  entry->fGlyph.fImage = image ? image : kNoImage_Image;
}

void hsGGlyphStrike::refEntryImage(hsGGlyphStrikeEntry* entry, UInt16 glyphIndex) {
  UInt32 ownerID = fID | glyphIndex;
  
  // return null if image not available in the cache
  entry->fGlyph.fImage = hsGGlyphCache::GetGlobalCache()->getMemCache().refGlyphImage(ownerID, entry->fCacheIndex);
}

void hsGGlyphStrike::assignEntryImage(hsGGlyphStrikeEntry* entry, UInt16 glyphIndex, Boolean* purged) {
  UInt32 ownerID = fID | glyphIndex;
  UInt32 memSize = entry->fGlyph.fHeight * entry->fGlyph.fRowBytes;
  fSizeHint = fSizeHint - fSizeHint >> 4 + memSize;

  entry->fCacheIndex = hsGGlyphCache::GetGlobalCache()->getMemCache().
    assignGlyphImage(ownerID, entry->fGlyph, purged); // cast away const on void* memory
}

void hsGGlyphStrike::GetMetricsWithImage(UInt16 index, hsGGlyph* glyph, hsFixedPoint2* advance, Boolean* refcheck)
{
        hsAssert(index < fGlyphCount, "bad index");
        if (index >= fGlyphCount) {
            if (glyph) {
                memset(glyph, 0, sizeof(*glyph));
            }
            if (advance) {
                memset(advance, 0, sizeof(*advance));
            }
            return;
        }

        hsGGlyphStrikeEntry* entry = getEntry(index);
        getEntryImage(entry, index, refcheck);

        if (        (entry->fGlyph.fWidth == kNoMetrics_Width)
                ||  (entry->fGlyph.fImage == kNoImage_Image)) {

            fScalerContext->GenerateMetricsWithImage(index, &entry->fGlyph, &entry->fAdvance);
            assignEntryImage(entry, index, refcheck);

            hsAssert(entry->fGlyph.fWidth != kNoMetrics_Width, "bad width flag");
            hsAssert(entry->fGlyph.fImage != kNoImage_Image, "bad image");
        }
        if (glyph)
                *glyph = entry->fGlyph;
        if (advance)
                *advance = entry->fAdvance;
}

void hsGGlyphStrike::GetItalicAngle(hsFixedPoint2* italicAngle) {
    fScalerContext->GetCaretAngle(italicAngle);
}

const void* hsGGlyphStrike::RefImage(UInt16 index) {
        hsAssert(index < fGlyphCount, "bad index");
        if (index >= fGlyphCount) {
            return nil;
        }

        hsGGlyphStrikeEntry* entry = getEntry(index);
        refEntryImage(entry, index);
        return entry->fGlyph.fImage;
}

void hsGGlyphStrike::PrepareCache(UInt32 numGlyphs, Boolean* purged) {
  hsGGlyphCache::GetGlobalCache()->getMemCache().reserveGlyphs(numGlyphs, numGlyphs * fSizeHint >> 4, purged);
}

const void* hsGGlyphStrike::GetImage(UInt16 index, Boolean* refcheck)
{
	hsAssert(index < fGlyphCount, "bad index");
        if (index >= fGlyphCount) {
            return nil;
        }

	hsGGlyphStrikeEntry* entry = getEntry(index);
	getEntryImage(entry, index, refcheck);
	if (entry->fGlyph.fImage == kNoImage_Image ||
	    entry->fGlyph.fWidth == kNoMetrics_Width) {
	    entry->fGlyph.fImage = nil;

	    if (entry->fGlyph.fWidth == kNoMetrics_Width) {
	      fScalerContext->GenerateMetricsWithImage(index, &entry->fGlyph, &entry->fAdvance);
	    } else {
            UInt32 imageSize = entry->fGlyph.fHeight * entry->fGlyph.fRowBytes;
            void* image = HSMemory::SoftNew(imageSize);

            if (image) {
#ifndef CC_NOEX
                try {
#endif // !CC_NOEX
                    fScalerContext->GenerateImage(index, &entry->fGlyph, image);
		    entry->fGlyph.fImage = image;
		    assignEntryImage(entry, index, refcheck);
#ifndef CC_NOEX
                }
                catch (...) {
                    HSMemory::Delete(image);
                    image = kNoImage_Image;
                }
#endif // !CC_NOEX
            }
	    }
	}
	return entry->fGlyph.fImage;
}

void hsGGlyphStrike::GetPath(UInt16 index, hsPathSpline* path, hsFixedPoint2* advance)
{
    hsAssert(index < fGlyphCount, "bad index");
    if (index >= fGlyphCount) {
        return;
    }
    fScalerContext->GeneratePath(index, path, advance);
}

Boolean hsGGlyphStrike::GetGlyphPoint(UInt16 index, Int32 pointNumber, hsFixedPoint2* point)
{
    hsAssert(index < fGlyphCount, "bad index");
    if (index >= fGlyphCount) {
        return false;
    }

    return fScalerContext->GetGlyphPoint(index, pointNumber, point);
}

void hsGGlyphStrike::TransformFunits(Int16 xFunits, Int16 yFunits, hsFixedPoint2 *pixels)
{
    fScalerContext->TransformFunits(xFunits, yFunits, pixels);
}

void hsGGlyphStrike::GetLineHeight(hsFixedPoint2* ascent, hsFixedPoint2* descent,
                                   hsFixedPoint2* baseline, hsFixedPoint2 *leading,
                                   hsFixedPoint2* maxAdvance)
{
        if (ascent)
                *ascent = fAscent;
        if (descent)
                *descent = fDescent;
        if (baseline)
                *baseline = fBaseline;
        if (leading)
                *leading = fLeading;
        if (maxAdvance)
                *maxAdvance = fMaxAdvance;
}

hsGGlyphStrike* hsGGlyphStrike::RefStrike(hsConstDescriptor desc)
{
        hsGGlyphCache* cache = hsGGlyphCache::GetGlobalCache();
        hsGGlyphStrike* strike = cache->FindStrike(desc);

        if (strike == nil) {
            strike = new hsGGlyphStrike(desc);
            cache->AddStrike(strike);
            
            if (gID > 0x7fff) { // empty glyph mem cache, reset ids of all current strikes
              gID = 0;
              cache->resetStrikeIDs();
            }

        }
        // this is balanced by UnRef() the client when they're done using it
        strike->Ref();  
        
        return strike;
}

void hsGGlyphStrike::KillGlyphCache()
{
        hsGGlyphCache::Kill();
}

Boolean hsGGlyphStrike::isStyleSupported(int aStyle)
{
    if (fScalerContext) {
        return fScalerContext->isStyleSupported(aStyle);
    }
    return false;
}

///////////////////////////////////////////////////////////////////////////////////

static Boolean unrefStrike(hsGGlyphStrike*& strike)
{
        hsAssert(strike->RefCnt() == 1, "too many refs on strike");
        strike->UnRef();
        return false;
}

hsGGlyphCache::~hsGGlyphCache()
{
        (void)fStrikeList.ForEach(unrefStrike);
}

hsGGlyphStrike* hsGGlyphCache::FindStrike(hsConstDescriptor desc)
{
    hsGGlyphStrike**    strikeList = fStrikeList.AcquireArray();
    hsGGlyphStrike**    strike = strikeList;
    hsGGlyphStrike**    stop = strike + fStrikeList.GetCount();
    
    while (strike != stop) {
        if (hsDescriptor_Equal((*strike)->GetDesc(), desc)) {
            fStrikeList.ReleaseArray(strikeList);
            return *strike;
        }
        strike += 1;
    }
    fStrikeList.ReleaseArray(strikeList);
    return nil;
}

void hsGGlyphCache::resetStrikeIDs() {
    fMemCache.releaseAllGlyphs();

    hsGGlyphStrike**    strikeList = fStrikeList.AcquireArray();
    hsGGlyphStrike**    strike = strikeList;
    hsGGlyphStrike**    stop = strike + fStrikeList.GetCount();
    
    while (strike != stop) {
        (**strike).resetID();
        strike++;
    }
    fStrikeList.ReleaseArray(strikeList);
}

/* This was 64 in earlier 1.4 builds, but applications which exceed this limit
 * see a major hit (see bug 4483024 for more info). Need to make this
 * degradation more graceful
 */
#define kStrikeLimit 256

void hsGGlyphCache::AddStrike(hsGGlyphStrike* strike)
{
        int count = fStrikeList.GetCount();
        if (count > kStrikeLimit)
        {       Boolean result = this->Purge();
                hsAssert(result, "cache is locked");
                count = 0;
        }
        fStrikeList.Append(strike);
}

Boolean hsGGlyphCache::Purge()
{
        Boolean didPurge = false;

        for (int i = fStrikeList.GetCount() - 1; i >= 0; i--)
        {       hsGGlyphStrike* strike = fStrikeList.Get(i);
        
                if (strike->RefCnt() == 1)
                {       delete strike;
                        fStrikeList.Remove(i);
                        didPurge = true;
                }
        }
        return didPurge;
}

void hsGGlyphCache::Kill()
{
        if (gCache)
        {       delete gCache;
                gCache = nil;
#if HS_BUILD_FOR_WIN32
//              set_new_handler(gPrevHandler);
#else
//                      set_new_handler(gPrevHandler);
#endif
        }
}

void hsGGlyphCache::New_Handler()
{
        if (gCache && gCache->Purge())
                return;

        if (gPrevHandler)
                gPrevHandler();
#ifndef CC_NOEX
        else
                throw "new failed";
#endif // !CC_NOEX
}

#undef HS_BUILD_FOR_WIN32
