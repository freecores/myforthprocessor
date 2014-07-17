/*
 * @(#)ProgressEntry.java	1.17 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package sun.net;

import java.net.URL;

public class ProgressEntry {
    public static final int HTML  = 0;
    public static final int IMAGE = 1;
    public static final int CLASS = 2;
    public static final int AUDIO = 3;
    public static final int OTHER = 4;

    /* bytes needed */
    public int need = 0;
    /* bytes read */
    public int read = 0;
    /* the last thing to happen to this entry */
    public int what;
    /* where this entry falls into the array 
     * of all entries we're monitoring
     */
    public int index;
    public boolean connected = false;

    // Label for this entry.
    public String label;

    public int type;

    public ProgressEntry(String l, String ctype) {
	label = l;
	type = OTHER;
	need = 0;
	setType(l, ctype);
    }


    /*
     * Usually called when a URL is finally connected after the
     * content type is known.
     */
    public void setType(String l, String ctype) {
	if (ctype != null) {
	    if (ctype.startsWith("image")) {
		type = IMAGE;
	    } else if (ctype.startsWith("audio")) {
		type = AUDIO;
	    } else if (ctype.equals("application/java-vm")) {
		type = CLASS;
	    } else if (ctype.startsWith("text/html")) {
		type = HTML;
	    }
	}
	if (type == OTHER) {
	    if (l.endsWith(".html") || l.endsWith("/") || l.endsWith(".htm")) {
		type = HTML;		
	    } else if (l.endsWith(".class")) {
		type = CLASS;
	    } else if (l.endsWith(".gif") || l.endsWith(".xbm")
		|| l.endsWith(".jpeg") || l.endsWith(".jpg")
		|| l.endsWith(".jfif")) {
		type = IMAGE;
	    } else if (l.endsWith(".au")) {
		type = AUDIO;
	    }
	}
    }

    public void update(int total_read, int total_need) {
	if (need == 0) {
	    need = total_need;
	}
	read = total_read;
    }

/*
    this returns the previous value of the connected boolean.
    typical usage as found in Progressdata is something like
		if (te.connected() == false) {
		    lastchanged = i;
		    setChanged();
		    notifyObservers();
		}

*/
    public synchronized boolean connected() {
	if (!connected) {
	    connected = true;    
	    return false;
	} 
	return true;
    }
}



