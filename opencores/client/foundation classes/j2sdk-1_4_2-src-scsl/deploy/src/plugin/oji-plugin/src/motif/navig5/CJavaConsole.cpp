/*
 * @(#)CJavaConsole.cpp	1.7 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

///=--------------------------------------------------------------------------=
//
// CJavaConsole.cpp  by Stanley Man-Kit Ho
//
///=--------------------------------------------------------------------------=
//
// CJavaConsole encapsulates most functionalities in Java Console.
//

#include "nsplugin.h"
#include "nsAgg.h"
#include "jni.h"
#include "remotejni.h"
#include "nspr.h"
#include "nsIFactory.h"
#include "nsIJVMPlugin.h"
#include "nsIJVMConsole.h"
#include "ProxySupport5.h"
#include "nsIGenericFactory.h"
#include "JavaPluginFactory5.h"
#include "CJavaConsole.h"
#include "Debug.h"

#define TRACE(m) trace("JavaConsole:%s\n", m);

NS_DEFINE_IID(kIJVMConsole, NS_IJVMCONSOLE_IID);
NS_DEFINE_IID(kISupportsIID, NS_ISUPPORTS_IID);

////////////////////////////////////////////////////////////////////////////
// from nsISupports and AggregatedQueryInterface:

// Thes macro expands to the aggregated query interface scheme.

NS_IMPL_AGGREGATED(CJavaConsole);

NS_METHOD
CJavaConsole::AggregatedQueryInterface(const nsIID& aIID, void** aInstancePtr)
{
    TRACE("CJavaConsole::AggregatedQueryInterface\n");

    if (aIID.Equals(kISupportsIID)) {
      *aInstancePtr = GetInner();
      AddRef();
      return NS_OK;
    }
    if (aIID.Equals(kIJVMConsole)) {
        *aInstancePtr = (nsIJVMConsole *)this;
        AddRef();
        return NS_OK;
    }
    return NS_NOINTERFACE;
}



///=--------------------------------------------------------------------------=
// CJavaConsole::CJavaConsole
///=--------------------------------------------------------------------------=
// Implements the CJavaConsole object for showing, hiding, printing to the Java
// Console.
//
// parameters :
//
// return :
// 
// notes :
//
CJavaConsole::CJavaConsole(nsISupports *aOuter, JavaPluginFactory5* pJavaPluginFactory5)
{
    TRACE("CJavaConsole::CJavaConsole\n");

    NS_INIT_AGGREGATED(aOuter);

    m_pJavaPluginFactory5=pJavaPluginFactory5;
}



///=--------------------------------------------------------------------------=
// CJavaConsole::~CJavaConsole
///=--------------------------------------------------------------------------=
// Implements the CJavaConsole object for showing, hiding, printing to the Java
// Console.
//
// parameters :
//
// return :
// 
// notes :
//
CJavaConsole::~CJavaConsole()  
{
    TRACE("CJavaConsole::~CJavaConsole\n");
}

///=--------------------------------------------------------------------------=
// CJavaConsole::Create
///=--------------------------------------------------------------------------=
// Create the CJavaConsole object for showing, hiding, printing to the Java
// Console.
//
// parameters :
//
// return :
// 
// notes :
//
NS_METHOD
CJavaConsole::Create(nsISupports* outer, JavaPluginFactory5* pJavaPluginFactory5, 
		     const nsIID& aIID, void* *aInstancePtr)
{
    TRACE("CJavaConsole::Create\n");

    if (outer && !aIID.Equals(kISupportsIID))
        return NS_NOINTERFACE;   // XXX right error?
    CJavaConsole* console = new CJavaConsole(outer, pJavaPluginFactory5);
    if (console == NULL)
        return NS_ERROR_OUT_OF_MEMORY;
    //console->AddRef();
    *aInstancePtr = console->GetInner();
    *aInstancePtr = (outer != NULL)? (void *)console->GetInner(): (void *)console;
    return NS_OK;
}


////////////////////////////////////////////////////////////////////////////
// from nsIJVMConsole:
//

///=--------------------------------------------------------------------------=
// CJavaConsole::Show
//=---------------------------------------------------------------------------=
//
NS_IMETHODIMP CJavaConsole::Show(void) 
{
    TRACE("CJavaConsole::Show\n");

    return m_pJavaPluginFactory5->ShowJavaConsole();
}


///=--------------------------------------------------------------------------=
// CJavaConsole::Hide
//=---------------------------------------------------------------------------=
//
NS_IMETHODIMP CJavaConsole::Hide(void)
{
    TRACE("CJavaConsole::Hide\n");

    return NS_OK;
}


///=--------------------------------------------------------------------------=
// CJavaConsole::IsConsoleVisible
//=---------------------------------------------------------------------------=
//
NS_IMETHODIMP CJavaConsole::IsVisible(PRBool *result) 
{
    TRACE("CJavaConsole::IsConsoleVisible\n");

    if (result == NULL)
	return NS_ERROR_NULL_POINTER;

    return NS_OK;
}


///=--------------------------------------------------------------------------=
// CJavaConsole::Print
//=---------------------------------------------------------------------------=
//
// Prints a message to the Java console. The encodingName specifies the
// encoding of the message, and if NULL, specifies the default platform
// encoding.
//
NS_IMETHODIMP CJavaConsole::Print(const char* msg, const char* encodingName) 
{
    TRACE("CJavaConsole::Print\n");

    if (msg == NULL || encodingName == NULL)
	return NS_ERROR_NULL_POINTER;

    return NS_OK;
}
