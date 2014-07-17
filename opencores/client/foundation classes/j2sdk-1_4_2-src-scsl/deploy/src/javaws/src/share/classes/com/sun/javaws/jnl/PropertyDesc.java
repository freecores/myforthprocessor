/*
 * @(#)PropertyDesc.java	1.7 03/01/23
 * 
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.javaws.jnl;
import com.sun.javaws.xml.*;
/**
 * A object to store information about property elements
 */
public class PropertyDesc implements ResourceType {
    private String _key;
    private String _value;
    
    public PropertyDesc(String key, String value) {
	_key = key;
	_value = value;
    }
    
    // Accessors
    String getKey() { return _key; }
    String getValue() { return _value; }
    
    /** Visitor dispatch */
    public void visit(ResourceVisitor rv) {
	rv.visitPropertyDesc(this);
    }
    
    /** Converts to XML */
    public XMLNode asXML() {
	return new XMLNode("property",
			   new XMLAttribute("name", getKey(),
			   new XMLAttribute("value", getValue())), null, null);
    }
}

