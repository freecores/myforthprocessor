/*
 * @(#)MailToURLConnection.java	1.15 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.net.www.protocol.mailto;

import java.net.URL;
import java.net.InetAddress;
import java.net.SocketPermission;
import java.io.*;
import java.util.Enumeration;
import java.util.Hashtable;
import java.util.StringTokenizer;
import java.security.Permission;
import sun.net.www.*;
import sun.net.smtp.SmtpClient;
import sun.net.www.ParseUtil;


/** 
 * Handle mailto URLs. To send mail using a mailto URLConnection,
 * call <code>getOutputStream</code>, write the message to the output
 * stream, and close it.
 *
 * @version 1.15, 01/23/03 
 */
public class MailToURLConnection extends URLConnection {
    InputStream is = null;
    OutputStream os = null;

    SmtpClient client;
    Permission permission;

    MailToURLConnection(URL u) {
	super(u);

	MessageHeader props = new MessageHeader();
	props.add("content-type", "text/html");
	setProperties(props);
    }

    /**
     * Get the user's full email address - stolen from 
     * HotJavaApplet.getMailAddress().
     */
    String getFromAddress() {
	String str = System.getProperty("user.fromaddr");
	if (str == null) {
	    str = System.getProperty("user.name");
	    if (str != null) {
		String host = System.getProperty("mail.host");
		if (host == null) {
		    try {
			host = InetAddress.getLocalHost().getHostName();
		    } catch (java.net.UnknownHostException e) {
		    }
		}
		str += "@" + host;
	    } else {
		str = "";
	    }
	}
	return str;
    }    

    public void connect() throws IOException {
	client = new SmtpClient();
    }

    public synchronized OutputStream getOutputStream() throws IOException {
	if (os != null) {
	    return os;
	} else if (is != null) {
	    throw new IOException("Cannot write output after reading input.");
	}
	connect();

	String to = ParseUtil.decode(url.getPath());
	client.from(getFromAddress());
	client.to(to);

	os = client.startMessage();
	return os;
    }

    public Permission getPermission() throws IOException {
	if (permission == null) {
	    connect();
	    String host = client.getMailHost() + ":" + 25;
	    permission = new SocketPermission(host, "connect");
	}
	return permission;
    }	
}
