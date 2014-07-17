/*
 * @(#)NameFilter.java	1.3 03/01/23 11:47:06
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.tools.jcore;

import sun.jvm.hotspot.oops.*;
import java.util.regex.*;

public class NameFilter implements ClassFilter
{
    public Pattern includePattern;

    public NameFilter() {
        this(System.getProperty("sun.jvm.hotspot.tools.jcore.NameFilter.pattern"));
    }

    public NameFilter(String pattern) {
        if (pattern == null) pattern = "*";
        includePattern = Pattern.compile(pattern);
    }

    public boolean canInclude(InstanceKlass kls) {
        String klassName = kls.getName().asString().replace('/', '.'); 
        Matcher m = includePattern.matcher(klassName);
        return m.matches();  
    }
}
