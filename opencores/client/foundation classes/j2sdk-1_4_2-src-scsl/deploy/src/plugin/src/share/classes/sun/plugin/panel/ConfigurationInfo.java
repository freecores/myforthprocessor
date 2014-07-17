/*
 * @(#)ConfigurationInfo.java	1.47 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.panel;

 /**
  * Model class for all configuration information pertinent to the Java
  * Activator. 
  *
  * Note that we want the contents of the poeprties file to be locale
  * independent.  We therefore do *NOT* internationalize the tag strings
  * or the other contents of the property file.
  *
  * End users should never be editing the properties file directly.  They
  * should be using the (carefully internatiuonalzied) ControlPanel.
  *
  * @author Jerome Dochez
  * @version 1.23, 02/11/02
  */

import java.beans.*;
import java.util.*;
import java.io.*;
import java.net.*;
import java.security.*;
import java.security.cert.*;
import java.security.cert.Certificate;
import java.text.MessageFormat;
import sun.plugin.security.CertificateStore;
import sun.plugin.security.PluginCertificateStore;
import sun.plugin.security.PluginHttpsCertStore;
import sun.plugin.security.RootCACertificateStore;
import sun.plugin.security.HttpsRootCACertStore;
import sun.plugin.util.Trace;
import sun.plugin.util.UserProfile;
import sun.plugin.util.DialogFactory;

public class ConfigurationInfo {

    /**
     * Contruct a new configuration information
     * 
     */
    public ConfigurationInfo() {
	if (theUserPropertiesFile == null) 
	{
	    theUserPropertiesFile = new File(UserProfile.getPropertyFile());
	    new File(theUserPropertiesFile.getParent()).mkdirs();
	}
        reset();
    }

    /**
     * enable/disable the Java Activator Console Window
     * 
     * @param String to enable/minimize/disable the console
     */
    public void setConsoleEnabled(String value) {
	showConsole = value;
	firePropertyChange();
    }

    /**
     * @return "show" if the console is enabled, "hide" if
     * the console is minimized, or "nothing" if we do not
     * want the console.
     */
    public String isConsoleEnabled() {
	return showConsole;
    }

    /**
     * enable/disable showing exception dialogboxes
     * 
     * @param true to enable
     */
    public void setShowException(boolean value) {
	showException = value;
	firePropertyChange();
    }

    /**
     * enable/disable System Tray icon
     * 
     * @param true to enable
     */
    public void setShowSysTray(boolean value) {
	showSysTray = value;
	firePropertyChange();
    }


    /**
     * @return true if the console is enabled
     */
    public boolean isShowExceptionEnabled() {
	return showException;
    }

    /**
     * @return true if the console is enabled
     */
    public boolean isSysTrayEnabled() {
	return showSysTray;
    }

    /**
     * Set the list of installed JRE on the local machine 
     * The vector contains <jre version, jre path>*nb of installed jres
     */
    public void setInstalledJREList(Vector vector) {
	String[][] temp = new String[vector.size()/2][2];
	int i=0;
        String version, location;
        
	for (Enumeration e = vector.elements() ; e.hasMoreElements();) {
	    version = (String) e.nextElement();
            location = (String) e.nextElement();
            if (version.trim().lastIndexOf(".")>2){
                /*
                 * This is a string of type x.x.x  Add it to the list.
                 */
                temp[i][0] = version;            	        
                temp[i][1] = location;
                i++;
            }
        }
        installedJREs = new String[i][2];
        for (int x=0;x<i;x++) {
            installedJREs[x][0] = temp[x][0];
            installedJREs[x][1] = temp[x][1];
        }
    }

    /**
     * @return the list of installed jre
     * the returned arrray is a two dimension array.
     * First arrray dimension is the number of JREs installed in the machine
     * the second dimension is always of size 2. The first element is the 
     * jre version, the second dimension of the jre path
     */
    public String[][] getInstalledJREList() {
	return installedJREs;
    }

    /**
     * Set the list of installed JDK on the local machine 
     * The vector contains <jdk version, jdk path>*nb of installed jres
     */
    public void setInstalledJDKList(Vector vector) {
	String[][] temp = new String[vector.size()/2][2];
	int i=0;
        String version, location;

	for (Enumeration e = vector.elements() ; e.hasMoreElements();) {
            version = (String) e.nextElement();
            location = (String) e.nextElement();            
            if ( version.trim().lastIndexOf(".")>2){
                /*
                 * This is a string of type x.x.x  Add it to the list.
                 */                
	        temp[i][0] = version;
	        temp[i][1] = location;
                i++;
            }
	}
        installedJDKs = new String[i][2];
        for (int x=0;x<i;x++) {
            installedJDKs[x][0] = temp[x][0];
            installedJDKs[x][1] = temp[x][1];
        }
    }

