/*
 * @(#)JavaPluginInstance5.cpp	1.46 02/03/27
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


/* 
 * A Plugin instance is a single instantiation of the plugin, with its
 * own window and its own corresponding MotifAppletViewer. It will
 * share a classloader with other plugin instances that have the same
 * document base 
 */
#include <stdlib.h>
#include <stdio.h>
#include <X11/Xlib.h>
#include "remotejni.h"
#include "nsIPluginInstance.h"
#include "nsIJVMPluginInstance.h"
#include "nsIEventHandler.h"
#include "nsplugin.h"
#include "Navig5.h"
#include "JavaPluginInstance5.h"
#include "PluginPrint.h"
#include "IUniqueIdentifier.h"

#define TRACE(m) trace("JavaPluginInstance5:%s\n", m);
#define TRACE_STR(m, p) trace("JavaPluginInstance5::%s %s\n", m, p);
#define TRACE_INT(m, i) trace("JavaPluginInstance5::%s %d\n", m, i);

NS_DEFINE_IID(kPluginInstanceIID, NS_IPLUGININSTANCE_IID);
NS_DEFINE_IID(kJVMPluginInstanceIID, NS_IJVMPLUGININSTANCE_IID);
NS_DEFINE_IID(kSupportsIID, NS_ISUPPORTS_IID);
NS_DEFINE_IID(kPluginTagInfoIID, NS_IPLUGINTAGINFO_IID);
NS_DEFINE_IID(kPluginTagInfo2IID, NS_IPLUGINTAGINFO2_IID);

nsresult JavaPluginInstance5::QueryInterface(REFNSIID aIID, 
					     void** aInstancePtr)  {

    TRACE("QueryInterface");

    if (NULL == aInstancePtr)                                         
	return NS_ERROR_NULL_POINTER;  

    if (aIID.Equals(kPluginInstanceIID)) {
		*aInstancePtr = (void*) (nsIPluginInstance * ) this;
		AddRef();
		return NS_OK;		
	}
	else if (aIID.Equals(kSupportsIID) ||
		aIID.Equals(kJVMPluginInstanceIID))
	{
		*aInstancePtr = (void*) this;
		AddRef();
		return NS_OK;		
	} else {
	    return NS_NOINTERFACE;
	}
}

NS_IMPL_ADDREF(JavaPluginInstance5);
NS_IMPL_RELEASE(JavaPluginInstance5);

/*
 * Create a new instance - should only be called by the factory 
 */
JavaPluginInstance5::JavaPluginInstance5(JavaPluginFactory5 *plugin) {
    TRACE("Create");
    NS_INIT_REFCNT();
    plugin_number = -1;
    current_request = NULL;
    plugin_factory = plugin;
    instance_peer = NULL;
    window = NULL;
    mIsDestroyed = false;
}

/*
 * Get a pointer that represents the Applet object (in Java)
 * associated with this plugin instance. Used as an entry point
 * by liveconnect
 */
NS_IMETHODIMP 
JavaPluginInstance5::GetJavaObject(jobject *result) {

  TRACE_INT("Instance GetJavaObject", plugin_number);
  if (result == NULL) 
    return NS_ERROR_NULL_POINTER;


  *result = plugin_factory->GetJavaObjectForInstance(plugin_number);
 
  if (result == NULL) {
    plugin_error("GetJavaObject - could not find applet's java object");
    return NS_ERROR_NULL_POINTER;
  } else {
    return NS_OK;
  }

}

/*
 * ??
 */
NS_IMETHODIMP 
JavaPluginInstance5::GetText(const char* *unused_result) {
  UNUSED(unused_result);
  return NS_ERROR_NOT_IMPLEMENTED;
}

/*
 * Return the value for an instance variable
 */
nsresult
JavaPluginInstance5::GetValue(nsPluginInstanceVariable var, 
					    void* unused_val) {
  if (var == nsPluginInstanceVariable_WindowlessBool) {
    TRACE("GetValue Windowlessbool");
  } else if (var == nsPluginInstanceVariable_TransparentBool) {
    TRACE("Transparent");
  } else if (var == nsPluginInstanceVariable_DoCacheBool) {
    TRACE("DoCacheBool");
    *(PRBool *)unused_val = PR_FALSE;
    return NS_OK;
  } else if (var ==  nsPluginInstanceVariable_CallSetWindowAfterDestroyBool) {
    TRACE("CallSetWindowAfterDestroyBool");
    *(PRBool *)unused_val = PR_FALSE;
    return NS_OK;
  } else {
    plugin_error("JavaPluginInstance5::GetValue - Not implemented");
  }
  UNUSED(unused_val);
  return NS_ERROR_NOT_IMPLEMENTED;
}

