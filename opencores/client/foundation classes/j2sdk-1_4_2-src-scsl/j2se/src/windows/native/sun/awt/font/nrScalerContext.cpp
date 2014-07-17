/*
 * @(#)nrScalerContext.cpp	1.39 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * @author Xueming Shen
 * @author Charlton Innovations, Inc.
 */

#include <stdio.h>
#include <windows.h>
#include <winnls.h>

#include <java_awt_Font.h>
#include "nrScalerContext.h"
#include "t2kScalerContext.h"
#include "swaps.h"
#include "hsMemory.h"

#if 0
static Boolean bIsNT = false;
static Boolean bIsNT_ja = false;

static Boolean isNT40CJKSP3NOPATCH() {
   Boolean       ret = false;

   // test OS version
   OSVERSIONINFO osvi;
   osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
   if (::GetVersionEx(&osvi) == 0) {
       return false;
   }

   if (osvi.dwPlatformId != VER_PLATFORM_WIN32_NT) {
       return false;
   }
   bIsNT = true;

   if (osvi.dwMajorVersion != 4) {
       return false;
   }
  
   if (stricmp(osvi.szCSDVersion, "Service Pack 3") != 0) {
       return false;
   }

   WORD lang = PRIMARYLANGID(LOWORD(::GetSystemDefaultLCID()));
   if (lang != LANG_JAPANESE && lang != LANG_CHINESE && lang != LANG_KOREAN) {
       return false;
   }
   //workaround for bug#4180881, licensee strongly requests this workaround.
   if (lang == LANG_JAPANESE)
       bIsNT_ja = true;

   HMODULE     hUser32 = ::GetModuleHandle("User32");

   /*may use ImageSize but it needs additional PSAPI.DLL which only exists in SDK
     so go the most portable way, the file size:-(   */
   char   szFileName[MAX_PATH];   
   DWORD  dwFileSize;
   if (::GetModuleFileName(hUser32, szFileName, sizeof(szFileName)) != 0) {
       HANDLE hFile = ::CreateFile(szFileName, GENERIC_READ, FILE_SHARE_READ,
                        NULL, OPEN_EXISTING, 
                        FILE_ATTRIBUTE_READONLY | FILE_FLAG_RANDOM_ACCESS, NULL);
       if (hFile != NULL) {
           dwFileSize = ::GetFileSize(hFile, NULL);
           if (dwFileSize != 0x5c5e0){
               ret = true;
           }
           ::CloseHandle(hFile);
       }
   }
   return ret;
}

static Boolean bIsNT40CJKSP3NOPATCH = isNT40CJKSP3NOPATCH();

//Call W version API on NT.
#define GETGLYPHOUTLINE(hdc, char, format, lpGm, cbBuf, lpBuf, lpMat2)        \
 ((bIsNT)?(::GetGlyphOutlineW(hdc, char, format, lpGm, cbBuf, lpBuf, lpMat2)) \
         :(::GetGlyphOutlineA(hdc, char, format, lpGm, cbBuf, lpBuf, lpMat2)))

class UnicodeMapper : public CharToGlyphMapper {
  public:
    UnicodeMapper(  sfntFileFontObject* f,
                    CharToGlyphMapper& parentMapper,
                    int numGlyphs)
        :   font(f), fParentMapper(parentMapper),
            fUnicodes(NULL), fNumGlyphs(numGlyphs)
    {
        fUnicodes = new Unicode32[fNumGlyphs];
        if (fUnicodes) {
            memset(fUnicodes, 0xffff, sizeof(fUnicodes[0]) * fNumGlyphs);
        }
    }
    virtual ~UnicodeMapper() {
		if (fUnicodes) {
				delete [] fUnicodes;
				fUnicodes = NULL;
		}
    }

    virtual void CharsToGlyphs(
        int count, const Unicode16 unicodes[], UInt32 glyphs[]) const;

    virtual void CharsToGlyphs(
        int count, const Unicode32 unicodes[], UInt32 glyphs[]) const;

    virtual Boolean canDisplay(Unicode ch) const {
        UInt32 glyph;
        CharsToGlyphs(1, &ch, &glyph);
        return glyph != getMissingGlyphCode();
    };
    virtual Unicode32 unicodeFromGlyphID(UInt32 glyphID) const;

  private:
    sfntFileFontObject* font;
    CharToGlyphMapper&  fParentMapper;
    Unicode32           *fUnicodes;
    int                 fNumGlyphs;
};

class Win32nrScalerContext : public t2kScalerContext {
private:
    const fontObject    *fFont;
    Boolean             fDoAntiAlias;
    Boolean             fDoFracEnable;
    double              fMatrix[4];

    UnicodeMapper       *fMapper;
    UInt16              fMissingGlyph;

    HDC     m_hMemDC;
    HFONT   m_hFont;
    HFONT   m_hOrgFont;
    int     m_logPixelsY;
    MAT2    m_mat2;
    Boolean m_Valid;
    Boolean m_bIsNotDBCS;

    int fUnitsPerEm;
    int fFontHeight;

private:
    UInt16  getUserLangID();
    HFONT   CreateHFont(char* name, int style, int height);
    FIXED   FixedFromDouble(double d);
    UInt16  GetMbFromWb(Unicode uChar, BOOL& fDefault, BOOL& fGlyphIndex);
    void    CopyBW2Grey8(const void* srcImage, int srcRowBytes,
                         void* dstImage, int dstRowBytes,
                         int width, int height);
    int     initMatrix(const float matrix[]);
    void    initDC(void);
    Unicode32 glyphUnicode(UInt16 glyphID);
    const Unicode *getName(int& nameLength);

public:
    Win32nrScalerContext(fontObject* fo,
                         const float matrix[],
                         Boolean    doAntiAlias,
                         Boolean    doFractEnable,
                         int        style);
    virtual ~Win32nrScalerContext();

