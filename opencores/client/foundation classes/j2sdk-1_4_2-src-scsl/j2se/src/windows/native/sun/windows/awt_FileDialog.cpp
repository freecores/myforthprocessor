/*
 * @(#)awt_FileDialog.cpp	1.43 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "awt.h"
#include "awt_FileDialog.h"
#include "awt_Dialog.h"
#include "awt_Toolkit.h"
#include "awt_dlls.h"
#include <commdlg.h>
#include <cderr.h>
#include <shlobj.h>


/************************************************************************
 * AwtFileDialog fields
 */

/* WFileDialogPeer ids */
jfieldID AwtFileDialog::parentID;
jfieldID AwtFileDialog::hwndID;
jfieldID AwtFileDialog::fileFilterID;
jmethodID AwtFileDialog::handleSelectedMID;
jmethodID AwtFileDialog::handleCancelMID;
jmethodID AwtFileDialog::checkFilenameFilterMID;

/* FileDialog ids */
jfieldID AwtFileDialog::modeID;
jfieldID AwtFileDialog::dirID;
jfieldID AwtFileDialog::fileID;
jfieldID AwtFileDialog::filterID;

/* Localized filter string */
static char s_fileFilterString[128];
/* Non-localized suffix of the filter string */
static const char s_additionalString[] = " (*.*)\0*.*\0";

/***********************************************************************/

void
AwtFileDialog::Initialize(JNIEnv *env, jstring filterDescription)
{
    int length = env->GetStringLength(filterDescription);
    DASSERT(length * 2 + 1 < 128);
    AwtFont::javaString2multibyte(env, filterDescription, s_fileFilterString,
				  length);

    //AdditionalString should be terminated by two NULL characters (Windows
    //requirement), so we have to organize the following cycle and use memcpy
    //unstead of, for example, strcat.
    char *s = s_fileFilterString;
    while (*s) {
        ++s;
	DASSERT(s < s_fileFilterString + 128);
    }
    DASSERT(s + sizeof(s_additionalString) < s_fileFilterString + 128);
    memcpy(s, s_additionalString, sizeof(s_additionalString));
}

static UINT_PTR CALLBACK
FileDialogHookProc(HWND hdlg, UINT uiMsg, WPARAM wParam, LPARAM lParam)
{
    TRY;

    switch(uiMsg) {
        case WM_INITDIALOG: {
            OPENFILENAME *ofn = (OPENFILENAME *)lParam;
            jobject peer = (jobject)(ofn->lCustData);
            JNIEnv *env = (JNIEnv *)JNU_GetEnv(jvm, JNI_VERSION_1_2);
            env->SetLongField(peer, AwtFileDialog::hwndID,
            (jlong)::GetParent(hdlg));
            ::SetWindowLongPtr(hdlg, DWLP_USER, (UINT_PTR)peer);
            // fix for 4508670 - disable CS_SAVEBITS
            DWORD style = ::GetClassLong(hdlg,GCL_STYLE);
            ::SetClassLong(hdlg,GCL_STYLE,style & ~CS_SAVEBITS);            
            break;
        }
        case WM_DESTROY: {
            jobject peer = (jobject)::GetWindowLongPtr(hdlg, DWLP_USER);
            JNIEnv *env = (JNIEnv *)JNU_GetEnv(jvm, JNI_VERSION_1_2);
            env->SetLongField(peer, AwtFileDialog::hwndID, 0L);
            break;
        }
        case WM_NOTIFY: {
            OFNOTIFYEX* notifyEx = (OFNOTIFYEX*)lParam;
            if (notifyEx && notifyEx->hdr.code == CDN_INCLUDEITEM) {
                jobject peer = (jobject)::GetWindowLongPtr(hdlg, DWLP_USER);
                JNIEnv *env = (JNIEnv *)JNU_GetEnv(jvm, JNI_VERSION_1_2);
                LPITEMIDLIST pidl = (LPITEMIDLIST)notifyEx->pidl;
                // Get the filename and directory
                char szPath[MAX_PATH];
                if (!get_path_from_idlist(pidl,szPath)) {
                    return TRUE;
                }
                jstring strPath = JNU_NewStringPlatform(env, szPath);
                // Call FilenameFilter.accept with path and filename
                UINT uRes = (env->CallBooleanMethod(peer,
                    AwtFileDialog::checkFilenameFilterMID, strPath) == JNI_TRUE);
                env->DeleteLocalRef(strPath);
                return uRes;
            }
        }
    }

    return FALSE;

    CATCH_BAD_ALLOC_RET(TRUE);
}

