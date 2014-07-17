/*
 * @(#)Sasl.java	1.15 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.security.sasl.preview;

import java.util.Map;
import java.util.StringTokenizer;
import java.util.Enumeration;
import javax.security.auth.callback.CallbackHandler;

/**
 * A static class for creating SASL clients and servers.
 *<p>
 * This class defines the policy of how to locate, load, and instantiate
 * SASL clients and servers. 
 *<p>
 * For example, an application or library gets a SASL client by doing
 * something like:
 *<blockquote><pre>
 * SaslClient sc = Sasl.createSaslClient(mechanisms,
 *     authorizationId, protocol, serverName, props, callbackHandler);
 *</pre></blockquote>
 * It can then proceed to use the instance to create an authentication connection.
 *<p>
 * Similarly, a server gets a SASL server by using code that looks as follows:
 *<blockquote><pre>
 * SaslServer ss = Sasl.createSaslServer(mechanism,
 *     protocol, serverName, props, callbackHandler);
 *</pre></blockquote>
 *
 * IMPLEMENTATION NOTE FOR THE JAVA 2 PLATFORM: 
 * <br>To use the create methods, 
 * the caller needs the following permissions:
 *<ul><tt>
 *<li>java.util.PropertyPermission("javax.security.sasl.client.pkgs", "read");
 *<li>java.util.PropertyPermission("javax.security.sasl.server.pkgs", "read");
 *</tt></ul>
 * To use the set factory methods, the caller needs the following permission:
 *<ul><tt>
 *<li>java.lang.RuntimePermission("setFactory");
 *</tt></ul>
 *
 * @author Rosanna Lee
 * @author Rob Weltman
 */
public class Sasl {
    private static SaslClientFactory clientFactory = null;
    private static SaslServerFactory serverFactory = null;

    // Cannot create one of these
    private Sasl() { 
    }

    /**
     * The name of the property that specifies the <tt>SaslClientFactory</tt>s
     * to use. The property contains a list of client package names, 
     * separated by
     * '|'. Each package must contain a class named <tt>ClientFactory</tt> that
     * implements the <tt>SaslClientFactory</tt> interface.
     * Its value is <tt>"javax.security.sasl.client.pkgs"</tt>.
     */
    public static final String CLIENT_PKGS = "javax.security.sasl.client.pkgs";
    
    /**
     * The name of the property that specifies the <tt>SaslServerFactory</tt>s
     * to use.
     * The property contains a list of server package names, 
     * separated by
     * '|'. Each package must contain a class named <tt>ServerFactory</tt> that
     * implements the <tt>SaslServerFactory</tt> interface.
     * Its value is <tt>"javax.security.sasl.server.pkgs"</tt>.
     */
    public static final String SERVER_PKGS = "javax.security.sasl.server.pkgs";

    /**
     * The name of a property that specifies the quality-of-protection to use.
     * The property contains a comma-separated, ordered list
     * of quality-of-protection values that the
     * client or server is willing to support.  A qop value is one of
     * <ul>
     * <li><tt>"auth"</tt> - authentication only</li>
     * <li><tt>"auth-int"</tt> - authentication plus integrity protection</li>
     * <li><tt>"auth-conf"</tt> - authentication plus integrity and confidentiality
     * protection</li>
     * </ul>
     *
     * The order of the list specifies the preference order of the client or
     * server. If this property is absent, the default qop is <tt>"auth"</tt>.
     * The value of this constant is <tt>"javax.security.sasl.qop"</tt>.
     */
    public static final String QOP = "javax.security.sasl.qop";

    /**
     * The name of a property that specifies the cipher strength to use.
     * The property contains a comma-separated, ordered list
     * of cipher strength values that
     * the client or server is willing to support. A strength value is one of
     * <ul>
     * <li><tt>"low"</tt></li>
     * <li><tt>"medium"</tt></li>
     * <li><tt>"high"</tt></li>
     * </ul>
     * The order of the list specifies the preference order of the client or
     * server.  An implementation should allow configuration of the meaning
     * of these values.  An application may use the Java Cryptography
     * Extension (JCE) with JCE-aware mechanisms to control the selection of
     *cipher suites that match the strength values.
     * <BR>
     * If this property is absent, the default strength is 
     * <tt>"high,medium,low"</tt>.
     * The value of this constant is <tt>"javax.security.sasl.strength"</tt>.
     */
    public static final String STRENGTH = "javax.security.sasl.strength";