    virtual UInt32  CountGlyphs();

    virtual void    GenerateMetrics(
                        UInt16 glyphID, hsGGlyph* glyph,
                        hsFixedPoint2* advance);
    virtual void    GenerateMetricsWithImage(
                        UInt16 glyphID, hsGGlyph* glyph,
                        hsFixedPoint2* advance);
    virtual void    GenerateImage(
                        UInt16 glyphID,
                        const hsGGlyph* glyph,
                        void* buffer);
    virtual void    GeneratePath(UInt16 index, hsPathSpline* path,
                        hsFixedPoint2* advance);
    virtual void    GetLineHeight(hsFixedPoint2* ascent,
                        hsFixedPoint2* descent,
                        hsFixedPoint2* baseline,
                        hsFixedPoint2* leading,
                        hsFixedPoint2* maxAdvance);
    virtual void GetCaretAngle(hsFixedPoint2* caret);
    virtual CharToGlyphMapper *getMapper();
    virtual Boolean isStyleSupported(int aStyle);
    virtual Boolean GetGlyphPoint(UInt16 glyphID, Int32 pointNumber, hsFixedPoint2 *point);
    virtual void TransformFunits(Int16 xFunits, Int16 yFunits, hsFixedPoint2 *pixels);
};

//////////////////////////////////////////////////////////////////////////////

Win32nrScalerContext::Win32nrScalerContext(
                        fontObject *fo, const float matrix[],
                        Boolean doAntiAlias, Boolean doFractEnable, int style)
    : fFont(fo), fDoAntiAlias(doAntiAlias), fDoFracEnable(doFractEnable),
      m_hMemDC(NULL), m_hFont(NULL), m_hOrgFont(NULL), m_logPixelsY(72),
      fMapper(NULL), m_Valid(false), m_bIsNotDBCS(false),
      t2kScalerContext( fo, matrix, doAntiAlias, doFractEnable, /*style*/ 0)
{
    fFontHeight = initMatrix(matrix);
    fUnitsPerEm = fo->GetUnitsPerEM();
    initDC();

    if (getMapper()) {
        fMissingGlyph = fMapper->getMissingGlyphCode();
    }

    if (bIsNT40CJKSP3NOPATCH
        && (m_mat2.eM11.value != 1 || m_mat2.eM11.fract != 0
            || m_mat2.eM12.value != 0 || m_mat2.eM12.fract != 0
            || m_mat2.eM21.value != 0 || m_mat2.eM21.fract != 0
            || m_mat2.eM22.value != 1 || m_mat2.eM22.fract != 0)) {
#ifdef DEBUG
        fprintf(stderr, "NT4.0 CJK SP3 NOPATCH\n");
#endif
        m_Valid = false;
        return;
    }

    if (fFont->GetFormat () == kType1FontFormat) {
	m_Valid = false;
	return;
    }

    int    nameLength;
    const Unicode *name = getName(nameLength);

    if (name && nameLength){
        int mblen = ::WideCharToMultiByte(CP_ACP, 0, name, nameLength, NULL, 0, NULL, NULL);
	char* chartmp = new char[mblen+1];
	chartmp[mblen] = '\0';
        ::WideCharToMultiByte(CP_ACP, 0, name, nameLength, chartmp, mblen, NULL, NULL);
        m_hFont = CreateHFont(chartmp, style, fFontHeight);

        LOGFONTA tmpLogFont;
        ::GetObject(m_hFont, sizeof(LOGFONTA), &tmpLogFont);
        if (strcmp(&tmpLogFont.lfFaceName[0], chartmp) == 0){
// REMIND: set true count when implemented,
// set to ZERO for unimplemented or unknown request
            m_Valid = true;
	}
	delete [] chartmp;
	m_hOrgFont = (HFONT)::SelectObject(m_hMemDC, m_hFont);
        //The only way to get the charset info is to lookup the OS/2
        //table if the fontObject does not offer it, but currently 
        //take the "easiest" way, assume a DBCS font should at least 
        //has more than 4096 Chinese characters.
        m_bIsNotDBCS = (CountGlyphs() < 0x1000);
    }
    if (name) {
        delete [] (Unicode *)name;
    }
}

int
Win32nrScalerContext::initMatrix(const float matrix[])
{
    float determinant = matrix[0] * matrix[3] - matrix[1] * matrix[2];
    if (determinant != 0) {
        fMatrix[0] = matrix[0];
        fMatrix[1] = matrix[1];
        fMatrix[2] = matrix[2];
        fMatrix[3] = matrix[3];
    } else {
        fMatrix[0] = 1.0;
        fMatrix[1] = 0.0;
        fMatrix[2] = 0.0;
        fMatrix[3] = 1.0;
        determinant = 1.0;
    }
    float size = sqrt(fabs(determinant));

    m_mat2.eM11 = FixedFromDouble(fMatrix[0]/size);
    m_mat2.eM12 = FixedFromDouble(-fMatrix[1]/size);
    m_mat2.eM21 = FixedFromDouble(-fMatrix[2]/size);
    m_mat2.eM22 = FixedFromDouble(fMatrix[3]/size);

    int    fontSize = (int)(size + 0.5);
    return fontSize;
}

void
Win32nrScalerContext::initDC(void)
{
    HWND hWnd = ::GetDesktopWindow();
    HDC  hTmpDC = ::GetWindowDC(hWnd);
    m_hMemDC = ::CreateCompatibleDC(hTmpDC);
    ::SetMapMode(m_hMemDC, MM_TEXT);
    m_logPixelsY = ::GetDeviceCaps(hTmpDC, LOGPIXELSY);
    ::ReleaseDC(hWnd, hTmpDC);
}

