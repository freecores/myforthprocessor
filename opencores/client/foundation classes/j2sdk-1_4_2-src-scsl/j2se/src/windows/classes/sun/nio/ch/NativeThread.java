/*
 * @(#)NativeThread.java	1.2 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.nio.ch;


// Signalling operations on native threads


class NativeThread {

    static long current() { return 0; }

    static void signal(long nt) { }

}
