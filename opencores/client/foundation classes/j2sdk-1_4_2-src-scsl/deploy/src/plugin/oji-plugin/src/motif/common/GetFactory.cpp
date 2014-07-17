/*
 * @(#)GetFactory.cpp	1.17 03/01/23
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
#include <sys/socket.h>
#include <libintl.h>
#include <dlfcn.h>
//#include "JavaPluginFactory.h"
#include "Navig4.h"


#define PLUGIN_OJI_SO "libjavaplugin_oji.so"
typedef nsIJVMPlugin* (*CreateFactoryFnType)(nsISupports* );

extern "C" {
#include "Debug.h"
#include <assert.h>
}

#ifndef NO_MINIMAL_TRACE
#define TRACE(m) trace("JavaPluginFactory(main):%s\n", m);
#endif

NS_DEFINE_IID(kPluginIID, NS_IPLUGIN_IID);
NS_DEFINE_IID(kJVMPluginIID, NS_IJVMPLUGIN_IID);
NS_DEFINE_IID(kPluginManagerIID, NS_IPLUGINMANAGER_IID);
NS_DEFINE_IID(kPluginInstanceIID, NS_IPLUGININSTANCE_IID);
NS_DEFINE_IID(kFactoryIID, NS_IFACTORY_IID);
NS_DEFINE_IID(kSupportsIID, NS_ISUPPORTS_IID);
NS_DEFINE_CID(kPluginCID, NS_PLUGIN_CID);

/* True if the factory_delegate should be used */
static int in_oji_version;	

/* Report an error indicating that the 4.0 version of 'method_name'
 * should not be called in a 5.0 browser
 */
void
version_error(char *method_name) {
    plugin_error("%s method should not be called in a 4.0 browser.\n", 
		 method_name );
}


nsresult JPI_NSGetFactory(nsISupports * pProvider,
			  const nsCID &aClass,
			  const char * aClassName,
			  const char * aProgID,
			  nsIFactory **aFactory) 
{
  (void) aClassName; // defeat compiler "not used" warning.
  (void) aProgID;

  trace("JPI_NSGetFactory\n");

  if (aFactory == NULL){
	plugin_error("Received a null pointer to pointer in NSGetFactory!\n");
	return NS_ERROR_UNEXPECTED;
    }

    if (aClass.Equals(kPluginCID)) {

#ifdef __linux__
	void *handle = 0;
#else
	void *handle = dlopen("libnspr4.so", RTLD_LAZY);
#endif
	if (!handle) {

	    /* Navigator 4.0 or less */
	    in_oji_version = 0;
	    *aFactory = new CJavaPluginFactory();
	} else {

	    TRACE("Detected Mozilla 5 or greater\n");
	    char factory5path[MAXPATHLEN];
	    in_oji_version = 1;
	    // Initialize the 5.0 factory functions from libjavaplugin5.so
	    char *home = getenv("HOME");
	    if (home == NULL) {
		plugin_error("Could not load 5.0 plugin. $HOME not set", 
			     PLUGIN_OJI_SO);
		return NS_ERROR_UNEXPECTED;
	    }

	    trace("$HOME for loading %s = %s\n", PLUGIN_OJI_SO, home);
	    if ((strlen(home) + 100) > MAXPATHLEN) {
		plugin_error("Too long $HOME %s \n", home);
		return NS_ERROR_UNEXPECTED;
	    }

	    sprintf(factory5path, "%s/.netscape/java/lib/%s",
		    home, PLUGIN_OJI_SO);
	    if (tracing) 
		trace("Loading Mozilla 5 plugin from: %s \n", factory5path);
	    void *factory5_handle = dlopen(factory5path, RTLD_LAZY);

	    if (!factory5_handle) {
		plugin_error("Could not load %s: linking error=%s\n",
			     PLUGIN_OJI_SO, dlerror());
		return NS_ERROR_UNEXPECTED;
	    }

	    CreateFactoryFnType create_factory_fn =
	      (CreateFactoryFnType) (jlong)
	      load_function(factory5_handle, "CreateOJIFactory");
	    *aFactory = (*create_factory_fn)(pProvider);
	} 	

	init_utils();
	(*aFactory)->AddRef();
	return NS_OK;
    }
    
    return NS_ERROR_NO_INTERFACE;
}

/*
NS_IMPL_ADDREF(JavaPluginFactory)
NS_IMPL_RELEASE(JavaPluginFactory)
*/
