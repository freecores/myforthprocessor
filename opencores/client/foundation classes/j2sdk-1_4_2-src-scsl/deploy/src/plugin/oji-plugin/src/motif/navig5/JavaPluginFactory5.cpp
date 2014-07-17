/*
 * @(#)JavaPluginFactory5.cpp	1.53 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/* 
 * Version of the plugin factory that works with the 5.0 version of
 * the browser. Provides a function pointer table that is used
 * by the main PluginFactory. 
 * The plugin factory is responsible for all the main plugin fuctionality.
 * It delegates to two other classes - the ProxySupport object for help
 * with interfacing to proxies and the JavaVM for handling the
 * actual link to the out-of-proc VM
 */


#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <libintl.h>
#include "remotejni.h"
#include "nsILiveconnect.h"

// Mozilla changes : Because we need it
#include "nsIGenericFactory.h"
#include "nsIServiceManager.h"
#include "nsIFactory.h"
#include "nsIJVMPluginInstance.h"
#include "nsISecureEnv.h"
#include "nsIJVMManager.h"
#include "nsIJVMConsole.h"
#include "Navig5.h"
#include "nsICookieStorage.h"
#include "JavaPluginFactory5.h"
#include "CookieSupport.h"
#include "CJavaConsole.h"
#include "pluginversion.h"

extern "C" {
#include <assert.h>
}

#define TRACE(m) trace("JavaPluginFactory5:%s\n", m);
#define TRACE_INT(m, i) trace("JavaPluginFactory5: %s %d\n", m, i);
#define TRACE3(m,q,w,e) trace(m,q,w,e);


/* Global plugin factory */
static JavaPluginFactory5* g_plugin_factory = NULL;

JavaPluginFactory5* get_global_factory() {
    if (g_plugin_factory == NULL) 
	plugin_error("No global plugin factory!");
    return  g_plugin_factory;
}

NS_DEFINE_IID(kIPluginIID, NS_IPLUGIN_IID);
NS_DEFINE_IID(kIPluginInstanceIID, NS_IPLUGININSTANCE_IID);
NS_DEFINE_IID(kIFactoryIID, NS_IFACTORY_IID);
NS_DEFINE_IID(kISupportsIID, NS_ISUPPORTS_IID);
NS_DEFINE_IID(kIJVMConsoleIID, NS_IJVMCONSOLE_IID);
NS_DEFINE_IID(kILiveConnectIID, NS_ILIVECONNECT_IID);
NS_DEFINE_IID(kIJVMManagerIID, NS_IJVMMANAGER_IID);
NS_DEFINE_IID(kIPluginManagerIID, NS_IPLUGINMANAGER_IID);
NS_DEFINE_IID(kJVMPluginIID, NS_IJVMPLUGIN_IID);
NS_DEFINE_IID(kIPluginManager2IID, NS_IPLUGINMANAGER2_IID);
NS_DEFINE_IID(kICookieStorageIID, NS_ICOOKIESTORAGE_IID);

NS_DEFINE_CID(kCLiveConnectCID, NS_CLIVECONNECT_CID);
NS_DEFINE_CID(kCJVMManagerCID, NS_JVMMANAGER_CID);
NS_DEFINE_CID(kCPluginManagerCID, NS_PLUGINMANAGER_CID);
NS_DEFINE_CID(kCCookieStorageCID,NS_COOKIESTORAGE_CID);
NS_DEFINE_CID(kPluginCID, NS_PLUGIN_CID);

NS_DEFINE_CID(kJavaPluginCID,NS_JAVAPLUGIN_CID);

