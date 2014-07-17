/*
 * @(#)CSecurityContext.cpp	1.16 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

///=------------------------------------------------------------------------=
//
// CSecurityContext.cpp  by Stanley Man-Kit Ho
//
///=------------------------------------------------------------------------=
//
// These functions create object, invoke method, get/set field in JNI with
// security context.
//
 
#include "nsplugin.h"
#include "nsAgg.h"
#include "nsISecurityContext.h"
#include "CSecurityContext.h"

#include "Debug.h"

#if !defined(UNUSED)
#define UNUSED(x) x=x
#endif


// Mozilla changes : Added a semicolon at the end
NS_DEFINE_IID(kISecurityContextIID, NS_ISECURITYCONTEXT_IID);
NS_DEFINE_IID(kISupportsIID, NS_ISUPPORTS_IID);

// This macro expands to the aggregated query interface scheme.
NS_IMPL_AGGREGATED(CSecurityContext);

NS_METHOD
CSecurityContext::AggregatedQueryInterface(const nsIID& aIID, void** aInstancePtr)
{
    if (aIID.Equals(kISupportsIID)) {
      *aInstancePtr = GetInner();
      AddRef();
      return NS_OK;
    }
    if (aIID.Equals(kISecurityContextIID)) {
        *aInstancePtr = (nsISecurityContext *)this;
        AddRef();
        return NS_OK;
    }
    return NS_NOINTERFACE;
}



// Implements the CSecurityContext object for encapsulating security context.
// This security context object encapuslates the security associated with
// a particular JS call from Java. 
CSecurityContext::CSecurityContext(nsISupports *aOuter, 
				   const char* lpszURL,
                                   PRBool isAllPermission) : m_isAllPermission(isAllPermission)
{
    trace("CSecurityContext::CSecurityContext\n");

    NS_INIT_AGGREGATED(aOuter);
	m_lpszURL = NULL;

    if (lpszURL != NULL) {
		m_lpszURL = new char[strlen(lpszURL) + 1];
        strcpy(m_lpszURL, lpszURL);
	}
}


// Destroy the security context. Who does this?
CSecurityContext::~CSecurityContext()  
{
    trace("CSecurityContext::~CSecurityContext\n");
	if(NULL != m_lpszURL)
		delete[] m_lpszURL;
}


// Create the CSecurityContext object for creating object, invoking method, 
// getting/setting field in JNI with security context.
NS_METHOD
CSecurityContext::Create(nsISupports* outer, 
			 const char* lpszURL,PRBool isAllPermission, 
			 const nsIID& aIID, void* *aInstancePtr)
{
    /* If this is an aggregated object creation (i.e. outer exists)
       then we expect to return the special object Internal, which
       provides the nondelegating functions */

    if (outer && !aIID.Equals(kISupportsIID))
        return NS_NOINTERFACE;   // XXX right error?
    CSecurityContext* context = new CSecurityContext(outer, lpszURL, isAllPermission);
    if (context == NULL)
        return NS_ERROR_OUT_OF_MEMORY;
    context->AddRef();
    *aInstancePtr = (outer != NULL)
	?(void *)context->GetInner()
	:(void *)context;
    return NS_OK;
}


////////////////////////////////////////////////////////////////////////////
// from nsISecurityContext:
//

// Get the security context to be used in LiveConnect.
// This is used for JavaScript <--> Java.
//
// @param target        -- Possible target.
// @param action        -- Possible action on the target.
// @return              -- NS_OK if the action is permitted 
//                      -- NS_FALSE otherwise.
NS_IMETHODIMP CSecurityContext::Implies(const char* target, 
					const char* action, 
					PRBool* bActionAllowed)
{
    trace("CSecurityContext::Implies\n");

    UNUSED(action);

    if (target == NULL || bActionAllowed == NULL)
        return NS_ERROR_NULL_POINTER;

    *bActionAllowed = m_isAllPermission;

    return NS_OK;
}


////////////////////////////////////////////////////////////////////////////
// from nsISecurityContext:
//

// Get the origin associated with the context.
//
// @param buf        -- Result buffer (managed by the caller.)
// @param len        -- Buffer length.
// @return           -- NS_OK if the origin string was obtained.
//                   -- NS_FALSE otherwise.

NS_IMETHODIMP CSecurityContext::GetOrigin(char* buf, int len)
{
    trace("CSecurityContext::GetOrigin\n");
    
    if (buf == NULL)
        return NS_ERROR_NULL_POINTER;

    if (((unsigned int) len) <= strlen(m_lpszURL))
        return NS_ERROR_FAILURE;

    // Copy origin
    strcpy(buf, m_lpszURL);


    return NS_OK;
}


////////////////////////////////////////////////////////////////////////////
// from nsISecurityContext:
//

// Get the certificate associated with the context.
// 
//  @param buf        -- Result buffer (managed by the caller.)
//  @param len        -- Buffer length.
//  @return           -- NS_OK if the certificate string was obtained.
//                    -- NS_FALSE otherwise.

NS_IMETHODIMP CSecurityContext::GetCertificateID(char* buf, int len)

{
    trace("CSecurityContext::GetCertificateID\n");

    UNUSED(buf);
    UNUSED(len);

    return NS_OK;
}

   

