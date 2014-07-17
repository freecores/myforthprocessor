/*
 * @(#)fontObject.cpp	1.138 03/02/19
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>
#ifdef WIN32
#include <io.h>
#include <windows.h>
#else /* Solaris/Linux (ie unix) */
#include <unistd.h>
#endif
#include <sys/stat.h>
#include <fcntl.h>

#include "fontObject.h"
#include "hsConfig.h"
#include "hsFixedTypes.h"
#include "hsScalar.h"
#include "hsGFontScaler.h"
#include "hsGGlyphCache.h"
#include "t2kScalerContext.h"
#include "nrScalerContext.h"
#include "FontGlue.h"
#include "Strike.h"
#ifdef __solaris__
#include "CStrike.h"
#endif
#include "cmaps.h"
#include "swaps.h"
#include "cpu.h"

#include <jni.h>
#include <jni_util.h>
#include <jlong_md.h>
#include <sun_awt_font_NativeFontWrapper.h>
#include <java_awt_Font.h>

#include "gdefs.h"

#ifdef WIN32
#define READ_FLAGS O_RDONLY|O_BINARY|_O_NOINHERIT 
#else /* Solaris/Linux (ie unix) */
#define READ_FLAGS O_RDONLY
#endif

extern "C" JavaVM * jvm;

int debugFonts = 0; // set by JAVA2D_DEBUGFONTS env. variable

inline jfloat fixedToFloat(hsFixed fixed) {
    return fixed / (float)(65536);
}
fontObject *DefaultFontObject();

#undef PRINT_INFO

#include "sfnt_types.h"



typedef struct {
    UInt32 tag;
    UInt32 version;
    UInt32 count;
} TTCHeader;


#define CHUNK_LEN 4096

#define GET16(P)    (UInt16)(   (((UInt8 *)&(P))[0] << 8) \
                                | (((UInt8 *)&(P))[1]) )

#define GET32(P)    (UInt32)(   (((UInt8 *)&(P))[0] << 24) \
                                | (((UInt8 *)&(P))[1] << 16) \
                                | (((UInt8 *)&(P))[2] << 8) \
                                | (((UInt8 *)&(P))[3]) )

#define PUT32(P, V)    do {                                 \
                        UInt8 *val = (UInt8 *)&(P);         \
                        val[0] = ((V) >>  24) & 0xff;       \
                        val[1] = ((V) >>  16) & 0xff;       \
                        val[2] = ((V) >>  8) & 0xff;        \
                        val[3] = (V) & 0xff;                \
                    } while (0)

static Unicode *strDupASCIItoUNICODE(const char *src, int& retNameLen, int maxInput = 0);

#define SUSPECT_CODE 0

/*
 * Get the default language ID
 */
//#ifdef WIN32
//extern "C" int getDefaultLocID(void);
//#define DEFAULTLOCID getDefaultLocID()
//#else
#define DEFAULTLOCID 0x0409  /* US English */
//#endif


#ifndef CLOSE_ON_USECOUNT
#define FONTFILECACHESIZE 10

static fontObject * FFCache [FONTFILECACHESIZE];
static int          FFCacheLen = 0;

void fontfilecachecheck(fileFontObject * fo) {
    int   i;

    if (FFCacheLen < FONTFILECACHESIZE) {
        FFCache[FFCacheLen++] = fo;
    } else {
        if (FFCache[0] == fo) {
            return;
        }
        FFCache[0]->FileClose();
        for (i=1; i<FONTFILECACHESIZE; i++)
            FFCache[i-1] = FFCache[i];
        FFCache[FONTFILECACHESIZE-1] = fo;
    }
}

void fontfilecachedelete(fileFontObject * fo) {
    int   i;
    for (i=0; i<FFCacheLen; i++) {
        if (FFCache[i] == fo) {
	  FFCache[i]->FileClose();
            for (i++; i<FFCacheLen; i++) {
                FFCache[i-1] = FFCache[i];
            }
            FFCacheLen--;
            return;
        }
    }
}
#endif

#ifdef DEBUG
static void printUnicode(const UInt16 *name, int nameLen) {
    for (int i=0; i < nameLen; i++) {
        printf( "%c", (char)name[i]);
    }
    printf ("\n" );
}
#endif
/*
 * Copy a unicode string and return the length
 */
int ustr_copy(Unicode * targ, Unicode * src) {
    int    len = 0;
    if (!src)
        return 0;
    while (*src) {
        if (targ) {
            *targ++ = *src;
        }
        src++;
        len++;
    }
    if (targ)
        *targ = 0;
    return len;
}


fontObject::~fontObject() {
    if (fFontname != NULL) {
        delete [] fFontname;
        fFontname = NULL;
    }

    if (fFullName != NULL) {
        delete [] fFullName;
        fFullName = NULL;
    }

    if (fFullNameAlt != NULL) {
        delete [] fFullNameAlt;
        fFullNameAlt = NULL;
    }

    if (fFamilyName != NULL) {
        delete [] fFamilyName;
        fFamilyName = NULL;
    }

    if (fFamilyNameAlt != NULL) {
        delete [] fFamilyNameAlt;
        fFamilyNameAlt = NULL;
    }

    if (fPSFontName != NULL) {
        delete [] fPSFontName;
        fPSFontName = NULL;
    }

    if (fStrike != NULL) {
        delete fStrike;
        fStrike = NULL;
    }

    if (fStrikeTx != NULL) {
        delete fStrikeTx;
        fStrikeTx = NULL;
    }

    if (fGlyphToCharMap != NULL) {
        delete fGlyphToCharMap;
        fGlyphToCharMap = NULL;
    }
}


/*
 * Get any strike.  This handles cases where we do not care about the size.
 */
Strike& fontObject::getStrike() {
  if (fStrike) {
        return *fStrike;
  }

  if (!fStrikeTx) {
        fStrikeTx = new FontTransform();
  }

  return getStrike(*fStrikeTx, false, false);
}


/*
 * Get a strike of a specified matrix and attributes
 */
Strike& fontObject::getStrike(
                    FontTransform& tx,
                    jboolean isAntiAliased,
                    jboolean usesFractionalMetrics) {
    if (        (fStrike == NULL)
            ||  (!fStrikeTx)
            ||  (isAntiAliased != fStrikeIsAntiAliased)
            ||  (usesFractionalMetrics != fStrikeUsesFractionalMetrics)
            ||  (m_currentStyle != fStrikeCurrentStyle)
            ||  (!tx.equals(*fStrikeTx))) {

        if (fStrike) {
            delete fStrike;
            fStrike = NULL;
        }

/** REMIND next call may have side effect of setting the strike */
        getScalerID(tx, isAntiAliased, usesFractionalMetrics);
        if (fStrike == NULL) {
            fStrike = new Strike(*this, tx, isAntiAliased, usesFractionalMetrics);
        }
        if (fStrikeTx) {
            delete fStrikeTx;
            fStrikeTx = NULL;
        }
        fStrikeTx = new FontTransform(tx);
        fStrikeIsAntiAliased = isAntiAliased;
        fStrikeUsesFractionalMetrics = usesFractionalMetrics;
        fStrikeCurrentStyle = m_currentStyle;
    }
    return *fStrike;
}

void fontObject::setStrike(
                    Strike *theStrike, FontTransform& tx,
                    jboolean isAntiAliased,
                    jboolean usesFractionalMetrics)
{
    if(NULL != fStrike){
        delete fStrike;
    }
    fStrike = theStrike;
    if(NULL != fStrikeTx){
        delete fStrikeTx;
    }
    fStrikeTx = new FontTransform(tx);
    fStrikeIsAntiAliased = isAntiAliased;
    fStrikeUsesFractionalMetrics = usesFractionalMetrics;
    fStrikeCurrentStyle = m_currentStyle;
}


UInt32 fontObject::FindTableSize(UInt32 tableTag, UInt32* offset)
{
        // baseclass does not implement this, 'cause it may not have tables
        return 0;
}

const UInt16 fileFontObject::GetUnitsPerEM()
{
    return (UInt16)fUnitsPerEM;
}

const void* fontObject::ReadTable(UInt32 tableTag, UInt32* tableLength)
{
    UInt32 offset;
    UInt32 length = this->FindTableSize(tableTag, &offset);
    const void* table = nil;

    if (length)
       table = this->ReadChunk(offset, length);
        if (tableLength)
                *tableLength = length;
    return table;
}

class FOTempChunk {
    fontObject* fFontObj;
    const void* fChunk;
public:
    FOTempChunk(fontObject* fo, UInt32 offset, UInt32 length) {
        fFontObj = fo;
        fChunk = fo->ReadChunk(offset, length);
    }

    ~FOTempChunk() {
        fFontObj->ReleaseChunk(fChunk);
    }

    const void* GetData() const { return fChunk; }
};



class FOTempTable {
    sfntFileFontObject* fFontObj;
    UInt32 fOffset, fLength;
    const void* fTable;
public:
    FOTempTable(sfntFileFontObject* fo, UInt32 tableTag) : fFontObj(fo) {
#ifdef PRINT_INFO
        fprintf(stderr, "FOTempTable - CP1\n");
#endif
        fLength = fo->FindTableSize(tableTag, &fOffset);
        assert(fLength > 0);

#ifdef PRINT_INFO
        fprintf(stderr, "FOTempTable() fOffset: %d, fLength: %d.\n", fOffset, fLength);
#endif

        fTable = fo->ReadChunk(fOffset, fLength);
    }

    ~FOTempTable() {
        fFontObj->ReleaseChunk(fTable);
        fTable = nil;           // debug only
    }

    UInt32 GetOffset() const { return fOffset; }
    const void* CreateTable() const {
        void *tbl = new char[fLength];
        memcpy(tbl, fTable, fLength);
        return tbl;
    }
};


class FOTempFontInfo {
    type1FileFontObject* fFontObj;
    t1FontInfo *fFontInfo;
public:
    FOTempFontInfo(type1FileFontObject* fo) : fFontObj(fo) {
        fFontInfo = fFontObj->GetFontInfo();
    }

    ~FOTempFontInfo() {
        fFontObj->DeleteFontInfo(fFontInfo);
        fFontInfo = nil;
    }
    Boolean   multipleMasterFont;
    t1FontInfo* GetFontInfo() const { return fFontInfo; }
};

