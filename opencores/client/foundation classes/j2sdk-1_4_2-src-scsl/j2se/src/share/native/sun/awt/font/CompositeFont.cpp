/*
 * @(#)CompositeFont.cpp	1.32 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * @author Charlton Innovations, Inc.
 */

#include "dtypes.h"
#include "FontGlue.h"
#include "CompositeFont.h"
#include "Strike.h"         // REMIND: solely for counting glyphs (not any more...)
#include "LEScripts.h"
#include "ScriptRun.h"
#include <stdlib.h>
#include <ctype.h>

const void*
CompositeFont::ReadChunk(UInt32 offset, UInt32 length, void* chunk) {
    return NULL;
}

// REMIND: need failure status
void CompositeFont::readBlock(size_t offs, size_t size, char *dest) {
    return;
}

void CompositeFont::ReleaseChunk(const void* chunk) {
}

// REMIND: need failure status (or some junk)
const UInt16 CompositeFont::GetUnitsPerEM()
{
    return 0;
}

Boolean CompositeFont::MatchName(int nameID, const UInt16 *name, int nameLen) {
    Boolean retval = (nameLen == fNameLen) && fCompositeName;
    if (retval) {
	UInt16 val;
        for (int i=0; i<nameLen && retval ; i++) {
	    if (name [i] > 0xff) {
		val = name [i];
	    } else {
		val = tolower (name[i]);
	    }
            retval &= (val == fCompositeName[i]);
        }
    }
    return retval;
}

// REMIND: move to common location
#define GET16(P)    (UInt16)(   (((UInt8 *)&(P))[0] << 8) \
                                | (((UInt8 *)&(P))[1]) )

// REMIND: need max name length
int CompositeFont::GetName(UInt16& platformID, UInt16& scriptID,
                           UInt16& languageID, UInt16& nameID, UInt16 *name) {
	int i;

    for (i = 0; i < fNameLen; i += 1) {
        if (name != NULL) {
            name[i] = fCompositeName[i]; 
        }

        if (fCompositeName[i]=='.' && nameID == kFamilyName) {
            break;
        }
    }
    
    if (name != NULL) {
        name[i] = 0;
    }

    platformID = 3;
    scriptID = 1;

    return i;

}

const Unicode *
CompositeFont::GetFontName(int& nameLen) {
    nameLen = fNameLen;
    return fCompositeName;
}

UInt32 CompositeFont::GetDataLength() {
    return 0;
}

CompositeFont::CompositeFont(JNIEnv *env, jstring compositeNameArg)
    : fCompositeName(NULL),
      fRanges(NULL), fMaxIndices(NULL),
      fComponentNames(NULL), fComponentFonts(NULL), fComponentChunks(NULL),
      fNumComponents(0), fInitialized(JNI_FALSE)
{
    JStringBuffer compositeName(env, compositeNameArg);
    fNameLen = compositeName.getLength();
    fCompositeName = new jchar[fNameLen];
    if (fCompositeName) {
        memcpy(fCompositeName, compositeName.buffer(),
                                                fNameLen * sizeof(jchar));
    }

    fFormat = kCompositeFontFormat;
}

void
CompositeFont::addRanges(JNIEnv *env, jintArray exclusionRanges) {
    UInt32Buffer ranges(env, exclusionRanges);
    jint num = ranges.getNumElements();
    fRanges = new jint[num];
    if (fRanges) {
        memcpy(fRanges, ranges.buffer(),
            sizeof(jint) * num);
    }
}

int
CompositeFont::isExcludedChar(int slot, Unicode32 unicode) {
    if (!fRanges || !fMaxIndices) {
        return false;
    }
    int minIndex = 0;
    int maxIndex = fMaxIndices[slot];
    if (slot > 0) {
        minIndex = fMaxIndices[slot - 1];
    }
    int curIndex = minIndex;
    while (maxIndex > curIndex) {
        if (    (unicode >= (Unicode32)fRanges[curIndex])
                && (unicode <= (Unicode32)fRanges[curIndex+1]))
        {
            // excluded
            return true;
        }
        curIndex += 2;
    }
    return false;
}

