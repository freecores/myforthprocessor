/*
 * @(#)fontObject_md.cpp	1.18 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
 
/* 
 * @author Joe Warzecha
 */ 

#include "fontObject_md.h"
#include "hsTemplates.h"
#include <ctype.h>

#include <jni.h>
#include <jni_util.h>

X11FontObject::~X11FontObject () 
{
    if (fontName != NULL) {
	free (fontName);
    }
    if (PSfontName != NULL) {
	free (PSfontName);
    }
    if (familyName != NULL) {
	free (familyName);
    }
    if (slant != NULL) {
	free (slant);
    }
    if (style != NULL) {
	free (style); 
    }
}

// unicode char to lowercase
inline UInt16 toLowerU(UInt16 unicodeChar) 
{
    if ((unicodeChar > 0x40) && (unicodeChar <= 0x40 + 26)) {
        return unicodeChar + 0x20;
    }
    return unicodeChar;
}

static int equalUnicodeToAsciiNC (const UInt16 *name, int unicodeNameLen,
                               const char *ascii, int asciiBytes = 0)
 
{
    int retval = 0;
    if (!asciiBytes && ascii) {
        asciiBytes = strlen(ascii);
    }
    if (name && unicodeNameLen && asciiBytes && ascii && *ascii) {
        if (asciiBytes == unicodeNameLen) {
            int i;
            retval = 1;
            for (i=0; i<unicodeNameLen; i++) {
                if ((name[i] > 255) || (name[i] != (0xff & ascii[i]))) {
                    if (toLowerU(name[i]) == toLowerU(0xff & ascii[i])) {
                        continue;
                    }
                    retval = 0;
                    break;
                }
            }
        }
    }
    return retval;
}    

Boolean X11FontObject::MatchName(int nameID, const UInt16 *name,
                                       int nameLen)
{
    Boolean retval;
    char *matchName = NULL;

    if ((fontName == NULL) || (familyName == NULL)) {
	int status = MakeFontName ();
	if (status != Success) {
            JNIEnv *env = (JNIEnv *) JNU_GetEnv (jvm, JNI_VERSION_1_2);
            JNU_ThrowOutOfMemoryError (env,
                        "Cannot create native data structure.");
	    return false;
	}
    }
	
    switch (nameID) {
	case kFullName:
	    if (fontName == NULL) {
		return false;
	    }
	    matchName = fontName; 
	    break;
	case kFamilyName:
	    if (familyName == NULL) {
		return false;
	    }
	    matchName = familyName; 
	    break;
	default:
	    return false;
    }

    retval = equalUnicodeToAsciiNC (name, nameLen, matchName, 
				    strlen (matchName));
    return retval;
}

int X11FontObject::MakeFontName () 
{
    int len;
    // X11 font objects are for fonts which we access exclusively via
    // platform APIs, hence by definition they should always have 
    // exactly ONE native name. Nonetheless, lets check to make sure.
    const char *propVal = GetFontNativeName ();
    if (propVal == NULL) {
	return -1;
    }

    char *end, *start;
    const char *tmpFamily = NULL, *tmpSlant = NULL, *tmpStyle = NULL, *encoding;
    int useDefault = 0;
    char xlfd [512];

    strcpy (xlfd, propVal);
    end = xlfd;

    do {
	NEXT_HYPHEN; /* skip FOUNDRY */
        NEXT_HYPHEN; /* set FAMILY_NAME */
	tmpFamily = start;
        NEXT_HYPHEN; /* set STYLE_NAME */
	tmpStyle = start;
        NEXT_HYPHEN; /* set SLANT */
	tmpSlant = start;
	NEXT_HYPHEN; /* skip SETWIDTH_NAME */
        NEXT_HYPHEN; /* skip ADD_STYLE_NAME */
        NEXT_HYPHEN; /* skip PIXEL_SIZE */
        NEXT_HYPHEN; /* skip POINT_SIZE */
        NEXT_HYPHEN; /* skip RESOLUTION_X */
        NEXT_HYPHEN; /* skip RESOLUTION_Y */
        NEXT_HYPHEN; /* skip SPACING */
        NEXT_HYPHEN; /* skip AVERAGE_WIDTH */
    } while (0);
        	
    if (useDefault) { /* We didn't find what we wanted */
	tmpFamily = "Unknown";
	encoding = "iso8859-1";
	style = 0;
	slant = 0;
    } else {
	encoding = (end + 1);
    }


    char buf [512];	/* BAD, but would a font name ever be longer
			         * than this?? */
    char tmpBuf [100];

    /* Copy family name into buffer, capitalizing after spaces
     * in the family name */
    const char *a = tmpFamily;
    char *b = buf;
    *b++ = toupper (*a++);
    while (*a != 0) {
	if (*a == ' ') {
	    *b++ = toupper (*++a);
            a++;
        } else {
            *b++ = *a++;
	}
    }
    *b = '\0';
    familyName = strdup (buf);

    if ((tmpStyle != NULL) &&
	(strstr (tmpStyle, "medium") == 0) &&
	(strstr (tmpStyle, "normal") == 0)) 
    {
	const char *boldPart = strstr (tmpStyle, "bold");
        int index = 0;
	if (boldPart != NULL) {
	    index = boldPart - tmpStyle;
	}
	strcpy (tmpBuf, tmpStyle);
	tmpBuf [0] = toupper (tmpBuf[0]);
	if (index != 0) {
	    tmpBuf [index] = toupper (tmpBuf[index]);
	}
	strcat (buf, " ");
	strcat (buf, tmpBuf);
	style = strdup (tmpBuf);
    }

    if (tmpSlant != NULL) {
	if (strcmp (tmpSlant, "i") == 0) {
	    strcat (buf, " Italic");
	    slant = strdup ("Italic");
	} else if (strcmp (tmpSlant, "o") == 0) {
	    strcat (buf, " Oblique");
	    slant = strdup ("Oblique");
	}
    }

    /* Check encoding */
    if ((strstr (encoding, "iso") == 0) &&
	(strstr (encoding, "fontspecific") == 0) &&
	(strstr (encoding, "symbol") == 0) &&
	(strstr (encoding, "dingbats") == 0)) 
    {
	strcat (buf, " (");
	strcat (buf, encoding);
	strcat (buf, ")");
    }

    len = strlen (buf);
    fontName = (char *) calloc (1, sizeof (char) * (len + 1));
    if (fontName == NULL) {
	return -1;
    }
    strcpy (fontName, buf);
    return Success;

}

