/*
 * @(#)CJavaPluginInstance.cpp	1.24 02/03/27
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/* 
 * A Plugin instance is a single instantiation of the plugin, with its
 * own window and its own corresponding MotifAppletViewer. It will
 * share a classloader with other plugin instances that have the same
 * document base (there is a shared table of class loaders indexed by 
 * document base.
 */
#include <stdlib.h>
#include <stdio.h>
#include <X11/Xlib.h>
#include <X11/Intrinsic.h>
#include <X11/Shell.h>

#include "Navig4.h"
#include "CPluginPrint.h"


static const char *s_szDocument_locator_url = "javascript:document.location";

NS_DEFINE_IID(kPluginInstanceIID, NS_IPLUGININSTANCE_IID);
NS_DEFINE_IID(kSupportsIID, NS_ISUPPORTS_IID);
NS_DEFINE_IID(kPluginTagInfoIID, NS_IPLUGINTAGINFO_IID);
NS_DEFINE_IID(kIUniqueIdentifierIID, UNIQUE_IDENTIFIER_IID);

long CJavaPluginInstance::s_uniqueId = 1;
/**
 * Standard QI. Supports EventHandler and PluginInstance.
 */
nsresult 
CJavaPluginInstance::QueryInterface(REFNSIID aIID, void** aInstancePtr)  {

    trace("CJavaPluginInstance::QueryInterface\n");

    if (NULL == aInstancePtr)                                         
	return NS_ERROR_NULL_POINTER;  

	if(aIID.Equals(kIUniqueIdentifierIID)) {
		trace("CJavaPluginInstance::QueryInterface - IUniquePluginInstance\n");
		*aInstancePtr = (void*)((IUniqueIdentifier*)this);
		AddRef();
		return NS_OK;
	}

    if (aIID.Equals(kPluginInstanceIID) || 
	aIID.Equals(kSupportsIID)) {
		trace("CJavaPluginInstance::QueryInterface - nsIPluginInstance\n");
	    *aInstancePtr = (void*)((nsIPluginInstance*) this);
	    AddRef();

	    return NS_OK;		
    } else {
	return NS_NOINTERFACE;
    }
}

NS_IMPL_ADDREF(CJavaPluginInstance);
NS_IMPL_RELEASE(CJavaPluginInstance);

/*
 * Create a new instance - should only be called by the factory 
 */
CJavaPluginInstance::CJavaPluginInstance(CJavaPluginFactory *plugin) 
{
    trace("CJavaPluginInstance::Create\n");

    NS_INIT_REFCNT();
    m_iPlugin_number = -1;
    m_PluginFactory = plugin;
    current_request = NULL;
    mPeer = NULL;
	m_uniqueId = 0;
} 

/* 
 * Start this instance. Just propagate to the JVM. The start
 * may actually have to wait until other information such as
 * the document base etc. are available. See MotifAppletViewer.java
 * maybeInit()
 */
NS_IMETHODIMP 
CJavaPluginInstance::Start(void) 
{
    trace("[%d] CJavaPluginInstance::Start JAVA_PLUGIN_START\n", m_iPlugin_number);
    char buff[8];
    put_int(buff, 0, JAVA_PLUGIN_START);
    put_int(buff, 4, m_iPlugin_number);
    m_PluginFactory->SendRequest(buff, 8, FALSE);
    return NS_OK;
}


/* 
 * Nothing is done here.
 */
NS_IMETHODIMP 
CJavaPluginInstance::HandleEvent(nsPluginEvent* event, PRBool
					       *handled) {
    *handled = PR_FALSE;
    trace("[%d]Handle event %d\n", m_iPlugin_number, event);
    return NS_OK;
}

/* 
 * Get the peer NPIPluginInstancePeer for this NPIPluginInstance
 */
NS_IMETHODIMP 
CJavaPluginInstance::GetPeer(nsIPluginInstancePeer**resultingPeer) 
{
    trace("[%d] CJavaPluginInstance::GetPeer\n", m_iPlugin_number);
    *resultingPeer = mPeer;

    if (mPeer == NULL) 
	trace("CJavaPluginInstance::Peer is null! %d\n", m_iPlugin_number);

    mPeer->AddRef();
    return NS_OK;
}

