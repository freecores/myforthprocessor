/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-*/
/*
 * @(#)JavaPluginInstance5.h	1.16 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * Contains declaration of the CNetscapePlugin. Modified version of
 *  Stanley's Win32 header.
 */


#ifndef JAVAPLUGININSTANCE5_H
#define JAVAPLUGININSTANCE5_H

#include "nsIJVMPluginInstance.h"

class JavaPluginFactory5;

class JavaPluginInstance5 : public nsIJVMPluginInstance, nsIPluginInstance {
public:

    NS_DECL_ISUPPORTS
		

    JavaPluginInstance5(JavaPluginFactory5 *plugin);
    virtual	~JavaPluginInstance5(void);

    JavaPluginFactory5* GetPluginFactory(void);
    // See comment for nsIPlugin::NewInstance, above.
    NS_IMETHOD Initialize(nsIPluginInstancePeer *peer);
    NS_IMETHOD Start(void);
    NS_IMETHOD GetPeer(nsIPluginInstancePeer* *resultingPeer);

    NS_IMETHOD GetJavaObject(jobject *result);

    NS_IMETHOD GetText(const char* *result);

    // The old NPP_Destroy call has been factored into two plugin instance 
    // methods:
    // Stop -- called when the plugin instance is to be stopped (e.g. by 
    // displaying another plugin manager window, causing the page containing 
    // the plugin to become removed from the display).
    //
    // Release -- called once, before the plugin instance peer is to be 
    // destroyed. This method is used to destroy the plugin instance.
    NS_IMETHOD Stop(void);

    NS_IMETHOD Destroy(void);

    // (Corresponds to NPP_SetWindow.)
    NS_IMETHOD SetWindow(nsPluginWindow* window);

    // (Corresponds to NPP_NewStream.)
    NS_IMETHOD NewStream(nsIPluginStreamListener** listener);

    // (Corresponds to NPP_Print.)
    NS_IMETHOD Print(nsPluginPrint* platformPrint);

    NS_IMETHOD GetValue(nsPluginInstanceVariable var, void* val);

    // (Corresponds to NPP_HandleEvent.)
    // Note that for Unix and Mac the NPPluginEvent structure is different
    // from the old NPEvent structure -- it's no longer the native event
    // record, but is instead a struct. Done for future extensibility,
    // and so that the Mac could receive the window argument too. For Windows
    // and OS2, it's always been a struct, so there's no change for them.
    NS_IMETHOD HandleEvent(nsPluginEvent* event, PRBool *handled);

    // (Corresponds to NPP_URLNotify.)
    NS_IMETHOD URLNotify(const char* url, const char* target,
				nsPluginReason reason, void* notifyData);

    NS_IMETHOD_(void) SetDocbase(const char *url);
 
    NS_IMETHOD_(void) EnterRequest(char* msg);
    NS_IMETHOD_(void) ExitRequest(char* msg);

    // Return the index of this plugin
    int GetPluginNumber(void) { return plugin_number; } ;
    // Mozilla changes: This will embed the given AppletWindow ID into Navigator's
    // containing browser window
    void EmbedAppletWindow(int windowID);

    

protected:
    nsIPluginInstancePeer*	instance_peer;
    JavaPluginFactory5*         plugin_factory;
    int plugin_number;
    
    // Window
    nsPluginWindow* window;

    // If non-null indicate that a request is in process
    char* current_request;

    // Need to know if we have had Destroy called on us 
    bool mIsDestroyed;
};

#endif 
