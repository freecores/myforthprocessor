/*
 * @(#)Constants.java	1.9 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.rmi.rmic;

import sun.tools.java.Identifier;

public interface Constants extends sun.tools.java.Constants {

    /*
     * Identifiers potentially useful for all Generators
     */
    public static final Identifier idRemote =
	Identifier.lookup("java.rmi.Remote");
    public static final Identifier idRemoteException =
	Identifier.lookup("java.rmi.RemoteException");
}
