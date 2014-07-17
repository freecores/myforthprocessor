/*
 * @(#)MPlatformService.java	1.4 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.services;

import java.io.File;


/** 
 * MPlatformService is a class that encapsulates the platform service
 * in Unix.
 */
public final class MPlatformService extends PlatformService
{
    /** 
     * Return user profile path in Deployment Infrastructure.
     */
    public String getUserProfilePath()
    {
	return (String) java.security.AccessController.doPrivileged(
	       	        new sun.security.action.GetPropertyAction("user.home"));
    }

    /**
     * Return system profile path in Deployment Infrastructure.
     */
    public String getSystemProfilePath()
    {
	return "/etc";
    }

    /**
     * Return deployment infrastructure path.
     */
    public String getInfrastructurePath()
    {
	return ".java" + File.separator + "deployment";
    }
}
