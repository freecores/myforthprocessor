/*
 * @(#)Strike.cpp	1.54 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * @author Charlton Innovations, Inc.
 */

#include "Strike.h"
#ifdef __solaris__
#include "CStrike.h"
#endif
#include "CompositeFont.h"
#include <string.h>
#include <java_awt_Font.h>

//////////////////////////////////////////////////////////////////////////////

const unsigned int NPLANES      = 256 * 17;

const unsigned int SLOTMASK     = 0xff000000;
const unsigned int GLYPHMASK    = 0x00ffffff;
#define GLYPHSLOT(g)            ((g >> 24) & 0xff)
#define GLYPHCODE(s, g) \
        (( (unsigned int)s << 24) | ((unsigned int)g & GLYPHMASK))

#define UNINITIALIZED_GLYPH ((UInt32)-1)

#ifdef __solaris__
int useNativeScaler();
int isUniformScale(FontTransform& tx);
#endif

class CompositeGlyphMapper : public CharToGlyphMapper {
private:
    class GlyphMapEntry {
    public:
//      int     fSlot;      // not needed, slot is coded in fGlyphCode
        UInt32  fGlyphCode;
    };

private:
    CompositeFont&  fCompositeFont;
    const Strike&   fStrike;
    UInt32          fMissingGlyph;
    int             fNumSlots;
    GlyphMapEntry   *fGlyphMaps[NPLANES];

private:
    CompositeGlyphMapper();                         // no implementation
    CompositeGlyphMapper(CompositeGlyphMapper&);    // no implementation
    CompositeGlyphMapper& operator =(const CompositeGlyphMapper&);
    UInt32 getCachedGlyphCode(const Unicode32 unicode) const;
    void setCachedGlyphCode(const Unicode32 unicode,
                            const UInt32 glyphCode,
                            const int slot) const;

public:
    virtual ~CompositeGlyphMapper() {
        for (unsigned int i=0; i < NPLANES; i++) {
            if (fGlyphMaps[i]) {
                delete [] fGlyphMaps[i];
                fGlyphMaps[i] = NULL;
            }
        }
    };

    CompositeGlyphMapper(CompositeFont& cf, const Strike& strike)
            : fCompositeFont(cf), fStrike(strike) {
        fMissingGlyph = UNINITIALIZED_GLYPH;
        fNumSlots = fCompositeFont.countSlots();
        memset(fGlyphMaps, 0, sizeof(fGlyphMaps));
    };

    virtual void CharsToGlyphs(
        int count, const Unicode16 unicodes[], UInt32 glyphs[]) const;

    virtual void CharsToGlyphs(
        int count, const Unicode32 unicodes[], UInt32 glyphs[]) const;
    virtual Boolean canDisplay(Unicode ch) const;
    virtual int getMissingGlyphCode() const;
};
//////////////////////////////////////////////////////////////////////////////

Strike::Strike(fontObject& fo, FontTransform& tx,
	       jboolean isAntiAliased, jboolean usesFractionalMetrics)
  :   fFont(fo),
      strike(NULL),
      fMapper(NULL),
      fCachedLineMetrics(JNI_FALSE),
      compositeStrike(NULL),
      isComposite(JNI_FALSE),
      totalCompositeGlyphs(0),
#ifdef __solaris__
      latinCharsToGlyphs(NULL),
#endif
      fStrikeDescs(NULL)
{
    enum FontFormats format = fFont.GetFormat();
    enum FontFormats scalerID = ((fontObject&)fFont).getScalerID(
                                            tx,
                                            isAntiAliased,
                                            usesFractionalMetrics);
    if (format == kCompositeFontFormat) {
        isComposite = JNI_TRUE;
        templateDesc.fFontObj = NULL;
        tx.getMatrixInto(templateDesc.fMatrix, 4);
        templateDesc.fDoAntiAlias = isAntiAliased;
        templateDesc.fDoFracEnable = usesFractionalMetrics;
        templateDesc.fStyle = fFont.m_currentStyle;
	templateDesc.fIndex = 0;
        return;
    }

    UInt32  sizes[2] = { sizeof(UInt32), sizeof(FontStrikeDesc) };

    hsDescriptor desc = hsDescriptor_New(2, sizes);
    FontStrikeDesc* javaDesc;

    hsDescriptor_Add32(desc, kScalerID_FontScalerDesc, scalerID);
    javaDesc = (FontStrikeDesc *)hsDescriptor_Add(desc,
                                                kFontStrikeDescTag,
                                                sizeof(FontStrikeDesc));
    javaDesc->fFontObj = (fontObject *)&fFont;
    tx.getMatrixInto(javaDesc->fMatrix, 4);
    javaDesc->fDoAntiAlias = isAntiAliased;
    javaDesc->fDoFracEnable = usesFractionalMetrics;
    javaDesc->fStyle = fFont.m_currentStyle;
    javaDesc->fIndex = 0;

    hsDescriptor_UpdateCheckSum(desc);

    strike = hsGGlyphStrike::RefStrike(desc);
    
    hsDescriptor_Delete(desc);
}

