/*
 * @(#)AWTLockAccess.java	1.6 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt.motif;

final class AWTLockAccess {
    static native void awtLock();
    static native void awtUnlock();
    static void awtWait() { awtWait(0); }
    static native void awtWait(long timeout);
    static native void awtNotifyAll();
}
