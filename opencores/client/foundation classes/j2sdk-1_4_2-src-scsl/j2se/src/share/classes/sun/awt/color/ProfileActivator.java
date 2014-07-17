/*
 * @(#)ProfileActivator.java	1.5 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt.color;


/**
 * An interface to allow the ProfileDeferralMgr to activate a
 * deferred profile.
 */
public interface ProfileActivator {

    /**
     * Activate a previously deferred ICC_Profile object.
     */
    public void activate();

}
