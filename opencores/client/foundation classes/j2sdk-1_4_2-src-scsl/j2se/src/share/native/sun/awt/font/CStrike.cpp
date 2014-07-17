/*
 * @(#)CStrike.cpp	1.5 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifdef __solaris__
#include "Strike.h"
#include <string.h>
#include <java_awt_Font.h>
#include "CStrike.h"

#define LATINCACHESIZE 256

/*
 * CStrike should only be created for non-composite fonts.
 * It is used when a TT or T1 font is accessed via native APIs
 * It provides for cases where the native API cannot access all the
 * font, trying multiple scalers.
 * CStrike does all its own initialisation, using only the default
 * super-class constructor.
 */
CStrike::CStrike(fontObject& fo, FontTransform& tx,
		 jboolean isAntiAliased, jboolean usesFractionalMetrics) 
    : Strike(fo),
      nrStrikes(NULL)

{
    enum FontFormats format = fFont.GetFormat();
    assert( (format == kTrueTypeFontFormat || format == kType1FontFormat) &&
	    (isAntiAliased == JNI_FALSE));

    /* Allocate the T2K strike */
    UInt32  sizes[2] = { sizeof(UInt32), sizeof(FontStrikeDesc) };

    hsDescriptor desc = hsDescriptor_New(2, sizes);
    FontStrikeDesc* javaDesc;

    /* there may be 2 scalers registered with Headspin cache : T2K & native.
     * In this case we always want to use T2K, not native.
     * Note that later when native strikes are created they will have
     * a different scaler id so will be distinct in the cache.
     */
    hsDescriptor_Add32(desc, kScalerID_FontScalerDesc, kT2KFontFormat);
    javaDesc = (FontStrikeDesc *)hsDescriptor_Add(desc,
                                                kFontStrikeDescTag,
                                                sizeof(FontStrikeDesc));
    javaDesc->fFontObj = (fontObject *)&fFont;
    tx.getMatrixInto(javaDesc->fMatrix, 4);
    javaDesc->fDoAntiAlias = isAntiAliased;
    javaDesc->fDoFracEnable = usesFractionalMetrics;
    javaDesc->fStyle = fFont.m_currentStyle;
    javaDesc->fIndex = 0; // not used for scalerID=kT2KFontFormat

    hsDescriptor_UpdateCheckSum(desc);
    strike = hsGGlyphStrike::RefStrike(desc);
    hsDescriptor_Delete(desc);  

    /* Initialise map used to get char codes back from T2K glyph codes */
    if (fo.fGlyphToCharMap == NULL) {
	fo.fGlyphCharMapLen = strike->CountGlyphs();
	if (fo.fGlyphCharMapLen >= 0) {
	    fo.fGlyphToCharMap = new Unicode16[fo.fGlyphCharMapLen];
	    for (int i=0; i<fo.fGlyphCharMapLen; i++) {
		fo.fGlyphToCharMap[i] = (Unicode16)0;
            }
	} else {
	    fo.fGlyphCharMapLen = 0;
	}
    }

    templateDesc.fFontObj = (fontObject *)&fFont;
    tx.getMatrixInto(templateDesc.fMatrix, 4);
    templateDesc.fDoAntiAlias = isAntiAliased;
    templateDesc.fDoFracEnable = usesFractionalMetrics;
    templateDesc.fStyle = fFont.m_currentStyle;
    templateDesc.fIndex = 0;

    /* Allocate the native strike array, populate as needed */
    if (!nrStrikes) {
        int numStrikes = fFont.GetNumberOfNativeNames();
        if (numStrikes <= DEFAULT_STRIKE_SLOTS) {
            nrStrikes = defaultStrikes;
        } else {
            nrStrikes = new Strike *[numStrikes];
        }
        if (nrStrikes) {
            for (int i=0; i < numStrikes; i++) {
                nrStrikes[i] = UNALLOCATEDSTRIKE;
            }
        }
    }
}