   /**
     * @return the list of installed jdk
     * the returned arrray is a two dimension array.
     * First arrray dimension is the number of JDKs installed in the machine
     * the second dimension is always of size 2. The first element is the 
     * jdk version, the second dimension of the jdk path
     */
    public String[][] getInstalledJDKList() {
	return installedJDKs;
    }

    public void setJavaRuntimeVersion(String version) {
	if (this.javaVersion == version) 
	    return;
	this.javaVersion = version;
	firePropertyChange();
    }

    public String getJavaRuntimeVersion() {
	return javaVersion;
    }

    public void setJavaRuntimeType(String type) 
    {
	if (type != null)
	{
	    if (type.equalsIgnoreCase("Default"))
	    {
		javaRuntimeType = "Default";
		javaRuntimePath = null;
	    }
	    else if (type.equalsIgnoreCase("JDK"))
	    {
		javaRuntimeType = "JDK";
		javaRuntimePath = null;
	    }
	    else if (type.equalsIgnoreCase("JRE"))
	    {
		javaRuntimeType = "JRE";
		javaRuntimePath = null;
	    }
	    else
	    {
		javaRuntimeType = "Other";
		javaRuntimePath = type;
	    }

	    firePropertyChange();
	}
    }
    
    public String getJavaRuntimeType() 
    {
	if (javaRuntimeType != null && javaRuntimeType.equalsIgnoreCase("Other"))
	    return javaRuntimePath;
	else
	    return javaRuntimeType;
    }

    /**
     * @return true if JPI should become IExplorer default VM
     */
    public boolean isIExplorerDefault() {
	return iexplorerDefault;
    }

    /**
     * Sets JPI to be IExplorer default VM
     *
     * @param b true for using JPI as default VM
     */
    public void setIExplorerDefault(boolean b) {
	iexplorerDefault=b;
	firePropertyChange();
    }

    /**
     * @return true if JPI should become Netscape6 default VM
     */
    public boolean isNetscape6Default() {
	return netscape6Default;
    }

    /**
     * Sets JPI to be Netscape6 default VM
     *
     * @param b true for using JPI as default VM
     */
    public void setNetscape6Default(boolean b) {
	netscape6Default=b;
	firePropertyChange();
    }

    /**
     * @return true if JPI should become Netscape6 default VM
     */
    public boolean isMozillaDefault() {
	return mozillaDefault;
    }

    /**
     * Sets JPI to be Netscape6 default VM
     *
     * @param b true for using JPI as default VM
     */
    public void setMozillaDefault(boolean b) {
	mozillaDefault=b;
	firePropertyChange();
    }

    /**
     * @return true if the javaplugin should use the browser settings
     */
    public boolean useDefaultProxy() {
	return useDefaultProxy;
    }

    /**
     * Sets the flag to use the default proxy settings of the browser
     *
     * @param b true for using the browser's settings
     */
    public void setDefaultProxySettings(boolean b) {
	useDefaultProxy=b;
	firePropertyChange();
    }

    /** 
     * Sets the proxy settings string
     * 
     * @param str encoded strings for proxy settings
     */
    public void setProxySettingsString(String str) {
	proxySettings = str;
	firePropertyChange();
    }

    /**
     * @return the proxy bypass list
     */
    public String getProxyByPass() {
	return proxyByPass;
    }

    /** 
     * Sets the proxy bypass list
     * 
     * @param list Proxy bypass list
     */
    public void setProxyByPass(String list) 
    {
	proxyByPass = list;
	firePropertyChange();
    }

    /**
     * @return the auto proxy URL
     */
    public String getAutoProxyURL() 
    {
	return autoProxyURL;
    }

    /** 
     * Sets the auto proxy URL
     * 
     * @param url Auto proxy URL
     */
    public void setAutoProxyURL(String url) 
    {
	autoProxyURL = url;
	firePropertyChange();
    }

    /**
     * @return the encoded proxy settings string
     */
    public String getProxySettingsString() {
	return proxySettings;
    }

    /**
     * Set the additional java parameters to be set when invoking the Java
     * Virtual Machine
     *
     * @parm str the parameters
     */
    public void setJavaParms(String str) {
	javaParms = str;
	firePropertyChange();
    }

    /**
     @return the additional parameters for the Java Virtual machine
     */
    public String getJavaParms() {
	return javaParms;
    }

