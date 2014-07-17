/*
 * @(#)MInputMethodControl.java	1.9 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt.motif;

import sun.awt.motif.MComponentPeer;
import sun.awt.motif.X11InputMethod;

/**
 * An interface for controlling containment hierarchy configuration to
 * keep track of existence of any TextArea or TextField and to manage
 * input method status area.
 *
 * @version	1.9 01/23/03
 * @auther	JavaSoft International
 */
interface MInputMethodControl {

    /**
     * Informs Frame or Dialog that a text component has been added to
     * the hierarchy.
     * @param	textComponentPeer	peer of the text component
     */
    void addTextComponent(MComponentPeer textComponentPeer);

    /**
     * Informs Frame or Dialog that a text component has been removed
     * from the hierarchy.
     * @param textComponentPeer peer of the text component
     */
    void removeTextComponent(MComponentPeer textComponentPeer);

    /**
     * Returns a text component peer in the containment hierarchy 
     * to obtain the Motif status area information
     */
    MComponentPeer getTextComponent();

    /**
     * Inform Frame or Dialog that an X11InputMethod has been
     * constructed so that Frame and Dialog can invoke the method in
     * X11InputMethod to reconfigure XICs.
     * @param	inputMethod	an X11InputMethod instance
     */
    void addInputMethod(X11InputMethod inputMethod);

    /**
     * Inform Frame or Dialog that an X11InputMethod is being destroyed.
     * @param	inputMethod	an X11InputMethod instance
     */
    void removeInputMethod(X11InputMethod inputMethod);
}
