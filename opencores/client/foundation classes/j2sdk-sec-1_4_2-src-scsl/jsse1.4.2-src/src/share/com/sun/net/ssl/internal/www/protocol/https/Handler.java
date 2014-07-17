/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package com.sun.net.ssl.internal.www.protocol.https;

import java.io.IOException;
import java.net.URL;

/** 
 * This class exists for compatibility with previous JSSE releases
 * only. The HTTPS implementation can now be found in
 * sun.net.www.protocol.https.
 *
 * @version 1.8 06/24/03
 */
public class Handler extends sun.net.www.protocol.https.Handler {

    public Handler() {
        super();
    }

    public Handler(String proxy, int port) {
        super(proxy, port);
    }

    protected java.net.URLConnection openConnection(URL u) throws IOException {
	return new HttpsURLConnectionOldImpl(u, this);
    }
}
