/*
 * @(#)t2kScalerContext.cpp	1.50 03/02/19
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * Copyright (C) 1996-1998 all rights reserved by HeadSpin Technology Inc. Chapel Hill, NC USA
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
 * PARTICULAR PURPOSE, OR NON-INFRINGEMENT. HEADSPIN SHALL NOT BE LIABLE FOR
 * ANY DAMAGES SUFFERED BY LICENSEE AS A RESULT OF USING, MODIFYING OR
 * DISTRIBUTING THIS SOFTWARE OR ITS DERIVATIVES.
 *
 * This information in this software is subject to change without notice
*/

#include <malloc.h>
#include <assert.h>
#include <string.h>
#include <java_awt_Font.h>
#include "t2kScalerContext.h"
#include "hsGText.h"
#include "hsTemplates.h"
#include "fontObject.h"
#include "hsMemory.h"
#include "cmaps.h"

////////// Call-backs need by the T2K scaler ///////////

static hsFixed FixedBiasedRound(hsFixed theValue)
{
    theValue += 0x7FFFL;
    theValue = ( theValue & 0xFFFF0000L );
    return theValue;
}


struct sizeBlock {
    /* t2k_malloc must maintain mallocs alignment 
       of bouble the size of a pointer */
    union {
            void *size_pad[2];
            size_t  size;
    } u;
};

void* t2k_malloc(size_t size)
{
    sizeBlock* block = (sizeBlock*)HSMemory::SoftNew(size + sizeof(sizeBlock));

    if (block)
      {       block->u.size = size;
          return block + 1;
      }
    return nil;
}

void t2k_free(void* p)
{
    HSMemory::Delete((sizeBlock*)p - 1);
}

void* t2k_realloc(void* oldp, size_t newSize)
{
    size_t      oldSize = ((sizeBlock*)oldp - 1)->u.size;
    void*       newp = t2k_malloc(newSize);

    if (newp)
      {       ((sizeBlock*)newp - 1)->u.size = newSize;

          HSMemory::BlockMove(oldp, newp, hsMinimum(oldSize, newSize));

          t2k_free(oldp);
      }
    return newp;
}

#ifdef HS_DEBUGGING
void t2k_Assert(int cond)
{
    hsAssert(cond, "T2K assert");
}
#endif

#define F26Dot6ToFixed(n)               ((n) << 10)
#define kF26Dot6_One                    (1 << 6)

/////////////////////////////////////////////////////////////////////////////
/*-----------------------------------------------------------------------*
 *
 *  Callback function for getting file fragments
 *
 *-----------------------------------------------------------------------*/

extern "C" {
    static void ReadFileDataFunc(void *id, tt_uint8 *dest_ram, tt_int32 offset, tt_int32 numBytes)
      {
          int error;
          size_t count;
          fontObject *fo = (fontObject *) id;

          //printf("ReadFileDataFunc(%08X, %08X, %d, %d)\n", id, dest_ram, offset, numBytes);

          assert(fo != NULL);

          fo->readBlock(offset, numBytes, (char *) dest_ram);
      }
}


class T2KEntry : public hsRefCnt {
    T2K*        fT2K;
    // const void* fFontData;
    // These are the key for lookup
    fontObject *	fFont;
    hsFixed	fBoldness;
    hsFixed     fItalic;
  public:
    int         fNumGlyphs;
    T2KEntry(fontObject * font, hsFixed boldness, hsFixed italic)
      : fT2K(nil), /* fFontData(nil), */ fFont(font), fBoldness(boldness), fItalic(italic),
    fNumGlyphs(0) { }
    virtual	~T2KEntry();

    fontObject *	GetFont() const { return fFont; }
    hsFixed	GetBoldness() const { return fBoldness; }
    hsFixed	GetItalicness() const { return fItalic; }

    T2K*        GetT2K();
    void        ZapT2K();               // when an error occurs
};

class hsGT2KCache {
    static hsDynamicArray<T2KEntry*>    gT2KCache;

    static void Remove(T2KEntry* entry);

    friend class T2KEntry;
  public:
    static T2KEntry*    RefEntry(fontObject * font, hsFixed boldness, hsFixed italic);
};

//////////////////////////////////////////////////////////////////////////////////////////

