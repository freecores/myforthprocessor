/*
 * @(#)ShellFolder.cpp	1.26 03/02/18
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#define OEMRESOURCE

#ifdef DEBUG
// Warning : do not depend on anything in <awt.h>.  Including this file
// is a fix for 4507525 to use the same operator new and delete as AWT.
// This file should stand independent of AWT and should ultimately be
// put into its own DLL.
#include <awt.h>
#endif

#include <windows.h>
#include <shlobj.h>
#include <shellapi.h>
#include "jni_util.h"
#include "alloc.h"


// Shell Functions
typedef BOOL (WINAPI *DestroyIconType)(HICON);
typedef HINSTANCE (WINAPI *FindExecutableType)(LPCSTR,LPCSTR,LPSTR);
typedef BOOL (WINAPI *GetIconInfoType)(HICON,PICONINFO);
typedef HRESULT (WINAPI *SHGetDesktopFolderType)(IShellFolder**);
typedef DWORD* (WINAPI *SHGetFileInfoType)(LPCSTR,DWORD,SHFILEINFO*,UINT,UINT);
typedef HRESULT (WINAPI *SHGetMallocType)(IMalloc**);
typedef BOOL (WINAPI *SHGetPathFromIDListType)(LPCITEMIDLIST,LPSTR);
typedef HRESULT (WINAPI *SHGetSpecialFolderLocationType)(HWND,int,LPITEMIDLIST*);

static DestroyIconType fn_DestroyIcon;
static FindExecutableType fn_FindExecutable;
static GetIconInfoType fn_GetIconInfo;
static SHGetDesktopFolderType fn_SHGetDesktopFolder;
static SHGetFileInfoType fn_SHGetFileInfo;
static SHGetMallocType fn_SHGetMalloc;
static SHGetPathFromIDListType fn_SHGetPathFromIDList;
static SHGetSpecialFolderLocationType fn_SHGetSpecialFolderLocation;

// Field IDs
static jfieldID FID_pIShellFolder;
static jfieldID FID_relativePIDL;
static jfieldID FID_pIDL;
static jfieldID FID_attributes;
static jfieldID FID_displayName;
static jfieldID FID_folderType;

// Other statics
static IMalloc* pMalloc;

static BOOL isXP;

extern "C" {

static BOOL initShellProcs()
{
    static HMODULE libShell32 = NULL;
    static HMODULE libUser32 = NULL;
    // If already initialized, return TRUE
    if (libShell32 != NULL && libUser32 != NULL) {
        return TRUE;
    }
    // Load libraries
    libShell32 = LoadLibrary("shell32.dll");
    if (libShell32 == NULL) {
        return FALSE;
    }
    libUser32 = LoadLibrary("user32.dll");
    if (libUser32 == NULL) {
        return FALSE;
    }
    // Set up procs - libShell32
	fn_FindExecutable = (FindExecutableType)GetProcAddress(libShell32,
		"FindExecutableA");
    if (fn_FindExecutable == NULL) {
        return FALSE;
    }
	fn_SHGetDesktopFolder = (SHGetDesktopFolderType)GetProcAddress(libShell32,
		"SHGetDesktopFolder");
    if (fn_SHGetDesktopFolder == NULL) {
        return FALSE;
    }
	fn_SHGetFileInfo = (SHGetFileInfoType)GetProcAddress(libShell32,
        "SHGetFileInfoA");
    if (fn_SHGetFileInfo == NULL) {
        return FALSE;
    }
	fn_SHGetMalloc = (SHGetMallocType)GetProcAddress(libShell32,
        "SHGetMalloc");
    if (fn_SHGetMalloc == NULL) {
        return FALSE;
    }
    // Set up IMalloc
    if (fn_SHGetMalloc(&pMalloc) != S_OK) {
        return FALSE;
    }
	fn_SHGetPathFromIDList = (SHGetPathFromIDListType)GetProcAddress(
        libShell32, "SHGetPathFromIDListA");
    if (fn_SHGetPathFromIDList == NULL) {
        return FALSE;
    }
	fn_SHGetSpecialFolderLocation = (SHGetSpecialFolderLocationType)
        GetProcAddress(libShell32, "SHGetSpecialFolderLocation");
    if (fn_SHGetSpecialFolderLocation == NULL) {
        return FALSE;
    }
    // Set up procs - libUser32
    fn_GetIconInfo = (GetIconInfoType)GetProcAddress(libUser32, "GetIconInfo");
    if (fn_GetIconInfo == NULL) {
        return FALSE;
    }
    fn_DestroyIcon = (DestroyIconType)GetProcAddress(libUser32, "DestroyIcon");
    if (fn_DestroyIcon == NULL) {
        return FALSE;
    }
    return TRUE;
}

/*
 * Class:     sun_awt_shell_Win32ShellFolder
 * Method:    initIDs
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_sun_awt_shell_Win32ShellFolder_initIDs
    (JNIEnv* env, jclass cls)
{
    if (!initShellProcs()) {
        JNU_ThrowInternalError(env, "Could not initialize shell library");
        return;
    }
    FID_pIShellFolder = env->GetFieldID(cls, "pIShellFolder", "J");
    FID_relativePIDL = env->GetFieldID(cls, "relativePIDL", "J");
    FID_pIDL = env->GetFieldID(cls, "pIDL", "J");
    FID_attributes = env->GetFieldID(cls, "attributes", "J");
    FID_displayName = env->GetFieldID(cls, "displayName", "Ljava/lang/String;");
    FID_folderType = env->GetFieldID(cls, "folderType", "Ljava/lang/String;");

    // Find out if we are on XP or later
    long version = GetVersion();
    isXP = (!(version & 0x80000000) && \
	    LOBYTE(LOWORD(version)) >= 5 && \
	    HIBYTE(LOWORD(version)) >= 1);
}

/*
 * Class:     sun_awt_shell_Win32ShellFolder
 * Method:    initDesktop
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_sun_awt_shell_Win32ShellFolder_initDesktop
    (JNIEnv* env, jobject desktop)
{
    IShellFolder* pDesktop;
    // Get desktop IShellFolder
    HRESULT res = fn_SHGetDesktopFolder(&pDesktop);
    if (res != S_OK) {
        JNU_ThrowInternalError(env, "Could not get desktop shell folder");
        return;
    }
    // Set field ID for pIShellFolder
    env->SetLongField(desktop, FID_pIShellFolder, (jlong)pDesktop);
    // Get desktop relative PIDL
    LPITEMIDLIST relPIDL;
    res = fn_SHGetSpecialFolderLocation(NULL, CSIDL_DESKTOP, &relPIDL);
    if (res != S_OK) {
        JNU_ThrowInternalError(env,
            "Could not get desktop shell folder ID list");
        return;
    }
    // Set field ID for relative PIDL
    env->SetLongField(desktop, FID_relativePIDL, (jlong)relPIDL);
}

/*
 * Class:     sun_awt_shell_Win32ShellFolder
 * Method:    initSpecial
 * Signature: (JI)V
 */