    /**
     * Two set of certificate vectors are used to support the 
     * "Certificate Remove" option for removing Netscape Object 
     * Signing certificate from the store.
     * 
     * When the program starts, all certificates are in the "Active"
     * set of certificate vector. As users remove certificate from
     * the control panel, these certificates will be moved to the
     * "Inactive" set of certificate vector. When users click the
     * "Apply" button, the certificates in the "Inactive" set will 
     * be removed from the store. When users click the "Reset"
     * button, the certificates in the "Inactive" set will be
     * moved back to the "Active" set.
     *
     * Certificates are only loaded and saved when OnLoad() and
     * OnSave() are called.
     */

    /**
     * Return a collection which represents the list of alias 
     * of all the active certificate in the certificate store.
     *
     * @return Collection of active certificate alias
     */
    public Collection getCertificateAliases() {

	return activeCertsMap.keySet();
    }

    /* For Https Certificates */
    public Collection getHttpsCertificateAliases() {

        return activeHttpsCertsMap.keySet();
    }

    /* For Root CA */
    public Collection getRootCACertificateAliases() {

        return activeRootCACertsMap.keySet();
    }

    /* For Https Root CA */
    public Collection getHttpsRootCACertAliases() {

        return activeHttpsRootCACertsMap.keySet();
    }

    /* Get certificate from deployment.certs based on the alias */
    public Certificate getCertificate(String alias)
    {
	try
	{
	    // Open certificate store
	    CertificateStore certStore = new PluginCertificateStore();

	    // Load certificates from store
	    certStore.load();

	    // Obtain certificate iterator
	    Iterator iter = certStore.iterator();

	    while (iter.hasNext())
	    {
		X509Certificate cert = (X509Certificate) iter.next();

		String als = extractAliasName(cert);
		
		if (als.equals(alias))
		    return cert;
	    }
	}
	catch (Exception e)
	{
	    Trace.printException(e);
	}

	return null;
    }

    /* Get certificate from jpihttpscerts140 based on the alias */
    public Certificate getHttpsCertificate(String alias)
    {
        try
        {
            // Open certificate store
            CertificateStore certHttpsStore = new PluginHttpsCertStore();

            // Load certificates from store
            certHttpsStore.load();

            // Obtain certificate iterator
            Iterator iter = certHttpsStore.iterator();

            while (iter.hasNext())
            {
                X509Certificate certHttps = (X509Certificate) iter.next();

                String als = extractAliasName(certHttps);

                if (als.equals(alias))
                    return certHttps;
            }
        }
        catch (Exception e)
        {
            Trace.printException(e);
        }

        return null;
    }

    /* Get root certificate from cacerts based on the alias */
    public Certificate getRootCACertificate(String alias)
    {
        try
        {
            // Open Root CA certificate store
            CertificateStore cacertStore = new RootCACertificateStore();

            // Load certificates from store
            cacertStore.load();

            // Obtain certificate iterator
            Iterator iter = cacertStore.iterator();

            while (iter.hasNext())
            {
                X509Certificate rootCAcert = (X509Certificate) iter.next();

                String als = extractAliasName(rootCAcert);

                if (als.equals(alias))
                    return rootCAcert;
            }
        }
        catch (Exception e)
        {
            Trace.printException(e);
        }

        return null;
    }

    /* Get root certificate from jssecacerts based on the alias */
    public Certificate getHttpsRootCACertificate(String alias)
    {
        try
        {
            // Open Https Root CA certificate store
            CertificateStore jssecacertStore = new HttpsRootCACertStore();

            // Load certificates from store
            jssecacertStore.load();

            // Obtain certificate iterator
            Iterator iter = jssecacertStore.iterator();

            while (iter.hasNext())
            {
                X509Certificate HttpsRootCAcert = (X509Certificate) iter.next();

                String als = extractAliasName(HttpsRootCAcert);

                if (als.equals(alias))
                    return HttpsRootCAcert;
            }
        }
        catch (Exception e)
        {
            Trace.printException(e);
        }

        return null;
    }

    /**
     * Change a certificate from the active set to the inactive 
     * set of certificates for Netscape Object Signing. 
     *
     * @param index Index of certificate in active set
     */
    public void deactivateCertificate(String alias)  {

	Object cert = activeCertsMap.remove(alias);
	inactiveCertsMap.put(alias, cert);
	firePropertyChange();
    }