T2KEntry::~T2KEntry()
{
    if (fT2K != nil)
      {       int                     errCode;
          tsiMemObject*   mem = fT2K->mem;
          sfntClass*      fontClass = fT2K->font;
          InputStream*    stream = fontClass->in;

          DeleteT2K(fT2K, &errCode);
          hsIfDebugMessage(errCode, "DeleteT2K failed", errCode);
          Delete_sfntClass(fontClass, &errCode);
          hsIfDebugMessage(errCode, "Delete_sfntClass failed", errCode);
          Delete_InputStream(stream, &errCode);
          hsIfDebugMessage(errCode, "Delete_InputStream failed", errCode);
          tsi_DeleteMemhandler(mem);
          // FOReleaseChunk(fFont, fFontData);
      }
    hsGT2KCache::Remove(this);
}

T2K* T2KEntry::GetT2K()
{
    if (fT2K == nil)
      {       int     errCode;

          tsiMemObject*   mem = tsi_NewMemhandler(&errCode);
          hsIfDebugMessage(errCode, "tsi_NewMemhandler failed", errCode);

          short           fontType = 0;
          tt_uint32   dataLength;
          unsigned char *data;
          InputStream *stream = NULL;

          // need to balance this in the destructor with ReleaseData()
          //fFontData = FOGetData(fFont);
          dataLength = FOGetDataLength(fFont);

          switch (fFont->GetFormat()) {
            case kTrueTypeFontFormat:
            case kT2KFontFormat:
              fontType = FONT_TYPE_TT_OR_T2K;

              stream =  New_NonRamInputStream(mem, (void *) fFont, ReadFileDataFunc, dataLength, &errCode);
              break;

            case kType1FontFormat:
              fontType = FONT_TYPE_1;
              data = (unsigned char *) FOGetData(fFont);
              if (*data == 0x80) {              //- PFB file, remove the headers
                  data = ExtractPureT1FromPCType1(data, &dataLength);
                  if (data == NULL) {
                    hsDebugMessage("Detected broken type1 font. Marking invalid.", 0);
                    FOReleaseChunk(fFont, nil);
                    fFont->isValid = false;
                    tsi_DeleteMemhandler(mem);
                    return NULL;
                  }
              }
              stream = New_InputStream3(mem, data, dataLength, &errCode);
              FOReleaseChunk(fFont, nil);
              break;
            default:
#ifndef CC_NOEX
              throw "unknown font format";
#endif
              break;
          }


          hsIfDebugMessage(errCode, "New_InputStream3 failed", errCode);

          sfntClass*      fontClass;

          if (fBoldness != hsFixed1 || fItalic != 0) {
              T2K_AlgStyleDescriptor	desc;
#ifdef ALGORITHMIC_STYLES
//            desc.StyleFunc	       	= tsi_SHAPET_BOLDITALIC_GLYPH_PreAdjust;
              desc.StyleMetricsFunc	= tsi_SHAPET_BOLD_METRICS;
              desc.StyleFuncPost        = tsi_SHAPET_BoldItalic_GLYPH_Hinted;
#else
              desc.StyleFunc              = 0;
              desc.StyleMetricsFunc       = 0;
#endif

              desc.params[0]		= fBoldness;
              desc.params[1]              = fItalic;

              fontClass = New_sfntClassLogical(mem, fontType, fFont->fCurFont, stream, &desc, &errCode);
          } else {
              fontClass = New_sfntClassLogical(mem, fontType, fFont->fCurFont, stream, nil, &errCode);
          }

          hsIfDebugMessage(errCode, "New_sfntClass failed", errCode);

          fT2K = NewT2K(mem, fontClass, &errCode);
         hsIfDebugMessage(errCode, "NewT2K failed", errCode);
          fNumGlyphs = GetNumGlyphs_sfntClass(fontClass);
          if (fontClass && fontClass->head) {
              fFont->fUnitsPerEM = fontClass->head->unitsPerEm;
              fFont->fMacStyle   = fontClass->head->macStyle;
          } else {
              if (fT2K->caretDx)
                fFont->fMacStyle = 2;
          }
      }
    return fT2K;
}

void T2KEntry::ZapT2K()
{
    fT2K = nil;
    // FOReleaseChunk(fFont, fFontData);
}

//////////////////////////////////////////////////////////////////////////////////////////

hsDynamicArray<T2KEntry*>       hsGT2KCache::gT2KCache;

static Boolean findByFont(T2KEntry*& entry, void* font, void* params)
{
    return entry->GetFont() == (fontObject *)font
      && entry->GetBoldness() == ((hsFixed*)params)[0]
        && entry->GetItalicness() == ((hsFixed*)params)[1];
}

T2KEntry* hsGT2KCache::RefEntry(fontObject * font, hsFixed boldness, hsFixed italic)
{
    hsFixed params[] = {boldness, italic};
    T2KEntry** entry = gT2KCache.ForEach(findByFont, font, &params);

    if (entry) {
        (*entry)->Ref();
        return *entry;
    } else {
        Int32 index = gT2KCache.Append(new T2KEntry(font,boldness, italic));
        return gT2KCache[index - 1];
    }
}