JNIEXPORT void JNICALL Java_sun_awt_shell_Win32ShellFolder_initSpecial
    (JNIEnv* env, jobject folder, jlong desktopIShellFolder, jint folderType)
{
    // Get desktop IShellFolder interface
    IShellFolder* pDesktop = (IShellFolder*)desktopIShellFolder;
    if (pDesktop == NULL) {
        JNU_ThrowInternalError(env, "Desktop shell folder missing");
        return;
    }
    // Get special folder relative PIDL
    LPITEMIDLIST relPIDL;
    HRESULT res = fn_SHGetSpecialFolderLocation(NULL, folderType,
        &relPIDL);
    if (res != S_OK) {
        JNU_ThrowIOException(env, "Could not get shell folder ID list");
        return;
    }
    // Set field ID for relative PIDL
    env->SetLongField(folder, FID_relativePIDL, (jlong)relPIDL);
    // Get special folder IShellFolder interface
    IShellFolder* pFolder;
    res = pDesktop->BindToObject(relPIDL, NULL, IID_IShellFolder,
        (void**)&pFolder);
    if (res != S_OK) {
        JNU_ThrowInternalError(env,
            "Could not bind shell folder to interface");
        return;
    }
    // Set field ID for pIShellFolder
    env->SetLongField(folder, FID_pIShellFolder, (jlong)pFolder);
}

