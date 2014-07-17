/*
 * @(#)COMEventListener.java	1.2 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.com.event;

import java.lang.reflect.Method;

public interface COMEventListener {
    public void notify(Object event, Method method) throws Throwable;
}


