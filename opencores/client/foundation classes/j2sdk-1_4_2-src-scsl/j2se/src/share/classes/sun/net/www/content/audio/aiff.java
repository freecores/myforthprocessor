/*
 * @(#)aiff.java	1.6 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/**
 * Basic .aiff audio handler.
 * @version 1.6, 01/23/03
 * @author  Jeff Nisewanger
 */
package sun.net.www.content.audio;

import java.net.*;
import java.io.IOException;
import sun.applet.AppletAudioClip;

/**
 * Returns an AppletAudioClip object.
 */
public class aiff extends ContentHandler {
    public Object getContent(URLConnection uc) throws IOException {
	return new AppletAudioClip(uc);
    }
}
