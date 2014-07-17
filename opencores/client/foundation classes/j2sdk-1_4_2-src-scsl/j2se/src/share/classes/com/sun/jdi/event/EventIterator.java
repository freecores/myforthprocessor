/*
 * @(#)EventIterator.java	1.8 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.jdi.event;

import com.sun.jdi.*;

import java.util.Iterator;

/**
 * EventIterators are unmodifiable.
 *
 * @see Event
 * @see EventSet
 * @see EventSet#iterator
 *
 * @author Robert Field
 * @since  1.3
 */

public interface EventIterator extends Iterator {

    /**
     * @return The next {@link Event} in an {@link EventSet}.
     */
    Event nextEvent();
}