void hsGT2KCache::Remove(T2KEntry* entry)
{
    for (int i = 0; i < gT2KCache.GetCount(); i++) {
        if (entry == gT2KCache[i]) {
            gT2KCache.Remove(i);
            return;
        }
    }

    hsDebugMessage("UnRefT2K failed", 0);
}

/////////////////////////////////////////////////////////////////////////////

t2kScalerContext::t2kScalerContext(fontObject * font, const float matrix[],
                                   Boolean doAntiAlias, Boolean doFractEnable,
                                   int style)
    : fFont(font),
      t1Mapper(NULL), ttMapper(NULL)
{
    int algoStyle = style & ~font->GetRealStyle();
    fBoldness = hsFixed1;	// this controls algo boldness
    fItalic = 0;

#if 0
    if (style & java_awt_Font_BOLD) {
        // This seems like an optimal value tried with 1.15 and 1.25
        // actually this should be user configurable to get various
        // weights - must look for ways to specify this from JAVA level.
        fBoldness = hsFloatToFixed ( 1.2 );
    }
#endif

#ifdef ALGORITHMIC_STYLES

    if (algoStyle & java_awt_Font_BOLD) {
        fBoldness = hsFloatToFixed ( 1.33 );
    }

    if (algoStyle & java_awt_Font_ITALIC) {
        fItalic =  hsFloatToFixed ( 0.7 );
    }
#endif

    fT2KEntry = hsGT2KCache::RefEntry(font, fBoldness, fItalic);
    for (int i = 0; i < 4; i++)
      fMatrix[i] = matrix[i];
    fDoAntiAlias = doAntiAlias;
    fDoFractEnable = doFractEnable;

#if 0
    if (style & java_awt_Font_ITALIC) {
        // apply horizontal shear
        // matches AffineTransform.java
        fMatrix[2] = fMatrix[0] * -0.2 + fMatrix[2];
        fMatrix[3] = fMatrix[1] * -0.2 + fMatrix[3];
    }
#endif
    if (fDoAntiAlias)
      fGreyLevel = GREY_SCALE_BITMAP_HIGH_QUALITY;
    else
      fGreyLevel = BLACK_AND_WHITE_BITMAP;

    if (font->GetFormat() == kTrueTypeFontFormat) {
        fT2KFlags = T2K_GRID_FIT | T2K_CODE_IS_GINDEX;
    } else if (font->GetFormat() == kType1FontFormat) {
        fT2KFlags = T2K_GRID_FIT | T2K_CODE_IS_GINDEX;
    }


    //  if (unhinted outlines)
    //          fT2KFlags &= ~T2K_GRID_FIT;

    if (fT2KEntry->GetFont()->GetFormat() == kType1FontFormat)
      hsGScalerContext::fPathType = hsPathSpline::kCubicType;
    else
      hsGScalerContext::fPathType = hsPathSpline::kQuadType;

    T2K* t2k = fT2KEntry->GetT2K();
    fNumGlyphs = fT2KEntry->fNumGlyphs;
}

t2kScalerContext::~t2kScalerContext()
{
    fT2KEntry->UnRef();
    if (t1Mapper) {
        delete t1Mapper;
        t1Mapper = NULL;
    }
    if (ttMapper) {
        delete ttMapper;
        ttMapper = NULL;
    }
}

T2K* t2kScalerContext::SetupTrans()
{
    int                                 errCode = 0;
    T2K_TRANS_MATRIX    t2kMatrix;
    T2K*                                t2k = fT2KEntry->GetT2K();

    // pull from corresponding java affine transform data
    t2kMatrix.t00       = hsScalarToFixed(fMatrix[0]);
    t2kMatrix.t10       = -hsScalarToFixed(fMatrix[1]);
    t2kMatrix.t01       = -hsScalarToFixed(fMatrix[2]);
    t2kMatrix.t11       = hsScalarToFixed(fMatrix[3]);

    // set up true flag for getting sbits if available
    // in the case where algorithmic styles are being applied
    // make sure the bitmaps are not being used because we do not know
    // in t2k how to transform bitmaps. Better results are obtained
    // anyways if outlines are transformed and hints are used to get
    // algorithmic outlines.
    // if AntiAliasing is required or fractional metrics are asked for
    // we do not want bitmaps as they will not be accurate in the case of fm
    // where as in the aa case we should ignore bitamps altogether
    if (fBoldness != hsFixed1 || fItalic != 0
	|| fDoAntiAlias == true || fDoFractEnable == true ) {
        T2K_NewTransformation(t2k, true, 72, 72, &t2kMatrix, false, &errCode);
    }
    else {
        T2K_NewTransformation(t2k, true, 72, 72, &t2kMatrix, true, &errCode);
    }

    hsIfDebugMessage(errCode, "T2K_NewTransformation failed", errCode);

    return t2k;
}