    /* Add a certificate to both active and inactiveImp set. */
    public void deactivateImpCertificate(X509Certificate cert)  {

        String alias = extractAliasName(cert);

        activeCertsMap.put(alias, cert);
        inactiveImpCertsMap.put(alias, cert);
        firePropertyChange();
    }

    /* Get Http certificate based on the alias */
    public X509Certificate getImpCertificate(String alias) {

	Object ImpCert  = inactiveImpCertsMap.get(alias);
	return (X509Certificate)ImpCert;
    }

    /**
     * Change a Https certificate from the active set to the inactive
     * set of Https certificates for Netscape Object Signing.
     *
     * @param index Index of Https certificate in active set
     */
    public void deactivateHttpsCertificate(String alias)  {

        Object certHttps = activeHttpsCertsMap.remove(alias);
        inactiveHttpsCertsMap.put(alias, certHttps);
        firePropertyChange();
    }

    /* Add a certificate to both active and inactiveImp set for Https. */
    public void deactivateImpHttpsCertificate(X509Certificate cert)  {

        String alias = extractAliasName(cert);

        activeHttpsCertsMap.put(alias, cert);
        inactiveImpHttpsCertsMap.put(alias, cert);
        firePropertyChange();
    }

    /* Get Https certificate based on the alias */
    public X509Certificate getImpHttpsCertificate(String alias) {

	Object ImpHttpsCert  = inactiveImpHttpsCertsMap.get(alias);
	return (X509Certificate)ImpHttpsCert;
    }

    public void setUpdateEnabled(boolean b) {
        update_enabled = b;
	firePropertyChange();
    }

    public boolean getUpdateEnabled() {
        return update_enabled;
    }

    public void setUpdateNotify(int i) {
        update_notify = i;
	firePropertyChange();
    }

    // return the index into ComboBox
    public int getUpdateNotify() {
        return update_notify;
    }

    public void setUpdateFrequency(int freq) {
	update_frequency = freq;
	firePropertyChange();
    }

    public void setUpdateDay(int day) {
        update_day = day;
	firePropertyChange();
    }

    public int getUpdateFrequency() {
        return update_frequency;
    }

    public int getUpdateDay() {
        return update_day;
    }

    public void setUpdateSchedule(int sched) {
        update_schedule = sched;
	firePropertyChange();
    }

    public int getUpdateSchedule() {
        return update_schedule;
    }

    public void setUpdateLastRun(String s) {
        update_lastrun = s;
	firePropertyChange();
    }

    // return the index into ComboBox
    public String getUpdateLastRun() {
        return update_lastrun;
    }


    /** 
     * Extract from quote
     */
    private String extractFromQuote(String s, String prefix)
    {
	if ( s == null)
	    return null;

	// Search for issuer name
	//
	int x = s.indexOf(prefix);
	int y = 0;

	if (x >= 0)
	{
	    x = x + prefix.length();

	    // Search for quote
	    if (s.charAt(x) == '\"')
	    {
		// if quote is found, search another quote

		// skip the first quote
		x = x + 1;
		
		y = s.indexOf('\"', x);
	    }
	    else
	    {
		// quote is not found, search for comma
		y = s.indexOf(',', x);
	    }

	    if (y < 0)
		return s.substring(x);			
	    else
		return s.substring(x, y);			
	}
	else
	{
	    // No match
	    return null;
	}
    }

    /**
     * Extrace CN from DN in the certificate.
     *
     * @param cert X509 certificate
     * @return CN
     */
    private String extractAliasName(X509Certificate cert)
    {
	String subjectName = mh.getMessage("unknownSubject");
	String issuerName = mh.getMessage("unknownIssuer");

	// Extract CN from the DN for each certificate
	try 
	{
       	    Principal principal = cert.getSubjectDN();
       	    Principal principalIssuer = cert.getIssuerDN();

	    // Extract subject name
	    String subjectDNName = principal.getName();
	    String issuerDNName = principalIssuer.getName();

	    // Extract subject name
	    subjectName = extractFromQuote(subjectDNName, "CN=");

	    // If no 'CN=' attribute available, we try 'O=' and 'OU'
	    if (subjectName == null)
	    {
		String subOName = extractFromQuote(subjectDNName, "O=");
		String subOUName = extractFromQuote(subjectDNName, "OU=");

		if (subOName!=null || subOUName!=null)
		{
		   MessageFormat mfSubject = new MessageFormat(mh.getMessage("certShowOOU"));
		   Object[] args = {subOName, subOUName};

		   if (subOName == null)
		      args[0] = "";

		   if (subOUName == null)
		      args[1] = "";

		   subjectName = mfSubject.format(args);
		}
	    }

	    if (subjectName == null)
		subjectName = mh.getMessage("unknownSubject");

	    // Extract issuer name
	    issuerName = extractFromQuote(issuerDNName, "CN=");

	    // If no 'CN=' attribute available, we try 'O=' and 'OU'
	    if (issuerName == null)
	    {
		String issOName = extractFromQuote(issuerDNName, "O=");
		String issOUName = extractFromQuote(issuerDNName, "OU=");

		if (issOName!=null || issOUName!=null)
		{
		   MessageFormat mfIssuer= new MessageFormat(mh.getMessage("certShowOOU"));
		   Object[] args = {issOName, issOUName};

		   if (issOName == null)
		      args[0] = "";

		   if (issOUName == null)
		      args[1] = "";

		   issuerName = mfIssuer.format(args);
		}
	    }

	    if (issuerName == null)
		issuerName = mh.getMessage("unknownIssuer");
	}
	catch (Exception e) 
	{
	    Trace.printException(e);
	}

	// Add Subject name and Issuer name in the return string
	MessageFormat mf = new MessageFormat(mh.getMessage("certShowName"));
	Object[] args = {subjectName, issuerName};
	return mf.format(args);
    }