/*
 * Get the value of some plugin instance variable.
 */
nsresult
CJavaPluginInstance::GetValue(nsPluginInstanceVariable var, void* val) {
    trace("CJavaPluginInstance::GetValue not yet implemented %d %d!",
	  (int) (&(var)), (int) val);
    return NS_OK;
}


/*
 * Initialization essentially consists of determining the document base.
 * This is different for 3.0, 4.0 and 5.0.
 * In 5.0, query the tag info
 * In 4.0 send a DOCBASE_QUERY to the browser
 * In 3.0 the information may be available as an argument, or
 *        otherwise send a DOCBASE_READ, which is handled in URLNotify.
 */
NS_IMETHODIMP 
CJavaPluginInstance::Initialize(nsIPluginInstancePeer *peer) {
    const char *pluginType;
    unsigned short argc;
    char **argn;
    char **argv;
    nsIPluginTagInfo *tagInfo;
    nsIPluginManager* pIPluginManager = NULL;
    nsresult res;

    trace("CJavaPluginInstance::Initialize  %d \n",  (int) peer);
    peer->AddRef(); 
    mPeer = peer;

    // Determine the mime type
    if (NS_OK != (res = peer->GetMIMEType(&pluginType))){
	trace("InstanceInitialize: Could not get MIME type!\n");
	return res; 
    }
    
    // Get information about the plugin's tags in an argc, argv form
    peer->QueryInterface(kPluginTagInfoIID, (void **)&tagInfo);
    if (tagInfo != NULL) {
        tagInfo->GetAttributes(argc, (const char * const *&)argn, 
			       (const char * const*&) argv);
        tagInfo->Release();
    }

    // Register this instance
    m_iPlugin_number = m_PluginFactory->RegisterInstance(this);
    if (m_iPlugin_number < 0) {
	trace("Out of memory in registering instance!\n");
	return NS_ERROR_OUT_OF_MEMORY;
    }

	char** lppszNames = new char*[argc + 1];
	char** lppszValues = new char*[argc + 1];
	
	int nCount = 0;
	int index;
	for(index = 0; index < argc; index ++) {
		if(NULL == argn[index])
			continue;

		lppszNames[nCount] = argn[index];
		lppszValues[nCount] = argv[index];

		nCount ++;
	}

	char szBuf[16];
	sprintf(szBuf, "%x", m_uniqueId);
	lppszNames[nCount] = (char*)UNIQUE_IDENTIFIER_ID;
	lppszValues[nCount] = szBuf;

    // Create the actual applet for this instance
    m_PluginFactory->CreateApplet(pluginType, m_iPlugin_number, 
				  nCount + 1, lppszNames, lppszValues);


    //  We need to figure out our document base.

    // In order to locate our DOCBASE invoke the JavaScript method
    // "document.location" via a "javascript:" URL.
    // We'll get the result back in NPP_Write.
    trace("Get Document base");
    pIPluginManager = m_PluginFactory->GetPluginManager();

    pIPluginManager->GetURL((nsIPluginInstance*)this, s_szDocument_locator_url, 0, 
			    (void *)JAVA_PLUGIN_DOCBASE_QUERY);


	delete[] lppszNames;
	delete[] lppszValues;

    return NS_OK;
}


/* 
 *  Set the window parameters of this plugin instance. Called at
 *  creation and after a resize. 'window' holds dimension and other 
 * information
 */