Strike::Strike(fontObject& fo, enum FontFormats scalerID, FontTransform& tx,
	       jboolean isAntiAliased, jboolean usesFractionalMetrics,
	       int encodingIndex)
  :   fFont(fo), strike(NULL), 
      fMapper(NULL), 
      fCachedLineMetrics(JNI_FALSE),
      compositeStrike(NULL), isComposite(JNI_FALSE),
      totalCompositeGlyphs(0),
#ifdef __solaris__
      latinCharsToGlyphs(NULL),
#endif
      fStrikeDescs(NULL)
{
    enum FontFormats format = fFont.GetFormat();
    if (format == kCompositeFontFormat) {
        isComposite = JNI_TRUE;
        templateDesc.fFontObj = NULL;
        tx.getMatrixInto(templateDesc.fMatrix, 4);
        templateDesc.fDoAntiAlias = isAntiAliased;
        templateDesc.fDoFracEnable = usesFractionalMetrics;
        templateDesc.fStyle = fFont.m_currentStyle;
	templateDesc.fIndex = 0;
        return;
    }

    UInt32  sizes[2] = { sizeof(UInt32), sizeof(FontStrikeDesc) };

    hsDescriptor desc = hsDescriptor_New(2, sizes);
    FontStrikeDesc* javaDesc;

    hsDescriptor_Add32(desc, kScalerID_FontScalerDesc, scalerID);
    javaDesc = (FontStrikeDesc *)hsDescriptor_Add(desc,
                                                kFontStrikeDescTag,
                                                sizeof(FontStrikeDesc));
    javaDesc->fFontObj = (fontObject *)&fFont;
    tx.getMatrixInto(javaDesc->fMatrix, 4);
    javaDesc->fDoAntiAlias = isAntiAliased;
    javaDesc->fDoFracEnable = usesFractionalMetrics;
    javaDesc->fStyle = fFont.m_currentStyle;
    if (scalerID == kNRFontFormat) {
	javaDesc->fIndex = encodingIndex;
    } else {
	javaDesc->fIndex = 0;
    }

    hsDescriptor_UpdateCheckSum(desc);

    strike = hsGGlyphStrike::RefStrike(desc);

    hsDescriptor_Delete(desc);
}

Strike::~Strike() {
    if (strike) {
        hsRefCnt_SafeUnRef(strike);
        strike = NULL;
    } else {
        if (fMapper) {
            delete fMapper;
        }
    }
    if (compositeStrike) {
        CompositeFont *cf = (CompositeFont *)&fFont;
        int numSlots = cf->countSlots();
        for (int i=0; i < numSlots; i++) {
            if (compositeStrike[i]) {
                delete compositeStrike[i];
                compositeStrike[i] = NULL;
            }
        }
        if (compositeStrike != defaultStrikes) {
            delete [] compositeStrike;
        }
        compositeStrike = NULL;
    }
    if (fStrikeDescs) {
        if (fStrikeDescs != defaultStrikeDescs) {
            delete [] fStrikeDescs;
        }
        fStrikeDescs = NULL;
    }
#ifdef __solaris__
    if (latinCharsToGlyphs) {
      delete [] latinCharsToGlyphs;
    }
#endif
}

Boolean
Strike::getOutlineBounds(int glyphCode, hsRect& bounds)
{
    Boolean retval = false;
    hsGGlyphStrike *theStrike = strike;
    if (!theStrike) {
        theStrike = compositeStrikeForGlyph(glyphCode);
    }
    if (theStrike) {
        theStrike->GetOutlineBounds(glyphCode, &bounds);
        retval = true;
    }
    return retval;
}

Boolean
Strike::getMetrics(int glyphCode, hsGGlyph& glyphRef, hsFixedPoint2& advXY)
{
    Boolean retval = false;
    hsGGlyphStrike *theStrike = strike;
    if (!theStrike) {
        theStrike = compositeStrikeForGlyphImage(glyphCode);
    }
    if (theStrike) {
        theStrike->GetMetrics(glyphCode, &glyphRef, &advXY);
        retval = true;
    }
    return retval;
}

