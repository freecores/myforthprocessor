/*
 * @(#)Month.java	1.3 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.tools.javazic;

import java.util.HashMap;

/**
 * Month class handles month related manipulation.
 *
 * @since 1.4
 */
class Month {
    private static HashMap months = new HashMap();
    static {
	months.put("Jan", new Integer(0));
	months.put("Feb", new Integer(1));
	months.put("Mar", new Integer(2));
	months.put("Apr", new Integer(3));
	months.put("May", new Integer(4));
	months.put("Jun", new Integer(5));
	months.put("Jul", new Integer(6));
	months.put("Aug", new Integer(7));
	months.put("Sep", new Integer(8));
	months.put("Oct", new Integer(9));
	months.put("Nov", new Integer(10));
	months.put("Dec", new Integer(11));
    }

    /**
     * Parses the specified string as a month abbreviation.
     * @param name the month abbreviation
     * @return the month value
     */
    static int parse(String name) {
	Integer m = (Integer) months.get(name);
	if (m != null) {
	    return m.intValue();
	}
	throw new IllegalArgumentException("wrong month name: " + name);
    }

    private static String upper_months[] = {
	"JANUARY", "FEBRUARY", "MARCH", "APRIL", "MAY", "JUNE",
	"JULY", "AUGUST", "SEPTEMBER", "OCTOBER", "NOVEMBER", "DECEMBER"
    };

    /**
     * @param month the numth number
     * @return the month name in uppercase of the specified month
     */
    static String toString(int month) {
	if (month >= 0 && month <= 11) {
	    return "Calendar."+ upper_months[month];
	}
	throw new IllegalArgumentException("wrong month number: " + month);
    }
}
