/*
 * @(#)CPluginServiceProvider.cpp	1.14 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

//------------------------------------------------------------------------------------
//
// CPluginServiceProvider.cpp  by Stanley Man-Kit Ho
//
//------------------------------------------------------------------------------------
//
// Contains implementation of the CPluginServiceProvider
//


#include "nsplugin.h"
#include "nsAgg.h"
#include "nsIPluginManager.h"
#include "nsIJVMManager.h"
// Mozilla changes
#include "CPluginServiceProvider.h"
#include "Debug.h"
#include "utils.h"
#include "nsICookieStorage.h"
#include <stdlib.h>

#include "nsIServiceManager.h"

//------------------------------------------------------------------------------------
// nsISupports Methods
//------------------------------------------------------------------------------------

static NS_DEFINE_IID(kIServiceManagerIID, NS_ISERVICEMANAGER_IID);
static NS_DEFINE_IID(kISupportsIID, NS_ISUPPORTS_IID);
static NS_DEFINE_IID(kIPluginManagerIID, NS_IPLUGINMANAGER_IID);
static NS_DEFINE_IID(kIJVMManagerIID, NS_IJVMMANAGER_IID);
static NS_DEFINE_IID(kICookieStorageIID, NS_ICOOKIESTORAGE_IID);

// Mozilla changes
static NS_DEFINE_CID(kCPluginManagerCID, NS_PLUGINMANAGER_CID);
static NS_DEFINE_CID(kCJVMManagerCID, NS_JVMMANAGER_CID);
static NS_DEFINE_CID(kCCookieStorageCID,NS_COOKIESTORAGE_CID);




//------------------------------------------------------------------------------------
// CPluginServiceProvider Methods
//------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------
// CPluginServiceProvider::CPluginServiceProvider
//------------------------------------------------------------------------------------
CPluginServiceProvider::CPluginServiceProvider(nsISupports* pProvider)
    : mService(NULL), mMgr(NULL), 
      mPluginManager(NULL), mJVMManager(NULL), mCookieStorage(NULL)

{
#ifdef RAPTOR_API
    // Try to obtain nsIServiceManager if running inside Mozilla
    if (NS_FAILED(pProvider->QueryInterface(kIServiceManagerIID, (void**)&mService))) {
      plugin_error("Did not find the service manager!");
    }
#else
    // We are running inside Mozilla classic. The provider is a plugin manager
    mMgr = pProvider;
	
    if (mMgr)
      mMgr->AddRef();
#endif
}


//------------------------------------------------------------------------------------
// CPluginServiceProvider::~CPluginServiceProvider
//------------------------------------------------------------------------------------
CPluginServiceProvider::~CPluginServiceProvider(void)
{
  if (mService != NULL)
    {    
      if (mPluginManager != NULL)
	mService->ReleaseService(kCPluginManagerCID, mPluginManager, NULL);

      if (mJVMManager != NULL)
	mService->ReleaseService(kCJVMManagerCID, mJVMManager, NULL);

      if (mCookieStorage != NULL)
	mService->ReleaseService(kCCookieStorageCID, mCookieStorage, NULL);

      mService->Release();
    }
}


//------------------------------------------------------------------------------------
// CPluginServiceProvider::QueryService
//------------------------------------------------------------------------------------
//
// Obtain a plugin service. An nsIPluginServiceProvider will obtain the 
// plugin service according to the current browser version.
//
// @param clsid - the CLSID of the requested service.
// @param iid - the IID of the request service.
// @param result - the interface pointer of the requested service
// @return - NS_OK if this operation was successful.
//
// Notes:
//
NS_METHOD
CPluginServiceProvider::QueryService(/*[in]*/  const nsCID& clsid, 
				     /*[in]*/  const nsIID& iid,
				     /*[out]*/ nsISupports* *result) 
{

    if (result == NULL)
	return NS_ERROR_NULL_POINTER;

    *result = NULL;

    nsresult err = NS_OK;

    if (mService != NULL)   {
	// We are running inside Raptor
	//err = mService->GetService(clsid, iid, result, NULL);
	// Mozilla changes: GetService only needs 3 args
	err = mService->GetService(clsid, iid, result);
    }

    return err;
}


//------------------------------------------------------------------------------------
// CPluginServiceProvider::ReleaseService
//------------------------------------------------------------------------------------
// Release a plugin service. An nsIPluginServiceProvider will release the 
// plugin service according to the current browser version.
//
// @param clsid - the CLSID of the service.
// @param result - the interface pointer of the service
// @return - NS_OK if this operation was successful.
//
// Notes:
//
NS_METHOD
CPluginServiceProvider::ReleaseService(/*[in]*/ const nsCID& clsid, 
				       /*[in]*/ nsISupports* pService)
{

    if (pService == NULL)
	return NS_ERROR_NULL_POINTER;

    // We are running inside Raptor and have nsIServiceManager
    if (mService != NULL)
    {
	if (clsid.Equals(kCPluginManagerCID) && mPluginManager != NULL)  {
	    return pService->Release();
	}
	else if (clsid.Equals(kCJVMManagerCID) && mJVMManager != NULL)  {
	  return pService->Release();
	}
	else if (clsid.Equals(kCCookieStorageCID) && mCookieStorage != NULL){
	  return  pService->Release();
	}
	else  {
	    // Unknown service
	    return mService->ReleaseService(clsid, pService, NULL);
	}
    }
    
    return NS_OK;
}

