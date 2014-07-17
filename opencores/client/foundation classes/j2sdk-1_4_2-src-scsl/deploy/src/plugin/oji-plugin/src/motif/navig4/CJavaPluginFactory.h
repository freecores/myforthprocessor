/*
 * @(#)CJavaPluginFactory.h	1.15 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * These functions create, initialize, and shutdown a plugin.
 */

#include "nsIJVMPlugin.h"
#include "jni.h"

class CJavaVM;
class CJavaPluginInstance;


class CJavaPluginFactory : public nsIPlugin {
public:
    // The Release method on nsIPlugin corresponds to NPP_Shutdown.

    // The old NPP_New call has been factored into two plugin instance methods:
    //
    // NewInstance -- called once, after the plugin instance is created. This 
    // method is used to initialize the new plugin instance (although the 
    // actual plugin instance object will be created by the plugin manager).
    //
    // NPIPluginInstance::Start -- called when the plugin instance is to be
    // started. This happens in two circumstances: 
    // (1) after the plugin instance is first initialized, and 
    // (2) after a plugin instance is returned to
    // (e.g. by going back in the window history) after previously 
    // being stopped by the Stop method. 

    NS_DECL_ISUPPORTS
    
    NS_IMETHOD CreateInstance(nsISupports *aOuter,
                              REFNSIID aIID,
                              void **result);
    NS_IMETHOD LockFactory(PRBool aLock);
    NS_IMETHOD Initialize(nsISupports* pluginMgr);
    NS_IMETHOD Shutdown(void);
    
    // (Corresponds to NPP_GetMIMEDescription.)
    NS_IMETHOD GetMIMEDescription(const char **resDesc);

    // (Corresponds to NPP_GetValue.)
    NS_IMETHOD GetValue(nsPluginVariable variable, void *value);

    // (Corresponds to NPP_SetValue.)
    NS_IMETHOD SetValue(nsPluginVariable variable, void *value);

    NS_IMETHOD  StartupJVM(nsJVMInitArgs *initargs);
    NS_IMETHOD  ShutdownJVM(PRBool fullShutdown = PR_FALSE);
    NS_IMETHOD  GetPluginJavaVM(CJavaVM* *result);

    NS_IMETHOD_(void) CreateApplet(const char *appletType, int appletNumber, 
				   int argc, char **argn, char **argv);

    NS_IMETHOD_(void) SendRequest(char *buff, int len, int wait_for_reply);
    NS_IMETHOD_(int) RegisterInstance(CJavaPluginInstance * pi);
    NS_IMETHOD_(void) UnregisterInstance(CJavaPluginInstance * pi);

    CJavaPluginInstance* GetInstance(int index);
    nsIPluginManager* GetPluginManager(void) {return mMgr;} ;

    CJavaPluginFactory(void);
    
    virtual ~CJavaPluginFactory(void);


private:
    nsIPluginManager*   mMgr;
    // m_vm_init indicates whether StartJavaVM has been called or not
    int m_vm_init; 

    CJavaPluginInstance **m_plugin_instances;
    // Pointer to the CJavaVM that is the proxy for the out of process VM
    CJavaVM * m_javaVM;

    // Name of this plugin
    char *pluginNameString;

};


    
