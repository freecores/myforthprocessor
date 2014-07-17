/*
 * @(#)FontInstanceAdapter.cpp	1.10 03/01/23
 *
 * (C) Copyright IBM Corp. 1998-2001 - All Rights Reserved
 *
 * Portions Copyright 2003 by Sun Microsystems, Inc.,
 * 901 San Antonio Road, Palo Alto, California, 94303, U.S.A.
 * All rights reserved.
 *
 * This software is the confidential and proprietary information
 * of Sun Microsystems, Inc. ("Confidential Information").  You
 * shall not disclose such Confidential Information and shall use
 * it only in accordance with the terms of the license agreement
 * you entered into with Sun.
 *
 * The original version of this source code and documentation is
 * copyrighted and owned by IBM. These materials are provided
 * under terms of a License Agreement between IBM and Sun.
 * This technology is protected by multiple US and International
 * patents. This notice and attribution to IBM may not be removed.
 */

#include "LETypes.h"
#include "LEFontInstance.h"
#include "FontInstanceAdapter.h"

#include "Strike.h"
#include "hsGText.h"

FontInstanceAdapter::FontInstanceAdapter(fontObject *theFontObject, Strike *theStrike,
                                         FontTransform *tx, le_int32 xRes, le_int32 yRes)
    : fFontObject(theFontObject), fStrike(theStrike), xppem(0), yppem(0),
      xScaleUnitsToPoints(0), yScaleUnitsToPoints(0), xScalePixelsToUnits(0), yScalePixelsToUnits(0)
{
    float upem = (float) fFontObject->GetUnitsPerEM();
    float matrix[4];
    float xPointSize, yPointSize;

    tx->getMatrixInto(matrix, 4);

    xPointSize = euclidianDistance(matrix[0], matrix[1]);
    yPointSize = euclidianDistance(matrix[2], matrix[3]);

    xppem = ((float) xRes / 72) * xPointSize;
    yppem = ((float) yRes / 72) * yPointSize;

    xScaleUnitsToPoints = xPointSize / upem;
    yScaleUnitsToPoints = yPointSize / upem;

    xScalePixelsToUnits = upem / xppem;
    yScalePixelsToUnits = upem / yppem;
};

void FontInstanceAdapter::mapCharsToGlyphs(const LEUnicode chars[], le_int32 offset, le_int32 count, le_bool reverse, const LECharMapper *mapper, LEGlyphID glyphs[]) const
{
    le_int32 i, out = 0, dir = 1;

    if (reverse) {
        out = count - 1;
        dir = -1;
    }

    for (i = offset; i < offset + count; i += 1, out += dir) {
		LEUnicode16 high = chars[i];
		LEUnicode32 code = high;

		if (i < offset + count - 1 && high >= 0xD800 && high <= 0xDBFF) {
			LEUnicode16 low = chars[i + 1];

			if (low >= 0xDC00 && low <= 0xDFFF) {
				code = (high - 0xD800) * 0x400 + low - 0xDC00 + 0x10000;
			}
		}

        glyphs[out] = mapCharToGlyph(code, mapper);

		if (code >= 0x10000) {
			i += 1;
			glyphs[out += dir] = 0xFFFF;
		}
    }
}

LEGlyphID FontInstanceAdapter::mapCharToGlyph(LEUnicode32 ch, const LECharMapper *mapper) const
{
    LEUnicode32 mappedChar = mapper->mapChar(ch);

    if (mappedChar == 0xFFFF || mappedChar == 0xFFFE) {
        return 0xFFFF;
    }

    if (mappedChar == 0x200C || mappedChar == 0x200D) {
        return 1;
    }

    return (LEGlyphID) fStrike->CharToGlyph(mappedChar);
}

void FontInstanceAdapter::mapCharsToWideGlyphs(const LEUnicode chars[], le_int32 offset, le_int32 count, le_bool reverse, const LECharMapper *mapper, le_uint32 glyphs[]) const
{
    le_int32 i, out = 0, dir = 1;

    if (reverse) {
        out = count - 1;
        dir = -1;
    }

    for (i = offset; i < offset + count; i += 1, out += dir) {
		LEUnicode16 high = chars[i];
		LEUnicode32 code = high;

		if (i < offset + count - 1 && high >= 0xD800 && high <= 0xDBFF) {
			LEUnicode16 low = chars[i + 1];

			if (low >= 0xDC00 && low <= 0xDFFF) {
				code = (high - 0xD800) * 0x400 + low - 0xDC00 + 0x10000;
			}
		}

        glyphs[out] = mapCharToWideGlyph(code, mapper);

		if (code >= 0x10000) {
			i += 1;
			glyphs[out += dir] = 0xFFFF;
		}
    }
}

le_uint32 FontInstanceAdapter::mapCharToWideGlyph(LEUnicode32 ch, const LECharMapper *mapper) const
{
    LEUnicode32 mappedChar = mapper->mapChar(ch);

    if (mappedChar == 0xFFFF) {
        return 0xFFFF;
    }

    if (mappedChar == 0x200C || mappedChar == 0x200D) {
        return 1;
    }

    return fStrike->CharToGlyph(mappedChar);
}

void FontInstanceAdapter::getGlyphAdvance(LEGlyphID glyph, LEPoint &advance) const
{
    hsGGlyph glyphRef;
    hsFixedPoint2 adv;

    // FIXME: return value?
    fStrike->getMetrics(glyph, glyphRef, adv);

    advance.fX = fixedToFloat(adv.fX);
    advance.fY = fixedToFloat(adv.fY);
}

void FontInstanceAdapter::getWideGlyphAdvance(le_uint32 glyph, LEPoint &advance) const
{
    hsGGlyph glyphRef;
    hsFixedPoint2 adv;

    // FIXME: return value?
    fStrike->getMetrics(glyph, glyphRef, adv);

    advance.fX = fixedToFloat(adv.fX);
    advance.fY = fixedToFloat(adv.fY);
}

le_bool FontInstanceAdapter::getGlyphPoint(LEGlyphID glyph, le_int32 pointNumber, LEPoint &point) const
{
    hsFixedPoint2 pt;
    le_bool result;

    result = fStrike->GetGlyphPoint(glyph, pointNumber, pt);

    if (result) {
        point.fX = fixedToFloat(pt.fX);
        point.fY = fixedToFloat(pt.fY);
    }

    return result;
}

void FontInstanceAdapter::transformFunits(float xFunits, float yFunits, LEPoint &pixels) const
{
    hsFixedPoint2 pt;

    long xFunitsL = (long)xFunits;
    long yFunitsL = (long)yFunits;
    fStrike->TransformFunits(0, (short)xFunitsL, (short)yFunitsL, pt);

    pixels.fX = fixedToFloat(pt.fX);
    pixels.fY = fixedToFloat(pt.fY);
}

float FontInstanceAdapter::euclidianDistance(float a, float b)
{
    if (a < 0) {
        a = -a;
    }

    if (b < 0) {
        b = -b;
    }

    if (a == 0) {
        return b;
    }

    if (b == 0) {
        return a;
    }

    float root = a > b ? a + (b / 2) : b + (a / 2); /* Do an initial approximation, in root */

	/* An unrolled Newton-Raphson iteration sequence */
    root = (root + (a * (a / root)) + (b * (b / root)) + 1) / 2;
    root = (root + (a * (a / root)) + (b * (b / root)) + 1) / 2;
    root = (root + (a * (a / root)) + (b * (b / root)) + 1) / 2;

    return root;
}
