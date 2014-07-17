/*
 * @(#)TypeConverter.java	1.5 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.com;

import java.lang.reflect.Array;
import sun.plugin.util.Trace;
import sun.plugin.resources.ResourceHandler;
import java.text.NumberFormat;
import java.text.ParseException;

/**
 *  TypeConverter is used for converting an array of objects to the appropiate
 *  types that is used for method invocation in reflection.
 *
 *  Basically, only primitive types and string are converted if possible. This
 *  should make the method invocation through reflection much easier.
 */
public class TypeConverter
{
    /**
     *  Convert all parameters in an object array into the appropiate types
     *  accordingly to the defined class object in the class array.
     *
     *  @param clazzArray Class array
     *  @param params Object array
     *  @return Converted object array
     */
    static Object[] convertObjectArray(Class[] clazzArray, Object[] params)
            throws IllegalArgumentException
    {
        if (clazzArray.length != params.length)
            throw new IllegalArgumentException(ResourceHandler.getMessage("com.method.argCountInvalid"));

        Object[] result = new Object[params.length];

        // Convert parameter one-by-one
        for (int i=0; i < params.length; i++)
        {
            result[i] = convertObject(clazzArray[i], params[i]);
        }

        return result;
    }


    /**
     *  Convert object into the appropiate type accordingly to
     *  the class object.
     *
     *  @param clazz Class object
     *  @param param Object
     *  @return Converted object
     */
    public static Object convertObject(Class clazz, Object param)
                    throws IllegalArgumentException
    {
        if (param == null)
            return param;

        Class paramClazz = param.getClass();

        Trace.msgLiveConnectPrintln("com.field.needsConversion", 
				    new Object[] {paramClazz.getName(), clazz.getName()});

        // If the type matches, no need to convert
        if (clazz.isAssignableFrom(paramClazz)) {
            return param;
	}

        // Conversion is only done for primitive types and string, or primitive array
        if (clazz == java.lang.String.class)
        {
	    if(param instanceof Number) {
		NumberFormat nf = NumberFormat.getNumberInstance();
		try {
		     return nf.parse(param.toString()).toString();		    
		}catch(ParseException pexc) {
		    //Ignore and return the value of toString()
		}
	    }
	    return param.toString();
        }
        else if (clazz.isArray())
        {
            if (paramClazz.isArray())
            {
                Class compType = clazz.getComponentType();
                int len = Array.getLength(param);
                Object result = Array.newInstance(compType, len);

                // Convert array element one-by-one, recursively.
                for (int i=0; i < len; i++)
                    Array.set(result, i, convertObject(compType, Array.get(param, i)));

                return result;
            }
        }
        else if (clazz.isPrimitive())
        {
            String clazzName = clazz.getName();

            // The conversion is for primitive type
            //
            // Notice that char is not converted automatically
            //
            if (clazzName.equals("boolean") || clazzName.equals("java.lang.Boolean"))
            {
                return new Boolean(param.toString());
            }

            // Convert remaining primitive type
            //
            if (param instanceof java.lang.Number)
            {
                Number number = (Number) param;

                if (clazzName.equals("byte") || clazzName.equals("java.lang.Byte"))
                {
                    return new Byte(number.byteValue());
                }
                else if (clazzName.equals("short") || clazzName.equals("java.lang.Short"))
                {
                    return new Short(number.shortValue());
                }
                else if (clazzName.equals("int") || clazzName.equals("java.lang.Integer"))
                {
                    return new Integer(number.intValue());
                }
                else if (clazzName.equals("long") || clazzName.equals("java.lang.Long"))
                {
                    return new Long(number.longValue());
                }
                else if (clazzName.equals("float") || clazzName.equals("java.lang.Float"))
                {
                    return new Float(number.floatValue());
                }
                else if (clazzName.equals("double") || clazzName.equals("java.lang.Double"))
                {
                    return new Double(number.doubleValue());
                }
                else if (clazzName.equals("char") || clazzName.equals("java.lang.Character"))
                {
                    return new Character((char) number.shortValue());
                }
                else
                {
                    // Should be void
                    return param;
                }
            }
            else
            {
                // The type should be "char"
                return param;
            }
        }


        // We cannot handle conversion for non-primitive type
        // good luck.
        throw new IllegalArgumentException( paramClazz.getName() + 
		ResourceHandler.getMessage("com.field.typeInvalid") + clazz.getName() );
    }
}



