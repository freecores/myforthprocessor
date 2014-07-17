/*
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.services;

import java.net.PasswordAuthentication;

final class WIExplorerBrowserAuthenticator implements BrowserAuthenticator {
	public PasswordAuthentication getAuthentication(String protocol, String siteName, int port, String scheme, String realm) {
		char[] credential = null;

		// IE 5.5 appends port number to siteName
		if(port != -1) {
		    credential = getAuthentication(siteName + ":" + port + "/" + realm);
		}


		if(credential == null) {
		    credential = getAuthentication(siteName + "/" + realm);
		}


		if(null == credential)
		    return null;

		int index = 0;
		while(index < credential.length && ':' != credential[index])
		    index ++;
		
		PasswordAuthentication pa = null;
		if(index < credential.length) {
			String userName = new String(credential, 0, index);
			char[] password = extractArray(credential, index + 1);
			pa = new PasswordAuthentication(userName, password);
			resetArray(password);
		}
		resetArray(credential);

		return pa;
	}

	private native char[] getAuthentication(String key);

	private void resetArray(char[] arr) {
		java.util.Arrays.fill(arr, ' ');
	}


	private char[] extractArray(char[] src, int start) {
		char[] dest = new char[src.length - start];
		for(int index = 0; index < dest.length; index ++)
			dest[index] = src[index + start];

		return dest;
	}
}