/*
 * Class:     sun_awt_shell_Win32ShellFolder
 * Method:    initFile
 * Signature: (JLjava/lang/String;Z)J
 */
JNIEXPORT jlong JNICALL Java_sun_awt_shell_Win32ShellFolder_initFile
    (JNIEnv* env, jobject folder, jlong desktopIShellFolder,
    jstring absolutePath, jboolean initAttributes)
{
    // Get desktop IShellFolder interface
    IShellFolder* pDesktop = (IShellFolder*)desktopIShellFolder;
    if (pDesktop == NULL) {
        JNU_ThrowInternalError(env, "Desktop shell folder missing");
        return 0;
    }
    // Get PIDL for file from desktop
    LPITEMIDLIST pIDL;
    int nLength = env->GetStringLength(absolutePath);
    jchar* wszPath = new jchar[nLength + 1];
    const jchar* strPath = env->GetStringChars(absolutePath, NULL);
    wcsncpy(wszPath, strPath, nLength);
    wszPath[nLength] = 0;
    HRESULT res = pDesktop->ParseDisplayName(NULL, NULL,
        const_cast<jchar*>(wszPath), NULL, &pIDL, NULL);
    if (res != S_OK) {
        JNU_ThrowIOException(env, "Could not parse file path");
        delete[] wszPath;
        env->ReleaseStringChars(absolutePath, strPath);
        return 0;
    }
    delete[] wszPath;
    env->ReleaseStringChars(absolutePath, strPath);

    // Get display name, folder type, and possibly attributes
    SHFILEINFO fileInfo;
    UINT flags;

    if (initAttributes) {
	flags = SHGFI_ATTRIBUTES | SHGFI_DISPLAYNAME | SHGFI_TYPENAME | SHGFI_PIDL;
    } else {
	flags = SHGFI_DISPLAYNAME | SHGFI_TYPENAME | SHGFI_PIDL;
    }

    if (fn_SHGetFileInfo((char *)pIDL, 0L, &fileInfo, sizeof(fileInfo), flags) != 0) {

	if (initAttributes) {
	    env->SetLongField(folder, FID_attributes, (jlong)fileInfo.dwAttributes);
	}

	env->SetObjectField(folder, FID_displayName,
			    JNU_NewStringPlatform(env, fileInfo.szDisplayName));

	env->SetObjectField(folder, FID_folderType,
			    JNU_NewStringPlatform(env, fileInfo.szTypeName));
    }
    return (jlong)pIDL;
}

/*
 * Class:     sun_awt_shell_Win32ShellFolder
 * Method:    releaseIShellFolder
 * Signature: (J)V
 */
JNIEXPORT void JNICALL Java_sun_awt_shell_Win32ShellFolder_releaseIShellFolder
    (JNIEnv* env, jobject folder, jlong pIShellFolder)
{
    if (pIShellFolder != 0L) {
        ((IShellFolder*)pIShellFolder)->Release();
    }
}

/*
 * Class:     sun_awt_shell_Win32ShellFolder
 * Method:    releasePIDL
 * Signature: (J)V
 */
JNIEXPORT void JNICALL Java_sun_awt_shell_Win32ShellFolder_releasePIDL
    (JNIEnv* env, jobject folder, jlong pIDL)
{
    if (pIDL != 0L) {
        pMalloc->Free((LPITEMIDLIST)pIDL);
    }
}