/* 
 * Nothing is done here.
 */
NS_IMETHODIMP JavaPluginInstance5::HandleEvent(nsPluginEvent* unused_event, 
					       PRBool *handled) {
    TRACE("HandleEvent");
    *handled = PR_FALSE;
    UNUSED(unused_event);
    return NS_OK;
}

/*
 * Initialization essentially consists of determining the document base.
 * This is different for 3.0, 4.0 and 5.0.
 * In 5.0, query the tag info
 */
NS_IMETHODIMP JavaPluginInstance5::Initialize(nsIPluginInstancePeer *peer) {
  const char *pluginType;
  PRUint16 argc1 = 0, argc2 = 0, i;
  int argc;
  char **keys1, **vals1, **keys2, **vals2, **keys, **vals;
  nsIPluginTagInfo *tagInfo;
  nsIPluginTagInfo2 *extTagInfo;
  nsresult res = NS_OK;
  
  TRACE("Initialize");
  
  peer->AddRef(); 
  instance_peer = peer;
  
  if (NS_OK != (res = peer->GetMIMEType(&pluginType))){
    plugin_error("Could not get the mime type to init instance");
    return res; 
  }

  peer->QueryInterface(kPluginTagInfoIID, (void **)&tagInfo);

  if (tagInfo != NULL) {
    // Cast to a reference to a constant pointer to an array 
    // of constant strings ! 
    tagInfo->GetAttributes(argc1, (const char* const* &)keys1, 
			   (const char* const* &) vals1);      	
    tagInfo->Release();
    }

  // We need to figure out our document base from the taginfo
  res = peer->QueryInterface(kPluginTagInfo2IID, (void **)&extTagInfo);
  if ((res == NS_OK) && (extTagInfo != NULL)) {
    nsPluginTagType tagType = nsPluginTagType_Unknown;
    if (NS_SUCCEEDED(extTagInfo->GetTagType(&tagType)))
      {
	if (tagType == nsPluginTagType_Applet || 
	    tagType == nsPluginTagType_Object) {
	  extTagInfo->GetParameters(argc2, (const char* const* &) keys2, 
				   (const char* const* &) vals2);
	}
      }
    
    const char *docbase = NULL;
    if (NS_OK != (res = extTagInfo->GetDocumentBase(&docbase))) {
      plugin_error("[%d] Initialize. No docbase?", plugin_number); 
      return res;
    }
    TRACE_STR("Initialize. Docbase", docbase);

    plugin_number = plugin_factory->RegisterInstance(this);
    if (plugin_number < 0) 
      return NS_ERROR_OUT_OF_MEMORY;

    argc = argc1 + argc2 + 1;
    keys = (char**) malloc(argc*sizeof(char*));
    vals = (char**) malloc(argc*sizeof(char*));

    // Fix for 4648112
    // check if we are being called because of classid="clsid"
    // if so, replace all codebase attributes with "."
    // any actual codebases paramaters given later will override
    // This only applies to object tags

    bool replace_codebase = false;
    const char * s_codebase = "codebase";
    const char * s_classid = "classid";
    const char * s_clsid = "clsid:";
    if (tagType == nsPluginTagType_Object) {
        for (i=0; i < argc1; i++) {
            if (!strncasecmp(keys1[i],s_classid,strlen(s_classid))) {
                if(!strncasecmp(vals1[i],s_clsid,strlen(s_clsid))) {
                   replace_codebase = true;
                   break;
                }
            }
        }
    }

    if(replace_codebase) {
        for (i=0; i < argc1; i++) {
            if(!strncasecmp(keys1[i],s_codebase,strlen(s_codebase))) {
                vals[i] = ".";
            } else {
                vals[i] = vals1[i];
            }
            keys[i] = keys1[i];
        }
    } else {
        for (i=0; i < argc1; i++) {
            keys[i] = keys1[i];
            vals[i] = vals1[i];
        }
    }

    for (i=0; i < argc2; i++) {
      keys[i+argc1] = keys2[i];
      vals[i+argc1] = vals2[i];
    }	 

	keys[argc - 1] = (char*)UNIQUE_IDENTIFIER_ID;
	vals[argc - 1] = (char*)docbase;

    plugin_factory->CreateApplet(pluginType, plugin_number, 
				 argc, keys, vals);
    free(keys);
    free(vals);

    extTagInfo->Release();
    if (docbase != NULL) {
      SetDocbase(docbase);
      return NS_OK;
    }
    return NS_ERROR_FAILURE;
  } else {
    plugin_error("Could not get TagInfo from the browser");
    return NS_ERROR_FAILURE;
  }
}