void t2kScalerContext::GenerateMetrics(UInt16 glyphID, hsGGlyph* glyph,
                                       hsFixedPoint2* advance)
{
    int         errCode;
    T2K*        t2k = this->SetupTrans();

    T2K_RenderGlyph(t2k, glyphID, 0, 0, fGreyLevel,
                    fT2KFlags | T2K_SCAN_CONVERT |
                    T2K_SKIP_SCAN_BM, &errCode);
    hsIfDebugMessage(errCode, "T2K_RenderGlyph failed", errCode);

    glyph->fWidth               = (UInt16)t2k->width;
    glyph->fHeight              = (UInt16)t2k->height;
    glyph->fTopLeft.fX  = F26Dot6ToFixed(t2k->fLeft26Dot6);
    glyph->fTopLeft.fY  = -F26Dot6ToFixed(t2k->fTop26Dot6);

    if (fGreyLevel == BLACK_AND_WHITE_BITMAP)
#if 0   // this assumes that the blitters handle a bit-cache
      glyph->fRowBytes = t2k->rowBytes;
#else
      glyph->fRowBytes = glyph->fWidth;
#endif
    else
      glyph->fRowBytes = glyph->fWidth;

    if (fDoFractEnable) {
        advance->fX     = t2k->xLinearAdvanceWidth16Dot16;
        advance->fY     = -t2k->yLinearAdvanceWidth16Dot16;
    } else {
        // rounding advances in both x and y causes falloff from the baseline
        // so in this case we don't round (rounding the magnitude of the advance
        // vector really has no point to it -- caller can round resulting positions
        // if desired.
        if (!t2k->yAdvanceWidth16Dot16) {
            advance->fY = 0;
            advance->fX = (hsFixedRound ( t2k->xAdvanceWidth16Dot16 )) << 16;
        } else if (!t2k->xAdvanceWidth16Dot16) {
            advance->fX = 0;
            advance->fY= -(hsFixedRound ( t2k->yAdvanceWidth16Dot16 )) << 16;
        } else {
            advance->fX = t2k->xAdvanceWidth16Dot16;
            advance->fY = -t2k->yAdvanceWidth16Dot16;
        }
    }

    T2K_PurgeMemory(t2k, 1, &errCode);  // to relase the bitmap/outline
    hsIfDebugMessage(errCode, "T2K_PurgeMemory failed", errCode);
}

inline unsigned T2KByteToAlpha255(unsigned value)
{
    return (value << 4) + value >> 3;
}

static void CopyBW2Grey8(const void* srcImage, int srcRowBytes,
                         void* dstImage, int dstRowBytes,
                         int width, int height)
{
    const UInt8* srcRow = (UInt8*)srcImage;
    UInt8* dstRow = (UInt8*)dstImage;
    int wholeByteCount = width >> 3;
    int remainingBitsCount = width & 7;
    int i, j;

    while (height--) {
        const UInt8* src8 = srcRow;
        UInt8* dstByte = dstRow;
        unsigned srcValue;

        srcRow += srcRowBytes;
        dstRow += dstRowBytes;

        for (i = 0; i < wholeByteCount; i++) {
            srcValue = *src8++;
            for (j = 0; j < 8; j++) {
                *dstByte++ = (srcValue & 0x80) ? 0xFF : 0;
                srcValue <<= 1;
            }
        }
        if (remainingBitsCount) {
            srcValue = *src8;
            for (j = 0; j < remainingBitsCount; j++) {
                *dstByte++ = (srcValue & 0x80) ? 0xFF : 0;
                srcValue <<= 1;
            }
        }
    }
}

