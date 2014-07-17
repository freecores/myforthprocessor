/*
 * @(#)PeerEvent.java	1.6 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt;

import java.awt.event.InvocationEvent;

public class PeerEvent extends InvocationEvent {
    public static final long PRIORITY_EVENT = 0x01;

    private long flags;

    public PeerEvent(Object source, Runnable runnable, long flags) {
        this(source, runnable, null, false, flags);
    }

    public PeerEvent(Object source, Runnable runnable, Object notifier,
		      boolean catchExceptions, long flags) {
        super(source, runnable, notifier, catchExceptions);
	this.flags = flags;
    }

    public long getFlags() {
        return flags;
    }

    public PeerEvent coalesceEvents(PeerEvent newEvent) {
	return null;
    }
}
