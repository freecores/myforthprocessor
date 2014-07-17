/*
 * @(#)Resources.java	1.16 03/01/23
 * 
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.javaws;

import java.util.Locale;
import java.util.ResourceBundle;
import java.util.MissingResourceException;
import java.text.MessageFormat;
import java.text.NumberFormat;
import java.awt.Color;
import java.lang.reflect.Field;
import javax.swing.Icon;
import javax.swing.ImageIcon;
import com.sun.javaws.debug.Debug;

/**
* Contains static reference to the resources used by javaws
* plus a bunch of utility methods to internationalize the sources
*/
public class Resources {
    
    /** Reference to global javaws resource object */
    private static ResourceBundle _resources = null;
    private static NumberFormat   _numberFormat = null;
        
    /**
    * Returns resource bundle for current local
    */
    public static ResourceBundle getResources() {
        if (_resources == null) {
            synchronized (Resources.class) {
                // Load the resources
                _resources = ResourceBundle.getBundle("com/sun/javaws/resources/strings");
            }
            
            // Get number formatter object for current local
            _numberFormat = NumberFormat.getInstance();
        }
        return _resources;
    }
    
    /**
    * Returns a string from the resources
    */
    static public String getString(String key) {
        try {
            return Resources.getResources().getString(key);
        } catch (MissingResourceException mre) {
            Debug.fatal("Missing resource: " + key);
            return null;
        }
    }
    
    /**
    * Returns an Integer from the resources
    */
    static public int getInteger(String key) {
        try {
            return Integer.parseInt(getString(key),16);
        } catch (MissingResourceException mre) {
            Debug.fatal("Missing resource: " + key);
            return -1;
        }
    }
    
    /**
    * Returns a string from a resource, substituting argument 1
    */
    static public String getString(String key, String arg) {
        Object[] messageArguments = { arg };
        return applyPattern(key, messageArguments);
    }
    
    /**
    * Returns a string from a resource, substituting argument 1 and 2
    */
    static public String getString(String key, String arg1, String arg2) {
        Object[] messageArguments = { arg1, arg2};
        return applyPattern(key, messageArguments);
    }

    /**
    * Returns a string from a resource, substituting argument 1 and 2
    */
    static public String getString(String key, Long arg1, Long arg2) {
        Object[] messageArguments = { arg1, arg2};
        return applyPattern(key, messageArguments);
    }
    
    /**
    * Returns a string from a resource, substituting argument 1,2, and 3
    */
    static public String getString(String key, String arg1, String arg2, String arg3) {
        Object[] messageArguments = { arg1, arg2, arg3 };
        return applyPattern(key, messageArguments);
    }
     
    /**
    *  Returns a string from a resource, substituting the integer
    */
    static public String getString(String key, int arg1) {
        Object[] messageArguments = { new Integer(arg1) };
        return applyPattern(key, messageArguments);
    }
    
    /**
    *  Returns a string from a resource, substituting the three integers
    */
    static public String getString(String key, int arg1, int arg2, int arg3) {
        Object[] messageArguments = { new Integer(arg1), new Integer(arg2), new Integer(arg3) };
        return applyPattern(key, messageArguments);
    }
    
    static public String getString(String key, String arg1, int arg2, String arg3) {
        Object[] messageArguments = { arg1, new Integer(arg2), arg3 };
        return applyPattern(key, messageArguments);
    }
    
    static public String getString(String key, String arg1, int arg2) {
        Object[] messageArguments = { arg1, new Integer(arg2) };
        return applyPattern(key, messageArguments);
    }
        
    static public synchronized String formatDouble(double d, int digits) {
        _numberFormat.setGroupingUsed(true);
        _numberFormat.setMaximumFractionDigits(digits);
        _numberFormat.setMinimumFractionDigits(digits);
        return _numberFormat.format(d);
    }
        
    /**
    * Returns the Icon given a resource name
    */
    static public ImageIcon getIcon(String key) {
        String resourceName = getString(key);
        return new ImageIcon(Resources.class.getResource(resourceName));
    }
    
    /** Helper function that applies the messageArguments to a message from the resource object */
    static private String applyPattern(String key, Object[] messageArguments) {
        String message = getString(key);
        MessageFormat formatter = new MessageFormat(message);
        String output = formatter.format(message, messageArguments);
        return output;
    }

    /**
    * Returns a Color Object from the resources
    */
    static public Color getColor(String key) {
	int rgb = getInteger(key);
	return new Color(rgb);
    }

    /**
    * Returns a Virtual Key Code from the resources
    */
    static Class _keyEventClazz = null;
    static public int getVKCode(String key) {
	String resource = getString(key);
	if (resource != null && resource.startsWith("VK_")) try {
            if (_keyEventClazz == null) {
                _keyEventClazz= Class.forName("java.awt.event.KeyEvent");
	    }
            Field field = _keyEventClazz.getDeclaredField(resource);
            int value = field.getInt(null);
            return value;
	} catch (ClassNotFoundException cnfe) { 
            Debug.ignoredException(cnfe);    
        } catch (NoSuchFieldException nsfe) {
            Debug.ignoredException(nsfe);
        } catch (SecurityException se) {
            Debug.ignoredException(se);
        } catch (Exception e) {
            Debug.ignoredException(e);
        }
	return 0;
    }

}



