/*
 * @(#)CJavaStream.h	1.6 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 *  Taken from Stanley Ho's Windows version.
 * 
 */

#ifndef CJAVASTREAM_H
#define CJAVASTREAM_H

#define MAX_PATH 4096

class CJavaPluginInstance;

class CJavaStream : public CNetscapeStream 
{

public:
    CJavaStream( CJavaPluginInstance* pPlugin, nsIPluginStreamPeer* stream);

    virtual ~CJavaStream();

    // Handle a piece of data in the stream
    NS_IMETHOD Write(const char* buffer, PRInt32 offset, PRInt32
			       len, PRInt32 *bytesWritten);
    
    // (Corresponds to NPP_NewStream's stype return parameter.)
    NS_IMETHOD GetStreamType(nsPluginStreamType *result);

    NS_IMETHOD AsFile(const char *fname);

protected:
    // Plugin factory object
    CJavaPluginInstance*  m_PluginInstance;		

    // Number of times write is called
    long  m_iCount;

    // URL of the stream data
    char* m_pszURL;	
};

#endif
