/*
 * @(#)CalendarDate.java	1.3 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.util.calendar;

/**
 * <code>CalendarDate</code> holds the date fields, such as year and
 * month, to represent a particular date and time.
 * <code>CalendarDate</code> is independent from any calendar
 * systems. Unlike {@link java.util.Calendar Calendar},
 * <code>CalendarDate</code> has methods only to set or get a field and
 * any calendar calculation operations must be performed with a
 * {@link CalendarSystem} implementation.
 *
 * @see CalendarSystem
 * @see Gregorian
 */
public class CalendarDate {

    public static final int UNKNOWN = Integer.MIN_VALUE;

    private int year;
    private int month;
    private int mday;
    private int dow = UNKNOWN;
    private int millis;		// milliseconds within the day

    public CalendarDate() {
    }

    public CalendarDate(int year, int month, int date) {
	this.year = year;
	this.month = month;
	this.mday = date;
    }

    public void setYear(int year) {
	this.year = year;
    }

    public int getYear() {
	return year;
    }

    public void setMonth(int month) {
	this.month = month;
    }

    public int getMonth() {
	return month;
    }

    public void setDate(int date) {
	mday = date;
    }

    public int getDate() {
	return mday;
    }

    public void setDayOfWeek(int dayOfWeek) {
	dow = dayOfWeek;
    }

    public int getDayOfWeek() {
	return dow;
    }

    public void setTimeOfDay(int time) {
	millis = time;
    }

    public int getTimeOfDay() {
	return millis;
    }

    public boolean equals(Object obj) {
	if (obj == null || !(obj instanceof CalendarDate)) {
	    return false;
	}
	CalendarDate that = (CalendarDate) obj;
	return (year == that.year
		&& month == that.month
		&& mday == that.mday
		&& millis == that.millis);
    }

    public int hashCode() {
	return year << 20 | month << 16 | mday << 11 | (millis >> 10) & 0x3ff;
    }

    public String toString() {
	int h, m, ms;
	h = millis / CalendarSystem.ONE_HOUR;
	ms = millis % CalendarSystem.ONE_HOUR;
	m = ms / CalendarSystem.ONE_MINUTE;
	ms %= CalendarSystem.ONE_MINUTE;

	String ss, mss;
	if (ms == 0) {
	    ss = "";
	    mss = "";
	} else {
	    ss = ":" + sprintf02d(ms / CalendarSystem.ONE_SECOND);
	    ms %= CalendarSystem.ONE_SECOND;
	    if (ms == 0) {
		mss = "";
	    } else {
		mss = ".";
		if (ms < 100) {
		    mss += "0";
		}
		mss += sprintf02d(ms);
	    }
	}

	return year + "/" + sprintf02d(month+1) + "/" + sprintf02d(mday)
	    + " "
	    + sprintf02d(h) + ":" + sprintf02d(m) + ss + mss;
    }

    private static final String sprintf02d(int d) {
	StringBuffer sb = new StringBuffer();
	if (d < 10) {
	    sb.append('0');
	}
	sb.append(Integer.toString(d));
	return sb.toString();
    }
}