void t2kScalerContext::GenerateImage(UInt16 glyphID, const hsGGlyph* glyph, void* buffer)
{
    int         errCode;
    T2K*        t2k = this->SetupTrans();

    T2K_RenderGlyph(t2k, glyphID, 0, 0, fGreyLevel,
                    fT2KFlags | T2K_SCAN_CONVERT, &errCode);
    if (errCode)
      {       fT2KEntry->ZapT2K();
#ifndef CC_NOEX
          throw hsOSException(errCode);
#endif
      }

    Int32       size = glyph->fHeight * glyph->fRowBytes;

    if (fGreyLevel == BLACK_AND_WHITE_BITMAP)
#if 0   // this assumes that the blitters handle a bit-cache
      HSMemory::BlockMove(t2k->baseAddr, buffer, size);
#else
      CopyBW2Grey8(t2k->baseAddr, t2k->rowBytes,
                   buffer, glyph->fRowBytes,
                   glyph->fWidth, glyph->fHeight);
#endif
    else
      {       Byte* dstRow = (Byte*)buffer;
          const UInt8* srcRow = (UInt8*)t2k->baseAddr;
          int                     width = glyph->fWidth;

          for (int y = 0; y < glyph->fHeight; y++)
            {   for (int x = 0; x < width; x++)
              dstRow[x] = T2KByteToAlpha255(srcRow[x]);

                dstRow += glyph->fRowBytes;
                srcRow += t2k->rowBytes;
            }
      }

    T2K_PurgeMemory(t2k, 1, &errCode);  // to relase the bitmap/outline
    hsIfDebugMessage(errCode, "T2K_PurgeMemory failed", errCode);
}

void t2kScalerContext::GenerateMetricsWithImage(
    UInt16 glyphID, hsGGlyph* glyph,
    hsFixedPoint2* advance)
{
    int         errCode;
    T2K*        t2k = this->SetupTrans();

    T2K_RenderGlyph(t2k, glyphID, 0, 0, fGreyLevel,
                    fT2KFlags | T2K_SCAN_CONVERT, &errCode);
    if (errCode) {
        return;
    }

    glyph->fWidth       = (UInt16)t2k->width;
    glyph->fHeight      = (UInt16)t2k->height;
    glyph->fTopLeft.fX  = F26Dot6ToFixed(t2k->fLeft26Dot6);
    glyph->fTopLeft.fY  = -F26Dot6ToFixed(t2k->fTop26Dot6);
    glyph->fRowBytes    = glyph->fWidth;

    {
        UInt32 imageSize = glyph->fHeight * glyph->fRowBytes;
        glyph->fImage = HSMemory::SoftNew(imageSize);
    }

    if (glyph->fImage) {
        if (fGreyLevel == BLACK_AND_WHITE_BITMAP) {
            CopyBW2Grey8(   t2k->baseAddr, t2k->rowBytes,
                            (void *)glyph->fImage, glyph->fRowBytes,
                            glyph->fWidth, glyph->fHeight);
        } else {
            Byte*           dstRow = (Byte*)glyph->fImage;
            const UInt8*    srcRow = (UInt8*)t2k->baseAddr;
            int             width = glyph->fWidth;

            for (int y = 0; y < glyph->fHeight; y++) {
                for (int x = 0; x < width; x++) {
                    dstRow[x] = T2KByteToAlpha255(srcRow[x]);
                }
                dstRow += glyph->fRowBytes;
                srcRow += t2k->rowBytes;
            }
        }
    }

    if (fDoFractEnable) {
        advance->fX     = t2k->xLinearAdvanceWidth16Dot16;
        advance->fY     = -t2k->yLinearAdvanceWidth16Dot16;
    } else {
        if (!t2k->yAdvanceWidth16Dot16) {
            advance->fY = 0;
            advance->fX = (hsFixedRound ( t2k->xAdvanceWidth16Dot16 )) << 16;
        } else if (!t2k->xAdvanceWidth16Dot16) {
            advance->fX = 0;
            advance->fY= -(hsFixedRound ( t2k->yAdvanceWidth16Dot16 )) << 16;
        } else {
            advance->fX = t2k->xAdvanceWidth16Dot16;
            advance->fY = -t2k->yAdvanceWidth16Dot16;
        }
    }

    T2K_PurgeMemory(t2k, 1, &errCode);  // to relase the bitmap/outline
    hsIfDebugMessage(errCode, "T2K_PurgeMemory failed", errCode);
}

inline hsScalar F26Dot6_To_Scalar(F26Dot6 value)
{
#if HS_SCALAR_IS_FIXED
    return value << 10;
#else
    return hsScalar(value) / hsScalar(64);
#endif
}