/*
 * Class:     sun_awt_shell_Win32ShellFolder
 * Method:    initAttributes
 * Signature: (J)V
 */
JNIEXPORT void JNICALL Java_sun_awt_shell_Win32ShellFolder_initAttributes
    (JNIEnv* env, jobject folder, jlong pIDL)
{
    SHFILEINFO fileInfo;
    if (fn_SHGetFileInfo((char *)pIDL, 0L, &fileInfo, sizeof(fileInfo),
	SHGFI_ATTRIBUTES | SHGFI_PIDL) != 0) {

	env->SetLongField(folder, FID_attributes, (jlong)fileInfo.dwAttributes);
    }
}

/*
 * Class:     sun_awt_shell_Win32ShellFolder
 * Method:    hasAttribute
 * Signature: (JJI)Z
 */
JNIEXPORT jboolean JNICALL Java_sun_awt_shell_Win32ShellFolder_hasAttribute__JJI
    (JNIEnv* env, jobject folder, jlong parentIShellFolder, jlong relativePIDL,
    jint dwAttribute)
{
    IShellFolder* pParent = (IShellFolder*)parentIShellFolder;
    if (pParent == NULL) {
        return false;
    }
    LPITEMIDLIST pidl = (LPITEMIDLIST)relativePIDL;
    if (pidl == NULL) {
        return false;
    }
    ULONG attrib = (ULONG)dwAttribute;
    pParent->GetAttributesOf(1, const_cast<LPCITEMIDLIST*>(&pidl), &attrib);
    return ((dwAttribute & attrib) != 0);
}

static jstring jstringFromSTRRET(JNIEnv* env, LPITEMIDLIST pidl, STRRET* pStrret) {
    switch (pStrret->uType) {
        case STRRET_CSTR :
            return JNU_NewStringPlatform(env, pStrret->cStr);
        case STRRET_OFFSET :
            // Note : this may need to be WCHAR instead
            return JNU_NewStringPlatform(env,
					 ((CHAR*)pidl + pStrret->uOffset));
        case STRRET_WSTR :
            return env->NewString(pStrret->pOleStr,
                static_cast<jsize>(wcslen(pStrret->pOleStr)));
    }
    return NULL;
}

/*
 * Class:     sun_awt_shell_Win32ShellFolder
 * Method:    getFileSystemPath
 * Signature: (JJ)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_sun_awt_shell_Win32ShellFolder_getFileSystemPath__JJ
    (JNIEnv* env, jclass cls, jlong parentIShellFolder, jlong relativePIDL)
{
    IShellFolder* pParent = (IShellFolder*)parentIShellFolder;
    if (pParent == NULL) {
        return NULL;
    }
    LPITEMIDLIST pidl = (LPITEMIDLIST)relativePIDL;
    if (pidl == NULL) {
        return NULL;
    }
    STRRET strret;
    pParent->GetDisplayNameOf(pidl, SHGDN_NORMAL | SHGDN_FORPARSING, &strret);
    return jstringFromSTRRET(env, pidl, &strret);
}

/*
 * Class:     sun_awt_shell_Win32ShellFolder
 * Method:    getFileSystemPath
 * Signature: (I)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_sun_awt_shell_Win32ShellFolder_getFileSystemPath__I
    (JNIEnv* env, jclass cls, jint csidl)
{
    LPITEMIDLIST relPIDL;
    CHAR szBuf[MAX_PATH];
    HRESULT res = fn_SHGetSpecialFolderLocation(NULL, csidl, &relPIDL);
    if (res != S_OK) {
        JNU_ThrowIOException(env, "Could not get shell folder ID list");
        return NULL;
    }
    if (fn_SHGetPathFromIDList(relPIDL, szBuf)) {
	return JNU_NewStringPlatform(env, szBuf);
    } else {
	return NULL;
    }
}

/*
 * Class:     sun_awt_shell_Win32ShellFolder
 * Method:    getEnumObjects
 * Signature: (JZ)J
 */