    /**
     * The name of a property that specifies whether the
     * server must authenticate to the client. The property contains 
     * <tt>"true"</tt> if the server must
     * authenticate the to client; <tt>"false"</tt> otherwise.
     * The default is <tt>"false"</tt>.
     * <br>The value of this constant is
     * <tt>"javax.security.sasl.server.authentication"</tt>.
     */
    public static final String SERVER_AUTH = 
    "javax.security.sasl.server.authentication";

    /**
     * The name of a property that specifies the maximum size of the receive
     * buffer in bytes of <tt>SaslClient</tt>/<tt>SaslServer</tt>.
     * The property contains the string representation of an integer.
     * <br>If this property is absent, the default size
     * is defined by the mechanism.
     * <br>The value of this constant is <tt>"javax.security.sasl.maxbuffer"</tt>.
     */
    public static final String MAX_BUFFER = "javax.security.sasl.maxbuffer";

    /**
     * The name of a property that specifies the maximum size of the raw send
     * buffer in bytes of <tt>SaslClient</tt>/<tt>SaslServer</tt>.
     * The property contains the string representation of an integer.
     * The value of this property is negotiated between the client and server
     * during the authentication exchange.
     * <br>The value of this constant is <tt>"javax.security.sasl.rawsendsize"</tt>.
     */
    public static final String RAW_SEND_SIZE = "javax.security.sasl.rawsendsize";

    /**
     * The name of a property that specifies
     * whether mechanisms susceptible to simple plain passive attacks (e.g.,
     * "PLAIN") are not permitted. The property
     * contains <tt>"true"</tt> if such mechanisms are not permitted;
     * <tt>"false"</tt> if such mechanisms are permitted.
     * The default is <tt>"false"</tt>.
     * <br>The value of this constant is 
     * <tt>"javax.security.sasl.policy.noplaintext"</tt>.
     */
    public static final String POLICY_NOPLAINTEXT =
    "javax.security.sasl.policy.noplaintext";

    /**
     * The name of a property that specifies whether
     * mechanisms susceptible to active (non-dictionary) attacks
     * are not permitted.
     * The property contains <tt>"true"</tt> 
     * if mechanisms susceptible to active attacks
     * are not permitted; <tt>"false"</tt> if such mechanisms are permitted.
     * The default is <tt>"false"</tt>.
     * <br>The value of this constant is 
     * <tt>"javax.security.sasl.policy.noactive"</tt>.
     */
    public static final String POLICY_NOACTIVE =
    "javax.security.sasl.policy.noactive";

    /**
     * The name of a property that specifies whether
     * mechanisms susceptible to passive dictionary attacks are not permitted.
     * The property contains <tt>"true"</tt>
     * if mechanisms susceptible to dictionary attacks are not permitted;
     * </tt>"false"</tt> if such mechanisms are permitted.
     * The default is <tt>"false"</tt>.
     *<br>
     * The value of this constant is 
     * <tt>"javax.security.sasl.policy.nodictionary"</tt>.
     */
    public static final String POLICY_NODICTIONARY =
    "javax.security.sasl.policy.nodictionary";

    /**
     * The name of a property that specifies whether mechanisms that accept
     * anonymous login are not permitted. The property contains <tt>"true"</tt> 
     * if mechanisms that accept anonymous login are not permitted; 
     * <tt>"false"</tt>
     * if such mechanisms are permitted. The default is <tt>"false"</tt>.
     *<br>
     * The value of this constant is 
     * <tt>"javax.security.sasl.policy.noanonymous"</tt>.
     */
    public static final String POLICY_NOANONYMOUS =
    "javax.security.sasl.policy.noanonymous";


     /** 
      * The name of a property that specifies whether mechanisms that implement
      * forward secrecy between sessions are required. Forward secrecy
      * means that breaking into one session will not automatically
      * provide information for breaking into future sessions. 
      * The property
      * contains <tt>"true"</tt> if mechanisms that implement forward secrecy 
      * between sessions are required; <tt>"false"</tt> if such mechanisms 
      * are not required. The default is <tt>"false"</tt>.
      *<br>
      * The value of this constant is 
      * <tt>"javax.security.sasl.policy.forward"</tt>.
      */
    public static final String POLICY_FORWARD_SECRECY =
    "javax.security.sasl.policy.forward";

