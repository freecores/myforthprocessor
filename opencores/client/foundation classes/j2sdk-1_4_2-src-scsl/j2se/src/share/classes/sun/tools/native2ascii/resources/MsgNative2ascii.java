/*
 * @(#)MsgNative2ascii.java	1.7 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.tools.native2ascii.resources;

import java.util.ListResourceBundle;

public class MsgNative2ascii extends ListResourceBundle {

    public Object[][] getContents() {
        return contents;
    }

    static final Object[][] contents = {
        {"err.bad.arg", "-encoding requires argument"},
        {"err.cannot.read",  "{0} could not be read."},
        {"err.cannot.write", "{0} could not be written."},
        {"usage", "Usage: native2ascii" +
         " [-reverse] [-encoding encoding] [inputfile [outputfile]]"},
    };
}