JNIEXPORT jlong JNICALL Java_sun_awt_shell_Win32ShellFolder_getEnumObjects
    (JNIEnv* env, jobject folder, jlong pIShellFolder,
     jboolean isDesktop, jboolean includeHiddenFiles)
{
    IShellFolder* pFolder = (IShellFolder*)pIShellFolder;
    if (pFolder == NULL) {
        return 0;
    }
    DWORD dwFlags = SHCONTF_FOLDERS | SHCONTF_NONFOLDERS;
    if (includeHiddenFiles) {
	dwFlags |= SHCONTF_INCLUDEHIDDEN;
    }
	/*
    if (!isDesktop) {
        dwFlags = dwFlags | SHCONTF_NONFOLDERS;
    }
	*/
    IEnumIDList* pEnum;
    if (pFolder->EnumObjects(NULL, dwFlags, &pEnum) != S_OK) {
        return 0;
    }
    return (jlong)pEnum;
}

/*
 * Class:     sun_awt_shell_Win32ShellFolder
 * Method:    getNextChild
 * Signature: (J)J
 */
JNIEXPORT jlong JNICALL Java_sun_awt_shell_Win32ShellFolder_getNextChild
    (JNIEnv* env, jobject folder, jlong pEnumObjects)
{
    IEnumIDList* pEnum = (IEnumIDList*)pEnumObjects;
    if (pEnum == NULL) {
        return 0;
    }
    LPITEMIDLIST pidl;
    if (pEnum->Next(1, &pidl, NULL) != S_OK) {
        return 0;
    }
    return (jlong)pidl;
}

/*
 * Class:     sun_awt_shell_Win32ShellFolder
 * Method:    releaseEnumObjects
 * Signature: (J)V
 */
JNIEXPORT void JNICALL Java_sun_awt_shell_Win32ShellFolder_releaseEnumObjects
    (JNIEnv* env, jobject folder, jlong pEnumObjects)
{
    IEnumIDList* pEnum = (IEnumIDList*)pEnumObjects;
    if (pEnum == NULL) {
        return;
    }
    pEnum->Release();
}

/*
 * Class:     sun_awt_shell_Win32ShellFolder
 * Method:    bindToObject
 * Signature: (JJ)J
 */
JNIEXPORT jlong JNICALL Java_sun_awt_shell_Win32ShellFolder_bindToObject
    (JNIEnv* env, jobject folder, jlong parentIShellFolder, jlong relativePIDL)
{
    IShellFolder* pParent = (IShellFolder*)parentIShellFolder;
    if (pParent == NULL) {
        return 0;
    }
    LPITEMIDLIST pidl = (LPITEMIDLIST)relativePIDL;
    if (pidl == NULL) {
        return 0;
    }
    IShellFolder* pFolder;
    if (pParent->BindToObject(pidl, NULL, IID_IShellFolder, (void**)&pFolder)
        != S_OK)
    {
        return 0;
    }
    return (jlong)pFolder;
}