Boolean
CStrike::getMetricsWithImage(int glyphCode, hsGGlyph& glyphRef,
			     hsFixedPoint2& advXY, Boolean* refcheck)
{
    Boolean retval = false;
    int gc = glyphCode;

    hsGGlyphStrike *theStrike = getStrikeForGlyph(gc);

    if (theStrike) {
        theStrike->GetMetricsWithImage(gc, &glyphRef, &advXY, refcheck);
        retval = true;
    }

    return retval;
}

const void *
CStrike::getImage(int glyphCode, Boolean* refcheck)
{
    const void *retval = NULL;
    int gc = glyphCode;
    hsGGlyphStrike *theStrike = getStrikeForGlyph(gc);

    if (theStrike) {
        retval = theStrike->GetImage(gc, refcheck);
    }
    return retval;
}

const void *
CStrike::refImage(int glyphCode)
{
    const void *retval = NULL;
    int gc = glyphCode;

    hsGGlyphStrike *theStrike = getStrikeForGlyph(gc);

    if (theStrike) {
        retval = theStrike->RefImage(gc);
    }
    return retval;
}

/*
 * Caution: this method mutates the glyphCode parameter to be a glyph
 * in the strike returned. This is needed where the returned strike
 * refers to a native scaler context. It is required that the glyphCode on
 * entry be a T2K glyphcode.
 */
hsGGlyphStrike *
CStrike::getStrikeForGlyph(int& glyphCode) {
    // the glyph code obtained from the font by T2K will not be
    // usable with native scaler context. Need to map it back to the
    // char code.
    Unicode16 charCode = 0;
    if (fFont.fGlyphToCharMap != NULL && glyphCode < fFont.fGlyphCharMapLen) {
	charCode = fFont.fGlyphToCharMap[glyphCode];
    }

    if (charCode == 0) {
	return strike;
    } else {
	int numStrikes = fFont.GetNumberOfNativeNames();
	for (int ns=0; ns < numStrikes; ns++) {
	    if (nrStrikes[ns] == NULL) {
	        continue; // this was already tried - don't try it again.
	    }
	    if (nrStrikes[ns] == UNALLOCATEDSTRIKE) {//create the native strike
		// REMIND: what about the array of descs?
		FontTransform tx(templateDesc.fMatrix);
		if (debugFonts) {
		    fprintf(stderr,"creating strike=%d nn=%s\n", ns,
			    fFont.GetFontNativeName(ns));
		}
		nrStrikes[ns] =	new Strike(*templateDesc.fFontObj,
					   kNRFontFormat, tx,
					   templateDesc.fDoAntiAlias,
					   templateDesc.fDoFracEnable, ns);
		if (nrStrikes[ns]->GetNumGlyphs() <=0) {
 		    delete nrStrikes[ns];
		    // once this is assigned NULL we will not try again.
		    nrStrikes[ns] = NULL;
		    continue;
		}
		nrStrikes[ns]->latinCharsToGlyphs = new UInt32[LATINCACHESIZE];
		if (nrStrikes[ns]->latinCharsToGlyphs) {
		    memset(nrStrikes[ns]->latinCharsToGlyphs,
			   0, sizeof(UInt32) * LATINCACHESIZE);
		}

	    }
	    // test that the required glyph is displayable in this mapping.
	    // if the missing glyph is returned it means it can't be.
	    // any other value is interpreted as a valid glyph id.
	    // the test is expensive and slows text rendering markedly so
	    // for latin characters cache the result.
	    // REMIND: improve this for other text in later release
	    UInt32 gc = 0;
	    if (charCode < LATINCACHESIZE) {
	        gc = nrStrikes[ns]->latinCharsToGlyphs[charCode];
	    }
	    if (gc == 0) {
	        gc = nrStrikes[ns]->CharToGlyph(charCode);
		if (charCode < LATINCACHESIZE) {
		    nrStrikes[ns]->latinCharsToGlyphs[charCode] = gc;
	      }
	    }

	     if (gc != nrStrikes[ns]->getMissingGlyphCode()) {
	         glyphCode = (int)gc;
		 if (debugFonts) {
		     fprintf(stderr,"using strike=%d\n", ns);
		}
		return nrStrikes[ns]->getHSStrike();
	    }
	}
    }
    // if not found a native strike/scaler, return T2K
    return strike;
}

#endif /* __solaris__ */
