/*
 * @(#)WJcovUtil.java	1.3 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin;

/*
 * This class is here to provide a hook to call into the Windows
 * Native code to dump all the jcov data
 *
 * @version 1.1
 * @author
 *
 */

public class WJcovUtil
{
    public static native boolean dumpJcovData();
}
