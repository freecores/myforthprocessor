/*
 * @(#)JavaClass.java	1.4 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.com;

import java.lang.reflect.Field;
import java.lang.reflect.Method;
import java.lang.reflect.InvocationTargetException;
import sun.plugin.util.Trace;
import sun.plugin.resources.ResourceHandler;

/**
 *  JavaClass is the Java side companion of the JavaClass COM object.
 *  Its job is to keep track of all the things we need to know about a
 *  Java class. Particularly what methods it has and how to call them.
 *  This class helps the JavaProxy class to make method calls on a
 *  particular instance of a Java object.
 */
public class JavaClass
{
    private static final boolean writeDebug = false;

    /* Flags for IDispatch::Invoke */
    protected Class wrappedClass = null;

    private boolean collected = false;
    protected Method methods[] = null;
    protected Field fields[] = null;

    public JavaClass(Class classToWrap)
    {
        wrappedClass = classToWrap;
    }

    public String getName()
    {
        return wrappedClass.getName();
    }

    public Method getMethod(int index) {
	return methods[index - Dispatch.methodBase];
    }

    public Method getMethod1(int index, Object [] args) throws Exception{
        int correlationIndex = -1;
        int minConversionCount = java.lang.Integer.MAX_VALUE;
	boolean ambiguous = false;

        // Find the minimum overloading correlation by enumerating through
        // the Method object array.
        for (int i=0; i < methods.length; i++){
	    if( !methods[i].getName().equals(getMethod(index).getName()) )
		continue;

            Class[] paramTypes = methods[i].getParameterTypes();
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

    public Field getField(int index) {
	return fields[index - Dispatch.propertyBase];
    }

    public Dispatcher getDispatcher(int flag, int index, Object[] params) throws Exception{
	Dispatcher disp = null;

	if((flag & Dispatch.METHOD) > 0 && (Dispatch.methodBase & index) > 0) {
	    Method method = getMethod1(index, params);
	    if(method != null)
		disp = new MethodDispatcher(method);
	}
	
	if((Dispatch.propertyBase & index) > 0) {
	    Field field = getField(index);
	    if(field != null) {
		if( (flag & Dispatch.PROPERTYGET) > 0 ) {
		    disp = new PropertyGetDispatcher(field);
		} else {
		    disp = new PropertySetDispatcher(field);
		}
	    }
	}

	return disp;
    }

    protected void collect()
    {
        if (collected == false) {
	    methods = wrappedClass.getMethods();
	    fields = wrappedClass.getFields();
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
	if(fields != null) {
	    for(int i=0;i<fields.length;i++) {
		if(fields[i].getName().equalsIgnoreCase(name)) {
		    return (i + Dispatch.propertyBase);
		}
	    }
	}

	throw new Exception( name + ResourceHandler.getMessage("com.notexists") );
    }
}