    /**
     * Apply all the changes to the storage medium
     */
    public void applyChanges() throws java.io.IOException {
	// First, update the poperties we read earlier from disk.

	// Activator Console
        props.put("javaplugin.console", showConsole);  

	// show Exception dialogboxes
	props.put("javaplugin.exception", (showException?"true":"false"));

	// Proxy settings
	if (useDefaultProxy) {
	    props.put("javaplugin.proxy.usebrowsersettings", "true");
	} else {
	    props.put("javaplugin.proxy.usebrowsersettings", "false");
	}
	if (proxySettings!=null) {
    	    props.put("javaplugin.proxy.settings", proxySettings);
        } else {
	    props.remove("javaplugin.proxy.settings");
	}

	if (proxyByPass!=null) {
    	    props.put("javaplugin.proxy.bypass", proxyByPass);
        } else {
	    props.remove("javaplugin.proxy.bypass");
	}

	try
	{
	    if (autoProxyURL!=null && !autoProxyURL.trim().equals("")) 
	    {
		URL url = new URL(autoProxyURL);

    		props.put("javaplugin.proxy.auto.url", autoProxyURL);
	    }
	    else
	    {
		props.remove("javaplugin.proxy.auto.url");
	    }
	}
	catch (MalformedURLException ex1)
	{
	    DialogFactory.showExceptionDialog(ex1, mh.getMessage("proxy.autourl.invalid.text"),
					      mh.getMessage("proxy.autourl.invalid.caption"));

	    props.remove("javaplugin.proxy.auto.url");
	    autoProxyURL = null;
	}

	//
	// Store the JRE type into "javaplugin.jre.type".
	//
	// There are four possible options:
	// a) "Default"
	// b) "JDK"
	// c) "JRE"
	// d) "Other"
	//
	if (javaRuntimeType != null) {
	    props.put("javaplugin.jre.type", javaRuntimeType);
	} else {
	    javaRuntimeType = "Default";
	    props.put("javaplugin.jre.type", "Default");
	}


	//////////////////////////////////////////////////////////
	//
	// Store the JRE path into "javaplugin.jre.path". It always
	// contains the actual path to the JRE, or "Default".
	//
	String path = null;

	// If Java version is set, it must be either JDK/JRE
	if (javaVersion != null && javaVersion.length() > 0) 
	{
	    String[][] list = null;
	    if (javaRuntimeType.equalsIgnoreCase("JRE")) 
	    {
		list = getInstalledJREList();
	    }
	    if (javaRuntimeType.equalsIgnoreCase("JDK")) 
	    {
		list = getInstalledJDKList();
	    }
	    if (list != null) {
		for (int i=0; i<list.length; i++) {
		    if (list[i][0].equalsIgnoreCase(javaVersion)) 
			path = list[i][1];
		}	    
	    }
	}

	// If the Java Runtime type is "Other", we should use the path.
	//
	if (javaRuntimeType.equalsIgnoreCase("Other")) 
	{
	    // Make sure the path is not empty
	    if (javaRuntimePath != null && javaRuntimePath.trim().equals("") == false)
	    {
		path = javaRuntimePath;
	    }
	}

	// If JRE path not found or malformed, fallback to "default"
	if (path != null) 
	{
	    props.put("javaplugin.jre.path", path);
	}
	else 
	{
	    props.put("javaplugin.jre.type", "Default");
	    props.put("javaplugin.jre.path", "Default");
	}

	///////////////////////////////////////////////////////


	if (javaVersion != null && javaVersion.length() > 0) {
	    props.put("javaplugin.jre.version", javaVersion);
	} else {
	    props.remove("javaplugin.jre.version");
	}

	if (javaParms != null && javaParms.length() > 0) {
	    props.put("javaplugin.jre.params", javaParms);
	} else {
	    props.remove("javaplugin.jre.params");
	}


        // Save JAR caching parameters
        
        // Set JAR cache enabled
        if (!cacheEnabled) {
            props.put("javaplugin.cache.disabled", "true");
        } else {
            props.remove("javaplugin.cache.disabled");
        }
        
        // Set JAR cache location
        File dir = new File(cacheLocation);
        if (dir.equals(defaultCacheLocation)) {
            props.remove("javaplugin.cache.directory");
        } else {
            props.put("javaplugin.cache.directory", cacheLocation);
        }
        
        // Set JAR cache size
        if (!cacheSize.equals("50m")) {
            props.put("javaplugin.cache.size", cacheSize);
        } else {
            props.remove("javaplugin.cache.size");
        }
        
        // Set JAR cache compression
        if (JARCacheCompression != 0) {
            props.put("javaplugin.cache.compression", 
                      String.valueOf(JARCacheCompression));
        } else {
            props.remove("javaplugin.cache.compression");
        }

	try
	{
	    // Save properties
	    FileOutputStream fos = new FileOutputStream(theUserPropertiesFile);
	    BufferedOutputStream bos = new BufferedOutputStream(fos);
	    PrintWriter pw = new PrintWriter(bos);
	    pw.println(mh.getMessage("property_file_header"));
	    pw.flush();
	    props.store(bos, "");
	    bos.close();
	    fos.close();

	    ////////////////////////////////////////////////////////////////////
	    // Apply change to certificate store
	    //
	    CertificateStore certStore = new PluginCertificateStore();
	    CertificateStore certHttpsStore = new PluginHttpsCertStore();
	    
	    // Load certificates from store
	    certStore.load();
	    certHttpsStore.load();

	    // Remove inactive Http certificate one-by-one from store
	    Collection certs = inactiveCertsMap.values();
	    Iterator iter = certs.iterator();

	    while (iter.hasNext())
	    {
		certStore.remove((Certificate) iter.next());
	    }
	    
	    // add import certificate one-by-one from store for Http
            Collection certsImp = inactiveImpCertsMap.values();
            Iterator iterImp = certsImp.iterator();

            while (iterImp.hasNext())
            {
                certStore.add((Certificate) iterImp.next());
            }

            // Remove inactive Https certificate one-by-one from store
            Collection certsHttps = inactiveHttpsCertsMap.values();
            Iterator iterHttps = certsHttps.iterator();

            while (iterHttps.hasNext())
            {
                certHttpsStore.remove((Certificate) iterHttps.next());
            }

            // add import certificate one-by-one from store for Https
            Collection certsImpHttps = inactiveImpHttpsCertsMap.values();
            Iterator iterImpHttps = certsImpHttps.iterator();

            while (iterImpHttps.hasNext())
            {
                certHttpsStore.add((Certificate) iterImpHttps.next());
            }

	    // Save the changes
	    certStore.save();
	    certHttpsStore.save();
	}
	catch (Exception e)
	{
	    Trace.printException(e);
	}

	// Clear inactive certificate lists
	inactiveCertsMap.clear();
	inactiveImpCertsMap.clear();

	inactiveHttpsCertsMap.clear();
	inactiveImpHttpsCertsMap.clear();
    }

