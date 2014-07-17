/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * The contents of this file are subject to the Netscape Public License
 * Version 1.0 (the "NPL"); you may not use this file except in
 * compliance with the NPL.  You may obtain a copy of the NPL at
 * http://www.mozilla.org/NPL/
 *
 * Software distributed under the NPL is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the NPL
 * for the specific language governing rights and limitations under the
 * NPL.
 *
 * The Initial Developer of this code under the NPL is Netscape
 * Communications Corporation.  Portions created by Netscape are
 * Copyright (C) 1998 Netscape Communications Corporation.  All Rights
 * Reserved.
 */

////////////////////////////////////////////////////////////////////////////////
// Backward Adapter
// This acts as a adapter layer to allow 5.0 plugins work with the 4.0/3.0 
// browser.
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// SECTION 1 - Includes
////////////////////////////////////////////////////////////////////////////////

//extern "C" {

// Mozilla changes: Compiler seems to complain if this is not here...
#include "string.h"

#ifdef XP_UNIX
#define _UINT32
#define _INT32
#else // XP_UNIX
//};
#include "npapi.h"
#endif
//include "nsplugin.h"
#include "nsIPluginManager.h"
#include "nsIPluginInstancePeer.h"
#include "nsIPluginTagInfo.h"
#include "nsIPlugin.h"
#include "nsIOutputStream.h"
#include "nsIPluginStreamPeer.h"
#include "nsIFactory.h"
#include "nsIPluginInstance.h"
#include "nsRepository.h"
#include "nsIPluginStream.h"
#include "pluginversion.h"
#include "IUniqueIdentifier.h"

#include "nsDebug.h"
#ifdef XP_UNIX
#include "npapi.h"
#endif

#define UNUSED(x) x=x

////////////////////////////////////////////////////////////////////////////////
// SECTION 3 - Classes
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
//
// CPluginManager
//
// This is the dummy plugin manager that interacts with the 5.0 plugin.
//
class CPluginManager : public nsIPluginManager {
public:

    CPluginManager(void);
    virtual ~CPluginManager(void);

    NS_DECL_ISUPPORTS

    ////////////////////////////////////////////////////////////////////////////
    // from nsIPluginManager:

    // (Corresponds to NPN_GetValue.)
    NS_IMETHOD
    GetValue(nsPluginManagerVariable variable, void *value);

    // (Corresponds to NPN_SetValue.)
    NS_IMETHOD
    SetValue(nsPluginManagerVariable variable, void *value);
    
    NS_IMETHOD
    ReloadPlugins(PRBool reloadPages);

    // (Corresponds to NPN_UserAgent.)
    NS_IMETHOD
    UserAgent(const char* *result);

    NS_IMETHOD
    GetURL(nsISupports* inst, const char* url, const char* target,
           void* notifyData = NULL, const char* altHost = NULL,
           const char* referrer = NULL, PRBool forceJSEnabled = PR_FALSE);

    NS_IMETHOD
    PostURL(nsISupports* inst, const char* url, const char* target,
            PRUint32 postDataLen, const char* postData,
            PRBool isFile = PR_FALSE, void* notifyData = NULL,
            const char* altHost = NULL, const char* referrer = NULL,
            PRBool forceJSEnabled = PR_FALSE,
            PRUint32 postHeadersLength = 0, const char* postHeaders = NULL);
};

////////////////////////////////////////////////////////////////////////////////
//
// CPluginManagerStream
//
// This is the dummy plugin manager stream that interacts with the 5.0 plugin.
//
class CPluginManagerStream : public nsIOutputStream {

public:

    CPluginManagerStream(NPP npp, NPStream* pstr);
    virtual ~CPluginManagerStream(void);

    NS_DECL_ISUPPORTS

    //////////////////////////////////////////////////////////////////////////
    //
    // Taken from nsIStream
    //
    
    /** Write data into the stream.
     *  @param aBuf the buffer into which the data is read
     *  @param aOffset the start offset of the data
     *  @param aCount the maximum number of bytes to read
     *  @param errorResult the error code if an error occurs
     *  @return number of bytes read or -1 if error
     */   
    NS_IMETHOD
    Write(const char* aBuf, PRInt32 aOffset, PRInt32 len, PRInt32 *count); 

    //////////////////////////////////////////////////////////////////////////
    //
    // Specific methods to nsIPluginManagerStream.
    //
    
    // Corresponds to NPStream's url field.
    NS_IMETHOD
    GetURL(const char*  *result);

    // Corresponds to NPStream's end field.
    NS_IMETHOD
    GetEnd(PRUint32 *result);

    // Corresponds to NPStream's lastmodfied field.
    NS_IMETHOD
    GetLastModified(PRUint32 *result);

    // Corresponds to NPStream's notifyData field.
    NS_IMETHOD
    GetNotifyData(void*  *result);

    // Corresponds to NPStream's url field.
    NS_IMETHOD Close(void);

protected:

    // npp
    // The plugin instance that the manager stream belongs to.
    NPP npp;

    // pstream
    // The stream the class is using.
    NPStream* pstream;

};

////////////////////////////////////////////////////////////////////////////////
//
// CPluginInstancePeer
//
// This is the dummy instance peer that interacts with the 5.0 plugin.
// In order to do LiveConnect, the class subclasses nsILiveConnectPluginInstancePeer.
//
class CPluginInstancePeer : public nsIPluginInstancePeer, public nsIPluginTagInfo {

public:

    // XXX - I add parameters to the constructor because I wasn't sure if
    // XXX - the 4.0 browser had the npp_instance struct implemented.
    // XXX - If so, then I can access npp_instance through npp->ndata.
    CPluginInstancePeer(NPP npp, nsMIMEType typeString, nsPluginMode type,
        PRUint16 attribute_cnt, const char** attribute_list, const char** values_list);
    virtual ~CPluginInstancePeer(void);

    NS_DECL_ISUPPORTS

    // (Corresponds to NPN_GetValue.)
    NS_IMETHOD
    GetValue(nsPluginInstancePeerVariable variable, void *value);

    // (Corresponds to NPN_SetValue.)
    NS_IMETHOD
    SetValue(nsPluginInstancePeerVariable variable, void *value);

    // Corresponds to NPP_New's MIMEType argument.
    NS_IMETHOD
    GetMIMEType(nsMIMEType *result);

    // Corresponds to NPP_New's mode argument.
    NS_IMETHOD
    GetMode(nsPluginMode *result);