NS_IMETHODIMP 
CJavaPluginInstance::SetWindow(nsPluginWindow* window) {

    nsPluginSetWindowCallbackStruct *ws_info;
    char buff[40];

    trace("CJavaPluginInstance::SetWindow JAVA_PLUGIN_WINDOW %d\n", 
	  m_iPlugin_number);

    if (window == NULL || window->window == NULL) {
        trace("SetWindow NULL");
        put_int(buff, 0, JAVA_PLUGIN_WINDOW);
        put_int(buff, 4, m_iPlugin_number);
        put_int(buff, 8, 0);
        m_PluginFactory->SendRequest(buff, 28, TRUE);
        return NS_OK;
    }

    // Determine the window dimensions.
    ws_info = (nsPluginSetWindowCallbackStruct *) window->ws_info;
    trace("%d: SetWindow  win=0x%X, x=%d, y=%d, wd=%d, ht=%d, dp= %d,"
	  " ws_info=0x%x\n",
	  m_iPlugin_number,
	  window->window, window->x, window->y, window->width,
	  window->height, ws_info->depth, ws_info);

    // Flush any pending X requests.  This is necessary because window updates
    // affecting the target window may not yet have reached the server, and
    // the javaplugin child process may get out of step.
    XSync(ws_info->display, 0);

    // forward the request to the server 
    put_int(buff, 0, JAVA_PLUGIN_WINDOW);
    put_int(buff, 4, m_iPlugin_number);
    put_int(buff, 8, (int)window->window);
    put_int(buff, 12, window->width);
    put_int(buff, 16, window->height);
    put_int(buff, 20, window->x);
    put_int(buff, 24, window->y);
    m_PluginFactory->SendRequest(buff, 28, TRUE);
    return NS_OK;
}

/*
 * Create a new stream for the browser to write data into.
 */
NS_IMETHODIMP 
CJavaPluginInstance::NewStream(nsIPluginStreamPeer* peer,
					     nsIPluginStream* *result) {

    trace("CJavaPluginInstance::NewStream %d\n", m_iPlugin_number);

    *result = new CJavaStream(this, peer);
    return NS_OK;
}
/*
 * Provides printing support 
 */

NS_IMETHODIMP CJavaPluginInstance::Print(nsPluginPrint* printInfo) {
    CPluginPrint *printer;
    int result;
    nsPluginEmbedPrint ep;
    ep = printInfo->print.embedPrint;
    nsPluginWindow * win= (nsPluginWindow*) &(ep.window);
    if (win == NULL){
	delete printer;
	return NS_OK;
    }
    printer = new CPluginPrint(this, printInfo );
    result= printer->Print();
    delete printer;
    return result;
}

/*
 * Notification as the result of a query into the browser i.e.
 * the return of a GetURL query.
 * The sequence of events is:
 *     GetURLNotify
 *     NewStream, 
 *     A sequence of writes to the stream
 *     Notify the instance that the writes are done
 * The notification happens *after* all writes to the stream
 * have completed.
 */
NS_IMETHODIMP 
CJavaPluginInstance::URLNotify(const char* url, 
			       const char*target, 
			       nsPluginReason reason, 
			       void* notifyData) {
    int key = (int)notifyData;
    if (tracing) {
	char head[31];
	int clen = slen(url);
	if (clen > 30) {
	    clen = 30;
	}
	memcpy(head, url, clen);
	head[clen] = 0;
	UNUSED(target);
        trace("%d: NPP_URLNotify: key=0x%X %s => %d\n", m_iPlugin_number,
	      key, head, reason);
	    
    }
    if (key == JAVA_PLUGIN_JAVASCRIPT_REQUEST) {
	// A general 4.0 JS request has just finished giving us all its data
	char buff[10];
	put_int(buff, 0, JAVA_PLUGIN_JAVASCRIPT_END);
	put_int(buff, 4, m_iPlugin_number);
	m_PluginFactory->SendRequest(buff, 8, FALSE);
    } else if (key == JAVA_PLUGIN_DOCBASE_QUERY) {
	// In 4.0 we get the docbase through a JS request.
    } else {
	trace("[%d] Other URLNotify %X \n", m_iPlugin_number, key);
    }
    return NS_OK;
}


/*
 * Return the associated plugin instance factory.
 */
CJavaPluginFactory * CJavaPluginInstance::GetPluginFactory(void){
    trace("[%d] Getting plugin factory\n", m_iPlugin_number);
    return m_PluginFactory;
}


/*
 * Set the document base for this instance to the url 'url'.
 * This is called either during initialization, if the document
 * base is obtained directly, or later during a call to URLNotify,
 * as the result of one of the asynch calls into the browser.
 * Send the data to the jvm, which might now have all the information
 * required to start an applet.
 */