    /**
     * Read all the information from the configuration storage
     */
    public void reset() {
	firePropertyChange();
        props = new Properties();
	try {
	    FileInputStream fis = new FileInputStream(theUserPropertiesFile);
	    BufferedInputStream bis = new BufferedInputStream(fis);

	    props.load(bis);

	    bis.close();
	    fis.close();
	} catch (Exception e) {
	    // Its OK if the file is missing,
	}

	String propValue;
	propValue = props.getProperty("javaplugin.console");
	if (propValue != null) {
            showConsole = propValue;
        } else {
            showConsole = "hide";
        }

	propValue = props.getProperty("javaplugin.exception");
	if (propValue == null || propValue.equals("false")) {
	    showException = false;
	} else {
	    showException = true;
	}

	propValue = props.getProperty("javaplugin.proxy.usebrowsersettings");
	if (propValue == null || propValue.equals("true")) {
	    useDefaultProxy = true;
	} else {
	    useDefaultProxy = false;
	}
	
   	proxySettings = props.getProperty("javaplugin.proxy.settings");
   	proxyByPass = props.getProperty("javaplugin.proxy.bypass");
	
	try
	{
	    // Make sure the URL is in proper form
	    String autoURL = props.getProperty("javaplugin.proxy.auto.url");

	    if (autoURL != null && !autoURL.trim().equals(""))
	    {
		URL url = new URL(autoURL);
   		autoProxyURL = autoURL;
	    }
	}
	catch (MalformedURLException ex1)
	{
	    DialogFactory.showExceptionDialog(ex1,
					      mh.getMessage("proxy.autourl.invalid.text"),
					      mh.getMessage("proxy.autourl.invalid.caption"));

	    autoProxyURL = null;
	}

	// Check JRE path
	javaRuntimePath = props.getProperty("javaplugin.jre.path");

	// Check JRE type
	javaRuntimeType = props.getProperty("javaplugin.jre.type");

	if (javaRuntimeType == null) 
	{
	    // If nothing specify, if javaRuntimePath is specified, default to others
	    if (javaRuntimePath != null && javaRuntimePath.trim().equals("") == false)
		javaRuntimeType = "Other";
	    else
		javaRuntimeType = "Default";
	}
	else if (javaRuntimeType.equalsIgnoreCase("JRE"))
	{
	    javaRuntimeType = "JRE";
	}
	else if (javaRuntimeType.equalsIgnoreCase("JDK"))
	{
	    javaRuntimeType = "JDK";
	}
	else if (javaRuntimeType.equalsIgnoreCase("Other"))
	{
	    if (javaRuntimePath != null && javaRuntimePath.trim().equals("") == false) 
		javaRuntimeType = "Other";
	    else
		javaRuntimeType = "Default";
	}
	else
	{
	    // Unrecognized type, fall back to default.
	    javaRuntimeType = "Default";
	}

	if (javaRuntimeType == null)
	    javaRuntimeType = "Default";


	javaVersion = props.getProperty("javaplugin.jre.version");

	javaParms = props.getProperty("javaplugin.jre.params");


        // Read JAR caching parameters
        
        // Check if JAR cache is disabled
        propValue = props.getProperty("javaplugin.cache.disabled");
        if ((propValue != null) && propValue.equalsIgnoreCase("true")) {
            cacheEnabled = false;
        } else {
            cacheEnabled = true;
        }
        
        // Check JAR cache location
        defaultCacheLocation = new File(sun.plugin.util.UserProfile.getPluginCacheDirectory());

        cacheLocation = props.getProperty("javaplugin.cache.directory");
        if (cacheLocation == null) {
            cacheLocation = defaultCacheLocation.getPath();
        }
        
        // Check JAR cache size
        cacheSize = props.getProperty("javaplugin.cache.size", "50m");
        
        // Check JAR cache compression
        propValue = props.getProperty("javaplugin.cache.compression");
        if (propValue != null) {
            try {
                JARCacheCompression = Integer.valueOf(propValue).intValue();
            } catch (NumberFormatException e) {
                JARCacheCompression = 0;
            }
        } else {
            JARCacheCompression = 0;
        }

	//////////////////////////////////////////////////////////////////////////
    	// Load the certificates from the certificate store

	// Clear the certificate maps
	activeCertsMap.clear();
	inactiveCertsMap.clear();
	inactiveImpCertsMap.clear();

        activeHttpsCertsMap.clear();
        inactiveHttpsCertsMap.clear();
        inactiveImpHttpsCertsMap.clear();

        activeRootCACertsMap.clear();
        activeHttpsRootCACertsMap.clear();

	try
	{
	    // Open certificate store
	    CertificateStore certStore = new PluginCertificateStore();
	    CertificateStore certHttpsStore = new PluginHttpsCertStore();
            CertificateStore rootCAcertStore = new RootCACertificateStore();
            CertificateStore HttpsRootCAcertStore = new HttpsRootCACertStore();

	    // Load certificates from store
	    certStore.load();
	    certHttpsStore.load();
            rootCAcertStore.load();
            HttpsRootCAcertStore.load();

	    // Obtain certificate iterator
	    Iterator iter = certStore.iterator();
	    Iterator iterHttps = certHttpsStore.iterator();
            Iterator iterCA = rootCAcertStore.iterator();
            Iterator iterHttpsCA = HttpsRootCAcertStore.iterator();

	    while (iter.hasNext())
	    {
		X509Certificate cert = (X509Certificate) iter.next();
		String alias = extractAliasName(cert);
		activeCertsMap.put(alias, cert);
	    }

	    while (iterHttps.hasNext())
            {
                X509Certificate certHttps = (X509Certificate) iterHttps.next();
                String alias = extractAliasName(certHttps);
                activeHttpsCertsMap.put(alias, certHttps);
            }

            while (iterCA.hasNext())
            {
                X509Certificate rootCAcert = (X509Certificate) iterCA.next();
                String aliasRoot = extractAliasName(rootCAcert);
                activeRootCACertsMap.put(aliasRoot, rootCAcert);
            }

            while (iterHttpsCA.hasNext())
            {
                X509Certificate HttpsRootCAcert = (X509Certificate) iterHttpsCA.next();
                String aliasHttpsRoot = extractAliasName(HttpsRootCAcert);
                activeHttpsRootCACertsMap.put(aliasHttpsRoot, HttpsRootCAcert);
            }

	}
	catch (Exception e)
	{
	    Trace.printException(e);
	}
    }

