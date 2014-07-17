/*
 * @(#)PluginAuthenticator.java	1.21 02/04/19
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.security;

import java.net.Authenticator;
import java.net.PasswordAuthentication;
import java.util.HashMap;
import javax.swing.BoxLayout;
import javax.swing.JOptionPane;
import javax.swing.JTextField;
import javax.swing.JPasswordField;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.Box;
import javax.swing.LookAndFeel;
import sun.plugin.resources.ResourceHandler;
import sun.plugin.util.Trace;
import sun.plugin.util.DialogFactory;
import sun.plugin.util.PluginUIManager;
import sun.plugin.services.BrowserAuthenticator;
import sun.plugin.services.BrowserService;
import sun.plugin.services.ServiceProvider;
import sun.plugin.util.PluginSysUtil;
import sun.plugin.util.PluginSysAction;

/** 
 * Assist proxy authentication and web server authentication.
 * A unique instance of this class is registered at startup by the plugin
 * @see AppletViewer#initEnvironment
 * 
 * @see java.net.Authenticator
 * @see java.net.PasswordAuthentication
 */
public final class PluginAuthenticator extends Authenticator {

    /**
     * <p>
     * HTTP/HTTPS connection to a specified site, only needs to be authenticated once.
     * To avoid that if BrowserAuthenticator actually returns a wrong username/passowrd,
     * which will cause fall back no happening, and Java side dialog box never show up,
     * we will have to track the site that has already been authenticated with 
     * BrowserAuthenticator's username/password.
     * </p>
     */
     private static HashMap baSites = new HashMap();

    /**
     * <p>
     * Called when password authorization is needed by the superclass. The
     * instance is registered as being the default http proxy authentication
     * facility and will be called by the HTTP framework classes.
     * </p>
     * @return The PasswordAuthentication collected from the
     *		user, or null if none is provided.
     */
    protected synchronized PasswordAuthentication getPasswordAuthentication()
    {
		PasswordAuthentication pa = null; 	
		
		try
		{
			java.net.InetAddress site = getRequestingSite();
			String siteName = getMessage("net.authenticate.unknownSite");
			if (site != null)
				siteName = site.toString();

			StringBuffer key = new StringBuffer(getRequestingProtocol());
			key.append(':');
			key.append(getRequestingHost());
			key.append(':');
			key.append(getRequestingPort());
			key.append(':');
			key.append(getRequestingScheme());
			key.append(':');
			key.append(getRequestingPrompt());
			// If we already try browser authenticator, but still can not get us through, don't try again
			if(!baSites.containsKey(key.toString())) {
			    BrowserAuthenticator browserAuthenticator = ServiceProvider.getService().getBrowserAuthenticator();

			    if(browserAuthenticator != null) {
				pa = browserAuthenticator.getAuthentication(getRequestingProtocol(), getRequestingHost(), getRequestingPort(), getRequestingScheme(), getRequestingPrompt());
				    if(pa != null) {
					baSites.put(key.toString(), key.toString());
					return pa;
				}
			    }
			}

			// Print out tracing 
			StringBuffer buffer = new StringBuffer();
			buffer.append("Firewall authentication: site=");
			buffer.append(getRequestingSite());
			buffer.append(":" + getRequestingPort());
			buffer.append(", protocol=");
			buffer.append(getRequestingProtocol());
			buffer.append(", prompt=");
			buffer.append(getRequestingPrompt());
			buffer.append(", scheme=");
			buffer.append(getRequestingScheme());

			Trace.netPrintln(buffer.toString());
						
			// Request the username/password from the user
			pa = openDialog(siteName, getRequestingPrompt(), getRequestingScheme());
		}
		catch (Exception e)
		{
			// We should catch all exception so the connection may continue
			Trace.netPrintException(e);
		}

		return pa; 
    }