/*
 * Class:     sun_awt_shell_Win32ShellFolder
 * Method:    getLinkLocation
 * Signature: (JJ)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_sun_awt_shell_Win32ShellFolder_getLinkLocation
    (JNIEnv* env, jobject folder, jlong parentIShellFolder, jlong relativePIDL)
{
    HRESULT hres;
    IShellLink* psl;
    char szGotPath[MAX_PATH];
    WIN32_FIND_DATA wfd;
    CHAR szBuf[MAX_PATH];
    STRRET strret;
    OLECHAR olePath[MAX_PATH]; // wide-char version of path name
    LPWSTR wstr;

    IShellFolder* pParent = (IShellFolder*)parentIShellFolder;
    if (pParent == NULL) {
        return NULL;
    }

    LPITEMIDLIST pidl = (LPITEMIDLIST)relativePIDL;
    if (pidl == NULL) {
        return NULL;
    }

    pParent->GetDisplayNameOf(pidl, SHGDN_NORMAL | SHGDN_FORPARSING, &strret);

    switch (strret.uType) {
      case STRRET_CSTR :
	// IShellFolder::ParseDisplayName requires the path name in Unicode.
	MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, strret.cStr, -1, olePath, MAX_PATH);
	wstr = olePath;
	break;

      case STRRET_OFFSET :
	MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, (CHAR *)(pidl + strret.uOffset), -1, olePath, MAX_PATH);
	wstr = olePath;
	break;

      case STRRET_WSTR :
	wstr = strret.pOleStr;
	break;
    }

    CoInitialize(NULL);
    hres = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (LPVOID *) &psl);
    if (SUCCEEDED(hres)) {
	IPersistFile* ppf;
	hres = psl->QueryInterface(IID_IPersistFile, (void**)&ppf);
	if (SUCCEEDED(hres)) {
	    hres = ppf->Load(wstr, STGM_READ);
	    if (SUCCEEDED(hres)) {
		//hres = psl->Resolve(NULL, SLR_NO_UI);
		hres = psl->Resolve(NULL, 0);
		if (SUCCEEDED(hres)) {
		    hres = psl->GetPath(szGotPath, MAX_PATH, (WIN32_FIND_DATA *)&wfd, SLGP_SHORTPATH);
		    if (SUCCEEDED(hres)) {
			lstrcpy(szBuf, szGotPath);
		    }
		}
	    }
	    ppf->Release();
	}
	psl->Release();
    }
    CoUninitialize();

    if (SUCCEEDED(hres)) {
	return JNU_NewStringPlatform(env, szBuf);
    } else {
	return NULL;
    }
}

/*
 * Class:     sun_awt_shell_Win32ShellFolder
 * Method:    getDisplayName
 * Signature: (JJ)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_sun_awt_shell_Win32ShellFolder_getDisplayName__JJ
    (JNIEnv* env, jobject folder, jlong parentIShellFolder, jlong relativePIDL)
{
    IShellFolder* pParent = (IShellFolder*)parentIShellFolder;
    if (pParent == NULL) {
        return NULL;
    }
    LPITEMIDLIST pidl = (LPITEMIDLIST)relativePIDL;
    if (pidl == NULL) {
        return NULL;
    }
    STRRET strret;
    if (pParent->GetDisplayNameOf(pidl, SHGDN_NORMAL, &strret) != S_OK) {
        return NULL;
    }
    return jstringFromSTRRET(env, pidl, &strret);
}

/*
 * Class:     sun_awt_shell_Win32ShellFolder
 * Method:    getFolderType
 * Signature: (J)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_sun_awt_shell_Win32ShellFolder_getFolderType
    (JNIEnv* env, jobject folder, jlong pIDL)
{
    SHFILEINFO fileInfo;
    if (fn_SHGetFileInfo((CHAR*)pIDL, 0L, &fileInfo, sizeof(fileInfo),
        SHGFI_TYPENAME | SHGFI_PIDL) == 0) {
        return NULL;
    }
    return JNU_NewStringPlatform(env, fileInfo.szTypeName);
}

/*
 * Class:     sun_awt_shell_Win32ShellFolder
 * Method:    getExecutableType
 * Signature: (Ljava/lang/String;)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_sun_awt_shell_Win32ShellFolder_getExecutableType
    (JNIEnv* env, jobject folder, jbyteArray path_bytes)
{
    CHAR szBuf[MAX_PATH];
    jboolean dummy;
    LPCSTR szPath = (LPCSTR)env->GetByteArrayElements(path_bytes, &dummy);
    if (szPath == NULL) {
	return NULL;
    }
    HINSTANCE res = fn_FindExecutable(szPath, szPath, szBuf);
    env->ReleaseByteArrayElements(path_bytes, (jbyte *)szPath, JNI_ABORT);
    if ((UINT_PTR)res < 32) {
        return NULL;
    }
    return JNU_NewStringPlatform(env, szBuf);
}

/*
 * Class:     sun_awt_shell_Win32ShellFolder
 * Method:    getIcon
 * Signature: (JZ)J
 */
