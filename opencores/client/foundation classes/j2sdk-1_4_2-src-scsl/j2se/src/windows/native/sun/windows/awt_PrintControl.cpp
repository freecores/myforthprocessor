/*
 * @(#)awt_PrintControl.cpp	1.18 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "awt_Component.h"
#include "awt_PrintControl.h"
#include "awt.h"
#include "awt_PrintDialog.h"
#include "awt_Font.h"
#include <sun_awt_print_PrinterCapabilities.h>
#include <sun_awt_print_AwtPrintControl.h>
#include <commdlg.h>
#include <winspool.h>
#include <limits.h>

#define ROUNDTOINT(x) ((int)((x)+0.5))
static const int DEFAULT_RES = 72;
static const double TENTHS_MM_TO_POINTS = 3.527777778;

/* sun.awt.print.AwtPrintControl */

jfieldID  AwtPrintControl::dialogOwnerPeerID;

jmethodID AwtPrintControl::findWin32MediaID;
jmethodID AwtPrintControl::getWin32MediaID;
jmethodID AwtPrintControl::setWin32MediaID;


/* java.awt.print.JobAttributes */

jobject AwtPrintControl::ALL;
jobject AwtPrintControl::RANGE;
jobject AwtPrintControl::SELECTION;
jobject AwtPrintControl::FILE;
jobject AwtPrintControl::PRINTER;
jobject AwtPrintControl::COMMON;
jobject AwtPrintControl::NATIVE;
jobject AwtPrintControl::NONE;
jobject AwtPrintControl::SEPARATE_DOCUMENTS_COLLATED_COPIES;
jobject AwtPrintControl::SEPARATE_DOCUMENTS_UNCOLLATED_COPIES;
jobject AwtPrintControl::ONE_SIDED;
jobject AwtPrintControl::TWO_SIDED_LONG_EDGE;
jobject AwtPrintControl::TWO_SIDED_SHORT_EDGE;


/* java.awt.print.PageAttributes */

jobject AwtPrintControl::COLOR;
jobject AwtPrintControl::MONOCHROME;
jobject AwtPrintControl::PORTRAIT;
jobject AwtPrintControl::LANDSCAPE;
jobject AwtPrintControl::PHYSICAL;
jobject AwtPrintControl::PRINTABLE;
jobject AwtPrintControl::HIGH;
jobject AwtPrintControl::NORMAL;
jobject AwtPrintControl::DRAFT;


/* sun.awt.print.PrintControl */

jfieldID  AwtPrintControl::doctitleID;

jmethodID AwtPrintControl::getColorID;
jmethodID AwtPrintControl::setColorID;
jmethodID AwtPrintControl::getCopiesID;
jmethodID AwtPrintControl::setCopiesID;
jmethodID AwtPrintControl::getSelectID;
jmethodID AwtPrintControl::setSelectID;
jmethodID AwtPrintControl::getDestID;
jmethodID AwtPrintControl::setDestID;
jmethodID AwtPrintControl::getDialogID;
jmethodID AwtPrintControl::getFileNameID;
//jmethodID AwtPrintControl::setFileNameID;
jmethodID AwtPrintControl::getFromPageID;
jmethodID AwtPrintControl::setFromPageID;
jmethodID AwtPrintControl::getMaxPageID;
jmethodID AwtPrintControl::getMinPageID;
jmethodID AwtPrintControl::getMDHID;
jmethodID AwtPrintControl::setMDHID;
jmethodID AwtPrintControl::getOrientID;
jmethodID AwtPrintControl::setOrientID;
jmethodID AwtPrintControl::getOriginID;
jmethodID AwtPrintControl::getQualityID;
jmethodID AwtPrintControl::setQualityID;
jmethodID AwtPrintControl::getPrintToFileEnabledID;
jmethodID AwtPrintControl::getPrinterID;
jmethodID AwtPrintControl::setPrinterID;
jmethodID AwtPrintControl::getResID;
jmethodID AwtPrintControl::getSidesID;
jmethodID AwtPrintControl::setSidesID;
jmethodID AwtPrintControl::getToPageID;
jmethodID AwtPrintControl::setToPageID;


/* sun.awt.print.PrintCapabilities */

jfieldID AwtPrintControl::printerNameID;
jfieldID AwtPrintControl::capabilitiesID;
jfieldID AwtPrintControl::sizesID;


BOOL AwtPrintControl::IsSupportedLevel(HANDLE hPrinter, DWORD dwLevel) {
    BOOL isSupported = FALSE;
    DWORD cbBuf = 0;
    LPBYTE pPrinter = NULL;

    DASSERT(hPrinter != NULL);

    VERIFY(::GetPrinter(hPrinter, dwLevel, NULL, 0, &cbBuf) == 0);
    if (::GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
        pPrinter = new BYTE[cbBuf];
        if (::GetPrinter(hPrinter, dwLevel, pPrinter, cbBuf, &cbBuf)) {
            isSupported = TRUE;
        }
        delete[] pPrinter;
    }

    return isSupported;
}

BOOL AwtPrintControl::FindPrinter(jstring printerName, LPBYTE pPrinterEnum,
				  LPDWORD pcbBuf, LPTSTR * foundPrinter,
				  LPTSTR * foundPort)
{
    JNIEnv *env = (JNIEnv *)JNU_GetEnv(jvm, JNI_VERSION_1_2);

    BOOL nt = IS_NT;
    DWORD cReturned = 0;

    if (pPrinterEnum == NULL) {
        // Compute size of buffer
        DWORD cbNeeded = 0;
        if (nt) {
	    ::EnumPrinters(PRINTER_ENUM_LOCAL | PRINTER_ENUM_CONNECTIONS,
			   NULL, 2, NULL, 0, &cbNeeded, &cReturned);
	}
	::EnumPrinters(PRINTER_ENUM_LOCAL,
		       NULL, 5, NULL, 0, pcbBuf, &cReturned);
	if (cbNeeded > (*pcbBuf)) {
	    *pcbBuf = cbNeeded;
	}
	return TRUE;
    }

    DASSERT(printerName != NULL);

    DWORD cbBuf = *pcbBuf, dummyWord = 0;

    JavaStringBuffer printerNameBuf(env, printerName);
    char *lpcPrinterName = (char *)printerNameBuf;
    DASSERT(lpcPrinterName != NULL);

    // For NT, first do a quick check of all remote and local printers.
    // This only allows us to search by name, though. PRINTER_INFO_4
    // doesn't support port searches. So, if the user has specified the
    // printer name as "LPT1:" (even though this is actually a port
    // name), we won't find the printer here.
    if (nt) {
        if (!::EnumPrinters(PRINTER_ENUM_LOCAL | PRINTER_ENUM_CONNECTIONS,
			    NULL, 4, pPrinterEnum, cbBuf, &dummyWord, &cReturned)) {
	    return FALSE;
	}

	for (DWORD i = 0; i < cReturned; i++) {
	    PRINTER_INFO_4 *info4 = (PRINTER_INFO_4 *)
	        (pPrinterEnum + i * sizeof(PRINTER_INFO_4));
	    if (info4->pPrinterName != NULL &&
		_stricmp(lpcPrinterName, info4->pPrinterName) == 0) {

                // Fix for BugTraq Id 4281380.
                // Get the port name since some drivers may require
                // this name to be passed to ::DeviceCapabilities().
                HANDLE hPrinter = NULL;
                if (::OpenPrinter(info4->pPrinterName, &hPrinter, NULL)) {
                    // Fix for BugTraq Id 4286812.
                    // Some drivers don't support PRINTER_INFO_5.
                    // In this case we try PRINTER_INFO_2, and if that 
                    // isn't supported as well return NULL port name.
                    try {
                        if (AwtPrintControl::IsSupportedLevel(hPrinter, 5)) {
                            VERIFY(::GetPrinter(hPrinter, 5, pPrinterEnum, cbBuf,
                                                &dummyWord));
                            PRINTER_INFO_5 *info5 = (PRINTER_INFO_5 *)pPrinterEnum;
                            *foundPrinter = info5->pPrinterName;
                            // pPortName may specify multiple ports. We only want one.
			    *foundPort = (info5->pPortName != NULL)
			        ? strtok(info5->pPortName, ",") : NULL;
                        } else if (AwtPrintControl::IsSupportedLevel(hPrinter, 2)) {
                            VERIFY(::GetPrinter(hPrinter, 2, pPrinterEnum, cbBuf,
                                                &dummyWord));
                            PRINTER_INFO_2 *info2 = (PRINTER_INFO_2 *)pPrinterEnum;
                            *foundPrinter = info2->pPrinterName;
                            // pPortName may specify multiple ports. We only want one.
			    *foundPort = (info2->pPortName != NULL)
			        ? strtok(info2->pPortName, ",") : NULL;
                        } else {
                            *foundPrinter = info4->pPrinterName;
                            // We failed to determine port name for the found printer.
                            *foundPort = NULL;
                        }
                    } catch (std::bad_alloc&) {
                        VERIFY(::ClosePrinter(hPrinter));
                        throw;
                    }

                    VERIFY(::ClosePrinter(hPrinter));

		    return TRUE;
		}

                return FALSE;
	    }
	}
    }

    // We still haven't found the printer, or we're using 95/98.
    // PRINTER_INFO_5 supports both printer name and port name, so
    // we'll test both. On NT, PRINTER_ENUM_LOCAL means just local
    // printers. This is what we want, because we already tested all
    // remote printer names above (and remote printer port names are
    // the same as remote printer names). On 95/98, PRINTER_ENUM_LOCAL
    // means both remote and local printers. This is also what we want
    // because we haven't tested any printers yet.
    if (!::EnumPrinters(PRINTER_ENUM_LOCAL,
			NULL, 5, pPrinterEnum, cbBuf, &dummyWord, &cReturned)) {
        return FALSE;
    }

    for (DWORD i = 0; i < cReturned; i++) {
        PRINTER_INFO_5 *info5 = (PRINTER_INFO_5 *)
	    (pPrinterEnum + i * sizeof(PRINTER_INFO_5));
	if (nt) {
	    // pPortName can specify multiple ports. Test them one at
	    // a time.
	    if (info5->pPortName != NULL) {
	        LPTSTR port = strtok(info5->pPortName, ",");
		while (port != NULL) {
		    if (_stricmp(lpcPrinterName, port) == 0) {
		        *foundPrinter = info5->pPrinterName;
			*foundPort = port;
			return TRUE;
		    }
		    port = strtok(NULL, ",");
		}
	    }
	} else {
	    if ((info5->pPrinterName != NULL &&
		 _stricmp(lpcPrinterName, info5->pPrinterName) == 0) ||
		(info5->pPortName != NULL &&
		 _stricmp(lpcPrinterName, info5->pPortName) == 0)) {
	        *foundPrinter = info5->pPrinterName;
                *foundPort = info5->pPortName;
		return TRUE;
	    }
	}
    }

    return FALSE;
}