void 
CJavaPluginInstance::SetDocbase(const char *url) 
{
    trace("[%d] CJavaPluginInstance::SetDocbase JAVA_PLUGIN_DOCBASE %s\n", 
	  m_iPlugin_number, url);

    int len = slen(url);
    char *buff = (char *)malloc(len+20);

    if (url == NULL) 
	plugin_error("Null Docbase?");
   
    put_int(buff, 0, JAVA_PLUGIN_DOCBASE);
    put_int(buff, 4, m_iPlugin_number);
    put_short(buff, 8, len);
    memcpy(buff+10, url, len);
    m_PluginFactory->SendRequest(buff, len+10, FALSE);
    free(buff);
}

/*
 * Reply to a javascript query. No call for this as yet?
 */
void 
CJavaPluginInstance::JavascriptReply(const char *reply) 
{
    trace("[%d] CJavaPluginInstance::JavascriptReply JAVA_PLUGIN_JAVASCRIPT_REPLY\n", m_iPlugin_number);

    int len = slen(reply);    
    char *buff = (char *)malloc(len + 20);
    put_int(buff, 0, JAVA_PLUGIN_JAVASCRIPT_REPLY);
    put_int(buff, 4, m_iPlugin_number);
    put_short(buff, 8, len);
    memcpy(buff+10, reply, len);
    m_PluginFactory->SendRequest(buff, len+10, FALSE);
    free(buff);
}


/* 
 * Stop this instance. Just propagate to the JVM.
 */
NS_IMETHODIMP 
CJavaPluginInstance::Stop(void) 
{
    trace("[%d] CJavaPluginInstance::Stop JAVA_PLUGIN_STOP\n",
	  m_iPlugin_number);

    char buff[8];
    put_int(buff, 0, JAVA_PLUGIN_STOP);
    put_int(buff, 4, m_iPlugin_number);
    m_PluginFactory->SendRequest(buff, 8, FALSE);
    return NS_OK;
}

/* 
 * Destroy this instance. Just propagate to the JVM.
 */
NS_IMETHODIMP 
CJavaPluginInstance::Destroy(void) 
{
    trace("[%d] CJavaPluginInstance::Destroy JAVA_PLUGIN_DESTROY\n",
          m_iPlugin_number);

    // Unregister the instance
    if (m_iPlugin_number >= 0)
    {
        m_PluginFactory->UnregisterInstance(this);
    }

    // Tell the VM end to go die

    char buff[8];
    put_int(buff, 0, JAVA_PLUGIN_DESTROY);
    put_int(buff, 4, m_iPlugin_number);
    m_PluginFactory->SendRequest(buff, 8, TRUE);

    if (current_request != NULL) {
        trace("Destroying instance, abruptly terminating request!");
        char msg[4];
        put_int(msg, 0, JAVA_PLUGIN_REQUEST_ABRUPTLY_TERMINATED);
        m_PluginFactory->SendRequest(msg, 4, FALSE);
    }

    m_iPlugin_number = -1;


    // The object destructor will be called when refs go to zero
    trace("CJavaPluginInstance::Destroy done %d \n", m_iPlugin_number);
    return NS_OK;
}

/* 
 * Destroy a plugin instance. Called after release?
 */
CJavaPluginInstance::~CJavaPluginInstance(void) 
{
    trace("[%d] CJavaPluginInstance::~destructor\n", m_iPlugin_number);

    if (m_iPlugin_number >= 0)
	m_PluginFactory->UnregisterInstance(this);
    if (mPeer != NULL)
	mPeer->Release();
    m_PluginFactory = NULL;
}

void CJavaPluginInstance::EnterRequest(const char* requestName) 
{
    trace("[%d] CJavaPluginInstance::EnterRequest: %s\n", 
	  m_iPlugin_number, requestName);
    current_request = requestName;
}

void CJavaPluginInstance::ExitRequest(const char* requestName) 
{
    trace("[%d] CJavaPluginInstance::ExitRequest: %s\n",
	  m_iPlugin_number, requestName);
    current_request = NULL;
}

NS_IMETHODIMP CJavaPluginInstance::SetUniqueId(long uniqueId) {
	// already set
	if(0 != m_uniqueId)
		return NS_OK;

	if(0 == uniqueId) {
		m_uniqueId =  s_uniqueId ++;
	} else {
		m_uniqueId = uniqueId;
	}
	return NS_OK;
}

NS_IMETHODIMP CJavaPluginInstance::GetUniqueId(long* pId) {
	*pId = m_uniqueId;
	return NS_OK;
}

