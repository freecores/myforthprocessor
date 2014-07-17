/*
 * @(#)DisposerRecord.java	1.3 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.java2d;

/**
 * This class is used to hold the resource to be 
 * disposed.
 */
public abstract class DisposerRecord {
    public abstract void dispose();
}
