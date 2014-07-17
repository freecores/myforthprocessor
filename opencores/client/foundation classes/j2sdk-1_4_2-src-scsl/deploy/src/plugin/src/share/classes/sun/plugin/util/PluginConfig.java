/*
 * @(#)PluginConfig.java	1.11 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.util;

import java.io.File;
import sun.plugin.services.PlatformService;


/**
 * Plugin Configuration in the Deployment Infrastructure.
 *
 * @author Stanley Man-Kit Ho
 */
final class PluginConfig extends DeploymentConfig
{    
    private static final String CACHE_VERSION = "v1.0";

    /**
     * Construct PluginConfig object.
     */    
    PluginConfig()
    {
	new File(getPluginCacheDirectory()).mkdirs();
    }


    /**
     * Returns the user profile path in the Deployment Infrastructure.
     */
    protected String getUserProfilePath()
    {
	// Check for the user specified plugin home directory
	String homeDir = (String) java.security.AccessController.doPrivileged(
			 new sun.security.action.GetPropertyAction("javaplugin.user.profile"));

	if (homeDir == null || homeDir.trim().equals(""))
	{
	    // Obtain user path from Platform service
	    //
	    homeDir = PlatformService.getService().getUserProfilePath();

	    // Default to {user.home}
	    //
	    if (homeDir == null)
	    {
		homeDir = (String) java.security.AccessController.doPrivileged(
			 new sun.security.action.GetPropertyAction("user.home"));
    	    }
	}

	return homeDir;
    }


    /**
     * Returns the system profile path in the Deployment Infrastructure.
     */
    protected String getSystemProfilePath()
    {
	// Obtain system path from Platform service
	//
	String systemProfile = PlatformService.getService().getSystemProfilePath();

	// Default to {deployment.user.profile}
	//
	if (systemProfile == null)
	{
	    systemProfile = getUserProfilePath();
    	}

	return systemProfile;
    }


    /**
     * Returns deployment infrastructure path.
     */
    protected String getInfrastructurePath()
    {
	// Obtain infrastructure path from Platform service
	//
	return PlatformService.getService().getInfrastructurePath();
    }
    
    /** 
     * Returns the directory structure under the top of the cache directory
     */
    private String getCacheDirectorySubStructure()
    {
        return InfrastructureConstant.Namespaces.JAVAPI + File.separator
	    	 + CACHE_VERSION;
    }
    
    /**
     * Returns the cache directory in the Deployment Infrastructure.
     */
    public String getPluginCacheDirectory()
    {
	// Check for the user specified cache directory
	//
	String cacheDir = (String) java.security.AccessController.doPrivileged(
			   new sun.security.action.GetPropertyAction("javaplugin.cache.directory"));

	if (cacheDir == null || cacheDir.trim().equals(""))
	{
	    // Use default
	    cacheDir = super.getCacheDirectory() + File.separator + getCacheDirectorySubStructure();
	}

	return cacheDir;
    }


    /**
     * Returns the log directory in the Deployment Infrastructure.
     */
    public String getLogDirectory()
    {
	// Check for any user specified path for output files
	//
	String logDir = (String) java.security.AccessController.doPrivileged(
			 new sun.security.action.GetPropertyAction("javaplugin.outputfiles.path"));

	if (logDir == null || logDir.trim().equals(""))
	{
	    // Use default
	    logDir = super.getLogDirectory();
	}

	return logDir;
    }
}