    /**
     * The name of a property that specifies whether
     * mechanisms that pass client credentials are required. The property
     * contains <tt>"true"</tt> if mechanisms that pass
     * client credentials are required; <tt>"false"</tt>
     * if such mechanisms are not required. The default is <tt>"false"</tt>.
     *<br>
     * The value of this constant is 
     * <tt>"javax.security.sasl.policy.credentials"</tt>.
     */
    public static final String POLICY_PASS_CREDENTIALS =
    "javax.security.sasl.policy.credentials";


    /**
     * Creates a <tt>SaslClient</tt> using the parameters supplied.
     * The algorithm for selecting a <tt>SaslClient</tt> is as follows.
     *<ol>
     *<li>If a factory has been installed via <tt>setSaslClientFactory()</tt>,
     *invoke <tt>createSaslClient()</tt> on it.
     *If the method invocation returns a non-null <tt>SaslClient</tt>
     *instance, return the <tt>SaslClient</tt> instance; otherwise continue.
     *
     *<li>Create a list of fully qualified class names
     * using the package names listed in the <tt>CLIENT_PKGS</tt> 
     * (<tt>"javax.security.sasl.client.pkgs"</tt>)
     * property in <tt>props</tt> and the class name, <tt>ClientFactory</tt>.
     * Each class named on this list names a <tt>SaslClientFactory</tt> 
     * implementation. Starting with the first class on the list,
     * create an instance of <tt>SaslClientFactory</tt> using the class's 
     * public no-argument constructor and
     * invoke <tt>createSaslClient()</tt> on it. If the method invocation
     * returns a non-null <tt>SaslClient</tt> instance, return it;
     * otherwise repeat using the next class on the list until a non-null
     * <tt>SaslClient</tt> is produced or the list exhausted.
     *
     *<li>Repeat previous step using the <tt>CLIENT_PKGS</tt>
     * (<tt>"javax.security.sasl.client.pkgs"</tt>) System property instead
     * of the property in <tt>props</tt>.
     *
     *<li>As per the Java 2, Standard Edition, v 1.3 service provider guidelines, 
     * check for the existence of one of more files named
     * <tt>META-INF/services/com.sun.security.sasl.preview.SaslClientFactory</tt> in
     * the classpath and installed JAR files.
     * Each file lists the fully qualified
     * class names of the factories (i.e., implementations of
     * <tt>SaslClientFactory</tt>) found in the JAR or classpath.
     * Construct a complete list of fully qualified class names using these
     * files and repeat Step 2 using this list.
     * If there are more than one of these files, the order in
     * which they are processed is undefined.
     *<li>If no non-null <tt>SaslClient</tt> instance is produced, return null.
     *</ol>
     *
     * @param mechanisms The non-null list of mechanism names to try. Each is the
     * IANA-registered name of a SASL mechanism. (e.g. "GSSAPI", "CRAM-MD5").
     * @param authorizationId The possibly null protocol-dependent 
     * identification to be used for authorization.
     * If null or empty, the server derives an authorization 
     * ID from the client's authentication credentials.
     * When the SASL authentication completes successfully, 
     * the specified entity is granted access. 
     *
     * @param protocol The non-null string name of the protocol for which
     * the authentication is being performed (e.g., "ldap").
     *
     * @param serverName The non-null fully-qualified host name of the server
     * to authenticate to.
     *
     * @param props The possibly null set of properties used to
     * select the SASL mechanism and to configure the authentication
     * exchange of the selected mechanism.
     * For example, if <tt>props</tt> contains the 
     * <code>Sasl.POLICY_NOPLAINTEXT</code> property with the value 
     * <tt>"true"</tt>, then the selected
     * SASL mechanism must not be susceptible to simple plain passive attacks.
     * In addition to the standard properties declared in this class, 
     * other, possibly mechanism-specific, properties can be included. 
     * Properties not relevant to the selected mechanism are ignored.
     *
     * @param cbh The possibly null callback handler to used by the SASL
     * mechanisms to get further information from the application/library
     * to complete the authentication. For example, a SASL mechanism might
     * require the authentication ID, password and realm from the caller.
     * The authentication ID is requested by using a <tt>NameCallback</tt>.
     * The password is requested by using a <tt>PasswordCallback</tt>.
     * The realm is requested by using a <tt>RealmChoiceCallback</tt> if there is a list
     * of realms to choose from, and by using a <tt>RealmCallback</tt> if
     * the realm must be entered. 
     *
     *@return A possibly null <tt>SaslClient</tt> created using the parameters
     * supplied. If null, cannot find a <tt>SaslClientFactory</tt>
     * that will produce one.
     *@exception SaslException If cannot create a <tt>SaslClient</tt> because
     * of an error.
     * @see #CLIENT_PKGS
     */
    public static SaslClient createSaslClient(
	String[] mechanisms,
	String authorizationId,
	String protocol,
	String serverName,
	Map props, 
	CallbackHandler cbh) throws SaslException {
        
        SaslClient mech = null;

	Enumeration enum = getSaslClientFactories(props);

	SaslClientFactory fac;
	while (mech == null && enum.hasMoreElements()) {
	    fac = (SaslClientFactory)enum.nextElement();
	    mech = fac.createSaslClient(mechanisms, authorizationId, 
		protocol, serverName, props, cbh);
	}

	return mech;
    }

