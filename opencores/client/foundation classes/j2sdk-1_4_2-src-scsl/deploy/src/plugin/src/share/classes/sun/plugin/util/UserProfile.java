/*
 * @(#)UserProfile.java	1.18 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.util;

import java.io.File;
import java.util.Date;
import java.util.Properties;


/**
 * UserProfile object encapsulates all the user specific 
 * infrastructure information to be used by Java Plug-in,
 */
public class UserProfile
{
    // Deployment configuration
    private static PluginConfig config = new PluginConfig();

    static
    {
	// Ensure directory is created
	try
	{
	    new File(getPluginCacheDirectory()).mkdirs();
	    new File(getJarCacheDirectory()).mkdirs();
	    new File(getFileCacheDirectory()).mkdirs();
	    new File(getExtensionDirectory()).mkdirs();
	    new File(getTempDirectory()).mkdirs();
	}
	catch (Throwable e)
	{   
	    e.printStackTrace();
	}
    }	
    

    /**
     * Generate filename.
     */	
    private static String generateFilename(String fileExt)
    {
	StringBuffer nameBuffer = new StringBuffer();
	
	// Prefix file with .plugin
	//
	nameBuffer.append("plugin");

	String noDotVersion = (String) java.security.AccessController.doPrivileged(
			      new sun.security.action.GetPropertyAction("javaplugin.nodotversion"));

	nameBuffer.append(noDotVersion);
	

	// Check whether user want to generate unique file.
	//
	Boolean overWrite = (Boolean) java.security.AccessController.doPrivileged(
			    new sun.security.action.GetBooleanAction("javaplugin.outputfiles.overwrite"));

	if (overWrite == Boolean.FALSE)
	{
	    // Generate unique filename using user.name and timestamp
	    //
	    String userName = (String) java.security.AccessController.doPrivileged(
		    	      new sun.security.action.GetPropertyAction("user.name"));
	    nameBuffer.append(userName);
	    nameBuffer.append(new Date().hashCode());
	}

	nameBuffer.append(fileExt);

	return nameBuffer.toString();
    }
    

    /**
     * Returns the plugin property file for this user.
     */
    public static String getPropertyFile()
    {
	return config.getPropertiesFile();
    }


    /**
     * Returns the trace file for this user.
     */
    public static String getTraceFile()
    {	
	return config.getLogDirectory() + File.separator + generateFilename(".trace");
    }


    /**
     * Returns the log file for this user.
     */
    public static String getLogFile()
    {
	return config.getLogDirectory() + File.separator + generateFilename(".log");
    }
    

    /**
     * Returns the top-level cache directory for this user.
     */
    public static String getPluginCacheDirectory()
    {
	return config.getPluginCacheDirectory();
    }

    /**
     * Returns the jar cache directory for this user.
     */
    public static String getJarCacheDirectory()
    {
	return config.getPluginCacheDirectory() + File.separator + "jar";
    }


    /**
     * Returns the file cache directory for this user.
     */
    public static String getFileCacheDirectory()
    {
	return config.getPluginCacheDirectory() + File.separator + "file";
    }


    /**
     * Returns the ext directory for this user.
     */
    public static String getExtensionDirectory()
    {
	return config.getPluginCacheDirectory() + File.separator + "ext";
    }


    /**
     * Returns the temp directory for this user.
     */
    public static String getTempDirectory()
    {
	return config.getPluginCacheDirectory() + File.separator + "tmp";
    }


    /**
     * Returns the root ca certstore for this user.
     */
    public static String getRootCertificateFile()
    {
	return config.getRootCACertStore();
    }


    /**
     * Returns the jsse ca certstore for this user.
     */
    public static String getHttpsRootCertFile()
    {
	return config.getJsseCACertStore();
    }


    /**
     * Returns the trusted signing certstore for this user.
     */
    public static String getCertificateFile()
    {
	return config.getTrustedSigningCertStore();
    }


    /**
     * Returns the trusted jsse certstore for this user.
     */
    public static String getHttpsCertificateFile()
    {
	return config.getTrustedJsseCertStore();
    }

    /**
     * Returns the set of properties in the plugin config infrastructure.
     */
    public static Properties getConfigProperties()
    {
	return config.getConfigProperties();
    }
}