Boolean
Strike::getMetricsWithImage(int glyphCode, hsGGlyph& glyphRef, hsFixedPoint2& advXY, Boolean* refcheck)
{
    Boolean retval = false;
    hsGGlyphStrike *theStrike = strike;

    if (!theStrike) {
        theStrike = compositeStrikeForGlyphImage(glyphCode);
    }
    if (theStrike) {
        theStrike->GetMetricsWithImage(glyphCode, &glyphRef, &advXY, refcheck);
        retval = true;
    }

    return retval;
}

const void *
Strike::getImage(int glyphCode, Boolean* refcheck)
{
    const void *retval = NULL;
    hsGGlyphStrike *theStrike = strike;

    if (!theStrike) {
        theStrike = compositeStrikeForGlyphImage(glyphCode);
    }
    if (theStrike) {
        retval = theStrike->GetImage(glyphCode, refcheck);
    }
    return retval;
}

const void *
Strike::refImage(int glyphCode)
{
    const void *retval = NULL;
    hsGGlyphStrike *theStrike = strike;

    if (!theStrike) {
        theStrike = compositeStrikeForGlyphImage(glyphCode);
    }
    if (theStrike) {
        retval = theStrike->RefImage(glyphCode);
    }
    return retval;
}

void
Strike::prepareCache(int numGlyphs, Boolean* purged) {
  hsGGlyphStrike *theStrike = strike;
  if (!theStrike) {
    // assume strike 0 will do
    theStrike = compositeStrikeForSlot(0);
  }
  if (theStrike) {
    theStrike->PrepareCache(numGlyphs, purged);
  }
}

static void useMax(hsFixedPoint2& target, hsFixedPoint2& src) {
    if (src.fX > target.fX) {
        target.fX = src.fX;
    }
    if (src.fY > target.fY) {
        target.fY = src.fY;
    }
}

static void useMin(hsFixedPoint2& target, hsFixedPoint2& src) {
    if (src.fX < target.fX) {
        target.fX = src.fX;
    }
    if (src.fY < target.fY) {
        target.fY = src.fY;
    }
}

void
Strike::GetLineHeight(hsFixedPoint2& ascent,
                      hsFixedPoint2& descent,
                      hsFixedPoint2& baseline,
                      hsFixedPoint2& leading,
                      hsFixedPoint2& maxAdvance)
{
    if (fCachedLineMetrics) {
        ascent      = fAscent;
        descent     = fDescent;
        baseline    = fBaseline;
        leading     = fLeading;
        maxAdvance  = fMaxAdvance;
        return;
    }

    if (strike) {
        strike->GetLineHeight(&ascent, &descent,
                              &baseline, &leading, &maxAdvance);
    } else {
        ascent.fX       = 0;
        ascent.fY       = 0;
        descent.fX      = 0;
        descent.fY      = 0;
        baseline.fX     = 0;
        baseline.fY     = 0;
        leading.fX      = 0;
        leading.fY      = 0;
        maxAdvance.fX   = 0;
        maxAdvance.fY   = 0;
        if (isComposite) {
            CompositeFont *cf = (CompositeFont *)&fFont;
            int numSlots = cf->countSlots();
            for (int i=0; i < numSlots; i++) {
                hsFixedPoint2 lAscent = {0, 0};
                hsFixedPoint2 lDescent = {0, 0};
                hsFixedPoint2 lBaseline = {0, 0};
                hsFixedPoint2 lLeading = {0, 0};
                hsFixedPoint2 lMaxAdvance = {0, 0};

                hsGGlyphStrike *currentStrike = compositeStrikeForSlot(i);
                if (currentStrike) {
                    currentStrike->GetLineHeight(&lAscent, &lDescent,
                                                &lBaseline,
                                                &lLeading, &lMaxAdvance);
                }
                useMin(ascent,      lAscent);
                useMax(descent,     lDescent);
                useMax(baseline,    lBaseline);
                useMin(leading,     lLeading);
                useMax(maxAdvance,  lMaxAdvance);
            }
        }
    }
    fCachedLineMetrics = JNI_TRUE;
    fAscent      = ascent;
    fDescent     = descent;
    fBaseline    = baseline;
    fLeading     = leading;
    fMaxAdvance  = maxAdvance;
}

void
Strike::GetItalicAngle(hsFixedPoint2& italicAngle)
{
    hsGGlyphStrike *theStrike = strike;
    if (!theStrike) {
        int glyphCode = 0;
        theStrike = compositeStrikeForGlyph(glyphCode);
    }
    if (theStrike) {
        theStrike->GetItalicAngle(&italicAngle);
    } else {
        italicAngle.fX = 0;
        italicAngle.fY = 0;
    }
}

