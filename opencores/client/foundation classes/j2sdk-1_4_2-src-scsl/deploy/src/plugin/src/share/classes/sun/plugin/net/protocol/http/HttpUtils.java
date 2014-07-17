/*
 * @(#)HttpUtils.java	1.9 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.net.protocol.http;

import java.io.InputStream;
import java.io.IOException;
import java.net.InetAddress;
import java.net.UnknownHostException;
import java.net.URL;
import java.net.URLConnection;
import java.net.HttpURLConnection;

// Some simple HTTP utilities used by the jar cache.
public class HttpUtils {

    // Opens a connection, following redirects.  Similar to
    // HttpURLConnection.openConnectionCheckRedirects(), but returns the
    // URLConnection object instead of an InputStream.  This allows the
    // caller to get the response code from the final host.
    public static HttpURLConnection followRedirects(URLConnection c)
        throws IOException {
        boolean redir;
        int redirects = 0;
        InputStream in = null;

        do {
            if (c instanceof HttpURLConnection) {
                ((HttpURLConnection) c).setInstanceFollowRedirects(false);
            }

            // We want to open the input stream before
            // getting headers, because getHeaderField()
            // et al swallow IOExceptions.
            in = c.getInputStream();
            redir = false;

            if (c instanceof HttpURLConnection) {
                HttpURLConnection http = (HttpURLConnection) c;
                int stat = http.getResponseCode();
                if (stat >= 300 && stat <= 305 &&
                    stat != HttpURLConnection.HTTP_NOT_MODIFIED) {
                    URL base = http.getURL();
                    String loc = http.getHeaderField("Location");
                    URL target = null;
                    if (loc != null) {
                        target = new URL(base, loc);
                    }
                    http.disconnect();
                    if (target == null
                        || !base.getProtocol().equals(target.getProtocol())
                        //|| base.getPort() != target.getPort()
                        //|| !hostsEqual(base, target)
                        || redirects >= 5) {
                        throw new SecurityException("illegal URL redirect");
                    }
                    redir = true;
                    c = target.openConnection();
                    redirects++;
                }
            }
        } while (redir);
        if (!(c instanceof HttpURLConnection)) {
            throw new IOException(c.getURL() + " redirected to non-http URL");
        }
        return (HttpURLConnection)c;
    }
}


