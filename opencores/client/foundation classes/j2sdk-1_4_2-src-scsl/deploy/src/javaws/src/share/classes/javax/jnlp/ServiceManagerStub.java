/*
 * @(#)ServiceManagerStub.java	1.8 03/01/23
 * 
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package javax.jnlp;

/**
 * A <code>ServiceManagerStub</code> object implements the particular
 * lookup of JNLP services by the JNLP Client. The object
 * is registered with the <code>ServiceManager</code> using the
 * <code>{@link ServiceManager#setServiceManagerStub}</code> method.
 *
 * @version 1.0
 *
 * @see     javax.jnlp.ServiceManager
 */
public interface ServiceManagerStub {
   
    /** See description for {@link ServiceManager#lookup} */
    public Object lookup(String name) throws UnavailableServiceException;
    
    /** See description for {@link ServiceManager#getServiceNames} */
    public String[] getServiceNames();
}
