/*
 * @(#)ThreadDeathRequest.java	1.17 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.jdi.request;

import com.sun.jdi.*;

/**
 * Request for notification when a thread terminates in the target VM.
 * When an enabled ThreadDeathRequest is satisfied, an
 * {@link com.sun.jdi.event.EventSet event set} containing a
 * {@link com.sun.jdi.event.ThreadDeathEvent ThreadDeathEvent}
 * will be placed on the
 * {@link com.sun.jdi.event.EventQueue EventQueue}.
 * The collection of existing ThreadDeathRequests is 
 * managed by the {@link EventRequestManager}
 *
 * @see com.sun.jdi.event.ThreadDeathEvent
 * @see com.sun.jdi.event.EventQueue
 * @see EventRequestManager
 *
 * @author Robert Field
 * @since  1.3
 */
public interface ThreadDeathRequest extends EventRequest {

    /**
     * Restricts the events generated by this request to those in
     * the given thread.
     * @param thread the thread to filter on.
     * @throws InvalidRequestStateException if this request is currently
     * enabled or has been deleted. 
     * Filters may be added only to disabled requests.
     */
    void addThreadFilter(ThreadReference thread);
}


			