    // Get a ptr to the paired list of attribute names and values,
    // returns the length of the array.
    //
    // Each name or value is a null-terminated string.
    NS_IMETHOD
    GetAttributes(PRUint16& n, const char* const*& names, const char* const*& values);

    // Get the value for the named attribute.  Returns null
    // if the attribute was not set.
    NS_IMETHOD
    GetAttribute(const char* name, const char* *result);

    // Corresponds to NPN_NewStream.
    NS_IMETHOD
    NewStream(nsMIMEType type, const char* target, nsIOutputStream* *result);

    // Corresponds to NPN_ShowStatus.
    NS_IMETHOD
    ShowStatus(const char* message);

    /**
     * Set the desired size of the window in which the plugin instance lives.
     *
     * @param width - new window width
     * @param height - new window height
     * @result - NS_OK if this operation was successful
     */
    NS_IMETHOD
    SetWindowSize(PRUint32 width, PRUint32 height);   
    
    // XXX - Where did this go?
    NS_IMETHOD
    Version(int* plugin_major, int* plugin_minor,
                      int* netscape_major, int* netscape_minor);

	NPP GetNPPInstance(void)   {
		return npp;
	}
protected:

    NPP npp;
    // XXX - The next five variables may need to be here since I
    // XXX - don't think np_instance is available in 4.0X.
    nsMIMEType typeString;
	nsPluginMode type;
	PRUint16 attribute_cnt;
	char** attribute_list;
	char** values_list;
};

////////////////////////////////////////////////////////////////////////////////
//
// CPluginStreamPeer
//
// This is the dummy stream peer that interacts with the 5.0 plugin.
//
class CPluginStreamPeer : public nsIPluginStreamPeer {

public:
    
    CPluginStreamPeer(nsMIMEType type, NPStream* npStream,
		PRUint16* stype);
    virtual ~CPluginStreamPeer();

    NS_DECL_ISUPPORTS

    // (Corresponds to NPStream's url field.)
    NS_IMETHOD
    GetURL(const char* *result);

    // (Corresponds to NPStream's end field.)
    NS_IMETHOD
    GetEnd(PRUint32 *result);

    // (Corresponds to NPStream's lastmodified field.)
    NS_IMETHOD
    GetLastModified(PRUint32 *result);

    // (Corresponds to NPStream's notifyData field.)
    NS_IMETHOD
    GetNotifyData(void* *result);

	//////////////////////////////////////////////////////////////////////////
    //
    // From nsIPluginStreamPeer
    //

    // Corresponds to NPP_DestroyStream's reason argument.
    NS_IMETHOD
    GetReason(nsPluginReason *result);

    // Corresponds to NPP_NewStream's MIMEType argument.
    NS_IMETHOD
    GetMIMEType(nsMIMEType *result);

protected:

	nsMIMEType type;
	NPStream* npStream;
	PRUint16* stype;
    nsPluginReason reason;

};

//////////////////////////////////////////////////////////////////////////////

#ifdef XP_UNIX
#define TRACE(foo) trace(foo)
#endif

#ifdef XP_MAC
#undef assert
#define assert(cond)
#endif

//#if defined(__cplusplus)
//extern "C" {
//#endif

////////////////////////////////////////////////////////////////////////////////
// SECTION 1 - Includes
////////////////////////////////////////////////////////////////////////////////

#if defined(XP_UNIX) || defined(XP_MAC)
#include <stdio.h>
#include <stdlib.h>
//#include <string.h>
#else
#include <windows.h>
#endif

////////////////////////////////////////////////////////////////////////////////
// SECTION 2 - Global Variables
////////////////////////////////////////////////////////////////////////////////

//
// thePlugin and thePluginManager are used in the life of the plugin.
//
// These two will be created on NPP_Initialize and destroyed on NPP_Shutdown.
//
nsIPluginManager* thePluginManager = NULL;
nsIPlugin* thePlugin = NULL;

//
// nsISupports IDs
//
// Interface IDs for nsISupports
//
NS_DEFINE_CID(kPluginCID, NS_PLUGIN_CID);
 
NS_DEFINE_IID(kISupportsIID, NS_ISUPPORTS_IID);
NS_DEFINE_IID(kIPluginIID, NS_IPLUGIN_IID);
NS_DEFINE_IID(kIPluginInstanceIID, NS_IPLUGININSTANCE_IID);
NS_DEFINE_IID(kIPluginManagerIID, NS_IPLUGINMANAGER_IID);
NS_DEFINE_IID(kIPluginTagInfoIID, NS_IPLUGINTAGINFO_IID);
NS_DEFINE_IID(kIOutputStreamIID, NS_IOUTPUTSTREAM_IID);
NS_DEFINE_IID(kIPluginInstancePeerIID, NS_IPLUGININSTANCEPEER_IID); 
NS_DEFINE_IID(kIPluginStreamPeerIID, NS_IPLUGINSTREAMPEER_IID);
NS_DEFINE_IID(kIEventHandlerIID, NS_IEVENTHANDLER_IID);
NS_DEFINE_IID(kIUniqueIdentifierIID, UNIQUE_IDENTIFIER_IID);

// mapping from NPError to nsresult
nsresult fromNPError[] = {
    NS_OK,                          // NPERR_NO_ERROR,
    NS_ERROR_FAILURE,               // NPERR_GENERIC_ERROR,
    NS_ERROR_FAILURE,               // NPERR_INVALID_INSTANCE_ERROR,
    NS_ERROR_NOT_INITIALIZED,       // NPERR_INVALID_FUNCTABLE_ERROR,
    NS_ERROR_FACTORY_NOT_LOADED,    // NPERR_MODULE_LOAD_FAILED_ERROR,
    NS_ERROR_OUT_OF_MEMORY,         // NPERR_OUT_OF_MEMORY_ERROR,
    NS_NOINTERFACE,                 // NPERR_INVALID_PLUGIN_ERROR,
    NS_ERROR_ILLEGAL_VALUE,         // NPERR_INVALID_PLUGIN_DIR_ERROR,
    NS_NOINTERFACE,                 // NPERR_INCOMPATIBLE_VERSION_ERROR,
    NS_ERROR_ILLEGAL_VALUE,         // NPERR_INVALID_PARAM,
    NS_ERROR_ILLEGAL_VALUE,         // NPERR_INVALID_URL,
    NS_ERROR_ILLEGAL_VALUE,         // NPERR_FILE_NOT_FOUND,
    NS_ERROR_FAILURE,               // NPERR_NO_DATA,
    NS_ERROR_FAILURE                // NPERR_STREAM_NOT_SEEKABLE,
};

