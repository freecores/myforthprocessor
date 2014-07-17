/*
 * @(#)Calendar.java	1.3 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.tools.javazic;

import sun.util.calendar.Gregorian;

/**
 * Gregorian calendar utility methods. The algorithms are from
 * <i>"Calendrical Calculation"</i> by Nachum Dershowitz and Edward
 * M. Reingold (ISBN: 0-521-56474-3).
 *
 * @since 1.4
 */

class Calendar extends Gregorian {
    /**
     * Returns a fixed date of the n-th day-of-week which is after or
     * before the given fixed date.
     * @param nth specifies the n-th one. A positive number specifies
     * <em>after</em> the <code>fixedDate</code>. A negative number
     * or 0 specifies <em>before</em> the <code>fixedDate</code>.
     * @param dayOfWeek the day of week
     * @param fixedDate the fixed date
     * @return the fixed date of the <code>nth dayOfWeek</code> after
     * or before <code>fixedDate</code>
     */
    static int getNthDayOfWeek(int nth, int dayOfWeek, int fixedDate) {
	if (nth > 0) {
	    return (7 * nth + getDayOfWeekDateBefore(fixedDate, dayOfWeek));
	}
	return (7 * nth + getDayOfWeekDateAfter(fixedDate, dayOfWeek));
    }

    /**
     * Returns a date of the given day of week before the given fixed
     * date.
     * @param fixedDate the fixed date
     * @param dayOfWeek the day of week
     * @return the calculated date
     */
    private static int getDayOfWeekDateBefore(int fixedDate, int dayOfWeek) {
	return (getDayOfWeekDateOnOrBefore(fixedDate - 1, dayOfWeek));
    }

    /*
     * Returns a date of the given day of week that is closest to and
     * after the given fixed date.
     * @param fixedDate the fixed date
     * @param dayOfWeek the day of week
     * @return the calculated date
     */
    private static int getDayOfWeekDateAfter(int fixedDate, int dayOfWeek) {
	return (getDayOfWeekDateOnOrBefore(fixedDate + 7, dayOfWeek));
    }

    /*
     * Returns a date of the given day of week on or before the given fixed
     * date.
     * @param fixedDate the fixed date
     * @param dayOfWeek the day of week
     * @return the calculated date
     */
    private static int getDayOfWeekDateOnOrBefore(int fixedDate, int dayOfWeek) {
	--dayOfWeek;
	return (fixedDate - ((fixedDate - dayOfWeek) % 7));
    }
}
