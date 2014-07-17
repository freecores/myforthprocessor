/*
 * @(#)CJavaPluginInstance.h	1.11 01/12/03
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * Contains declaration of the plugin instance, modified from the 
 * Win32 version
 */


#ifndef CJAVAPLUGININSTANCE_H
#define CJAVAPLUGININSTANCE_H

#include "nsPlugin.h"
#include "IUniqueIdentifier.h"

class CJavaPluginFactory;

class CJavaPluginInstance : public nsIPluginInstance, public IUniqueIdentifier {

public:
    // Support for QI
    NS_DECL_ISUPPORTS

    // See comment for nsIPlugin::NewInstance, above.
    NS_IMETHOD Initialize(nsIPluginInstancePeer *peer);

    NS_IMETHOD Start(void);

    NS_IMETHOD GetPeer(nsIPluginInstancePeer* *resultingPeer);

    // The old NPP_Destroy call has been factored into two plugin instance 
    // methods:
    //
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
    NS_IMETHOD NewStream(nsIPluginStreamPeer* peer, nsIPluginStream* *result);

    // (Corresponds to NPP_Print.)
    NS_IMETHOD Print(nsPluginPrint* platformPrint);

    NS_IMETHOD GetValue(nsPluginInstanceVariable var, void* val);

    // Handle events. We don't.
    NS_IMETHOD HandleEvent(nsPluginEvent* event, PRBool *handled);

    // (Corresponds to NPP_URLNotify.)
    NS_IMETHOD URLNotify(const char* url, const char* target,
				nsPluginReason reason, void* notifyData);


    CJavaPluginInstance(CJavaPluginFactory *plugin);

    // The following methods handle the return codes from various
    // GetURL requests
    NS_IMETHOD_(void) SetDocbase(const char *url);

    NS_IMETHOD_(void) JavascriptReply(const char *reply);



    NS_IMETHOD SetUniqueId(long id);
    NS_IMETHOD GetUniqueId(long* pId);
 

    // Return the plugin factory associated with this instance
    CJavaPluginFactory * GetPluginFactory(void);

    virtual	~CJavaPluginInstance(void);
    
    // Return the index of this plugin
    int GetPluginNumber(void) { return m_iPlugin_number; } ;

      // Indicate to this instance that a request is in progress
    // and abrupt termination must be sent if this instance dies
    void EnterRequest(const char *requestName);

    void ExitRequest(const char *requestName);

protected:
    // Browser side object
    nsIPluginInstancePeer*	mPeer;

    // Pointer to the factory
    CJavaPluginFactory *m_PluginFactory;

    // Index of this plugin in the plugin table
    int m_iPlugin_number;

    // If non-null indicate that a request is in process
    const char* current_request;

private:
	long			m_uniqueId;
	static long		s_uniqueId;
};


#endif 
