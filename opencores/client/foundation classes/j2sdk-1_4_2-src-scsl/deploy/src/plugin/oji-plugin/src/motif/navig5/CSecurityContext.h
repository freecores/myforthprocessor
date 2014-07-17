/*
 * @(#)CSecurityContext.h	1.10 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

///=-----------------------------------------------------------------------=
// CSecurityContext.h   Based on win32 version by Stanley Man-Kit Ho
///=------------------------------------------------------------------------=
/* 
   This implementation of the security context is a wrapper around the
   Java protection domain in a Java to JS call. It calls back into
   Java to perform the "implies"

   */
#ifndef CSecurityContext_h___
#define CSecurityContext_h___

#include "jni.h"
#include "nsAgg.h"
#include "nsISecurityContext.h"

class CSecurityContext : public nsISecurityContext
{
public:

    //////////////////////////////////////////////////////////////////////////
    // from nsISupports and AggregatedQueryInterface:

    NS_DECL_AGGREGATED

    static NS_METHOD Create(nsISupports* outer, 
			    const char* lpszURL, PRBool isAllPermission,
                            const nsIID& aIID, 
			    void* *aInstancePtr);

    ////////////////////////////////////////////////////////////////////////
    // from nsISecurityContext:

    /**
     * Get the security context to be used in LiveConnect.
     * This is used for JavaScript <--> Java.
     *
     * @param target        -- Possible target.
     * @param action        -- Possible action on the target.
     * @return              -- NS_OK if the target and action is permitted 
     *                      -- NS_FALSE otherwise.
     */
    NS_IMETHOD Implies(const char* target, const char* action, 
		       PRBool* bActionAllowed);

    /**
     * Get the origin associated with the context.
     *
     * @param buf        -- Result buffer (managed by the caller.)
     * @param len        -- Buffer length.
     * @return           -- NS_OK if the codebase string was obtained.
     *                   -- NS_FALSE otherwise.
     */
    NS_IMETHOD GetOrigin(char* buf, int len);

    /**
     * Get the certificate associated with the context.
     *
     * @param buf        -- Result buffer (managed by the caller.)
     * @param len        -- Buffer length.
     * @return           -- NS_OK if the codebase string was obtained.
     *                   -- NS_FALSE otherwise.
     */
    NS_IMETHOD GetCertificateID(char* buf, int len);

   
    //////////////////////////////////////////////////////////////////////
    // from nsISecureJNI:

    CSecurityContext(nsISupports *aOuter, 
		     const char* lpszURL, PRBool isAllPermission);
    virtual ~CSecurityContext(void);

protected:
    char*	 m_lpszURL;
    PRBool   m_isAllPermission;
};

#endif // CSecurityContext_h___





