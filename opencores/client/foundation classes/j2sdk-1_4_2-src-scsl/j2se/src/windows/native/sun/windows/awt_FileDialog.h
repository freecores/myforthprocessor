/*
 * @(#)awt_FileDialog.h	1.16 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef AWT_FILE_DIALOG_H
#define AWT_FILE_DIALOG_H

#include "stdhdrs.h"
#include <commdlg.h>

#include "awt_Toolkit.h"
#include "awt_Component.h"
#include "awt_Dialog.h"

#include "java_awt_FileDialog.h"
#include "sun_awt_windows_WFileDialogPeer.h"


/************************************************************************
 * AwtFileDialog class
 */

class AwtFileDialog {
public:
    /* sun.awt.windows.WFileDialogPeer field and method ids */
    static jfieldID parentID;
    static jfieldID hwndID;
    static jfieldID fileFilterID;
    static jmethodID handleSelectedMID;
    static jmethodID handleCancelMID;
    static jmethodID checkFilenameFilterMID;

    /* java.awt.FileDialog field and method ids */
    static jfieldID modeID;
    static jfieldID dirID;
    static jfieldID fileID;
    static jfieldID filterID;

    static void Initialize(JNIEnv *env, jstring filterDescription);
    static void Show(void *peer);

    static BOOL GetOpenFileName(LPOPENFILENAMEA);
    static BOOL GetSaveFileName(LPOPENFILENAMEA);

    virtual BOOL InheritsNativeMouseWheelBehavior();
};

#endif /* FILE_DIALOG_H */
