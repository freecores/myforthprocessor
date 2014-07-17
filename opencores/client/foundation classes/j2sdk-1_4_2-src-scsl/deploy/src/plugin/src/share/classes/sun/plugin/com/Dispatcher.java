/*
 * @(#)Dispatcher.java	1.2 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.com;

import java.lang.reflect.*;

public interface Dispatcher
{
    /**
     * Invoke a method according to the method index.
     *
     * @param obj wrapped object
     * @param args Arguments.
     * @return Java object.
     */
    public Object invoke(Object obj, Object []parameters)
        throws  Exception;
}
