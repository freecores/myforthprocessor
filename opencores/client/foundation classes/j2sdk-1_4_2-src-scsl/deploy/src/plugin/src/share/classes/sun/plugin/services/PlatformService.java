/*
 * @(#)PlatformService.java	1.5 03/02/13
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.services;

import sun.plugin.util.Trace;


/** 
 * PlatformService is an interface that encapsulates the platform service.
 *
 * @since 1.4.2
 */
public abstract class PlatformService
{
    /** 
     * Return user profile path in Deployment Infrastructure.
     */
    public abstract String getUserProfilePath();

    /**
     * Return system profile path in Deployment Infrastructure.
     */
    public abstract String getSystemProfilePath();

    /**
     * Return deployment infrastructure path.
     */
    public abstract String getInfrastructurePath();    

    /**
     * Signal an event, the default behavior is doing nothing
     */
    public void signalEvent(int handle) {
    }

    /**
     * Wait on an event, the default behavior is doing nothing
     */
     public void waitEvent(int handle) {
     }

     public void waitEvent(int winHandle, int handle) {
	waitEvent(handle);
     }

    /**
     * Dispatch platform native event, the default behavior is doing nothing
     */
    public void dispatchNativeEvent() {
    }

    /**
     * Return platform service.
     */
    public static synchronized PlatformService getService()
    {
	if (ps == null)
	{
	    String osName = (String) java.security.AccessController.doPrivileged(
	 	    	    new sun.security.action.GetPropertyAction("os.name"));
	    
	    try 
	    {
		String platformClassName = null;

		if (osName.indexOf("Windows") != -1)
		    platformClassName = "sun.plugin.services.WPlatformService";
		else
		    platformClassName = "sun.plugin.services.MPlatformService";

		Class platformClass = Class.forName(platformClassName);

		if (platformClass != null) 
		{
		    Object obj = platformClass.newInstance();

		    if (obj instanceof PlatformService) 
			ps = (PlatformService) obj;
		}
	    } 
	    catch(Exception e) 
	    {
		Trace.printException(e);
	    }
	}
	
	return ps;
    }


    // Platform service instance
    private static PlatformService ps = null;
}


