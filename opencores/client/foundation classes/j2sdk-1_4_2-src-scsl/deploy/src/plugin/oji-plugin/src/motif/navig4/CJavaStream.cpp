/*
 * @(#)CJavaStream.cpp	1.24 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * Modified from Stanley Ho's Windows version by RS, BG.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include "Navig4.h"

/*
 * Construct and initialize a new stream
 */
CJavaStream::CJavaStream(CJavaPluginInstance* pPluginInstance,
			 nsIPluginStreamPeer* peer):	
    CNetscapeStream(peer), m_PluginInstance(pPluginInstance), m_iCount(0)
{
    trace("CJavaStream::CJavaStream Creating a new stream\n");

    const char* szURL;
    peer->GetURL(&szURL);

    if (szURL == NULL) {

	m_pszURL = NULL;  // Patch for Opera Browser

    } else 
    {
	int len = slen(szURL);
	m_pszURL = (char*) malloc(sizeof(char) * len);
	memcpy(m_pszURL, szURL, len);
	m_pszURL[len] = '\0';
    }
}


/*
 * Destroy the stream
 */
CJavaStream::~CJavaStream()
{
    trace("CJavaStream::~CJavaStream\n");

    free(m_pszURL);
}

/*
 * StreamAsFile is called to return the filename in the cache.
 */
NS_IMETHODIMP CJavaStream::AsFile(const char *) 
{
    return NS_OK;
}


/* 
 * Write is called to handle a piece of data in the plugin stream.
 */
NS_IMETHODIMP CJavaStream::Write(const char* buffer, PRInt32 offset, PRInt32
				 len, PRInt32 *bytesWritten)  
{
    /* 
       Notice that the offset only represents where the buffer is in 
       the entire stream, NOT where the data is in the buffer. Therefore,
       offset should never never be used, because the buffer already 
       contains all the data we need. 
    */

    (void) offset; // defeat compiler "not used" warning.
    nsresult errorResult = NS_OK;
    int key;
    *bytesWritten = 0;

    // Get the notify key, which we use to indicate the type of request
    // from the stream peer 
    if (NS_OK != (errorResult = mStreamPeer->GetNotifyData((void **)&key)))
	return errorResult;

    // Dump the write request data
    if (tracing) {
	char head[81];
	int clen = slen(m_pszURL);
	if (clen > 30) {
	    clen = 30;
	}
	memcpy(head, m_pszURL, clen);
	head[clen] = 0;
        trace("CJavaStream::NPP_Write \"%s\" %d\n", head, len);
	if (len > 70) {	
	    memcpy(head, buffer, 70);
	    head[70] = 0;
            fprintf(stderr, "%s...\n", head);
	} else {
	    memcpy(head, buffer, len);
	    head[len] = 0;
            fprintf(stderr, "%s\n", head);
	}	
    }

    if (key == JAVA_PLUGIN_DOCBASE_QUERY) {
	// We've just executed a javascript command to find the docbase.
	// Send the result to the java process.
	m_PluginInstance->SetDocbase(buffer);	
	return errorResult;

    } else if (key == JAVA_PLUGIN_JAVASCRIPT_REQUEST) {

	// A general java script reply has arrived
	m_PluginInstance->JavascriptReply(buffer);

    } else {
        trace(" Other Stream Write %X \n",  key);
    }
    *bytesWritten = len;

    // Number of bytes accepted
    return errorResult;        
}

/*
 * Corresponds to NPP_NewStream's stype return parameter.
 */
NS_IMETHODIMP CJavaStream::GetStreamType(nsPluginStreamType *result)
{

    int key;
    nsresult res;

    trace("CJavaStream::GetStreamType\n");

    // Get the notify data
    if ( NS_OK != (res = mStreamPeer->GetNotifyData((void **) &key)))
	return res;

    return CNetscapeStream::GetStreamType(result);
}

