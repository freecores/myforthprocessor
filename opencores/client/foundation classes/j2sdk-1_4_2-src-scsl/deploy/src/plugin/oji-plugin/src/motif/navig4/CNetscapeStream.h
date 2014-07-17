/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-*/
/*
 * @(#)CNetscapeStream.h	1.4 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
//
// CNetscapeStream.h  by Stanley Man-Kit Ho
//
//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
//
// Contains declaration of the CNetscapeStream
//

#ifndef CNETSCAPESTREAM_H
#define CNETSCAPESTREAM_H


class CNetscapeStream : public nsIPluginStream
{
public:
    CNetscapeStream(nsIPluginStreamPeer* peer);
    virtual	~CNetscapeStream();
		
    //================================================================
    // nsISupport
    //================================================================
    NS_DECL_ISUPPORTS		

    //================================================================
    // nsIStream
    //================================================================

    NS_METHOD Close(void);

    // (Corresponds to nsP_Write and nsN_Write.)
    NS_IMETHOD Write(const char* buffer, PRInt32 offset, PRInt32 len, PRInt32 *bytesWritten);

    //================================================================
    // nsIPluginStream
    //================================================================

    // (Corresponds to nsP_NewStream's stype return parameter.)
    NS_IMETHOD GetStreamType(nsPluginStreamType *result);

    // (Corresponds to nsP_StreamAsFile.)
    NS_IMETHOD AsFile(const char* fname);		
protected:
    static const long	 kMaxBufSize;

    nsIPluginStreamPeer* mStreamPeer;
};

#endif
