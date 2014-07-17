/*
 * @(#)CCryptoService.cpp	1.3 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

//=--------------------------------------------------------------------------=
// CCryptoService.cpp    by Stanley Man-Kit Ho
//=--------------------------------------------------------------------------=
//
// CCryptoService is a C++ class that encapsulate the MS Crypto APIs
//

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0400
#endif


#include <jni.h>
#include <windows.h>
#include <Wincrypt.h>
#include <stdio.h>

/////////////////////////////////////////////////////////////////////////////
//

/*
 * Class:     com_sun_javaws_security_WSeedGenerator
 * Method:    generateSeed
 * Signature: (I)[B
 */
extern "C" {

JNIEXPORT jbyteArray JNICALL Java_com_sun_javaws_security_WSeedGenerator_generateSeed
  (JNIEnv *env, jclass, jint num)
{
    HCRYPTPROV hCryptProv = NULL;
    BYTE*        pbData = new BYTE[num];
    jbyteArray	 result = NULL;
  
    __try
    {
	//  Acquire a CSP context.
	if(::CryptAcquireContext(    
	   &hCryptProv,
	   "JavaWebStart",
	   NULL,
	   PROV_RSA_FULL,
	   0) == FALSE) 
	{
	    // If CSP context hasn't been created, create one.
	    //
	    if (::CryptAcquireContext(    
	        &hCryptProv,
	        "JavaWebStart",
	        NULL,
	        PROV_RSA_FULL,
	        CRYPT_NEWKEYSET) == FALSE)
	    {
		__leave;
	    }
	}

	// Generate a random initialization vector.
	if(::CryptGenRandom(
	   hCryptProv, 
	   num, 
	   pbData) == FALSE) 
	{
	    __leave;
	}

	result = env->NewByteArray(num);
	env->SetByteArrayRegion(result, 0, num, (jbyte*) pbData);
    }
    __finally
    {
	//--------------------------------------------------------------------
	// Clean up.

	if (pbData)
	    delete [] pbData;
    
	if (hCryptProv)
	    ::CryptReleaseContext(hCryptProv, 0);
    }

    return result;
}


}

