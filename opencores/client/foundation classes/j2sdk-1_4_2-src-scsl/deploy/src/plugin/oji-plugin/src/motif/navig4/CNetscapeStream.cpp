/*
 * @(#)CNetscapeStream.cpp	1.7 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
//
// CNetscapeStream.cpp  by Stanley Man-Kit Ho
//
//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
//
// Contains implementation of the CNetscapeStream
//

#include <stdio.h>
#include <stdlib.h>
#include "Navig4.h"


#define TRACE_NSTREAM(s) if (tracing) trace("CNetscapeStream %s\n", s);

const long CNetscapeStream::kMaxBufSize = 0X0FFFFFFF;

//======================================================================
//		CNetscapeStream::CNetscapeStream
//======================================================================

CNetscapeStream::CNetscapeStream(nsIPluginStreamPeer* stream)
:	mStreamPeer( stream )
{
   	TRACE_NSTREAM("CNetscapeStream::CNetscapeStream");

	NS_INIT_REFCNT();

	mStreamPeer->AddRef();
}

//======================================================================
//		CNetscapeStream::~CNetscapeStream
//======================================================================

CNetscapeStream::~CNetscapeStream()
{
   	TRACE_NSTREAM("CNetscapeStream::~CNetscapeStream\n");

	if (mStreamPeer != NULL)
		mStreamPeer->Release();
}


NS_DEFINE_IID(kStreamIID, NS_IBASESTREAM_IID);
NS_DEFINE_IID(kPluginStreamIID, NS_IPLUGINSTREAM_IID);
NS_DEFINE_IID(kOutputStreamIID, NS_IOUTPUTSTREAM_IID);
NS_DEFINE_IID(kSupportsIID, NS_ISUPPORTS_IID);
NS_IMPL_ADDREF(CNetscapeStream);
NS_IMPL_RELEASE(CNetscapeStream);

/**
 * A run time mechanism for interface discovery.
 * @param aIID [in] A requested interface IID
 * @param aInstancePtr [out] A pointer to an interface pointer to
 * receive the result.
 * @return <b>NS_OK</b> if the interface is supported by the associated
 * instance, <b>NS_NOINTERFACE</b> if it is not. 
 * <b>NS_ERROR_INVALID_POINTER</b> if <i>aInstancePtr</i> is <b>NULL</b>.
 */
nsresult CNetscapeStream::QueryInterface(REFNSIID aIID, void** aInstancePtr)  {

    TRACE_NSTREAM("CNetscapeStream::QueryInterface\n");

    if (NULL == aInstancePtr)                                         
	return NS_ERROR_NULL_POINTER;  

    if (aIID.Equals(kStreamIID) || aIID.Equals(kPluginStreamIID) 
	     || aIID.Equals(kOutputStreamIID) || aIID.Equals(kSupportsIID))  {
		*aInstancePtr = (void*) this;
		AddRef();

		return NS_OK;		
	}
	else
		return NS_NOINTERFACE;
}


//======================================================================
//		CNetscapeStream::Write
//======================================================================
NS_IMETHODIMP
CNetscapeStream::Write(const char* buffer, PRInt32 offset, PRInt32 len,
		       PRInt32 *bytesWritten)
{
   	if (tracing) 
	    trace("CNetscapeStream::Write %d %d \n", (int) offset, (int) len);
	*bytesWritten = len;
        UNUSED(buffer);
	return NS_OK;
}

NS_IMETHODIMP CNetscapeStream::Close(void) {
    if (tracing) 
	trace("CNetscapeStream::Close\n");
    return NS_OK;
}

//======================================================================
//		CNetscapeStream::GetStreamType
//======================================================================

// (Corresponds to NPP_NewStream's stype return parameter.)
NS_IMETHODIMP
CNetscapeStream::GetStreamType(nsPluginStreamType *result)
{
    if (tracing)
   	trace("CNetscapeStream::GetStreamType\n");

    *result =  nsPluginStreamType_Normal;
    return NS_OK;
}

//======================================================================
//		CNetscapeStream::CNetscapeStream
//======================================================================

// (Corresponds to NPP_StreamAsFile.)
NS_IMETHODIMP
CNetscapeStream::AsFile(const char* fname)
{ 
    if (tracing) trace("CNetscapeStream::AsFile %s\n", fname);
    return NS_OK;
}
