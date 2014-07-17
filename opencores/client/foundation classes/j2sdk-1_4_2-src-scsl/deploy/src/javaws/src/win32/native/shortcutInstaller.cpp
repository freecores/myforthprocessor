/*
 * @(#)shortcutInstaller.cpp	1.19 03/04/30
 * 
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

// This exposes some simple functions for adding shortcuts on the desktop
// and start menu.
// In Windows shortcuts are manipulated through the COM interfaces
// IShellLink and IPersistFile. In the end, they stored as binary files.
// The locations for these files can be found in the registry under:
// [HKEY_CURRENT_USER]\Software\Microsoft\Windows\CurrentVersion\Explorer\Shell Folders
// and the value for key 'Desktop' giving the path to desktop shortcuts,
// and the value for key 'Start Menu' giving the path to the users start menu
// shortcuts.
// It should be noted that IPersistFile does not expose methods for
// removing the link, instead the usual functions for remove a file are
// used.

#include <jni.h>
#include <stdio.h>
#include <windows.h>
#include <shlobj.h>
#include <sys/types.h>
#include <sys/stat.h>

char* javawsWideCharToMBCS(const unsigned short* uString) {
  char * mbcs = (char*)malloc(sizeof(char)*MAX_PATH);
  ZeroMemory(mbcs, sizeof(char)*MAX_PATH);
  WideCharToMultiByte(CP_ACP, 0, uString, -1, mbcs, MAX_PATH, NULL, NULL);
  return mbcs;
}

// type indicates the type of shortcut and should be either:
//   0 for a desktop shortcut
//   1 for a 'start menu' shortcut
// Returns 0 on success, else failure
int installShortcut(const WORD *shortcutPath, const unsigned short *shortcutNameU,
		    const unsigned short *descriptionU, const unsigned short *pathU,
		    const unsigned short *argumentsU, const unsigned short *workingDirectoryU,
		    const unsigned short *iconPathU) {
    char *shortcutName = javawsWideCharToMBCS(shortcutNameU);
    char *description = javawsWideCharToMBCS(descriptionU);
    char *path = javawsWideCharToMBCS(pathU);
    char *arguments = javawsWideCharToMBCS(argumentsU);
    char *workingDirectory = javawsWideCharToMBCS(workingDirectoryU);
    char *iconPath = javawsWideCharToMBCS(iconPathU);
 
    // Initialize COM, stash the result to know if we need to call
    // CoUnintialize
    HRESULT comStart = CoInitialize(NULL);

    HRESULT tempResult;
    IShellLink *shell;
    
    int retValue = 0;
   
    // Find IShellLink interface.
    tempResult = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER,
				  IID_IShellLink, (void **)&shell);

    if (SUCCEEDED(tempResult)) {
	IPersistFile* persistFile;

	// Query IShellLink for the IPersistFile interface for 
	// saving the shell link in persistent storage.
	tempResult = shell->QueryInterface(IID_IPersistFile,
					   (void **)&persistFile);
	if (SUCCEEDED(tempResult)) {	   

	    // Set the path to the shell link target.
	    tempResult = shell->SetPath(path);

	    if (!SUCCEEDED(tempResult)) {
		// Couldn't set the path
		retValue = -2;
	    }
	 
	    // Set the description of the shell link.
	    // fix for 4499382
	    // make sure description length is less than MAX_PATH
	    // else truncate the string before setting description
	    if (retValue == 0 && description != NULL && 
		strlen(description) < MAX_PATH &&
		!SUCCEEDED(shell->SetDescription(description))) {
	      retValue = -3;
	    } else {
	      char *desc = (char*)malloc(sizeof(char)*MAX_PATH);
	      desc = strncpy(desc, description, MAX_PATH - 1);
	      if (!SUCCEEDED(shell->SetDescription(desc))) {
		retValue = -3;
	      }
	    }
	  
	    // Set the arguments
	    if (retValue == 0 && arguments != NULL &&
		!(SUCCEEDED(shell->SetArguments(arguments)))) {
		retValue = -4;
	    }	   

	    // Working directory
	    if (retValue == 0 && workingDirectory != NULL &&
		!(SUCCEEDED(shell->SetWorkingDirectory(workingDirectory)))) {
		retValue = -5;
	    }

	    // Sets the icon location, default to an icon index of 0.
	    if (retValue == 0 && iconPath != NULL &&
		!(SUCCEEDED(shell->SetIconLocation(iconPath, 0)))) {
		retValue = -6;
	    }
	    // PENDING: if iconPath == null, should install a link to
	    // the default icon!

	    // Defaults to a normal window.
	    if (retValue == 0) {
		shell->SetShowCmd(SW_NORMAL);	
		// Save the link via the IPersistFile::Save method.
		if (!SUCCEEDED(persistFile->Save(shortcutPath, TRUE))) {
		    retValue = -7;
		}
	    }
	    // Release pointer to IPersistFile.
	    persistFile->Release();
	}
	else {
	    // No persist file
	    retValue = -8;
	}
	// Release pointer to IShellLink.
	shell->Release();
    }
    else {
	// No shell!
	retValue = -9;
    }
    if (comStart == S_OK) {
	CoUninitialize();
    } 
    free(shortcutName);
    free(description);
    free(path);
    free(arguments);
    free(workingDirectory);
    free(iconPath);
    return retValue;
}

extern "C" {
//
// Install
//

JNIEXPORT jint JNICALL Java_com_sun_javaws_WinInstallHandler_installShortcut(JNIEnv *env, jobject wHandler, jstring pathS, jstring nameS, jstring descriptionS, jstring appPathS, jstring argsS, jstring directoryS, jstring iconPathS) 
{
    const WORD *path = (pathS != NULL) ? env->GetStringChars
	                (pathS, NULL) : NULL;
    const unsigned short *name = (nameS != NULL) ? env->GetStringChars
	                (nameS, NULL) : NULL;
    const unsigned short *description = (descriptionS != NULL) ?
	              env->GetStringChars(descriptionS, 0) : NULL;
    const unsigned short *appPath = (appPathS != NULL) ? env->GetStringChars
	                  (appPathS, 0) : NULL;
    const unsigned short *args = (argsS != NULL) ? env->GetStringChars
	               (argsS, 0) : NULL;
    const unsigned short *directory = (directoryS != NULL) ? env->GetStringChars
	                    (directoryS, 0) : NULL;
    const unsigned short *iconPath = (iconPathS != NULL) ? env->GetStringChars
	                   (iconPathS, 0) : NULL;
   

    jint iShortcut = -1;

    // Find the valid name
    if (name != NULL && path != NULL) {
	iShortcut = installShortcut(path, name, description,
					appPath, args, directory, iconPath);
	
	// Should log error somehow.
    }

    if (path != NULL) {
	env->ReleaseStringChars(pathS, path);
    }
    if (name != NULL) {
	env->ReleaseStringChars(nameS, name);
    }
    if (description != NULL) {
	env->ReleaseStringChars(descriptionS, description);
    }
    if (appPath != NULL) {
	env->ReleaseStringChars(appPathS, appPath);
    }
    if (args != NULL) {
	env->ReleaseStringChars(argsS, args);
    }
    if (directory != NULL) {
	env->ReleaseStringChars(directoryS, directory);
    }
    if (iconPath != NULL) {
	env->ReleaseStringChars(iconPathS, iconPath);
    }  
    return iShortcut;
}

}
