/*
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.services;

import java.net.PasswordAuthentication;

public interface BrowserAuthenticator {
	public PasswordAuthentication getAuthentication(String protocol, String host, int port, String scheme, String realm);
}
