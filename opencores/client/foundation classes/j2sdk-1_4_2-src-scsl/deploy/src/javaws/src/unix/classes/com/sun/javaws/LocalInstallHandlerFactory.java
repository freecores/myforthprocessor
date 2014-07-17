/*
 * @(#)LocalInstallHandlerFactory.java	1.7 03/01/23
 * 
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.javaws;

import com.sun.javaws.debug.*;
import com.sun.javaws.jnl.*;
import java.security.*;
import java.util.*;

/**
 * LocalInstallHandlerFactory for Solaris. Returns null as Solaris does not
 * yet support an install.
 *
 * @version 1.7 01/23/03
 */
public class LocalInstallHandlerFactory {
    /**
     * Returns null, Solaris does not support an installer yet.
     */
    public static LocalInstallHandler newInstance() {
        return new UnixInstallHandler();
    }
}
