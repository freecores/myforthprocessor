/*
 * @(#)DeploymentConfig.java	1.11 03/02/12
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.util;

import java.io.BufferedInputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.net.URI;
import java.net.URL;
import java.net.MalformedURLException;
import java.net.URISyntaxException;
import java.util.Properties;


/**
 * Deployment Configuration in the Deployment Infrastructure.
 *
 * @author Stanley Man-Kit Ho
 */
public abstract class DeploymentConfig
{
    // Properties loaded from deployment.config
    private Properties configFileProps = null;
         
    /**
     *  Construct a DeploymentConfig object.
     */
    public DeploymentConfig()
    {
	// load deployment config properties
	configFileProps = loadConfigProperties();

	// Make sure all the directories are created
	new File(getUserHome()).mkdirs();
	new File(getSystemHome()).mkdirs();
	new File(getCacheDirectory()).mkdirs();
	new File(getTempDirectory()).mkdirs();
	new File(getLogDirectory()).mkdirs();
	new File(getSecurityDirectory()).mkdirs();
	new File(getExtensionDirectory()).mkdirs();
	new File(new File(getPropertiesFile()).getParent()).mkdirs();
    }


    /**
     * Load deployment config properties from deployment.config file
     */
    private Properties loadConfigProperties()
    {
	Properties props = new Properties();

	// Check if users have overridden any deployment configuration in deployment.config
	//
	try
	{
	    // Load properties from {deployment.user.home}/deployment.config file
	    //
	    FileInputStream fis = new FileInputStream(getUserHome() + File.separator + "deployment.config");
	    BufferedInputStream bis = new BufferedInputStream(fis);

	    props.load(bis);

	    bis.close();
	    fis.close();
	}
	catch(FileNotFoundException e)
	{
	    // Config file doesn't exists
	}
	catch(IOException e2)
	{
	    // Config file cannot be read
	}

	// Check if users have overridden any deployment configuration in deployment.properties
	//
	try
	{
	    Properties props2 = new Properties();

	    // Load properties from {deployment.user.home}/deployment.properties file
	    //
	    FileInputStream fis = new FileInputStream(getPropertiesFile());
	    BufferedInputStream bis = new BufferedInputStream(fis);

	    props2.load(bis);

	    bis.close();
	    fis.close();

	    // Extract {deployment.user.cachedir}
	    String cacheDir = props2.getProperty(InfrastructureConstant.Properties.CACHE_DIR);

	    if (cacheDir != null)
		props.setProperty(InfrastructureConstant.Properties.CACHE_DIR, cacheDir);

	    // Extract {deployment.user.logdir}
	    String logDir = props2.getProperty(InfrastructureConstant.Properties.LOG_DIR);

	    if (logDir != null)
		props.setProperty(InfrastructureConstant.Properties.LOG_DIR, logDir);
	}
	catch(FileNotFoundException e)
	{
	    // Config file doesn't exists
	}
	catch(IOException e2)
	{
	    // Config file cannot be read
	}

	return props;
    }


    /**
     * Returns java home in the Deployment Infrastructure.
     */
    public String getJavaHome()
    {
	return (String) java.security.AccessController.doPrivileged(
	 	    	new sun.security.action.GetPropertyAction("java.home"));
    }


    /**
     * Returns deployment infrastructure path.
     */
    protected abstract String getInfrastructurePath();


    /**
     * Returns user home in the Deployment Infrastructure.
     */
    public String getUserHome()
    {
	// ${deployment.user.profile}\Java\Deployment   (Windows)
	// ${deployment.user.profile}/.java/deployment  (Unix)
	//
	return getUserProfilePath() + File.separator + getInfrastructurePath();
    }


    /**
     * Returns the user profile path in the Deployment Infrastructure.
     */
    protected abstract String getUserProfilePath();


