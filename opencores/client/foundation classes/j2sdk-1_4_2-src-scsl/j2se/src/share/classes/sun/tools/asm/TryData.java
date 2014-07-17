/*
 * @(#)TryData.java	1.17 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.tools.asm;

import sun.tools.java.*;
import java.util.Vector;

public final
class TryData {
    Vector catches = new Vector();
    Label endLabel = new Label();
    
    /**
     * Add a label
     */
    public CatchData add(Object type) {
	CatchData cd = new CatchData(type);
	catches.addElement(cd);
	return cd;
    }

    /**
     * Get a label
     */
    public CatchData getCatch(int n) {
	return (CatchData)catches.elementAt(n);
    }

    /**
     * Get the default label
     */
    public Label getEndLabel() {
	return endLabel;
    }
}
