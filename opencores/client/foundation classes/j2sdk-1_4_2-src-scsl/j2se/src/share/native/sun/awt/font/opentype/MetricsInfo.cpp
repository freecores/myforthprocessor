/*
 *  @(#)MetricsInfo.cpp	1.9 03/01/23
 *
 * (C) Copyright IBM Corp. 1999 - All Rights Reserved
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

#include "dtypes.h"
#include "HeadSpin.h"
#include "hsFixedTypes.h"
#include "FontGlue.h"
#include "fontObject.h"
#include "Strike.h"
#include "MetricsInfo.h"

MetricsInfo::MetricsInfo(fontObject *fo, FontTransform *tx,
                         Strike *theStrike, tt_int32 theSlot, tt_int32 xRes, tt_int32 yRes)
    : strike(theStrike), slot(theSlot), xppem(0), yppem(0),
      xScaleUnitsToPoints(0), yScaleUnitsToPoints(0),
      xScalePixelsToUnits(0), yScalePixelsToUnits(0)
{
    float upem = (float) fo->GetUnitsPerEM();
    float matrix[4];
    float xPointSize, yPointSize;
    float xppemFloat, yppemFloat;

    tx->getMatrixInto(matrix, 4);

    xPointSize = euclidianDistance(matrix[0], matrix[1]);
    yPointSize = euclidianDistance(matrix[2], matrix[3]);

    xppemFloat = ((float) xRes / 72) * xPointSize;
    yppemFloat = ((float) yRes / 72) * yPointSize;

    xppem = (tt_uint16) xppemFloat;
    yppem = (tt_uint16) yppemFloat;

    xScaleUnitsToPoints = xPointSize / upem;
    yScaleUnitsToPoints = yPointSize / upem;

    xScalePixelsToUnits = hsFloatToFixed(upem / xppemFloat);
    yScalePixelsToUnits = hsFloatToFixed(upem / yppemFloat);
}

MetricsInfo::~MetricsInfo()
{
    // nothing to do...
}

Boolean MetricsInfo::getGlyphPoint(tt_uint32 glyphID, tt_uint32 point, hsFixedPoint2 *anchor)
{
    return strike->GetGlyphPoint((int) glyphID, (int) point, *anchor);
}

void MetricsInfo::getGlyphAdvance(tt_uint32 glyphID, hsFixedPoint2 *advance)
{
    hsGGlyph gGlyph;
    hsFixedPoint2 adv;

    strike->getMetrics((int) glyphID, gGlyph, adv);

    advance->fX = hsFixMul(adv.fX, xScalePixelsToUnits);
    advance->fY = hsFixMul(adv.fY, yScalePixelsToUnits);
}

void MetricsInfo::transformFunits(tt_int16 xFunits, tt_int16 yFunits, hsFixedPoint2 *pixels)
{
    strike->TransformFunits(slot << 24, xFunits, yFunits, *pixels);
}

float MetricsInfo::euclidianDistance(float a, float b)
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
