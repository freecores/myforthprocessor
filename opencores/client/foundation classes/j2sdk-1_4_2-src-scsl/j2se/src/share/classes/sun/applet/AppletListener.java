/*
 * @(#)AppletListener.java	1.8 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.applet;

import java.util.EventListener;

/**
 * Applet Listener interface.  This interface is to be implemented
 * by objects interested in AppletEvents.
 * 
 * @version 1.8, 01/23/03
 * @author  Sunita Mani
 */

public interface AppletListener extends EventListener {
    public void appletStateChanged(AppletEvent e);
}
 