NS_METHOD JavaPluginFactory5::Create(nsISupports* sm,   
               const nsIID& aIID,    
               void* *aInstancePtr) {

    nsresult rv = NS_ERROR_UNEXPECTED;

//fprintf(stderr,"*******In JavaPluginFactory5::Create\n");

    if (aInstancePtr == NULL) {
        plugin_error("Received a null pointer to pointer in Create!\n");
    } else {
        JavaPluginFactory5 *res;

        if (!g_plugin_factory) {
            res = new JavaPluginFactory5(sm); // g_plugin_factory set in here
            // The peice of shit browser does not seem to call Initialize
            // on this code path!!!
            res->Initialize(); 
            init_utils();
        } else {
            res = g_plugin_factory;
        }

        rv = res->QueryInterface(aIID,aInstancePtr);
    }

    return rv;
}

/*
 * Performs simple initialization of various variables. This version
 * can dump the hacks from previous versions - it only has to work
 * with the 5.0 browser
 */
JavaPluginFactory5::JavaPluginFactory5(nsISupports* pProvider)  {
    TRACE("Constructor");
    NS_INIT_REFCNT();
    plugin_manager = NULL;
    isInitialized = false;
    is_java_vm_started = 0;
    g_plugin_factory = this;
    factory_monitor = PR_NewMonitor();
    javaVM = new JavaVM5(this);
    proxy_support = new ProxySupport5(javaVM);
    cookieSupport = new CookieSupport(javaVM);
    /* Allocate and zero out array  memory since NULL is used to 
       indicate an empty spot */
    int inst_sz = PLUGIN_INSTANCE_COUNT * sizeof(JavaPluginInstance5 *);
    plugin_instances = (JavaPluginInstance5 **)	malloc(inst_sz);
    memset(plugin_instances, 0, inst_sz);
    int env_sz = MAX_ENVS * sizeof(RemoteJNIEnv*);
    int proxy_env_sz = MAX_ENVS * sizeof(JNIEnv*);
    current_envs = (RemoteJNIEnv**) malloc(env_sz);
    current_proxy_envs = (JNIEnv**) malloc(proxy_env_sz);
    current_env_tids = (PRThread**) malloc(MAX_ENVS * sizeof(PRThread*));
    memset(current_envs, 0, env_sz);
    memset(current_proxy_envs, 0, proxy_env_sz);
    memset(current_env_tids, 0, MAX_ENVS * sizeof(PRThread*));
    pluginNameString=(char*)malloc(100);
    memset(pluginNameString, 0, sizeof(pluginNameString));

    service_provider = new CPluginServiceProvider(pProvider);

    CJavaConsole::Create((nsIJVMPlugin*)this, this, kISupportsIID, (void**)&m_pIJVMConsole);
}

//nsISupports interface methods. 
// Mozilla changes : Signature changed
NS_IMETHODIMP JavaPluginFactory5::QueryInterface(const nsIID & iid, void **ptr) {
    if (NULL == ptr)                                         
	return NS_ERROR_NULL_POINTER;  
    
    if (iid.Equals(kJVMPluginIID) || 
		iid.Equals(kISupportsIID)) {
		*ptr = (void*) (nsIJVMPlugin*) this;
		AddRef();
		return NS_OK;		
	}
	else if (iid.Equals(kIPluginIID) || 
		iid.Equals(kIFactoryIID)) {
		*ptr = (void*) (nsIPlugin*) this;
		AddRef();
		return NS_OK;		
	}
        else if (iid.Equals(kIJVMConsoleIID))  {
                return m_pIJVMConsole->QueryInterface(iid, ptr);
        }
	else
		return NS_NOINTERFACE;
}

NS_IMPL_ADDREF(JavaPluginFactory5)
NS_IMPL_RELEASE(JavaPluginFactory5)

// nsIPlugin methods


// (Corresponds to NPP_GetMIMEDescription.)
NS_IMETHODIMP JavaPluginFactory5::GetMIMEDescription(const char **resDesc) 
{
    trace("JavaPluginFactory::GetMIMEDescription\n");
    *resDesc = "application/x-java-vm::Java(tm) Plug-in;"PLUGIN_MIMETABLE;

    return NS_OK;
}


