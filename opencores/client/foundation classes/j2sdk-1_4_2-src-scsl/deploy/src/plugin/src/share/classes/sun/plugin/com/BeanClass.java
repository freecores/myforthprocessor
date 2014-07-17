/*
 * @(#)BeanClass.java	1.3 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.com;

import java.lang.reflect.Method;
import java.lang.reflect.InvocationTargetException;
import sun.plugin.util.Trace;
import sun.plugin.resources.ResourceHandler;
import java.beans.BeanInfo;
import java.beans.MethodDescriptor;
import java.beans.PropertyDescriptor;
import java.beans.EventSetDescriptor;
import java.beans.Introspector;
import java.beans.IntrospectionException;
import java.util.Vector;

/**
 *  BeanClass is the Java side companion of the BeanClass COM object.
 *  Its job is to keep track of all the things we need to know about a
 *  Java class. Particularly what methods it has and how to call them.
 *  This class helps the JavaProxy class to make method calls on a
 *  particular instance of a Java object.
 */
public class BeanClass extends JavaClass
{
    private BeanInfo bInfo = null;

    private boolean collected = false;
    private MethodDescriptor methods[] = null;
    private PropertyDescriptor props[] = null;
    private EventSetDescriptor eds[] = null;

    private Vector evtMethods = new Vector();

    public BeanClass(Class classToWrap){
        super(classToWrap);
	try {
	    bInfo = Introspector.getBeanInfo(wrappedClass);
	}catch(IntrospectionException ixc){
	    ixc.printStackTrace();
	}
    }

    private MethodDescriptor getMethodDescriptor(int index) {
	return methods[index - Dispatch.methodBase];
    }

    private PropertyDescriptor getProperty(int index) {
	return props[index - Dispatch.propertyBase];
    }

    private Method getEventMethod(int index) {
	return (Method)evtMethods.elementAt(index - Dispatch.eventBase);
    }

    public BeanInfo getBeanInfo(){
	return bInfo;
    }

    private MethodDescriptor getMethodDescriptor1(int index, Object [] args) throws Exception{
        int correlationIndex = -1;
        int minConversionCount = java.lang.Integer.MAX_VALUE;
	boolean ambiguous = false;

        // Find the minimum overloading correlation by enumerating through
        // the Method object array.
        for (int i=0; i < methods.length; i++){
	    if( !methods[i].getName().equals(getMethodDescriptor(index).getName()) )
		continue;

            Class[] paramTypes = methods[i].getMethod().getParameterTypes();
	    ParameterListCorrelator correlator = new ParameterListCorrelator(paramTypes, args);
	    if(correlator.parametersCorrelateToClasses()) {
		// Check overloading correlation for all parameters
		// to the appropiate type
		int conversionCount = correlator.numberOfConversionsNeeded();
		if(conversionCount < minConversionCount) {
		    correlationIndex = i;
		    minConversionCount = conversionCount;
		    ambiguous = false;
		    if(conversionCount == 0)
			break;
		} else if (conversionCount == minConversionCount) {
		    ambiguous = true;
		}
	    }
        }

	if(ambiguous == true) {
            // Collision occurs in overloading.
            throw new InvocationTargetException(
                new Exception( ResourceHandler.getMessage("com.method.ambiguous") ));
	}


        // The minimum overloading conversionCount has been found.
        // Check if there is collision in overloading.
        if (minConversionCount == java.lang.Integer.MAX_VALUE)
	{
            throw new InvocationTargetException(
		new Exception( getMethod(index).getName() + 
				ResourceHandler.getMessage("com.method.notexists") ));
	}

	return methods[correlationIndex];
    }

    public Dispatcher getDispatcher(int flag, int index, Object[] params) throws Exception{
	collect();
	Dispatcher disp = null;

	if((flag & Dispatch.METHOD) > 0 && (Dispatch.methodBase & index) > 0) {
	    MethodDescriptor md = getMethodDescriptor1(index, params);
	    if(md != null)
		disp = new MethodDispatcher(md.getMethod());
	}
	
	if((Dispatch.propertyBase & index) > 0) {
	    PropertyDescriptor pd = getProperty(index);
	    if(pd != null) {
		if( (flag & Dispatch.PROPERTYGET) > 0 ) {
		    disp = new MethodDispatcher(pd.getReadMethod());
		} else {
		    disp = new MethodDispatcher(pd.getWriteMethod());
		}
	    }
	}

	return disp;
    }

    protected void collect()
    {
        if (collected == false) {
	    methods = bInfo.getMethodDescriptors();
	    props = bInfo.getPropertyDescriptors();
	    eds = bInfo.getEventSetDescriptors();

	    for(int i=0;i<eds.length;i++){
		Method m[] = eds[i].getListenerMethods();
		for(int j=0;j<m.length;j++){
		    evtMethods.addElement(m[j]);
		}
	    }

            collected = true;
        }
    }

    protected int getIdForName(String name) throws Exception {
	collect();

	//match methods
	if(methods != null) {
	    for(int i=0;i<methods.length;i++) {
		if(methods[i].getName().equalsIgnoreCase(name)) {
		    return (i + Dispatch.methodBase);
		}
	    }
	}

	//match properties
	if(props != null) {
	    for(int i=0;i<props.length;i++) {
		if(props[i].getName().equalsIgnoreCase(name)) {
		    return (i + Dispatch.propertyBase);
		}
	    }
	}

	//match events
	if(evtMethods != null) {
	    for(int i=0;i<evtMethods.size();i++){
		String evtName = ((Method)evtMethods.elementAt(i)).getName();
		if(evtName.equalsIgnoreCase(name)){
		    return (i+Dispatch.eventBase);
		}
	    }
	}

	throw new Exception( name + ResourceHandler.getMessage("com.notexists") );
    }


    public int getEventId(String name){
	collect();
    	//match events
	if(evtMethods != null) {
	    for(int i=0;i<evtMethods.size();i++){
		String evtName = ((Method)evtMethods.elementAt(i)).getName();
		if(evtName.equalsIgnoreCase(name)){
		    return (i+Dispatch.eventBase);
		}
	    }
	}
	
	return -1;
    }

    public int getPropertyId(String name){
	collect();
	//match properties
	if(props != null) {
	    for(int i=0;i<props.length;i++) {
		if(props[i].getName().equalsIgnoreCase(name)) {
		    return (i + Dispatch.propertyBase);
		}
	    }
	}
	
	return -1;
    }

}

