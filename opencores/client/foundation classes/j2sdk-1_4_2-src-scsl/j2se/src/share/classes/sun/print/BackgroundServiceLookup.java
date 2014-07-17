/*
 * @(#)BackgroundServiceLookup.java	1.3 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.print;

/**
 * Lookup services may implement this interface so that clients may 
 * avoid blocking waiting for all services to be located.
 */
public interface BackgroundServiceLookup {

   /**
    *
    */
    public void getServicesInbackground(BackgroundLookupListener listener);

}