////////////////////////////////////////////////////////////////////////////////
// SECTION 4 - API Shim Plugin Implementations
// Glue code to the 5.0x Plugin.
//
// Most of the NPP_* functions that interact with the plug-in will need to get 
// the instance peer from npp->pdata so it can get the plugin instance from the
// peer. Once the plugin instance is available, the appropriate 5.0 plug-in
// function can be called:
//          
//  CPluginInstancePeer* peer = (CPluginInstancePeer* )instance->pdata;
//  nsIPluginInstance* inst = peer->GetUserInstance();
//  inst->NewPluginAPIFunction();
//
// Similar steps takes place with streams.  The stream peer is stored in NPStream's
// pdata.  Get the peer, get the stream, call the function.
//

////////////////////////////////////////////////////////////////////////////////
// UNIX-only API calls
////////////////////////////////////////////////////////////////////////////////

#ifdef XP_UNIX
NPError NPP_SetValue(NPP instance, NPNVariable variable, void *value);

extern
nsresult JPI_NSGetFactory(nsISupports * pProvider,
                          const nsCID &aClass,
                          const char * aClassName,
                          const char * aProgID,
                          nsIFactory **aFactory);

char* NPP_GetMIMEDescription(void)
{
  return (char *) PLUGIN_MIMETABLE;

  /*
    int freeFac = 0;
    fprintf(stderr, "MIME description\n");
    if (thePlugin == NULL) {
        fprintf(stderr, "Geting plug-in\n");
        freeFac = 1;
        JPI_NSGetFactory(NULL,
			 kPluginCID, 
			 NULL,  
			 NULL,
			 (nsIFactory** )(&thePlugin));
    }
    fprintf(stderr, "Allocated Plugin 0x%08x\n", thePlugin);
    const char * ret;
    nsresult err = thePlugin->GetMIMEDescription(&ret);
    if (err) return NULL;
    fprintf(stderr, "Get response %s\n", ret);
    if (freeFac) {
        fprintf(stderr, "Freeing plugin...");
        thePlugin->Release();
        thePlugin = NULL;
    }
    fprintf(stderr, "Done\n");
    return (char*)ret;

    */
}


NPError
NPP_GetValue(NPP instance, NPPVariable variable, void *value) {
    int freeFac = 0;
    //fprintf(stderr, "MIME description\n");
    if (thePlugin == NULL) {
        freeFac = 1;
        if (JPI_NSGetFactory(NULL,
			     kPluginCID, 
			     NULL,
			     NULL,
			     (nsIFactory** )(&thePlugin)) != NS_OK)
            return NPERR_GENERIC_ERROR;
    }
    //fprintf(stderr, "Allocated Plugin 0x%08x\n", thePlugin);
    nsresult err = thePlugin->GetValue((nsPluginVariable)variable, value);
    if (err) return NPERR_GENERIC_ERROR;
    //fprintf(stderr, "Get response %08x\n", ret);
    if (freeFac) {
        //fprintf(stderr, "Freeing plugin...");
        thePlugin->Release();
        thePlugin = NULL;
    }
    //fprintf(stderr, "Done\n");
    UNUSED(instance);
    return NPERR_NO_ERROR;
}
//+++++++++++++++++++++++++++++++++++++++++++++++++
// NPP_SetValue:
//+++++++++++++++++++++++++++++++++++++++++++++++++

NPError
NPP_SetValue(NPP instance, NPNVariable variable, void *value)
{
    UNUSED(instance);
    UNUSED(variable);
    UNUSED(value);
    return NPERR_GENERIC_ERROR; // nothing to set
}

#endif // XP_UNIX

//+++++++++++++++++++++++++++++++++++++++++++++++++
// NPP_Initialize:
// Provides global initialization for a plug-in, and returns an error value. 
//
// This function is called once when a plug-in is loaded, before the first instance
// is created. thePluginManager and thePlugin are both initialized.
//+++++++++++++++++++++++++++++++++++++++++++++++++

NPError
NPP_Initialize(void)
{
//    TRACE("NPP_Initialize\n");

    // Only call initialize the plugin if it hasn't been created.
    // This could happen if GetJavaClass() is called before
    // NPP Initialize.  
    if (thePluginManager == NULL) {
        // Create the plugin manager and plugin classes.
        thePluginManager = new CPluginManager();	
        if ( thePluginManager == NULL ) 
            return NPERR_OUT_OF_MEMORY_ERROR;  
        thePluginManager->AddRef();
    }
    nsresult error = NS_OK;  
    // On UNIX the plugin might have been created when calling NPP_GetMIMEType.
    if (thePlugin == NULL) {
        // create nsIPlugin factory
        error = (NPError)JPI_NSGetFactory(NULL,
					  kPluginCID, 
					  NULL,
					  NULL,
					  (nsIFactory**) &thePlugin);
        if (NS_SUCCEEDED(error))
          thePlugin->Initialize(thePluginManager);
    }
    return (NPError) error;	
}

//+++++++++++++++++++++++++++++++++++++++++++++++++
// NPP_GetJavaClass:
// New in Netscape Navigator 3.0. 
// 
// NPP_GetJavaClass is called during initialization to ask your plugin
// what its associated Java class is. If you don't have one, just return
// NULL. Otherwise, use the javah-generated "use_" function to both
// initialize your class and return it. If you can't find your class, an
// error will be signalled by "use_" and will cause the Navigator to
// complain to the user.
//+++++++++++++++++++++++++++++++++++++++++++++++++

