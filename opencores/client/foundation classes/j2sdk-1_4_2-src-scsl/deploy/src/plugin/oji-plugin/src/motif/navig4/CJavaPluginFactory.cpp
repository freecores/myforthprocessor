/*
 * @(#)CJavaPluginFactory.cpp	1.29 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/* 
 * JavaPluginFactory.cpp  by Robert Szewczyk
 * These functions create, initialize, and shutdown a plugin.
 */


#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <X11/Xlib.h>
#include <X11/Intrinsic.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
/* For MAXPATHLEN */
#include <sys/param.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <libintl.h>
#include <dlfcn.h>
#include "Navig4.h"
#include "pluginversion.h"

extern "C" {
#include "Debug.h"
#include <assert.h>
}

#ifndef NO_MINIMAL_TRACE
#define TRACE(m) trace("CJavaPluginFactory:%s\n", m);
#endif


NS_DEFINE_IID(kPluginIID, NS_IPLUGIN_IID);
NS_DEFINE_IID(kPluginManagerIID, NS_IPLUGINMANAGER_IID);
NS_DEFINE_IID(kPluginInstanceIID, NS_IPLUGININSTANCE_IID);
NS_DEFINE_IID(kFactoryIID, NS_IFACTORY_IID);
NS_DEFINE_IID(kSupportsIID, NS_ISUPPORTS_IID);


/*
 * CTOR. Performs simple initialization of various variables.Note that we kept
 * the nasty hacks from the previous versions so that we will be able to work
 * with the older plugins. 
 */
CJavaPluginFactory::CJavaPluginFactory(void)  {
    TRACE("CJavaPluginFactory::constructor\n");
    NS_INIT_REFCNT();
    mMgr = NULL;
    m_vm_init = 0;
    m_javaVM = new CJavaVM(this);
    m_plugin_instances = (CJavaPluginInstance **)
	malloc(PLUGIN_INSTANCE_COUNT * sizeof(CJavaPluginInstance *));

    for (int i=0; i < PLUGIN_INSTANCE_COUNT; i++)
      m_plugin_instances[i] = NULL;

    pluginNameString=(char*)malloc(100);
    memset(pluginNameString, 0, 100);
    // Can't construct the monitor just yet
}

/* 
 * Destructor releases the manager and shutsdown the vm and deletes
 * the related structures.
 */
CJavaPluginFactory::~CJavaPluginFactory(void)  {
    trace("CJavaPluginFactory::destructor\n");
    if (mMgr != NULL)
	mMgr->Release();
    if (m_vm_init)
	ShutdownJVM();
    delete(m_javaVM);
    free(pluginNameString);    
    free(m_plugin_instances);
}

// nsIFactory methods. 

/*
 *CreateInstance. Creates the nsPluginInstance. Since we don't know how we
 *will get called, we have to potentially start the VM if it is not running.
 */
NS_IMETHODIMP
CJavaPluginFactory::CreateInstance(nsISupports *aOuter,
				   REFNSIID aIID,
				   void **result)
{
    trace("CJavaPluginFactory::CreateInstance\n");
    if (result == NULL)
	return NS_ERROR_UNEXPECTED;
    *result = NULL;

    if (aOuter != NULL)
	return NS_ERROR_NO_AGGREGATION;
    if (! (aIID.Equals(kPluginInstanceIID) ||
	   aIID.Equals(kSupportsIID)))
	return NS_ERROR_NO_INTERFACE;
    if (!m_vm_init) {
	nsJVMInitArgs args;
	args.version = nsJVMInitArgs_Version;
	args.classpathAdditions = NULL;
	nsresult ret = StartupJVM(&args);
	if (NS_OK != ret) {
	    m_vm_init = 0;
	    return ret;
	}
    }
    *result = new CJavaPluginInstance(this);
    ((CJavaPluginInstance *)(*result))->AddRef();

    return NS_OK;
}


//nsISupports interface methods. 

NS_IMETHODIMP 
CJavaPluginFactory::QueryInterface(REFNSIID iid, void **ptr) {
    if (NULL == ptr)                                         
	return NS_ERROR_NULL_POINTER;  
    
    if (iid.Equals(kPluginIID) 
	|| iid.Equals(kFactoryIID)
	|| iid.Equals(kSupportsIID))  {
	*ptr = (void*) this;
	AddRef();
	
	return NS_OK;		
    }
    else
	return NS_NOINTERFACE;
}

NS_IMPL_ADDREF(CJavaPluginFactory)
NS_IMPL_RELEASE(CJavaPluginFactory)

// nsIPlugin methods