static void ConvertGlyphToPath(const GlyphClass* glyph, hsPathSpline* path)
{
    int ctrCount, i, j;

    // Count significant contours (pointCount > 2)
    ctrCount = 0;
    for (i = 0; i < glyph->contourCount; i++)
      if (glyph->ep[i] - glyph->sp[i] > 1)
        ctrCount += 1;

    path->fContourCount = ctrCount;
    if (ctrCount == 0)
      {       path->fContours = nil;
          return;
      }
    path->fContours = new hsPathContour[ctrCount];

    ctrCount = 0;
    for (i = 0; i < glyph->contourCount; i++)
      {       if (glyph->ep[i] - glyph->sp[i] > 1)
        {   int     srcIndex = glyph->sp[i];
            int ptCount = glyph->ep[i] - srcIndex + 1;
            int ctrlBitLongs = hsPathContour::ControlBitLongs(ptCount);

            path->fContours[ctrCount].fPointCount       = ptCount;
            path->fContours[ctrCount].fPoints           = new hsPoint2[ptCount];
            path->fContours[ctrCount].fControlBits      = new UInt32[ctrlBitLongs];

            HSMemory::Clear(path->fContours[ctrCount].fControlBits, ctrlBitLongs * sizeof(UInt32));

            for (j = 0; j < ptCount; j++)
              {       path->fContours[ctrCount].fPoints[j].fX = F26Dot6_To_Scalar(glyph->x[srcIndex + j]);
                  path->fContours[ctrCount].fPoints[j].fY = F26Dot6_To_Scalar(- glyph->y[srcIndex + j]);

                  if (glyph->onCurve[srcIndex + j] == false)
                    {   int     ctrlBitIndex = j >> 5;
                        int ctrlBitShift = 31 - (j & 31);
                        path->fContours[ctrCount].fControlBits[ctrlBitIndex] |= (1UL << ctrlBitShift);
                    }
              }
            ctrCount += 1;
        }
      }
}

void t2kScalerContext::GeneratePath(UInt16 index, hsPathSpline* path,
                                    hsFixedPoint2* advance)
{
    int         errCode;
    T2K*        t2k = this->SetupTrans();

    T2K_RenderGlyph(t2k, index, 0, 0, fGreyLevel,
                    (fT2KFlags | T2K_RETURN_OUTLINES) & ~T2K_GRID_FIT, &errCode);
    hsIfDebugMessage(errCode, "T2K_RenderGlyph failed", errCode);

    if (advance)
      {       if (fDoFractEnable)
        {   advance->fX     = t2k->xLinearAdvanceWidth16Dot16;
            advance->fY = -t2k->yLinearAdvanceWidth16Dot16;
        }
              else
                {   advance->fX     = t2k->xAdvanceWidth16Dot16;
                    advance->fY = -t2k->yAdvanceWidth16Dot16;
                }
      }

    ConvertGlyphToPath(t2k->glyph, path);

    T2K_PurgeMemory(t2k, 1, &errCode);
    hsIfDebugMessage(errCode, "T2K_PurgeMemory failed", errCode);
}

Boolean t2kScalerContext::GetGlyphPoint(UInt16 glyphID, Int32 pointNumber, hsFixedPoint2 *point)
{
    Boolean result = false;
    int errCode;
    T2K *t2k = this->SetupTrans();

    T2K_RenderGlyph(t2k, glyphID, 0, 0, fGreyLevel,
                    fT2KFlags | T2K_RETURN_OUTLINES | T2K_SCAN_CONVERT, &errCode);
    hsIfDebugMessage(errCode, "T2K_RenderGlyph failed", errCode);

    if (!t2k->embeddedBitmapWasUsed && point != 0) {
        if (pointNumber < t2k->glyph->pointCount) {
            point->fX = t2k->glyph->x[pointNumber] << 10;
            point->fY = t2k->glyph->y[pointNumber] << 10;
            result = true;
        }
    }

    T2K_PurgeMemory(t2k, 1, &errCode);  // to relase the bitmap/outline
    hsIfDebugMessage(errCode, "T2K_PurgeMemory failed", errCode);

    return result;
}

void t2kScalerContext::TransformFunits(Int16 xFunits, Int16 yFunits, hsFixedPoint2 *pixels)
{
    hsFixedPoint2 xPixels, yPixels;
    T2K *t2k = this->SetupTrans();

    T2K_TransformXFunits(t2k, xFunits, &xPixels.fX, &xPixels.fY);
    T2K_TransformYFunits(t2k, yFunits, &yPixels.fX, &yPixels.fY);

    pixels->fX = xPixels.fX + yPixels.fX;
    pixels->fY = xPixels.fY + yPixels.fY;
}

static void ProjectUnitVector(hsFixed projX, hsFixed projY,
                              hsFract dirX, hsFract dirY,
                              hsFract baseX, hsFract baseY,
                              hsFixedPoint2* result)
{
    hsFixed     dist = hsFracMul(projX, baseY) - hsFracMul(projY, baseX);
    hsFixed     scale = hsFracDiv(dist,
                                  hsFracMul(dirX, baseY) - hsFracMul(dirY, baseX));

    result->Set(hsFracMul(dirX, scale), hsFracMul(dirY, scale));
}