JNIEXPORT jlong JNICALL Java_sun_awt_shell_Win32ShellFolder_getIcon__JZ
    (JNIEnv* env, jobject folder, jlong pIDL, jboolean getLargeIcon)
{
    SHFILEINFO fileInfo;
    if (fn_SHGetFileInfo((CHAR*)pIDL, 0L, &fileInfo, sizeof(fileInfo),
        SHGFI_ICON | (getLargeIcon ? 0 : SHGFI_SMALLICON) | SHGFI_PIDL) == 0) {
        return 0;
    }
    return (jlong)fileInfo.hIcon;
}

/*
 * Class:     sun_awt_shell_Win32ShellFolder
 * Method:    getIcon
 * Signature: (JJZ)J
 */
JNIEXPORT jlong JNICALL Java_sun_awt_shell_Win32ShellFolder_getIcon__JJZ
    (JNIEnv* env, jobject folder, jlong parentIShellFolder, jlong relativePIDL,
     jboolean getLargeIcon)
{
    IShellFolder* pParent = (IShellFolder*)parentIShellFolder;
    if (pParent == NULL) {
        return 0;
    }
    LPITEMIDLIST pidl = (LPITEMIDLIST)relativePIDL;
    if (pidl == NULL) {
        return 0;
    }
    IExtractIcon* pIcon;
    if (pParent->GetUIObjectOf(NULL, 1, const_cast<LPCITEMIDLIST*>(&pidl),
        IID_IExtractIcon, NULL, (void**)&pIcon) != S_OK) {
        return 0;
    }
    CHAR szBuf[MAX_PATH];
    INT index;
    UINT flags;
    if (pIcon->GetIconLocation(
        GIL_FORSHELL, szBuf, MAX_PATH, &index, &flags)
        != S_OK) {
        pIcon->Release();
        return 0;
    }
    HICON hIcon;
    HICON hIconLarge;
    if (pIcon->Extract(szBuf, index, &hIconLarge, &hIcon, (16 << 16) + 32) != S_OK) {
        pIcon->Release();
        return 0;
    }
    return (jlong)(getLargeIcon ? hIconLarge : hIcon);
}

/*
 * Class:     sun_awt_shell_Win32ShellFolder
 * Method:    disposeIcon
 * Signature: (J)V
 */
JNIEXPORT void JNICALL Java_sun_awt_shell_Win32ShellFolder_disposeIcon
    (JNIEnv* env, jobject folder, jlong hicon)
{
    fn_DestroyIcon((HICON)hicon);
}

/*
 * Class:     sun_awt_shell_Win32ShellFolder
 * Method:    getIconBits
 * Signature: (JI)[I
 */