// (Corresponds to NPP_GetMIMEDescription.)
NS_IMETHODIMP CJavaPluginFactory::GetMIMEDescription(const char **resDesc) 
{
    trace("JavaPluginFactory::GetMIMEDescription\n");
    *resDesc = PLUGIN_MIMETABLE;
    return NS_OK;
}


/* LockFactory does nothing at the moment */
NS_IMETHODIMP 
CJavaPluginFactory::LockFactory(PRBool aLock) {
    trace("LockFactory %d\n", (int) aLock);
    return NS_OK;
}

NS_IMETHODIMP CJavaPluginFactory::GetPluginJavaVM(CJavaVM* *result)
{
  *result = m_javaVM;
  return NS_OK;
}

/* Initialize registers the plugin manager for future use. It
 * also sets up the plugin_nspr wrapper functions for future use.
 * Initialize must be called immediately after the plugin has beencreated. 
 */
NS_IMETHODIMP CJavaPluginFactory::Initialize(nsISupports *pluginMgr) {
    pluginMgr->QueryInterface(kPluginManagerIID, (void **)&mMgr);
    trace("JavaPluginFactory::Initialize\n");
    if (mMgr != NULL) {
	trace("JavaPluginFactory::doing Initialize\n");
	if (tracing) {
	    char *cp = getenv("CLASSPATH");
	    char *jtrace = getenv("JAVA_PLUGIN_TRACE");
	    char *vmwait  = getenv("JAVA_VM_WAIT");
	    char *ldpath = getenv("LD_LIBRARY_PATH");
	    if (cp) trace("CLASSPATH = %s\n", cp);
	    if (jtrace) trace("JAVA_PLUGIN_TRACE = %s\n", jtrace);
	    if (vmwait) trace("JAVA_VM_WAIT = %s\n", vmwait);
	    if (ldpath) trace("LD_LIBRARY_PATH = %s\n", ldpath);
	}
	// Determine whether this is >= navigator 5.  
	const char *agent = "No agent";
	int res;
	if (NS_OK != (res = GetPluginManager()->UserAgent(&agent))) {
	    trace("Initializing JavaPluginFactory:Could not get the agent\n");
	    return res;
	}
	
	/* Navigator version - only valid after initialization */
	int navigator_version = 0;

	sscanf(agent, "Mozilla/%d", &navigator_version);
	trace("User agent=%s. Version = %d \n", agent, navigator_version);
	if (navigator_version < 0 || navigator_version >= 5) {
	    plugin_error("Some problem with the version %d\n", 
			 navigator_version);
	    return NS_ERROR_UNEXPECTED;
	} else {
	    TRACE("Detected pre-mozilla Navigator");
	}

	// Create the Factory monitor which is shared by the VM, invoke 
	// and this
	return NS_OK;
    } else {
	plugin_error("Initializing factory. No Manager???");
	return NS_ERROR_ILLEGAL_VALUE;
    }
}

//performs the cleanup to operations done by Initialize
NS_IMETHODIMP CJavaPluginFactory::Shutdown(void) {
    trace("CJavaPluginFactory::Shutdown\n");
    int res = NS_OK;
    if (mMgr != NULL) {
	mMgr->Release();
	mMgr = NULL;
    };
    return res;
}

// getValue returns values for a few variables present in the plugin.
NS_IMETHODIMP CJavaPluginFactory::GetValue(nsPluginVariable variable, 
					   void* value) {
    trace("CJavaPluginFactory::GetValue\n");

    nsresult err = NS_OK;
    switch (variable) {
      case nsPluginVariable_NameString:
	  // Maybe this should be in the monitor, but it may be
	  // called before the factory is initialized
	  if (pluginNameString[0] == 0)
	      sprintf(pluginNameString,"Java(TM) Plug-in %s", VERSION);
	  *((char **)value) = pluginNameString;
	  break;
      case nsPluginVariable_DescriptionString:
	*((const char **)value) = "Java(TM) Plug-in "PLUGIN_VERSION;
	break;
      default:
	err = NS_ERROR_ILLEGAL_VALUE;
    }
    return err;
}

/*
 * currently there are no variables that we would like to set through this
 * call. 
 */
NS_IMETHODIMP CJavaPluginFactory::SetValue(nsPluginVariable variable,
					    void *value) {
    trace("CJavaPluginFactory::SetValue Do nothing %d %d\n",(int) variable,
	  (int) value);
    return NS_OK;
}



/*
 * We would like to be able to recover PluginInstances from the plugin. To
 * that endd we keep a registry of currently running instances. We provide the
 * functions to register, unregister, and get a plugin instance from that
 * registry. 
 */
