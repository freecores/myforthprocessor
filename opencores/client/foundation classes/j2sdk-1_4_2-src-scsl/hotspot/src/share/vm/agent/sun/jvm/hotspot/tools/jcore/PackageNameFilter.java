/*
 * @(#)PackageNameFilter.java	1.3 03/01/23 11:47:08
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.tools.jcore;

import sun.jvm.hotspot.oops.*;
import java.util.*;

public class PackageNameFilter implements ClassFilter
{
    public Object[] pkgList;

    public PackageNameFilter() {
        // give comma separated list of package names to include
        this(System.getProperty("sun.jvm.hotspot.tools.jcore.PackageNameFilter.pkgList"));
    }

    public PackageNameFilter(String pattern) {
        try {
            StringTokenizer st = new StringTokenizer(pattern, ",");
            List l = new LinkedList();
            while (st.hasMoreTokens()) {
                l.add(st.nextToken());
            }
            pkgList = l.toArray();
        } catch (Exception exp) {
           exp.printStackTrace();
        }
    }

    public boolean canInclude(InstanceKlass kls) {
        String klassName = kls.getName().asString().replace('/', '.'); 
       	final int len = pkgList.length;
        for (int i=0; i < len; i++) 
            if (klassName.startsWith((String) pkgList[i] )) return true;
        return false;
    }
}
