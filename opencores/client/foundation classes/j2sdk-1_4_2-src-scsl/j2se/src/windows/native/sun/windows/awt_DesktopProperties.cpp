/*
 * @(#)awt_DesktopProperties.cpp	1.22 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "jlong.h"
#include "awt.h"
#include "awt_DesktopProperties.h"
#include "awt_dlls.h"
#include "sun_awt_windows_WDesktopProperties.h"
#include "java_awt_Font.h"
#include "mmsystem.h"
#include "awtmsg.h"
#include "Zmouse.h"

// WDesktopProperties fields
jfieldID AwtDesktopProperties::pDataID = 0;
jmethodID AwtDesktopProperties::setBooleanPropertyID = 0;
jmethodID AwtDesktopProperties::setIntegerPropertyID = 0;
jmethodID AwtDesktopProperties::setStringPropertyID = 0;
jmethodID AwtDesktopProperties::setColorPropertyID = 0;
jmethodID AwtDesktopProperties::setFontPropertyID = 0;
jmethodID AwtDesktopProperties::setSoundPropertyID = 0;

AwtDesktopProperties::AwtDesktopProperties(jobject self) {
    this->self = GetEnv()->NewGlobalRef(self);
    GetEnv()->SetLongField( self, AwtDesktopProperties::pDataID,
			    ptr_to_jlong(this) );
}

AwtDesktopProperties::~AwtDesktopProperties() {
    GetEnv()->DeleteGlobalRef(self);
}

//
// Reads Windows parameters and sets the corresponding values
// in WDesktopProperties
//
void AwtDesktopProperties::GetWindowsParameters() {    
    if (GetEnv()->EnsureLocalCapacity(MAX_PROPERTIES) < 0) {
	DASSERT(0);
	return;
    }
    // this number defines the set of properties available, it is incremented
    // whenever more properties are added (in a public release of course)
    // for example, version 1 defines the properties available in Java SDK version 1.3.
    SetIntegerProperty( "win.properties.version", AWT_DESKTOP_PROPERTIES_VERSION);
    GetNonClientParameters();
    GetIconParameters();
    GetColorParameters();
    GetOtherParameters();
    GetSoundEvents();
    GetSystemProperties();
    if (IS_WINXP) {
	GetXPStyleProperties();
    }
}

void AwtDesktopProperties::GetSystemProperties() {
    HDC dc = CreateDC("DISPLAY", NULL, NULL, NULL);

    if (dc != NULL) {
        SetFontProperty(dc, ANSI_FIXED_FONT, "win.ansiFixed.font");
        SetFontProperty(dc, ANSI_VAR_FONT, "win.ansiVar.font");
        SetFontProperty(dc, DEVICE_DEFAULT_FONT, "win.deviceDefault.font");
        SetFontProperty(dc, DEFAULT_GUI_FONT, "win.defaultGUI.font");
        SetFontProperty(dc, OEM_FIXED_FONT, "win.oemFixed.font");
        SetFontProperty(dc, SYSTEM_FONT, "win.system.font");
        SetFontProperty(dc, SYSTEM_FIXED_FONT, "win.systemFixed.font");
        DeleteDC(dc);
    }
}


// Local function for getting values from the Windows registry
// Note that it uses malloc() and returns the pointer to allocated
// memory, so remember to use free() when you are done with its
// result.
static char* getXPStylePropFromReg(char* valueName) {
    char* subKey = "Software\\Microsoft\\Windows\\CurrentVersion\\ThemeManager";

    HKEY handle;
    if (RegOpenKeyEx(HKEY_CURRENT_USER, subKey, 0, KEY_READ, &handle) != 0) {
	return NULL;
    }
    DWORD valueType, valueSize;
    if (RegQueryValueEx((HKEY)handle, valueName, NULL,
			&valueType, NULL, &valueSize) != 0) {
	return NULL;
    }
    char *buffer = (char*)safe_Malloc(valueSize);
    if (RegQueryValueEx((HKEY)handle, valueName, NULL,
			&valueType, (unsigned char *)buffer, &valueSize) != 0) {
	free(buffer);
	return NULL;
    }
    RegCloseKey(handle);

    if (valueType == REG_EXPAND_SZ) {  
	// Pending: buffer must be null-terminated at this point
	valueSize = ExpandEnvironmentStrings(buffer, NULL, 0);
	char *buffer2 = (char *)safe_Malloc(valueSize);
	valueSize = ExpandEnvironmentStrings(buffer, buffer2, valueSize);
	free(buffer);
	return buffer2;
    } else if (valueType == REG_SZ) {  
	return buffer;
    } else {
	free(buffer);
	return NULL;
    }
}

// Used in AwtMenuItem to determine the color of top menus,
// since they depend on XP style. ThemeActive property is
// '1' for XP Style, '0' for Windows classic style.
BOOL AwtDesktopProperties::IsXPStyle() {
    char* style = getXPStylePropFromReg("ThemeActive");
    BOOL result = (style != NULL && *style == '1');
    free(style);
    return result;
}

void AwtDesktopProperties::GetXPStyleProperties() {
    char* value;

    value = getXPStylePropFromReg("ThemeActive");
    SetBooleanProperty("win.xpstyle.themeActive", (value != NULL && *value == '1'));
    if (value != NULL) {
	free(value);
    }
    value = getXPStylePropFromReg("DllName");
    if (value != NULL) {
	SetStringProperty("win.xpstyle.dllName", value);
	free(value);
    }
    value = getXPStylePropFromReg("SizeName");
    if (value != NULL) {
	SetStringProperty("win.xpstyle.sizeName", value);
	free(value);
    }
    value = getXPStylePropFromReg("ColorName");
    if (value != NULL) {
	SetStringProperty("win.xpstyle.colorName", value);
	free(value);
    }
}


void AwtDesktopProperties::GetNonClientParameters() {
    //
    // general window properties
    //
    NONCLIENTMETRICS	ncmetrics;

    ncmetrics.cbSize = sizeof(ncmetrics);
    VERIFY( SystemParametersInfo(SPI_GETNONCLIENTMETRICS, ncmetrics.cbSize, &ncmetrics, FALSE) );

    SetFontProperty( "win.frame.captionFont", ncmetrics.lfCaptionFont );
    SetIntegerProperty( "win.frame.captionHeight", ncmetrics.iCaptionHeight );
    SetIntegerProperty( "win.frame.captionButtonWidth", ncmetrics.iCaptionWidth );
    SetIntegerProperty( "win.frame.captionButtonHeight", ncmetrics.iCaptionHeight );
    SetFontProperty( "win.frame.smallCaptionFont", ncmetrics.lfSmCaptionFont );
    SetIntegerProperty( "win.frame.smallCaptionHeight", ncmetrics.iSmCaptionHeight );
    SetIntegerProperty( "win.frame.smallCaptionButtonWidth", ncmetrics.iSmCaptionWidth );
    SetIntegerProperty( "win.frame.smallCaptionButtonHeight", ncmetrics.iSmCaptionHeight );
    SetIntegerProperty( "win.frame.sizingBorderWidth", ncmetrics.iBorderWidth );
    
    // menu properties
    SetFontProperty( "win.menu.font", ncmetrics.lfMenuFont );
    SetIntegerProperty( "win.menu.height", ncmetrics.iMenuHeight );
    SetIntegerProperty( "win.menu.buttonWidth", ncmetrics.iMenuWidth );
    
    // scrollbar properties
    SetIntegerProperty( "win.scrollbar.width", ncmetrics.iScrollWidth );
    SetIntegerProperty( "win.scrollbar.height", ncmetrics.iScrollHeight );
    
    // status bar and tooltip properties
    SetFontProperty( "win.status.font", ncmetrics.lfStatusFont );
    SetFontProperty( "win.tooltip.font", ncmetrics.lfStatusFont );

    // message box properties
    SetFontProperty( "win.messagebox.font", ncmetrics.lfMessageFont );
}

void AwtDesktopProperties::GetIconParameters() {
    //
    // icon properties
    //
    ICONMETRICS	iconmetrics;

    iconmetrics.cbSize = sizeof(iconmetrics);
    VERIFY( SystemParametersInfo(SPI_GETICONMETRICS, iconmetrics.cbSize, &iconmetrics, FALSE) );

    SetIntegerProperty("win.icon.hspacing", iconmetrics.iHorzSpacing);
    SetIntegerProperty("win.icon.vspacing", iconmetrics.iVertSpacing);
    SetBooleanProperty("win.icon.titleWrappingOn", iconmetrics.iTitleWrap != 0);
    SetFontProperty("win.icon.font", iconmetrics.lfFont);
}

void AwtDesktopProperties::GetColorParameters() {

    if (IS_WIN98 || IS_WIN2000) {
        SetColorProperty("win.frame.activeCaptionGradientColor", 
                              GetSysColor(COLOR_GRADIENTACTIVECAPTION));
        SetColorProperty("win.frame.inactiveCaptionGradientColor", 
                              GetSysColor(COLOR_GRADIENTINACTIVECAPTION));
        SetColorProperty("win.item.hotTrackedColor", 
                              GetSysColor(COLOR_HOTLIGHT));
    }    
    SetColorProperty("win.3d.darkShadowColor", GetSysColor(COLOR_3DDKSHADOW));
    SetColorProperty("win.3d.backgroundColor", GetSysColor(COLOR_3DFACE));
    SetColorProperty("win.3d.highlightColor", GetSysColor(COLOR_3DHIGHLIGHT));
    SetColorProperty("win.3d.lightColor", GetSysColor(COLOR_3DLIGHT));
    SetColorProperty("win.3d.shadowColor", GetSysColor(COLOR_3DSHADOW));
    SetColorProperty("win.button.textColor", GetSysColor(COLOR_BTNTEXT));
    SetColorProperty("win.desktop.backgroundColor", GetSysColor(COLOR_DESKTOP));
    SetColorProperty("win.frame.activeCaptionColor", GetSysColor(COLOR_ACTIVECAPTION));
    SetColorProperty("win.frame.activeBorderColor", GetSysColor(COLOR_ACTIVEBORDER));
    
    // ?? ?? ??
    SetColorProperty("win.frame.color", GetSysColor(COLOR_WINDOWFRAME)); // ?? WHAT THE HECK DOES THIS MEAN ??
    // ?? ?? ??
    
    SetColorProperty("win.frame.backgroundColor", GetSysColor(COLOR_WINDOW));
    SetColorProperty("win.frame.captionTextColor", GetSysColor(COLOR_CAPTIONTEXT));
    SetColorProperty("win.frame.inactiveBorderColor", GetSysColor(COLOR_INACTIVEBORDER));
    SetColorProperty("win.frame.inactiveCaptionColor", GetSysColor(COLOR_INACTIVECAPTION));
    SetColorProperty("win.frame.inactiveCaptionTextColor", GetSysColor(COLOR_INACTIVECAPTIONTEXT));
    SetColorProperty("win.frame.textColor", GetSysColor(COLOR_WINDOWTEXT));
    SetColorProperty("win.item.highlightColor", GetSysColor(COLOR_HIGHLIGHT));
    SetColorProperty("win.item.highlightTextColor", GetSysColor(COLOR_HIGHLIGHTTEXT));
    SetColorProperty("win.mdi.backgroundColor", GetSysColor(COLOR_APPWORKSPACE));
    SetColorProperty("win.menu.backgroundColor", GetSysColor(COLOR_MENU));
    SetColorProperty("win.menu.textColor", GetSysColor(COLOR_MENUTEXT));
    SetColorProperty("win.scrollbar.backgroundColor", GetSysColor(COLOR_SCROLLBAR));
    SetColorProperty("win.text.grayedTextColor", GetSysColor(COLOR_GRAYTEXT));
    SetColorProperty("win.tooltip.backgroundColor", GetSysColor(COLOR_INFOBK));
    SetColorProperty("win.tooltip.textColor", GetSysColor(COLOR_INFOTEXT));
}

void AwtDesktopProperties::GetOtherParameters() {
    // TODO BEGIN: On NT4, some setttings don't trigger WM_SETTINGCHANGE --
    // check whether this has been fixed on Windows 2000 and Windows 98
    // ECH 10/6/2000 seems to be fixed on NT4 SP5, but not on 98
    SetBooleanProperty("win.frame.fullWindowDragsOn", GetBooleanParameter(SPI_GETDRAGFULLWINDOWS));
    SetBooleanProperty("win.text.fontSmoothingOn", GetBooleanParameter(SPI_GETFONTSMOOTHING));
    // TODO END

    SetIntegerProperty("win.drag.width", GetSystemMetrics(SM_CXDRAG));
    SetIntegerProperty("win.drag.height", GetSystemMetrics(SM_CYDRAG));

    SetIntegerProperty("awt.multiClickInterval", GetDoubleClickTime());

    // BEGIN cross-platform properties
    // Note that these are cross-platform properties, but are being stuck into
    // WDesktopProperties.  WToolkit.lazilyLoadDesktopProperty() can find them,
    // but if a Toolkit subclass uses the desktopProperties
    // member, these properties won't be there. -bchristi, echawkes
    // This property is called "win.frame.fullWindowDragsOn" above
    // This is one of the properties that don't trigger WM_SETTINGCHANGE
    SetBooleanProperty("awt.dynamicLayoutSupported", GetBooleanParameter(SPI_GETDRAGFULLWINDOWS));

    // 95 MouseWheel support
    // More or less copied from the MSH_MOUSEWHEEL MSDN entry
    if (IS_WIN95 && !IS_WIN98) {
        HWND hdlMSHWHEEL = NULL;
        UINT msgMSHWheelSupported = NULL;
        BOOL wheelSupported = FALSE;

        msgMSHWheelSupported = RegisterWindowMessage(MSH_WHEELSUPPORT);
        hdlMSHWHEEL = FindWindow(MSH_WHEELMODULE_CLASS, MSH_WHEELMODULE_TITLE);
        if (hdlMSHWHEEL && msgMSHWheelSupported) {
            wheelSupported = (BOOL)::SendMessage(hdlMSHWHEEL,
                                                 msgMSHWheelSupported, 0, 0);
        }
        SetBooleanProperty("awt.wheelMousePresent", wheelSupported);

    }
    else {
        SetBooleanProperty("awt.wheelMousePresent",
                           ::GetSystemMetrics(SM_MOUSEWHEELPRESENT));
    }

    // END cross-platform properties

    if (IS_WIN98 || IS_WIN2000) {
      //DWORD	menuShowDelay;
        //SystemParametersInfo(SPI_GETMENUSHOWDELAY, 0, &menuShowDelay, 0);
	// SetIntegerProperty("win.menu.showDelay", menuShowDelay);
        SetBooleanProperty("win.frame.captionGradientsOn", GetBooleanParameter(SPI_GETGRADIENTCAPTIONS));
        SetBooleanProperty("win.item.hotTrackingOn", GetBooleanParameter(SPI_GETHOTTRACKING));
    }

    if (IS_WIN2000) {
        SetBooleanProperty("win.menu.keyboardCuesOn", GetBooleanParameter(SPI_GETKEYBOARDCUES));
    }

    // High contrast accessibility property
    HIGHCONTRAST contrast;
    contrast.cbSize = sizeof(HIGHCONTRAST);
    if (SystemParametersInfo(SPI_GETHIGHCONTRAST, sizeof(HIGHCONTRAST),
			     &contrast, 0) != 0 &&
	      (contrast.dwFlags & HCF_HIGHCONTRASTON) == HCF_HIGHCONTRASTON) {
      SetBooleanProperty("win.highContrast.on", TRUE);
    }
    else {
      SetBooleanProperty("win.highContrast.on", FALSE);
    }
}

void AwtDesktopProperties::GetSoundEvents() {
    /////
    SetSoundProperty("win.sound.default", ".Default");
    SetSoundProperty("win.sound.close", "Close");
    SetSoundProperty("win.sound.maximize", "Maximize");
    SetSoundProperty("win.sound.minimize", "Minimize");
    SetSoundProperty("win.sound.menuCommand", "MenuCommand");
    SetSoundProperty("win.sound.menuPopup", "MenuPopup");
    SetSoundProperty("win.sound.open", "Open");
    SetSoundProperty("win.sound.restoreDown", "RestoreDown");
    SetSoundProperty("win.sound.restoreUp", "RestoreUp");
    /////
    SetSoundProperty("win.sound.asterisk", "SystemAsterisk");
    SetSoundProperty("win.sound.exclamation", "SystemExclamation");
    SetSoundProperty("win.sound.exit", "SystemExit");
    SetSoundProperty("win.sound.hand", "SystemHand");
    SetSoundProperty("win.sound.question", "SystemQuestion");
    SetSoundProperty("win.sound.start", "SystemStart");
}

BOOL AwtDesktopProperties::GetBooleanParameter(UINT spi) {
    BOOL	flag;
    SystemParametersInfo(spi, 0, &flag, 0);
    DASSERT(flag == TRUE || flag == FALSE); // should be simple boolean value
    return flag;
}

void AwtDesktopProperties::SetStringProperty(const char * propName, char * value) {
    jstring key = GetEnv()->NewStringUTF(propName);
    GetEnv()->CallVoidMethod(self,
			     AwtDesktopProperties::setStringPropertyID,
			     key, GetEnv()->NewStringUTF(value));
}

void AwtDesktopProperties::SetIntegerProperty(const char * propName, int value) {
    jstring key = GetEnv()->NewStringUTF(propName);
    GetEnv()->CallVoidMethod(self,
			     AwtDesktopProperties::setIntegerPropertyID,
			     key, (jint)value);
}

void AwtDesktopProperties::SetBooleanProperty(const char * propName, BOOL value) {
    jstring key = GetEnv()->NewStringUTF(propName);
    GetEnv()->CallVoidMethod(self,
			     AwtDesktopProperties::setBooleanPropertyID,
			     key, value ? JNI_TRUE : JNI_FALSE);
}

void AwtDesktopProperties::SetColorProperty(const char * propName, DWORD value) {    
    jstring key = GetEnv()->NewStringUTF(propName);
    GetEnv()->CallVoidMethod(self,
			     AwtDesktopProperties::setColorPropertyID,
			     key, GetRValue(value), GetGValue(value),
			     GetBValue(value));
}

void AwtDesktopProperties::SetFontProperty(HDC dc, int fontID,
                                           const char *propName) {
    HGDIOBJ font = GetStockObject(fontID);
    if (font != NULL && SelectObject(dc, font) != NULL) {
        int length = GetTextFace(dc, 0, NULL);

        if (length > 0) {
            CHAR *face = new CHAR[length];

            if (GetTextFace(dc, length, face) > 0) {
                TEXTMETRIC metrics;

                if (GetTextMetrics(dc, &metrics) > 0) {
                    jstring fontName = GetEnv()->NewStringUTF(face);
                    jint pointSize = metrics.tmHeight -
		                     metrics.tmInternalLeading;
                    jint style = java_awt_Font_PLAIN;

                    if (metrics.tmWeight >= FW_BOLD) {
                        style =  java_awt_Font_BOLD;
                    }
                    if (metrics.tmItalic ) {
                        style |= java_awt_Font_ITALIC;
                    }

                    jstring key = GetEnv()->NewStringUTF(propName);
                    GetEnv()->CallVoidMethod(self,
                              AwtDesktopProperties::setFontPropertyID,
                              key, fontName, style, pointSize);
                }
            }
            delete[] face;
        }
    }
}

void AwtDesktopProperties::SetFontProperty(const char * propName, const LOGFONT & font) {
    jstring fontName;
    jint pointSize;
    jint style;

    fontName = JNU_NewStringPlatform(GetEnv(), font.lfFaceName);

#if 0
    HDC		hdc;
    int		pixelsPerInch = GetDeviceCaps(hdc, LOGPIXELSY);
    // convert font size specified in pixels to font size in points
    hdc = GetDC(NULL);
    pointSize = (-font.lfHeight)*72/pixelsPerInch;
    ReleaseDC(NULL, hdc);
#endif    
    // Java uses point sizes, but assumes 1 pixel = 1 point
    pointSize = -font.lfHeight;

    // convert Windows font style to Java style
    style = java_awt_Font_PLAIN;
    DTRACE_PRINTLN1("weight=%d", font.lfWeight);
    if ( font.lfWeight >= FW_BOLD ) {
	style =  java_awt_Font_BOLD;
    }
    if ( font.lfItalic ) {
	style |= java_awt_Font_ITALIC;
    }

    jstring key = GetEnv()->NewStringUTF(propName);
    GetEnv()->CallVoidMethod(self, AwtDesktopProperties::setFontPropertyID,
			     key, fontName, style, pointSize);
}

void AwtDesktopProperties::SetSoundProperty(const char * propName, const char * winEventName) {
    jstring key = GetEnv()->NewStringUTF(propName);
    jstring event = GetEnv()->NewStringUTF(winEventName);
    GetEnv()->CallVoidMethod(self,
			     AwtDesktopProperties::setSoundPropertyID,
			     key, event);
}

void AwtDesktopProperties::PlayWindowsSound(const char * event) {    
    // stop any currently playing sounds
    AwtWinMM::PlaySoundWrapper(NULL, NULL, SND_PURGE);
    // play the sound for the given event name
    AwtWinMM::PlaySoundWrapper(event, NULL, SND_ASYNC|SND_ALIAS|SND_NODEFAULT);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

static AwtDesktopProperties * GetCppThis(JNIEnv *env, jobject self) {
    jlong longProps = env->GetLongField(self, AwtDesktopProperties::pDataID);
    AwtDesktopProperties * props =
        (AwtDesktopProperties *)jlong_to_ptr(longProps);
    DASSERT( !IsBadReadPtr(props, sizeof(*props)) );
    return props;
}

JNIEXPORT void JNICALL
Java_sun_awt_windows_WDesktopProperties_initIDs(JNIEnv *env, jclass cls) {
    TRY;

    AwtDesktopProperties::pDataID = env->GetFieldID(cls, "pData", "J");
    DASSERT(AwtDesktopProperties::pDataID != 0);

    AwtDesktopProperties::setBooleanPropertyID = env->GetMethodID(cls, "setBooleanProperty", "(Ljava/lang/String;Z)V");
    DASSERT(AwtDesktopProperties::setBooleanPropertyID != 0);
    
    AwtDesktopProperties::setIntegerPropertyID = env->GetMethodID(cls, "setIntegerProperty", "(Ljava/lang/String;I)V");
    DASSERT(AwtDesktopProperties::setIntegerPropertyID != 0);

    AwtDesktopProperties::setStringPropertyID = env->GetMethodID(cls, "setStringProperty", "(Ljava/lang/String;Ljava/lang/String;)V");
    DASSERT(AwtDesktopProperties::setStringPropertyID != 0);

    AwtDesktopProperties::setColorPropertyID = env->GetMethodID(cls, "setColorProperty", "(Ljava/lang/String;III)V");
    DASSERT(AwtDesktopProperties::setColorPropertyID != 0);

    AwtDesktopProperties::setFontPropertyID = env->GetMethodID(cls, "setFontProperty", "(Ljava/lang/String;Ljava/lang/String;II)V");
    DASSERT(AwtDesktopProperties::setFontPropertyID != 0);

    AwtDesktopProperties::setSoundPropertyID = env->GetMethodID(cls, "setSoundProperty", "(Ljava/lang/String;Ljava/lang/String;)V");
    DASSERT(AwtDesktopProperties::setSoundPropertyID != 0);

    CATCH_BAD_ALLOC;
}

JNIEXPORT void JNICALL
Java_sun_awt_windows_WDesktopProperties_init(JNIEnv *env, jobject self) {
    TRY;

    new AwtDesktopProperties(self);

    CATCH_BAD_ALLOC;
}

JNIEXPORT void JNICALL
Java_sun_awt_windows_WDesktopProperties_finalize(JNIEnv *env, jobject self) {
    TRY_NO_VERIFY;

    AwtDesktopProperties *props = GetCppThis(env, self);
    DASSERT(props != NULL);

    env->SetLongField(self, AwtDesktopProperties::pDataID,
		      ptr_to_jlong(NULL));
    delete props;

    CATCH_BAD_ALLOC;
}

JNIEXPORT void JNICALL
Java_sun_awt_windows_WDesktopProperties_getWindowsParameters(JNIEnv *env, jobject self) {
    TRY;

    GetCppThis(env, self)->GetWindowsParameters();

    CATCH_BAD_ALLOC;
}

JNIEXPORT void JNICALL
Java_sun_awt_windows_WDesktopProperties_playWindowsSound(JNIEnv *env, jobject self, jstring event) {
    TRY;

    const char * winEventName;
    winEventName = JNU_GetStringPlatformChars(env, event, NULL);
    if ( winEventName == NULL ) {
	return;
    }
    GetCppThis(env, self)->PlayWindowsSound(winEventName);
    JNU_ReleaseStringPlatformChars(env, event, winEventName);

    CATCH_BAD_ALLOC;
}