int
Strike::canRotate ()
{
    int retval = sun_awt_font_NativeFontWrapper_ROTATE_FALSE;

    if (!strike) {
        retval = compositeCanRotate ();
    } else {
        retval = strike->canRotate ();
    }

    return retval;
}

Boolean
Strike::canRotateString (int fNumGlyphs, UInt32 *glyphs)
{
    Boolean retval = true;

    for (int i = 0; (i < fNumGlyphs) && (retval); i++) {
        int glyphCode = glyphs [i];
        hsGGlyphStrike *theStrike = strike;
        if (!theStrike) {
            theStrike = compositeStrikeForGlyph (glyphCode);
        }
        if (theStrike) {
            retval = theStrike->canRotate ();
        } else {
            retval = false;
        }
    }
    return retval;
}

void
Strike::GetGlyphOutline(int glyphCode, hsPathSpline& path)
{
    hsGGlyphStrike *theStrike = strike;
    if (!theStrike) {
        theStrike = compositeStrikeForGlyph(glyphCode);
    }
    if (theStrike) {
        theStrike->GetPath(glyphCode, &path, nil);
    }
}

Boolean
Strike::GetGlyphPoint(int glyphCode, int pointNumber, hsFixedPoint2 &point)
{
    hsGGlyphStrike *theStrike = strike;

    if (theStrike == 0) {
        theStrike = compositeStrikeForGlyph(glyphCode);
    }

    if (theStrike != 0) {
        return theStrike->GetGlyphPoint(glyphCode, pointNumber, &point);
    }

    return false;
}

void
Strike::TransformFunits(int glyphCode, short xFunits, short yFunits, hsFixedPoint2 &pixels)
{
    hsGGlyphStrike *theStrike = strike;

    if (theStrike == 0) {
        theStrike = compositeStrikeForGlyph(glyphCode);
    }

    if (theStrike != 0) {
        theStrike->TransformFunits(xFunits, yFunits, &pixels);
    }
}
int Strike::GetNumGlyphs()
{
    int retval = 0;
    if (strike) {
        retval = strike->CountGlyphs();
    } else if (isComposite) {
        if (!totalCompositeGlyphs) {
            CompositeFont *cf = (CompositeFont *)&fFont;
            int numSlots = cf->countSlots();
            for (int i=0; i < numSlots; i++) {
                hsGGlyphStrike *slotStrike = compositeStrikeForSlot(i);
                if (slotStrike) {
                    totalCompositeGlyphs += slotStrike->CountGlyphs();
                }
            }
        }
        retval = totalCompositeGlyphs;
    }
    return retval;
}

void
Strike::allocateStrikes()
{
    if (!compositeStrike) {
        CompositeFont *cf = (CompositeFont *)&fFont;
        int numSlots = cf->countSlots();
        if (numSlots <= DEFAULT_STRIKE_SLOTS) {
            compositeStrike = defaultStrikes;
        } else {
            compositeStrike = new Strike *[numSlots];
        }
        if (compositeStrike) {
            for (int i=0; i < numSlots; i++) {
                compositeStrike[i] = NULL;
            }
        }
    }
}

void
Strike::allocateDescs()
{
    if (!fStrikeDescs) {
        CompositeFont *cf = (CompositeFont *)&fFont;
        int numSlots = cf->countSlots();
        if (numSlots <= DEFAULT_STRIKE_SLOTS) {
            fStrikeDescs = defaultStrikeDescs;
        } else {
            fStrikeDescs = new FontStrikeDesc[numSlots];
        }
        if (fStrikeDescs) {
            memset(fStrikeDescs, 0, numSlots * sizeof(FontStrikeDesc));
        }
    }
}

static int
CompAsciiUnicode(const char *aStr, const Unicode *uStr, int len) {
    if (!len || !aStr || !uStr) {
        return 0;
    }
    while (*aStr && len--) {
        if (*aStr++ != *uStr++) {
            return 0;
        }
    }
    return (*aStr == 0);
}


static Boolean EqualUnicodeStrings ( const Unicode *str1, int str1Len, 
                                     const Unicode *str2, int str2Len )
{
  if ( str1Len == str2Len ) {
    for ( int i = 0; i < str1Len; i++) {
      if ( str1[i] != str2[i] )
        return false;
    }
    return true;
  }
  return false;
}

static void printUnicode(const UInt16 *name, int nameLen) {
    for (int i=0; i < nameLen; i++) {
        printf( "%c", (char)name[i]);
    }
    printf ("\n" );
}