    // Added methods for getting and setting JAR caching parameters
    
    boolean getCacheEnabled() {
        return cacheEnabled;
    }
    
    void setCacheEnabled(boolean enabled) {
        cacheEnabled = enabled;
        firePropertyChange();
    }
    
    String getCacheLocation() {
        return cacheLocation;
    }
    
    void setCacheLocation(String location) {
        cacheLocation = location;
        firePropertyChange();
    }
    
    String getCacheSize() {
        return cacheSize;
    }
    
    void setCacheSize(String size) {
        cacheSize = size;
        firePropertyChange();
    }

    int getJARCacheCompression() {
        return JARCacheCompression;
    }
    
    void setJARCacheCompression(int compression) {
        JARCacheCompression = compression;
        firePropertyChange();
    }

    /**
     * Notify our PropertyChangeListeners that we have changed.
     */
    protected void firePropertyChange() {
	pcs.firePropertyChange(null, null, null);
    }

    /**
     * Add a property change listener.
     */
    public void addPropertyChangeListener(PropertyChangeListener pcl) {
	pcs.addPropertyChangeListener(pcl);
    }

    /**
     * Remove a property change listener.
     */
    public void removePropertyChangeListener(PropertyChangeListener pcl) {
	pcs.removePropertyChangeListener(pcl);
    }

