/*
 * @(#)awt_DesktopProperties.h	1.11 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef AWT_DESKTOP_PROPERTIES_H
#define AWT_DESKTOP_PROPERTIES_H

#include "awt.h"
#include "jni.h"

class AwtDesktopProperties {
    public:
	enum {
	    MAX_PROPERTIES = 100,
	    AWT_DESKTOP_PROPERTIES_1_3 = 1, // properties version for Java SDK 1.3
	    // NOTE: MUST INCREMENT this whenever you add new
	    // properties for a given public release
	    AWT_DESKTOP_PROPERTIES_1_4 = 2, // properties version for Java SDK 1.4
	    AWT_DESKTOP_PROPERTIES_VERSION = AWT_DESKTOP_PROPERTIES_1_4,
	};

	AwtDesktopProperties(jobject self);	
	~AwtDesktopProperties();
	
	void GetWindowsParameters();
	void PlayWindowsSound(const char * eventName);
        static BOOL IsXPStyle();

	static jfieldID pDataID;
	static jmethodID setStringPropertyID;
	static jmethodID setIntegerPropertyID;
	static jmethodID setBooleanPropertyID;
	static jmethodID setColorPropertyID;
	static jmethodID setFontPropertyID;
	static jmethodID setSoundPropertyID;

    private:
	void GetXPStyleProperties();
	void GetSystemProperties();
	void GetNonClientParameters();
	void GetIconParameters();
	void GetColorParameters();
	void GetOtherParameters();
	void GetSoundEvents();

	static BOOL GetBooleanParameter(UINT spi);

	void SetBooleanProperty(const char *, BOOL);
	void SetIntegerProperty(const char *, int);
	void SetStringProperty(const char *, char *);
	void SetColorProperty(const char *, DWORD);
	void SetFontProperty(HDC, int, const char *);
	void SetFontProperty(const char *, const LOGFONT &);
	void SetSoundProperty(const char *, const char *);

	JNIEnv * GetEnv() { 
	    return (JNIEnv *)JNU_GetEnv(jvm, JNI_VERSION_1_2);
	}

	jobject		self;
};

#endif // AWT_DESKTOP_PROPERTIES_H
