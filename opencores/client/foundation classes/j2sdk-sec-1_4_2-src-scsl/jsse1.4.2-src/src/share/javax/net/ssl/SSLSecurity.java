/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package javax.net.ssl;

import java.util.*;
import java.io.*;
import java.security.*;

/**
 * This class instantiates implementations of JSSE engine classes from
 * providers registered with the java.security.Security object.
 *
 * @since 1.4
 * @author Jan Luehe
 * @author Jeff Nisewanger
 * @version  1.7 06/24/03
 */

final class SSLSecurity {

    /*
     * Don't let anyone instantiate this.
     */
    private SSLSecurity() {
    }

    /*
     * Retrieves the property with the given key from the given provider.
     */
    private static String getProviderProperty(String key, Provider prov) {
	String prop = prov.getProperty(key);
	if (prop != null) {
	    return prop;
	}
	// Is there a match if we do a case-insensitive property name
	// comparison? Let's try ...
	for (Enumeration e = prov.keys(); e.hasMoreElements(); ) {
	    String matchKey = (String)e.nextElement();
	    if (key.equalsIgnoreCase(matchKey)) {
		return prov.getProperty(matchKey);
	    }
	}
	return null;
    }

    /*
     * Converts an alias name to the standard name.
     */
    private static String getStandardName(String alias, String engineType,
					  Provider prov) {
	return getProviderProperty("Alg.Alias." + engineType + "." + alias,
				   prov);
    }

    /**
     * Instantiates the implementation of the requested algorithm
     * (of the requested engine type) from the requested provider.
     */
    private static Class getImplClass(String algName, String engineType,
				      Provider prov)
	throws NoSuchAlgorithmException
    {
	Class implClass = null;

	// get the implementation class name
	String key = engineType + "." + algName;
	String className = getProviderProperty(key, prov);
	if (className == null) {
	    // consider "algName" an alias name, and try to map it onto
	    // the corresponding standard name
	    String stdName = getStandardName(algName, engineType, prov);
	    if (stdName != null)
		key = engineType + "." + stdName;
	    if (stdName == null ||
		(className = getProviderProperty(key, prov)) == null) {
		throw new NoSuchAlgorithmException("No such algorithm: " +
						   algName);
	    }
	}

	// Load the implementation class with the same class loader
	// that was used to load the provider.
	// In order to get the class loader of a class, the
	// caller's class loader must be the same as or an ancestor of
	// the class loader being returned. Otherwise, the caller must
	// have "getClassLoader" permission, or a SecurityException
	// will be thrown.
	try {
	    ClassLoader cl = prov.getClass().getClassLoader();
	    if (cl == null) {
		// system class
		implClass = Class.forName(className);
	    } else {
		implClass = cl.loadClass(className);
	    }
	} catch (ClassNotFoundException e) {
	    throw new NoSuchAlgorithmException("Class " + className +
					       " configured for " +
					       engineType +
					       " not found: " +
					       e.getMessage());
	} catch (SecurityException e) {
	    throw new NoSuchAlgorithmException("Class " + className +
					       " configured for " +
					       engineType +
					       " cannot be accessed: " +
					       e.getMessage());
	}

	return implClass;
    }

    /**
     * The body of the driver for the getImpl method.
     */
    private static Object[] getImpl1(Class implClass,
	String engineType, Provider prov) throws NoSuchAlgorithmException
    {
	// Make sure the implementation class is a subclass of the
	// corresponding engine class.
	Class typeClass = null;
	try {
	    // (The following Class.forName() always works, because this class
	    // and all the SPI classes in javax.crypto are loaded by the same
	    // class loader.)
	    typeClass = Class.forName("javax.net.ssl." + engineType + "Spi");
	} catch (ClassNotFoundException e) {
	    throw new NoSuchAlgorithmException(e.getMessage());
	}

	if (!checkSuperclass(implClass, typeClass)) {
	    throw new NoSuchAlgorithmException("Class " +
					       implClass.getName() +
					       " configured for " +
					       engineType + ": not a " +
					       engineType);
	}

	// Instantiate the SPI implementation
	try {
	    Object obj = implClass.newInstance();
	    return new Object[] { obj, prov };
	} catch (InstantiationException e) {
	    throw new NoSuchAlgorithmException("Class " +
					       implClass.getName() +
					       " configured for " +
					       engineType +
					       " cannot be instantiated: " +
					       e.getMessage());
	} catch (IllegalAccessException e) {
	    throw new NoSuchAlgorithmException("Class " +
					       implClass.getName() +
					       " configured for " +
					       engineType +
					       " cannot be accessed: " +
					       e.getMessage());
	}

    }

    /**
     * Returns an array of objects: the first object in the array is
     * an instance of an implementation of the requested algorithm
     * and type, and the second object in the array identifies the provider
     * of that implementation.
     * The <code>provName</code> argument can be null, in which case all
     * configured providers will be searched in order of preference.
     */
    static Object[] getImpl(String algName, String engineType, String provName)
	throws NoSuchAlgorithmException, NoSuchProviderException
    {
	Class implClass = null;
	Provider prov = null;

	if (provName != null) {
	    // check if the requested provider is installed
	    prov = Security.getProvider(provName);
	    if (prov == null) {
		throw new NoSuchProviderException("No such provider: " +
						  provName);
	    }
	    implClass = getImplClass(algName, engineType, prov);
	} else {
	    // get all currently installed providers
	    Provider[] provs = Security.getProviders();

	    // get the implementation class from the first provider
	    // that supplies an implementation that we can load
	    boolean found = false;
	    for (int i = 0; (i < provs.length) && (!found); i++) {
		try {
		    implClass = getImplClass(algName, engineType, provs[i]);
		    found = true;
		    prov = provs[i];
		} catch (NoSuchAlgorithmException nsae) {
		    // do nothing, check the next provider
		}
	    }
	    if (!found) {
		throw new NoSuchAlgorithmException("Algorithm " + algName
						   + " not available");
	    }
	}
	return getImpl1(implClass, engineType, prov);
    }


    /**
     * Returns an array of objects: the first object in the array is
     * an instance of an implementation of the requested algorithm
     * and type, and the second object in the array identifies the provider
     * of that implementation.
     * The <code>prov</code> argument can be null, in which case all
     * configured providers will be searched in order of preference.
     */
    static Object[] getImpl(String algName, String engineType, Provider prov)
	throws NoSuchAlgorithmException
    {
	Class implClass = null;

	if (prov != null) {
	    implClass = getImplClass(algName, engineType, prov);
	} else {
	    // get all currently installed providers
	    Provider[] provs = Security.getProviders();

	    // get the implementation class from the first provider
	    // that supplies an implementation that we can load
	    boolean found = false;
	    for (int i = 0; (i < provs.length) && (!found); i++) {
		try {
		    implClass = getImplClass(algName, engineType, provs[i]);
		    found = true;
		    prov = provs[i];
		} catch (NoSuchAlgorithmException nsae) {
		    // do nothing, check the next provider
		}
	    }
	    if (!found) {
		throw new NoSuchAlgorithmException("Algorithm " + algName
						   + " not available");
	    }
	}
	return getImpl1(implClass, engineType, prov);
    }

    /*
     * Checks whether one class is the superclass of another
     */
    private static boolean checkSuperclass(Class subclass, Class superclass) {
	while (!subclass.equals(superclass)) {
	    subclass = subclass.getSuperclass();
	    if (subclass == null) {
		return false;
	    }
	}
	return true;
    }
}