const Unicode *
Win32nrScalerContext::getName(int& nameLength)
{
    Unicode *retval = 0;
    UInt16 platformID = 3;
    UInt16 scriptID = 1;
    UInt16 languageID = getUserLangID();
    UInt16 nameID = kFamilyName;
    fontObject *fo = (fontObject *)fFont;

    nameLength = fo->GetName(platformID, scriptID, languageID, nameID, NULL);

    if (nameLength == 0) {
        platformID = 3;
        scriptID = 1;
        languageID = 0;
        nameID = kFamilyName;
        nameLength = fo->GetName(platformID, scriptID, languageID, nameID, NULL);
    }

    if (nameLength != 0) {
        retval = new Unicode[nameLength+1];
        fo->GetName(platformID, scriptID, languageID, nameID, retval);
    }

    return retval;
}

Win32nrScalerContext::~Win32nrScalerContext()
{
    if (fMapper) {
        delete fMapper;
        fMapper = NULL;
    }
    if (m_hMemDC) {
        if (m_hOrgFont != NULL){
            ::SelectObject(m_hMemDC, m_hOrgFont);
	    m_hOrgFont = NULL;
        }
        if (m_hFont != NULL){
	    ::DeleteObject(m_hFont);
	    m_hFont = NULL;
        }
        ::DeleteDC(m_hMemDC);
        m_hMemDC = NULL;
    }
}


UInt32
Win32nrScalerContext::CountGlyphs()
{
    return t2kScalerContext::CountGlyphs();
}