    public void setShowBrowserPanel(boolean x) {
	showBrowserPanel = x;
    }

    public boolean isShowBrowserPanel() {
	return showBrowserPanel;
    }

    static private File theUserPropertiesFile;

    // The properties we read from disk.
    private Properties props;

    private String	showConsole = "hide";    //default - as it was before - start Console, but do not show it = hide.
    private boolean	showException = false;
    private boolean	showSysTray = true;
    private String[][]  installedJREs;
    private String[][]  installedJDKs;
    private boolean	useDefaultProxy = true;
    private String	proxySettings;
    private String	proxyByPass;
    private String      autoProxyURL;
    private String	javaParms;
    private String      javaVersion;
    private String      javaRuntimePath = null;
    private String	javaRuntimeType = "Default";

    // JAR caching parameters
    private boolean     cacheEnabled;
    private String      cacheLocation = null;
    private String      cacheSize;
    private int         JARCacheCompression;
    private File        defaultCacheLocation;

    // Browser panel
    private boolean	iexplorerDefault = false;
    private boolean	netscape6Default = false;
    private boolean     mozillaDefault   = false;
    private boolean	showBrowserPanel = false;


    // For RSA signing support in Win32 and UNIX
    private HashMap	activeCertsMap = new HashMap();
    private HashMap	inactiveCertsMap = new HashMap();
    private HashMap     inactiveImpCertsMap = new HashMap();

    private HashMap     activeHttpsCertsMap = new HashMap();
    private HashMap     inactiveHttpsCertsMap = new HashMap();
    private HashMap     inactiveImpHttpsCertsMap = new HashMap();

    private HashMap     activeRootCACertsMap = new HashMap();
    private HashMap     activeHttpsRootCACertsMap = new HashMap();

    private PropertyChangeSupport pcs = new PropertyChangeSupport(this);

    private MessageHandler mh = new MessageHandler("config");

    // Update Panel
    private boolean	update_enabled = true;
    private int		update_notify = 0;
    private int		update_frequency = 1;
    private int		update_day = 1;
    private int		update_schedule = 0;
    private String	update_lastrun ;
}