int 
CJavaPluginFactory::RegisterInstance(CJavaPluginInstance* pluginInstance) 
{
    trace("CJavaPluginFactory::RegisterInstance\n");
    for (int i = 0; i < PLUGIN_INSTANCE_COUNT; i++) {
	if (m_plugin_instances[i] == NULL) {	
	    trace("CJavaPluginFactory::RegisterInstance 0x%x at %d\n", pluginInstance, i);
	    m_plugin_instances[i] = pluginInstance;
	    pluginInstance->AddRef();
	    return i;
	}
    }
    plugin_error("Plugin: couldn't allocate plugin_instance");
    return -1;
}

/* 
 * Converse of register instance
 */
void
CJavaPluginFactory::UnregisterInstance(CJavaPluginInstance* pluginInstance) 
{
    trace("CJavaPluginFactory::UnregisterInstance\n");
    int plugin_number = pluginInstance->GetPluginNumber();
    trace("UnregisterInstance at %d \n", plugin_number);
    for (int i = 0; i < PLUGIN_INSTANCE_COUNT; i++) {
	if (m_plugin_instances[i] == pluginInstance) {	
	    trace("UnregisterInstance 0x%x at %d\n", pluginInstance, i);
	    m_plugin_instances[i] = NULL;
	    pluginInstance->Release();
	}
    }
    trace("Plugin: couldn't find plugin_instance %d\n", plugin_number);
    return;
}
   
/* Obtain an instance, given an index into the instance table */
CJavaPluginInstance *CJavaPluginFactory::GetInstance(int index) {
    CJavaPluginInstance* result = NULL;
    trace("GetInstance %d in %d\n", index, PLUGIN_INSTANCE_COUNT);
    if (index == -1) {
	//pick a random instance
	for (int i = 0; i < PLUGIN_INSTANCE_COUNT; i++) {
	    if (m_plugin_instances[i] != (CJavaPluginInstance *)NULL) {
		result = m_plugin_instances[i];
		break;
	    }
	}
    }
    else if ((index <0) || (index >= PLUGIN_INSTANCE_COUNT)) {
	result =  (CJavaPluginInstance *)NULL;
    } else {
	result = (CJavaPluginInstance *) m_plugin_instances[index];    
    }

    trace("GetInstance 0x%x in %d\n", result, index);

    // The instance may have been destroyed or has not yet been registered.
    return result;
}


// These methods start up and shutdown the VM. They will eventually transform
// into the corresponding methods in nsIJVMPlugin

/*
 * Start the JVM. If the vm has not yet been started up, call the
 * corresponding StartJVM method in CJavaVM
 */
NS_IMETHODIMP CJavaPluginFactory::StartupJVM(nsJVMInitArgs *initargs) {
    nsresult ret = NS_OK;

    if (!m_vm_init) {
	trace("CJavaPluginFactory::StartupJVM call actual startup\n");
	ret = m_javaVM->StartJavaVM(initargs->classpathAdditions);
	if (ret == NS_OK) {
	    trace("CJavaPluginFactory::StartupJVM startup was ok %d\n",
		  m_javaVM);
	    m_vm_init = 1;
	} else {
	    trace("!!!!!Plugin Factory could not start java vm\n");
	    fprintf(stderr, "Could not start java vm\n");
	}
    } else {
	plugin_error("StartupJVM has already been called by mozilla.\n");
    }
    return ret;
}

NS_IMETHODIMP CJavaPluginFactory::ShutdownJVM(PRBool fullShutdown) {
    trace("CJavaPluginFactory::ShutdownJVM %d\n", (int) fullShutdown);
    // Cannot be protected with a monitor, since the mgr will be null
    // before it can properly exit!
    m_javaVM->ShutdownJavaVM(0);
    m_vm_init = 0;
    return NS_OK;
}


/*
 * A bunch of methods needed for communication with the VM, and general
 * information. 
 */
void CJavaPluginFactory::SendRequest(char *buff, int len, int wait_for_reply)
{
    while (!m_vm_init) {
	trace("CJavaPluginFactory::SendRequest sleeping... for init\n");
	sleep(1);
    }
    if (m_vm_init)
	m_javaVM->SendRequest(buff, len, wait_for_reply);
    else
	plugin_error("VM not initialized in send request!");

}

void CJavaPluginFactory::CreateApplet(const char *appletType, 
				      int appletNumber,
				      int argc, char **argn, char **argv) {
    trace("CJavaPluginFactory::CreateApplet\n");
    while (!m_vm_init) {
	trace("CJavaPluginFactory::CreateApplet sleeping...\n");
	sleep(1);
    }
    if (m_vm_init) {
	trace("CJavaPluginFactory::calling m_javaVM %d %d\n",
	      appletNumber, (int) m_javaVM);
	m_javaVM->CreateApplet(appletType, appletNumber, argc, argn, argv);
    } else {
	plugin_error("VM not initialized. Cannot create applet!");
    }
}




