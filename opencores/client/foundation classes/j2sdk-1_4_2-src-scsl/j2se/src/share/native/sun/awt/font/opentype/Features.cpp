/*
 * @(#)Features.cpp	1.7 01/10/09 
 *
 * (C) Copyright IBM Corp. 1998, 1999, 2000, 2001 - All Rights Reserved
 *
 */

#include "LETypes.h"
#include "OpenTypeTables.h"
#include "Features.h"
#include "LESwaps.h"

const FeatureTable *FeatureListTable::getFeatureTable(le_uint16 featureIndex, LETag *featureTag) const
{
    if (featureIndex >= SWAPW(featureCount)) {
        return 0;
    }

    Offset featureTableOffset = featureRecordArray[featureIndex].featureTableOffset;

    *featureTag = SWAPT(featureRecordArray[featureIndex].featureTag);

    return (const FeatureTable *) ((char *) this + SWAPW(featureTableOffset));
}