    /**
     * Returns system home in the Deployment Infrastructure.
     */
    public String getSystemHome()
    {
	// ${deployment.system.profile}\Java\Deployment   (Windows)
	// ${deployment.system.profile}/.java/deployment  (Unix)
	//
	return getSystemProfilePath() + File.separator + getInfrastructurePath();
    }

    
    /**
     * Returns the system profile path in the Deployment Infrastructure.
     */
    protected abstract String getSystemProfilePath();


    /**
     * Returns path to the deployment properties file.
     */
    public String getPropertiesFile()
    {
	// ${deployment.user.home}\deployment.properties  (Windows)
	// ${deployment.user.home}/deployment.properties  (Unix)
	//
	return getUserHome() + File.separator + getPropertiesFilename();
    }


    /**
     * Returns deployment properties filename.
     */
    protected String getPropertiesFilename()
    {
	return "deployment.properties";
    }


    /**
     * Returns the temp directory in the Deployment Infrastructure.
     */
    public String getTempDirectory()
    {
	// ${deployment.user.cachedir}\tmp  (Windows)
	// ${deployment.user.cachedir}/tmp  (Unix)
	//
	return getCacheDirectory() + File.separator + "tmp";
    }


    /**
     * Returns the cache directory in the Deployment Infrastructure.
     */
    public String getCacheDirectory()
    {
	// Check if cache directory has been set in deployment.config
	//
	String cacheDirURL = configFileProps.getProperty(InfrastructureConstant.Properties.CACHE_DIR);

        if (cacheDirURL != null && cacheDirURL.startsWith("file:")) 
        {
            // Convert cacheDirURL from URL to file path
            try
            {
                return new File(URI.create(cacheDirURL)).toString();
            }
            catch (IllegalArgumentException e)
            {
            }
        }

	// ${deployment.user.home}\cache  (Windows)
	// ${deployment.user.home}/cache  (Unix)
	//
	return getUserHome() + File.separator + "cache";
    }


    /**
     * Returns the log directory in the Deployment Infrastructure.
     */
    public String getLogDirectory()
    {
	// Check if log directory has been set in deployment.config
	//
	String logDirURL = configFileProps.getProperty(InfrastructureConstant.Properties.LOG_DIR);

        if (logDirURL != null && logDirURL.startsWith("file:")) 
        {
            try {
                return new File(URI.create(logDirURL)).toString();
            }
            catch(IllegalArgumentException e)
            {
            }
        }

        // ${deployment.user.home}\log  (Windows)
        // ${deployment.user.home}/log  (Unix)
        //
        return getUserHome() + File.separator + "log";
    }


    /**
     * Returns the ext directory in the Deployment Infrastructure.
     */
    public String getExtensionDirectory()
    {
	// Check if ext directory has been set in deployment.config
	//
	String extDirURL = configFileProps.getProperty(InfrastructureConstant.Properties.EXT_DIR);

        if (extDirURL != null && extDirURL.startsWith("file:")) 
        {
            // Convert extDirURL from URL to file path
            try
            {
                return new File(URI.create(extDirURL)).toString();
            }
            catch (IllegalArgumentException e)
            {
            }
        }

        // ${deployment.user.home}\ext  (Windows)
        // ${deployment.user.home}/ext  (Unix)
        //
        return getUserHome() + File.separator + "ext"; 
    }


    /**
     * Returns the security directory in the Deployment Infrastructure.
     */
    public String getSecurityDirectory()
    {
	// ${deployment.user.home}\security  (Windows)
	// ${deployment.user.home}/security  (Unix)
	//
	return getUserHome() + File.separator + "security";
    }


    /**
     * Returns the user specific security policy file in the Deployment Infrastructure.
     */
    public URL getUserSecurityPolicyURL()
    {
	// Check if user security policy has been set in deployment.config
	//
	String userSecurityPolicyURL = configFileProps.getProperty(InfrastructureConstant.Properties.USER_SECURITY_POLICY);

        if (userSecurityPolicyURL != null)
        {
            // Convert userSecurityPolicyURL from URL to file path
            try
            {
                return URI.create(userSecurityPolicyURL).toURL();
            }
            catch (IllegalArgumentException e)
            {
            }
            catch (MalformedURLException e2)
            {
            }
        }

        // ${deployment.user.home}\security\java.policy  (Windows)
        // ${deployment.user.home}/security/java.policy  (Unix)
        //
        try
        {
            return new File(getUserHome() + File.separator + "security" + File.separator + "java.policy").toURI().toURL();
        }
        catch (MalformedURLException e)
        {
            return null;
        }
    }


