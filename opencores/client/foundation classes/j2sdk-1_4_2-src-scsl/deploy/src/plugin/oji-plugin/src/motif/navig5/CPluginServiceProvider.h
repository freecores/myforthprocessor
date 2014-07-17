/*
 * @(#)CPluginServiceProvider.h	1.9 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

//
// CPluginServiceProvider.h  by Stanley Man-Kit Ho
//
//
// Contains declaration of the CPluginServiceProvider
//

#ifndef CPLUGINSERVICEPROVIDER_H
#include "nsIServiceManager.h"

#define CPLUGINSERVICEPROVIDER_H


class CPluginServiceProvider {
public:

    // from nsIPluginServiceProvider:

    /**
     * Obtain a plugin service. An nsIPluginServiceProvider will obtain the 
     * plugin service according to the current browser version.
     *
     * @param clsid - the CLSID of the requested service.
     * @param iid - the IID of the request service.
     * @param result - the interface pointer of the requested service
     * @return - NS_OK if this operation was successful.
     */
    NS_IMETHOD
    QueryService(/*[in]*/  const nsCID& clsid, 
		 /*[in]*/  const nsIID& iid,
                 /*[out]*/ nsISupports* *result);


    /**
     * Release a plugin service. An nsIPluginServiceProvider will release the 
     * plugin service according to the current browser version.
     *
     * @param clsid - the CLSID of the service.
     * @param result - the interface pointer of the service
     * @return - NS_OK if this operation was successful.
     */
    NS_IMETHOD
    ReleaseService(/*[in]*/ const nsCID& clsid, 
		   /*[in]*/ nsISupports* pService);


    // CPluginServiceProvider specific methods:

    CPluginServiceProvider(nsISupports* pProvoder);
    virtual ~CPluginServiceProvider(void);

protected:
    nsIServiceManager* mService;
    nsISupports* mMgr;

    // For caching purpose
    nsISupports* mPluginManager;
    nsISupports* mJVMManager;
    nsISupports* mCookieStorage;
};

#endif // CPLUGINSERVICEPROVIDER_H