void
CompositeFont::addMaxIndices(JNIEnv *env, jintArray maxIndices) {
    UInt32Buffer maxIndex(env, maxIndices);
    fNumComponents = maxIndex.getNumElements();

    fMaxIndices = new jint[fNumComponents + 1];
    fComponentNames = new jchar *[fNumComponents + 1];
    fComponentNameLengths = new int [fNumComponents + 1];
    fComponentFonts = new fontObject *[fNumComponents + 1];
    fComponentChunks = new const void *[fNumComponents + 1];

    if (fMaxIndices) {
        memcpy(fMaxIndices, maxIndex.buffer(), sizeof(jint) * fNumComponents);
    }
    if (fComponentNames) {
        memset(fComponentNames, 0, sizeof(jchar *) * fNumComponents);
    }
    if (fComponentNameLengths) {
        memset(fComponentNameLengths, 0, sizeof(int) * fNumComponents);
    }
    if (fComponentFonts) {
        memset(fComponentFonts, 0, sizeof(fontObject *) * fNumComponents);
    }
    if (fComponentChunks) {
        memset(fComponentChunks, 0, sizeof(const void *) * fNumComponents);
    }

#ifdef WIN32
    sfntFileFontObject *systemDefaultEUDCFont = getLinkedEUDCFont();
    if (systemDefaultEUDCFont){
        UInt16 platformID = 3;
        UInt16 scriptID = 1;
        UInt16 languageID = 0xffff;
        UInt16 nameID = kFamilyName;
        UInt16 nameLen = 0;
        Unicode *name = NULL;

        nameLen = systemDefaultEUDCFont->GetName(platformID,
                                                 scriptID,
                                                 languageID,
                                                 nameID,
                                                 NULL);
        if (nameLen != 0) {
            name = new Unicode[nameLen];
            systemDefaultEUDCFont->GetName(platformID,
                                           scriptID,
                                           languageID,
                                           nameID,
                                           name);       
            fComponentNameLengths[fNumComponents] = nameLen;   
            fComponentNames[fNumComponents] = name;
            fComponentFonts[fNumComponents] = systemDefaultEUDCFont;
            fComponentChunks[fNumComponents] = systemDefaultEUDCFont->ReadChunk(0,1,NULL);
            //no exclusion ranges
            fMaxIndices[fNumComponents] = fMaxIndices[fNumComponents -1];
            fNumComponents++;
	}
    }
#endif
}

void
CompositeFont::registerComponentName(int slot, JStringBuffer& componentName) {
    if (fComponentNames
            && fComponentNameLengths
            && (slot >= 0)
            && (slot < fNumComponents)
            && fComponentFonts) {
        if (fComponentNames[slot]) {
            delete [] fComponentNames[slot];
            fComponentNames[slot] = NULL;
        }
        int nameLen = componentName.getLength();
        fComponentNameLengths[slot] = nameLen;
        fComponentNames[slot] = new jchar[nameLen];
        if (fComponentNames[slot]) {
            memcpy(fComponentNames[slot], componentName.buffer(),
                                                    nameLen * sizeof(jchar));
        }
    }
    fInitialized = JNI_TRUE;
    fInitialized = fInitialized && fCompositeName;
    fInitialized = fInitialized && fRanges;
    fInitialized = fInitialized && fMaxIndices;
    fInitialized = fInitialized && fComponentNames;
    if (fInitialized) {
        for (int i = 0; i < fNumComponents; i++) {
            fInitialized = fInitialized && fComponentNames[i];
        }
    }
}

int
CompositeFont::isValid() {
    return fInitialized;
}

int
CompositeFont::countSlots(void) {
    return fNumComponents;
}

fontObject *
CompositeFont::getSlotFont(int slot) {
    fontObject *retval = NULL;
    if ((slot >= 0) && (slot < fNumComponents)) {
        if (fComponentFonts) {
            retval = fComponentFonts[slot];
            if (retval == NULL) {
                fComponentFonts[slot] =
                    ::FindFontObject(   fComponentNames[slot],
                                        fComponentNameLengths[slot], 0);
// REMIND: later speedup to use font file name for quick locator
//                fComponentFonts[slot] =
//                    ::GetFontObject(fComponentNames[slot], nameLen);
                // ??? The old code further down assumes that it's OK to return
		// null, but not all callers are prepared for that.
		// FindFontObject used to return DefaultFontObject if it
		// couldn't find anything, so the following statement continues
		// this tradition. Most likely we never get here because
		// higher level code should guarantee that only available
		// physical fonts are used when creating the composite font.
		if (fComponentFonts[slot] == NULL) {
		    fComponentFonts[slot] = ::DefaultFontObject();
		}
                retval = fComponentFonts[slot];
                if (retval == NULL) {
                    // font not located.
                    fComponentFonts[slot] = (fontObject *)-1;
                }
                else if (fComponentChunks) {
                    // Leave this file open
                    if (fComponentChunks[slot] == NULL) {
                        fComponentChunks[slot] = retval->ReadChunk(0, 1, NULL);
                    }
                }
            }
        }
    }
    if (retval == (fontObject *)-1) {
        retval = 0;
    }
    return retval;
}

CompositeFont::~CompositeFont() {
    if (fCompositeName != NULL) {
        delete [] fCompositeName;
        fCompositeName = NULL;
    }

    if (fRanges != NULL) {
        delete [] fRanges;
        fRanges = NULL;
    }

    if (fMaxIndices != NULL) {
        delete [] fMaxIndices;
        fMaxIndices = NULL;
    }

    if (fComponentNames != NULL) {
        for (int i=0; i < fNumComponents; i++) {
            delete [] fComponentNames[i];
            fComponentNames[i] = NULL;
        }

        delete [] fComponentNames;
        fComponentNames = NULL;
    }

    if (fComponentNameLengths != NULL) {
        delete [] fComponentNameLengths;
        fComponentNameLengths = NULL;
    }

    if (fComponentChunks != NULL) {
        if (fComponentFonts != NULL) {
            for (int i=0; i < fNumComponents; i++) {
                if (fComponentChunks[i]) {
                    fComponentFonts[i]->ReleaseChunk(fComponentChunks[i]);
                }
            }
        }

        delete [] fComponentChunks;
        fComponentChunks = NULL;
    }

    if (fComponentFonts != NULL) {
        delete [] fComponentFonts;
        fComponentFonts = NULL;
    }
}
