/*
 * @(#)SwingBeanInfo.java	1.19 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package javax.swing.beaninfo;

import java.beans.*;
import java.lang.reflect.*;
import java.awt.Image;

/**
 * The superclass for all Swing BeanInfo classes.  It provides
 * default implementations of <code>getIcon</code> and 
 * <code>getDefaultPropertyIndex</code> as well as utility
 * methods, like createPropertyDescriptor, for writing BeanInfo
 * implementations.  This classes is intended to be used along
 * with <code>GenSwingBeanInfo</code> a BeanInfo class code generator.
 * 
 * @see GenSwingBeanInfo
 * @version 1.19 01/23/03
 * @author Hans Muller
 */
public abstract class SwingBeanInfo extends SimpleBeanInfo
{
    /* The values of these createPropertyDescriptor() and 
     * createBeanDescriptor() keywords are the names of the 
     * properties they're used to set.
     */
    public static final String BOUND = "bound";
    public static final String CONSTRAINED = "constrained";
    public static final String PROPERTYEDITORCLASS = "propertyEditorClass";
    public static final String READMETHOD = "readMethod";
    public static final String WRITEMETHOD = "writeMethod";
    public static final String DISPLAYNAME = "displayName";
    public static final String EXPERT = "expert";
    public static final String HIDDEN = "hidden";
    public static final String PREFERRED = "preferred";
    public static final String SHORTDESCRIPTION = "shortDescription";
    public static final String CUSTOMIZERCLASS = "customizerClass";

    /**
     * Fatal errors are handled by calling this method.
     */
    protected void throwError(Exception e, String s) {
	throw new Error(e.toString() + " " + s);
    }


    private void initFeatureDescriptor(FeatureDescriptor fd, String key, Object value)
    {
	if (DISPLAYNAME.equals(key)) {
	    fd.setDisplayName((String)value);
	}

	if (EXPERT.equals(key)) {
	    fd.setExpert(((Boolean)value).booleanValue());
	}

	if (HIDDEN.equals(key)) {
	    fd.setHidden(((Boolean)value).booleanValue());
	}

	if (PREFERRED.equals(key)) {
	    fd.setPreferred(((Boolean)value).booleanValue());
	}

	else if (SHORTDESCRIPTION.equals(key)) {
	    fd.setShortDescription((String)value);
	}

	/* Otherwise assume that we have an arbitrary FeatureDescriptor
	 * "attribute".
	 */
	else {
	    fd.setValue(key, value);
	}
    }


    /**
     * Create a beans PropertyDescriptor given an of keyword/value 
     * arguments.  The following sample call shows all of the supported
     * keywords:
     *<pre>
     *      createPropertyDescriptor("contentPane", new Object[] {
     *			   BOUND, Boolean.TRUE,
     *  	     CONSTRAINED, Boolean.TRUE,
     *	     PROPERTYEDITORCLASS, package.MyEditor.class,
     *		      READMETHOD, "getContentPane",
     *  	     WRITEMETHOD, "setContentPane",
     *  	     DISPLAYNAME, "contentPane",
     *			  EXPERT, Boolean.FALSE,
     *			  HIDDEN, Boolean.FALSE,
     *		       PREFERRED, Boolean.TRUE,
     *		SHORTDESCRIPTION, "A top level window with a window manager border",
     *	       "random attribute","random object value" 
     *	      }
     *	   );
     * </pre>
     * The keywords correspond to <code>java.beans.PropertyDescriptor</code> and 
     * <code>java.beans.FeatureDescriptor</code> properties, e.g. providing a value
     * for displayName is comparable to <code>FeatureDescriptor.setDisplayName()</code>.
     * Using createPropertyDescriptor instead of the PropertyDescriptor
     * constructor and set methods is preferrable in that it regularizes
     * the code in a <code>java.beans.BeanInfo.getPropertyDescriptors()</code>
     * method implementation.  One can use <code>createPropertyDescriptor</code>
     * to set <code>FeatureDescriptor</code> attributes, as in "random attribute"
     * "random object value". 
     * <p>
     * All properties should provide a reasonable value for the 
     * <code>SHORTDESCRIPTION</code> keyword and should set <code>BOUND</code> 
     * to <code>Boolean.TRUE</code> if neccessary.  The remaining keywords 
     * are optional.  There's no need to provide values for keywords like
     * READMETHOD if the correct value can be computed, i.e. if the properties
     * get/is method follows the standard beans pattern.
     * <p>
     * The PREFERRED keyword is not supported by the JDK1.1 java.beans package.
     * It's still worth setting it to true for properties that are most
     * likely to be interested to the average developer, e.g. AbstractButton.title
     * is a preferred property, AbstractButton.focusPainted is not.
     * 
     * @see java.beans#BeanInfo
     * @see java.beans#PropertyDescriptor
     * @see java.beans#FeatureDescriptor
     */
    public PropertyDescriptor createPropertyDescriptor(Class cls, String name, Object[] args) 
    {
	PropertyDescriptor pd = null;
	try {
	    pd = new PropertyDescriptor(name, cls);
        } catch (IntrospectionException e) {
            // Try creating a read-only property, in case setter isn't defined.
            try {
                pd = createReadOnlyPropertyDescriptor(name, cls);
            } catch (IntrospectionException ie) {
                throwError(ie, "Can't create PropertyDescriptor for " + name + " ");
            }
        }

	for(int i = 0; i < args.length; i += 2) {
	    String key = (String)args[i];
	    Object value = args[i + 1];
	    
	    if (BOUND.equals(key)) {
		pd.setBound(((Boolean)value).booleanValue());
	    }
 
	    else if (CONSTRAINED.equals(key)) {
		pd.setConstrained(((Boolean)value).booleanValue());
	    }

	    else if (PROPERTYEDITORCLASS.equals(key)) {
		pd.setPropertyEditorClass((Class)value);
	    }

	    else if (READMETHOD.equals(key)) {
		String methodName = (String)value;
		Method method;
		try {
		    method = cls.getMethod(methodName, new Class[0]);
		    pd.setReadMethod(method);
		}
		catch(Exception e) {
		    throwError(e, cls + " no such method as \"" + methodName + "\"");
		}
	    }

	    else if (WRITEMETHOD.equals(key)) {
		String methodName = (String)value;
		Method method;
		try {
		    Class type = pd.getPropertyType();
		    method = cls.getMethod(methodName, new Class[]{type});
		    pd.setWriteMethod(method);
		}
		catch(Exception e) {
		    throwError(e, cls + " no such method as \"" + methodName + "\"");
		}
	    }

	    else {
		initFeatureDescriptor(pd, key, value);
	    }
	}

	return pd;
    }