UInt16
Win32nrScalerContext::GetMbFromWb(Unicode uChar, BOOL& fDefault, BOOL& fGlyphIndex)
{
    static UInt16 Symbol_math[] = {
	(UInt16)0042, (UInt16)0000, (UInt16)0144, (UInt16)0044,
	(UInt16)0000, (UInt16)0306, (UInt16)0104, (UInt16)0321,    // 00
	(UInt16)0316, (UInt16)0317, (UInt16)0000, (UInt16)0000,
	(UInt16)0000, (UInt16)0047, (UInt16)0000, (UInt16)0120,
	(UInt16)0000, (UInt16)0345, (UInt16)0055, (UInt16)0000,
	(UInt16)0000, (UInt16)0244, (UInt16)0000, (UInt16)0052,    // 10
	(UInt16)0260, (UInt16)0267, (UInt16)0326, (UInt16)0000,
	(UInt16)0000, (UInt16)0265, (UInt16)0245, (UInt16)0000,
	(UInt16)0000, (UInt16)0000, (UInt16)0000, (UInt16)0275,
	(UInt16)0000, (UInt16)0000, (UInt16)0000, (UInt16)0331,    // 20
	(UInt16)0332, (UInt16)0307, (UInt16)0310, (UInt16)0362,
	(UInt16)0000, (UInt16)0000, (UInt16)0000, (UInt16)0000,
	(UInt16)0000, (UInt16)0000, (UInt16)0000, (UInt16)0000,
	(UInt16)0134, (UInt16)0000, (UInt16)0000, (UInt16)0000,    // 30
	(UInt16)0000, (UInt16)0000, (UInt16)0000, (UInt16)0000,
	(UInt16)0176, (UInt16)0000, (UInt16)0000, (UInt16)0000,
	(UInt16)0000, (UInt16)0000, (UInt16)0000, (UInt16)0000,
	(UInt16)0000, (UInt16)0100, (UInt16)0000, (UInt16)0000,    // 40
	(UInt16)0273, (UInt16)0000, (UInt16)0000, (UInt16)0000,
	(UInt16)0000, (UInt16)0000, (UInt16)0000, (UInt16)0000,
	(UInt16)0000, (UInt16)0000, (UInt16)0000, (UInt16)0000,
	(UInt16)0000, (UInt16)0000, (UInt16)0000, (UInt16)0000,    // 50
	(UInt16)0000, (UInt16)0000, (UInt16)0000, (UInt16)0000,
	(UInt16)0000, (UInt16)0000, (UInt16)0000, (UInt16)0000,
	(UInt16)0271, (UInt16)0272, (UInt16)0000, (UInt16)0000,
	(UInt16)0243, (UInt16)0263, (UInt16)0000, (UInt16)0000,    // 60
	(UInt16)0000, (UInt16)0000, (UInt16)0000, (UInt16)0000,
	(UInt16)0000, (UInt16)0000, (UInt16)0000, (UInt16)0000,
	(UInt16)0000, (UInt16)0000, (UInt16)0000, (UInt16)0000,
	(UInt16)0000, (UInt16)0000, (UInt16)0000, (UInt16)0000,    // 70
	(UInt16)0000, (UInt16)0000, (UInt16)0000, (UInt16)0000,
	(UInt16)0000, (UInt16)0000, (UInt16)0000, (UInt16)0000,
	(UInt16)0000, (UInt16)0000, (UInt16)0314, (UInt16)0311,
	(UInt16)0313, (UInt16)0000, (UInt16)0315, (UInt16)0312,    // 80
	(UInt16)0000, (UInt16)0000, (UInt16)0000, (UInt16)0000,
	(UInt16)0000, (UInt16)0000, (UInt16)0000, (UInt16)0000,
	(UInt16)0000, (UInt16)0000, (UInt16)0000, (UInt16)0000,
	(UInt16)0000, (UInt16)0305, (UInt16)0000, (UInt16)0304,    // 90
	(UInt16)0000, (UInt16)0000, (UInt16)0000, (UInt16)0000,
	(UInt16)0000, (UInt16)0000, (UInt16)0000, (UInt16)0000,
	(UInt16)0000, (UInt16)0000, (UInt16)0000, (UInt16)0000,
	(UInt16)0000, (UInt16)0136, (UInt16)0000, (UInt16)0000,    // a0
	(UInt16)0000, (UInt16)0000, (UInt16)0000, (UInt16)0000,
	(UInt16)0000, (UInt16)0000, (UInt16)0000, (UInt16)0000,
	(UInt16)0000, (UInt16)0000, (UInt16)0000, (UInt16)0000,
	(UInt16)0000, (UInt16)0000, (UInt16)0000, (UInt16)0000,    // b0
	(UInt16)0000, (UInt16)0000, (UInt16)0000, (UInt16)0000,
	(UInt16)0000, (UInt16)0000, (UInt16)0000, (UInt16)0000,
	(UInt16)0000, (UInt16)0000, (UInt16)0000, (UInt16)0000,
	(UInt16)0340, (UInt16)0327, (UInt16)0000, (UInt16)0000,    // c0
	(UInt16)0000, (UInt16)0000, (UInt16)0000, (UInt16)0000,
	(UInt16)0000, (UInt16)0000, (UInt16)0000, (UInt16)0000,
	(UInt16)0000, (UInt16)0000, (UInt16)0000, (UInt16)0000,
	(UInt16)0000, (UInt16)0000, (UInt16)0000, (UInt16)0000,    // d0
	(UInt16)0000, (UInt16)0000, (UInt16)0000, (UInt16)0000,
	(UInt16)0000, (UInt16)0000, (UInt16)0000, (UInt16)0000,
	(UInt16)0000, (UInt16)0000, (UInt16)0000, (UInt16)0000,
	(UInt16)0000, (UInt16)0000, (UInt16)0000, (UInt16)0000,    // e0
	(UInt16)0000, (UInt16)0000, (UInt16)0000, (UInt16)0000,
	(UInt16)0000, (UInt16)0000, (UInt16)0000, (UInt16)0274,
    };

    static UInt16 WingDings[] = {
	(UInt16)0x00, (UInt16)0x23, (UInt16)0x22, (UInt16)0x00,  // 0x2700
	(UInt16)0x00, (UInt16)0x00, (UInt16)0x29, (UInt16)0x3e,  // 0x2704
	(UInt16)0x51, (UInt16)0x2a, (UInt16)0x00, (UInt16)0x00,  // 0x2708
	(UInt16)0x41, (UInt16)0x3f, (UInt16)0x00, (UInt16)0x00,  // 0x270c

	(UInt16)0x00, (UInt16)0x00, (UInt16)0x00, (UInt16)0xfc,  // 0x2710
	(UInt16)0x00, (UInt16)0x00, (UInt16)0x00, (UInt16)0xfb,  // 0x2714
	(UInt16)0x00, (UInt16)0x00, (UInt16)0x00, (UInt16)0x00,  // 0x2718
	(UInt16)0x00, (UInt16)0x00, (UInt16)0x56, (UInt16)0x00,  // 0x271c

	(UInt16)0x58, (UInt16)0x59, (UInt16)0x00, (UInt16)0x00,  // 0x2720
	(UInt16)0x00, (UInt16)0x00, (UInt16)0x00, (UInt16)0x00,  // 0x2724
	(UInt16)0x00, (UInt16)0x00, (UInt16)0xb5, (UInt16)0x00,  // 0x2728
	(UInt16)0x00, (UInt16)0x00, (UInt16)0x00, (UInt16)0x00,  // 0x272c

	(UInt16)0xb6, (UInt16)0x00, (UInt16)0x00, (UInt16)0x00,  // 0x2730
	(UInt16)0xad, (UInt16)0xaf, (UInt16)0xac, (UInt16)0x00,  // 0x2734
	(UInt16)0x00, (UInt16)0x00, (UInt16)0x00, (UInt16)0x00,  // 0x2738
	(UInt16)0x00, (UInt16)0x00, (UInt16)0x00, (UInt16)0x7c,  // 0x273c

	(UInt16)0x7b, (UInt16)0x00, (UInt16)0x00, (UInt16)0x00,  // 0x2740
	(UInt16)0x54, (UInt16)0x00, (UInt16)0x00, (UInt16)0x00,  // 0x2744
	(UInt16)0x00, (UInt16)0x00, (UInt16)0x00, (UInt16)0x00,  // 0x2748
	(UInt16)0x00, (UInt16)0xa6, (UInt16)0x00, (UInt16)0x00,  // 0x274c

	(UInt16)0x00, (UInt16)0x71, (UInt16)0x72, (UInt16)0x00,  // 0x2750
	(UInt16)0x00, (UInt16)0x00, (UInt16)0x75, (UInt16)0x00,  // 0x2754
	(UInt16)0x00, (UInt16)0x00, (UInt16)0x00, (UInt16)0x00,  // 0x2758
	(UInt16)0x00, (UInt16)0x7d, (UInt16)0x7e, (UInt16)0x00,  // 0x275c

	(UInt16)0x00, (UInt16)0x00, (UInt16)0x00, (UInt16)0x00,  // 0x2760
	(UInt16)0x00, (UInt16)0x00, (UInt16)0x00, (UInt16)0x00,  // 0x2764
	(UInt16)0x00, (UInt16)0x00, (UInt16)0x00, (UInt16)0x00,  // 0x2768
	(UInt16)0x00, (UInt16)0x00, (UInt16)0x00, (UInt16)0x00,  // 0x276c

	(UInt16)0x00, (UInt16)0x00, (UInt16)0x00, (UInt16)0x00,  // 0x2770
	(UInt16)0x00, (UInt16)0x00, (UInt16)0x8c, (UInt16)0x8d,  // 0x2774
	(UInt16)0x8e, (UInt16)0x8f, (UInt16)0x90, (UInt16)0x91,  // 0x2778
	(UInt16)0x92, (UInt16)0x93, (UInt16)0x94, (UInt16)0x95,  // 0x277c

	(UInt16)0x81, (UInt16)0x82, (UInt16)0x83, (UInt16)0x84,  // 0x2780
	(UInt16)0x85, (UInt16)0x86, (UInt16)0x87, (UInt16)0x88,  // 0x2784
	(UInt16)0x89, (UInt16)0x8a, (UInt16)0x8c, (UInt16)0x8d,  // 0x2788
	(UInt16)0x8e, (UInt16)0x8f, (UInt16)0x90, (UInt16)0x91,  // 0x278c

	(UInt16)0x92, (UInt16)0x93, (UInt16)0x94, (UInt16)0x95,  // 0x2790
	(UInt16)0xe8, (UInt16)0x00, (UInt16)0x00, (UInt16)0x00,  // 0x2794
	(UInt16)0x00, (UInt16)0x00, (UInt16)0x00, (UInt16)0x00,  // 0x2798
	(UInt16)0x00, (UInt16)0x00, (UInt16)0x00, (UInt16)0x00,  // 0x279c

	(UInt16)0x00, (UInt16)0xe8, (UInt16)0xd8, (UInt16)0x00,  // 0x27a0
	(UInt16)0x00, (UInt16)0xc4, (UInt16)0xc6, (UInt16)0x00,  // 0x27a4
	(UInt16)0x00, (UInt16)0xf0, (UInt16)0x00, (UInt16)0x00,  // 0x27a8
	(UInt16)0x00, (UInt16)0x00, (UInt16)0x00, (UInt16)0x00,  // 0x27ac

	(UInt16)0x00, (UInt16)0x00, (UInt16)0x00, (UInt16)0xdc,  // 0x27b0
	(UInt16)0x00, (UInt16)0x00, (UInt16)0x00, (UInt16)0x00,  // 0x27b4
	(UInt16)0x00, (UInt16)0x00, (UInt16)0x00, (UInt16)0x00,  // 0x27b8
	(UInt16)0x00, (UInt16)0x00, (UInt16)0x00                 // 0x27bc
    };

    if (m_bIsNotDBCS){
        if (uChar >= 0x2701 && uChar <= 0x27be){
            //On NT, some legal wingdings characters are not in LATIN-1 
            //code set but in WindowCharacter code set, "add 0xf000" does
            //help to display them correctly
            if (bIsNT) {
                return WingDings[uChar - 0x2700] + 0xf000;
            } else {
                return WingDings[uChar - 0x2700];
            }
        }
        if (uChar >= 0x2200 && uChar <= 0x22ef){
            return Symbol_math[uChar - 0x2200];
        }
	// handle arabic and hebrew specially to get proper glyph shapes
        // on arabic and hebrew win95
        // Also: win9x wide-to-multibyte only seems to work if the characters
        // are supported by the current code page, so we will use glyph index
        // instead for characters > 256 and in a range we know to work.
	if (!bIsNT && 
	    ((uChar >= 0x0590 && uChar < 0x0700)
             || (uChar >= 0x0900 && uChar <= 0x097F)
	     || (uChar >= 0xfe70 && uChar < 0xfefd))) {
	  fGlyphIndex = true;
	}

        if (!bIsNT  && uChar == 0x20ac){
            return 0x80;
        }

	//for no-DBCS fonts, calling
        //WbToMb causes problem for some 0x80--0xff characters which do have
        //entries in Unicode=>DBCS(SJIS) mapping.

	return uChar;
    }
    //For NT, we don't need to do WbToMb
    if (bIsNT) {
        return uChar;
    }

    int mblen = ::WideCharToMultiByte(
                            CP_ACP, 0, &uChar, 1, NULL, 0, NULL, NULL);

    if (mblen == 1){
        char c;
        ::WideCharToMultiByte(CP_ACP, 0, &uChar, 1, &c, 1, NULL, &fDefault);
        if (!fDefault) {
            uChar = c & 0xff;
        }
    } else {
        char cs[2];
        ::WideCharToMultiByte(CP_ACP, 0, &uChar, 1, cs, 2, NULL, &fDefault);
        if (!fDefault) {
            uChar = (cs[0] << 8) | cs[1] & 0xff;
        }
    }
    return uChar;
}