int X11FontObject::GetName(UInt16& platformID, UInt16& scriptID, UInt16& languageID, UInt16& nameID, UInt16 *name)
{
    int len = 0;
    if ((fontName == NULL) || (familyName == NULL)) {
	int retVal = MakeFontName ();
	if (retVal != Success) {
            JNIEnv *env = (JNIEnv *) JNU_GetEnv (jvm, JNI_VERSION_1_2);
            JNU_ThrowOutOfMemoryError (env,
                        "Cannot create native data structure.");
            return 0;
	}
    }

    if (name) {
	switch (nameID) {
	    case kFullName:
		if (fontName != NULL) {
		    strcpy ((char *) name, fontName);
    		    len = strlen (fontName);
		}
		break;
	    case kFamilyName:
		if (familyName != NULL) {
		    strcpy ((char *) name, familyName);
		    len = strlen (familyName);
		}
		break;
	    default:
		break;
	}

	platformID = 1;
	scriptID = 0;
    }

    return (len);
}
 
static void copyAsciiToUnicode(const char *src, UInt16 *dest, int copySize = 0)
{
    if (copySize == 0 ) {
        // REMIND add some boundary overflow checking
        while ((*dest++ = (UInt16)(*src++))) {
        }
    } else {
        for (int i=0; i < copySize; i++) {
            dest[i] = src[i] & 0xff;
        }
    }
}
 
Unicode *strDupASCIItoUNICODE(const char *src, int& retNameLen, int maxInput = 0) {
    if (src) {
        if (!maxInput) {
            retNameLen = strlen(src);
        } else {
            retNameLen = maxInput;
        }
        Unicode *retval = new Unicode[retNameLen];
         
        if( nil != retval ) {
          copyAsciiToUnicode(src, retval, retNameLen);

        } else {
#ifdef DEBUG
          fprintf(
                  stderr,
                  "new returns nil. fontObject_md.cpp, strDupASCIItoUNICODE(const char*,int&,int)\n"
                  );
#endif
          retNameLen = 0;
        }
        return retval;
    } else {
        retNameLen = 0;
        return NULL;
    }
}

const Unicode *X11FontObject::GetFontName(int& nameLen)
{
    if (!fFontname) {
        if (fontName == NULL) {
	    int retVal = MakeFontName ();
	    if (retVal != Success) {
                JNIEnv *env = (JNIEnv *) JNU_GetEnv (jvm, JNI_VERSION_1_2);
                JNU_ThrowOutOfMemoryError (env,
                        "Cannot create native data structure.");
		nameLen = 0;
                return 0;
	    }
        }
	if (fontName != NULL) {
            fFontname = strDupASCIItoUNICODE(fontName, fFontNameLen);
	}
    }
    nameLen = fFontNameLen;
    return fFontname;
}

fileFontObject *GetNativeFontObject () {
	return new X11FontObject ();
}

