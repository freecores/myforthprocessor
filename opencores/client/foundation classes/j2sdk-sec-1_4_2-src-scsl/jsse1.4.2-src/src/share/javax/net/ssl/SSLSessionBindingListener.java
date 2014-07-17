/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */


package javax.net.ssl;

import java.util.EventListener;

/**
 * This interface is implemented by objects which want to know when
 * they are being bound or unbound from a SSLSession.  When either event
 * occurs via {@link SSLSession#putValue(String, Object)}
 * or {@link SSLSession#removeValue(String)}, the event is communicated
 * through a SSLSessionBindingEvent identifying the session.
 *
 * @see SSLSession
 * @see SSLSessionBindingEvent
 *
 * @since 1.4
 * @author Nathan Abramson
 * @author David Brownell
 * @version 1.11
 */
public
interface SSLSessionBindingListener
extends EventListener
{
    /**
     * This is called to notify the listener that it is being bound into
     * an SSLSession.
     *
     * @param event the event identifying the SSLSession into
     *		which the listener is being bound.
     */
    public void valueBound(SSLSessionBindingEvent event);

    /**
     * This is called to notify the listener that it is being unbound
     * from a SSLSession.
     *
     * @param event the event identifying the SSLSession from
     *		which the listener is being unbound.
     */
    public void valueUnbound(SSLSessionBindingEvent event);
}