FIXED
Win32nrScalerContext::FixedFromDouble(double d)
{
    long l;
    l = (long) (d * 65536L);
    return *(FIXED *)&l;
}

void
Win32nrScalerContext::CopyBW2Grey8(
                        const void* srcImage, int srcRowBytes,
                        void* dstImage, int dstRowBytes,
                        int width, int height)
{
    const UInt8* srcRow = (UInt8*)srcImage;
    UInt8* dstRow = (UInt8*)dstImage;
    int wholeByteCount = width >> 3;
    int remainingBitsCount = width & 7;
    int i, j;

    while (height--) {
        const UInt8* src8 = srcRow;
        UInt8* dstByte = dstRow;
        unsigned srcValue;

        srcRow += srcRowBytes;
        dstRow += dstRowBytes;

        for (i = 0; i < wholeByteCount; i++) {
            srcValue = *src8++;
            for (j = 0; j < 8; j++) {
                *dstByte++ = (srcValue & 0x80) ? 0xFF : 0;
                srcValue <<= 1;
            }
        }
        if (remainingBitsCount) {
            srcValue = *src8;
            for (j = 0; j < remainingBitsCount; j++) {
                *dstByte++ = (srcValue & 0x80) ? 0xFF : 0;
                srcValue <<= 1;
            }
        }
    }
}

