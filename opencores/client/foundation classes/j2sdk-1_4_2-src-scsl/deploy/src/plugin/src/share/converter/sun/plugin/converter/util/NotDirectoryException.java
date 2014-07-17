/*
 * @(#)NotDirectoryException.java	1.5 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.converter.util;

import sun.plugin.converter.*;

public class NotDirectoryException extends java.io.IOException {

    public NotDirectoryException() { super(); }
    public NotDirectoryException(String s) { super(ResourceHandler.getMessage("caption.absdirnotfound") + " " + s); }
}