extern "C" {

JNIEXPORT void JNICALL
Java_sun_awt_print_AwtPrintControl_initIDs(JNIEnv *env, jclass cls)
{
    TRY;

    // cls = env->FindClass("sun/awt/windows/AwtPrintControl");
    AwtPrintControl::dialogOwnerPeerID =
        env->GetFieldID(cls, "dialogOwnerPeer",
//                        "Lsun/awt/windows/WFramePeer;");
                        "Ljava/awt/peer/ComponentPeer;");
    AwtPrintControl::findWin32MediaID =
        env->GetMethodID(cls, "findWin32Media", "(IIIZ)I");
    AwtPrintControl::getWin32MediaID =
        env->GetMethodID(cls, "getWin32MediaAttrib", "()C");
    AwtPrintControl::setWin32MediaID =
        env->GetMethodID(cls, "setWin32MediaAttrib", "(III)V");


    cls = env->FindClass("java/awt/JobAttributes$DefaultSelectionType");
    jfieldID allID = env->GetStaticFieldID(cls, "ALL",
	"Ljava/awt/JobAttributes$DefaultSelectionType;");
    jfieldID rangeID = env->GetStaticFieldID(cls, "RANGE",
	"Ljava/awt/JobAttributes$DefaultSelectionType;");
    jfieldID selectionID = env->GetStaticFieldID(cls, "SELECTION",
	"Ljava/awt/JobAttributes$DefaultSelectionType;");
    AwtPrintControl::ALL =
        env->NewGlobalRef(env->GetStaticObjectField(cls, allID));
    AwtPrintControl::RANGE =
        env->NewGlobalRef(env->GetStaticObjectField(cls, rangeID));
    AwtPrintControl::SELECTION =
        env->NewGlobalRef(env->GetStaticObjectField(cls, selectionID));


    cls = env->FindClass("java/awt/JobAttributes$DestinationType");
    jfieldID fileID = env->GetStaticFieldID(cls, "FILE",
	"Ljava/awt/JobAttributes$DestinationType;");
    jfieldID printerID = env->GetStaticFieldID(cls, "PRINTER",
	"Ljava/awt/JobAttributes$DestinationType;");
    AwtPrintControl::FILE =
        env->NewGlobalRef(env->GetStaticObjectField(cls, fileID));
    AwtPrintControl::PRINTER =
        env->NewGlobalRef(env->GetStaticObjectField(cls, printerID));


    cls = env->FindClass("java/awt/JobAttributes$DialogType");
    jfieldID commonID = env->GetStaticFieldID(cls, "COMMON",
	"Ljava/awt/JobAttributes$DialogType;");
    jfieldID nativeID = env->GetStaticFieldID(cls, "NATIVE",
	"Ljava/awt/JobAttributes$DialogType;");
    jfieldID noneID = env->GetStaticFieldID(cls, "NONE",
	"Ljava/awt/JobAttributes$DialogType;");
    AwtPrintControl::COMMON =
        env->NewGlobalRef(env->GetStaticObjectField(cls, commonID));
    AwtPrintControl::NATIVE =
        env->NewGlobalRef(env->GetStaticObjectField(cls, nativeID));
    AwtPrintControl::NONE =
        env->NewGlobalRef(env->GetStaticObjectField(cls, noneID));


    cls = env->FindClass(
        "java/awt/JobAttributes$MultipleDocumentHandlingType");
    jfieldID sdccID = 
        env->GetStaticFieldID(cls, "SEPARATE_DOCUMENTS_COLLATED_COPIES",
            "Ljava/awt/JobAttributes$MultipleDocumentHandlingType;");
    jfieldID sducID =
        env->GetStaticFieldID(cls, "SEPARATE_DOCUMENTS_UNCOLLATED_COPIES",
            "Ljava/awt/JobAttributes$MultipleDocumentHandlingType;");
    AwtPrintControl::SEPARATE_DOCUMENTS_COLLATED_COPIES =
        env->NewGlobalRef(env->GetStaticObjectField(cls, sdccID));
    AwtPrintControl::SEPARATE_DOCUMENTS_UNCOLLATED_COPIES =
        env->NewGlobalRef(env->GetStaticObjectField(cls, sducID));


    cls = env->FindClass("java/awt/JobAttributes$SidesType");
    jfieldID oneSidedID = env->GetStaticFieldID(cls, "ONE_SIDED",
	"Ljava/awt/JobAttributes$SidesType;");
    jfieldID twoSidedLEID = env->GetStaticFieldID(cls, "TWO_SIDED_LONG_EDGE",
	"Ljava/awt/JobAttributes$SidesType;");
    jfieldID twoSidedSEID = env->GetStaticFieldID(cls, "TWO_SIDED_SHORT_EDGE",
	"Ljava/awt/JobAttributes$SidesType;");
    AwtPrintControl::ONE_SIDED =
        env->NewGlobalRef(env->GetStaticObjectField(cls, oneSidedID));
    AwtPrintControl::TWO_SIDED_LONG_EDGE =
        env->NewGlobalRef(env->GetStaticObjectField(cls, twoSidedLEID));
    AwtPrintControl::TWO_SIDED_SHORT_EDGE =
        env->NewGlobalRef(env->GetStaticObjectField(cls, twoSidedSEID));


    cls = env->FindClass("java/awt/PageAttributes$ColorType");
    jfieldID colorID = env->GetStaticFieldID(cls, "COLOR",
	"Ljava/awt/PageAttributes$ColorType;");
    jfieldID monochromeID = env->GetStaticFieldID(cls, "MONOCHROME",
	"Ljava/awt/PageAttributes$ColorType;");
    AwtPrintControl::COLOR =
        env->NewGlobalRef(env->GetStaticObjectField(cls, colorID));
    AwtPrintControl::MONOCHROME =
        env->NewGlobalRef(env->GetStaticObjectField(cls, monochromeID));


    cls = env->FindClass(
        "java/awt/PageAttributes$OrientationRequestedType");
    jfieldID portraitID = env->GetStaticFieldID(cls, "PORTRAIT",
	"Ljava/awt/PageAttributes$OrientationRequestedType;");
    jfieldID landscapeID = env->GetStaticFieldID(cls, "LANDSCAPE",
	"Ljava/awt/PageAttributes$OrientationRequestedType;");
    AwtPrintControl::PORTRAIT =
        env->NewGlobalRef(env->GetStaticObjectField(cls, portraitID));
    AwtPrintControl::LANDSCAPE =
        env->NewGlobalRef(env->GetStaticObjectField(cls, landscapeID));


    cls = env->FindClass("java/awt/PageAttributes$OriginType");
    jfieldID physicalID = env->GetStaticFieldID(cls, "PHYSICAL",
	"Ljava/awt/PageAttributes$OriginType;");
    jfieldID printableID = env->GetStaticFieldID(cls, "PRINTABLE",
	"Ljava/awt/PageAttributes$OriginType;");
    AwtPrintControl::PHYSICAL =
        env->NewGlobalRef(env->GetStaticObjectField(cls, physicalID));
    AwtPrintControl::PRINTABLE =
        env->NewGlobalRef(env->GetStaticObjectField(cls, printableID));


    cls = env->FindClass("java/awt/PageAttributes$PrintQualityType");
    jfieldID highID = env->GetStaticFieldID(cls, "HIGH",
	"Ljava/awt/PageAttributes$PrintQualityType;");
    jfieldID normalID = env->GetStaticFieldID(cls, "NORMAL",
	"Ljava/awt/PageAttributes$PrintQualityType;");
    jfieldID draftID = env->GetStaticFieldID(cls, "DRAFT",
	"Ljava/awt/PageAttributes$PrintQualityType;");
    AwtPrintControl::HIGH =
        env->NewGlobalRef(env->GetStaticObjectField(cls, highID));
    AwtPrintControl::NORMAL =
        env->NewGlobalRef(env->GetStaticObjectField(cls, normalID));
    AwtPrintControl::DRAFT =
        env->NewGlobalRef(env->GetStaticObjectField(cls, draftID));


    cls = env->FindClass("sun/awt/print/PrintControl");
    AwtPrintControl::doctitleID =
        env->GetFieldID(cls, "doctitle", "Ljava/lang/String;");
    
    AwtPrintControl::getColorID = env->GetMethodID(cls, "getColorAttrib",
        "()Ljava/awt/PageAttributes$ColorType;");
    AwtPrintControl::setColorID = env->GetMethodID(cls, "setColorAttrib",
        "(Ljava/awt/PageAttributes$ColorType;)V");
    AwtPrintControl::getCopiesID =
        env->GetMethodID(cls, "getCopiesAttrib", "()I");
    AwtPrintControl::setCopiesID =
        env->GetMethodID(cls, "setCopiesAttrib", "(I)V");
    AwtPrintControl::getSelectID = env->GetMethodID(cls, "getSelectAttrib",
        "()Ljava/awt/JobAttributes$DefaultSelectionType;");
    AwtPrintControl::setSelectID = env->GetMethodID(cls, "setSelectAttrib",
        "(Ljava/awt/JobAttributes$DefaultSelectionType;)V");
    AwtPrintControl::getDestID = env->GetMethodID(cls, "getDestAttrib",
        "()Ljava/awt/JobAttributes$DestinationType;");
    AwtPrintControl::setDestID = env->GetMethodID(cls, "setDestAttrib",
        "(Ljava/awt/JobAttributes$DestinationType;)V");
    AwtPrintControl::getDialogID = env->GetMethodID(cls, "getDialogAttrib",
        "()Ljava/awt/JobAttributes$DialogType;");
    AwtPrintControl::getFileNameID = env->GetMethodID(cls, "getFileNameAttrib",
        "()Ljava/lang/String;");
    // AwtPrintControl::setFileNameID = env->GetMethodID(cls,
        // "setFileNameAttrib", "(Ljava/lang/String;)V");
    AwtPrintControl::getFromPageID = env->GetMethodID(cls, "getFromPageAttrib",
       "()I");
    AwtPrintControl::setFromPageID = env->GetMethodID(cls, "setFromPageAttrib",
       "(I)V");
    AwtPrintControl::getMaxPageID = env->GetMethodID(cls, "getMaxPageAttrib",
       "()I");
    AwtPrintControl::getMinPageID = env->GetMethodID(cls, "getMinPageAttrib",
       "()I");
    AwtPrintControl::getMDHID = env->GetMethodID(cls, "getMDHAttrib",
       "()Ljava/awt/JobAttributes$MultipleDocumentHandlingType;");
    AwtPrintControl::setMDHID = env->GetMethodID(cls, "setMDHAttrib",
       "(Ljava/awt/JobAttributes$MultipleDocumentHandlingType;)V");
    AwtPrintControl::getOrientID = env->GetMethodID(cls, "getOrientAttrib",
	"()Ljava/awt/PageAttributes$OrientationRequestedType;");
    AwtPrintControl::setOrientID = env->GetMethodID(cls, "setOrientAttrib",
	"(Ljava/awt/PageAttributes$OrientationRequestedType;)V");
    AwtPrintControl::getOriginID = env->GetMethodID(cls, "getOriginAttrib",
	"()Ljava/awt/PageAttributes$OriginType;");
    AwtPrintControl::getQualityID = env->GetMethodID(cls, "getQualityAttrib",
	"()Ljava/awt/PageAttributes$PrintQualityType;");
    AwtPrintControl::setQualityID = env->GetMethodID(cls, "setQualityAttrib",
	"(Ljava/awt/PageAttributes$PrintQualityType;)V");
    AwtPrintControl::getPrintToFileEnabledID = env->GetMethodID(cls,
        "getPrintToFileEnabled", "()Z");
    AwtPrintControl::getPrinterID = env->GetMethodID(cls, "getPrinterAttrib",
        "()Ljava/lang/String;");
    AwtPrintControl::setPrinterID = env->GetMethodID(cls, "setPrinterAttrib",
        "(Ljava/lang/String;)V");
    AwtPrintControl::getResID = env->GetMethodID(cls, "getResAttrib", "()I");
    AwtPrintControl::getSidesID = env->GetMethodID(cls, "getSidesAttrib",
	"()Ljava/awt/JobAttributes$SidesType;");
    AwtPrintControl::setSidesID = env->GetMethodID(cls, "setSidesAttrib",
	"(Ljava/awt/JobAttributes$SidesType;)V");
    AwtPrintControl::getToPageID = env->GetMethodID(cls, "getToPageAttrib",
        "()I");
    AwtPrintControl::setToPageID = env->GetMethodID(cls, "setToPageAttrib",
        "(I)V");

    cls = env->FindClass("sun/awt/print/PrinterCapabilities");
    AwtPrintControl::printerNameID =
        env->GetFieldID(cls, "printerName", "Ljava/lang/String;");
    AwtPrintControl::capabilitiesID =
        env->GetFieldID(cls, "capabilities", "J");
    AwtPrintControl::sizesID = env->GetFieldID(cls, "sizes", "[I");

    DASSERT(AwtPrintControl::dialogOwnerPeerID != NULL);

    DASSERT(AwtPrintControl::findWin32MediaID != NULL);
    DASSERT(AwtPrintControl::getWin32MediaID != NULL);
    DASSERT(AwtPrintControl::setWin32MediaID != NULL);

    DASSERT(AwtPrintControl::ALL != NULL);
    DASSERT(AwtPrintControl::RANGE != NULL);
    DASSERT(AwtPrintControl::SELECTION != NULL);

    DASSERT(AwtPrintControl::FILE != NULL);
    DASSERT(AwtPrintControl::PRINTER != NULL);

    DASSERT(AwtPrintControl::COMMON != NULL);
    DASSERT(AwtPrintControl::NATIVE != NULL);
    DASSERT(AwtPrintControl::NONE != NULL);

    DASSERT(AwtPrintControl::SEPARATE_DOCUMENTS_COLLATED_COPIES != NULL);
    DASSERT(AwtPrintControl::SEPARATE_DOCUMENTS_UNCOLLATED_COPIES != NULL);

    DASSERT(AwtPrintControl::ONE_SIDED != NULL);
    DASSERT(AwtPrintControl::TWO_SIDED_LONG_EDGE != NULL);
    DASSERT(AwtPrintControl::TWO_SIDED_SHORT_EDGE != NULL);

    DASSERT(AwtPrintControl::COLOR != NULL);
    DASSERT(AwtPrintControl::MONOCHROME != NULL);

    DASSERT(AwtPrintControl::PORTRAIT != NULL);
    DASSERT(AwtPrintControl::LANDSCAPE != NULL);

    DASSERT(AwtPrintControl::PHYSICAL != NULL);
    DASSERT(AwtPrintControl::PRINTABLE != NULL);

    DASSERT(AwtPrintControl::HIGH != NULL);
    DASSERT(AwtPrintControl::NORMAL != NULL);
    DASSERT(AwtPrintControl::DRAFT != NULL);

    DASSERT(AwtPrintControl::doctitleID != NULL);

    DASSERT(AwtPrintControl::getColorID != NULL);
    DASSERT(AwtPrintControl::setColorID != NULL);
    DASSERT(AwtPrintControl::getCopiesID != NULL);
    DASSERT(AwtPrintControl::setCopiesID != NULL);
    DASSERT(AwtPrintControl::getSelectID != NULL);
    DASSERT(AwtPrintControl::setSelectID != NULL);
    DASSERT(AwtPrintControl::getDestID != NULL);
    DASSERT(AwtPrintControl::setDestID != NULL);
    DASSERT(AwtPrintControl::getDialogID != NULL);
    DASSERT(AwtPrintControl::getFileNameID != NULL);
    // DASSERT(AwtPrintControl::setFileNameID != NULL);
    DASSERT(AwtPrintControl::getFromPageID != NULL);
    DASSERT(AwtPrintControl::setFromPageID != NULL);
    DASSERT(AwtPrintControl::getMaxPageID != NULL);
    DASSERT(AwtPrintControl::getMinPageID != NULL);
    DASSERT(AwtPrintControl::getMDHID != NULL);
    DASSERT(AwtPrintControl::setMDHID != NULL);
    DASSERT(AwtPrintControl::getOrientID != NULL);
    DASSERT(AwtPrintControl::setOrientID != NULL);
    DASSERT(AwtPrintControl::getOriginID != NULL);
    DASSERT(AwtPrintControl::getQualityID != NULL);
    DASSERT(AwtPrintControl::setQualityID != NULL);
    DASSERT(AwtPrintControl::getPrintToFileEnabledID != NULL);
    DASSERT(AwtPrintControl::getPrinterID != NULL);
    DASSERT(AwtPrintControl::setPrinterID != NULL);
    DASSERT(AwtPrintControl::getResID != NULL);
    DASSERT(AwtPrintControl::getSidesID != NULL);
    DASSERT(AwtPrintControl::setSidesID != NULL);
    DASSERT(AwtPrintControl::getToPageID != NULL);
    DASSERT(AwtPrintControl::setToPageID != NULL);

    DASSERT(AwtPrintControl::printerNameID != NULL);
    DASSERT(AwtPrintControl::capabilitiesID != NULL);
    DASSERT(AwtPrintControl::sizesID != NULL);

    CATCH_BAD_ALLOC;
}

struct Paper {
    // For a standard size, set appropriately.
    // For a non-standard size, set to 0.
    short dmPaperSize;

    // Always define width and length in case a static mapping isn't
    // available for a particular driver.
    short dmPaperWidth;
    short dmPaperLength;

    // Used for caching previous results
    short driverIndex; // index into pDmPaperSize, pDmPaperDim
    short driverWidth;
    short driverLength;
};

static Paper printControlToDmPaper[] = {
    { /*4A0*/ 0, 16820, 23780 }, { /*2A0*/ 0, 11890, 16820 },
    { /*A0*/ 0, 8410, 11890 }, { /*A1*/ 0, 5940, 8410 },
    { /*A2*/ 0, 4200, 5940 }, { DMPAPER_A3, 2970, 4200 },
    { DMPAPER_A4, 2100, 2970 }, { DMPAPER_A5, 1480, 2100 },
    { /*A6*/ 0, 1050, 1480 }, { /*A7*/ 0, 740, 1050 },
    { /*A8*/ 0, 520, 740 }, { /*A9*/ 0, 370, 520 }, { /*A10*/ 0, 260, 370 },
    { /*B0*/ 0, 10000, 14140 }, { /*B1*/ 0, 7070, 10000 },
    { /*B2*/ 0, 5000, 7070 }, { /*B3*/ 0, 3530, 5000 },
    { DMPAPER_B4, 2500, 3530 }, { DMPAPER_ENV_B5, 1760, 2500 },
    { DMPAPER_ENV_B6, 1250, 1760 }, { /*B7*/ 0, 880, 1250 },
    { /*B8*/ 0, 620, 880 }, { /*B9*/ 0, 440, 620 }, { /*B10*/ 0, 310, 440 },
    { /*JIS B0*/ 0, 10300, 14560 }, { /*JIS B1*/ 0, 7280, 10300 },
    { /*JIS B2*/ 0, 5150, 7280 }, { /*JIS B3*/ 0, 3640, 5150 },
    { /*JIS B4*/ 0, 2570, 3640 }, { DMPAPER_B5, 1820, 2570 },
    { /*JIS B6*/ 0, 1280, 1820 }, { /*JIS B7*/ 0, 910, 1280 },
    { /*JIS B8*/ 0, 640, 910 }, { /*JIS B9*/ 0, 450, 640 },
    { /*JIS B10*/ 0, 320, 450 }, { /*C0*/ 0, 9170, 12970 },
    { /*C1*/ 0, 6480, 9170 }, { /*C2*/ 0, 4580, 6480 },
    { DMPAPER_ENV_C3, 3240, 4580 }, { DMPAPER_ENV_C4, 2290, 3240 },
    { DMPAPER_ENV_C5, 1620, 2290 }, { DMPAPER_ENV_C6, 1140, 1620 },
    { /*C7*/ 0, 810, 1140 }, { /*C8*/ 0, 570, 810 }, { /*C9*/ 0, 400, 570 },
    { /*C10*/ 0, 280, 400 }, { DMPAPER_ENV_DL, 1100, 2200 },
    { DMPAPER_EXECUTIVE, 1841, 2667 }, { DMPAPER_FOLIO, 2159, 3302 },
    { DMPAPER_STATEMENT, 1397, 2159 }, { DMPAPER_LEDGER, 2794, 4318 },
    { DMPAPER_LETTER, 2159, 2794 }, { DMPAPER_LEGAL, 2159, 3556 },
    { DMPAPER_QUARTO, 2150, 2750 }, { /*A*/ DMPAPER_LETTER, 2159, 2794 },
    { /*B*/ DMPAPER_11X17, 2794, 4318 }, { DMPAPER_CSHEET, 4318, 5588 },
    { DMPAPER_DSHEET, 5588, 8636 }, { DMPAPER_ESHEET, 8636, 11176 }, 
    { /*10x15*/ 0, 2540, 3810 }, { DMPAPER_10X14, 2540, 3556 },
    { /*10x13*/ 0, 2540, 3302 }, { /*9x12*/ 0, 2286, 3048 },
    { /*9x11*/ 0, 2286, 2794 }, { /*7x9*/ 0, 1778, 2286 },
    { /*6x9*/ 0, 1524, 2286 }, { DMPAPER_ENV_9, 984, 2254 },
    { DMPAPER_ENV_10, 1048, 2413 }, { DMPAPER_ENV_11, 1143, 2635 },
    { DMPAPER_ENV_12, 1206, 2794 }, { DMPAPER_ENV_14, 1270, 2921 },
    { /*Invite-Env*/ 0, 2200, 2200 }, { DMPAPER_ENV_ITALY, 1100, 2300 },
    { DMPAPER_ENV_MONARCH, 984, 1905 }, { DMPAPER_ENV_PERSONAL, 921, 1651 }
};

JNIEXPORT jchar JNICALL
Java_sun_awt_print_AwtPrintControl__1getWin32MediaAttrib(JNIEnv *env,
							 jobject peer,
							 jint pcIndex,
							 jstring printerName)
{
    TRY;

    if (printerName == NULL) {
        return 0;
    }

    LPTSTR foundPrinter = NULL, foundPort = NULL, papersBuf = NULL,
        papersizeBuf = NULL;
    LPBYTE buffer = NULL;
    DWORD cbBuf = 0;

    try {
        VERIFY(AwtPrintControl::FindPrinter(NULL, NULL, &cbBuf, NULL, NULL));
	buffer = new BYTE[cbBuf];
	if (!AwtPrintControl::FindPrinter(printerName, buffer, &cbBuf,
					  &foundPrinter, &foundPort)) {
	    delete [] buffer;
	    return 0;
	}

	int numSizes = ::DeviceCapabilities(foundPrinter, foundPort, DC_PAPERS,
					    NULL, NULL);
        if (numSizes == -1) {
            delete [] buffer;
            return 0;
        }

	papersBuf = new char[numSizes * sizeof(WORD)];
	papersizeBuf = new char[numSizes * sizeof(POINT)];

        if ((::DeviceCapabilities(foundPrinter, foundPort, 
                                  DC_PAPERS, papersBuf, NULL) == -1) ||
            (::DeviceCapabilities(foundPrinter, foundPort, 
                                  DC_PAPERSIZE, papersizeBuf, NULL) == -1)) {
            delete [] papersizeBuf;
            delete [] papersBuf;
            delete [] buffer;
            return 0;
        }

	WORD *pDmPaperSize = (WORD *)papersBuf;
	POINT *pDmPaperDim = (POINT *)papersizeBuf;

	short width, length;
	int i, bestError, pError, lError;
	short staticSize = printControlToDmPaper[pcIndex].dmPaperSize;

	if (staticSize != 0) {
	    if (printControlToDmPaper[pcIndex].driverWidth == 0) {
	        // Static mapping -- verify that the printer driver supports
	        // the size.
	        for (i = 0; i < numSizes; i++, pDmPaperSize++) {
		    if (staticSize == *pDmPaperSize) {
		    
		        // The driver supports the size. Cache the driverIndex
		        // to avoid O(n) operations in the future. Cache width
		        // and length as coherency information.
		        printControlToDmPaper[pcIndex].driverIndex = i;
			printControlToDmPaper[pcIndex].driverWidth = (short)
			    pDmPaperDim[i].x;
			printControlToDmPaper[pcIndex].driverLength = (short)
			    pDmPaperDim[i].y;

			goto done;
		    }
		}
	    } else {
	        // Cached mapping -- verify integrity because user may have
	        // changed printers.
	        short driverIndex = printControlToDmPaper[pcIndex].driverIndex;
		if (driverIndex < numSizes) {
		    if (staticSize == pDmPaperSize[driverIndex] &&
			printControlToDmPaper[pcIndex].driverWidth ==
		            pDmPaperDim[driverIndex].x &&
			printControlToDmPaper[pcIndex].driverLength ==
		            pDmPaperDim[driverIndex].y) {

		      goto done;
		    }
		}
	    }
	}

	// reset this pointer because we might have failed the static map test
	pDmPaperSize = (WORD *)papersBuf;

	width = printControlToDmPaper[pcIndex].dmPaperWidth;
	length = printControlToDmPaper[pcIndex].dmPaperLength;
	bestError = INT_MAX;

	for (i = 0; i < numSizes; i++, pDmPaperSize++, pDmPaperDim++) {
            pError = abs(width - pDmPaperDim->x) +
	             abs(length - pDmPaperDim->y);

	    // try rotated orientation as well
	    lError = abs(width - pDmPaperDim->y) +
	             abs(length - pDmPaperDim->x);

	    pError = (pError < lError) ? pError : lError;

	    if (pError < bestError) {
	        bestError = pError;
		printControlToDmPaper[pcIndex].dmPaperSize = *pDmPaperSize;

		// The driver supports the size. Cache the driverIndex
		// to avoid O(n) operations in the future. Cache width
		// and length as coherency information.
		printControlToDmPaper[pcIndex].driverIndex = i;
		printControlToDmPaper[pcIndex].driverWidth = (short)
		    pDmPaperDim->x;
		printControlToDmPaper[pcIndex].driverLength = (short)
		    pDmPaperDim->y;

		if (bestError == 0) {
		    // found an exact match
		    break;
		}
	    }
	}
    } catch (std::bad_alloc&) {
        delete [] papersizeBuf;
	delete [] papersBuf;
	delete [] buffer;
	throw;
    }

done:
    delete [] papersizeBuf;
    delete [] papersBuf;
    delete [] buffer;

    return printControlToDmPaper[pcIndex].dmPaperSize;

    CATCH_BAD_ALLOC_RET(0);
}

JNIEXPORT jstring JNICALL
Java_sun_awt_print_AwtPrintControl_getDefaultPrinterName(JNIEnv *env,
							 jobject peer)
{
    TRY;

    PRINTDLG pd;
    memset(&pd, 0, sizeof(PRINTDLG));
    pd.lStructSize = sizeof(PRINTDLG);
    pd.Flags = PD_RETURNDEFAULT;
    if (!AwtPrintDialog::PrintDlg(&pd)) {
        // No printers installed
        return NULL;
    }
    
    if (pd.hDevNames == NULL) {
        // Name of default printer not available
        if (pd.hDevMode != NULL) {
	    ::GlobalFree(pd.hDevMode);
	}
	return NULL;
    }

    DEVNAMES *devnames = (DEVNAMES *)::GlobalLock(pd.hDevNames);
    DASSERT(!IsBadReadPtr(devnames, sizeof(DEVNAMES)));
    char *lpcDevnames = (char *)devnames;
    char *printerName = lpcDevnames + devnames->wDeviceOffset;

    jstring jPrinterName = JNU_NewStringPlatform(env, printerName);

    ::GlobalUnlock(pd.hDevNames);
    ::GlobalFree(pd.hDevNames);
    if (pd.hDevMode != NULL) {
        ::GlobalFree(pd.hDevMode);
    }

    return jPrinterName;

    CATCH_BAD_ALLOC_RET(NULL);
}

JNIEXPORT jboolean JNICALL
Java_sun_awt_print_AwtPrintControl__1getCapabilities(JNIEnv *env, jobject peer,
						     jobject capabilities)
{
    TRY;

    jstring printerName = (jstring)
        env->GetObjectField(capabilities, AwtPrintControl::printerNameID);
    if (printerName == NULL) {
        return JNI_FALSE;
    }

    LPTSTR foundPrinter = NULL, foundPort = NULL, papersBuf = NULL,
        papersizeBuf = NULL;
    LPBYTE buffer = NULL, pPrinter = NULL;
    DWORD cbBuf = 0;

    try {
        VERIFY(AwtPrintControl::FindPrinter(NULL, NULL, &cbBuf, NULL, NULL));
	buffer = new BYTE[cbBuf];
	if (!AwtPrintControl::FindPrinter(printerName, buffer, &cbBuf,
					  &foundPrinter, &foundPort)) {
	    delete [] buffer;
	    return JNI_FALSE;
	}
	cbBuf = 0;

        jlong caps = 0;
    
        // Turn on duplex even if DeviceCapabilities isn't implemented in the
        // driver (the call will return -1), since printer still might support
        // duplex in this case. 
	if (::DeviceCapabilities(foundPrinter, foundPort, DC_DUPLEX, NULL,
				 NULL)) { 
	    caps |= sun_awt_print_PrinterCapabilities_DUPLEX;
	}

	// For now, turn on color printing unconditionally. When printing
	// to a file, the user should be able to print in color even if the
	// the printer doesn't support it. The user may be generating a
	// generic Postscript or PCL file which needs to be portable to other
	// printers.
	//
	// We could detect "print to file" and only reenable color printing
	// in that case, but that could be unintuitive to the user since
	// the color setting is on a different tab pane than the destination
	// setting.
	caps |= sun_awt_print_PrinterCapabilities_COLOR;

#if 0    
	HANDLE hPrinter = NULL;
	::OpenPrinter(foundPrinter, &hPrinter, NULL);
	::GetPrinter(hPrinter, 2, NULL, 0, &cbBuf);
	LPBYTE pPrinter = NULL;
	try {
	    pPrinter = new BYTE[cbBuf];
	} catch (std::bad_alloc&) {
	    ::ClosePrinter(hPrinter);
	    throw;
	}
	::GetPrinter(hPrinter, 2, pPrinter, cbBuf, &cbBuf);
	PRINTER_INFO_2 *info2 = (PRINTER_INFO_2 *)pPrinter;
	if (info2->pDevMode->dmFields & DM_COLOR) {
	    caps |= sun_awt_print_PrinterCapabilities_COLOR;
	}
	::ClosePrinter(hPrinter);
	delete [] pPrinter; pPrinter = NULL; info2 = NULL; hPrinter = NULL;
#endif

	int numSizes = ::DeviceCapabilities(foundPrinter, foundPort, DC_PAPERS,
					    NULL, NULL);
        // Fix for BugTraq Id 4281380.
        // If either of DeviceCapabilities calls fail don't set MEDIA cap 
        // and leave null 'sizes' field as it is. PrintDialog will use all 
        // known sizes in this case.
        if (numSizes != -1) {
	    papersBuf = new char[numSizes * sizeof(WORD)];
	    papersizeBuf = new char[numSizes * sizeof(POINT)];
            if ((::DeviceCapabilities(foundPrinter, foundPort, 
                                      DC_PAPERS, papersBuf, NULL) != -1) &&
		(::DeviceCapabilities(foundPrinter, foundPort, DC_PAPERSIZE,
				      papersizeBuf, NULL) != -1)) {

	        WORD *pDmPaperSize = (WORD *)papersBuf;
		POINT *pDmPaperDim = (POINT *)papersizeBuf;

		jintArray jpcIndicesArray = env->NewIntArray(numSizes);
		if (jpcIndicesArray == NULL) {
		    throw std::bad_alloc();
		}
		jboolean isCopy;
		jint *jpcIndices = env->GetIntArrayElements(jpcIndicesArray,
							    &isCopy);
		for (int i = 0; i < numSizes;
		     i++, pDmPaperSize++, pDmPaperDim++) {
		    jpcIndices[i] = 
		        env->CallIntMethod(peer,
					   AwtPrintControl::findWin32MediaID,
					   *pDmPaperSize, pDmPaperDim->x,
					   pDmPaperDim->y, JNI_TRUE);
		}
		env->ReleaseIntArrayElements(jpcIndicesArray, jpcIndices, 0);

		env->SetObjectField(capabilities, AwtPrintControl::sizesID,
				    jpcIndicesArray);

		caps |= sun_awt_print_PrinterCapabilities_MEDIA;

	    }
	    delete [] papersizeBuf;
	    delete [] papersBuf;
	}
	delete [] buffer;

        env->SetLongField(capabilities, AwtPrintControl::capabilitiesID,
                          caps);

    } catch (std::bad_alloc&) {
	delete [] papersizeBuf;
	delete [] papersBuf;
	delete [] buffer;
	throw;
    }

    return JNI_TRUE;

    CATCH_BAD_ALLOC_RET(JNI_FALSE);
}

JNIEXPORT jobjectArray JNICALL
Java_sun_awt_print_AwtPrintControl__1getPrinterList(JNIEnv *env,
						    jobject peer)
{
    TRY;

    DWORD cbNeeded = 0;
    DWORD cReturned = 0;
    LPBYTE pPrinterEnum = NULL;

    jstring utf_str;
    jclass clazz = env->FindClass("java/lang/String");
    jobjectArray nameArray;

    try {
        if (IS_NT) {
	    ::EnumPrinters(PRINTER_ENUM_LOCAL | PRINTER_ENUM_CONNECTIONS,
			   NULL, 4, NULL, 0, &cbNeeded, &cReturned);
	    pPrinterEnum = new BYTE[cbNeeded];
	    ::EnumPrinters(PRINTER_ENUM_LOCAL | PRINTER_ENUM_CONNECTIONS,
			   NULL, 4, pPrinterEnum, cbNeeded, &cbNeeded,
			   &cReturned);

	    if (cReturned > 0) {
	        nameArray = env->NewObjectArray(cReturned, clazz, NULL);
		if (nameArray == NULL) {
		    throw std::bad_alloc();
		}
	    } else {
	        nameArray = NULL;
	    }

	    for (DWORD i = 0; i < cReturned; i++) {
	        PRINTER_INFO_4 *info4 = (PRINTER_INFO_4 *)
		    (pPrinterEnum + i * sizeof(PRINTER_INFO_4));
		utf_str = JNU_NewStringPlatform(env, info4->pPrinterName);
		if (utf_str == NULL) {
		    throw std::bad_alloc();
		}
		env->SetObjectArrayElement(nameArray, i, utf_str);
		env->DeleteLocalRef(utf_str);
	    }
	} else {
	    ::EnumPrinters(PRINTER_ENUM_LOCAL,
			   NULL, 5, NULL, 0, &cbNeeded, &cReturned);
	    pPrinterEnum = new BYTE[cbNeeded];
	    ::EnumPrinters(PRINTER_ENUM_LOCAL,
			   NULL, 5, pPrinterEnum, cbNeeded, &cbNeeded,
			   &cReturned);

	    if (cReturned > 0) {
	        nameArray = env->NewObjectArray(cReturned, clazz, NULL);
		if (nameArray == NULL) {
		    throw std::bad_alloc();
		}
	    } else {
	        nameArray = NULL;
	    }

	    for (DWORD i = 0; i < cReturned; i++) {
	        PRINTER_INFO_5 *info5 = (PRINTER_INFO_5 *)
		    (pPrinterEnum + i * sizeof(PRINTER_INFO_5));
		utf_str = JNU_NewStringPlatform(env, info5->pPrinterName);
		if (utf_str == NULL) {
	            throw std::bad_alloc();
		}
		env->SetObjectArrayElement(nameArray, i, utf_str);
		env->DeleteLocalRef(utf_str);
	    }
	}
    } catch (std::bad_alloc&) {
        delete [] pPrinterEnum;
	throw;
    }

    delete [] pPrinterEnum;
    return nameArray;

    CATCH_BAD_ALLOC_RET(NULL);
}

BOOL CALLBACK PrintDlgHook(HWND hDlg, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
    TRY;

    if (iMsg == WM_INITDIALOG) {
	SetForegroundWindow(hDlg);
	return FALSE;
    }
    return FALSE;

    CATCH_BAD_ALLOC_RET(TRUE);
}

BOOL AwtPrintControl::CreateDevModeAndDevNames(PRINTDLG *ppd,
					       LPTSTR pPrinterName,
					       LPTSTR pPortName)
{
    DWORD cbNeeded = 0;
    LPBYTE pPrinter = NULL;
    BOOL retval = FALSE;
    HANDLE hPrinter;

    try {
        if (!::OpenPrinter(pPrinterName, &hPrinter, NULL)) {
	    goto done;
	}
	VERIFY(::GetPrinter(hPrinter, 2, NULL, 0, &cbNeeded) == 0);
	if (::GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
	    goto done;
	}
	pPrinter = new BYTE[cbNeeded];
	if (!::GetPrinter(hPrinter, 2, pPrinter, cbNeeded, &cbNeeded)) {
	    goto done;
	}
	PRINTER_INFO_2 *info2 = (PRINTER_INFO_2 *)pPrinter;

	// Create DEVMODE, if it exists.
	if (info2->pDevMode != NULL) {
	    size_t devmodeSize =
	        sizeof(DEVMODE) + info2->pDevMode->dmDriverExtra;
	    ppd->hDevMode = ::GlobalAlloc(GHND, devmodeSize);
	    if (ppd->hDevMode == NULL) {
	        throw std::bad_alloc();
	    }
	    DEVMODE *devmode = (DEVMODE *)::GlobalLock(ppd->hDevMode);
	    DASSERT(!::IsBadWritePtr(devmode, devmodeSize));
	    memcpy(devmode, info2->pDevMode, devmodeSize);
	    VERIFY(::GlobalUnlock(ppd->hDevMode) == 0);
	    DASSERT(::GetLastError() == NO_ERROR);
	}

	// Create DEVNAMES.
	if (IS_NT) {
	    if (pPortName != NULL) {
	        info2->pPortName = pPortName;
	    } else if (info2->pPortName != NULL) {
	        // pPortName may specify multiple ports. We only want one.
	        info2->pPortName = strtok(info2->pPortName, ",");
	    }
	}

	size_t lenDriverName = ((info2->pDriverName != NULL)
				    ? strlen(info2->pDriverName)
				    : 0) + 1;
	size_t lenPrinterName = ((pPrinterName != NULL)
				     ? strlen(pPrinterName)
				     : 0) + 1;
	size_t lenOutputName = ((info2->pPortName != NULL)
				    ? strlen(info2->pPortName)
				    : 0) + 1;
	
	ppd->hDevNames = ::GlobalAlloc(GHND,
				       sizeof(DEVNAMES) +
				       lenDriverName +
				       lenPrinterName +
				       lenOutputName
				       );
	if (ppd->hDevNames == NULL) {
	    throw std::bad_alloc();
	}

	DEVNAMES *devnames =
	    (DEVNAMES *)::GlobalLock(ppd->hDevNames);
	DASSERT(!IsBadWritePtr(devnames, sizeof(DEVNAMES) + lenDriverName +
			       lenPrinterName + lenOutputName));
	char *lpcDevnames = (char *)devnames;
	devnames->wDriverOffset = sizeof(DEVNAMES);
	devnames->wDeviceOffset = static_cast<WORD>(sizeof(DEVNAMES) + lenDriverName);
	devnames->wOutputOffset =
	    static_cast<WORD>(sizeof(DEVNAMES) + lenDriverName + lenPrinterName);
	if (info2->pDriverName != NULL) {
	    strcpy(lpcDevnames + devnames->wDriverOffset,
		   info2->pDriverName);
	} else {
	    *(lpcDevnames + devnames->wDriverOffset) = '\0';
	}
	if (pPrinterName != NULL) {
	    strcpy(lpcDevnames + devnames->wDeviceOffset,
		   pPrinterName);
	} else {
	    *(lpcDevnames + devnames->wDeviceOffset) = '\0';
	}
	if (info2->pPortName != NULL) {
	    strcpy(lpcDevnames + devnames->wOutputOffset,
		   info2->pPortName);
	} else {
	    *(lpcDevnames + devnames->wOutputOffset) = '\0';
	}
	VERIFY(::GlobalUnlock(ppd->hDevNames) == 0);
	DASSERT(::GetLastError() == NO_ERROR);
    } catch (std::bad_alloc&) {
        if (ppd->hDevNames != NULL) {
	    VERIFY(::GlobalFree(ppd->hDevNames) == NULL);
	    ppd->hDevNames = NULL;
	}
	if (ppd->hDevMode != NULL) {
	    VERIFY(::GlobalFree(ppd->hDevMode) == NULL);
	    ppd->hDevMode = NULL;
	}
	delete [] pPrinter;
	VERIFY(::ClosePrinter(hPrinter));
	hPrinter = NULL;
	throw;
    }

    retval = TRUE;

done:
    delete [] pPrinter;
    if (hPrinter) {
        VERIFY(::ClosePrinter(hPrinter));
	hPrinter = NULL;
    }

    return retval;
}

/*
 * Copy settings into a print dialog & any devmode
 */
void AwtPrintControl::InitPrintDialog(JNIEnv *env,
				      jobject printCtrl, PRINTDLG &pd) {


    HWND hwndOwner = NULL;
    jobject dialogOwner =
        env->GetObjectField(printCtrl, AwtPrintControl::dialogOwnerPeerID);
    if (dialogOwner != NULL) {
        AwtComponent *dialogOwnerComp =
	    (AwtComponent *)JNI_GET_PDATA(dialogOwner);
	hwndOwner = dialogOwnerComp->GetHWnd();
	env->DeleteLocalRef(dialogOwner);
	dialogOwner = NULL;
    }

    jobject mdh = NULL;
    jobject dest = NULL;
    jobject select = NULL;
    jobject dialog = NULL;

    // If the user didn't specify a printer, then this call returns the
    // name of the default printer.
    jstring printerName = (jstring)
        env->CallObjectMethod(printCtrl, AwtPrintControl::getPrinterID);

    if (printerName != NULL) {
        LPTSTR foundPrinter = NULL;
	LPTSTR foundPort = NULL;
	DWORD cbBuf = 0;
	
	VERIFY(AwtPrintControl::FindPrinter(NULL, NULL, &cbBuf, NULL, NULL));
	LPBYTE buffer = new BYTE[cbBuf];
 
	if (AwtPrintControl::FindPrinter(printerName, buffer, &cbBuf,
					 &foundPrinter, &foundPort)) {
 
	    if (!AwtPrintControl::CreateDevModeAndDevNames(&pd,
						   foundPrinter, foundPort)) {
	        delete [] buffer;
		JNU_ThrowInternalError(env, "Unable to open printer");
	        return;
	    }

	    DASSERT(pd.hDevNames != NULL);
	} else {
	    delete [] buffer;
	    JNU_ThrowIllegalArgumentException(
                env, "Invalid value for property printer");
	    return;
	}

	delete [] buffer;
    } else {
        // There is no default printer. This means that there are no
        // printers installed at all.
        dialog = env->CallObjectMethod(printCtrl,
				       AwtPrintControl::getDialogID);

	if (env->IsSameObject(dialog, AwtPrintControl::NATIVE)) {
	    // If we were to display the native print dialog, instead
	    // display a native warning message by calling PrintDlg
	    // without the PD_RETURNDEFAULT flag set.
	    AwtPrintDialog::PrintDlg(&pd);
	} else {
	    JNU_ThrowInternalError(env, "No default printer installed");
	}

	env->DeleteLocalRef(dialog); dialog = NULL;
	return;
    }

    // Now, set-up the struct for the real calls to ::PrintDlg and ::CreateDC
    pd.hwndOwner = hwndOwner;
    pd.Flags = PD_ENABLEPRINTHOOK;
    pd.lpfnPrintHook = (LPPRINTHOOKPROC)PrintDlgHook;

    mdh = env->CallObjectMethod(printCtrl, AwtPrintControl::getMDHID);
    if (env->IsSameObject(mdh,
		      AwtPrintControl::SEPARATE_DOCUMENTS_COLLATED_COPIES)) {
        pd.Flags |= PD_COLLATE;
    }
    env->DeleteLocalRef(mdh); mdh = NULL;
    dest = env->CallObjectMethod(printCtrl, AwtPrintControl::getDestID);
    if (env->IsSameObject(dest, AwtPrintControl::FILE)) {
        pd.Flags |= PD_PRINTTOFILE;
    }

    env->DeleteLocalRef(dest); dest = NULL;
    if (!env->CallBooleanMethod(printCtrl,
				AwtPrintControl::getPrintToFileEnabledID)) {
        pd.Flags |= PD_DISABLEPRINTTOFILE;
    }

    select = env->CallObjectMethod(printCtrl, AwtPrintControl::getSelectID);
    if (env->IsSameObject(select, AwtPrintControl::ALL)) {
        pd.Flags |= PD_ALLPAGES;
    } else if (env->IsSameObject(select, AwtPrintControl::RANGE)) {
        pd.Flags |= PD_PAGENUMS;
    } else {
        pd.Flags |= PD_SELECTION;
    }
    env->DeleteLocalRef(select); select = NULL;
    pd.nFromPage = (WORD)env->CallIntMethod(printCtrl, 
                                            AwtPrintControl::getFromPageID);
    pd.nToPage = (WORD)env->CallIntMethod(printCtrl, 
                                          AwtPrintControl::getToPageID);
    pd.nMinPage = (WORD)env->CallIntMethod(printCtrl,
                                           AwtPrintControl::getMinPageID);
    jint maxPage = env->CallIntMethod(printCtrl, AwtPrintControl::getMaxPageID);
    pd.nMaxPage = (maxPage <= (jint)((WORD)-1)) ? (WORD)maxPage : (WORD)-1;


    pd.nCopies = (WORD)env->CallIntMethod(printCtrl,
                                          AwtPrintControl::getCopiesID);

    // Set-up DEVMODE struct, if it exists. Win16 printer drivers do not
    // support DEVMODE.

    if (pd.hDevMode != NULL) {
        DEVMODE *devmode = (DEVMODE *)::GlobalLock(pd.hDevMode);
	DASSERT(!IsBadWritePtr(devmode, sizeof(DEVMODE)));

        devmode->dmFields |= DM_COPIES | DM_COLLATE | DM_ORIENTATION |
                             DM_PAPERSIZE | DM_PRINTQUALITY | DM_COLOR |
                             DM_DUPLEX;

	// We have to set this copies field because its value could override
	// the value of pd.nCopies in the native print dialog. We have to
	// reset it to 1 before we call ::CreateDC, however.
        devmode->dmCopies = pd.nCopies;

        devmode->dmCollate = (pd.Flags & PD_COLLATE) ? DMCOLLATE_TRUE
                                                     : DMCOLLATE_FALSE;

        jobject orient = env->CallObjectMethod(printCtrl,
                                               AwtPrintControl::getOrientID);
        if (env->IsSameObject(orient, AwtPrintControl::PORTRAIT)) {
            devmode->dmOrientation = DMORIENT_PORTRAIT;
        } else if (env->IsSameObject(orient,
                                     AwtPrintControl::LANDSCAPE)) {
            devmode->dmOrientation = DMORIENT_LANDSCAPE;
        }
        env->DeleteLocalRef(orient); orient = NULL;

	short paperSize =
	    env->CallCharMethod(printCtrl, AwtPrintControl::getWin32MediaID);
	if (paperSize > 0) {
	    devmode->dmPaperSize = paperSize;
	}

        jobject quality =
	    env->CallObjectMethod(printCtrl, AwtPrintControl::getQualityID);
        if (env->IsSameObject(quality, AwtPrintControl::HIGH)) {
            devmode->dmPrintQuality = DMRES_HIGH;
        } else if (env->IsSameObject(quality,
                                     AwtPrintControl::NORMAL)) {
            devmode->dmPrintQuality = DMRES_MEDIUM;
        } else {
            devmode->dmPrintQuality = DMRES_DRAFT;
        }
        env->DeleteLocalRef(quality); quality = NULL;

        jobject color = env->CallObjectMethod(printCtrl,
                                              AwtPrintControl::getColorID);
        if (env->IsSameObject(color, AwtPrintControl::COLOR)) {
            devmode->dmColor = DMCOLOR_COLOR;
        } else {
            devmode->dmColor = DMCOLOR_MONOCHROME;
        }
        env->DeleteLocalRef(color); color = NULL;

	jobject sides = env->CallObjectMethod(printCtrl,
					      AwtPrintControl::getSidesID);
	if (env->IsSameObject(sides, AwtPrintControl::ONE_SIDED)) {
	    devmode->dmDuplex = DMDUP_SIMPLEX;
	} else if (env->IsSameObject(sides,
				     AwtPrintControl::TWO_SIDED_LONG_EDGE)) {
	    devmode->dmDuplex = DMDUP_VERTICAL;
	} else {
	    devmode->dmDuplex = DMDUP_HORIZONTAL;
	}
	env->DeleteLocalRef(sides); sides = NULL;

        ::GlobalUnlock(pd.hDevMode);
	devmode = NULL;
    }
}

/*
 * Copy settings from print dialog & any devmode back into attributes
 * or properties.
 */
void AwtPrintControl::UpdateAttributes(JNIEnv *env,
				      jobject printCtrl, PRINTDLG &pd) {
    DEVNAMES *devnames = NULL;
    DEVMODE *devmode = NULL;

	int iResolutionX = ::GetDeviceCaps(pd.hDC, LOGPIXELSX);
	int iResolutionY = ::GetDeviceCaps(pd.hDC, LOGPIXELSY);
	int iPhysicalX = ::GetDeviceCaps(pd.hDC, PHYSICALWIDTH);
	int iPhysicalY = ::GetDeviceCaps(pd.hDC, PHYSICALHEIGHT);

	if (pd.hDevMode != NULL) {
	    devmode = (DEVMODE *)::GlobalLock(pd.hDevMode);
	    DASSERT(!IsBadReadPtr(devmode, sizeof(DEVMODE)));
	}
	env->CallVoidMethod(printCtrl, AwtPrintControl::setMDHID,
            (pd.Flags & PD_COLLATE)
	        ? AwtPrintControl::SEPARATE_DOCUMENTS_COLLATED_COPIES
		: AwtPrintControl::SEPARATE_DOCUMENTS_UNCOLLATED_COPIES);
	if (devmode != NULL && devmode->dmFields & DM_COLOR) {
	  env->CallVoidMethod(printCtrl,
               AwtPrintControl::setColorID,
                (devmode->dmColor == DMCOLOR_COLOR) 
                    ? AwtPrintControl::COLOR
                    : AwtPrintControl::MONOCHROME);
	}
	env->CallVoidMethod(printCtrl,
			    AwtPrintControl::setDestID,
			    (pd.Flags & PD_PRINTTOFILE)
				? AwtPrintControl::FILE
				: AwtPrintControl::PRINTER);
	// Set printer even when printing to a file because specific
	// printers could produce different output files (e.g., PCL v.
	// PostScript) or support different paper sizes
	if (pd.hDevNames != NULL) {
	    devnames = (DEVNAMES*)::GlobalLock(pd.hDevNames);
	    DASSERT(!IsBadReadPtr(devnames, sizeof(DEVNAMES)));
	    char *lpcNames = (char *)devnames;
	    char *buf = strdup(lpcNames + devnames->wDeviceOffset);
	    if (buf != NULL) {
	        env->CallVoidMethod(printCtrl,
				    AwtPrintControl::setPrinterID,
				    AwtFont::multibyte2javaString(env, buf));
		free(buf);
	    }
	    ::GlobalUnlock(pd.hDevNames);
	    devnames = NULL;
	}
	switch (pd.Flags & (PD_ALLPAGES | PD_PAGENUMS | PD_SELECTION)) {
	  case PD_ALLPAGES:
	    env->CallVoidMethod(printCtrl,
				AwtPrintControl::setSelectID,
				AwtPrintControl::ALL);
	    break;
	  case PD_PAGENUMS:
	    env->CallVoidMethod(printCtrl,
				AwtPrintControl::setSelectID,
				AwtPrintControl::RANGE);
	    break;
	  case PD_SELECTION:
	    env->CallVoidMethod(printCtrl,
				AwtPrintControl::setSelectID,
				AwtPrintControl::SELECTION);
	    break;
	}
	if (devmode != NULL && devmode->dmFields & DM_DUPLEX) {
	    jobject sides;
	    if (devmode->dmDuplex == DMDUP_SIMPLEX) {
	        sides = AwtPrintControl::ONE_SIDED;
	    } else if (devmode->dmDuplex == DMDUP_HORIZONTAL) {
	        sides = AwtPrintControl::TWO_SIDED_SHORT_EDGE;
	    } else {
	        sides = AwtPrintControl::TWO_SIDED_LONG_EDGE;
	    }
	    env->CallVoidMethod(printCtrl,
				AwtPrintControl::setSidesID, sides);
	}

	if ((devmode != NULL) && (devmode->dmFields & DM_COPIES)
	    && (pd.nCopies == 1)) {
	    env->CallVoidMethod(printCtrl,
			    AwtPrintControl::setCopiesID,
			    devmode->dmCopies);
	} else {
	    env->CallVoidMethod(printCtrl,
				AwtPrintControl::setCopiesID,
				pd.nCopies); 
	}

	if (devmode != NULL && devmode->dmFields & DM_ORIENTATION) {
	    env->CallVoidMethod(printCtrl,
				AwtPrintControl::setOrientID,
				(devmode->dmOrientation == DMORIENT_PORTRAIT) 
				    ? AwtPrintControl::PORTRAIT
				    : AwtPrintControl::LANDSCAPE);
	}
	if (devmode != NULL && devmode->dmFields & DM_PAPERSIZE) {
	    short paperSize = devmode->dmPaperSize;
	    if (paperSize >= DMPAPER_FIRST &&
		paperSize <= DMPAPER_FANFOLD_LGL_GERMAN) {
	        env->CallVoidMethod(printCtrl,
				    AwtPrintControl::setWin32MediaID,
				    paperSize, 0, 0);
	    } else if (paperSize == 0 ||
		       paperSize > DMPAPER_FANFOLD_LGL_GERMAN) {
	        short length = ROUNDTOINT((iPhysicalY/(double)iResolutionY) *
					  DEFAULT_RES * TENTHS_MM_TO_POINTS);
		short width = ROUNDTOINT((iPhysicalX/(double)iResolutionX) *
					 DEFAULT_RES * TENTHS_MM_TO_POINTS);
		env->CallVoidMethod(printCtrl,
				    AwtPrintControl::setWin32MediaID,
				    paperSize, width, length);
	    }
	}
	if (devmode != NULL && devmode->dmFields & DM_PRINTQUALITY) {
	    switch (devmode->dmPrintQuality) {
	      case DMRES_HIGH:
		env->CallVoidMethod(printCtrl,
				    AwtPrintControl::setQualityID,
				    AwtPrintControl::HIGH);
		break;
	      case DMRES_MEDIUM:
		env->CallVoidMethod(printCtrl,
				    AwtPrintControl::setQualityID,
				    AwtPrintControl::NORMAL);
		break;
	      case DMRES_LOW:
	      case DMRES_DRAFT:
		  env->CallVoidMethod(printCtrl,
				      AwtPrintControl::setQualityID,
				      AwtPrintControl::DRAFT);
		  break;
	    }
	}

	if (devmode != NULL && devmode->dmFields & DM_COLLATE) {
	  env->CallVoidMethod(printCtrl, AwtPrintControl::setMDHID,
            (devmode->dmCollate == DMCOLLATE_TRUE)
	        ? AwtPrintControl::SEPARATE_DOCUMENTS_COLLATED_COPIES
		: AwtPrintControl::SEPARATE_DOCUMENTS_UNCOLLATED_COPIES);
	}

	// Set toPage before fromPage because fromPage can never
	// exceed toPage.
	env->CallVoidMethod(printCtrl, AwtPrintControl::setToPageID,
			    pd.nToPage);
	env->CallVoidMethod(printCtrl, AwtPrintControl::setFromPageID,
			    pd.nFromPage);
	if (devmode != NULL) {
	    ::GlobalUnlock(pd.hDevMode);
	    devmode = NULL;
	}
}

} /* extern "C" */