void
AwtFileDialog::Show(void *p)
{
    JNIEnv *env = (JNIEnv *)JNU_GetEnv(jvm, JNI_VERSION_1_2);
    jobject peer = (jobject)p;
    char *titleBuffer = NULL;
    WCHAR unicodeChar = L' ';
    char *directoryBuffer = NULL;
    char *fileBuffer = NULL;
    char *currentDirectory = NULL;
    OPENFILENAME ofn;
    jint mode = 0;
    BOOL result = FALSE;
    DWORD dlgerr;
    jstring directory = NULL;
    jstring title = NULL;
    jstring file = NULL;
    jobject fileFilter = NULL;
    jobject target = NULL;
    jobject parent = NULL;
    AwtComponent* awtParent = NULL;

    try {
        DASSERT(peer);
	target = env->GetObjectField(peer, AwtObject::targetID);
	parent = env->GetObjectField(peer, AwtFileDialog::parentID); 
	if (parent != NULL) {
	    awtParent = (AwtComponent *)JNI_GET_PDATA(parent);
	}
	DASSERT(awtParent);
	title = (jstring)(env)->GetObjectField(target, AwtDialog::titleID);

        if (title == NULL || env->GetStringLength(title)==0) {
            title = env->NewString(&unicodeChar, 1);
        }

	if (title != NULL) {  
	    int length = env->GetStringLength(title);
	    titleBuffer = new char[length * 2 + 1];

	    AwtFont::javaString2multibyte(env, title, titleBuffer, length);
	}

	directory = 
	    (jstring)env->GetObjectField(target, AwtFileDialog::dirID);
	if (directory != NULL) {
	    int length = env->GetStringLength(directory);
	    directoryBuffer = new char[length * 2 + 1];

	    AwtFont::javaString2multibyte(env, directory, directoryBuffer,
					  length);
	}

	fileBuffer = new char[MAX_PATH+1];

	file = (jstring)env->GetObjectField(target, AwtFileDialog::fileID);
	if (file != NULL) {
	    int length = env->GetStringLength(file);
	    AwtFont::javaString2multibyte(env, file, fileBuffer, length);
	} else {
	    fileBuffer[0] = '\0';
	}

	memset(&ofn, 0, sizeof(ofn));

	ofn.lStructSize = sizeof(ofn);
	ofn.lpstrFilter = s_fileFilterString;
	ofn.nFilterIndex = 1;
	ofn.hwndOwner = awtParent ? awtParent->GetHWnd() : NULL;
	ofn.lpstrFile = fileBuffer;
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrTitle = titleBuffer;
	ofn.lpstrInitialDir = directoryBuffer;
	ofn.Flags = OFN_LONGNAMES | OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY |
	            OFN_ENABLEHOOK | OFN_EXPLORER;
        fileFilter = env->GetObjectField(peer,
        AwtFileDialog::fileFilterID);
    if (!JNU_IsNull(env,fileFilter)) {
        ofn.Flags |= OFN_ENABLEINCLUDENOTIFY;
    }
	ofn.lCustData = (LPARAM)peer;
	ofn.lpfnHook = (LPOFNHOOKPROC)FileDialogHookProc;

	// Save current directory, so we can reset if it changes.
	currentDirectory = new char[MAX_PATH+1];

	VERIFY(::GetCurrentDirectory(MAX_PATH, currentDirectory) > 0);
	
	mode = env->GetIntField(target, AwtFileDialog::modeID);
    
	// disable all top-level windows
	AwtDialog::ModalDisable(NULL);

        // Fix for 4364256 : call load_shell_procs()
        load_shell_procs();

	// show the Win32 file dialog
	if (mode == java_awt_FileDialog_LOAD) {
	    result = AwtFileDialog::GetOpenFileName(&ofn);
	} else {
	    result = AwtFileDialog::GetSaveFileName(&ofn);
	}
        // Fix for 4181310: FileDialog does not show up.
        // If the dialog is not shown because of invalid file name
        // replace the file name by empty string.
        if (!result) {
            dlgerr = AwtCommDialog::CommDlgExtendedError();
            if (dlgerr == FNERR_INVALIDFILENAME) {
                strcpy(fileBuffer, "");
                if (mode == java_awt_FileDialog_LOAD) {
                    result = AwtFileDialog::GetOpenFileName(&ofn);
                } else {
                    result = AwtFileDialog::GetSaveFileName(&ofn);
                }
            }
        }

	DASSERT(env->GetLongField(peer, AwtFileDialog::hwndID) == 0L);

	// re-enable top-level windows
	AwtDialog::ModalEnable(NULL);
	AwtDialog::ModalNextWindowToFront(ofn.hwndOwner);

	VERIFY(::SetCurrentDirectory(currentDirectory));

	// Report result to peer.
	if (result) {
	    env->CallVoidMethod(peer, AwtFileDialog::handleSelectedMID,
			    AwtFont::multibyte2javaString(env,ofn.lpstrFile)); 
	} else {
	    env->CallVoidMethod(peer, AwtFileDialog::handleCancelMID);
	}
	DASSERT(!safe_ExceptionOccurred(env));
    } catch (...) {

        env->DeleteLocalRef(target);
        env->DeleteLocalRef(parent);
        env->DeleteLocalRef(title);
        env->DeleteLocalRef(directory);
        env->DeleteLocalRef(file);
        env->DeleteLocalRef(fileFilter);

        delete[] currentDirectory;
	delete[] fileBuffer;
	delete[] directoryBuffer;
        delete[] titleBuffer;
	throw;
    }

    env->DeleteLocalRef(target);
    env->DeleteLocalRef(parent);
    env->DeleteLocalRef(title);
    env->DeleteLocalRef(directory);
    env->DeleteLocalRef(file);
    env->DeleteLocalRef(fileFilter);

    delete[] currentDirectory;
    delete[] fileBuffer;
    delete[] directoryBuffer;
    delete[] titleBuffer;
}

