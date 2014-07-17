/*
 * %W% %E%
 * 
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.javaws.util;
import java.util.ArrayList;
import java.util.Arrays;

/**
 *   VersionID contains a JNLP version ID.
 *
 *  The VersionID also contains a prefix indicator that can
 *  be used when stored with a VersionString
 *
 */
public class VersionID implements Comparable {
    private String[] _tuple;   // Array of Integer or String objects
    private boolean  _usePrefixMatch;   // star (*) prefix
    private boolean  _useGreatherThan;  // plus (+) greather-than
    
    /** Creates a VersionID object */
    public VersionID(String str) {
	_usePrefixMatch  = false;
	_useGreatherThan = false;
	if (str == null) {
	    _tuple = new String[0];
	    return;
	}
	
	// Check for postfix
	if (str.endsWith("+")) {
	    _useGreatherThan = true;
	    str = str.substring(0, str.length() - 1);
	} else if (str.endsWith("*")) {
	    _usePrefixMatch = true;
	    str = str.substring(0, str.length() - 1);
	}
	
	ArrayList list = new ArrayList();
	int start = 0;
	for(int i = 0; i < str.length(); i++) {
	    // Split at each separator character
	    if (".-_".indexOf(str.charAt(i)) != -1) {
		if (start < i) {
		    String value = str.substring(start, i);
		    list.add(value);
		}
		start = i + 1;
	    }
	}
	if (start < str.length()) list.add(str.substring(start, str.length()));
	
	_tuple = new String[list.size()];
	_tuple = (String[])list.toArray(_tuple);
    }
    
    /** Returns true if the prefix flag is set */
    public boolean usePrefixMatch() { return _usePrefixMatch; }
    
    /** Returns true if the prefix flag is set */
    public boolean useGreaterThanMatch() { return _useGreatherThan; }
    
    /** Match 'this' versionID against vid. The _usePrefixMatch/_useGreatherThan flag
     *  is used to determine if a prefix match of an exact match should be performed
     */
    public boolean match(VersionID vid) {
	return (usePrefixMatch()) ? this.isPrefixMatch(vid) :
	    (useGreaterThanMatch()) ? vid.isGreaterThanOrEqual(this) : equals(vid);
    }
    
    /** Compares if two version IDs are equal */
    public boolean equals(Object o) {
	// Check for null and type
	if (o == null || !(o instanceof VersionID)) return false;
	VersionID vid = (VersionID)o;
	
	// Normalize arrays
	String[] t1 = normalize(_tuple, vid._tuple.length);
	String[] t2 = normalize(vid._tuple, _tuple.length);
	
	// Check contents
	for(int i = 0; i < t1.length; i++) {
	    Object o1 = getValueAsObject(t1[i]);
	    Object o2 = getValueAsObject(t2[i]);
	    if (!o1.equals(o2)) return false;
	}
	return true;
    }
    
    private Object getValueAsObject(String value) {
	if (value.length() > 0 && value.charAt(0) != '-') {
	    try { return Integer.valueOf(value);
	    } catch(NumberFormatException nfe) { /* fall through */ }
	}
	return value;
    }
    
    public boolean isGreaterThan(VersionID vid) {
	return isGreaterThanOrEqualHelper(vid, false);
    }
    
    public boolean isGreaterThanOrEqual(VersionID vid) {
	return isGreaterThanOrEqualHelper(vid, true);
    }
    
    /** Compares if 'this' is greater than vid */
    public boolean isGreaterThanOrEqualHelper(VersionID vid, boolean allowEqual) {
	// Normalize the two strings
	String[] t1 = normalize(_tuple, vid._tuple.length);
	String[] t2 = normalize(vid._tuple, _tuple.length);
	
	for(int i = 0; i < t1.length; i++) {
	    // Compare current element
	    Object e1 = getValueAsObject(t1[i]);
	    Object e2 = getValueAsObject(t2[i]);
	    if (e1.equals(e2)) {
		// So far so good
	    } else {
		if (e1 instanceof Integer && e2 instanceof Integer) {
		    return ((Integer)e1).intValue() > ((Integer)e2).intValue();
		} else {
		    String s1 = t1[i].toString();
		    String s2 = t2[i].toString();
		    return s1.compareTo(s2) > 0;
		}
		
	    }
	}
	// If we get here, they are equal
	return allowEqual;
    }
    
    /** Checks if 'this' is a prefix of vid */
    public boolean isPrefixMatch(VersionID vid) {
	// Make sure that vid is at least as long as the prefix
	String[] t2 = normalize(vid._tuple, _tuple.length);
	
	for(int i = 0; i < _tuple.length; i++) {
	    Object e1 = _tuple[i];
	    Object e2 = t2[i];
	    if (e1.equals(e2)) {
		// So far so good
	    } else {
		// Not a prefix
		return false;
	    }
	}
	return true;
    }
    
    /** Normalize an array to a certain lengh */
    private String[] normalize(String[] list, int minlength) {
	if (list.length < minlength) {
	    // Need to do padding
	    String[] newlist = new String[minlength];
	    System.arraycopy(list, 0, newlist, 0, list.length);
	    Arrays.fill(newlist, list.length, newlist.length, "0");
	    return newlist;
	} else {
	    return list;
	}
    }
    
    public int compareTo(Object o) {
	if (o == null || !(o instanceof VersionID)) return -1;
	VersionID vid = (VersionID)o;
	return equals(vid) ? 0 : (isGreaterThanOrEqual(vid) ? 1 : -1);
    }
    /** Show it as a string */
    public String toString() {
	StringBuffer sb = new StringBuffer();
	for(int i = 0; i < _tuple.length -1; i++) {
	    sb.append(_tuple[i]);
	    sb.append('.');
	}
	if (_tuple.length > 0 ) sb.append(_tuple[_tuple.length - 1]);
	if (_usePrefixMatch) sb.append('+');
	return sb.toString();
    }
}