    /**
     * Returns the system security policy file in the Deployment Infrastructure.
     */
    public URL getSystemSecurityPolicyURL()
    {
        // Check if system security policy has been set in deployment.config
        //
        String systemSecurityPolicyURL = configFileProps.getProperty(InfrastructureConstant.Properties.SYSTEM_SECURITY_POLICY);

        if (systemSecurityPolicyURL != null)
        {
            // Convert systemSecurityPolicyURL from URL to file path
            try
            {
                return URI.create(systemSecurityPolicyURL).toURL();
            }
            catch (IllegalArgumentException e)
            {
            }
            catch (MalformedURLException e2)
            {
            }
        }

        // ${deployment.system.home}\security\java.policy  (Windows)
        // ${deployment.system.home}/security/java.policy  (Unix)
        //
        try
        {
            return new File(getSystemHome() + File.separator + "security" + File.separator + "java.policy").toURI().toURL();
        }
        catch (MalformedURLException e)
        {
            return null;
        }  
    }


    /**
     * Returns the root ca certstore in the Deployment Infrastructure.
     */
    public String getRootCACertStore()
    {
	// Check if cacerts has been set in deployment.config
	//
	String certStoreURL = configFileProps.getProperty(InfrastructureConstant.Properties.SYSTEM_CACERTS);

        if (certStoreURL != null && certStoreURL.startsWith("file:"))
        {
            // Convert certStoreURL from URL to file path
            try
            {
                return new File(URI.create(certStoreURL)).toString();
            }
            catch (IllegalArgumentException e)
            {
            }
        }

        // ${deployment.system.home}\security\cacerts  (Windows)
        // ${deployment.system.home}/security/cacerts  (Unix)
        //
        String certStore = getSystemHome() + File.separator + "security" + File.separator + "cacerts";

        // Check if certstore exists
        //
        if (new File(certStore).exists() == false)
        {
            // ${deployment.java.home}\lib\security\cacerts  (Windows)
            // ${deployment.java.home}/lib\security/cacerts  (Unix)
            //
            certStore = getJavaHome() + File.separator + "lib" + File.separator + "security" + File.separator + "cacerts";
        }

        return certStore;           
    }


    /**
     * Returns the jsse ca certstore in the Deployment Infrastructure.
     */
    public String getJsseCACertStore()
    {
	// Check if jsse cacerts has been set in deployment.config
	//
	String certStoreURL = configFileProps.getProperty(InfrastructureConstant.Properties.SYSTEM_JSSECACERTS);

        if (certStoreURL != null && certStoreURL.startsWith("file:"))
        {
            // Convert certStoreURL from URL to file path
            try
            {
                return new File(URI.create(certStoreURL)).toString();
            }
            catch (IllegalArgumentException e)
            {
            }
        }
	
	// ${deployment.system.home}\security\jssecacerts  (Windows)
	// ${deployment.system.home}/security/jssecacerts  (Unix)
	//
	String certStore = getSystemHome() + File.separator + "security" + File.separator + "jssecacerts";

	// Check if certstore exists
	//
	if (new File(certStore).exists() == false)
	{
	    // ${deployment.system.home}\security\cacerts  (Windows)
	    // ${deployment.system.home}/security/cacerts  (Unix)
	    //
	    certStore = getSystemHome() + File.separator + "security" + File.separator + "cacerts";

	    // Check if certstore exists
	    //
	    if (new File(certStore).exists() == false)
	    {
		// ${java.home}\security\jssecacerts  (Windows)
		// ${java.home}/security/jssecacerts  (Unix)
		//
		certStore = getJavaHome() + File.separator + "lib" + File.separator + "security" + File.separator + "jssecacerts";

		// Check if certstore exists
		//
		if (new File(certStore).exists() == false)
		{
		    // ${java.home}\lib\security\cacerts  (Windows)
		    // ${java.home}/lib\security/cacerts  (Unix)
		    //
		    certStore = getJavaHome() + File.separator + "lib" + File.separator + "security" + File.separator + "cacerts";
		}
	    }
	}

	return certStore;	    
    }