const void* fileFontObject::ReadChunk(UInt32 offset, UInt32 length,
                                      void* chunk)
{
#ifdef PRINT_INFO
    fprintf(stderr, "ReadChunk(%d, %d, %X)\n", offset, length, chunk);
#endif
    char* data = (char*)chunk;

#ifndef CLOSE_ON_USECOUNT
    if (fFile == -1) {
#else
    if (fUseCount == 0) {
#endif
	if (fFormat != kNRFontFormat) {
	    fFile = open(fFontLocalPlatformName, READ_FLAGS);
	    
	    if (debugFonts) {
		fprintf(stderr,"opened font file %s fd=%d\n",
			fFontLocalPlatformName, fFile);
	    }
	}
        if (fFile == -1 ) {
            return NULL;
        }
#ifndef CLOSE_ON_USECOUNT
        fontfilecachecheck(this);
#endif
        if (fFileSize == 0) {
            fFileSize = (UInt32)lseek(fFile, 0, SEEK_END);
        }
    }
    fUseCount += 1;

    if (length > 0 && fFileSize > 0) {
        assert(offset < fFileSize);
        if ((offset + length) > fFileSize)
            length = fFileSize - offset;

        if (data == nil) {
            data = new char[length];
        }

        if( nil == data ) {
#ifdef DEBUG
          fprintf(
                  stderr,"new returns nil, fontObject.cpp,%s\n",
                  "fileFontObject::ReadChunk(UInt32,UInt32,void*)\n"
                  );
#endif
          return NULL;
        }

        off_t err = lseek(fFile, offset, SEEK_SET);
#ifdef DEBUG
        if (err == (off_t)-1) {
            fprintf(stderr, "seek(%d) returned %d\n", offset, (int) err);
        }
#endif

	errno = 0;
        size_t bytesRead = read(fFile, data, length);
        if (bytesRead != length) {
            err = errno;
            int tellvalue = lseek(fFile, 0, SEEK_END);
#ifdef DEBUG
            fprintf(stderr,
                    "<%s> of %d, ln %d, rd %d, sz %d, tell %d, err %d\n",
                    (char *)fFileName, offset, length, (int)bytesRead,
                    fFileSize, tellvalue, (int) err);
#endif
        }
    }

    // If the caller provided the chunk, call release with nil
    // to decrement the usecount, since the caller won't call
    // release.
    if (chunk) {
        this->ReleaseChunk(nil);
    }
    return data;
}

//- Light-weight ReadChunk
void fileFontObject::readBlock(size_t offset, size_t size, char *dest)
{

  assert((int)size >= 0);

  if (size == 0) {
    return;
  }

#ifndef CLOSE_ON_USECOUNT
    if (fFile == -1) {
#else
    if (fUseCount == 0) {
#endif
        errno = 0;
	if (fFormat != kNRFontFormat) {
	    fFile = open(fFontLocalPlatformName, READ_FLAGS);
	}
	if (debugFonts) {
	    fprintf(stderr,"opened font file %s fd=%d\n",
		    fFontLocalPlatformName, fFile);
	}

        assert(fFile >= 0);
        assert(errno == 0);
#ifndef CLOSE_ON_USECOUNT
        fontfilecachecheck(this);
#endif
        if (fFileSize == 0) {
            fFileSize = (UInt32)lseek(fFile, 0, SEEK_END);
        }
    }

    if (((size_t)lseek(fFile, 0, SEEK_CUR)) != offset) {
        int offset1 = lseek(fFile, offset, SEEK_SET);
        assert ((int)offset == offset1);
    }
    errno = 0;
    size_t count = read(fFile, dest, size);
    assert(errno == 0 && count == (size_t) size);

    if (fUseCount == 0) {
#ifdef CLOSE_ON_USECOUNT
        close(fFile);
        fFile = -1;
#endif
    }
    else {
        fUseCount++;
    }
}


#ifndef CLOSE_ON_USECOUNT
void fileFontObject::FileClose()
{
  if (fFile != -1) {
    if (debugFonts) {
      fprintf(stderr,"closing font descriptor %d\n", fFile);
    }
    close(fFile);
    fFile = -1;
  }
}
#endif



void fileFontObject::ReleaseChunk(const void* chunk)
{

    assert(fUseCount > 0);

    if (chunk != nil) {
        delete[] (char*)chunk;
    }

    fUseCount -= 1;
#ifdef CLOSE_ON_USECOUNT
    if (fUseCount == 0) {
        int err = close(fFile);
        fFile = -1;
        if (err) {
#ifdef DEBUG
            fprintf(stderr, "<%s> closed = %d\n", (char *)fFileName, err);
#endif
        }
    }
#endif
}


UInt32 fileFontObject::GetDataLength()
{
        if (fFileSize == 0) {
                (void)this->ReadChunk(0, 0);
                this->ReleaseChunk(nil);
        }
        return fFileSize;
}

enum FontFormats
type1FileFontObject::getScalerID(
                        FontTransform& tx,
                        jboolean isAntiAliased,
                        jboolean usesFractionalMetrics)
{
    // On Solaris, we use the native scaler/rasterizer so we can retrieve 
    // all glyphs from the Type1 font, since the t2k code doesn't work for
    // characters over \u007f. On Win32, the t2kScalerContext will be
    // used.
    Strike *theStrike = new Strike(*this, kT2KFontFormat, tx, isAntiAliased, usesFractionalMetrics);
    if (theStrike->GetNumGlyphs()) {
        // finally ... all conditions passed
        setStrike (theStrike, tx, isAntiAliased, usesFractionalMetrics);
        return kT2KFontFormat;
    } else {
        delete theStrike;
    }
    return fontObject::getScalerID(tx, isAntiAliased, usesFractionalMetrics);
}

/* returns:
 *        0 got next char
 *        -1 end of file (or ASCII block) found
 */

int
type1FileFontObject:: NextChar(sDataRecord *sData) {
  if (sData->eof)
    return -1;

  sData->ch = sData->buffer[sData->boffs++];

  if (sData->ch == 0x80) {
    sData->eof = 1;
    return -1;
  }

  if (sData->boffs >= sData->buflen) {
    sData->buflen = read(sData->fd, sData->buffer, CHUNK_LEN);
    sData->boffs = 0;
    if (sData->buflen <= 0) {
      sData->eof = 1;
      return -1;
    }
  }
  return 0;
}

/* returns -1 if eof found, 0 otherwise.
 * Skips over white-space characters and Postscript comments
 */
int
type1FileFontObject:: SkipSpaces(sDataRecord *sData) {
  int res = 0;

  while (sData->ch == '%') {
    do {
      res = NextChar(sData);
      if (sData->ch == 13 || sData->ch == 10)
        break;
    } while (res != -1);
    if (res == -1)
      break;
  }

  if (res == -1)
    return -1;

  while (sData->ch <= 32) {
    res = NextChar(sData);
    if (res == -1)
      return -1;
  }
  return 0;
}

/* returns -1 on eof, token class otherwise.
 * Token classes:
 * 1 - /
 * 2 - string
 */
int
type1FileFontObject::NextToken(sDataRecord *sData, char *token, int toklen) {
  int res = SkipSpaces(sData);
  int i = 0;
  token[0] = 0;

  if (!res) {
    sData->prevToken = sData->curToken;

    if (sData->ch == '/') {
      NextChar(sData);

      return sData->curToken = 1;
    }

    if (sData->ch == '(') {             /* quoted string is found */
      res = NextChar(sData);
      while ((res != -1) && (sData->ch != ')')) {
        if (i < toklen-1)
          token[i++] = sData->ch;
        NextChar(sData);
      }
      if (i < toklen)
        token[i] = 0;

      return sData->curToken = 2;
    }

    while ((res != -1) && (sData->ch > 32)) {
      if (i < toklen-1)
        token[i++] = sData->ch;
      NextChar(sData);
    }
    if (i < toklen)
      token[i] = 0;
    if (!strcmp(token,"eexec"))                   /* hack! don't read data after eexec */
      sData->eof = 1;
    return sData->curToken = 2;
  }
  return -1;
}

/* Closes a font */
void
type1FileFontObject::CloseFont(sDataRecord *sData) {
  if (sData->buffer) {
    free(sData->buffer);
  }

  if (sData->fd >= 0) {
    close(sData->fd);
  }

  sData->eof = 1;
}


/* Opens a PFA or a PFB Postscript Type 1 font */

int
type1FileFontObject::OpenFont(const char *fileName, sDataRecord *sData) {
  int errflag = 0;

  assert(sData != NULL);
  sData->buffer = NULL;
  sData->eof = 0;
  sData->curToken = 0;
  sData->prevToken = 0;

#ifdef PRINT_INFO
  printf ( "The font file path is :%s\n", fileName );
#endif
  if ((sData->fd = open(fileName, READ_FLAGS)) >= 0) {
    if ((sData->buffer = (unsigned char*)malloc(CHUNK_LEN)) != NULL) {
      sData->buflen = read(sData->fd, sData->buffer, CHUNK_LEN);
      if (sData->buflen > 0) {
        if (sData->buffer[0] == 0x80)   /* font is PFB */
          sData->boffs = 6;
        else
          sData->boffs = 0;             /* font is PFA */
      } else
        errflag = 1;
    } else
      errflag = 2;
  } else
    errflag = 3;

  if (sData->boffs >= sData->buflen) {
    errflag = 4;
  }

  if (errflag) {
    CloseFont(sData);
    return -1;
  }

  NextChar(sData);

  return 0;
}

/* Public APIs */

t1FontInfo*
type1FileFontObject::GetFontInfo() {
  t1FontInfo *fi = NULL;
  sDataRecord sData;
  int res;
  char tokbuf[1024];
  int f1 = 0, f2 = 0, f3 = 0;
  char * p;
  Boolean  multipleMasterFont = false;
  if ( fFontNamesValid == false ) {
      if (OpenFont(GetFontLocalPlatformName(), &sData) != -1 &&
         (fi = (t1FontInfo*) malloc(sizeof(t1FontInfo))) != NULL) {
         memset(fi, 0, sizeof(*fi));
         do {
         res = NextToken(&sData, tokbuf, 1024);
         if (res == 2) {
            if (!strncmp(tokbuf, "FullName", 8) && sData.prevToken == 1) {
               if (!strcmp(tokbuf, "FullName") && sData.prevToken == 1) {
                 res = NextToken(&sData, tokbuf, 1024);
                 fi->fullName = strdup(tokbuf);
                 f1 = 1;
               }
            } else {            // MARKSYM.PFB on OS/2 has "FullName(MARKSYM)readonly"
               if (tokbuf[8] == '(') {
                  p = strchr(&tokbuf[9], ')');
                  if (p) *p = 0;
                  fi->fullName = strdup(&tokbuf[9]);
                  f1 = 1;
               }
            }

            if (!strncmp(tokbuf, "FamilyName", 10) && sData.prevToken == 1) {
               if (!strcmp(tokbuf, "FamilyName") && sData.prevToken == 1) {
                 res = NextToken(&sData, tokbuf, 1024);
                 fi->familyName = strdup(tokbuf);
                 f2 = 1;
               }
            } else {
               if (tokbuf[10] == '(') {
                  p = strchr(&tokbuf[11], ')');
                  if (p) *p = 0;
                  fi->familyName = strdup(&tokbuf[11]);
                  f2 = 1;
               }
            }

            if (!strcmp(tokbuf, "FontName") && sData.prevToken == 1) {
              res = NextToken(&sData, tokbuf, 1024);
              if (res == 1) {
                res = NextToken(&sData, tokbuf, 1024);
                fi->fontName = strdup(tokbuf);
                f3 = 1;
              }
            }
          }
	  if (f1 && f2 && f3) {
	    res = NextToken(&sData, tokbuf, 1024);
	    if ( res == -1 )
	       break;
	    if ( !strcmp ( tokbuf, "makeblendedfont" )) {
	        multipleMasterFont = true;
#if 0
		printf ( "got mm \n" );
#endif
		break;
	    }	    
	  }
        } while (res > 0);
      }
      CloseFont(&sData);


      if ( f3 ) {
        theFontNames.fontName = (char *) malloc ( (strlen ( fi->fontName ) + 1 ) * sizeof (char) );
        strcpy ( theFontNames.fontName,  fi->fontName );
      } else {
        theFontNames.fontName = (char *)malloc ( sizeof (char) );
        strcpy ( theFontNames.fontName, "" );
      }

      if ( f1 ) {
        theFontNames.fullName = (char *)malloc ( (strlen ( fi->fullName ) + 1 ) * sizeof (char) );
        strcpy ( theFontNames.fullName,  fi->fullName );
      } else {
        theFontNames.fullName = (char *)malloc ( sizeof ( char ));
        strcpy ( theFontNames.fullName, "" );
      }

      if ( f2 ) {
        theFontNames.familyName = (char *)malloc ( (strlen (fi->familyName ) +1 ) * sizeof (char) );
        strcpy ( theFontNames.familyName,  fi->familyName );
      } else {
        theFontNames.familyName = (char *) malloc ( sizeof (char) );
        strcpy ( theFontNames.familyName, "");
      }
      //font file can be missing and OpenFont can fail ...
      if (fi != NULL) {
        fi->multipleMasterFont = multipleMasterFont;
        fFontNamesValid = true;
      }
  } else {
      fi = (t1FontInfo*) malloc(sizeof(t1FontInfo));
      memset(fi, 0, sizeof(*fi));
      fi->fontName = strdup ( theFontNames.fontName );
      fi->fullName = strdup ( theFontNames.fullName );
      fi->familyName = strdup ( theFontNames.familyName );
      fi->multipleMasterFont = multipleMasterFont;
  }
  return fi;
}

void type1FileFontObject::DeleteFontInfo(t1FontInfo *fi) {
  if (fi != NULL) {
    if (fi->fontName) free(fi->fontName);
    if (fi->fullName) free(fi->fullName);
    if (fi->familyName) free(fi->familyName);
    free(fi);
  }
}

// unicode char to lowercase
// REMIND: This should be fixed to give the correct result
inline UInt16 toLowerU(UInt16 unicodeChar) {
    if ((unicodeChar > 0x40) && (unicodeChar <= 0x40 + 26)) {
        return unicodeChar + 0x20;
    }
    return unicodeChar;
}

// case insensitive
static int equalUnicodeToAsciiNC(const UInt16 *name, int unicodeNameLen,
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


/*
 * u_strncmpi - Compare unicode strings case independent
 */
int u_strncmpi(const Unicode * n1, const Unicode * n2, int len) {
    int  len2 = 0;
    const Unicode * str2 = n2;

    if (!str2)
        return -1;
    while (*str2++) len2++;
    if (len != len2)
        return 1;
    if (!len)
       return 0;
    while ((*n2) &&
           (*n1 == *n2 || toLowerU(*n1) == toLowerU(*n2)) &&
           (len-- > 1)) {
        n1++;
        n2++;
    }
    if ((*n1 == *n2))
        return 0;
    return toLowerU(*n1) - toLowerU(*n2);
}


Boolean type1FileFontObject::MatchName(int nameID, const UInt16 *name,
                                       int nameLen)
{
    const Unicode * matchName;
    int len;
    if (fFullName == NULL) {
        FOTempFontInfo fInfo(this);
        t1FontInfo *fi = fInfo.GetFontInfo();
        if (fi) {
            fFullName   = strDupASCIItoUNICODE(fi->fullName, len);
            fFamilyName = strDupASCIItoUNICODE(fi->familyName, len);
	    fPSFontName = strDupASCIItoUNICODE(fi->fontName, len);
        }
    }

    switch(nameID) {
    case kFullName:
        matchName = fFullName;
        break;
    case kFamilyName:
        matchName = fFamilyName;
        break;
    default:
        return false;
    }
    return !(Boolean)u_strncmpi((const Unicode *)name, matchName, nameLen);
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

static Unicode *strDupASCIItoUNICODE(const char *src, int& retNameLen, int maxInput) {
    if (src) {
        if (!maxInput) {
            retNameLen = strlen(src)+1;
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
                  "new returns nil. fontObject.cpp, strDupASCIItoUNICODE(const char*,int&,int)\n"
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


int type1FileFontObject::GetName(UInt16& platformID, UInt16& scriptID, UInt16& languageID, UInt16& nameID, UInt16 *name)
{
    Unicode * ustr;
    int len;
    if (fFullName == NULL) {
        FOTempFontInfo fInfo(this);
        t1FontInfo *fi = fInfo.GetFontInfo();
        if (fi) {
            fFullName   = strDupASCIItoUNICODE(fi->fullName, len);
            fFamilyName = strDupASCIItoUNICODE(fi->familyName, len);
	    fPSFontName = strDupASCIItoUNICODE(fi->fontName, len);
        }
    }
    switch (nameID) {
        case kFullName   : ustr = fFullName;
                	   break;
        case kFamilyName : ustr = fFamilyName;
	                   break;
        case kPSFontName : ustr = fPSFontName;
	                   break;
        default          : ustr = fFullName;
    }
    platformID = 3;
    scriptID   = 1;
    return ustr_copy((Unicode *)name, ustr);
}


const Unicode *type1FileFontObject::GetFontName(int& nameLen)
{
    if (!fFontname) {
        FOTempFontInfo fInfo(this);
        t1FontInfo *fi = fInfo.GetFontInfo();
        if (fi && fi->fullName) {
            fFontname = strDupASCIItoUNICODE(fi->fullName, fFontNameLen);
        }
    }
    nameLen = fFontNameLen;
    return fFontname;
}

///////////////////////////////////////////////////////////////////////
static void fprintUnicode(Unicode *name, int nameLen) {
    for (int i=0; i < nameLen; i++) {
        if (name[i] < 256)       
            fprintf(stderr,"%c", (char)name[i]);
        else
            fprintf(stderr,"<%x>", name[i]);
    }
}
#if 0
static void fprintUnicodeSW(const UInt16 *name, int nameLen) {
    for (int i=0; i < nameLen; i++) {
        fprintf(stderr,"%c", (char)GET16(name[i]));
    }
}
#endif


/*
 * Make a unicode name
 */
Unicode * makeNameU(UInt8 * name, int namebytes, short encoding) {
    Unicode * result;
    int       len, i;

    /*
     * If the name is not in Unicode, convert it.
     * This should really not be necessary, except that we have a
     * number of old DBCS fonts with this problem.
     */
    if (encoding>1 && encoding<=6) {
        JNIEnv  * env = (JNIEnv *)JNU_GetEnv(jvm, JNI_VERSION_1_2);
        jclass      cls;
        jobject     obj;
        jmethodID   mid;
        jbyteArray  inbytes;
        jcharArray  outchars;
        const char  * CPage;

        /* Convert encoding id to class name */
        switch (encoding) {
        case 2:  CPage = "sun/io/ByteToCharSJIS"; break;
        case 3:  CPage = "sun/io/ByteToCharGBK"; break;
	case 4:  CPage = "sun/io/ByteToCharMS950"; break;
	case 5:  CPage = "sun/io/ByteToCharEUC_KR"; break;
        case 6:  CPage = "sun/io/ByteToCharJohab"; break;
        }

        /* Load the converter */
        cls = env->FindClass(CPage);
        mid = 0;
        if (cls) {
            mid = env->GetMethodID(cls, "<init>", "()V");
            if (mid) {
                obj = env->NewObject(cls, mid);
                if (obj) {
                    mid = env->GetMethodID(cls, "convert", "([BII[CII)I");
                } else mid = 0;
            }
        }

       /* If we found the codepage convert it */
        if (mid) {
	    /*before we call java converter, we need to scan the "name" 
              to convert double-byte ascii character to single byte.
	     */
            int i = 0; 
            int j = 0;
            UInt8 *nameR = new UInt8[namebytes];
            while (i < namebytes){
	      if (name[i] != 0)
                  nameR[j++] = name[i];
              i++;
	    }
            namebytes = j;
            inbytes = env->NewByteArray(namebytes);
            env->SetByteArrayRegion(inbytes, 0, namebytes, (jbyte *)nameR);
            delete [] nameR;
            outchars = env->NewCharArray(namebytes);
            len = env->CallIntMethod(obj, mid,
                                     inbytes, 0, namebytes,
                                     outchars, 0, namebytes);
            result = new Unicode[len+1];
            env->GetCharArrayRegion(outchars, 0, len, (jchar *)result);
            result[len] = 0;
            return result;
        }
    }

    /* Return a unicode string with endian correction */
    len = namebytes/2;
    result = new Unicode[len+1];
    memcpy(result, name, namebytes);
#if defined(_LITTLE_ENDIAN)
    for (i=0; i<len; i++) {
        result[i] = SWAPW(result[i]);
    }
#endif
    result[len] = 0;
    return result;
}

void sfntFileFontObject::ReadNames(void) {
    FOTempTable table(this, nameFontTableTag);
    const sfntNameHeader* nameTable = (sfntNameHeader*)table.CreateTable();
    const sfntNameRecord* nameRecord = (sfntNameRecord*) ((UInt8*) nameTable +
                                       sizeof_sfntNameHeader);
    short count = GET16(nameTable->count), i;
    short nameID;
    short platformID;
    short encoding;
    short langID;
    UInt8 * namePtr;
    UInt8 * stringPtr;
    short nameBytes;
    short locid;

    stringPtr = ((UInt8 *)nameTable)+GET16(nameTable->stringOffset);
    locid = DEFAULTLOCID;

    for (i=0; i<count; i++) {
        platformID = GET16(nameRecord->platformID);
        langID     = GET16(nameRecord->languageID);
        encoding   = GET16(nameRecord->scriptID);
        nameID     = GET16(nameRecord->nameID);
        namePtr    = stringPtr + GET16(nameRecord->offset);
        nameBytes  = GET16(nameRecord->length);

        if (platformID == 3) {
            switch (nameID) {
            case kFamilyName:
                if (langID==locid && fFamilyNameAlt == NULL) {
                    fFamilyNameAlt = makeNameU(namePtr, nameBytes, encoding);
                }

                if (fFamilyName == NULL || langID==DEFAULTLOCID ) {
                    if (fFamilyName !=  NULL) {
                        delete [] fFamilyName;
                    }

                    fFamilyName = makeNameU(namePtr, nameBytes, encoding);
                }
                break;

            case kFullName:
                if (langID==locid && fFullNameAlt == NULL) {
                    fFullNameAlt = makeNameU(namePtr, nameBytes, encoding);
                }

                if (fFullName == NULL || langID==DEFAULTLOCID ) {
                    if (fFullName != NULL) {
                        delete [] fFullName;
                    }

                    fFullName = makeNameU(namePtr, nameBytes, encoding);
                }

                if (fFontname == NULL || langID==DEFAULTLOCID ) {
                    if (fFontname != NULL) {
                        delete [] fFontname;
                    }

                    fFontname = makeNameU(namePtr, nameBytes, encoding);
                    fFontNameLen = nameBytes/2;
                }
                break;

	    case kPSFontName:
	        if (fPSFontName == NULL || langID==DEFAULTLOCID ) {
                    if (fPSFontName != NULL) {
                        delete [] fPSFontName;
                    }

                    fPSFontName = makeNameU(namePtr, nameBytes, encoding);
                }
            }
        }

        nameRecord++;
    }
}

#if !SUSPECT_CODE
Boolean sfntFileFontObject::MatchName(int meaningID, const UInt16 *name,
                                      int nameLen)
{
    const sfntNameHeader* nameTable = (sfntNameHeader*) fFontNameTable;
    if ( nameTable == NULL ) {
      FOTempTable table(this, nameFontTableTag);
        nameTable = (sfntNameHeader*)table.CreateTable();
        fFontNameTable = (char *) nameTable;
    }
    const sfntNameRecord* nameRecord = (sfntNameRecord*) ((UInt8*) nameTable + 
                                                 sizeof_sfntNameHeader);
    short count = GET16(nameTable->count);
    short nameID;
    nameID = meaningID;

#if 0
    fprintf(stderr, "Locating ["); fprintUnicode(name, nameLen); fprintf(stderr, "]\n");
#endif
    for (--count; count >= 0; --count) {
        if (nameID == -1 || GET16(nameRecord->nameID) == nameID) {
            short platformID = GET16(nameRecord->platformID);
            short scriptID   = GET16(nameRecord->scriptID);
            UInt8 *namePtr = (UInt8*) nameTable;
            namePtr += GET16(nameTable->stringOffset);
            namePtr += GET16(nameRecord->offset);
            short nameBytes = GET16(nameRecord->length);

            if ((platformID == 0) || (platformID == 3)) {
                UInt16 *unicodeName = (UInt16 *)namePtr;
#if 0
                fprintf(stderr, "   Compare to U["); fprintUnicodeSW(unicodeName, nameBytes/2); fprintf(stderr, "]\n");
#endif
                if (platformID == 3 && scriptID >= 2 && scriptID <= 6 && (nameLen * 2) == nameBytes){
                    Boolean matched = true;
                    int j;
                    unicodeName = makeNameU(namePtr, nameBytes, scriptID);
                    for (j=0; j<nameLen; j++) {
                        if (toLowerU(name[j]) != toLowerU(unicodeName[j])) {
                            matched = false;
                            break;
			} 
		    }
                    delete unicodeName;
                    if (matched) {
                        return true;
                    }
		}
                else
                if ((nameLen * 2) == nameBytes) {
                    Boolean matched = true;
                    int j;
                    for (j=0; j<nameLen; j++) {
                        if (toLowerU(name[j]) != toLowerU(GET16(unicodeName[j]))) {
                            matched = false;
                            break;
                        }
                    }
                    if (matched) {
                        return true;
                    }
                }

            } else {
#if 0
                fprintf(stderr, "   Compare to A[%.*s]\n", nameBytes, (char *)namePtr);
#endif		
                if (equalUnicodeToAsciiNC(name, nameLen, (char *)namePtr, nameBytes)) {
                    return true;
                }
            }
        }
	nameRecord++;
    }
    return false;
}
#else // SUSPECT_CODE
Boolean sfntFileFontObject::MatchName(int meaningID, const UInt16 *name,
                                      int nameLen)
{
    int   match;

    if (fFullName == NULL) {
        this->ReadNames();
    }

    switch(meaningID) {
    case kFullName:
        match = u_strncmpi(name, fFullName, nameLen);
        if (match)
            match = u_strncmpi(name, fFullNameAlt, nameLen);
        break;

    case kFamilyName:
        match = u_strncmpi(name, fFamilyName, nameLen);
        if (match)
            match = u_strncmpi(name, fFamilyNameAlt, nameLen);
        break;

    default:
        return false;
    }

    return !match;
}
// MatchName
#endif

sfntFileFontObject::~sfntFileFontObject()
{
    if (offsets != NULL) {
        delete [] offsets;
        offsets = NULL;
    }

    if ( fFontNameTable != NULL ) {
      delete fFontNameTable;
      fFontNameTable = NULL;
    }

    if (fGposTable != NULL) {
        delete[] fGposTable;
        fGposTable = NULL;
    }

    if (fGdefTable != NULL) {
	delete[] fGdefTable;
        fGdefTable = NULL;
    }

    if (fGsubTable != NULL) {
        delete[] fGsubTable;
        fGsubTable = NULL;
    }

    if (fMortTable != NULL) {
        delete[] fMortTable;
        fMortTable = NULL;
    }

}

Boolean ReadNameTable ( sfntFileFontObject *fo )
{
    UInt32 offset;
    fo->fFontNameTable = (char *) fo->ReadTable ( nameFontTableTag, &offset );
    if ( fo->fFontNameTable ) {
       fo->ReleaseChunk (nil);
       return true;
    }
    return false;
}

// This function verifies that the TrueType file is not corrupted
// by comparing the number of glyphs declared in 'maxp' table
// and the capacity of the 'loca' table. This is faster then
// calculating checksums (especially for Asian fonts).

#define tag_maxp 0x6D617870
#define tag_head 0x68656164
#define tag_loca 0x6C6F6361

Boolean VerifyTrueTypeFile(sfntFileFontObject *fo)
{
  // remind : put a better verificatiion test below. PARRY
#if 0

  // This test below fails for many fonts in the CJK locales
  // the fonts are off by atleast one in the loca table.
  // if we enforce the check then the fonts are rejected.
  // Obviously we have to register the fonts - which otherwise
  // behave properly.
  // To check for totally bogus font data a test to check for the
  // required tables like the cmap, loca, glyf etc. could be instituted.

    unsigned char *maxp = (unsigned char *) fo->ReadTable(tag_maxp, 0);
    unsigned char *head = (unsigned char *) fo->ReadTable(tag_head, 0);

    if (maxp == 0 || head == 0) {
        return false;
    }

    UInt32 locaOffs, locaSize;
    locaSize = fo->FindTableSize(tag_loca, &locaOffs);

    UInt16 numGlyphs = GetUInt16(maxp, 4);
    UInt16 indexfmt  = GetUInt16(head, 50);
    UInt32 locaCapacity = locaSize / (indexfmt ? 4 : 2);

    fo->ReleaseChunk(maxp);
    fo->ReleaseChunk(head);

    if (locaCapacity - 1 != numGlyphs) {
        return false;
    }
#endif
    return true;
}


Boolean sfntFileFontObject::Init(
        const Unicode *fileName, int nameLen, const char *localPlatformName,
        const char *nativeName, enum FontFormats format, int logicalNumber)
{
    fileFontObject::Init(fileName, nameLen, localPlatformName, nativeName,
                         format);
    Boolean ret(true);

    TTCHeader theHeader = { 0, 0, 0 };
    // Can't read header
    if (this->ReadChunk(0, sizeof theHeader, &theHeader) == NULL) {
        return false;
    }

    switch (GET32(theHeader.tag)) {
        case 0x74746366: {                        //- 'ttcf'
            fFontCount = GET32(theHeader.count);

            offsets = new UInt32 [fFontCount];

            if( nil == offsets ) {
#ifdef DEBUG
              fprintf(
                      stderr,"new returns nil. fontObject.cpp. %s %s\n",
                      "sfntFileFontObject::Init(const Unicode&,int,const char&,enum FontFormats,int)",
                      "switch(GET32(theHeader.tag)); case 0x74746366"
                      );
#endif
              return false;
            }

            this->ReadChunk(sizeof theHeader, fFontCount * sizeof *offsets, offsets);
            for (int i=0; i<fFontCount; i++) {
                offsets[i] = GET32(offsets[i]);
            }
            break;
        }
        case 0x00010000:                          //- TrueType version 1
        case 0x74727565: {                        //- 'true' - TrueType version 2
#ifdef __linux__
//	   This prevents linux TT fonts from going into the rasterizer
	   fFormat = kTrueTypeFontFormat;
#endif
	   fFontCount = 1;
            offsets = new UInt32 [1];
            if( nil == offsets ) {
#ifdef DEBUG
              fprintf(
                      stderr,"new returns nil. fontObject.cpp. %s %s\n",
                      "sfntFileFontObject::Init(const Unicode&,int,const char&,enum FontFormats,int)",
                      "switch(GET32(theHeader.tag)); case 0x74727565"
                      );
#endif
              return false;
            }


            offsets[0] = 0;
            if ( ! ReadNameTable ( this ) )
              ret = false;
            if (!VerifyTrueTypeFile(this)) {
#ifdef DEBUG
                fprintf(stderr, "Warning: TrueType font file `%s` is corrupted.\n", (char *) &localPlatformName);
#endif
                ret = false;
            }
            break;
        }
        default:
            ret = false;
#ifdef DEBUG
            fprintf(stderr, "Warning: Unrecognized font file format: `%s`.\n", (char *)&localPlatformName);
#endif
	    break;
    }


    if (logicalNumber < fFontCount) {
        fCurFont = logicalNumber;
    } else {
#ifdef DEBUG
        fprintf(stderr,"Incorrect logical font number: %d, number of logical fonts: %d.\n", logicalNumber, fFontCount);
#endif
        fCurFont = fFontCount-1;
    }
    return ret;
}

#if SUSPECT_CODE
int sfntFileFontObject::GetName(UInt16& platformID, UInt16& scriptID, UInt16& languageID, UInt16& nameID, UInt16 *name)
{
    Unicode * ustr;

    /* Fast path, use cached names */
    if ((platformID==3 || platformID==0 || platformID==0xffff) &&
        (nameID==kFullName || nameID==kFamilyName)) {
        if (!fFullName) {
            this->ReadNames();
        }

        switch (nameID) {
        case kFullName:
            if (languageID == DEFAULTLOCID && fFullNameAlt)
                ustr = fFullNameAlt;   /* Use native name */
            else
                ustr = fFullName;      /* Use English name */
            break;

        case kFamilyName:
            if (languageID == DEFAULTLOCID && fFamilyNameAlt)
                ustr = fFamilyNameAlt; /* Use native name */
            else
                ustr = fFamilyName;    /* Use English name */
            break;
        }

        platformID = 3;
        scriptID   = 1;
        return ustr_copy((Unicode *)name, ustr);
    }

  // Changed search a little: If an exact match is not found, try to at
  // least match the platform.
  // Old behavior was to pick the first record with the right nameID, but this
  // may be for another platform.

#ifdef PRINT_INFO
//    fprintf(stderr, "sfntFileFontObject::GetName(%d)\n", meaningID);
#endif

    const sfntNameHeader* nameTable = (sfntNameHeader*)fFontNameTable;

    if ( nameTable == NULL ) {
      FOTempTable table(this, nameFontTableTag);
      nameTable = (sfntNameHeader*)table.CreateTable();
      fFontNameTable = (char *) nameTable;
    }
    const sfntNameRecord* nameRecord = (sfntNameRecord*) ((UInt8*) nameTable +
                                                          sizeof_sfntNameHeader);
    const sfntNameRecord* recordToUse = NULL;
    const sfntNameRecord* firstNameMatch = NULL;
    const sfntNameRecord* firstPlatformMatch = NULL;

    short count = GET16(nameTable->count);


#ifdef PRINT_INFO
    fprintf(stderr,"count: %d.\n", count);
#endif

#ifdef PRINT_INFO
    // fprintf(stderr,"GetName: CP 1\n");
#endif

    for (--count; count >= 0; --count) {
#ifdef PRINT_INFO
      fprintf(stderr, "platformID: %d, scriptID: %d, languageID: %d, nameID: %d.\n",
                nameRecord->platformID, nameRecord->scriptID, nameRecord->languageID, nameRecord->nameID);
#endif
      // Do wildcard names really make sense?  I mean, can you really use
      // a result which may be the family name, copyright notice, manufacturer
      // name, or one of several URL's?
      if (nameID == 0xFFFF || nameID == GET16(nameRecord->nameID)) {
        if (firstNameMatch==NULL) {
          firstNameMatch = nameRecord;
        }

        if (platformID == 0xFFFF || GET16(nameRecord->platformID) == platformID) {

          if (firstPlatformMatch==NULL) {
            firstPlatformMatch = nameRecord;
          }

          if ((scriptID   == 0xFFFF || GET16(nameRecord->scriptID) == scriptID) &&
              (languageID == 0xFFFF || GET16(nameRecord->languageID) == languageID)) {

            recordToUse = nameRecord;
            break;
          }
        }
      }
      nameRecord++;
    }

    if (recordToUse == NULL) {
      if (firstPlatformMatch != NULL) {
        recordToUse = firstPlatformMatch;
      }
      else if (firstNameMatch != NULL) {
        recordToUse = firstNameMatch;
      }
    }

    if (recordToUse != NULL) {

        // write back the arguments
        platformID = GET16(recordToUse->platformID);
        scriptID   = GET16(recordToUse->scriptID);
        languageID = GET16(recordToUse->languageID);
        int  len = GET16(recordToUse->length);

        if (name) {
            char *buffer = (char *)name;
            memcpy(name, ((UInt8*)nameTable + GET16(nameTable->stringOffset))
                   + GET16(recordToUse->offset), len);
            if (platformID == 3 && scriptID == 1) {
                Unicode * uname = (Unicode *)name;
                int     i;

                len >>= 1;
#if defined(_LITTLE_ENDIAN)
                for (i=0; i<len ; i++) {
                    uname[i] = SWAPW(uname[i]);
                }
#endif
            }
            buffer[len]   = 0;
            buffer[len+1] = 0;
        } else {
            if (platformID == 3 && scriptID == 1)
                len >>= 1;
        }
        return len;
    } else {
        return 0;
    }
}
#else // !SUSPECT_CODE
int sfntFileFontObject::GetName(UInt16& platformID, UInt16& scriptID, UInt16& languageID, UInt16& nameID, UInt16 *name)
{
  // Changed search a little: If an exact match is not found, try to at 
  // least match the platform.
  // Old behavior was to pick the first record with the right nameID, but this
  // may be for another platform.

#ifdef PRINT_INFO
//    fprintf(stderr, "sfntFileFontObject::GetName(%d)\n", meaningID);
#endif
  
    const sfntNameHeader* nameTable = (sfntNameHeader*)fFontNameTable;

    if ( nameTable == NULL ) {
      FOTempTable table(this, nameFontTableTag);
      nameTable = (sfntNameHeader*)table.CreateTable();
      fFontNameTable = (char *) nameTable;
    }
    const sfntNameRecord* nameRecord = (sfntNameRecord*) ((UInt8*) nameTable + 
                                                          sizeof_sfntNameHeader);
    const sfntNameRecord* recordToUse = NULL;
    const sfntNameRecord* firstNameMatch = NULL;
    const sfntNameRecord* firstPlatformMatch = NULL;
        
    short count = GET16(nameTable->count);
    

#ifdef PRINT_INFO
    fprintf(stderr,"count: %d.\n", count);
#endif

    // fprintf(stderr,"GetName: CP 1\n");

#ifdef PRINT_INFO
      fprintf(stdout, "GetName platformID: %x, scriptID: %x, languageID: %x, nameID: %x.\n",
                platformID, scriptID, languageID, nameID);
#endif

    for (--count; count >= 0; --count) {

      // Do wildcard names really make sense?  I mean, can you really use
      // a result which may be the family name, copyright notice, manufacturer
      // name, or one of several URL's?
      if (nameID == 0xFFFF || nameID == GET16(nameRecord->nameID)) {
        if (firstNameMatch==NULL) {
          firstNameMatch = nameRecord;
        }

        if (platformID == 0xFFFF || GET16(nameRecord->platformID) == platformID) {

          if (firstPlatformMatch==NULL) {
            firstPlatformMatch = nameRecord;
          }

          if ((scriptID   == 0xFFFF || GET16(nameRecord->scriptID) == scriptID) &&
	      (languageID == 0xFFFF || GET16(nameRecord->languageID) == languageID)) {

	    recordToUse = nameRecord;
	    break;
	  }
	}
      }
      nameRecord++;
    }

    if (recordToUse == NULL) {
      if (firstPlatformMatch != NULL) {
	recordToUse = firstPlatformMatch;
      }
      else if (firstNameMatch != NULL) {
	recordToUse = firstNameMatch;
      }
    }

    if (recordToUse != NULL) {

	// write back the arguments
	platformID = GET16(recordToUse->platformID);
	scriptID   = GET16(recordToUse->scriptID);
	languageID = GET16(recordToUse->languageID);

#ifdef PRINT_INFO
      fprintf(stdout, "GetName found platformID: %x, scriptID: %x, languageID: %x, nameID: %x.\n",
                platformID, scriptID, languageID, nameID);
#endif

	int len = GET16(recordToUse->length);
        if (name) {
            char *buffer = (char *)name;
            memcpy(name, ((UInt8*)nameTable + GET16(nameTable->stringOffset))
                   + GET16(recordToUse->offset), GET16(recordToUse->length));
            buffer[len] = 0;
            buffer[len+1] = 0;

            if (platformID == 3){
                if (scriptID == 1 || scriptID == 0) {
                    Unicode * uname = (Unicode *)name;
		    int     i;
		    len >>= 1;

#if defined(_LITTLE_ENDIAN)
		    for (i=0; i<len ; i++) {
		      uname[i] = SWAPW(uname[i]);
		    }
#endif
		}
		else{
		/*MSoft supports scriptID 2,3,4,5,6 in previous version*/
                    Unicode * uname = makeNameU((UInt8*)name, len, scriptID);
                    if (uname){
                        memcpy(name, uname, len);
                        len >>= 1;
                        delete uname;
		    }
		}
	    }
        }
	return len;
    } else {
        return 0;
    }
}
#endif

// REMIND: need locale preference in request.
const Unicode *sfntFileFontObject::GetFontName(int& nameLen) {
    if (!fFontname) {
        this->ReadNames();
    }

    nameLen = fFontNameLen;
    return fFontname;
}



#define kGSUBTableTag 0x47535542
#define kmortTableTag 0x6D6F7274
#define kGDEFTableTag 0x47444546
#define kGPOSTableTag 0x47504F53

/*
 * Get a layout table with caching
 */
const void * sfntFileFontObject::GetTable(UInt32 tableTag) {
    char * * tpos;
    char *noTable = (char *) (intptr_t)-1;

    switch (tableTag) {
    case kGSUBTableTag:
        tpos = &fGsubTable;
        break;

    case kmortTableTag:
        tpos = &fMortTable;
        break;

    case kGDEFTableTag:
        tpos = &fGdefTable;
        break;

    case kGPOSTableTag:
        tpos = &fGposTable;
        break;

    default:
        tpos = &noTable;
        break;
    }

    if (*tpos == (void *) (intptr_t)-1) {     /* No table present */
        return (void *) NULL;
    }

    if (*tpos != NULL) {                    /* Table cached */
        return *tpos;
    }

    UInt32 offset;
    UInt32 length = this->FindTableSize(tableTag, &offset);


    if (length == 0) {
        *tpos = (char *) (intptr_t)-1;
        return (void *) NULL;
    }

    char * tbl = new char[length];

//  char tname[5];
//  memcpy(tname, &tableTag, 4);
//  tname[4] = 0;
//  printf("Read %s table\n", tname);

    this->ReadChunk(offset, length, (void *)tbl);
    *tpos = tbl;

    return tbl;
}


UInt32 sfntFileFontObject::FindTableSize(UInt32 tableTag, UInt32* offset)
{
#ifdef PRINT_INFO
    fprintf(stderr, "sfntFileFontObject::FindTableSize()\n");
#endif

    sfntDirectory       dir;
    (void)this->ReadChunk(offsets[fCurFont], sizeof_sfntDirectory, &dir);

    int numTables = GET16(dir.numOffsets);

    UInt32 tableOffset = 0;
    UInt32 tableLength = 0;

    if (numTables > 0) {
        const sfntDirectoryEntry* dirEntry =
            (sfntDirectoryEntry*)this->ReadChunk(offsets[fCurFont] + sizeof_sfntDirectory,
                                                 (UInt32)sizeof(sfntDirectoryEntry) *
                                                 numTables);

        const sfntDirectoryEntry *entry = dirEntry;
        const sfntDirectoryEntry *stop = entry + numTables;

        if( nil == entry ) {
#ifdef DEBUG
          fprintf(
                  stderr,"ReadChunk returns nil. fontObject.cpp, %s\n",
                  "sfntFileFontObject::FindTableSize(UInt32, UInt32*)"
                  );
#endif
          return tableLength;
        }

        do {
            if (GET32(entry->tableTag) == tableTag) {
                tableLength = GET32(entry->length);
                tableOffset = GET32(entry->offset);
                break;
            }
        } while (++entry < stop);

        this->ReleaseChunk(dirEntry);
    }
    // don't return something if we didn't find the table
    if (offset && tableOffset) {
        *offset = tableOffset;
    }
    return tableLength;
}

/////////////////////////////
// static jboolean ttContainsHintsTable(sfntFileFontObject *ttFO) {
// // If a font is hinted it should have atleast one of these tables

// #define tag_cvt  0x63767420
// #define tag_fpgm 0x6670676d
// #define tag_prep 0x70726570

//    if ( ttFO->fHintsTableChecked ) {
//       return ttFO->fContainsHintsTable;
//    }

//     UInt32 tableSize = 0;

//     ttFO->fHintsTableChecked = true;
//     //    return false; // this is a temporary hack to force it to go thru t2k scalers.

//     unsigned char *hintTable = (unsigned char *) ttFO->ReadTable(tag_cvt, &tableSize);
//     if ( hintTable != NULL ) {
//       ttFO->fContainsHintsTable = true;
//       ttFO->ReleaseChunk(hintTable);
//       return ttFO->fContainsHintsTable;
//    }

//     hintTable = (unsigned char *) ttFO->ReadTable(tag_fpgm, &tableSize);
//     if ( hintTable != NULL ) {
//       ttFO->fContainsHintsTable = true;
//       ttFO->ReleaseChunk(hintTable);
//       return ttFO->fContainsHintsTable;
//    }

//     hintTable = (unsigned char *) ttFO->ReadTable(tag_prep, &tableSize);
//     if ( hintTable != NULL ) {
//       ttFO->fContainsHintsTable = true;
//       ttFO->ReleaseChunk(hintTable);
//       return ttFO->fContainsHintsTable;
//    }


//    ttFO->fContainsHintsTable = false;
//    return ttFO->fContainsHintsTable;

// }

#ifdef __solaris__

static int useNR = -1;

int useNativeScaler() {
  if (useNR == -1) {
      char *scaler = getenv("J2DFONTSCALER");
      if (scaler != NULL && (strcmp("t2k", scaler)==0)) {
        fprintf(stderr,"using T2K scaler\n");
        useNR = 0;
      } else {
        useNR = 1;// this is our "default"
      }
  }
  return useNR;
}

int isUniformScale(FontTransform& tx) {
    float mat[4];
    tx.getMatrixInto(mat, 4);
    return (mat[1] == 0.0 && mat[2] == 0.0 &&
	    mat[0] > 0.0 && mat[0] == mat[3]);
}
#endif

enum FontFormats
sfntFileFontObject::getScalerID(
                        FontTransform& tx,
                        jboolean isAntiAliased,
                        jboolean usesFractionalMetrics)
{


#ifdef __solaris__

    // The t2k rasterizer is now used for TT & Type 1 fonts for most cases.
    // But Solaris sometimes has separate bitmap versions of fonts for key
    // point sizes which are tuned to look better than the T2K scaler can do.
    // These external PCF custom bitmap files (eg for Solaris CJK fonts) can
    // only be located & understood by the Xserver (native rasteriser).
    // But use of the native rasteriser is subject to several conditions :-
    // require opaque, non-antialiased, non-fractional metrics
    // require simple scaling transform (pointSize)
    // require that we haven't disabled this via system property
    // require that there is an Xserver (headful)
    // require recognition by native system (GetNumGlyphs() > 0)

  if (!isAntiAliased && !usesFractionalMetrics && isUniformScale(tx) &&
       useNativeScaler() &&!AWTIsHeadless() && !fUseT2KRasterizer &&
       GetRealStyle() ==  m_currentStyle && 
      (GetNumberOfNativeNames() > 0)) {

      Strike *theStrike =
	  new CStrike(*this, tx, isAntiAliased, usesFractionalMetrics);
      if (theStrike->GetNumGlyphs()) {
	  setStrike(theStrike, tx, isAntiAliased, usesFractionalMetrics);
	  return kNRFontFormat;
      } else {
	  delete theStrike;
      }
    }
#endif
    return fontObject::getScalerID(tx, isAntiAliased, usesFractionalMetrics);
}

///////////////////////////////////////////////////////////////////////

static int fontListCount = 0;
static struct fontListNode {
    int           type;
    fontObject   *font;
    fontListNode *next;
} *fontList = nil, *fontListTail = nil;



static void UnicodeToLower ( UInt16 *theString, int nameLen )
{
  for ( int i=0; i< nameLen; i++) {

    if ( theString[i] > 0x0040 && theString[i] < 0x005B) {
      theString[i] += 0x0020;
    }
  }
}

Boolean UnicodeStringContains ( UInt16 *srcStr, Int16 srcLen, UInt16 *subStr, Int16 subLen )
{
  //  printf ( " The src and subString " );
  //  printUnicode ( srcStr, srcLen );
  //  printUnicode ( subStr, subLen );

  for ( int i=0; i < srcLen; i++ ) {

    if ( srcStr[i] == subStr[0] ) {

      if ( i + subLen > srcLen )
	return false;
      Boolean match = true;
      for ( int j=0; j<subLen; j++) {
	if ( srcStr[i+j] != subStr[j] ) {
	  match = false;
	  break;
	}
      }
      if ( match == true )
	return true;
    }
  }
  return false;
}

static Boolean allocatedStyles = false;

// This function basically has a set of hardcoded common style names
// if new ones are discrovered they may be added to this list and
// updated for matching the styles to the names.

static const char* boldNames[] = {
          "bold", "demibold", "demi-bold", "demi bold", "negreta", "demi" };
static const char* italicNames[] = {
          "italic", "cursiva", "oblique", "inclined" };
static const char* boldItalicNames[] = {
          "bolditalic", "bold-italic", "bold italic", 
          "boldoblique", "bold-oblique", "bold oblique",
          "demibold italic", "negreta cursiva","demi oblique" };
static UInt16 **boldUnicodeStr, **italicUnicodeStr, **boldItalicUnicodeStr;


static int AssignStyles(int nameLen, UInt16 *theName)
{
    int  theStyle = 0;

    int numBoldStrings, numItalicStrings, numBoldItalicStrings;
    numBoldStrings = sizeof ( boldNames )/sizeof ( char *);
    numItalicStrings = sizeof ( italicNames )/sizeof ( char *);
    numBoldItalicStrings = sizeof ( boldItalicNames )/sizeof ( char *);

    UInt16 *lowerNameStr = new UInt16[nameLen];
    int i;
    for ( i=0; i < nameLen; i++ ) {
      lowerNameStr[i] = theName[i];
    }
    UnicodeToLower ( lowerNameStr, nameLen );

    if ( ! allocatedStyles ) {

      // allocate the Unicode comparison strings for the different styles
      boldUnicodeStr = new UInt16 *[numBoldStrings];
      italicUnicodeStr = new UInt16 *[numItalicStrings];
      boldItalicUnicodeStr = new UInt16 *[numBoldItalicStrings];

      int length = 0;
      int i;
      for ( i = 0; i < numBoldStrings; i++) {
	length = 0;
	boldUnicodeStr[i] = strDupASCIItoUNICODE (boldNames[i], length );
      }
      for ( i =0;  i < numItalicStrings; i++ ) {
	length = 0;
	italicUnicodeStr[i] = strDupASCIItoUNICODE (italicNames[i], length );
      }
      for ( i =0; i < numBoldItalicStrings; i++ ) {
	length = 0;
	boldItalicUnicodeStr[i] = strDupASCIItoUNICODE (boldItalicNames[i], length );
      }
      allocatedStyles = true;
    }

    Boolean styleMatch = false;

    int k;
    // check to see if there is a bold italic match
    for ( k = 0; k < numBoldItalicStrings; k++ ) {
      if ( UnicodeStringContains(lowerNameStr, nameLen, boldItalicUnicodeStr[k], 
			       strlen (boldItalicNames[k] ))) {
	styleMatch = true;
	theStyle = 3;
	break;
      }
    }
    // check to see if there is a italic match
    if ( !styleMatch ) {
      for ( k = 0; k < numItalicStrings; k++ ) {
	if ( UnicodeStringContains(lowerNameStr, nameLen, italicUnicodeStr[k], 
			       strlen (italicNames[k] ))) {
	  styleMatch = true;
	  theStyle = 2;
	  break;
	}
      }
    }
    // check to see if there is a bold match
    if ( !styleMatch ) {
      for ( k = 0; k < numBoldStrings; k++ ) {
	if ( UnicodeStringContains(lowerNameStr, nameLen, boldUnicodeStr[k], 
			       strlen (boldNames[k] ))) {
	  styleMatch = true;
	  theStyle = 1;
	  break;
	}
      }
    }

    //  printf ( "In assign styles the font name is " );
    //  printUnicode ( lowerNameStr, nameLen );
    delete[] lowerNameStr;
    //  printf ( " The style is %d\n", theStyle );

    return theStyle;
}

static void setRealStyle(fontObject *fo)
{
    int theNameLen = 0;
    UInt16 platformID = 3;
    UInt16 scriptID = 1;
    UInt16 languageID = 0xFFFF;
    UInt16 nameID = kFullName;
    UInt16 theName[256];
    UInt16 theFullName[256];

    theNameLen = fo->GetName(platformID, scriptID, languageID, nameID, theName);
    if ( platformID == 3 || platformID == 0 ) {
	for (int j = 0; j < theNameLen; j++) {
	  theFullName[j] = theName[j];
	}
    } else {
      // If it is an ascii string copy into a unicode string 
       unsigned char *theNamePtr = (unsigned char *) theName;
       for ( int j=0; j < theNameLen; j++ ) {
	 theFullName[j] = *theNamePtr++;
       }
    }

    fo->fRealStyle = AssignStyles(theNameLen, theFullName);
}

static void addToTheListHead(int type, fontObject *fo)
{
    fontListNode *node = new fontListNode;

    if (node) {
        node->type = type;
        node->font = fo;
        node->next = fontList;
        fontList = node;

        if (!fontListTail) {
            fontListTail = node;
        }

	setRealStyle(fo);
        fontListCount++;
    }
}

static void addToTheListTail(int type, fontObject *fo)
{
    fontListNode *node = new fontListNode;
 
    if (node) {
        node->type = type;
        node->font = fo;
        node->next = nil;

        if (!fontList) {
            fontList = node;
            fontListTail = node;
        } else {
	    fontListNode *temp = fontList;
	    // go past the composite fonts
	    while ( temp != NULL && temp->type == kCompositeFontFormat ) {
	      temp = temp->next;
	    }
	    // go past the existing truetype fonts
	    fontListNode *cur = temp;
	    while ( temp != NULL && temp->type == kTrueTypeFontFormat ) {
	      cur = temp;
	      temp = temp->next;
	    }
	    // insert TT font after "cur" which may be the tail node.
	    if ( type == kTrueTypeFontFormat && 
		 cur != NULL && cur != fontListTail ) {
	        node->next = cur->next;
		cur->next = node;
	    } else {
	        fontListTail->next = node;
		fontListTail = node;
	    }
        }

	setRealStyle(fo);
        fontListCount++;
    }
}


static fontListNode* getNodeByNumber(int n)
{
    fontListNode* node = fontList;

    while (n && node && node->next) {
        node = node->next;
        n--;
    }

    return node;
}

#ifdef PRINT_INFO
static void dumpList()
{
    fontListNode *node = fontList;
    int i(0);

    while (node) {
        printf("%d: %s `%s`\n", i++, ((fileFontObject *) node->font)->GetFileName(),
               node->font->GetFontName());
        node = node->next;
    }
}
#endif

extern "C" {

#if 0
void FillBytes(void *destination, long length, long pattern) {
        memset(destination, pattern, length);
}
#endif

/*
 * Class:     sun_awt_font_NativeFontWrapper
 * Method:    registerFonts
 * Signature: (Ljava/util/Vector;II)V
 */
JNIEXPORT void JNICALL Java_sun_awt_font_NativeFontWrapper_registerFonts
        (JNIEnv *env, jclass obj, jobject fonts, jint size,
         jobject names, jint jFormat, jboolean useJavaRasterizer)
{
    // REMIND this is an evil cast -- use types from javah headers
    enum FontFormats format = (enum FontFormats)jFormat;
    jclass vectorClass;
    jmethodID elementAtID;
    jmethodID sizeID;
    jstring fileName;
    jstring nativeString;
    jobject nameVector;
    jobject nameObject;
    const Unicode *fontFileName;
    int fontFileNameLen;
    const char *localPlatformName;
    const char *nativeName = NULL;
    int numNames = 0;
    int i;
    static jboolean scalerRegistered = false;

    if (!scalerRegistered) {
	debugFonts = (getenv("JAVA2D_DEBUGFONTS") != NULL) ? 1 : 0;
        hsGFontScaler::Register(kT2KFontFormat, new t2kFontScaler);
        if (!AWTIsHeadless()) {
            hsGFontScaler::Register(kNRFontFormat, new nrFontScaler);
        }
        scalerRegistered = true;
    }

    /* Need (size * 2) + 2 local references. size + 2 because
     * two local refs per font (fileName and nativeName) and
     * '+ 2' for vectorClass and elementAtID.
     */
    if (env->PushLocalFrame ((size * 2) + 2) < 0) {
        return;
    }

    vectorClass = env->GetObjectClass(fonts);
    sizeID = env->GetMethodID(vectorClass, "size", "()I");
    elementAtID = env->GetMethodID(vectorClass, "elementAt",
                                   "(I)Ljava/lang/Object;");

    if ((elementAtID == NULL) || (sizeID == NULL)) {
        JNU_ThrowInternalError(env, "elementAtID/sizeID failed\n");
        env->PopLocalFrame (0);
        return;
    }
    for (i=0; i<size; i++) {
	numNames = 0;
	nativeName = NULL;
	nativeString = NULL;
	nameVector = NULL;
        fileName = (jstring) env->CallObjectMethod(fonts, elementAtID, i);
        if (JNU_IsNull(env, fileName)) {
            JNU_ThrowInternalError(env, "Empty file name in registerFonts().\n");
            env->PopLocalFrame (0);
            return;
        }
        localPlatformName = JNU_GetStringPlatformChars (env, fileName, NULL);
        //printf("format: %d, file: %s\n", format, localPlatformName);
	// name be a string, or a vector of Strings.

	/* Note: useJavaRasterizer is used in this function to avoid
	 * creating "native" names and mappers. if its true then we don't
	 * create native names, and hence create no native mappers. This is a
	 * "good thing" as it prevents unnecessary construction of very large
	 * native char to glyph mappers and CStrike objects which are
	 * irrelevant since native will never be used. You might ask why we
	 * have native names for such fonts. Currently its for the JRE fonts
	 * where a X11 fonts.dir exists for the convenience of a developer who
	 * may wish to add that to his X font path to enable use of these
	 * fonts on AWT heavyweights or in other X apps.
	 */
	if (useJavaRasterizer == JNI_FALSE) {
	    nameObject = env->CallObjectMethod(names, elementAtID, i);
	    if (env->IsInstanceOf(nameObject, vectorClass)) {
		nameVector = nameObject;
		numNames = env->CallIntMethod(nameVector, sizeID);
		if (numNames > 0) {
		    nativeString = (jstring)
			env->CallObjectMethod(nameVector, elementAtID, 0);
		}
	    } else {
		nameVector = NULL;
		
		nativeString = (jstring)nameObject;
		if (!JNU_IsNull(env, nativeString)) {
		    numNames = 1;
		}
	    }
	    if (!JNU_IsNull(env, nativeString)) {
		nativeName = 
		    JNU_GetStringPlatformChars (env, nativeString, NULL);
	    }
	}
        switch (format) {
            case kTrueTypeFontFormat: {
                sfntFileFontObject* ffo;
                int j(0);
                do {// adg: at least one logical font in a TrueType or TTC file
                    ffo = new sfntFileFontObject;
                    Boolean valid(false);
                    {
                        JStringBufferCopy theFileName(env, fileName);
                        if (theFileName.invalid()) {
                            JNU_ReleaseStringPlatformChars(env,
							   fileName,
							   localPlatformName);
			    if (!JNU_IsNull(env, nativeString)) {
				JNU_ReleaseStringPlatformChars(env,
							       nativeString,
							       nativeName);
			    }
                            JNU_ThrowInternalError(env,
                                    "Empty file name in registerFonts().\n");
                            env->PopLocalFrame (0);
                            return;
                        }
                        fontFileName = theFileName.buffer();
                        fontFileNameLen = theFileName.getLength();
                        valid = ffo->Init(fontFileName, fontFileNameLen,
					  localPlatformName, nativeName,
					  kTrueTypeFontFormat, j++);
			if (nameVector != NULL) {
			    jstring nativeStr;
			    const char *nativeName2;
			    if (numNames > 1) {
				if (env->PushLocalFrame(numNames-1) < 0) {
				    env->PopLocalFrame (0); 
				    return;
				}
				for (int i=1; i<numNames; i++) {
				    nativeStr =(jstring)
					env->CallObjectMethod(nameVector,
							      elementAtID, i);
				    nativeName2 =
					JNU_GetStringPlatformChars(env,
								   nativeStr,
								   NULL);
				    ffo->AddNativeName(nativeName2);
				    JNU_ReleaseStringPlatformChars(env,
								   nativeStr,
								   nativeName2);
				}
				env->PopLocalFrame (0);
			    }
			}
                    }
                    if (!valid) {
		        if (debugFonts) {
		            fprintf(stderr,"Bad TrueType font %s\n",localPlatformName);
			}
                        delete ffo;
			ffo = NULL;
			break;
                    } else {
                        byte *cmap; // dummy entry
                        //addToTheListTail(format, ffo);

                        InitializeCMAP(*ffo, cmap);
			if ( !cmap ) {
			    if (debugFonts) {
			        fprintf(stderr,"No TrueType cmap %s\n",localPlatformName);
			    }
			    delete ffo;
			    ffo = NULL;
			    break;
			} else {
			    delete cmap; // make sure we do not add to the footprint
			    addToTheListTail(format, ffo);
			    ffo->fUseT2KRasterizer = useJavaRasterizer;
			}
                    }
                } while (ffo != NULL && j < ffo->getLogicalFontCount());
                break;
            }
            case kType1FontFormat: {
                type1FileFontObject* ffo = new type1FileFontObject;
                Boolean valid(false);
                {

                    JStringBufferCopy theFileName(env, fileName);
                    if (theFileName.invalid()) {
                        JNU_ReleaseStringPlatformChars(env, fileName, localPlatformName);
			if (!JNU_IsNull(env, nativeString)) {
			    JNU_ReleaseStringPlatformChars(env, nativeString, nativeName);
			}
                        JNU_ThrowInternalError(env, "Empty file name in registerFonts().\n");
                        env->PopLocalFrame (0);
                        return;
                    }
                    fontFileName = theFileName.buffer();
                    fontFileNameLen = theFileName.getLength();
                    valid = ffo->Init (fontFileName, fontFileNameLen, 
				       localPlatformName, nativeName, 
				       kType1FontFormat);
		    if (nameVector != NULL) {
			jstring nativeStr;
			const char *nativeName2;
			if (numNames > 1) {
			    if (env->PushLocalFrame(numNames-1) < 0) {
				env->PopLocalFrame (0); 
				return;
			    }
			    for (int i=1; i<numNames; i++) {
				nativeStr = (jstring)env->CallObjectMethod(nameVector, elementAtID, i);
				nativeName2 = JNU_GetStringPlatformChars (env, nativeStr, NULL);
				ffo->AddNativeName(nativeName2);
				JNU_ReleaseStringPlatformChars(env, nativeStr, nativeName2);
			    }
			    env->PopLocalFrame (0);
			}
		    }
                }

		// now check to see if it is a multiple master font 
		if (valid) {
		    FOTempFontInfo fInfo(ffo);
		    t1FontInfo *fi = fInfo.GetFontInfo();
		    if (fi != NULL && fi->multipleMasterFont == true) {
		      // printf ( "Its a mm font \n" );
			valid = false;
		    }
		}

                if (!valid) {
		    if (debugFonts) {
		        fprintf(stderr,"Bad Type1 font %s\n",localPlatformName);
		    }
                    delete ffo;
		    ffo = NULL;
		    break;
                } else {
                    addToTheListTail(format, ffo);
		    ffo->fUseT2KRasterizer = useJavaRasterizer;
                }
                break;
            }
            case kNRFontFormat: {
                fileFontObject * ffo = GetNativeFontObject ();
                Boolean valid(false);
                {
                    JStringBufferCopy theFileName(env, fileName);
                    if (theFileName.invalid()) {
                        JNU_ReleaseStringPlatformChars(env, fileName, localPlatformName);
			if (!JNU_IsNull(env, nativeString)) {
			    JNU_ReleaseStringPlatformChars(env, nativeString, nativeName);
			}
                        JNU_ThrowInternalError(env, "Empty file name in registerFonts().\n");
                        env->PopLocalFrame (0);
                        return;
                    }
                    fontFileName = theFileName.buffer();
                    fontFileNameLen = theFileName.getLength();
                    valid = ffo->Init (fontFileName, fontFileNameLen, 
				       localPlatformName, nativeName, 
				       kNRFontFormat);
                }
                if (!valid) {
		    if (debugFonts) {
		        fprintf(stderr,"Bad Native font %s\n",localPlatformName);
		    }
                    delete ffo;
		    ffo = NULL;
		    break;
                } else {
                    addToTheListTail(format, ffo);
		    // knrFonts should use the native font rasterizers :-
		    ffo->fUseT2KRasterizer = false;
                }
                break;
            }
            default:
                break;
        }
        JNU_ReleaseStringPlatformChars(env, fileName, localPlatformName);
	if (!JNU_IsNull(env, nativeString)) {
	    JNU_ReleaseStringPlatformChars(env, nativeString, nativeName);
	}
    }

#ifdef PRINT_INFO
    dumpList();
#endif

    env->PopLocalFrame (0);
}

} // extern "C"

//////////////// Name cache /////////////////
// #define  USE_FONTNAME_CACHE

#ifdef USE_FONTNAME_CACHE
class fontNameCache {
    struct Entry {
        Entry           *fNext;

        int             fLength;
        const Unicode   *fName;
        fontObject      *fFO;
    };
    Entry*      fEntry;
public:
    fontNameCache();
    ~fontNameCache();

    fontObject *Find(const Unicode name[], int nameLen) const;
    void Add(const Unicode name[], int nameLen, fontObject *fo);
};

fontNameCache::fontNameCache() : fEntry(nil)
{
}

fontNameCache::~fontNameCache()
{
    Entry   *entry = fEntry;

    while (entry) {
        Entry* next = entry->fNext;
        delete [] (Unicode *)entry->fName;
        delete entry;
        entry = next;
    }
}

fontObject *fontNameCache::Find(const Unicode name[], int nameLen) const
{
    const Entry* entry = fEntry;

    while (entry) {
        if (nameLen == entry->fLength &&
                memcmp(name, entry->fName, sizeof(Unicode) * nameLen) == 0) {
            return entry->fFO;
        }
        entry = entry->fNext;
    }
    return nil;
}

inline Unicode *DupName(const Unicode name[], int nameLen)
{
    Unicode *copy = new Unicode [nameLen];

    if (nil != copy ) {
        for (int i = 0; i < nameLen; i++) {
            copy[i] = name[i];
        }
    } else {
        #ifdef DEBUG
            fprintf(stderr,"new returns nil, fontObject.cpp, fontNameCache:DupName(const Unicode[], int)\n");
        #endif
    }
    return copy;
}

void fontNameCache::Add(const Unicode name[], int nameLen, fontObject *fo)
{
    Entry   *entry = new Entry;

    if (nil != entry ) {
        entry->fNext = fEntry;
        fEntry = entry;

        entry->fLength = nameLen;
        entry->fName = DupName(name, nameLen);
        entry->fFO = fo;
    } else {
        #ifdef DEBUG
            fprintf(stderr,"new returns nil, fontObject.cpp,fontNameCache: Add(const Unicode[], int,fontObject*)\n");
        #endif
    }
}
#endif

//////////////// Public functions /////////////////
#ifdef PRINT_INFO
static Boolean printedOnce = false;
#endif

static int getFamilyNameFromFullName(const UInt16 *name, int nameLen,
                                  UInt16 *familyName) {


    UInt16 platformID = 3;
    UInt16 scriptID = 1;
    UInt16 languageID = 0xFFFF;
    UInt16 nameID = kFamilyName;
    fontListNode *node = fontList;
    fontObject *fo = NULL;
    UInt16 tempFamilyName[1024];
    int tmpLen;
    int familyLen = 0;

    while (node) {
        if (node->font->MatchName(kFullName, name, nameLen) &&
             node->font->GetFormat() != kCompositeFontFormat) {
             fo = node->font;
             tmpLen = fo->GetName(platformID, scriptID, languageID,
                                      nameID, tempFamilyName);
             if (tmpLen > 0) {
                 familyLen = tmpLen;
                 if (platformID == 3 || platformID == 0) {
                     for (int j = 0; j < familyLen; j++) {
                         familyName[j] = SWAPW(tempFamilyName[j]);
                     }
                 } else {
                  // If it is an ascii string copy into a unicode string
                  unsigned char *namePtr = (unsigned char *)tempFamilyName;
                  for (int j=0; j < tmpLen; j++ ) {
                      familyName[j] = *namePtr++;
                  }
              }
              break;
            }
        }
       node = node->next;
    }
    return familyLen;
}

static int printedNodeList = 0;

static fontObject *findFO(const UInt16 *name, int nameLen, int style) {
    fontObject *fo = 0;
    
#ifdef USE_FONTNAME_CACHE
    static fontNameCache* gFontNameCache = nil;
    if (gFontNameCache == nil)
        gFontNameCache = new fontNameCache;

    fo = gFontNameCache->Find(name, nameLen);
    if (fo)
        return fo;
#endif

    fontListNode *node = fontList;

    if (debugFonts) {
	int nc=0;
	while (node&&!printedNodeList) {
	    fontObject *fObj = node->font;
	    node = node->next;
	    int len;
	    Unicode *name = (Unicode*)(fObj->GetFontName(len));
	    fprintf(stderr,"nodefont %d= ", nc++);
	    if (len >0) {
		fprintUnicode(name,len);
	    } else {
		fprintf(stderr,"no-name");
	    }
	    fprintf(stderr," fmt=%d ", fObj->GetFormat());
	    fprintf(stderr," file= ");
	    name = (Unicode*)(fObj->GetFileName(len));
	    if (len >0) {
		fprintUnicode(name,len);
	    } else {
		fprintf(stderr,"no-file");
	    }
	    int nncnt = fObj->GetNumberOfNativeNames();
	    fprintf(stderr," %d native names ", nncnt);
	    
	    if (nncnt > 0) {
		for (int i=0;i<nncnt;i++) {
		    fprintf(stderr,"nn[%d]= %s ",i,
			    (fObj->GetFontNativeName(i) == NULL) ? "null" :
			    fObj->GetFontNativeName(i));
		}
	    }
	    fprintf(stderr,"\n");
	}
	printedNodeList=1;
	node = fontList;
    }

    Boolean foundIt = false;
    //    printf ( "Asking for style = %d and name = ", style );
    //    printUnicode ( name, nameLen );

    // first search for full names if the style is plain 
    // maybe it is requesting the exact font.
    if ( style == 0 ) {

	// This is being added to search first by FamilyNames then by 
	// Font names - sometimes there is a conflict because of strange names
	// especially for Lucida's on Solaris
	while (node) {
	    if (node->font->MatchName(kFamilyName, name, nameLen)) {
		fo = node->font;
		if ( fo->GetRealStyle() == style ) {
		    foundIt = true;
		    break;
		}
	    }
	    node = node->next;
	}

	// This is the search by exact names i.e font names
	// the API new Font could have been a little less ambigiuous as per
	// the font names
	if ( !foundIt ) {
	    node = fontList;
	    while (node) {
		if (node->font->MatchName(kFullName, name, nameLen)) {
		    fo = node->font;
		    foundIt = true;
		    break;
		}
		node = node->next;
	    }
	}
    }

#ifdef PRINT_INFO

    if ( printedOnce == false ) {
    while ( node ) {
       fo = node->font;
       int theNameLen = 0;
       UInt16 platformID = 3;
       UInt16 scriptID = 1;
       UInt16 languageID = 0xFFFF;
       UInt16 nameID = kFullName;
       UInt16 theName[256];
       UInt16 theFullName[256], theFamilyName[256];

       theNameLen = fo->GetName(platformID, scriptID, languageID, nameID, theName);
       if ( platformID == 3 || platformID == 0 ) {
//	 theNameLen /= 2;
	 for (int j = 0; j < theNameLen; j++) {
	   theFullName[j] = theName[j];
	   theFullName[j] = SWAPW ( theFullName[j] );
	 } 
       }
       else {
	   // If it is an ascii string copy into a unicode string 
	   unsigned char *theNamePtr = (unsigned char *) theName;
	   for ( int j=0; j < theNameLen; j++ ) {
	     theFullName[j] = *theNamePtr++;
	   }
       }
       printf ( " The full name is = " );
       printUnicode ( theFullName, theNameLen );

       nameID = kFamilyName;
       theNameLen = fo->GetName(platformID, scriptID, languageID, nameID, theName);
       if ( platformID == 3 || platformID == 0 ) {
	 theNameLen /= 2;
	 for (int j = 0; j < theNameLen; j++) {
	   theFamilyName[j] = theName[j];
	   theFamilyName[j] = SWAPW ( theFamilyName[j] );
	 }
       } 
       else {
	   // If it is an ascii string copy into a unicode string 
	   unsigned char *theNamePtr = (unsigned char *) theName;
	   for ( int j=0; j < theNameLen; j++ ) {
	     theFamilyName[j] = *theNamePtr++;
	   }
       }

       printf ( " The family name is = " );
       printUnicode ( theFamilyName, theNameLen );
       node = node->next;
    }
    }
    printedOnce = true;
#endif
    // As noted above some full names of solaris Type1 Lucida's
    // happen to correspond to family names of the JRE versions.
    // To minimise the amount of loops over the font list it was
    // initially attempted to find a family name from the name passed in
    // by the client, assuming that if what the client passed *was* a
    // family name that there would be no full name matches.
    // This isn't so. It does lead to potential confusion for users -
    // that some fonts have the same names as the family names for some
    // other fonts but its unavoidable.
    // So we need to try to interpreting the client supplied name as a
    // family name first.
    if ( !foundIt ) {
        node = fontList;
    
        fontObject *tempFO = NULL;

        while (node) {
	  // look for a matching family name if there is a style attached to 
	  // the request - just matching a family name is not sufficient
	  // later look at the full name and parse its styles
	  // if the full name has the defined style we try to match it in
	  // AssignStyles and use the matched font - if it fails - we use one from
	  // the family -if all matches fail - then use the default
	  
          if (node->font->MatchName(kFamilyName, name, nameLen)) {
            fo = node->font;
	    int theStyle = style;

	    if ( fo->GetFormat() != kCompositeFontFormat ) {
		  theStyle = fo->GetRealStyle();
	    }

	    //	printf ( "The original style is %d the new style is %d \n", style, theStyle );
	    if ( theStyle == style ) {
	      // printf ( "The styles have matched \n" );
	      tempFO = fo;
	      foundIt = true;
	      break;
	    } else if ((theStyle & style) != 0 || tempFO == NULL) {
	      tempFO = fo;
	    }
          }

          node = node->next;
        }

        fo = tempFO;
    }

    if (!foundIt) {
        fontObject *tempFO = fo; // remember any partial match
        UInt16 familyName[1024];
        int familyLen = getFamilyNameFromFullName(name, nameLen, familyName);

        if (familyLen > 0) {

            node = fontList;
            while (node) {
                if (node->font->MatchName(kFamilyName,familyName, familyLen)) {
                    fo = node->font;
                    int theStyle = style;

                    if (fo->GetFormat() != kCompositeFontFormat ) {
                        theStyle = fo->GetRealStyle();
                    }

                    if (theStyle == style) {
                       tempFO = fo;
                       break;
                    } else if ((theStyle & style) != 0 || tempFO == NULL) {
                       tempFO = fo;
                    }
                }
                node = node->next;
            }
        }
    }    

#ifdef USE_FONTNAME_CACHE
    gFontNameCache->Add(name, nameLen, fo);
#endif
    return fo;
}

#if SUSPECT_CODE
// Generic version, match the 'standard' font names only

/*
 * Names to search for the default font name
 */
static UInt16 DefNames[] = {
   9, 's', 'a', 'n', 's', 's', 'e', 'r', 'i', 'f',
  15, 's', 'a', 'n', 's', 's', 'e', 'r', 'i', 'f','.','p','l','a','i','n',
   6, 'd', 'i', 'a', 'l', 'o', 'g',
  12, 'd', 'i', 'a', 'l', 'o', 'g','.','p','l','a','i','n',
   5, 's', 'e', 'r', 'i', 'f',
  11, 's', 'e', 'r', 'i', 'f','.','p','l','a','i','n',
   7, 'A', 'r', 'i', 'a', 'l', 'M', 'T',
   9, 'H', 'e', 'l', 'v', 'e', 't', 'i', 'c', 'a',
  10, 'L', 'u', 'c', 'i', 'd', 'a', 'S', 'a', 'n', 's',
   0
};

static fontObject *defaultFO() {
    // adg: should check for the best font for a given locale
    static fontObject *defaultFont = 0;
    UInt16 * defn;

    defn = DefNames;
    while (!defaultFont && *defn) {
        fontListNode *node = fontList;
        while (node) {
            if (node->font->MatchName(kFullName, defn+1, *defn)) {
                defaultFont = node->font;
                break;
            }
            node = node->next;
        }
        defn += *defn+1;
    }
    if (defaultFont == 0) {
            // XXX throw a Java exception istead
        fprintf(stderr,"\nFatal error: No usable fonts found.\n\n");
        exit(1);
    }
    return defaultFont;
}
#else // !SUSPECT_CODE

// This has solaris-specific messages in it!
static fontObject *defaultFO() {
    // adg: should check for the best font for a given locale
    static fontObject *defaultFont = 0;
    static fontObject *finalDefaultFont = 0; // will be the lucidasans
    if (defaultFont == 0) {
        fontListNode *node = fontList;
        while (node) {
            int nameLen = 0;
            const Unicode *nodeFontName = node->font->GetFontName(nameLen);
            if (nodeFontName && nameLen) {

                if (equalUnicodeToAsciiNC(nodeFontName, nameLen, "dialog") ||
                    equalUnicodeToAsciiNC(nodeFontName, nameLen, "dialog.plain")) {
                    defaultFont = node->font;
                    break;
                }

		/* Use LucidaSans as default (for Solaris). */
		// Make the default font Lucida Sans Regular - it will have the most
		// glyphs and we can render it accurately.

                if (equalUnicodeToAsciiNC(nodeFontName, nameLen, "Lucida Sans Regular"))
		{
                    finalDefaultFont = node->font;
                }
            }
            node = node->next;
        }

	// This is because we could not find the Arial which is usually the default
	// as a final choice if Lucida Sans Regular is available use it as default
	if ( defaultFont == NULL ) {
	   defaultFont = finalDefaultFont; 
	}
	
	// if still NULL make the default the first font at least  
	if ( defaultFont == NULL && fontList != NULL ) {
	    defaultFont = fontList->font;
	}

        if (defaultFont == NULL) {
            // XXX throw a Java exception istead
            fprintf(stderr,"\nFatal error: No usable fonts found.\n\n");
            exit(1);
        }
    }

    return defaultFont;
}
#endif


void registerFont(enum FontFormats format, fontObject *cf) {
    addToTheListHead(format, cf);
}

fontObject *GetFontObject(const Unicode *fileName, int fileNameLen) {
    if (!fileName || !fileNameLen) {
        return NULL;
    }
    fontListNode *node = fontList;
    while (node) {
        if (node->font) {
            int nodeNameLen = 0;
            const Unicode *nodeFileName = node->font->GetFileName(nodeNameLen);
            if (nodeFileName && nodeNameLen) {
                if (nodeNameLen >= fileNameLen) {
                    int nodeOffset = nodeNameLen - fileNameLen;
                    if (!memcmp(fileName, nodeFileName + nodeOffset, fileNameLen * sizeof(Unicode))) {
                        return node->font;
                    }
                }
            }
        }
        node = node->next;
    }
    return NULL;
}

fontObject* GetFontObject(unsigned index)
{
    if (fontListCount == 0) {
        fprintf(stderr,"\nFatal error: no fonts were loaded.\n\n");
        exit(1);
    }
    assert(index < (unsigned)fontListCount);
    return getNodeByNumber(index)->font;
}


///////////////////////////////////////////////////////////////////////////////


extern "C" {
int CountFontObjects()
{
    return fontListCount;
}

fontObject *DefaultFontObject() {
    return defaultFO();
}

fontObject *FindFontObject(const UInt16 * name, unsigned int nameLen, int style) {
    fontObject *newFO;

    if (!name || (nameLen <= 0)) {
        return DefaultFontObject();
    }
    if (equalUnicodeToAsciiNC(name, nameLen, "Default", strlen("Default"))) {
        return DefaultFontObject();
    }
    //    printf ( "Search for font name " );
    //printUnicode ( name, nameLen );

    newFO = findFO(name, nameLen, style);

    return newFO;
}

const void* FOReadChunk(fontObject *fo, UInt32 offset, UInt32 length)
{
    assert(fo != nil);

    return ((fontObject*)fo)->ReadChunk(offset, length);
}

void FOReleaseChunk(fontObject *fo, const void* chunk)
{
    assert(fo != nil);

    ((fontObject*)fo)->ReleaseChunk(chunk);
}

long FOFindTableSize(fontObject *fo, UInt32 tableTag, UInt32* offset)
{
    assert(fo != nil);

    return ((fontObject*)fo)->FindTableSize(tableTag, offset);
}

const void* FOGetData(fontObject *fo)
{
    assert(fo != nil);

        return ((fontObject*)fo)->GetData();
}

UInt32 FOGetDataLength(fontObject *fo)
{
    assert(fo != nil);

        return ((fontObject*)fo)->GetDataLength();
}


const Unicode *CreateTrueTypeFont ( Unicode *fontFileName, int fontFileNameLen, char *localName, int *nameLen )
{
    sfntFileFontObject* ffo;
    ffo = new sfntFileFontObject;
    Boolean valid = false;
    valid = ffo->Init(fontFileName, fontFileNameLen, localName, localName, kTrueTypeFontFormat, 0);
    if (!valid) {
         delete ffo;
	 return NULL;
     } else {
         addToTheListTail(kTrueTypeFontFormat, ffo);
    }
    *nameLen = 0;

    ffo->fHintsTableChecked = true;
    ffo->fContainsHintsTable = false;
    UInt16 platformID = 3;
    UInt16 scriptID = 1;
    UInt16 languageID = 0xFFFF;
    UInt16 nameID = kFullName;

    Unicode *fontName;

    fontName = new UInt16[256];

    *nameLen = ffo->GetName(platformID, scriptID, languageID, nameID, fontName);
    //   printf ( " The name length found is %d \n", *nameLen/2);
    //   printf ( "The font name is " );
    //   printUnicode ( fontName, *nameLen );

    fontfilecachedelete(ffo); // fixes temporary files not deleted

    if ( *nameLen )
      return fontName;
    return NULL;
}

#ifdef WIN32
/* Code deals with system default eudc font */ 
static sfntFileFontObject*  systemDefaultEUDCFont;
static char*                eudcKey = NULL;

#define EUDCKEY_JA_JP  "EUDC\\932"
#define EUDCKEY_ZH_CN  "EUDC\\936"
#define EUDCKEY_ZH_TW  "EUDC\\950"
#define EUDCKEY_KO_KR  "EUDC\\949"
#define LANGID_JA_JP   0x411
#define LANGID_ZH_CN   0x0804
#define LANGID_ZH_SG   0x1004
#define LANGID_ZH_TW   0x0404
#define LANGID_ZH_HK   0x0c04
#define LANGID_ZH_MO   0x1404
#define LANGID_KO_KR   0x0412

sfntFileFontObject*  getLinkedEUDCFont() {
    int    rc;
    HKEY   key;
    DWORD  type;
    char   fontPathBuf[MAX_PATH + 1];
    UInt32 fontPathLen = MAX_PATH + 1;
    char   tmpPath[MAX_PATH + 1];
    char   *fontPath = fontPathBuf;

    if (eudcKey == NULL) {
        LANGID langID = GetSystemDefaultLangID();
        //lookup for encoding ID, EUDC only supported in
        //codepage 932, 936, 949, 950 (and unicode)
        if (langID == LANGID_JA_JP) {
            eudcKey = EUDCKEY_JA_JP;
        }
        else
        if (langID == LANGID_ZH_CN || langID == LANGID_ZH_SG) {
            eudcKey = EUDCKEY_ZH_CN;
        }
        else
        if (langID == LANGID_ZH_HK || langID == LANGID_ZH_TW || langID == LANGID_ZH_MO) {
            eudcKey = EUDCKEY_ZH_TW;
	}
        else
        if (langID == LANGID_KO_KR) {
           eudcKey = EUDCKEY_KO_KR;
	} 
        else{
           eudcKey = "NoEUDC";
	}

        if (strcmp(eudcKey, "NoEUDC")) {
            rc = RegOpenKeyEx(HKEY_CURRENT_USER, eudcKey, 0, KEY_READ, &key);
            if (rc != ERROR_SUCCESS)
                return NULL;
            rc = RegQueryValueEx(key, 
                                "SystemDefaultEUDCFont", 
                                0, 
                                &type, 
                                (unsigned char*)fontPath,
                                &fontPathLen);
            if (rc != ERROR_SUCCESS || type != REG_SZ) {
                RegCloseKey(key);
                return NULL;
	    }
            RegCloseKey(key);
            fontPath[fontPathLen] = 0;
            if (strstr(fontPath, "%SystemRoot%")) {
                //if the fontPath includes %SystemRoot%
                char* systemRoot = getenv("SystemRoot");
                if (systemRoot != NULL
                    && sprintf(tmpPath, "%s%s", systemRoot, fontPath + 12) != -1) {
                    fontPath = tmpPath;
	        }
                else {
                    eudcKey = "NoEUDC";
                    return NULL;
	        }
	    }
            else
            if (strcmp(fontPath, "EUDC.TTE") == 0) {
                //else to see if it only inludes "EUDC.TTE"
                char systemRoot[MAX_PATH + 1];
                if (GetWindowsDirectory(systemRoot, MAX_PATH + 1) != 0) {
                    sprintf(tmpPath, "%s\\FONTS\\EUDC.TTE", systemRoot);
                    fontPath = tmpPath;
		}
                else {
                    eudcKey = "NoEUDC";
                    return NULL;
		}
	    }
            sfntFileFontObject *ffo = new sfntFileFontObject;
            Boolean valid = false;
            valid = ffo->Init((Unicode*)fontPath,
                              strlen(fontPath),
                              fontPath,
                              fontPath,
                              kTrueTypeFontFormat, 0);
	    if (!valid) {
                delete ffo;
                eudcKey = "NoEUDC";
                return NULL;
	    }
            addToTheListTail(kTrueTypeFontFormat, ffo);
            return systemDefaultEUDCFont = ffo;
	}
    }
    if (!strcmp(eudcKey, "NoEUDC")) {
        return NULL;
    }
    return systemDefaultEUDCFont;
}
#endif

/* The array index corresponds to a bit offset in the TrueType
 * font's OS/2 compatibility table's code page ranges fields.
 * These are two 32 bit unsigned int fields at offsets 78 and 82.
 * We are only interested in determining if the font supports
 * the windows encodings we expect as the default encoding in
 * supported locales, so we only map the first of these fields.
 */
static const char *encoding_mapping[] = {
    "cp1252",    /*  0:Latin 1  */
    "cp1250",    /*  1:Latin 2  */
    "cp1251",    /*  2:Cyrillic */
    "cp1253",    /*  3:Greek    */
    "cp1254",    /*  4:Turkish/Latin 5  */
    "cp1255",    /*  5:Hebrew   */
    "cp1256",    /*  6:Arabic   */
    "cp1257",    /*  7:Windows Baltic   */
    "",          /*  8:reserved for alternate ANSI */
    "",          /*  9:reserved for alternate ANSI */
    "",          /* 10:reserved for alternate ANSI */
    "",          /* 11:reserved for alternate ANSI */
    "",          /* 12:reserved for alternate ANSI */
    "",          /* 13:reserved for alternate ANSI */
    "",          /* 14:reserved for alternate ANSI */
    "",          /* 15:reserved for alternate ANSI */
    "ms874",     /* 16:Thai     */
    "ms932",     /* 17:JIS/Japanese */
    "gbk",       /* 18:PRC GBK Cp950  */
    "ms949",     /* 19:Korean Extended Wansung */
    "ms950",     /* 20:Chinese (Taiwan, Hongkong, Macau) */
    "ms1361",    /* 21:Korean Johab */
    "",          /* 22 */
    "",          /* 23 */
    "",          /* 24 */
    "",          /* 25 */
    "",          /* 26 */
    "",          /* 27 */
    "",          /* 28 */
    "",          /* 29 */
    "",          /* 30 */
    "",          /* 31 */
};


jboolean FOFontSupportsEncoding(JNIEnv *env,
				jobject theFont, jstring theEncoding) {
    tt_uint32 os2Offset=0, os2Length=0;
    tt_uint32 range1=0, range2=0;

    /* reserved bits must not be set, include symbol bits */
    static tt_uint32 reserved_bits1 = 0x80000000;
    static tt_uint32 reserved_bits2 = 0x0000ffff;
    
    fontObject* font = ::getFontPtr(env, theFont);
    /* if this returns non-zero its a truetype/opentype font and 
     * the fields we want are in 32 bit fields at offsets of 78 and 82 bytes
     */
    os2Length = FOFindTableSize(font, os2FontTableTag, &os2Offset);
    if (os2Length < 86) {
      return JNI_FALSE;
    }
    byte *buffer = new byte[os2Length];
    if (buffer == NULL) {
      return JNI_FALSE;
    }
    font->readBlock(os2Offset, os2Length, (char *) buffer);
    range1 = GetUInt32(buffer, 78); /* ulCodePageRange1 */
    range2 = GetUInt32(buffer, 82); /* ulCodePageRange2 */
    delete buffer;
    /* if any "reserved bits are set, return false */
    if ((range1 & reserved_bits1) | (range2 & reserved_bits2)) {
      return JNI_FALSE;
    }

    /* see if the bit is set that corresponds to the encoding.
     * use only "range1"
     */

    const char *encoding = JNU_GetStringPlatformChars (env, theEncoding, NULL);
    if (encoding == NULL) {
      return JNI_FALSE;
    }

    for (int em=0; em<31;em++) {
        if (strcmp(encoding_mapping[em], encoding) == 0) {
	  tt_uint32 mask = 1 << em;
	  if (mask & range1) {
	    JNU_ReleaseStringPlatformChars(env,theEncoding, encoding);
	    return JNI_TRUE;
	  }
	  break;
	}
    }
    JNU_ReleaseStringPlatformChars(env,theEncoding, encoding);

    return JNI_FALSE;
}

} // end of extern "C"
