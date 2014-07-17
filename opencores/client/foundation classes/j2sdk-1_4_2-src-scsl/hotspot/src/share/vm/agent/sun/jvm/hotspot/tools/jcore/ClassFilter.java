/*
 * @(#)ClassFilter.java	1.3 03/01/23 11:47:00
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.tools.jcore;

import sun.jvm.hotspot.oops.InstanceKlass;

public interface ClassFilter
{
    public boolean canInclude(InstanceKlass kls);
}