/* LockFactory does nothing at the moment */
NS_IMETHODIMP JavaPluginFactory5::LockFactory(PRBool aLock) {
    trace("LockFactory %d\n", (int) aLock);
    return NS_OK;
}

JavaVM5*
JavaPluginFactory5::GetJavaVM(void) {
  if (!is_java_vm_started) {
    nsJVMInitArgs args;
    args.version = nsJVMInitArgs_Version;
    args.classpathAdditions = NULL;
    nsresult ret = StartupJVM(&args);
    if (NS_OK != ret) {
      plugin_error("VM did not start up properly");
      is_java_vm_started = 0;
      return NULL;
    }
  }
  return javaVM;
}

JavaPluginFactory5::~JavaPluginFactory5(void)  {
    TRACE("******************** DESTROYING THE PLUGIN FACTORY! ***********");

    g_plugin_factory = NULL;
    if (plugin_manager != NULL)
		plugin_manager->Release();

    if (is_java_vm_started)
		ShutdownJVM(0);
    
    if (javaVM) 
		delete(javaVM);
    if (proxy_support)
		delete(proxy_support);
    if (plugin_instances)
		free(plugin_instances);
    if (cookieSupport)
		delete(cookieSupport);
    if(pluginNameString != NULL)
      free(pluginNameString);

    //XXXFIX Destroy the service provider? mProvider
  
/* 
    if (m_pIJVMConsole != NULL) 
	m_pIJVMConsole->Release;
*/
       
}

NS_IMETHODIMP JavaPluginFactory5::SpendTime(PRUint32 timeMillis) {
        UNUSED(timeMillis);
	return NS_OK;
}

NS_IMETHODIMP
JavaPluginFactory5::Initialize() {
  // With the new API, the service manager
  // made available in the GetFactory method and is 
  // then passed into the constructor of the factory,
  // where the CPluginServiceProvider is created.
  // No args to initialize
  
    static nsresult error = NS_ERROR_FAILURE;
    if (isInitialized) 
        return error;
    else 
        isInitialized = true;
    TRACE("JavaPluginFactory::Initialize\n");

    /* Set the plugin and jvm managers */
    //XXXFIX consider the lifetime of these objects and when
    // the refs should be released. They should probably
    // be freed when this object is destroyed, but there might
    // be circular references from the plugin manager to the
    // plugin factory and back.
    if (NS_FAILED(service_provider->QueryService(kCPluginManagerCID, 
						 kIPluginManager2IID, 
						 (nsISupports **)&plugin_manager)))
      plugin_error("Could not get the plugin manager");
    

    if (NS_FAILED(service_provider->QueryService(kCJVMManagerCID, 
						 kIJVMManagerIID, 
						 (nsISupports **) &jvm_manager)))
      plugin_error("Could not get the JVM manager");

    if (NS_FAILED(service_provider->QueryService(kCPluginManagerCID,
    						 kICookieStorageIID,
    						 (nsISupports **) &cookieStorage)))
     plugin_error("Could not get the CookieStorage");

    if (plugin_manager != NULL) {
	/* Dump the environment variables for debugging */
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

	/* This must be a 5.0+ browser */
	const char *agent = "No agent";
	int res;

	if (NS_OK != (res = GetPluginManager()->UserAgent(&agent))) {
	    plugin_error("JavaPluginFactory5 init - no agent?\n");
	    return res;
	}

	int navigator_version = 0;

	sscanf(agent, "Mozilla/%d", &navigator_version);
	trace("User agent=%s. Version = %d \n", agent, navigator_version);
	if (navigator_version < 5) 
	    plugin_error("Expected a version > 5! Version = %d\n", 
			 navigator_version);

        error = NS_OK;
	return error;
    } else {
	plugin_error("No manager for initializing factory?\n");
	error = NS_ERROR_ILLEGAL_VALUE;
	return error;
    }

}


