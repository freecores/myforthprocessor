/*
 * @(#)SignalHandler.java	1.7 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.misc;

/**
 * This is the signal handler interface expected in <code>Signal.handle</code>.
 *
 * @author   Sheng Liang
 * @author   Bill Shannon
 * @version  @(#)SignalHandler.java	1.7 03/01/23
 * @see      sun.misc.Signal
 * @since    JDK1.2
 */

public interface SignalHandler {

    /**
     * The default signal handler
     */
    public static final SignalHandler SIG_DFL = new NativeSignalHandler(0);
    /**
     * Ignore the signal
     */
    public static final SignalHandler SIG_IGN = new NativeSignalHandler(1);

    /**
     * Handle the given signal
     *
     * @param sig a signal object
     */
    public void handle(Signal sig);
}
