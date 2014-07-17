/*
 * @(#)fontObject_md.h	1.15 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
 
/* 
 * @author Joe Warzecha
 */ 

#ifndef _FONTOBJECT_MD_H_
#define _FONTOBJECT_MD_H_

#include "fontObject.h"
#include <jlong_md.h>
#include "font.h"

#define USE_NATIVE_RASTERIZER 
/* #undef USE_NATIVE_RASTERIZER */

class X11CharToGlyphMapper : public CharToGlyphMapper {
private:
    UInt16 *convertedGlyphs;
    int convertedCount;
    short nBytes;
    char *psEncoding;
public:
    X11CharToGlyphMapper (const char* javaclass, short numBytes, char *psEnc,
			  UInt16 maxUnicode, UInt16 minUnicode);
    virtual ~X11CharToGlyphMapper ();
    virtual void CharsToGlyphs (int count, const Unicode16 unicodes [],	
			  UInt32 glyphs []) const;
    virtual void CharsToGlyphs (int count, const Unicode32 unicodes [],	
			  UInt32 glyphs []) const;
    virtual Boolean canDisplay (Unicode ch) const;
    char *GetPSEncoding () { return psEncoding ; }
    void AddAsciiChars (Boolean lowPartOnly) ;
};

class X11FontObject : public fileFontObject {
 private:
    char *familyName;
    char *slant;
    char *style;
    char *fontName;
    char *PSfontName;
 public: 
    X11FontObject () :
	familyName (0),
	slant (0),
	style (0),
	fontName (0),
	PSfontName (0) { } ; 

    ~X11FontObject ();
    char *GetPSName () { return PSfontName; }
    char *GetX11Name () { return fontName; }
    char *GetFamilyName () { return familyName; }
    char *GetSlant () { return slant; }
    char *GetStyle () { return style; }
    void SetPSName (char *psfontname) { 
	PSfontName = strdup (psfontname);
    }
    virtual int GetName(UInt16& platformID, UInt16& scriptID,
                        UInt16& languageID, UInt16& nameID, UInt16 *name);
    virtual const Unicode *GetFontName(int& nameLen);
    virtual Boolean MatchName(int nameID, const UInt16 *name, int nameLen);
    virtual enum FontFormats getScalerID (FontTransform& tx,
				jboolean isAntiAliased,
				jboolean usesFractionalMetrics)
	{ return kNRFontFormat; }
private:
    int MakeFontName ();
};

#endif
