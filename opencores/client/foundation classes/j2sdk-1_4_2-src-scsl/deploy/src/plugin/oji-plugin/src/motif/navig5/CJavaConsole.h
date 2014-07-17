/*
 * @(#)CJavaConsole.h	1.4 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

///=--------------------------------------------------------------------------=
//
// CJavaConsole.h  by Stanley Man-Kit Ho
//
///=--------------------------------------------------------------------------=

#ifndef CJavaConsole_h__
#define CJavaConsole_h__

//=--------------------------------------------------------------------------=
//
// CJavaConsole object encapsulates the Java console
//

class CJavaConsole : public nsIJVMConsole
{
public:

    ////////////////////////////////////////////////////////////////////////////
    // from nsISupports and AggregatedQueryInterface:

    NS_DECL_AGGREGATED

    static NS_METHOD Create(nsISupports* outer, JavaPluginFactory5* pJavaPluginFactory, const nsIID& aIID, 
                            void* *aInstancePtr);

	//=--------------------------------------------------------------=
	// nsIJVMConsole
	//=--------------------------------------------------------------=
    NS_IMETHOD Show(void);

    NS_IMETHOD Hide(void);

    NS_IMETHOD IsVisible(PRBool *result);

    // Prints a message to the Java console. The encodingName specifies the
    // encoding of the message, and if NULL, specifies the default platform
    // encoding.
    NS_IMETHOD Print(const char* msg, const char* encodingName = NULL);

    CJavaConsole(nsISupports *aOuter, JavaPluginFactory5* pJavaPluginFactory);
    virtual ~CJavaConsole();

protected:
    JavaPluginFactory5*           m_pJavaPluginFactory5;
};


#endif // CJavaConsole_h___
