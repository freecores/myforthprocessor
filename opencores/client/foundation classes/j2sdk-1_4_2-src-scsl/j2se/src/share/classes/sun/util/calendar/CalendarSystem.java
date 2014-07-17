/*
 * @(#)CalendarSystem.java	1.3 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.util.calendar;

import java.util.Calendar;

public interface CalendarSystem {
    public static final int ONE_SECOND = 1 * 1000;
    public static final int ONE_MINUTE = 60 * ONE_SECOND;
    public static final int ONE_HOUR = 60 * ONE_MINUTE;
    public static final int ONE_DAY = 24 * ONE_HOUR;

    // month constants
    public static final int JANUARY = Calendar.JANUARY;
    public static final int FEBRUARY = Calendar.FEBRUARY;
    public static final int MARCH = Calendar.MARCH;
    public static final int APRIL = Calendar.APRIL;
    public static final int MAY = Calendar.MAY;
    public static final int JUNE = Calendar.JUNE;
    public static final int JULY = Calendar.JULY;
    public static final int AUGUST = Calendar.AUGUST;
    public static final int SEPTEMBER = Calendar.SEPTEMBER;
    public static final int OCTOBER = Calendar.OCTOBER;
    public static final int NOVEMBER = Calendar.NOVEMBER;
    public static final int DECEMBER = Calendar.DECEMBER;
    public static final int UNDECIMBER = Calendar.UNDECIMBER;

    // day of week constants
    public static final int SUNDAY = Calendar.SUNDAY;
    public static final int MONDAY = Calendar.MONDAY;
    public static final int TUESDAY = Calendar.TUESDAY;
    public static final int WEDNESDAY = Calendar.WEDNESDAY;
    public static final int THURSDAY = Calendar.THURSDAY;
    public static final int FRIDAY = Calendar.FRIDAY;
    public static final int SATURDAY = Calendar.SATURDAY;
}