    /**
     * Returns the trusted signing certstore in the Deployment Infrastructure.
     */
    public String getTrustedSigningCertStore()
    {
        // Check if trusted signing certstore has been set in deployment.config
        //
        String certStoreURL = configFileProps.getProperty(InfrastructureConstant.Properties.USER_CERTS);

        if (certStoreURL != null && certStoreURL.startsWith("file:"))
        {
            // Convert certStoreURL from URL to file path
            try
            {
                return new File(URI.create(certStoreURL)).toString();
            }
            catch (IllegalArgumentException e)
            {
            }
        }

        // ${deployment.user.home}\security\deployment.certs  (Windows)
        // ${deployment.user.home}/security/deployment.certs  (Unix)
        //
        return getUserHome() + File.separator + "security" + File.separator + "deployment.certs";
    }


    /**
     * Returns the trusted jsse certstore in the Deployment Infrastructure.
     */
    public String getTrustedJsseCertStore()
    {
        // Check if trusted jsse certstore has been set in deployment.config
        //
        String certStoreURL = configFileProps.getProperty(InfrastructureConstant.Properties.USER_JSSECERTS);

        if (certStoreURL != null && certStoreURL.startsWith("file:"))
        {
            // Convert certStoreURL from URL to file path
            try
            {
                return new File(URI.create(certStoreURL)).toString();
            }
            catch (IllegalArgumentException e)
            {
            }
        }

        // ${deployment.user.home}\security\deployment.jssecerts  (Windows)
        // ${deployment.user.home}/security/deployment.jssecerts  (Unix)
        //
        return getUserHome() + File.separator + "security" + File.separator + "deployment.jssecerts";
    }


    /**
     * Returns the set of properties in the Deployment Infrastructure.
     */
    public Properties getConfigProperties()
    {
	Properties props = new Properties();
	
	props.setProperty(InfrastructureConstant.Properties.USER_PROFILE, getUserProfilePath());
	props.setProperty(InfrastructureConstant.Properties.SYSTEM_PROFILE, getSystemProfilePath());
	props.setProperty(InfrastructureConstant.Properties.USER_HOME, getUserHome());
	props.setProperty(InfrastructureConstant.Properties.SYSTEM_HOME, getSystemHome());
	props.setProperty(InfrastructureConstant.Properties.CACHE_DIR, getCacheDirectory());
	props.setProperty(InfrastructureConstant.Properties.LOG_DIR, getLogDirectory());
	props.setProperty(InfrastructureConstant.Properties.EXT_DIR, getExtensionDirectory());
	props.setProperty(InfrastructureConstant.Properties.TMP_DIR, getTempDirectory());
	props.setProperty(InfrastructureConstant.Properties.USER_SECURITY_POLICY, getUserSecurityPolicyURL().toString());
	props.setProperty(InfrastructureConstant.Properties.SYSTEM_SECURITY_POLICY, getSystemSecurityPolicyURL().toString());
	props.setProperty(InfrastructureConstant.Properties.USER_CERTS, getTrustedSigningCertStore());
	props.setProperty(InfrastructureConstant.Properties.USER_JSSECERTS, getTrustedJsseCertStore());
	props.setProperty(InfrastructureConstant.Properties.SYSTEM_CACERTS, getRootCACertStore());
	props.setProperty(InfrastructureConstant.Properties.SYSTEM_JSSECACERTS, getJsseCACertStore());

	return props;
    }
}