void
Win32nrScalerContext::GenerateMetrics(
                UInt16 glyphID,
                hsGGlyph* glyph,
                hsFixedPoint2* advance)
{
    if (!m_Valid || (fMissingGlyph == glyphID)) {
        t2kScalerContext::GenerateMetrics(glyphID, glyph, advance);
        return;
    }
    Unicode uChar = glyphUnicode(glyphID);
    /*
    if (uChar == 0xffff) {
        t2kScalerContext::GenerateMetrics(glyphID, glyph, advance);
        return;
    }
    */
    Boolean retval = false;

    GLYPHMETRICS    gm;
    BOOL            fDefault = false;
    BOOL            fGlyphIndex = false;
    UINT            type = (true || bIsNT_ja) ? GGO_BITMAP: GGO_METRICS;

    uChar = GetMbFromWb(uChar, fDefault, fGlyphIndex);

    if (fDefault) {
        t2kScalerContext::GenerateMetrics(glyphID, glyph, advance);
        return;
    }

    if (uChar == 0xFFFF || fGlyphIndex) {
      type |= GGO_GLYPH_INDEX;
      uChar = glyphID;
    }

    int imageSize = GETGLYPHOUTLINE(m_hMemDC, uChar, type, &gm, 0, NULL, &m_mat2);

    if (imageSize == GDI_ERROR || gm.gmBlackBoxX == 0 || gm.gmBlackBoxY == 0) {
            t2kScalerContext::GenerateMetrics(glyphID, glyph, advance);
            return;
    }

    glyph->fWidth       = (UInt16)(gm.gmBlackBoxX);
    glyph->fHeight      = (UInt16)(gm.gmBlackBoxY);
    glyph->fTopLeft.fX  = hsFloatToFixed(gm.gmptGlyphOrigin.x);
    glyph->fTopLeft.fY  = hsFloatToFixed(-gm.gmptGlyphOrigin.y);
    glyph->fRowBytes    = glyph->fWidth;

    advance->fX = hsIntToFixed(gm.gmCellIncX);
    if (bIsNT) {
        advance->fY = hsIntToFixed(gm.gmCellIncY);
    } else {
        advance->fY = hsIntToFixed(-gm.gmCellIncY);
    }

    //workaround for bug#4180881, licensee strongly requests this workaround.
    if((true || bIsNT_ja) 
       && imageSize != ((((gm.gmBlackBoxX + 31) >> 5) << 2) * gm.gmBlackBoxY)){
      // safeguard against a divide by zero - bug # 4292227
      if ( gm.gmBlackBoxY == 0 ) {
	  glyph->fWidth = 0;
      }
      else {
          glyph->fWidth = glyph->fRowBytes 
                      = ((imageSize / gm.gmBlackBoxY) & ~3) * 8;
      }
    }

    if (uChar == 0x0020) {
      glyph->fWidth = glyph->fHeight = glyph->fRowBytes = 0;
    }
}

#if 0
void
Win32nrScalerContext::GenerateMetricsWithImage(
                UInt16 glyphID,
                hsGGlyph* glyph,
                hsFixedPoint2* advance)
{
    if (!m_Valid || (fMissingGlyph == glyphID)) {
        t2kScalerContext::GenerateMetricsWithImage(glyphID, glyph, advance);
        return;
    }
    Unicode uChar = glyphUnicode(glyphID);

    BOOL            fDefault = false;
    BOOL            fGlyphIndex = false;

    uChar = GetMbFromWb(uChar, fDefault, fGlyphIndex);

    if (fDefault) {
        t2kScalerContext::GenerateMetricsWithImage(glyphID, glyph, advance);
        return;
    }

    UINT type = GGO_BITMAP;
    if (uChar == 0xFFFF || fGlyphIndex) {
      type |= GGO_GLYPH_INDEX;
      uChar = glyphID;
    }

    GLYPHMETRICS gm;
    int imageSizeWin32 = GETGLYPHOUTLINE(m_hMemDC, uChar, type, &gm, 0, NULL, &m_mat2);

    if (imageSizeWin32 == GDI_ERROR || gm.gmBlackBoxX == 0 || gm.gmBlackBoxY == 0) {
      t2kScalerContext::GenerateMetricsWithImage(glyphID, glyph, advance);
            return;
    }

    advance->fX = hsIntToFixed(gm.gmCellIncX);
    if (bIsNT) {
        advance->fY = hsIntToFixed(gm.gmCellIncY);
    } else {
        advance->fY = hsIntToFixed(-gm.gmCellIncY);
    }

    glyph->fWidth       = (UInt16)(gm.gmBlackBoxX);
    glyph->fHeight      = (UInt16)(gm.gmBlackBoxY);
    glyph->fTopLeft.fX  = hsFloatToFixed(gm.gmptGlyphOrigin.x);
    glyph->fTopLeft.fY  = hsFloatToFixed(-gm.gmptGlyphOrigin.y);
    glyph->fRowBytes    = glyph->fWidth;

    //workaround for bug#4180881, licensee strongly requests this workaround.
    if (imageSizeWin32 != ((((gm.gmBlackBoxX + 31) >> 5) << 2) * gm.gmBlackBoxY)) {
      // safeguard against a divide by zero - bug # 4292227
      if ( gm.gmBlackBoxY == 0 ) {
	  glyph->fWidth = 0;
      }
      else {
          glyph->fWidth = glyph->fRowBytes 
          = ((imageSizeWin32 / gm.gmBlackBoxY) & ~3) * 8;
      }
    }

    if (uChar == 0x0020) {
      glyph->fWidth = glyph->fHeight = glyph->fRowBytes = 0;
    }

    UInt32 imageSize = glyph->fHeight * glyph->fRowBytes;
    glyph->fImage = HSMemory::SoftNew(imageSize);

    if (imageSizeWin32 == 0 || glyph->fHeight == 0) {
        memset((void*)glyph->fImage, 0, glyph->fHeight * glyph->fRowBytes);
    } else {
        void* image = HSMemory::SoftNew(imageSizeWin32);
        if (GETGLYPHOUTLINE(m_hMemDC, uChar, type, &gm, imageSizeWin32, image, &m_mat2) != imageSizeWin32){
           printf("pf_Generate ImageError2=%d\n", GetLastError());
        } else {
          CopyBW2Grey8(image, imageSizeWin32 / gm.gmBlackBoxY,
                       (void*)glyph->fImage, glyph->fRowBytes,
		       glyph->fWidth, glyph->fHeight);
	  }
	HSMemory::Delete(image);
    }
}
#else
void
Win32nrScalerContext::GenerateMetricsWithImage(
                UInt16 glyphID,
                hsGGlyph* glyph,
                hsFixedPoint2* advance)
{
    GenerateMetrics(glyphID, glyph, advance);
    UInt32 imageSize = glyph->fHeight * glyph->fRowBytes;
    glyph->fImage = HSMemory::SoftNew(imageSize);
    GenerateImage(glyphID, glyph, (void *)glyph->fImage);
}
#endif