/* #define ROUNDAWAYFROMZERO(x)   (*x = (*x > 0)? \
                                 (hsIntToFixed((*x) + 0xffff >> 16)) : \
                                 (-hsIntToFixed((-*x) + 0xffff >> 16)))
								 */

#define ROUNDAWAYFROMZERO(x)

void t2kScalerContext::GetLineHeight(hsFixedPoint2* ascent,
                                     hsFixedPoint2* descent,
                                     hsFixedPoint2* baseline,
                                     hsFixedPoint2* leading,
                                     hsFixedPoint2* maxAdvance)
{
    T2K*        t2k = this->SetupTrans();

    hsFixed     mag = hsMagnitude(t2k->caretDx, t2k->caretDy);
    hsFract     caretX = hsFracDiv(t2k->caretDx, mag);
    hsFract     caretY = hsFracDiv(t2k->caretDy, mag);

    mag = hsMagnitude(t2k->xMaxLinearAdvanceWidth, t2k->yMaxLinearAdvanceWidth);
    hsFract     baseX = hsFracDiv(t2k->xMaxLinearAdvanceWidth, mag);
    hsFract     baseY = hsFracDiv(t2k->yMaxLinearAdvanceWidth, mag);

    if (ascent) {
        ProjectUnitVector(      t2k->xAscender + (t2k->xLineGap >> 1),
                                - t2k->yAscender - (t2k->yLineGap >> 1),
                                caretX, - caretY, baseX, - baseY, ascent);
        if (!fDoFractEnable) {
            ROUNDAWAYFROMZERO(&ascent->fX);
            ROUNDAWAYFROMZERO(&ascent->fY);
        }
    }
    if (descent) {
        ProjectUnitVector(      t2k->xDescender + (t2k->xLineGap >> 1),
                                - t2k->yDescender - (t2k->yLineGap >> 1),
                                - caretX, caretY, baseX, - baseY, descent);
        if (!fDoFractEnable) {
            ROUNDAWAYFROMZERO(&descent->fX);
            ROUNDAWAYFROMZERO(&descent->fY);
        }
    }
    if (baseline) {
        baseline->Set(baseX, - baseY);
        if (!fDoFractEnable) {
            ROUNDAWAYFROMZERO(&baseline->fX);
            ROUNDAWAYFROMZERO(&baseline->fY);
        }
    }
    if (leading) {
        ProjectUnitVector(      t2k->xLineGap,
                                - t2k->yLineGap,
                                - caretX, caretY, baseX, - baseY, leading);
//         if (!fDoFractEnable) {
//             leading->fX = hsIntToFixed(hsFixedRound(leading->fX));
//             leading->fY = hsIntToFixed(hsFixedRound(leading->fY));
//         }
    }
    if (maxAdvance) {
        maxAdvance->Set(t2k->xMaxLinearAdvanceWidth,
                        t2k->yMaxLinearAdvanceWidth);
        if (!fDoFractEnable) {
            ROUNDAWAYFROMZERO(&maxAdvance->fX);
            ROUNDAWAYFROMZERO(&maxAdvance->fY);
        }
    }
}

UInt32 t2kScalerContext::CountGlyphs()
{
    return fT2KEntry->fNumGlyphs;
}

void
t2kScalerContext::GetCaretAngle(hsFixedPoint2* caret)
{
    this->GetLineHeight(caret, nil, nil, nil, nil);
    caret->fY = -caret->fY;
}

/////////////////////////////////////////////////////////////////////////////////////

hsGScalerContext* t2kFontScaler::CreateContext(hsConstDescriptor desc)
{
    const FontStrikeDesc* r
      = (FontStrikeDesc*)hsDescriptor_Find(desc, kFontStrikeDescTag, nil,
                                           nil);

    return new t2kScalerContext(r->fFontObj, r->fMatrix, r->fDoAntiAlias,
                                r->fDoFracEnable, r->fStyle);
}

/////////////////////////////

class type1CharToGlyphMapper : public CharToGlyphMapper {
    fontObject* fFont;
    T2K * ft2k;
  public:
    type1CharToGlyphMapper(fontObject* fo, T2K * t2k) : fFont(fo), ft2k(t2k) {
    }

    virtual ~type1CharToGlyphMapper() {
    }

