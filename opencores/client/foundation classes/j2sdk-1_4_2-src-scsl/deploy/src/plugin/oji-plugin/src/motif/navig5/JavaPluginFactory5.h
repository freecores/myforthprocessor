/*
 * @(#)JavaPluginFactory5.h	1.28 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef JAVAPLUGINFACTORY5_H
#define JAVAPLUGINFACTORY5_H

#include "CWriteBuffer.h"

#define NS_JAVAPLUGIN_CID                          \
{ /* 0e2b82f0-5e8a-11d5-8485-0050041a3ba3 */         \
     0x0e2b82f0,                                      \
     0x5e8a,                                          \
     0x11d5,                                          \
     {0x84, 0x85, 0x00, 0x50, 0x04, 0x1a, 0x3b, 0xa3} \
}

struct nsJVMInitArgs {
	PRUint32    version;
	const char* classpathAdditions;     // appended to the JVM's classpath
// other fields may be added here for version numbers beyond 0x00010000
};

/**
* nsJVMInitArgs_Version is the current version number for the nsJVMInitArgs
* struct specified above. The nsVersionOk macro should be used when comparing
* a supplied nsJVMInitArgs struct against this version.
*/
#define nsJVMInitArgs_Version   0x00010000L


/* Fill in the function pointer table with the entry points for the
 * 5.0 version
 */

class nsIJVMManager;
class CPluginServiceProvider;
class nsICookieStorage;
class CookieSupport;

/* Implementation of the 5.0 functionality required for the JavaPlugin.
 *  Basically, a class version of the PluginFactoryFunction table
 *  defined in JavaPluginFactory.h 
 */
class JavaPluginFactory5 : public nsIJVMPlugin, nsIPlugin {
public:
  
     NS_DECL_ISUPPORTS

    /* Constructor/destructor */    
    JavaPluginFactory5(nsISupports* pProvider);

    /* Destructor releases the manager and shutsdown the vm and deletes
     * the related structures. */
    virtual ~JavaPluginFactory5(void);

    /* Creates a nsPluginInstance. Since we don't know how we will get
     * called, we have to potentially start the VM if it is not running. */
    NS_IMETHOD CreateInstance(nsISupports *aOuter, const nsIID & iid,
			      void **result);
    NS_IMETHOD LockFactory(PRBool aLock);
    NS_IMETHOD CreatePluginInstance(nsISupports *aOuter, REFNSIID aIID,
                                    const char* aPluginMIMEType,
                                    void **aResult);
    NS_IMETHOD Initialize();
    NS_IMETHOD SpendTime(PRUint32 timeMillis);

    NS_IMETHOD UnwrapJavaWrapper(JNIEnv* jenv, jobject jobj, jint* obj);

    NS_IMETHOD Shutdown(void);

    // (Corresponds to NPP_GetMIMEDescription.)
    NS_IMETHOD GetMIMEDescription(const char **resDesc);
    // (Corresponds to NPP_GetValue.)
    NS_IMETHOD GetValue(nsPluginVariable variable, void *value);

    // (Corresponds to NPP_SetValue.)
    //NS_IMETHOD SetValue(nsPluginVariable variable, void *value);

    NS_IMETHOD  AddToClassPath(const char* dirPath);
    NS_IMETHOD  RemoveFromClassPath(const char* dirPath);
    NS_IMETHOD  GetClassPath(const char* *result);

    /* 
     * GetJavaWrapper provides a java wrapper for (C/C++) browser object
     * pointers.  Map between browser objects (browser_obj - C/C++
     * pointers) and Java objects, (java_obj - a handle to a
     * "JSObject"). Get a Java wrapper for a browser object, so that the
     * browser object can be handled within java code and then passed back
     * to the browser during some JSObject call.  The java wrapper object
     * (a JSObject) has a pointer to the original browser object
     * (browser_obj)
     */
    NS_IMETHOD  GetJavaWrapper(JNIEnv* jenv, jint jint, jobject* obj);
    NS_IMETHOD StartupJVM(nsJVMInitArgs *initargs);
    NS_IMETHOD ShutdownJVM(PRBool fullShutdown = PR_FALSE);

    NS_IMETHOD_(void) CreateApplet(const char *appletType, int appletNumber, 
				   int argc, char **argn, char **argv);

    NS_IMETHOD  CreateSecureEnv(JNIEnv* proxy, nsISecureEnv* *result);

    /* All the following functions are plugin internal i.e. they are 
     * not called by the browser and are not part of the abstract browser
     * interface to the plugin
     */