int
Strike::algorithmicStyle(
        const CompositeFont& cf, const fontObject& fo, int slot ) {
    int cfNameLen;
    const Unicode *cfName = ((fontObject&)cf).GetFontName(cfNameLen);
    if (!cfName) {
        return java_awt_Font_PLAIN;
    }
    for (int index=0; index < cfNameLen; index++) {
        if (cfName[index] == '.') {
            // it is a styled composite font
            fontObject *foPlain =
                ::FindFontObject(cfName, index, java_awt_Font_PLAIN);
            if (foPlain && (foPlain->GetFormat() == kCompositeFontFormat)) {
                // and root is also composite font
                CompositeFont *cfPlain = (CompositeFont *)foPlain;
                fontObject *foPlainSlot = cfPlain->getSlotFont(slot);
                if (foPlainSlot == &fo) {
                    // and root slot == styled slot
                    if (CompAsciiUnicode(".bolditalic",
                                    &cfName[index], cfNameLen-index)) {
                        return java_awt_Font_BOLD | java_awt_Font_ITALIC;
                    }
                    if (CompAsciiUnicode(".bold",
                                    &cfName[index], cfNameLen-index)) {
                        return java_awt_Font_BOLD;
                    }
                    if (CompAsciiUnicode(".italic",
                                    &cfName[index], cfNameLen-index)) {
                        return java_awt_Font_ITALIC;
                    }
                }
            }
            return java_awt_Font_PLAIN;
        }
    }
    return java_awt_Font_PLAIN;
}



void
Strike::needsAlgorithmicStyle(
        const CompositeFont& cf, int slot, Boolean *algorithmic) {
    int cfNameLen;
    const Unicode *cfName = ((fontObject&)cf).GetFontName(cfNameLen);
    if (!cfName) {
        return;
    }
    *algorithmic = false;
    //    printf ( " The slot being compared is = %d\n", slot );
    //    printf ( " The font name = " );
    //    printUnicode ( cfName, cfNameLen );
    fontObject *foSlot = ((CompositeFont& ) cf).getSlotFont(slot);
    if (foSlot == NULL) {
	return;
    }
    int styleLength =0;
    const Unicode *styleName = foSlot->GetFontName( styleLength );
    //    printf ( "The slot name is = " );
    //    printUnicode ( styleName, styleLength );

    for (unsigned int index=0; index < (unsigned int)cfNameLen; index++) {
        if (cfName[index] == '.') {
            // it is a styled composite font
	    // printf( " The font name being passed to FFO = " );
	    //   printUnicode ( cfName, index );
	    fontObject *foPlain = ::FindFontObject(cfName, index, java_awt_Font_PLAIN);
	    if ( !foPlain || foPlain->GetFormat() != kCompositeFontFormat ) {
	      // The japanese font.properties files names plain styles
	      // differently as dialog.plain.
	      // thus if it not a composite font - then there is a problem
	      // try appending the .plain to the name and get the font
	      // it needs to be a composite font
	        UInt16  tryPlainStr[256];
		int tryPlainStrLen = index;
		const char *plainStr = ".plain";

                unsigned int j;
                for ( j = 0; j < index; j++) {
                    tryPlainStr[j] = cfName[j];
                }
                for ( j = 0; j < strlen(plainStr) ; j++ ) {
                    tryPlainStr[j + index] = plainStr[j];
                }
                tryPlainStrLen += strlen(plainStr);
                // printf ( "The font plain object was not composite\n" );
                // printf ( "Try with the string " );
                // printUnicode ( tryPlainStr, tryPlainStrLen );
                foPlain = FindFontObject(tryPlainStr, tryPlainStrLen, java_awt_Font_PLAIN);
                // printf ( " Did another try with .plain and name = " );
                // int l = 0;
                // const Unicode *nm;
                // nm = foPlain->GetFontName(l);
                // printUnicode ( nm, l );
            }
              

            if (foPlain) {
                fontObject *foMatchingSlot = foPlain;
                if ( foPlain->GetFormat() == kCompositeFontFormat ) {
                    foMatchingSlot = ((CompositeFont *) foPlain)->getSlotFont(slot);
                }
		if (foMatchingSlot == NULL) {
		    return;
		}
                int plainLength = 0;
                const Unicode *plainName = foMatchingSlot->GetFontName(plainLength );
                // printf ( "The matching slot name is = " );
                // printUnicode ( plainName, plainLength );

                if (EqualUnicodeStrings (plainName, plainLength, styleName, styleLength )) {
                    // and root slot == styled slot
                    if (CompAsciiUnicode(".bolditalic",
                                    &cfName[index], cfNameLen-index)) {
                        *algorithmic = true;
                    }
                    else if (CompAsciiUnicode(".bold",
                                    &cfName[index], cfNameLen-index)) {
                        // printf ( " we are setting the flag here in bold\n" );
                        *algorithmic = true;
                    }
                    else if (CompAsciiUnicode(".italic",
                                    &cfName[index], cfNameLen-index)) {
                        *algorithmic = true;
                    }
                }
            }
            break;
        }
    }
}