    /**
     * Gets an enumeration of known factories for producing <tt>SaslClient</tt>.
     * This method uses the same sources for locating factories as
     * <tt>createSaslClient()</tt>.
     * @param props A possible <tt>null</tt> set of properties that may
     * contain the <tt>Sasl.CLIENT_PKGS</tt> property.
     * @return An enumeration of known factories for producing
     * <tt>SaslClient</tt>.
     * @see #createSaslClient
     */
    public static Enumeration getSaslClientFactories(Map props) {
	// %%% Get merged list of factories from 
	// 1. setSaslServerFactory
	// 2. CLIENT_PKGS prop from props
	// 3. CLIENT_PKGS system properties
	// 4. META-INF/services/javax.security.sasl.SaslClientFactory

	return new MergedEnumeration(clientFactory,
	    CLIENT_PKGS, 0, "ClientFactory", SaslClientFactory.class, props);
    }

    /**
     * Sets the default <tt>SaslClientFactory</tt> to use.
     * This method sets <tt>fac</tt> to be the default factory.
     * It can only be called with a non-null value once per VM. 
     * If a factory has been set already, this method throws
     * <tt>IllegalStateException</tt>.
     * @param fac The possibly null factory to set. If null, doesn't do anything.
     * @exception IllegalStateException If factory already set.
     * @exception SecurityException If a security manager exists and the
     * caller does not have permission to set a factory. 
     */
     /*On the Java 2 platform,
     * the required permission is
     * <tt>java.lang.RuntimePermission("setFactory")</tt>.
     */
    public static void setSaslClientFactory(SaslClientFactory fac) {
	if (clientFactory != null) {
	    throw new IllegalStateException (
		"SaslClientFactory already defined");
	}
	SecurityManager security = System.getSecurityManager();
	if (security != null) {
	    security.checkSetFactory();
	}
	clientFactory = fac;
    }
    