NS_IMETHODIMP
JavaPluginFactory5::Shutdown(void) {
    TRACE("Shutdown");

/*
    if (plugin_manager != NULL) {
	plugin_manager->Release();
	plugin_manager = NULL;
    }
*/
    return NS_OK;
}


// Mozilla changes : Probably should be doing more here...
NS_IMETHODIMP
JavaPluginFactory5::CreatePluginInstance(nsISupports *aOuter,
                                         REFNSIID aIID,
                                         const char * aMineType,
                                         void **result) {
  UNUSED (aMineType);
  TRACE("CreatePluginInstance");
  return CreateInstance(aOuter, aIID, result);
}

NS_IMETHODIMP
JavaPluginFactory5::CreateInstance(nsISupports *aOuter,
				       const nsIID & aIID,
				       void **result) {
    TRACE("CreateInstance");

    if (result == NULL) {
        plugin_error("NULL result in create instance");
	return NS_ERROR_UNEXPECTED;
    }

    *result = NULL;

    if (aOuter != NULL) {
        plugin_error("NO_AGGREGATION in create instance!");
	return NS_ERROR_NO_AGGREGATION;
    }

    if (! (aIID.Equals(kIPluginInstanceIID) ||
	   aIID.Equals(kISupportsIID)))
	return NS_ERROR_NO_INTERFACE;

    /* Startup the JVM if it is not already running */
    JavaVM5* vm = GetJavaVM();

    /* Create a new instance and refcount it */
    JavaPluginInstance5 * pluginInstance = new JavaPluginInstance5(this);

    *result = (nsIPluginInstance *) pluginInstance;
    pluginInstance->AddRef();

    UNUSED(vm);

    return NS_OK;
}


NS_IMETHODIMP
JavaPluginFactory5::StartupJVM(nsJVMInitArgs *initargs) {
    TRACE("StartupJVM");

    nsresult ret = NS_OK;

    if (is_java_vm_started) {
	plugin_error("StartupJVM is being called twice!\n");
	return NS_OK;
    }

    EnterMonitor("StartupJVM");
    
    /* Make sure someone did not start it up while we were
       waiting for the monitor */
    if (!is_java_vm_started) {
	ret = javaVM->StartJavaVM(initargs->classpathAdditions);
	if (ret == NS_OK) 
	    is_java_vm_started = 1;
	 else
	     plugin_error("Could not start JavaVM!\n");
    } else {
	plugin_error("StartupJVM has already been called.\n");
    }

    ExitMonitor("StartupJVM");

    return ret;

}


NS_IMETHODIMP
JavaPluginFactory5::ShutdownJVM(PRBool fullShutdown) {
    TRACE("ShutdownJVM");

    javaVM->ShutdownJavaVM(fullShutdown);
    is_java_vm_started = 0;
    return NS_OK;
}