    /**
     * Create a BeanDescriptor object given an of keyword/value 
     * arguments.  The following sample call shows all of the supported
     * keywords:
     *<pre>
     *      createBeanDescriptor(JWindow..class, new Object[] {
     *	         CUSTOMIZERCLASS, package.MyCustomizer.class,
     *  	     DISPLAYNAME, "JFrame",
     *			  EXPERT, Boolean.FALSE,
     *			  HIDDEN, Boolean.FALSE,
     *		       PREFERRED, Boolean.TRUE,
     *		SHORTDESCRIPTION, "A top level window with a window manager border",
     *	       "random attribute","random object value" 
     *	      }
     *	   );
     * </pre>
     * The keywords correspond to <code>java.beans.BeanDescriptor</code> and 
     * <code>java.beans.FeatureDescriptor</code> properties, e.g. providing a value
     * for displayName is comparable to <code>FeatureDescriptor.setDisplayName()</code>.
     * Using createBeanDescriptor instead of the BeanDescriptor
     * constructor and set methods is preferrable in that it regularizes
     * the code in a <code>java.beans.BeanInfo.getBeanDescriptor()</code>
     * method implementation.  One can use <code>createBeanDescriptor</code>
     * to set <code>FeatureDescriptor</code> attributes, as in "random attribute"
     * "random object value". 
     * 
     * @see java.beans#BeanInfo
     * @see java.beans#PropertyDescriptor
     */
    public BeanDescriptor createBeanDescriptor(Class cls, Object[] args) 
    {
	Class customizerClass = null;
	
	/* For reasons I don't understand, customizerClass is a 
	 * readOnly property.  So we have to find it and pass it
	 * to the constructor here.
	 */
	for(int i = 0; i < args.length; i += 2) {
	    if (CUSTOMIZERCLASS.equals((String)args[i])) {
		customizerClass = (Class)args[i + 1];
		break;
	    }
	}

	BeanDescriptor bd = new BeanDescriptor(cls, customizerClass);

	for(int i = 0; i < args.length; i += 2) {
	    String key = (String)args[i];
	    Object value = args[i + 1];
	    initFeatureDescriptor(bd, key, value);
	}

	return bd;
    }


    /**
     * The default index is always 0.  In other words the first property
     * listed in the getPropertyDescriptors() method is the one
     * to show a (JFC builder) user in a situation where just a single
     * property will be shown.
     */
    public int getDefaultPropertyIndex() {
	return 0;
    }


    /**
     * Returns a generic Swing icon, all icon "kinds" are supported.
     * Subclasses should defer to this method when they don't have
     * a particular beans icon kind.
     */
    public Image getIcon(int kind) {
	// PENDING(hmuller) need generic swing icon images.
	return null;
    }

    /**
     * Returns the BeanInfo for the superclass of our bean, so that
     * its PropertyDescriptors will be included.
     */
    public BeanInfo[] getAdditionalBeanInfo() {
        Class superClass = getBeanDescriptor().getBeanClass().getSuperclass();
        BeanInfo superBeanInfo = null;
        try {
            superBeanInfo = Introspector.getBeanInfo(superClass);
        } catch (IntrospectionException ie) {}
        if (superBeanInfo != null) {
            BeanInfo[] ret = new BeanInfo[1];
            ret[0] = superBeanInfo;
            return ret;
        }
        return null;
    }


    private PropertyDescriptor createReadOnlyPropertyDescriptor(
        String name, Class cls) throws IntrospectionException {

        Method readMethod = null;
        String base = capitalize(name);
        Class[] parameters = new Class[0];

        // Is it a boolean?
        try {
            readMethod = cls.getMethod("is" + base, parameters);
        } catch (Exception ex) {}
        if (readMethod == null) {
            try {
                // Try normal accessor pattern.
                readMethod = cls.getMethod("get" + base, parameters);
            } catch (Exception ex2) {}
        }
        if (readMethod != null) {
            return new PropertyDescriptor(name, readMethod, null);
        }

        try {
            // Try indexed accessor pattern.
            parameters = new Class[1];
            parameters[0] = int.class;
            readMethod = cls.getMethod("get" + base, parameters);
        } catch (NoSuchMethodException nsme) {
            throw new IntrospectionException(
                "cannot find accessor method for " + name + " property.");
        }
        return new IndexedPropertyDescriptor(name, null, null, readMethod, null);
    }

    // Modified methods from java.beans.Introspector
    private static String capitalize(String s) {
	if (s.length() == 0) {
 	    return s;
	}
	char chars[] = s.toCharArray();
	chars[0] = Character.toUpperCase(chars[0]);
	return new String(chars);
    }
}