    /**
     * Creates a <tt>SaslServer</tt> for the specified mechanism. It
     * returns <tt>null</tt> if no <tt>SaslServer</tt> can be created
     * for the specified mechanism.
     *<p>
     * The algorithm for selecting a <tt>SaslServer</tt> is as follows.
     *<ol>
     *<li>If a factory has been installed via <tt>setSaslServerFactory()</tt>,
     *invoke <tt>createSaslServer()</tt> on it.
     *If the method invocation returns a non-null <tt>SaslServer</tt>
     *instance, return the <tt>SaslServer</tt> instance; otherwise continue.
     *
     *<li>Create a list of fully qualified class names
     * using the package names listed in the <tt>SERVER_PKGS</tt> 
     * (<tt>"javax.security.sasl.server.pkgs"</tt>)
     * property in <tt>props</tt> and the class name, <tt>ServerFactory</tt>.
     * Each class named on this list names a <tt>SaslServerFactory</tt> 
     * implementation. Starting with the first class on the list,
     * create an instance of <tt>SaslServerFactory</tt> using the class's 
     * public no-argument constructor and
     * invoke <tt>createSaslServer()</tt> on it. If the method invocation
     * returns a non-null <tt>SaslServer</tt> instance, return it;
     * otherwise repeat using the next class on the list until a non-null
     * <tt>SaslServer</tt> is produced or the list exhausted.
     *
     *<li>Repeat previous step using the <tt>SERVER_PKGS</tt>
     * (<tt>"javax.security.sasl.server.pkgs"</tt>) System property instead
     * of the property in <tt>props</tt>.
     *
     *<li>As per the Java 2, Standard Edition, v 1.3 service provider guidelines, 
     * check for the existence of one of more files named
     * <tt>META-INF/services/com.sun.security.sasl.preview.SaslServerFactory</tt> 
     * in the classpath and installed JAR files.
     * Each file lists the fully qualified
     * class names of the factories (i.e., implementations of
     * <tt>SaslServerFactory</tt>).
     * Construct a complete list of fully qualified class names using these
     * files and repeat Step 2 using this list.
     * If there are more than one of these files, the order in
     * which they are processed is undefined.
     *<li>If no non-null <tt>SaslServer</tt> instance is produced, return null.
     *</ol>
     * @param mechanism The non-null mechanism name. It must be an
     * IANA-registered name of a SASL mechanism. (e.g. "GSSAPI", "CRAM-MD5").
     * @param protocol The non-null string name of the protocol for which
     * the authentication is being performed (e.g., "ldap").
     * @param serverName The non-null fully qualified host name of the server.
     * @param props The possibly null set of properties used to
     * select the SASL mechanism and to configure the authentication
     * exchange of the selected mechanism.
     * For example, if <tt>props</tt> contains the 
     * <code>Sasl.POLICY_NOPLAINTEXT</code> property with the value 
     * <tt>"true"</tt>, then the selected
     * SASL mechanism must not be susceptible to simple plain passive attacks.
     * In addition to the standard properties declared in this class, 
     * other, possibly mechanism-specific, properties can be included. 
     * Properties not relevant to the selected mechanism are ignored.
     *
     * @param cbh The possibly null callback handler to used by the SASL
     * mechanisms to get further information from the application/library
     * to complete the authentication. For example, a SASL mechanism might
     * require the authentication ID, password and realm from the caller.
     * The authentication ID is requested by using a <tt>NameCallback</tt>.
     * The password is requested by using a <tt>PasswordCallback</tt>.
     * The realm is requested by using a <tt>RealmChoiceCallback</tt> if there is a list
     * of realms to choose from, and by using a <tt>RealmCallback</tt> if
     * the realm must be entered. 
     *
     *@return A possibly null <tt>SaslServer</tt> created using the parameters
     * supplied. If null, cannot find a <tt>SaslServerFactory</tt>
     * that will produce one.
     *@exception SaslException If cannot create a <tt>SaslServer</tt> because
     * of an error.
     * @see #SERVER_PKGS
     **/
    public static SaslServer
	createSaslServer(String mechanism,
                    String protocol,
                    String serverName,
                    Map props,
                    javax.security.auth.callback.CallbackHandler cbh)
	throws SaslException {
        
        SaslServer mech = null;

	Enumeration enum = getSaslServerFactories(props);

	SaslServerFactory fac;
	while (mech == null && enum.hasMoreElements()) {
	    fac = (SaslServerFactory)enum.nextElement();
	    mech = fac.createSaslServer(mechanism, protocol, serverName, 
		props, cbh);
	}

	return mech;
    }

    /**
     * Gets an enumeration of known factories for producing <tt>SaslServer</tt>.
     * This method uses the same sources for locating factories as
     * <tt>createSaslServer()</tt>.
     * @param props A possible <tt>null</tt> set of properties that may
     * contain the <tt>Sasl.SERVER_PKGS</tt> property.
     * @return An enumeration of known factories for producing
     * <tt>SaslServer</tt>.
     * @see #createSaslServer
     */
    public static Enumeration getSaslServerFactories(Map props) {
	// %%% Get merged list of factories from 
	// 1. setSaslServerFactory
	// 2. SERVER_PKGS prop from props
	// 3. SERVER_PKGS system properties
	// 4. META-INF/services/javax.security.sasl.SaslClientFactory

	return new MergedEnumeration(serverFactory, SERVER_PKGS, 1,
	    "ServerFactory", SaslServerFactory.class, props);
    }
    
    /**
     * Sets the default <tt>SaslServerFactory</tt> to use.
     * This method sets <tt>fac</tt> to be the default factory.
     * It can only be called with a non-null value once per VM. 
     * If a factory has been set already, this method throws
     * <tt>IllegalStateException</tt>.
     *
     * @param fac The possibly null factory to set. If null, doesn't do anything.
     * @exception IllegalStateException If factory already set.
     * @exception SecurityException If a security manager exists and
     * the caller does not have permission to set a factory. 
     */
     /* On the Java 2 platform,
     * the required permission is
     * <tt>java.lang.RuntimePermission("setFactory")</tt>.
     */
    public static void setSaslServerFactory(SaslServerFactory fac) {
	if (serverFactory != null) {
	    throw new IllegalStateException (
		"SaslServerFactory already defined");
	}
	SecurityManager security = System.getSecurityManager();
	if (security != null) {
	    security.checkSetFactory();
	}
	serverFactory = fac;
    }
}