/* 
 * Get the peer NPIPluginInstancePeer for this NPIPluginInstance
 */
NS_IMETHODIMP JavaPluginInstance5::GetPeer(nsIPluginInstancePeer*
					   *resultingPeer) {
    TRACE("GetPeer");
    *resultingPeer = instance_peer;
    if (instance_peer == NULL) 
	plugin_error("Instance Peer is null! %d\n", plugin_number);
    instance_peer->AddRef();
    return NS_OK;
}

/* 
 * Start this instance. Just propagate to the JVM. The start
 * may actually have to wait until other information such as
 * the document base etc. are available. See MotifAppletViewer.java
 * maybeInit()
 */
NS_IMETHODIMP JavaPluginInstance5::Start(void) {
    CWriteBuffer wb;
    TRACE("Start");
    wb.putInt(JAVA_PLUGIN_START);
    wb.putInt(plugin_number);
    plugin_factory->SendRequest(wb, FALSE);
    return NS_OK;
}

/* 
 * Stop this instance. Just propagate to the JVM.
 */
NS_IMETHODIMP JavaPluginInstance5::Stop(void) {
    CWriteBuffer wb;
    TRACE("Stop JAVA_PLUGIN_STOP");
    wb.putInt(JAVA_PLUGIN_STOP);
    wb.putInt(plugin_number);
    plugin_factory->SendRequest(wb, FALSE);
    return NS_OK;
}

/* 
 * Destroy this instance. Just propagate to the JVM.
 */
NS_IMETHODIMP JavaPluginInstance5::Destroy(void) {
    CWriteBuffer wb;
    TRACE("Destroy");
    if(!mIsDestroyed) {
        TRACE("Doing Destroy");
        // Unregister the instance
        if (plugin_number >= 0)
	    plugin_factory->UnregisterInstance(this);
    
        // Tell the VM end to go die
        wb.putInt(JAVA_PLUGIN_DESTROY);
        wb.putInt(plugin_number);
        plugin_factory->SendRequest(wb, TRUE);
    
        if (current_request != NULL) {
            TRACE("Destroying instance, abruptly terminating request!");
            CWriteBuffer terminateRequest;
            terminateRequest.putInt(JAVA_PLUGIN_REQUEST_ABRUPTLY_TERMINATED);
            plugin_factory->SendRequest(terminateRequest, FALSE);
        }
    
        // The object destructor should be called later
        mIsDestroyed = true;
    }
    return NS_OK;
}

/* 
 *  Set the window parameters of this plugin instance. Called at
 *  creation and after a resize. 'window' holds dimension and other 
 * information
 */
