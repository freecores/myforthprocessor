/*
 * @(#)CatchData.java	1.16 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.tools.asm;

import sun.tools.java.*;
import java.util.Hashtable;

public final
class CatchData {
     Object type;
     Label label;

     /**
      * Constructor
      */
     CatchData(Object type) {
	 this.type = type;
	 this.label = new Label();
     }

     /**
      * Get the label
      */
     public Label getLabel() {
	 return label;
     }

     /**
      * Get the clazz
      */
     public Object getType() {
	 return type;
     }
}
