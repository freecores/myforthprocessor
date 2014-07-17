/*
 * @(#)Utils.java	1.6 03/01/24
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.com;

import sun.plugin.javascript.ocx.JSObject;
import java.lang.reflect.Array;

public final class Utils
{
    public static Object convertReturn(Class retType, Object value)
    {
	Object retValue = value;

	if(retValue == null) {
	    if (retType.equals(String.class)) {
		retValue = new NullString();
	    } else {
		if (retType.isArray()) {
		    retValue = new Object[0];
		}
	    }
	} else {
	    if(!retType.isArray()) { 
		return convertArg(value);
	    }else {
                Class compType = value.getClass().getComponentType();
		int len = Array.getLength(value);

		if(requiresWrapping(compType)) 
		    compType = DispatchImpl.class;
		else if(requiresUnWrapping(compType))
		    compType = DispatchClient.class;

		retValue = Array.newInstance(compType, len);

                // Convert array element one-by-one, recursively.
                for (int i=0; i < len; i++) {
		    if(compType == DispatchImpl.class || compType == DispatchClient.class){
			Array.set(retValue, i, convertReturn(compType, Array.get(value, i)));
		    }else{
			Array.set(retValue, i, Array.get(value, i));
		    }
		}

	    }
	}

        return retValue;
    }

    private static boolean requiresWrapping(Class type) {
	if( type.isPrimitive() || Number.class.isAssignableFrom(type) || 
	    type == String.class || type == Character.class || 
	    type == Boolean.class )
	    return false;
	else
	    return true;
    }

    private static boolean requiresUnWrapping(Class type) {
	if(sun.plugin.javascript.ocx.JSObject.class.isAssignableFrom(type))
	    return true;
	else 
	    return false;
    }

    public static Object[] convertArgs(Object args[]) {
	Object [] convertedObj = new Object[args.length];
	for(int i=0;i<args.length;i++ ) {
	    if(args[i] != null)
		convertedObj[i] = convertArg(args[i]);
	}
	return convertedObj;
    }

    public static Object convertArg(Object obj) {
	if(requiresUnWrapping(obj.getClass()))
	    return ((JSObject)obj).getDispatchClient();
	else if(requiresWrapping(obj.getClass())) {
	    assert(!(obj instanceof DispatchImpl));
	    return new DispatchImpl(obj);
	}else 
	    return obj;
    }
}
