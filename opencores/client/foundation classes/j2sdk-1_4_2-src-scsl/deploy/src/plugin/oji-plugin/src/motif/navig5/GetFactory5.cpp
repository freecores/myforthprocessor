/*
 * @(#)GetFactory5.cpp	1.15 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/* 
 * JavaPluginFactory5.cpp  by Mark Lin (mark.lin@eng.sun.com)
 * These functions create, initialize, and shutdown a plugin.
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
#include <dlfcn.h>
#include "nsIComponentManager.h"
#include "nsCOMPtr.h"

// Added by Mark : Mozilla 5.x headers
#include "nsIGenericFactory.h"
#include "remotejni.h"
#include "Navig5.h"

#include "nsIPlugin.h"
#include "pluginversion.h"

static NS_DEFINE_CID(kPluginCID, NS_PLUGIN_CID);

extern "C" {
#include "Debug.h"
#include <assert.h>
}

#ifndef NO_MINIMAL_TRACE
#define TRACE(m) trace("JavaPluginFactory5(main):%s\n", m);
#endif

/* The very first function called (Unless we are using nsIModule)           
 */                                                                             
extern "C" NS_EXPORT nsresult NSGetFactory(nsISupports * pProvider,             
                                           const nsCID &aClass,                 
                                           const char * aClassName,             
                                           const char * aProgID,                
                                           nsIFactory **aFactory) {             
  trace("NSGetFactory in GetFactory5\n");                                       
                                                                                
   UNUSED(aClassName);                                                         
   UNUSED(aProgID);                                                            
 
   nsresult rv = NS_OK;
                                                                               
   if (aFactory == NULL){                                                      
       plugin_error("Received a null pointer to pointer in NSGetFactory!\n");  
       return NS_ERROR_UNEXPECTED;                                             
   }                                                                           
   if (aClass.Equals(kPluginCID)) {                                            
           rv = JavaPluginFactory5::Create(pProvider,
                                           NS_GET_IID(nsIFactory),
                                           (void **)aFactory);
   } else {
       rv = NS_ERROR_NO_INTERFACE;
   } 

   return rv;
}                                                              
