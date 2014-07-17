/*
 * @(#)browser.cpp	1.5 03/01/23
 * 
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include <jni.h>
#include <windows.h>
#include <winreg.h>
#include <stdio.h>
#include <tchar.h>

#define TEMP_FILE_NAME "jawshtml.html"

/* Reference to browserExecutable */
static char* browserExecutable = NULL;

void inititalizeBrowser() {
  int begin, end;
  char dummy[MAX_PATH];
  char resultPath[MAX_PATH];
  char tempPath[MAX_PATH];
  HINSTANCE retval;

  /* Already inititalized? */
  if (browserExecutable != NULL) return;

  /* First create a known temp. HTML file */
  if (FAILED(GetTempPath(MAX_PATH, tempPath))) return;
  _tcscat(tempPath, TEMP_FILE_NAME);
  FILE* fp = fopen(tempPath, "w");
  fputs("<HTML></HTML>", fp);
  fclose(fp);

  dummy[0] = resultPath[0] = '\0';
  retval = FindExecutable(tempPath, dummy, resultPath);

  /* Trim result */  
  if ((int)retval > 32) {
     begin = 0;
     end = strlen(resultPath);
     while(resultPath[begin] != 0 && resultPath[begin] == ' ') begin++;  
     while(end > begin && resultPath[end-1] == ' ') end--;
     if (end > begin) {
       resultPath[end] = '\0';
       browserExecutable = strdup(resultPath + begin);    
     }
  }    

  /* Remove tempoary file */
  remove(tempPath);
}



extern "C" {

JNIEXPORT jint JNICALL Java_com_sun_javaws_WinBrowserSupport_showDocument(JNIEnv *env, jclass, jstring url)
{
    char dummy[MAX_PATH];
    HINSTANCE retval;
    dummy[0] = '\0';

    const char *urlStr = env->GetStringUTFChars(url, NULL);

    inititalizeBrowser();
    if (browserExecutable == NULL) return JNI_FALSE;

    retval = ShellExecute(NULL, "open", browserExecutable, urlStr, dummy, SW_SHOWNORMAL);

    return ((int)retval > 32) ? JNI_TRUE : JNI_FALSE;
}

} // End extern "C"