JNIEXPORT jintArray JNICALL Java_sun_awt_shell_Win32ShellFolder_getIconBits
    (JNIEnv* env, jobject folder, jlong hicon, jint iconSize)
{
    // Get the icon info
    ICONINFO iconInfo;
    if (!fn_GetIconInfo((HICON)hicon, &iconInfo)) {
        return NULL;
    }
    // Get the screen DC
    HDC dc = GetDC(NULL);
    if (dc == NULL) {
        return NULL;
    }
    // Set up BITMAPINFO
    BITMAPINFO bmi;
    memset(&bmi, 0, sizeof(BITMAPINFO));
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = iconSize;
    bmi.bmiHeader.biHeight = -iconSize;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;
    // Extract the color bitmap
    int nBits = iconSize * iconSize;
    long colorBits[1024];
    GetDIBits(dc, iconInfo.hbmColor, 0, iconSize, colorBits, &bmi, DIB_RGB_COLORS);
    // XP supports alpha in some icons, and depending on device. 
    // This should take precedence over the icon mask bits.
    BOOL hasAlpha = FALSE;
    if (isXP) {
	for (int i = 0; i < nBits; i++) {
	    if ((colorBits[i] & 0xff000000) != 0) {
		hasAlpha = TRUE;
		break;
	    }
	}
    }
    if (!hasAlpha) {
	// Extract the mask bitmap
	long maskBits[1024];
	GetDIBits(dc, iconInfo.hbmMask, 0, iconSize, maskBits, &bmi, DIB_RGB_COLORS);
	// Copy the mask alphas into the color bits
	for (int i = 0; i < nBits; i++) {
	    if (maskBits[i] == 0) {
		colorBits[i] |= 0xff000000;
	    }
	}
    }
    // Release DC
    ReleaseDC(NULL, dc);
    // Create java array
    jintArray iconBits = env->NewIntArray(nBits);
    // Copy values to java array
    env->SetIntArrayRegion(iconBits, 0, nBits, colorBits);
    // Fix 4745575 GDI Resource Leak
    // MSDN 
    // GetIconInfo creates bitmaps for the hbmMask and hbmColor members of ICONINFO. 
    // The calling application must manage these bitmaps and delete them when they 
    // are no longer necessary. 
    ::DeleteObject(iconInfo.hbmColor);
    ::DeleteObject(iconInfo.hbmMask);    
    return iconBits;
}

/*
 * Class:     sun_awt_shell_Win32ShellFolder
 * Method:    getFileChooserBitmapBits
 * Signature: ()[I
 */
JNIEXPORT jintArray JNICALL Java_sun_awt_shell_Win32ShellFolder_getFileChooserBitmapBits
    (JNIEnv* env, jclass cls)
{
    HBITMAP hBitmap = NULL;
    BITMAP bm;
    HINSTANCE libComCtl32;
    HINSTANCE libShell32;


    libShell32 = LoadLibrary("shell32.dll");
    if (libShell32 != NULL) {
	hBitmap = (HBITMAP)LoadImage(libShell32, MAKEINTRESOURCE(216),
				     IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);
    }
    if (hBitmap == NULL) {
	libComCtl32 = LoadLibrary("comctl32.dll");
	if (libComCtl32 != NULL) {
	    hBitmap = (HBITMAP)LoadImage(libComCtl32, MAKEINTRESOURCE(124),
					 IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);
	}
    }
    if (hBitmap == NULL) {
	return NULL;
    }

    GetObject(hBitmap, sizeof(bm), (LPSTR)&bm);

    // Get the screen DC
    HDC dc = GetDC(NULL);
    if (dc == NULL) {
        return NULL;
    }

    // Set up BITMAPINFO
    BITMAPINFO bmi;
    memset(&bmi, 0, sizeof(BITMAPINFO));
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = bm.bmWidth;
    bmi.bmiHeader.biHeight = -bm.bmHeight;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;
    // Extract the color bitmap
    int numPixels = bm.bmWidth * bm.bmHeight;
    //long colorBits[192 * 16];
    long *colorBits = (long*)safe_Malloc(numPixels * sizeof(long));
    if (GetDIBits(dc, hBitmap, 0, bm.bmHeight, colorBits, &bmi, DIB_RGB_COLORS) == 0) {
        return NULL;
    }

    // Release DC
    ReleaseDC(NULL, dc);

    // The color of the first pixel defines the transparency, according
    // to the documentation for LR_LOADTRANSPARENT at
    // http://msdn.microsoft.com/library/psdk/winui/resource_9fhi.htm
    long transparent = colorBits[0];
    for (int i=0; i < numPixels; i++) {
	if (colorBits[i] != transparent) {
	    colorBits[i] |= 0xff000000;
	}
    }

    // Create java array
    jintArray bits = env->NewIntArray(numPixels);
    // Copy values to java array
    env->SetIntArrayRegion(bits, 0, numPixels, colorBits);
    // Fix 4745575 GDI Resource Leak
    ::DeleteObject(hBitmap);
    ::FreeLibrary(libComCtl32);
    return bits;
}

} // extern "C"