jref
NPP_GetJavaClass(void)
{
    // Only call initialize the plugin if it hasn't been `d.
    /*  if (thePluginManager == NULL) {
        // Create the plugin manager and plugin objects.
        NPError result = CPluginManager::Create();	
        if (result) return NULL;
	    assert( thePluginManager != NULL );
        thePluginManager->AddRef();
        NP_CreatePlugin(thePluginManager, (nsIPlugin** )(&thePlugin));
        assert( thePlugin != NULL );
        }
        */
//	return thePlugin->GetJavaClass();
	return NULL;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++
// NPP_Shutdown:
// Provides global deinitialization for a plug-in. 
// 
// This function is called once after the last instance of your plug-in 
// is destroyed.  thePluginManager and thePlugin are delete at this time.
//+++++++++++++++++++++++++++++++++++++++++++++++++

void
NPP_Shutdown(void)
{
//	TRACE("NPP_Shutdown\n");

	if (thePlugin)
	{
		thePlugin->Shutdown();
		thePlugin->Release();
		thePlugin = NULL;
	}

	if (thePluginManager)  {
		thePluginManager->Release();
		thePluginManager = NULL;
	}
    
    return;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++
// NPP_New:
// Creates a new instance of a plug-in and returns an error value. 
// 
// A plugin instance peer and instance peer is created.  After
// a successful instansiation, the peer is stored in the plugin
// instance's pdata.
//+++++++++++++++++++++++++++++++++++++++++++++++++

NPError 
NPP_New(NPMIMEType pluginType,
	NPP instance,
	PRUint16 mode,
	int16 argc,
	char* argn[],
	char* argv[],
	NPSavedData* saved)
{
//    TRACE("NPP_New\n");
    
    if (instance == NULL)
        return NPERR_INVALID_INSTANCE_ERROR;

    // Create a new plugin instance and start it.
    nsIPluginInstance* pluginInstance = NULL;
    thePlugin->CreateInstance(NULL, kIPluginInstanceIID, (void**)&pluginInstance);
    if (pluginInstance == NULL) {
        return NPERR_OUT_OF_MEMORY_ERROR;
    } 
    
    // Create a new plugin instance peer,
    // XXX - Since np_instance is not implemented in the 4.0x browser, I
    // XXX - had to save the plugin parameter in the peer class.
    // XXX - Ask Warren about np_instance.
    CPluginInstancePeer* peer = 
        new CPluginInstancePeer(instance, (nsMIMEType)pluginType, 
                                (nsPluginMode)mode, (PRUint16)argc, 
                                (const char** )argn, (const char** )argv);
    assert( peer != NULL );
    if (!peer) return NPERR_OUT_OF_MEMORY_ERROR;
    peer->AddRef();
	IUniqueIdentifier* pInst = NULL;
	if(NS_SUCCEEDED(pluginInstance->QueryInterface(kIUniqueIdentifierIID, (void**)&pInst))) {
		long id = 0;
		if(NULL != saved) {
			id = saved->len;
			NPN_MemFree(saved);
		}
		pInst->SetUniqueId(id);
		pInst->Release();
	}

    pluginInstance->Initialize(peer);
    pluginInstance->Start();
    // Set the user instance and store the peer in npp->pdata.
    instance->pdata = pluginInstance;
    peer->Release();
    UNUSED(saved);

    return NPERR_NO_ERROR;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++
// NPP_Destroy:
// Deletes a specific instance of a plug-in and returns an error value. 
//
// The plugin instance peer and plugin instance are destroyed.
// The instance's pdata is set to NULL.
//+++++++++++++++++++++++++++++++++++++++++++++++++

NPError 
NPP_Destroy(NPP instance, NPSavedData** save)
{
//    TRACE("NPP_Destroy\n");
    
    if (instance == NULL)
        return NPERR_INVALID_INSTANCE_ERROR;
    
    nsIPluginInstance* pluginInstance = (nsIPluginInstance* )instance->pdata;
	IUniqueIdentifier* pInst = NULL;
	if(NS_SUCCEEDED(pluginInstance->QueryInterface(kIUniqueIdentifierIID, (void**)&pInst))) {
		*save = (NPSavedData*)NPN_MemAlloc(sizeof(NPSavedData));
		long id;

		pInst->GetUniqueId(&id);
		(*save)->buf = NULL;
		(*save)->len = id;
		pInst->Release();
	}

    pluginInstance->Stop();
    pluginInstance->Destroy();
    pluginInstance->Release();
    instance->pdata = NULL;
    
    return NPERR_NO_ERROR;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++
// NPP_SetWindow:
// Sets the window in which a plug-in draws, and returns an error value. 
//+++++++++++++++++++++++++++++++++++++++++++++++++

NPError 
NPP_SetWindow(NPP instance, NPWindow* window)
{
//    TRACE("NPP_SetWindow\n");
    
    if (instance == NULL)
        return NPERR_INVALID_INSTANCE_ERROR;

    nsIPluginInstance* pluginInstance = (nsIPluginInstance* )instance->pdata;
    
    if( pluginInstance == 0 )
        return NPERR_INVALID_PLUGIN_ERROR;

    return (NPError)pluginInstance->SetWindow((nsPluginWindow* ) window );
}

//+++++++++++++++++++++++++++++++++++++++++++++++++
// NPP_NewStream:
// Notifies an instance of a new data stream and returns an error value. 
//
// Create a stream peer and stream.  If succesful, save
// the stream peer in NPStream's pdata.
//+++++++++++++++++++++++++++++++++++++++++++++++++

NPError 
NPP_NewStream(NPP instance,
	      NPMIMEType type,
	      NPStream *stream, 
	      NPBool seekable,
	      PRUint16 *stype)
{
    // XXX - How do you set the fields of the peer stream and stream?
    // XXX - Looks like these field will have to be created since
    // XXX - We are not using np_stream.
   
//    TRACE("NPP_NewStream\n");

    UNUSED(seekable);
    if (instance == NULL)
        return NPERR_INVALID_INSTANCE_ERROR;
				
    // Create a new plugin stream peer and plugin stream.
    CPluginStreamPeer* speer = new CPluginStreamPeer((nsMIMEType)type, stream,
                                                     stype); 
    if (speer == NULL) return NPERR_OUT_OF_MEMORY_ERROR;
    speer->AddRef();
    nsIPluginStream* pluginStream = NULL; 
    nsIPluginInstance* pluginInstance = (nsIPluginInstance*) instance->pdata;
    nsresult err = pluginInstance->NewStream(speer, &pluginStream);
    if (err) return NPERR_OUT_OF_MEMORY_ERROR;
    speer->Release();
    
    if (pluginStream == NULL)
        return NPERR_OUT_OF_MEMORY_ERROR;
		
    stream->pdata = (void*) pluginStream;
    nsPluginStreamType stype1;
    err = pluginStream->GetStreamType(&stype1);
    *stype = (PRUint16) (stype1);
    assert(err == NS_OK);
	
    return NPERR_NO_ERROR;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++
// NPP_WriteReady:
// Returns the maximum number of bytes that an instance is prepared to accept
// from the stream. 
//+++++++++++++++++++++++++++++++++++++++++++++++++

int32 
NPP_WriteReady(NPP instance, NPStream *stream)
{
//    TRACE("NPP_WriteReady\n");

    if (instance == NULL)
        return -1;

    nsIPluginStream* theStream = (nsIPluginStream*) stream->pdata;	
    if( theStream == 0 )
        return -1;
	
    return 8192;
}


//+++++++++++++++++++++++++++++++++++++++++++++++++
// NPP_Write:
// Delivers data from a stream and returns the number of bytes written. 
//+++++++++++++++++++++++++++++++++++++++++++++++++

int32 
NPP_Write(NPP instance, NPStream *stream, int32 offset, int32 len, void *buffer)
{
//    TRACE("NPP_Write\n");

    if (instance == NULL)
        return -1;
	
    nsIPluginStream* theStream = (nsIPluginStream*) stream->pdata;
    if( theStream == 0 )
        return -1;
	
	PRInt32 count = 0;
	
	if (NS_SUCCEEDED(theStream->Write((const char* )buffer, offset, len, &count)))
		return count;
	else
		return -1;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++
// NPP_DestroyStream:
// Indicates the closure and deletion of a stream, and returns an error value. 
//
// The stream peer and stream are destroyed.  NPStream's
// pdata is set to NULL.
//+++++++++++++++++++++++++++++++++++++++++++++++++

NPError 
NPP_DestroyStream(NPP instance, NPStream *stream, NPReason reason)
{
//    TRACE("NPP_DestroyStream\n");

    if (instance == NULL)
        return NPERR_INVALID_INSTANCE_ERROR;
		
    nsIPluginStream* theStream = (nsIPluginStream*) stream->pdata;
    if( theStream == 0 )
        return NPERR_GENERIC_ERROR;
	
    theStream->Release();
    stream->pdata = NULL;
    UNUSED(reason);
	
    return NPERR_NO_ERROR;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++
// NPP_StreamAsFile:
// Provides a local file name for the data from a stream. 
//+++++++++++++++++++++++++++++++++++++++++++++++++

void 
NPP_StreamAsFile(NPP instance, NPStream *stream, const char* fname)
{
//	TRACE("NPP_StreamAsFile\n");

	if (instance == NULL)
		return;
		
	nsIPluginStream* theStream = (nsIPluginStream*) stream->pdata;
	if( theStream == 0 )
		return;

	theStream->AsFile( fname );
}

//+++++++++++++++++++++++++++++++++++++++++++++++++
// NPP_Print:
//+++++++++++++++++++++++++++++++++++++++++++++++++

void 
NPP_Print(NPP instance, NPPrint* printInfo)
{
//    TRACE("NPP_Print\n");

    if(printInfo == NULL)   // trap invalid parm
        return;

	if (instance != NULL)
	{
		nsIPluginInstance* pluginInstance = (nsIPluginInstance*) instance->pdata;
		pluginInstance->Print((nsPluginPrint* ) printInfo );
	}
}

//+++++++++++++++++++++++++++++++++++++++++++++++++
// NPP_URLNotify:
// Notifies the instance of the completion of a URL request. 
//+++++++++++++++++++++++++++++++++++++++++++++++++

void
NPP_URLNotify(NPP instance, const char* url, NPReason reason, void* notifyData)
{
//   	TRACE("NPP_URLNotify\n");

	if( instance != NULL )
	{
	    nsIPluginInstance* pluginInstance = (nsIPluginInstance*) instance->pdata;
		pluginInstance->URLNotify(url, NULL, (nsPluginReason)reason, notifyData);

	}
}
 

//////////////////////////////////////////////////////////////////////////////
// SECTION 5 - API Browser Implementations
//
// Glue code to the 4.0x Browser.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
//
// CPluginManager
//

//******************************************************************************
//
// Once we moved to the new APIs, we need to implement fJVMMgr.
//
//******************************************************************************

CPluginManager::CPluginManager(void) 
{
    // Set reference count to 0.
    NS_INIT_REFCNT();
}

CPluginManager::~CPluginManager(void) 
{
}

#if 0
//+++++++++++++++++++++++++++++++++++++++++++++++++
// MemAlloc:
//+++++++++++++++++++++++++++++++++++++++++++++++++

NS_METHOD
CPluginManager::MemAlloc(PRUint32 size)
{
    return NPN_MemAlloc(size);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++
// MemFree:
//+++++++++++++++++++++++++++++++++++++++++++++++++

NS_METHOD
CPluginManager::MemFree(void* ptr)
{
    assert( ptr != NULL );

    NPN_MemFree(ptr);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++
// MemFlush:
//+++++++++++++++++++++++++++++++++++++++++++++++++

NS_METHOD
CPluginManager::MemFlush(PRUint32 size)
{
#ifdef XP_MAC
	return NPN_MemFlush(size);	
#else
	return 0;
#endif
}
#endif

//+++++++++++++++++++++++++++++++++++++++++++++++++
// ReloadPlugins:
//+++++++++++++++++++++++++++++++++++++++++++++++++

NS_METHOD
CPluginManager::ReloadPlugins(PRBool reloadPages)
{
	NPN_ReloadPlugins(reloadPages);
    return NS_OK;
}


// (Corresponds to NPN_GetURL and NPN_GetURLNotify.)
//   notifyData: When present, URLNotify is called passing the notifyData back
//          to the client. When NULL, this call behaves like NPN_GetURL.
// New arguments:
//   peer:  A plugin instance peer. The peer's window will be used to display
//          progress information. If NULL, the load happens in the background.
//   altHost: An IP-address string that will be used instead of the host
//          specified in the URL. This is used to prevent DNS-spoofing attacks.
//          Can be defaulted to NULL meaning use the host in the URL.
//   referrer: 
//   forceJSEnabled: Forces JavaScript to be enabled for 'javascript:' URLs,
//          even if the user currently has JavaScript disabled. 
NS_METHOD
CPluginManager::GetURL(nsISupports* inst, const char* url, const char* target,
                       void* notifyData, const char* altHost,
                       const char* referrer, PRBool forceJSEnabled)
{
    // assert( npp != NULL );
    // assert( url != NULL );
 	assert( inst != NULL);


    nsIPluginInstance* pPluginInstance = NULL;
    nsIPluginInstancePeer* pPluginInstancePeer = NULL;

    if (NS_FAILED(inst->QueryInterface(kIPluginInstanceIID, (void**) &pPluginInstance)))
    {
        return NS_ERROR_FAILURE;
    }

    if (NS_FAILED(pPluginInstance->GetPeer(&pPluginInstancePeer)))
    {
        pPluginInstance->Release();
        return NS_ERROR_FAILURE;
    }

	CPluginInstancePeer* instancePeer = (CPluginInstancePeer*)pPluginInstancePeer;
	NPP npp = instancePeer->GetNPPInstance();

    pPluginInstance->Release();
    pPluginInstancePeer->Release();
    NPError err;
    // Call the correct GetURL* function.
    // This is determinded by checking notifyData.
    if (notifyData == NULL) {
        err = NPN_GetURL(npp, url, target);
    } else {
        err = NPN_GetURLNotify(npp, url, target, notifyData);
    }
    UNUSED(altHost);
    UNUSED(referrer);
    UNUSED(forceJSEnabled);
    return fromNPError[err];
}


NS_METHOD
CPluginManager::PostURL(nsISupports* inst, const char* url, const char* target,
                        PRUint32 postDataLen, const char* postData,
                        PRBool isFile, void* notifyData,
                        const char* altHost, const char* referrer,
                        PRBool forceJSEnabled,
                        PRUint32 postHeadersLength, const char* postHeaders)
{
    // assert( npp != NULL );
    // assert( url != NULL );
 	assert( inst != NULL);

    nsIPluginInstance* pPluginInstance = NULL;
    nsIPluginInstancePeer* pPluginInstancePeer = NULL;

    if (NS_FAILED(inst->QueryInterface(kIPluginInstanceIID, (void**) &pPluginInstance)))
    {
        return NS_ERROR_FAILURE;
    }

    if (NS_FAILED(pPluginInstance->GetPeer(&pPluginInstancePeer)))
    {
        pPluginInstance->Release();
        return NS_ERROR_FAILURE;
    }

	CPluginInstancePeer* instancePeer = (CPluginInstancePeer*)pPluginInstancePeer;
	NPP npp = instancePeer->GetNPPInstance();

    pPluginInstance->Release();
    pPluginInstancePeer->Release();
    NPError err;
    // Call the correct PostURL* function.
    // This is determinded by checking notifyData.
    if (notifyData == NULL) {
        err = NPN_PostURL(npp, url, target, postDataLen, postData, isFile);
    } else {
        err = NPN_PostURLNotify(npp, url, target, postDataLen, postData, isFile, notifyData);
    }
    UNUSED(altHost);
    UNUSED(referrer);
    UNUSED(forceJSEnabled);
    UNUSED(postHeadersLength);
    UNUSED(postHeaders);
    return fromNPError[err];
}


//+++++++++++++++++++++++++++++++++++++++++++++++++
// UserAgent:
//+++++++++++++++++++++++++++++++++++++++++++++++++

NS_METHOD
CPluginManager::UserAgent(const char* *result)
{
    *result = NPN_UserAgent(NULL);
    return NS_OK;
}

#ifdef XP_UNIX
int varMap[] = {
    0,
    (int) NPNVxDisplay,
    (int) NPNVxtAppContext,
    0
};
/*
int varMap[] = {
    0, // everything seems to be 1-based
    (int)NPNVxDisplay,                  // nsPluginManagerVariable_XDisplay = 1,
    (int)NPNVxtAppContext,              // nsPluginManagerVariable_XtAppContext,
    (int)NPNVnetscapeWindow,            // nsPluginManagerVariable_NetscapeWindow,
    (int)NPPVpluginWindowBool,          // nsPluginInstancePeerVariable_WindowBool,
    (int)NPPVpluginTransparentBool,     // nsPluginInstancePeerVariable_TransparentBool,
    (int)NPPVjavaClass,                 // nsPluginInstancePeerVariable_JavaClass,
    (int)NPPVpluginWindowSize,          // nsPluginInstancePeerVariable_WindowSize,
    (int)NPPVpluginTimerInterval,       // nsPluginInstancePeerVariable_TimerInterval
};
*/
#endif  // XP_UNIX

//+++++++++++++++++++++++++++++++++++++++++++++++++
// GetValue:
//+++++++++++++++++++++++++++++++++++++++++++++++++

NS_METHOD
CPluginManager::GetValue(nsPluginManagerVariable variable, void *value)
{
#ifdef XP_UNIX
    return fromNPError[NPN_GetValue(NULL, (NPNVariable)varMap[(int)variable], value)];
#else
    return fromNPError[NPERR_GENERIC_ERROR];
#endif // XP_UNIX
}

//+++++++++++++++++++++++++++++++++++++++++++++++++
// SetValue:
//+++++++++++++++++++++++++++++++++++++++++++++++++

NS_METHOD
CPluginManager::SetValue(nsPluginManagerVariable variable, void *value) 
{
    UNUSED(variable);
    UNUSED(value);
#ifdef XP_UNIX
    return (NS_METHOD)NULL;
    /*
    return fromNPError[NPN_SetValue(NULL, (NPPVariable)varMap[(int)variable], value)];
    */
#else
    return fromNPError[NPERR_GENERIC_ERROR];
#endif // XP_UNIX
}

//+++++++++++++++++++++++++++++++++++++++++++++++++
// nsISupports functions
//+++++++++++++++++++++++++++++++++++++++++++++++++

NS_IMPL_ADDREF(CPluginManager);
NS_IMPL_RELEASE(CPluginManager);

NS_METHOD
CPluginManager::QueryInterface(const nsIID& iid, void** ptr) 
{                                                                        
    if (NULL == ptr) {                                            
        return NS_ERROR_NULL_POINTER;                                        
    }                                                                      
  
    if (iid.Equals(kIPluginManagerIID)) {
        *ptr = (void*) (nsIPluginManager*)this;                                        
        AddRef();                                                            
        return NS_OK;                                                        
    }                                                                      
    if (iid.Equals(kISupportsIID)) {                                      
        *ptr = (void*) ((nsIPluginManager*)this);                        
        AddRef();                                                            
        return NS_OK;                                                        
    }                                                                      
    return NS_NOINTERFACE;                                                 
}


//////////////////////////////////////////////////////////////////////////////
//
// CPluginInstancePeer
//

CPluginInstancePeer::CPluginInstancePeer(NPP npp,
                                         nsMIMEType typeString, 
                                         nsPluginMode type,
                                         PRUint16 attr_cnt, 
                                         const char** attr_list, 
                                         const char** val_list)
    : npp(npp), typeString(typeString), type(type), attribute_cnt((PRUint16)NULL),
    attribute_list(NULL), values_list(NULL)
{
    // Set the reference count to 0.
    NS_INIT_REFCNT();

	attribute_list = (char**) NPN_MemAlloc(attr_cnt * sizeof(const char*));
	values_list = (char**) NPN_MemAlloc(attr_cnt * sizeof(const char*));

	if (attribute_list != NULL && values_list != NULL) {
		for (int i = 0, j = 0; i < attr_cnt; i++)   {
            if (attr_list[i] != NULL && val_list[i] != NULL) {           
                
                attribute_list[j] = (char*) NPN_MemAlloc(strlen(attr_list[i]) + 1);
                if (attribute_list[j] != NULL)
                    strcpy(attribute_list[j], attr_list[i]);

                values_list[j] = (char*) NPN_MemAlloc(strlen(val_list[i]) + 1);
                if (values_list[j] != NULL)
                    strcpy(values_list[j], val_list[i]);

                j = j + 1;
                attribute_cnt = j;
            }
		}
	}
}

CPluginInstancePeer::~CPluginInstancePeer(void) 
{
	if (attribute_list != NULL && values_list != NULL) {
		for (int i = 0; i < attribute_cnt; i++)   {
			NPN_MemFree(attribute_list[i]);
			NPN_MemFree(values_list[i]);
		}

		NPN_MemFree(attribute_list);
		NPN_MemFree(values_list);
	}
}   


//+++++++++++++++++++++++++++++++++++++++++++++++++
// GetValue:
//+++++++++++++++++++++++++++++++++++++++++++++++++

NS_METHOD
CPluginInstancePeer::GetValue(nsPluginInstancePeerVariable variable, void *value)
{
#ifdef XP_UNIX
    return fromNPError[NPN_GetValue(NULL, (NPNVariable)varMap[(int)variable], value)];
#else
    return fromNPError[NPERR_GENERIC_ERROR];
#endif // XP_UNIX
}

//+++++++++++++++++++++++++++++++++++++++++++++++++
// SetValue:
//+++++++++++++++++++++++++++++++++++++++++++++++++

NS_METHOD
CPluginInstancePeer::SetValue(nsPluginInstancePeerVariable variable, void *value) 
{
    UNUSED(variable);
    UNUSED(value);
#ifdef XP_UNIX
    return (NS_METHOD)NULL;
    /*
    return fromNPError[NPN_SetValue(NULL, (NPPVariable)varMap[(int)variable], value)];
    */
#else
    return fromNPError[NPERR_GENERIC_ERROR];
#endif // XP_UNIX
}

//+++++++++++++++++++++++++++++++++++++++++++++++++
// GetMIMEType:
// Corresponds to NPP_New's MIMEType argument.
//+++++++++++++++++++++++++++++++++++++++++++++++++

NS_METHOD
CPluginInstancePeer::GetMIMEType(nsMIMEType *result) 
{
    *result = typeString;
    return NS_OK;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++
// GetMode:
// Corresponds to NPP_New's mode argument.
//+++++++++++++++++++++++++++++++++++++++++++++++++

NS_METHOD
CPluginInstancePeer::GetMode(nsPluginMode *result)
{
    *result = type;
    return NS_OK;
}


// Get a ptr to the paired list of attribute names and values,
// returns the length of the array.
//
// Each name or value is a null-terminated string.
NS_METHOD
CPluginInstancePeer::GetAttributes(PRUint16& n, const char* const*& names, const char* const*& values)  
{
	n = attribute_cnt;
	names = attribute_list;
	values = values_list;

	return NS_OK;
}

#if defined(XP_MAC)

inline unsigned char toupper(unsigned char c)
{
	return (c >= 'a' && c <= 'z') ? (c - ('a' - 'A')) : c;
}

static int strcasecmp(const char * str1, const char * str2)
{
#if __POWERPC__
	
	const	unsigned char * p1 = (unsigned char *) str1 - 1;
	const	unsigned char * p2 = (unsigned char *) str2 - 1;
				unsigned long		c1, c2;
		
	while (toupper(c1 = *++p1) == toupper(c2 = *++p2))
		if (!c1)
			return(0);

#else
	
	const	unsigned char * p1 = (unsigned char *) str1;
	const	unsigned char * p2 = (unsigned char *) str2;
				unsigned char		c1, c2;
	
	while (toupper(c1 = *p1++) == toupper(c2 = *p2++))
		if (!c1)
			return(0);

#endif
	
	return(toupper(c1) - toupper(c2));
}

#endif /* XP_MAC */

// Get the value for the named attribute.  Returns null
// if the attribute was not set.
NS_METHOD
CPluginInstancePeer::GetAttribute(const char* name, const char* *result) 
{
	for (int i=0; i < attribute_cnt; i++)  {
#if defined(XP_UNIX) || defined(XP_MAC)
		if (strcasecmp(name, attribute_list[i]) == 0)
#else
        if (stricmp(name, attribute_list[i]) == 0) 
#endif
        {
            *result = values_list[i];
            return NS_OK;
        }
	}

	return NS_ERROR_FAILURE;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++
// Version:
//
// XXX - Where did this go in the new API?
//
//+++++++++++++++++++++++++++++++++++++++++++++++++

NS_METHOD
CPluginInstancePeer::Version(int* plugin_major, int* plugin_minor,
                             int* netscape_major, int* netscape_minor)
{
    NPN_Version(plugin_major, plugin_minor, netscape_major, netscape_minor);
	return NS_OK;
}


//+++++++++++++++++++++++++++++++++++++++++++++++++
// NewStream:
//+++++++++++++++++++++++++++++++++++++++++++++++++
NS_METHOD
CPluginInstancePeer::NewStream(nsMIMEType mtype, const char* target, 
                                     nsIOutputStream* *result)
{
    assert( npp != NULL );
    
    // Create a new NPStream.
    NPStream* ptr = NULL;
    NPError error = NPN_NewStream(npp, (NPMIMEType)mtype, target, &ptr);
    if (error) 
        return fromNPError[error];
    
    // Create a new Plugin Manager Stream.
    // XXX - Do we have to Release() the manager stream before doing this?
    // XXX - See the BAM doc for more info.
    CPluginManagerStream* mstream = new CPluginManagerStream(npp, ptr);
    if (mstream == NULL) 
        return NS_ERROR_OUT_OF_MEMORY;
    mstream->AddRef();
    *result = (nsIOutputStream* )mstream;

    return NS_OK;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++
// ShowStatus:
//+++++++++++++++++++++++++++++++++++++++++++++++++

NS_METHOD
CPluginInstancePeer::ShowStatus(const char* message)
{
    assert( message != NULL );

    NPN_Status(npp, message);
	return NS_OK;
}

NS_METHOD
CPluginInstancePeer::SetWindowSize(PRUint32 width, PRUint32 height)
{
    UNUSED(width);
    UNUSED(height);
#ifdef XP_MAC
    NPError err;
    NPSize size;
    size.width = width;
    size.height = height;
    err = NPN_SetValue(npp, NPPVpluginWindowSize, &size);
    return fromNPError[err];
#else
	return NS_ERROR_NOT_IMPLEMENTED;
#endif
}

//+++++++++++++++++++++++++++++++++++++++++++++++++
// nsISupports functions
//+++++++++++++++++++++++++++++++++++++++++++++++++

NS_IMPL_ADDREF(CPluginInstancePeer);
NS_IMPL_RELEASE(CPluginInstancePeer);

NS_METHOD
CPluginInstancePeer::QueryInterface(const nsIID& iid, void** ptr) 
{                                                                        
    if (NULL == ptr) {                                            
        return NS_ERROR_NULL_POINTER;                                        
    }                                                                      
  
    if (iid.Equals(kIPluginInstancePeerIID)) {
        *ptr = (void*) this;                                        
        AddRef();                                                            
        return NS_OK;                                                        
    }                                                                      
    if (iid.Equals(kIPluginTagInfoIID) || iid.Equals(kISupportsIID)) {                                      
        *ptr = (void*) ((nsIPluginTagInfo*)this);                        
        AddRef();                                                            
        return NS_OK;                                                        
    }                                                                      
    return NS_NOINTERFACE;                                                 
}

//////////////////////////////////////////////////////////////////////////////
//
// CPluginManagerStream
//

CPluginManagerStream::CPluginManagerStream(NPP npp, NPStream* pstr)
    : npp(npp), pstream(pstr)
{
    // Set the reference count to 0.
    NS_INIT_REFCNT();
}

CPluginManagerStream::~CPluginManagerStream(void)
{
    //pstream = NULL;
    NPN_DestroyStream(npp, pstream, NPRES_DONE);
}


//+++++++++++++++++++++++++++++++++++++++++++++++++
// Write:
//+++++++++++++++++++++++++++++++++++++++++++++++++

NS_METHOD
CPluginManagerStream::Write(const char* buffer, PRInt32 offset, PRInt32 len, PRInt32 *count)
{
    assert( npp != NULL );
    assert( pstream != NULL );
	assert( count != NULL );

    assert(offset == 0);    // XXX need to handle the non-sequential write case
    
	*count = NPN_Write(npp, pstream, len, (void* )buffer);

    return *count >= 0 ? NS_OK : NS_ERROR_FAILURE;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++
// GetURL:
//+++++++++++++++++++++++++++++++++++++++++++++++++

NS_METHOD
CPluginManagerStream::GetURL(const char* *result)
{
    assert( pstream != NULL );

    *result = pstream->url;
	return NS_OK;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++
// GetEnd:
//+++++++++++++++++++++++++++++++++++++++++++++++++

NS_METHOD
CPluginManagerStream::GetEnd(PRUint32 *result)
{
    assert( pstream != NULL );

    *result = pstream->end;
	return NS_OK;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++
// GetLastModified:
//+++++++++++++++++++++++++++++++++++++++++++++++++

NS_METHOD
CPluginManagerStream::GetLastModified(PRUint32 *result)
{
    assert( pstream != NULL );

    *result = pstream->lastmodified;
	return NS_OK;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++
// GetNotifyData:
//+++++++++++++++++++++++++++++++++++++++++++++++++

NS_METHOD
CPluginManagerStream::GetNotifyData(void* *result)
{
    assert( pstream != NULL );

    *result = pstream->notifyData;
	return NS_OK;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++
// GetNotifyData:
//+++++++++++++++++++++++++++++++++++++++++++++++++

NS_METHOD
CPluginManagerStream::Close(void)
{
    assert( pstream != NULL );

    return NS_OK;
}


//+++++++++++++++++++++++++++++++++++++++++++++++++
// nsISupports functions
//+++++++++++++++++++++++++++++++++++++++++++++++++

NS_IMPL_ADDREF(CPluginManagerStream);
NS_IMPL_RELEASE(CPluginManagerStream);

NS_IMPL_QUERY_INTERFACE(CPluginManagerStream, kIOutputStreamIID);

//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
//
// CPluginStreamPeer
//

CPluginStreamPeer::CPluginStreamPeer(nsMIMEType type, NPStream* npStream,
									 PRUint16* stype)
	: type(type), npStream(npStream),
	  stype(stype), reason(nsPluginReason_NoReason)
{
    // Set the reference count to 0.
    NS_INIT_REFCNT();
}

CPluginStreamPeer::~CPluginStreamPeer(void)
{
}

//+++++++++++++++++++++++++++++++++++++++++++++++++
// GetURL:
//+++++++++++++++++++++++++++++++++++++++++++++++++

NS_METHOD
CPluginStreamPeer::GetURL(const char* *result)
{
    *result = npStream->url;
    return NS_OK;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++
// GetEnd:
//+++++++++++++++++++++++++++++++++++++++++++++++++

NS_METHOD
CPluginStreamPeer::GetEnd(PRUint32 *result)
{
    *result = npStream->end;
    return NS_OK;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++
// GetLastModified:
//+++++++++++++++++++++++++++++++++++++++++++++++++

NS_METHOD
CPluginStreamPeer::GetLastModified(PRUint32 *result)
{
    *result = npStream->lastmodified;
    return NS_OK;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++
// GetNotifyData:
//+++++++++++++++++++++++++++++++++++++++++++++++++

NS_METHOD
CPluginStreamPeer::GetNotifyData(void* *result)
{
    *result = npStream->notifyData;
    return NS_OK;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++
// GetReason:
//+++++++++++++++++++++++++++++++++++++++++++++++++

NS_METHOD
CPluginStreamPeer::GetReason(nsPluginReason *result)
{
    *result = reason;
    return NS_OK;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++
// GetMIMEType:
//+++++++++++++++++++++++++++++++++++++++++++++++++

NS_METHOD
CPluginStreamPeer::GetMIMEType(nsMIMEType *result)
{
    *result = type;
    return NS_OK;
}


//+++++++++++++++++++++++++++++++++++++++++++++++++
// nsISupports functions
//+++++++++++++++++++++++++++++++++++++++++++++++++

NS_IMPL_ADDREF(CPluginStreamPeer);
NS_IMPL_RELEASE(CPluginStreamPeer);

nsresult CPluginStreamPeer::QueryInterface(const nsIID& iid, void** ptr) 
{
    if (NULL == ptr) {
        return NS_ERROR_NULL_POINTER; 
    } 
	if (iid.Equals(kIPluginStreamPeerIID) ||
			   iid.Equals(kISupportsIID)) {
        *ptr = (void*) ((nsIPluginStreamPeer*)this); 
        AddRef(); 
        return NS_OK; 
    } 
    return NS_NOINTERFACE; 
} 

//////////////////////////////////////////////////////////////////////////////

//#if defined(__cplusplus)
//} /* extern "C" */
//#endif