NS_IMETHODIMP JavaPluginInstance5::SetWindow(nsPluginWindow* win) {
    nsPluginSetWindowCallbackStruct *ws_info;
    CWriteBuffer wb;

    window = win;

    TRACE("SetWindow JAVA_PLUGIN_WINDOW");
    if (win == NULL || win->window == NULL) {
        /*
         * netscape send a setwindow with NULL, we take this as a clean up notification
         * need to use EnterMonitor?
         */
        wb.putInt(JAVA_PLUGIN_WINDOW);
        wb.putInt(plugin_number);
        wb.putInt(0);
        wb.putInt(0);
        wb.putInt(0);
        wb.putInt(0);
        wb.putInt(0);
        plugin_factory->SendRequest(wb, TRUE);
        return NS_OK;
    }
    plugin_factory->EnterMonitor("SetWindow");

/* On my sparc box and on some linux boxes the plugin was displaying in its
   own window instead of inside the brower.  This started happening after
   netscape changed from passing a GTK widget to an XID.  As near as we can
   tell some kind of race condition exists between their creating the XID
   and the X server actually thinking it has it.  So I added these lines
   so we can push the info out to the X server before the plugin (VM) is told
   about it.

   As you can see below, if netscape provided a value in ws_info, this code
   would not be needed.  That XSync() would do the job.  We could probably
   delete that code since this code will work in all cases.  Also, we have asked
   netscape to do the XSync() on their side (which is really how it should be).
   If that happens this code and that code could go.

[smk]
*/
    Display *display = (Display *) NULL;

    TRACE("Getting Display in SetWindow()\n");
    plugin_factory->GetPluginManager()
        ->GetValue(nsPluginManagerVariable_XDisplay, &display);

    XSync(display, False);


    // PENDING(mark.lin@eng.sun.com): Sometimes ws_info isn't even 
    // set in Mozilla5!
    ws_info = (nsPluginSetWindowCallbackStruct *) win->ws_info;
    // Flush any pending X requests.  This is necessary because win
    // updates affecting the target win may not yet have reached the
    // server, and the javaplugin child process may get out of step.
    // PENDING(mark.lin@eng.sun.com): Sometimes ws_info isn't even set 
    // in Mozilla5!

    if(ws_info != NULL) {
        XSync(ws_info->display, 0);
    }

    // forward the request to the server 
    wb.putInt(JAVA_PLUGIN_WINDOW);
    wb.putInt(plugin_number);
    wb.putInt((int)win->window);
    wb.putInt(win->width);
    wb.putInt(win->height);
    wb.putInt(win->x);
    wb.putInt(win->y);
    plugin_factory->SendRequest(wb, TRUE);
    plugin_factory->ExitMonitor("SetWindow");

    return NS_OK;
}


/*
 * Create a new stream for the browser to write data into.
 */
NS_IMETHODIMP JavaPluginInstance5::NewStream(nsIPluginStreamListener** listener) {
	if (listener == NULL)
		return NS_ERROR_NULL_POINTER;

	*listener = NULL;

	return NS_ERROR_FAILURE;
}

/*
 * Printing is not supported 
 */
NS_IMETHODIMP JavaPluginInstance5::Print(nsPluginPrint* printInfo) {
  
  UNUSED(printInfo);
  /*  PluginPrint *printer;
      int result;
      printer = new PluginPrint(this, printInfo);
      result= printer->Print();
      delete printer;
      return result;
  */
  fprintf(stderr,"Print is not implemented yet\n");
  return NS_OK;
}

/*
 * Notification as the result of a query into the browser i.e.
 * the return of a GetURL query.
 */
NS_IMETHODIMP JavaPluginInstance5::URLNotify(const char* unused_url, 
					     const char* unused_target, 
					     nsPluginReason unused_reason, 
					     void* unused_notifyData) {
    UNUSED(unused_url);
    UNUSED(unused_target);
    UNUSED(unused_reason);
    UNUSED(unused_notifyData);
    //XXXFIX Change to handle caching and https support
    plugin_error("URL Notify should not be called in Navigator 5\n");
    return NS_OK;
}


/* 
 * Destroy a plugin instance. Called after release?
 */
JavaPluginInstance5::~JavaPluginInstance5(void) {
    TRACE("~destructor");
    if (plugin_number >= 0 && !mIsDestroyed)
	plugin_factory->UnregisterInstance(this);
    if (instance_peer != NULL)
	instance_peer->Release();
    plugin_factory = NULL;
}

/*
 * Return the associated plugin instance factory.
 */
JavaPluginFactory5* JavaPluginInstance5::GetPluginFactory(void){
    TRACE("Getting plugin");
    return plugin_factory;
}

/*
 * Set the document base for this instance to the url 'url'.
 * This is called during initialization
 * Send the data to the jvm, which might now have all the information
 * required to start an applet.
 */
void JavaPluginInstance5::SetDocbase(const char *url) {
    CWriteBuffer wb;
    TRACE_STR("SetDocBase", url);

    wb.putInt(JAVA_PLUGIN_DOCBASE);
    wb.putInt(plugin_number);
    wb.putString(url);
    plugin_factory->SendRequest(wb, FALSE);
}

/*
 * Inform this plugin instance that a cache or https 
 * request is starting up so that it may be terminated
 * properly when the plugin quits
 */
void JavaPluginInstance5::EnterRequest(char* requestName) {
    current_request = requestName;
}

void JavaPluginInstance5::ExitRequest(char* unused_requestName) {
    UNUSED(unused_requestName);
    current_request = NULL;
}