hsGGlyphStrike *
Strike::compositeStrikeForSlotInt(int slot, int& glyphCode)
{
    hsGGlyphStrike *retval = NULL;
    CompositeFont *cf = (CompositeFont *)&fFont;
    int numSlots = cf->countSlots();
    if ((slot < 0) || (slot >= numSlots)) {
        return retval;
    }
    if (!compositeStrike) {
        allocateStrikes();
        if (!compositeStrike) {
            return retval;
        }
    }
    Strike *slotStrike = compositeStrike[slot];
    if (!slotStrike) {
        if (!fStrikeDescs) {
            allocateDescs();
        }
        if (!fStrikeDescs) {
            return retval;
        }
        FontStrikeDesc& desc = fStrikeDescs[slot];
        if (!desc.fFontObj) {
            desc = templateDesc;
            desc.fFontObj = cf->getSlotFont(slot);
            if (desc.fFontObj) {
                // if this is a styled composite font and this
                // slot's fontObject is the same as the "plain"
                // equivalent composite font, apply algorithmic
                // styles with a transform.
                FontTransform tx(desc.fMatrix);
                
                desc.fFontObj->m_currentStyle = cf->m_currentStyle;
                if (desc.fFontObj->m_currentStyle == java_awt_Font_PLAIN) {
                    desc.fFontObj->m_currentStyle = 
                                algorithmicStyle(*cf, *desc.fFontObj, slot );
                } else {
                    Boolean algorithmic = false;
                    needsAlgorithmicStyle (*cf, slot, &algorithmic);
                    if ( algorithmic == true ) {
                      // NOTE :: HACK HACK HACK ---
                      // by turning on this flag we are essentially turning on t2k
                      // rasterizing which can algorithmically bold stuff
                      // Windows will not algorithmicize stuff by the API's we
                      // are using - it always returns you the plain version of the
                      // font - so we insert this flag and force this to go thru t2k.
                      // printf ( "Need to algorithmicize this style \n" );
                      desc.fDoFracEnable = true;
                    }
                }
		enum FontFormats format = desc.fFontObj->GetFormat();
#ifdef __solaris__
		if ((format == kTrueTypeFontFormat ||
		     format == kType1FontFormat) &&
		    desc.fDoAntiAlias == JNI_FALSE &&
		    desc.fDoFracEnable == JNI_FALSE &&
		    isUniformScale(tx) &&
		    useNativeScaler() &&
		    !AWTIsHeadless() &&
		    desc.fFontObj->m_currentStyle == desc.fFontObj->GetRealStyle() && 
		    (desc.fFontObj->GetNumberOfNativeNames() > 0)) {
		    CStrike *cs = new CStrike(*desc.fFontObj, tx,
					      desc.fDoAntiAlias,
					      desc.fDoFracEnable); 	
                   compositeStrike[slot] = cs;
    } else {
#endif
	compositeStrike[slot] = new Strike(*desc.fFontObj, tx,
					   desc.fDoAntiAlias,
					   desc.fDoFracEnable);
#ifdef __solaris__
    }
#endif
            }
        }
        slotStrike = compositeStrike[slot];
    }
    if (slotStrike) {
	if (glyphCode > 0) {
	    retval = slotStrike->getStrikeForGlyph(glyphCode);
	} else {
	    retval = slotStrike->strike;
	}
    }
    return retval;
}


int
Strike::compositeCanRotate ()
{
    CompositeFont *cf = (CompositeFont *)&fFont;
    int numSlots = cf->countSlots();
    int retval = sun_awt_font_NativeFontWrapper_ROTATE_FALSE;
    if (numSlots > 0) {
        hsGGlyphStrike *tmpStrike = compositeStrikeForSlot (0);
        if (!tmpStrike) {
            return retval;
        }

        retval = tmpStrike->canRotate ();

        for (int i = 1; i < numSlots; i++) {
            tmpStrike = compositeStrikeForSlot (i);
            if (!tmpStrike) {
                return sun_awt_font_NativeFontWrapper_ROTATE_FALSE;
            } else {
                int val = tmpStrike->canRotate ();
                if (val != retval) {
                    return sun_awt_font_NativeFontWrapper_ROTATE_CHECK_STRING;
                }
            }
        }
    }

    return retval;
}