    /* These methods are called from CJavaConsole which we agregate */
    NS_IMETHOD ShowJavaConsole(void);

    /* Return the Proxy Support class, which handles interactions
     * with the browser to get the proxies, using GetURL */
    ProxySupport5* GetProxySupport(void);

    /* Send a request over the command pipe. */
    void SendRequest(const CWriteBuffer& wb, int wait_for_reply);

    /* Acquire the worker pipe and read data from the worker
     *  thread into the buffer 'buff'. 
     * For extra verification, to ensure that the current jni env is
     * the 'approved' receiver of this data from the worker thread, we
     * pass in an 'env_index' which is used to identify the env
     * that will next read data from the worker pipe. 
     */
/*
    void ReadWork(int env_index, void* buff, int length);
*/

    /* Both the tables below should be split out into their own classes */
    /* Interface to a table of plugin instances */
    int RegisterInstance(JavaPluginInstance5 * pi);
    void UnregisterInstance(JavaPluginInstance5 * pi);
    /* Obtain an instance, given an index into the instance table.
     * An index of -1 results in a random instance being picked */
    JavaPluginInstance5* GetInstance(int index);

    /* Interface to a table of remote envs. */
    int RegisterRemoteEnv(RemoteJNIEnv* env, JNIEnv* proxyenv);

    int UnregisterRemoteEnv(RemoteJNIEnv* env);

    RemoteJNIEnv* GetRemoteEnv(JNIEnv* proxy_env);

    /* Return the RemoteEnv for the current thread */
    RemoteJNIEnv* GetRemoteEnvForThread(void);

    /* Return the browser-side plugin manager */
    nsIPluginManager2* GetPluginManager(void) { 
	if (plugin_manager == NULL) 
	    fprintf(stderr, "Internal error: Null plugin manager");
	return plugin_manager;
    }
    
    CPluginServiceProvider* GetServiceProvider(void) {
      return service_provider;
    }

    nsIJVMManager* GetJVMManager(void) {
	if (jvm_manager == NULL) 
	    fprintf(stderr, "Internal error: Null jvm manager");
	return jvm_manager;
    }

    nsICookieStorage* GetCookieStorage(void) {
	if (cookieStorage == NULL) 
	    fprintf(stderr, "Internal error: Null cookiestorage");
	return cookieStorage;
    }

    /*returns a pointer to cookiesupport */
    CookieSupport *GetCookieSupport(void){
	if (cookieSupport == NULL)
	    fprintf(stderr, "Internal error: Null cookieSupport");
      return cookieSupport;
    }
    /* Enter/exit the monitor associated with this factory */
    /* Enter the monitor associated with this factory for use
     * in the associated classes, invoke.cpp and JavaVM5 */
    void EnterMonitor(const char* msg);

    /* Exit the monitor associated with this factory */
    void ExitMonitor(const char* msg);

    /* Get the java object associated with the plugin instance
       whose index is plugin_number */
    jobject GetJavaObjectForInstance(int plugin_number);

    /* Return the JavaVM object, making sure it is started if necc */
    JavaVM5* GetJavaVM(void);

    static NS_METHOD Create(nsISupports* sm, 
               const nsIID& aIID, 
               void* *aInstancePtr);

private:

    /* The plugin manager provides us with our link to the browser */
    nsIPluginManager2* plugin_manager;
    
    nsIJVMManager* jvm_manager;

    /* Indicates whether the Java VM has already been started or not */
    int is_java_vm_started;

    /* Pointer to cookieSupport class */
    CookieSupport *cookieSupport;
    
    /* where to obtain cookie */
    nsICookieStorage *cookieStorage;


    /* Table of plugin instances */
    JavaPluginInstance5 **plugin_instances;

    
    /* Table of remote envs */
    RemoteJNIEnv **current_envs;

    /* Table of corresponding proxy envs */
    JNIEnv **current_proxy_envs;

    /* Table of remote env thread ids */
    PRThread **current_env_tids;

    /* Pointer to the JavaVM that is the proxy for the out of process VM */
    JavaVM5* javaVM;

    /* Pointer to proxy supporting structures */
    ProxySupport5 *proxy_support;
   
    /* Name of this plugin */
    char *pluginNameString;

    /* Monitor used to protect the factory, and the command pipe */
    PRMonitor* factory_monitor;

    CPluginServiceProvider* service_provider;
    nsIJVMConsole * m_pIJVMConsole;

    bool isInitialized;
};

extern "C" {
JavaPluginFactory5* get_global_factory(void);
}

#endif