    /*
     * <p>
     * Open the dialog to request the user/password information 
     * from the user. The dialog box is based on Internet Explorer 4
     * dialog minus the icon
     * </p>
     *
     * @param site the HTTP site we are trying to connect
     * @param prompt the HTTP prompt from the server
     * @param scheme the HTTP scheme
     * @return the PasswordAuthentication object encapsulating the 
     * username/password entered by the user
     */    
    private PasswordAuthentication openDialog(String site, String prompt, String scheme) {
	final String theSite = site;
	final String thePrompt = prompt;
	final String theScheme = scheme;
	try {
	    return (PasswordAuthentication)PluginSysUtil.execute(new PluginSysAction() {
		public Object execute() throws Exception {
		    return openDialogImpl(theSite, thePrompt, theScheme);
		}});
	}
	catch(Exception e) {
	    // should never happen
	    assert(false);
	    return null;
	}
    }

    private PasswordAuthentication openDialogImpl(String site, String prompt, String scheme) 
    {
	LookAndFeel lookAndFeel = null;

	try
	{
	    // Change metal theme
	    lookAndFeel = PluginUIManager.setTheme();

	    if (site == null)
		site = "";

	    if (prompt == null)
		prompt = "";

	    if (scheme == null)
		scheme = "";

	    
	    JTextField userName = new JTextField();
	    userName.setColumns(15);
	    JPasswordField password = new JPasswordField();
	    password.setColumns(15);

	    JPanel optionPane = new JPanel();
	    optionPane.setLayout(new BoxLayout(optionPane, BoxLayout.Y_AXIS));
	    

	    optionPane.add(new JLabel(getMessage("net.authenticate.firewall")));
	    optionPane.add(Box.createVerticalStrut(5));
	    optionPane.add(new JLabel(getMessage("net.authenticate.realm")));
	    optionPane.add(Box.createVerticalStrut(5));
	    optionPane.add(new JLabel(getMessage("net.authenticate.scheme")));
	    optionPane.add(Box.createVerticalStrut(15));
	    optionPane.add(new JLabel(getMessage("net.authenticate.username")));
	    optionPane.add(Box.createVerticalStrut(15));
	    optionPane.add(new JLabel(getMessage("net.authenticate.password")));
	    optionPane.add(Box.createVerticalStrut(15));
	    JPanel optionPane2 = new JPanel();
	    optionPane2.setLayout(new BoxLayout(optionPane2, BoxLayout.Y_AXIS));
	    optionPane2.add(new JLabel(site));
	    optionPane2.add(Box.createVerticalStrut(5));
	    optionPane2.add(new JLabel(prompt));
	    optionPane2.add(Box.createVerticalStrut(5));
	    optionPane2.add(new JLabel(scheme));
	    optionPane2.add(Box.createVerticalStrut(10));
	    optionPane2.add(userName);
	    optionPane2.add(Box.createVerticalStrut(10));
	    optionPane2.add(password);
	    JPanel finalPane = new JPanel();
	    finalPane.setLayout(new BoxLayout(finalPane, BoxLayout.X_AXIS));
	    finalPane.add(optionPane);
	    finalPane.add(Box.createHorizontalStrut(15));
	    finalPane.add(optionPane2);
	    
	    Object[] objects = new Object[] { finalPane };

	    int result = DialogFactory.showConfirmDialog(objects, getMessage("net.authenticate.caption"));	    
	    
	    if (result == JOptionPane.OK_OPTION) 
	    {
		PasswordAuthentication pa;

		// for security purpose, DO NOT put password into String. Keep it in char array and 
		// reset after used.
		char[] userPassword = password.getPassword();
		pa = new PasswordAuthentication(userName.getText(), 
						userPassword);
		// for security reason, reset password
		java.util.Arrays.fill(userPassword, ' ');

		return pa;
		
	    } else {
		return null;
	    }
	}
	finally
	{
	    // Restore theme
	    PluginUIManager.restoreTheme(lookAndFeel);
	}
    }

    /*
     * <p>
     * Helper method to load resources for i18n
     * </p>
     */
    private String getMessage(String key) {
	return ResourceHandler.getMessage(key);
    }
	
}