hsGGlyphStrike *
Strike::compositeStrikeForSlot(int slot)
{
    int gc = 0; // lvalue required.
    return compositeStrikeForSlotInt(slot, gc);
}

hsGGlyphStrike *
Strike::compositeStrikeForGlyph(int& glyphCode)
{
    int gc = 0; // lvalue required.
    return compositeStrikeForSlotInt(GLYPHSLOT(glyphCode), gc);
}


// This function perturbs the glyphCode reference parameter to not
// only strip the slot, but possibly to change the glyphCode to
// match the glyph in a native scaler context associated with the
// returned cached strike.
// Clients of this already expect the parameter to be perturbed so
// this shouldn't require any ripple-effect changes.
hsGGlyphStrike *
Strike::compositeStrikeForGlyphImage(int& glyphCode)
{
    int slot = GLYPHSLOT(glyphCode);
    glyphCode &= GLYPHMASK;
    return compositeStrikeForSlotInt(slot, glyphCode);
}

Boolean Strike::isStyleSupported(int aStyle)
{
    Boolean retCode = false;
    if ( NULL != strike ) {
        retCode = strike->isStyleSupported(aStyle);
    }
    return retCode;
}

hsPathSpline::Type Strike::GetPathType (int glyphCode)
{
    hsGGlyphStrike *theStrike = strike;
    if (!theStrike) {
        theStrike = compositeStrikeForGlyph(glyphCode);
    }
    if (theStrike) {
        return theStrike->GetPathType ();
    }

    return hsPathSpline::kQuadType;
}

#if 0
UInt32 Strike::CharToGlyph(Unicode16 unicode)
{
    if (getMapper()) {
        UInt32 glyph;
        fMapper->CharsToGlyphs(1, &unicode, &glyph);
        return glyph;
    }
    return getMissingGlyphCode();
}
#endif

UInt32 Strike::CharToGlyph(Unicode32 unicode)
{
    if (getMapper()) {
        UInt32 glyph;
        fMapper->CharsToGlyphs(1, &unicode, &glyph);
        return glyph;
    }
    return getMissingGlyphCode();
}

int Strike::getMissingGlyphCode() const
{
    if (getMapper()) {
        return fMapper->getMissingGlyphCode();
    }
    return 0;
}

Boolean Strike::canDisplay(Unicode ch)
{
    if (getMapper()) {
        return fMapper->canDisplay(ch);
    }
    return 0;
}

CharToGlyphMapper *Strike::getMapper() const
{
    if (fMapper) {
        return fMapper;
    }
    // override const for private cached readonly value
    Strike *myPtr = (Strike *)this;
    hsGGlyphStrike *theStrike = strike;
    if (!theStrike) {
        myPtr->fMapper = new CompositeGlyphMapper(
                                    (CompositeFont&)fFont, *this);
    } else {
        myPtr->fMapper = theStrike->getMapper();
    }
    return fMapper;
}

UInt32
CompositeGlyphMapper::getCachedGlyphCode(const Unicode32 unicode) const
{
    UInt32 hiByte = (unicode >> 8) & 0x1fff;

    if (hiByte <= 0x10ff && fGlyphMaps[hiByte]) {
        UInt32 loByte = unicode & 0xff;

        if (fGlyphMaps[hiByte][loByte].fGlyphCode) {
            return fGlyphMaps[hiByte][loByte].fGlyphCode;
        }
    }

    return UNINITIALIZED_GLYPH;
}

void
CompositeGlyphMapper::setCachedGlyphCode(const Unicode32 unicode,
                                         const UInt32 glyphCode,
                                         const int slot) const
{
    // not really const ... but changes no external behavior
    // except to boost performance...we override const to maintain
    // read-only cached state
    CompositeGlyphMapper *myPtr = (CompositeGlyphMapper *)this;
    UInt32 hiByte = (unicode >> 8) & 0x1fff;

        if (hiByte > 0x10ff) {
                // FIXME: some sort of error is called for here...
                return;
        }

    if (!fGlyphMaps[hiByte]) {
        myPtr->fGlyphMaps[hiByte] = new GlyphMapEntry[256];

        if (fGlyphMaps[hiByte]) {
            memset(myPtr->fGlyphMaps[hiByte], UNINITIALIZED_GLYPH, 256 * sizeof(GlyphMapEntry));
        }
    }

    if (fGlyphMaps[hiByte]) {
        UInt32 loByte = unicode & 0xff;

//      myPtr->fGlyphMaps[hiByte][loByte].fSlot      = slot;
        myPtr->fGlyphMaps[hiByte][loByte].fGlyphCode =
                                                GLYPHCODE(slot, glyphCode);
    }
}