void
Win32nrScalerContext::GenerateImage(
                    UInt16 glyphID,
                    const hsGGlyph* glyph,
                    void* buffer)
{
    if (!m_Valid || (fMissingGlyph == glyphID)) {
        t2kScalerContext::GenerateImage(glyphID, glyph, buffer);
        return;
    }
    Unicode uChar = glyphUnicode(glyphID);
    /*
    if (uChar == 0xffff) {
        t2kScalerContext::GenerateImage(glyphID, glyph, buffer);
        return;
    }
    */

    GLYPHMETRICS    gm;
    BOOL            fDefault = false;
    BOOL            fGlyphIndex = false;

    uChar = GetMbFromWb(uChar, fDefault, fGlyphIndex);

    if (fDefault) {
        t2kScalerContext::GenerateImage(glyphID, glyph, buffer);
        return;
    }

    void *image;
    int   imageSize;
    UINT  grayLevel = GGO_BITMAP;

    if (uChar == 0xFFFF || fGlyphIndex) {
      grayLevel |= GGO_GLYPH_INDEX;
      uChar = glyphID;
    }

    imageSize = GETGLYPHOUTLINE(m_hMemDC, uChar, grayLevel, &gm, 0, NULL, &m_mat2);

    if (imageSize == -1 || gm.gmBlackBoxX == 0 || gm.gmBlackBoxY == 0) {
        t2kScalerContext::GenerateImage(glyphID, glyph, buffer);
//        printf("pf_GenerateMetrics Error1=%d\n", GetLastError());
    } else if (imageSize == 0 || ( glyph->fHeight == 0 )) {
        memset(buffer, 0, glyph->fHeight * glyph->fRowBytes);
        return;
    } else {
        image = HSMemory::SoftNew(imageSize);
        if (GETGLYPHOUTLINE(m_hMemDC, uChar, grayLevel, &gm, imageSize, image, &m_mat2)
           != imageSize){
//           printf("pf_Generate ImageError2=%d\n", GetLastError());
        } else {
//	  CopyBW2Grey8(image, ((glyph->fRowBytes + 31) >> 5) << 2,
	    int rb = imageSize / glyph->fHeight;
	    CopyBW2Grey8(image, rb,
		       buffer, glyph->fRowBytes,
		       glyph->fWidth, glyph->fHeight);
	  }
	HSMemory::Delete(image);
    }
    return;
}

void
Win32nrScalerContext::GeneratePath(
                    UInt16 glyphID,
                    hsPathSpline* path,
                    hsFixedPoint2* advance)
{
    t2kScalerContext::GeneratePath(glyphID, path, advance);
}

Unicode32
Win32nrScalerContext::glyphUnicode(UInt16 glyphID)
{
    Unicode32 retval = 0;
    if (getMapper()) {
        retval = fMapper->unicodeFromGlyphID(glyphID);
    }
    return retval;
}

void
Win32nrScalerContext::GetLineHeight(
                    hsFixedPoint2* ascent,
                    hsFixedPoint2* descent,
                    hsFixedPoint2* baseline,
                    hsFixedPoint2* leading,
                    hsFixedPoint2* maxAdvance)
{
    t2kScalerContext::GetLineHeight(ascent, descent, baseline, leading, maxAdvance);
}

void
Win32nrScalerContext::GetCaretAngle(hsFixedPoint2* caret)
{
    if (!m_Valid) {
        t2kScalerContext::GetCaretAngle(caret);
        return;
    }
    OUTLINETEXTMETRIC otm;
    if (::GetOutlineTextMetrics(m_hMemDC, sizeof(otm), &otm)) {
        caret->fX = otm.otmsCharSlopeRun;
        caret->fY = otm.otmsCharSlopeRise;
    } else {
        caret->fX = 0;
        caret->fY = 1;
    }
}


//////////////////////////////////////////////////////////////////////////////
UInt16
Win32nrScalerContext::getUserLangID()
{
    return ::GetUserDefaultLCID();
}

