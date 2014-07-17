/*
 * @(#)LoginUtility.java	1.4 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.security.jgss;

import javax.security.auth.login.LoginContext;
import javax.security.auth.login.LoginException;
import java.security.PrivilegedExceptionAction;
import java.security.AccessController;
import sun.security.action.GetPropertyAction;
import com.sun.security.auth.callback.DialogCallbackHandler;
import com.sun.security.auth.callback.TextCallbackHandler;

/*
 * This utility returns a credential that is not part of the current
 * Subject. It does this by first performing a JAAS login, and then
 * searching for the credential in the Subject that is returned by that
 * login. This Subject might be entirely different from the Subject on the
 * current ACC. In fact, the current ACC might not even have a Subject
 * available.
 *
 * @author Mayank Upadhyay
 * @version 1.4, 01/23/03
 * @since 1.4
 */

public class LoginUtility implements PrivilegedExceptionAction {

    public static String GSS_INITIATE_ENTRY = 
	"com.sun.security.jgss.initiate";

    public static String GSS_ACCEPT_ENTRY = 
	"com.sun.security.jgss.accept";

    String configEntry;

    /**
     * Instantiates a LoginUtility that will use the JAAS configuration entry
     * configEntry.
     */
    public LoginUtility(String configEntry) {
	this.configEntry = configEntry;
    }

    /**
     * Performs the login and returns the authenticated Subject.
     */
    public Object run() throws LoginException {
	LoginContext lc = null;
	// TBD: Which callback handler should we use?
	//lc = new LoginContext(configEntry, new DialogCallbackHandler());
	lc = new LoginContext(configEntry, new TextCallbackHandler());
	lc.login();
	return lc.getSubject();
    }

    /**
     * Determines if the application doesn't mind if the mechanism obtains
     * the required credentials from outside of the current Subject. Our
     * Kerberos v5 mechanism would do a JAAS login on behalf of the
     * application if this were the case.
     *
     * The application indicates this by explicitly setting the system
     * property javax.security.auth.useSubjectCredsOnly to false.
     */
    public static boolean useSubjectCredsOnly() {
	/*
	 * Don't use GetBooleanAction because the default value in the JRE
	 * (when this is unset) has to treated as true.
	 */
	String propValue = 
	    (String) AccessController.doPrivileged(
               new GetPropertyAction("javax.security.auth.useSubjectCredsOnly",
				     "true"));
	/*
	 * This property has to be explicitly set to "false". Invalid
	 * values should be ignored and the default "true" assumed.
	 */
	return(!propValue.equalsIgnoreCase("false"));
    }
}