void
CompositeGlyphMapper::CharsToGlyphs(
        int count, const Unicode16 unicodes[],
        UInt32 glyphs[]) const
{
    getMissingGlyphCode();
    if (count) {
        Strike& myStrike = (Strike&)fStrike;
        for (int i=0; i<count; i++) {
            Unicode16 high = unicodes[i];
            Unicode32 code = high;

            if (high >= 0xD800 && high <= 0xDBFF && i < count - 1) {
                Unicode16 low = unicodes[i + 1];

                if (low >= 0xDC00 && low <= 0xDFFF) {
                    code = (high - 0xD800) * 0x400 + low - 0xDC00 + 0x10000;
                    glyphs[i + 1] = 0xFFFF;
                }
            }

            glyphs[i] = getCachedGlyphCode(code);

            if (glyphs[i] == UNINITIALIZED_GLYPH) {
		glyphs[i] = fMissingGlyph;
                for (int slot = 0; slot < fNumSlots; slot++) {
                    hsGGlyphStrike *slotStrike =
                        myStrike.compositeStrikeForSlot(slot);
                    if (    slotStrike
                            && !fCompositeFont.isExcludedChar(slot, code))
                        {
                            CharToGlyphMapper *strikeMapper = slotStrike->getMapper();
                            if (strikeMapper) {
                                int glyphCode;
                                glyphCode = strikeMapper->CharToGlyph(code);
                                if (glyphCode != strikeMapper->getMissingGlyphCode()){
                                    glyphs[i] = GLYPHCODE(slot, glyphCode);
                                    setCachedGlyphCode(code, glyphCode, slot);
                                    break;
				}
			    }
                        }
		}
            }

            if (code >= 0x10000) {
                i += 1;
            }
        }
    }
}

void
CompositeGlyphMapper::CharsToGlyphs(
    int count, const Unicode32 unicodes[],
    UInt32 glyphs[]) const
{
    getMissingGlyphCode();
    if (count) {
        Strike& myStrike = (Strike&)fStrike;
        for (int i=0; i<count; i++) {
            Unicode32 code = unicodes[i];

            glyphs[i] = getCachedGlyphCode(code);

            if (glyphs[i] == UNINITIALIZED_GLYPH) {
		glyphs[i] = fMissingGlyph;
                for (int slot = 0; slot < fNumSlots; slot++) {
                    hsGGlyphStrike *slotStrike =
                        myStrike.compositeStrikeForSlot(slot);
                    if (    slotStrike
                            && !fCompositeFont.isExcludedChar(slot, code))
                        {
                            CharToGlyphMapper *strikeMapper = slotStrike->getMapper();
                            if (strikeMapper) {
                                int glyphCode;
                                glyphCode = strikeMapper->CharToGlyph(code);
                                if (glyphCode != strikeMapper->getMissingGlyphCode()){
                                    glyphs[i] = GLYPHCODE(slot, glyphCode);
                                    setCachedGlyphCode(code, glyphCode, slot);
                                    break;
				}
			    }
                        }
		}
            }
        }
    }
}

Boolean
CompositeGlyphMapper::canDisplay(Unicode ch) const
{
    UInt32 glyph = 0;
    CharsToGlyphs(1, &ch, &glyph);
    return (glyph&GLYPHMASK) != 0;
}

int
CompositeGlyphMapper::getMissingGlyphCode() const
{
    // some slots might be empty, so use missing glyph from first non-empty slot
    if (UNINITIALIZED_GLYPH == fMissingGlyph) {
        // remove const for cached read only init
        CompositeGlyphMapper *myPtr = (CompositeGlyphMapper *)this;
        myPtr->fMissingGlyph = 0;
        Strike& myStrike = (Strike&)fStrike;

	for (int i = 0; i < fNumSlots; ++i) {
	    hsGGlyphStrike *strike = myStrike.compositeStrikeForSlot(i);
	    if (strike) {
		CharToGlyphMapper *strikeMapper = strike->getMapper();
		if (strikeMapper) {
		    myPtr->fMissingGlyph = GLYPHCODE(i, strikeMapper->getMissingGlyphCode());
		    break;
		}
	    }
	}
    }
    return fMissingGlyph;
}
