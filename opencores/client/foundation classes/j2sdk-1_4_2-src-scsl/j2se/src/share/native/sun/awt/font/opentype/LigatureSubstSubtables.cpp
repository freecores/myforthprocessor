/*
 * @(#)LigatureSubstSubtables.cpp	1.3 01/10/09 
 *
 * (C) Copyright IBM Corp. 1998, 1999, 2000, 2001 - All Rights Reserved
 *
 */

#include "LETypes.h"
#include "LEGlyphFilter.h"
#include "OpenTypeTables.h"
#include "GlyphSubstitutionTables.h"
#include "LigatureSubstSubtables.h"
#include "GlyphIterator.h"
#include "LESwaps.h"

le_uint32 LigatureSubstitutionSubtable::process(GlyphIterator *glyphIterator, const LEGlyphFilter *filter) const
{
    LEGlyphID glyph = (LEGlyphID) glyphIterator->getCurrGlyphID();
    le_int32 coverageIndex = getGlyphCoverage(glyph);

    if (coverageIndex >= 0) {
        Offset ligSetTableOffset = SWAPW(ligSetTableOffsetArray[coverageIndex]);
        const LigatureSetTable *ligSetTable = (const LigatureSetTable *) ((char *) this + ligSetTableOffset);
        le_uint16 ligCount = SWAPW(ligSetTable->ligatureCount);

        for (le_uint16 lig = 0; lig < ligCount; lig += 1) {
            Offset ligTableOffset = SWAPW(ligSetTable->ligatureTableOffsetArray[lig]);
            const LigatureTable *ligTable = (const LigatureTable *) ((char *)ligSetTable + ligTableOffset);
            le_uint16 compCount = SWAPW(ligTable->compCount) - 1;
            le_int32 startPosition = glyphIterator->getCurrStreamPosition();
            LEGlyphID ligGlyph = SWAPW(ligTable->ligGlyph);
            le_uint16 comp;

            if (filter != NULL && ! filter->accept(ligGlyph)) {
                continue;
            }

            for (comp = 0; comp < compCount; comp += 1) {
                if (! glyphIterator->next()) {
                    break;
                }

                if ((LEGlyphID) glyphIterator->getCurrGlyphID() != SWAPW(ligTable->componentArray[comp])) {
                    break;
                }
            }

            if (comp == compCount) {
                GlyphIterator tempIterator(*glyphIterator);
                LEGlyphID deletedGlyph = tempIterator.ignoresMarks()? 0xFFFE : 0xFFFF;

                while (comp > 0) {
                    tempIterator.setCurrGlyphID(deletedGlyph);
                    tempIterator.prev();

                    comp -= 1;
                }

                tempIterator.setCurrGlyphID(ligGlyph);

                return compCount + 1;
            }

            glyphIterator->setCurrStreamPosition(startPosition);
        }

    }

    return 0;
}
