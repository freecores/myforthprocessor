/*
 * @(#)NameServiceDescriptor.java	1.3 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.net.spi.nameservice;

public interface NameServiceDescriptor {
    /**
     * Create a new instance of the corresponding name service.
     */
    public NameService createNameService () throws Exception ;

    /**
     * Returns this service provider's name
     *
     */
    public String getProviderName();

    /**
     * Returns this name service type
     * "dns" "nis" etc
     */
    public String getType();
}