HFONT
Win32nrScalerContext::CreateHFont(char* name, int style, int height)
{
    LOGFONTA logFont;
    // printf("Win32PfScalarContext  createHFont=%s %d\n", name, style);
    logFont.lfWidth = 0;
    logFont.lfEscapement = 0;
    logFont.lfOrientation = 0;
    logFont.lfUnderline = FALSE;
    logFont.lfStrikeOut = FALSE;
    //There is a known bug/problem  in DBCS win32(?) that sometime you 
    //must set the lfCharSet to "SYMBOL_CHARSET" to display some wingdings
    //characters correctly
    if (strstr(name, "Wingdings") || strstr(name, "Symbol")){
        logFont.lfCharSet = SYMBOL_CHARSET;
    } else {
        logFont.lfCharSet = DEFAULT_CHARSET;
    }
    //win95_ja gives me corrupt matrix/bitmap for some characters
    //in some point sizes, like 16, 24, 32... by specifying to
    //use TT font only, everyting works fine.
    if (height < 16) {
        logFont.lfOutPrecision = OUT_DEFAULT_PRECIS;
    } else {
        logFont.lfOutPrecision = OUT_TT_ONLY_PRECIS;
    }
    logFont.lfClipPrecision = CLIP_DEFAULT_PRECIS;
    logFont.lfQuality = DEFAULT_QUALITY;
    logFont.lfPitchAndFamily = DEFAULT_PITCH;

    // Set style
    logFont.lfWeight = (style & java_awt_Font_BOLD)? FW_BOLD: FW_NORMAL;
    logFont.lfItalic =
                ((style & java_awt_Font_ITALIC) == java_awt_Font_ITALIC);
    logFont.lfUnderline = 0;
    // Get point size
    logFont.lfHeight = -height;
    // Set font name
    strncpy(&logFont.lfFaceName[0], name, LF_FACESIZE-1);
    logFont.lfFaceName[LF_FACESIZE-1] = '\0';

    HFONT hFont = ::CreateFontIndirectA(&logFont);
    return hFont;
}

CharToGlyphMapper*
Win32nrScalerContext::getMapper()
{
    if (!fMapper) {
        CharToGlyphMapper *parentMapper = t2kScalerContext::getMapper();
        if (parentMapper) {
            fMapper = new UnicodeMapper(
                            (sfntFileFontObject *)fFont,
                            *parentMapper,
                            CountGlyphs());
        }
    }
    return fMapper;
}

Boolean
Win32nrScalerContext::isStyleSupported(int aStyle)
{
    if (aStyle & ~(java_awt_Font_BOLD | java_awt_Font_ITALIC)) {
        return false;
    }
    return true;
}

Boolean
Win32nrScalerContext::GetGlyphPoint(UInt16 glyphID, Int32 pointNumber, hsFixedPoint2 *point)
{
    return t2kScalerContext::GetGlyphPoint(glyphID, pointNumber, point);
}

void
Win32nrScalerContext::TransformFunits(Int16 xFunits, Int16 yFunits, hsFixedPoint2 *pixels)
{
    //t2kScalerContext::TransformFunits(xFunits, yFunits, pixels);
    pixels->fX = (xFunits * fFontHeight / fUnitsPerEm) << 16;
    pixels->fY = (yFunits * fFontHeight / fUnitsPerEm) << 16;
}

//////////////////////////////////////////////////////////////////////////////

hsGScalerContext*
nrFontScaler::CreateContext(hsConstDescriptor desc)
{
    const FontStrikeDesc *r = (FontStrikeDesc *)hsDescriptor_Find(
                                                        desc,
                                                        kFontStrikeDescTag,
                                                        nil,
                                                        nil);
#if 1
    return new Win32nrScalerContext(
                    r->fFontObj, r->fMatrix,
                    r->fDoAntiAlias, r->fDoFracEnable,
                    r->fStyle);
#else
    return new t2kScalerContext(
                    r->fFontObj, r->fMatrix,
                    r->fDoAntiAlias, r->fDoFracEnable,
                    r->fStyle);
#endif
}

void
UnicodeMapper::CharsToGlyphs(
        int count, const Unicode16 unicodes[], UInt32 glyphs[]) const
{
    fParentMapper.CharsToGlyphs(count, unicodes, glyphs);
    if (fUnicodes) {
        for (int i=0; i < count; i++) {
            if (glyphs[i] < fNumGlyphs) {
                fUnicodes[glyphs[i]] = unicodes[i];
            }
        }
    }
}


void
UnicodeMapper::CharsToGlyphs(
        int count, const Unicode32 unicodes[], UInt32 glyphs[]) const
{
    fParentMapper.CharsToGlyphs(count, unicodes, glyphs);
    if (fUnicodes) {
        for (int i=0; i < count; i++) {
            if (glyphs[i] < fNumGlyphs) {
                fUnicodes[glyphs[i]] = unicodes[i];
            }
        }
    }
}
Unicode32
UnicodeMapper::unicodeFromGlyphID(UInt32 glyphID) const
{
    Unicode retval = 0;
    if (fUnicodes) {
        retval = fUnicodes[glyphID];
    }
    return retval;
}

#else

hsGScalerContext*
nrFontScaler::CreateContext(hsConstDescriptor desc)
{
    const FontStrikeDesc *r = (FontStrikeDesc *)hsDescriptor_Find(
                                                        desc,
                                                        kFontStrikeDescTag,
                                                        nil,
                                                        nil);
    return new t2kScalerContext(
                    r->fFontObj, r->fMatrix,
                    r->fDoAntiAlias, r->fDoFracEnable,
                    r->fStyle);
}

void MakeNativeMapper (const char *nativeName) { };
fileFontObject *GetNativeFontObject () {return NULL; }

#endif

