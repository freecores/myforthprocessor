/*
 * @(#)InfrastructureConstant.java	1.7 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.util;

import java.io.File;


/**
 * Constants used in the Deployment Infrastructure.
 *
 * @author Stanley Man-Kit Ho
 */
public final class InfrastructureConstant
{
    // Namespaces to be used in the deployment infrastructure
    //
    public static final class Namespaces
    {
        public static final String JAVAPI = "javapi";
	public static final String JAVAWS = "javaws";
    };

    // System properties used in the deployment infrastructure
    //
    public static final class Properties
    {
	static final String USER_PROFILE = "deployment.user.profile";
	static final String SYSTEM_PROFILE = "deployment.system.profile";
	static final String USER_HOME = "deployment.user.home";
	static final String SYSTEM_HOME = "deployment.system.home";
	static final String CACHE_DIR = "deployment.user.cachedir";
	static final String LOG_DIR = "deployment.user.logdir";
	static final String EXT_DIR = "deployment.user.extdir";
	static final String TMP_DIR = "deployment.user.tmpdir";
	static final String USER_SECURITY_POLICY = "deployment.user.security.policy";
	static final String SYSTEM_SECURITY_POLICY = "deployment.system.security.policy";
	static final String USER_CERTS = "deployment.user.certs";
	static final String USER_JSSECERTS = "deployment.user.jssecerts";
	static final String SYSTEM_CACERTS = "deployment.system.cacerts";
	static final String SYSTEM_JSSECACERTS = "deployment.system.jssecacerts";
    };
}
