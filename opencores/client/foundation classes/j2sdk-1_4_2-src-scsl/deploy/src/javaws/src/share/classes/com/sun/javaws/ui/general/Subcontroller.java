/*
 * @(#)Subcontroller.java	1.6 03/01/23
 * 
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.javaws.ui.general;

/**
 * Subcontrollers are typicaly housed inside another widget. The
 * controlling widget will delegate to the Subcontrollers for at least
 * part of its functionality.
 * <p>
 * The semantics of when <code>apply</code>, and <code>revert</code>
 * are sent are left up to the hosting widget. For example, some
 * widgets may never invoke <code>apply</code> or <code>revert</code>
 * as it is implied by invoking <code>start</code> or
 * <code>stop</code>.
 * <p>
 * The normal sequence of events is:
 * <ol>
 *  <li><code>start</code> is invoked to initialize things
 *  <li><code>getComponent</code> is invoked to get the component,
 *    at which point it is added to the hosting widget, such as a
 *    JTabbedPane or JPanel.
 * </ol>
 * <p>
 * <code>stop</code> will be invoked when the Subcontroller is no longer
 * active, for example it is no longer the active tab in JTabbedPane. When
 * the user OKs the changes <code>apply</code> will be invoked.
 * <p>
 * The Controller may not necessarily be active at the time
 * <code>apply</code> or <code>revert</code> is invoked.
 */
public interface Subcontroller {
    /**
     * Should perform any initialization.
     */
    public void start();

    /**
     * Called when the component is no longer being used, should
     * do any necessary cleanup.
     */
    public void stop();

    /**
     * The component that will be displayed
     */
    public java.awt.Component getComponent();

    /**
     * Applys any pending changes.
     */
    public void apply();

    /**
     * Reverts the current set of changes.
     */
    public void revert();
}