    virtual void CharsToGlyphs(
        int count, const Unicode16 unicodes[], UInt32 glyphs[]) const
      {
	for (int i = 0; i < count; i++){
	  Unicode16 high = unicodes[i];
	  Unicode32 code = high;

	  if (i < count - 1 && high >= 0xD800 && high <= 0xDBFF) {
	    Unicode16 low = unicodes[i + 1];

	    code = (high - 0xD800) * 0x400 + low - 0xDC00 + 0x10000;
	  }

	  glyphs[i] = (UInt32) T2K_GetGlyphIndex (ft2k, code);
#ifdef __solaris__
	  if (fFont->fGlyphToCharMap != NULL && code < 0x10000) {
	      fFont->fGlyphToCharMap[glyphs[i]] = (Unicode16)code;
	  }
#endif
	}
      }

    virtual void CharsToGlyphs(
        int count, const Unicode32 unicodes[], UInt32 glyphs[]) const
    {
       for (int i = 0; i < count; i++){
            glyphs[i] = (UInt32) T2K_GetGlyphIndex (ft2k, unicodes[i]);
       }
    }

    virtual int getMissingGlyphCode() const {
        return ft2k->font->T1->notdefGlyphIndex;
    }

    virtual Boolean canDisplay(Unicode ch) const {
        UInt32 glyph;
        if (ft2k == NULL) 
          return false;
        CharsToGlyphs(1, &ch, &glyph);
        return glyph != (UInt32)getMissingGlyphCode();
    }
};

///////////////////////////////////////////////////////////////////////////

class CMAPMapper : public CharToGlyphMapper {
  private:
    unsigned char *cmap;    // reference, not owned

  public:
    CMAPMapper(sfntFileFontObject* f) : cmap(NULL), font(f) {
      // with this need not maintain multiple copies of the cmap
      // for the same font - simplifies the issues for memory handling
      if ( f->fCmap == NULL ) {
          InitializeCMAP(*font, f->fCmap);
      }
      cmap = f->fCmap;
    }
    virtual void CharsToGlyphs(
        int count, const Unicode16 unicodes[], UInt32 glyphs[]) const;
    virtual void CharsToGlyphs(
        int count, const Unicode32 unicodes[], UInt32 glyphs[]) const;
    virtual Boolean canDisplay(Unicode ch) const {
        UInt32 glyph;
        CharsToGlyphs(1, &ch, &glyph);
        return glyph != (UInt32)getMissingGlyphCode();
    };
    ~CMAPMapper() {
#if 0
      // do the deallocation at the font level - not with multiple copies
        if ( cmap != NULL )
          delete [] cmap;
#endif
        cmap = NULL;
    }

  private:
    sfntFileFontObject* font;
};

void
CMAPMapper::CharsToGlyphs(
    int count, const Unicode16 unicodes[], UInt32 glyphs[]) const
{
    ConvertUnicodeToGlyphs(*font, (unsigned char *)cmap, count, unicodes, glyphs);
}

void
CMAPMapper::CharsToGlyphs(
        int count, const Unicode32 unicodes[], UInt32 glyphs[]) const
{
    ConvertUnicodeToGlyphs(*font, (unsigned char *)cmap, count, unicodes, glyphs);
}

//////////////////////////////////////////////////////////////////////////////

CharToGlyphMapper*
t2kScalerContext::getMapper()
{
    CharToGlyphMapper *gm = NULL;

    switch (fFont->GetFormat()) {
      case kTrueTypeFontFormat:
        if (!ttMapper) {
            ttMapper = new CMAPMapper((sfntFileFontObject *)fFont);
        }
        gm = ttMapper;
        break;

      case kType1FontFormat:
        if (!t1Mapper) {
            t1Mapper = new type1CharToGlyphMapper(fFont, fT2KEntry->GetT2K());
        }
        gm = t1Mapper;
        break;

      default:
        break;
    }
    return gm;
}

Boolean
t2kScalerContext::isStyleSupported(int aStyle)
{
    return (aStyle & ~(java_awt_Font_BOLD | java_awt_Font_ITALIC)) == 0;
}



/*
 * hasBitmaps:  Indicates whether the context has embedded bitmaps
 */
Boolean
t2kScalerContext::hasBitmaps(void) {
    static char * noBitmaps = getenv("JAVA2D_NOFONTBITMAPS");
    if (fT2KEntry && !noBitmaps) {
        T2K * t2k = this->SetupTrans();
        if (t2k->enableSbits)
          return true;
    }
    return false;
}


/*
 * hasBitmaps:  Indicates whether the glyph has an embedded bitmap
 */
Boolean
t2kScalerContext::hasBitmaps(int glyph) {
    int  err;
    if (fT2KEntry) {
        T2K * t2k = this->SetupTrans();
        if (T2K_GlyphSbitsExists(t2k, (tt_uint16)glyph, &err))
          return true;
    }
    return false;
}
