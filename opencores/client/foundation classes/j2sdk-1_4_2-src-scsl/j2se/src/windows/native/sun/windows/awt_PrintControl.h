/*
 * @(#)awt_PrintControl.h	1.10 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef _AWT_PRINT_CONTROL_H_
#define _AWT_PRINT_CONTROL_H_

#include "stdhdrs.h"
#include <commdlg.h>

/************************************************************************
 * AwtPrintControl class
 */

class AwtPrintControl {
public:

    /* sun.awt.windows.AwtPrintControl */

    static jfieldID  dialogOwnerPeerID;

    static jmethodID findWin32MediaID;
    static jmethodID getWin32MediaID;
    static jmethodID setWin32MediaID;


    /* java.awt.print.JobAttributes */

    static jobject ALL;
    static jobject RANGE;
    static jobject SELECTION;
    static jobject FILE;
    static jobject PRINTER;
    static jobject COMMON;
    static jobject NATIVE;
    static jobject NONE;
    static jobject SEPARATE_DOCUMENTS_COLLATED_COPIES;
    static jobject SEPARATE_DOCUMENTS_UNCOLLATED_COPIES;
    static jobject ONE_SIDED;
    static jobject TWO_SIDED_LONG_EDGE;
    static jobject TWO_SIDED_SHORT_EDGE;


    /* java.awt.print.PageAttributes */

    static jobject COLOR;
    static jobject MONOCHROME;
    static jobject PORTRAIT;
    static jobject LANDSCAPE;
    static jobject PHYSICAL;
    static jobject PRINTABLE;
    static jobject HIGH;
    static jobject NORMAL;
    static jobject DRAFT;


    /* sun.awt.print.PrintControl */

    static jfieldID doctitleID;

    static jmethodID getColorID;
    static jmethodID setColorID;
    static jmethodID getCopiesID;
    static jmethodID setCopiesID;
    static jmethodID getSelectID;
    static jmethodID setSelectID;
    static jmethodID getDestID;
    static jmethodID setDestID;
    static jmethodID getDialogID;
    static jmethodID getFileNameID;
    // static jmethodID setFileNameID;
    static jmethodID getFromPageID;
    static jmethodID setFromPageID;
    static jmethodID getMaxPageID;
    static jmethodID getMinPageID;
    static jmethodID getMDHID;
    static jmethodID setMDHID;
    static jmethodID getOrientID;
    static jmethodID setOrientID;
    static jmethodID getOriginID;
    static jmethodID getQualityID;
    static jmethodID setQualityID;
    static jmethodID getPrintToFileEnabledID;
    static jmethodID getPrinterID;
    static jmethodID setPrinterID;
    static jmethodID getResID;
    static jmethodID getSidesID;
    static jmethodID setSidesID;
    static jmethodID getToPageID;
    static jmethodID setToPageID;


    /* sun.awt.print.PrintCapabilities */

    static jfieldID printerNameID;
    static jfieldID capabilitiesID;
    static jfieldID sizesID;


    static BOOL FindPrinter(jstring printerName, LPBYTE pPrinterEnum,
			    LPDWORD pcbBuf, LPTSTR * foundPrinter,
			    LPTSTR * foundPORT);
    // This function determines whether the printer driver
    // for the passed printer handle supports PRINTER_INFO
    // structure of level dwLevel.
    static BOOL IsSupportedLevel(HANDLE hPrinter, DWORD dwLevel);
    static BOOL AwtPrintControl::CreateDevModeAndDevNames(PRINTDLG *ppd,
					       LPTSTR pPrinterName,
					       LPTSTR pPortName);
    static void AwtPrintControl::InitPrintDialog(JNIEnv *env,
		 		      jobject printCtrl, PRINTDLG &pd);
    static void AwtPrintControl::UpdateAttributes(JNIEnv *env,
				      jobject printCtrl, PRINTDLG &pd); 
};

#endif
