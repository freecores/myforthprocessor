/*
 * @(#)PropertyGetDispatcher.java	1.5 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.com;

import java.lang.reflect.Field;
import sun.plugin.util.Trace;

/**
 * A <code>PropertyGetDispatcher</code> provides information about,
 * and access to, a property on a class or interface.  The
 * reflected property may be a class or an instance property
 * (including an abstract method).
 *
 * @see Member
 * @see java.lang.Class
 * @see java.lang.Field#get()
 * @see java.lang.Field#getType()
 */
public class PropertyGetDispatcher implements Dispatcher
{
    private Field field = null;

    /**
     * Constructor
     */
    public PropertyGetDispatcher(Field inField) {
	field = inField;
    }

    public Object invoke(Object obj, Object []params)throws Exception {
	Object propVal = null;
	if(field != null) {
	    Trace.msgLiveConnectPrintln("com.field.get", new Object[] {field});
	    propVal = field.get(obj);
	}	    

	return Utils.convertReturn(field.getType(), propVal);
    }
}