NS_IMETHODIMP
JavaPluginFactory5::GetValue(nsPluginVariable variable, void
					    *value) {
    TRACE("GetValue");

    nsresult err = NS_OK;

    switch (variable) {
      case nsPluginVariable_NameString:
	  /* Can there be a race here ? */
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


NS_IMETHODIMP
JavaPluginFactory5::AddToClassPath(const char* dirPath) {
    UNUSED(dirPath);
    TRACE("AddToClassPath");
    return NS_OK;
}


NS_IMETHODIMP
JavaPluginFactory5::RemoveFromClassPath(const char* dirPath) {
    if (tracing) trace("RemoveFromClassPath %s", dirPath);
    return NS_OK;
}

NS_IMETHODIMP
JavaPluginFactory5::GetClassPath(const char* *result) {
    TRACE("GetClassPath");
    *result = getenv("CLASSPATH");
    return NS_OK;
}

NS_IMETHODIMP
JavaPluginFactory5::GetJavaWrapper(JNIEnv* proxy_env, 
				   jint browser_obj, 
				   jobject* java_obj) {
    TRACE("JavaPluginFactory5::GetJavaWrapper()\n");

    /* Can determine the right RemoteJNIEnv from looking up the
       env for the current thread, or can look up the remote
       JNIEnvs to find out which one corresponds to this proxyenv */

    if (browser_obj == 0 || java_obj == NULL)    
	return NS_ERROR_NULL_POINTER;
    
    /* Get the live connect object for calls back on jobj */
    nsILiveconnect* pLiveConnect = NULL;
    RemoteJNIEnv* *env_ref;
    UNUSED(env_ref);
    
    if (NS_SUCCEEDED(service_provider->QueryService(kCLiveConnectCID,
						    kILiveConnectIID,
						    (nsISupports**) &pLiveConnect)))
      {
	/* GetJNIEnv(env_ref); */
	/* JNIEnv* env = *env_ref; */
	
	RemoteJNIEnv* env = GetRemoteEnv(proxy_env);
	env->ExceptionClear();
	jclass jsobj5_clazz = 
	    env->FindClass("sun/plugin/javascript/navig5/JSObject");

	if (jsobj5_clazz == NULL) 
	    plugin_error("Could not create the java wrapper. No JSObject\n");

	jmethodID jsobj_create_method = env->GetMethodID(jsobj5_clazz, 
							 "<init>",
							 "(II)V");
	*java_obj = env->NewObject(jsobj5_clazz, jsobj_create_method,
				   (jint) pLiveConnect, (jint) browser_obj);
	service_provider->ReleaseService(kCLiveConnectCID, pLiveConnect);
        return NS_OK;

    } else {
	plugin_error("JavaPluginFactory5::Could not get a live connect\n");
        return NS_ERROR_FAILURE;
    }
}

NS_IMETHODIMP
JavaPluginFactory5::UnwrapJavaWrapper(JNIEnv* jenv, jobject jobj, jint* obj) {

    // I copied this function from the Windows version and 
    // modified it to use the remote JNI (I hope [smk])

    TRACE("JavaPluginFactory5::UnwrapJavaWrapper\n");

    if (jenv == NULL || jobj == NULL || obj == NULL)
        return NS_ERROR_NULL_POINTER;

    RemoteJNIEnv* env = GetRemoteEnv(jenv);
    env->ExceptionClear();

    // Find class
    jclass clazz = env->FindClass("sun/plugin/javascript/navig5/JSObject");

    // Check if we may unwrap the JSObject
    if (env->IsInstanceOf(jobj, clazz) == JNI_FALSE)
    {
        return NS_ERROR_FAILURE;
    }

    jfieldID fid = env->GetFieldID(clazz, "nativeJSObject", "I");
    *obj = env->GetIntField(jobj, fid);

    return NS_OK;
}


/* Create a new applet, with a particular index */
void
JavaPluginFactory5::CreateApplet(const char* appletType, int appletNumber,
				 int argc, char **argn, char **argv) {
    TRACE("CreateApplet");

    /* Not sure if calling create applet is permitted before startup */
    while (!is_java_vm_started) {
	plugin_error("CreateApplet called before the VM is started\n?");
	sleep(1);
    }

    EnterMonitor("CreateApplet");

    /* The VM could have been shutdown again */
    if (is_java_vm_started) {
	trace("CreateApplet %d \n", appletNumber);
	javaVM->CreateApplet(appletType, appletNumber, argc, argn, argv);
    } else {
	plugin_error("VM not initialized. Cannot create applet!");
    }

    ExitMonitor("CreateApplet");
	
}


NS_IMETHODIMP
JavaPluginFactory5::CreateSecureEnv(JNIEnv* proxy_env,nsISecureEnv* *result) 
{
    TRACE("CreateSecureEnv");
    if (result != NULL) *result = NULL;

// This is left here to make it easy to turn off live connect support 
// should something prove to be out of wack! (Just un comment)
//    return NS_ERROR_FAILURE;

    JavaVM5 * vm = GetJavaVM();
    if(!(vm->GetJVMEnabled())) return NS_ERROR_FAILURE;

    RemoteJNIEnv* remote_env = javaVM->CreateRemoteJNIEnv(proxy_env);

    return CSecureJNIEnv::Create(NULL, remote_env, 
			       kISupportsIID, 
			       (void**) result);
}


/* Register an env and return an index associated with that env. 
   This index should not be used directly, but may be useful for 
   debugging purposes. We also associate the current thread with
   'env' */
int 
JavaPluginFactory5::RegisterRemoteEnv(RemoteJNIEnv* env, JNIEnv* proxy_env) {
    EnterMonitor("Register Env");
    for(int i = 0; i < MAX_ENVS; i++) {
	if (current_envs[i] == NULL) {
	    current_envs[i] = env;
	    current_env_tids[i] = PR_GetCurrentThread();
	    current_proxy_envs[i] = proxy_env;
	    TRACE3("JavaPluginFactory5: Register Env [%d] proxyenv=%d tid=%d", i, (int) proxy_env, (int) current_env_tids[i])
	    ExitMonitor("Register Env");
	    return i;
	}
    }
    ExitMonitor("Register Env");
    plugin_error("Env table is full!");
    return -1;
}

/* Free the index assocaited with the remote env 'env'. Should be called
   when the remote env is destroyed, which should be when the securejni
   is destroyed */
int 
JavaPluginFactory5::UnregisterRemoteEnv(RemoteJNIEnv* env) {
    EnterMonitor("Register Env");
    for(int i = 0; i < MAX_ENVS; i++) {
	if (current_envs[i] == env) {
	    current_envs[i] = NULL;
	    current_env_tids[i] = NULL;
	    current_proxy_envs[i] = NULL;
	    ExitMonitor("Register Env");
	    return i;
	}
    }
    ExitMonitor("Register Env");
    plugin_error("No such env found!");
    return -1;
}

/* The folowing methods are not locked since there should be no race
   between modifying and reading the remote env table i.e.
   Looking up an env by index must always work (or there's a bug)
   Looking up an env by the current thread  
     - if there is no env for the thread, then only the current thread
        can remove it and vice versa.
   */
RemoteJNIEnv*
JavaPluginFactory5::GetRemoteEnv(JNIEnv* proxy_env) {
    for(int i = 0; i < MAX_ENVS; i++) {
	if (current_proxy_envs[i] == proxy_env) {
	  return current_envs[i];
	}
    }
    plugin_error("No remote env found for the proxy_env\n");
    return NULL;
}

RemoteJNIEnv*
JavaPluginFactory5::GetRemoteEnvForThread(void) {
    PRThread* tid = PR_GetCurrentThread();
    TRACE_INT("Searching for env for tid: ", (int) tid);
    for(int i=0; i < MAX_ENVS; i++) {
	if (current_envs[i] != NULL && 
	    current_env_tids[i] == tid) 
	    return current_envs[i];
    }
    return NULL;
}

/*  
 * Plugin Internal: Register an instance with the plugin We would like
 * to be able to recover PluginInstances from the plugin.  So we keep
 * a registry of currently running instances. We provide the functions
 * to register, unregister, and get a plugin instance from that
 * registry.
 */
int 
JavaPluginFactory5::RegisterInstance(JavaPluginInstance5* pluginInstance) {
    TRACE("RegisterInstance");

    int i;

    EnterMonitor("RegisterInstance");
    for (i = 0; i < PLUGIN_INSTANCE_COUNT; i++) {
	if (plugin_instances[i] == NULL) {	
	    trace("JavaPluginFactory5::RegisterInstance %d at %d\n", 
		  (int) pluginInstance, i);
	    plugin_instances[i] = pluginInstance;
	    ExitMonitor("RegisterInstance");
	    return i;
	}
    }
    plugin_error("Could not register plugininstance\n");
    ExitMonitor("RegisterInstance");
    return NS_ERROR_FAILURE;
}

/* 
 * Plugin Internal: Unregister an instance from the internal table.
 */
void
JavaPluginFactory5::UnregisterInstance(JavaPluginInstance5 * pluginInstance) {
    int i;
    EnterMonitor("UnregisterInstance.");
    int plugin_number = pluginInstance->GetPluginNumber();
    trace("Unregistering %d \n", plugin_number);
    for (i = 0; i < PLUGIN_INSTANCE_COUNT; i++) {
	if (plugin_instances[i] == pluginInstance) {	
	    trace("Unregistering instance %d\n", i);
	    plugin_instances[i] = NULL;
	    ExitMonitor("UnregisterInstance-a");
	    return;
	}
    }
    trace("Plugin: couldn't find plugin_instance %d\n", plugin_number);
    ExitMonitor("UnregisterInstance-b");
    return;
}
   
JavaPluginInstance5 *JavaPluginFactory5::GetInstance(int index) {
    TRACE("GetInstance");

    JavaPluginInstance5* res;
    EnterMonitor("GetInstance");
    
    /* First handle the -1 index - pick a random instance */
    if (index == -1) {
	for (int i = 0; i < PLUGIN_INSTANCE_COUNT; i++) {
	    if (plugin_instances[i] != (JavaPluginInstance5 *)NULL) {
		if (tracing)
		    trace("Chose random instance %d\n", i);
		ExitMonitor("GetInstance-any");
		return plugin_instances[i];
	    }
	}
        TRACE("Returning NULL for random instance");
        return (JavaPluginInstance5 *)NULL; 
    }

    /* For a non-random index, check bounds */
    if ((index < 0) || (index >= PLUGIN_INSTANCE_COUNT)) {
	plugin_error("Plugin instance index out of bounds %d\n", index);
	res =  (JavaPluginInstance5 *)NULL;
    } else {
	res = (JavaPluginInstance5 *) plugin_instances[index];    
	if (res == NULL) 
	    trace("Returning a NULL instance! %d\n", index);
    }
    ExitMonitor("GetInstance-normal");

    return res;

}


/* Make a call to return the plugin instance for an object */
jobject 
JavaPluginFactory5::GetJavaObjectForInstance(int plugin_number) {

  /* Get the vm, making sure it is started if necessary */

  JavaVM5* jvm = GetJavaVM();
  
  return jvm->GetJavaObjectForInstance(plugin_number);

}

void JavaPluginFactory5::SendRequest(const CWriteBuffer& wb, int wait_for_reply)
{

    EnterMonitor("SendRequest");
    if (is_java_vm_started)
	javaVM->SendRequest(wb, wait_for_reply);
    else {
	plugin_error("VM is not yet started up in SendRequest!");
    }
    ExitMonitor("SendRequest");

}


ProxySupport5* JavaPluginFactory5::GetProxySupport(void) {
    TRACE("GetProxySupport");
    if (proxy_support == NULL) {
	plugin_error("Proxy support is null!");
    }
    return proxy_support;
}


void JavaPluginFactory5::EnterMonitor(const char *msg) {
    if (tracing)
	trace("JavaPluginFactory trying to enter %s\n", msg);

    PR_EnterMonitor(factory_monitor);

    if (tracing)
	trace("JavaPluginFactory Entered %s\n", msg);

}


void JavaPluginFactory5::ExitMonitor(const char *msg) {
    if (tracing)
	trace("JavaPluginFactory exiting %s\n", msg);
    PR_ExitMonitor(factory_monitor);
}


NS_IMETHODIMP JavaPluginFactory5::ShowJavaConsole(void) {

    JavaVM5 *vm = GetJavaVM();

    CWriteBuffer wb;
    TRACE("ShowJavaConsole");
    wb.putInt(JAVA_PLUGIN_CONSOLE_SHOW);
    SendRequest(wb, FALSE);

    UNUSED(vm);

    return NS_OK;
}