BOOL
AwtFileDialog::GetOpenFileName(LPOPENFILENAMEA data) {
    AwtCommDialog::load_comdlg_procs();
    return static_cast<BOOL>(reinterpret_cast<INT_PTR>(
        AwtToolkit::GetInstance().InvokeFunction((void*(*)(void*))
                     AwtCommDialog::GetOpenFileNameWrapper, data)));
		       
}

BOOL
AwtFileDialog::GetSaveFileName(LPOPENFILENAMEA data) {
    AwtCommDialog::load_comdlg_procs();
    return static_cast<BOOL>(reinterpret_cast<INT_PTR>(
        AwtToolkit::GetInstance().InvokeFunction((void *(*)(void *))
                     AwtCommDialog::GetSaveFileNameWrapper, data)));
		       
}

BOOL AwtFileDialog::InheritsNativeMouseWheelBehavior() {return true;}

/************************************************************************
 * WFileDialogPeer native methods
 */

extern "C" {

JNIEXPORT void JNICALL
Java_sun_awt_windows_WFileDialogPeer_initIDs(JNIEnv *env, jclass cls)
{
    TRY;

    AwtFileDialog::parentID = 
	env->GetFieldID(cls, "parent", "Lsun/awt/windows/WComponentPeer;");
    AwtFileDialog::hwndID =
        env->GetFieldID(cls, "hwnd", "J");
    AwtFileDialog::fileFilterID =
        env->GetFieldID(cls, "fileFilter", "Ljava/io/FilenameFilter;");
    AwtFileDialog::handleSelectedMID = 
	env->GetMethodID(cls, "handleSelected", "(Ljava/lang/String;)V");
    AwtFileDialog::handleCancelMID = 
	env->GetMethodID(cls, "handleCancel", "()V");
    AwtFileDialog::checkFilenameFilterMID = 
        env->GetMethodID(cls, "checkFilenameFilter", "(Ljava/lang/String;)Z");

    /* java.awt.FileDialog fields */
    cls = env->FindClass("java/awt/FileDialog");
    if (cls == NULL) {
	return;
    }
    AwtFileDialog::modeID = env->GetFieldID(cls, "mode", "I");
    AwtFileDialog::dirID = env->GetFieldID(cls, "dir", "Ljava/lang/String;");
    AwtFileDialog::fileID = env->GetFieldID(cls, "file", "Ljava/lang/String;");
    AwtFileDialog::filterID = 
	env->GetFieldID(cls, "filter", "Ljava/io/FilenameFilter;");

    DASSERT(AwtFileDialog::parentID != NULL);
    DASSERT(AwtFileDialog::hwndID != NULL);
    DASSERT(AwtFileDialog::handleSelectedMID != NULL);
    DASSERT(AwtFileDialog::handleCancelMID != NULL);

    DASSERT(AwtFileDialog::modeID != NULL);
    DASSERT(AwtFileDialog::dirID != NULL);
    DASSERT(AwtFileDialog::fileID != NULL);
    DASSERT(AwtFileDialog::filterID != NULL);

    CATCH_BAD_ALLOC;
}

JNIEXPORT void JNICALL
Java_sun_awt_windows_WFileDialogPeer_setFilterString(JNIEnv *env, jclass cls,
                                                     jstring filterDescription)
{
    TRY;

    AwtFileDialog::Initialize(env, filterDescription);

    CATCH_BAD_ALLOC;
}

JNIEXPORT void JNICALL
Java_sun_awt_windows_WFileDialogPeer__1show(JNIEnv *env, jobject peer)
{
    TRY;

    AwtToolkit::GetInstance().InvokeFunction(AwtFileDialog::Show, peer);

    CATCH_BAD_ALLOC;
}

JNIEXPORT void JNICALL
Java_sun_awt_windows_WFileDialogPeer__1dispose(JNIEnv *env, jobject peer)
{
    TRY_NO_VERIFY;

    HWND hdlg = (HWND)(env->GetLongField(peer, AwtFileDialog::hwndID));
    if (hdlg != NULL) {
        ::SendMessage(hdlg, WM_COMMAND, MAKEWPARAM(IDCANCEL, 0),
	             (LPARAM)hdlg);
    }

    CATCH_BAD_ALLOC;
}

JNIEXPORT void JNICALL
Java_sun_awt_windows_WFileDialogPeer__1hide(JNIEnv *env, jobject peer)
{
    TRY;

    HWND hdlg = (HWND)(env->GetLongField(peer, AwtFileDialog::hwndID));
    if (hdlg != NULL) {
        ::SendMessage(hdlg, WM_COMMAND, MAKEWPARAM(IDCANCEL, 0),
	             (LPARAM)hdlg);
    }

    CATCH_BAD_ALLOC;
}

} /* extern "C" */
