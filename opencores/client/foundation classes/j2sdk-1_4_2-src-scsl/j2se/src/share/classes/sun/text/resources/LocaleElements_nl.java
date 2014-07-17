/*
 * @(#)LocaleElements_nl.java	1.15 03/01/23
 */

/*
 * Portions Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * (C) Copyright Taligent, Inc. 1996, 1997 - All Rights Reserved
 * (C) Copyright IBM Corp. 1996 - 1998 - All Rights Reserved
 *
 * The original version of this source code and documentation
 * is copyrighted and owned by Taligent, Inc., a wholly-owned
 * subsidiary of IBM. These materials are provided under terms
 * of a License Agreement between Taligent and Sun. This technology
 * is protected by multiple US and International patents.
 *
 * This notice and attribution to Taligent may not be removed.
 * Taligent is a registered trademark of Taligent, Inc.
 *
 */

package sun.text.resources;

import java.util.ListResourceBundle;

public class LocaleElements_nl extends ListResourceBundle {
    /**
     * Overrides ListResourceBundle
     */
    public Object[][] getContents() {
        return new Object[][] {
            { "Languages", // language names
                new String[][] {
                    { "nl", "Nederlands" }
                }
            },
            { "Countries", // country names
                new String[][] {
                    { "NL", "Nederland" },
                    { "BE", "Belgi\u00eb" }
                }
            },
            { "MonthNames",
                new String[] {
                    "januari", // january
                    "februari", // february
                    "maart", // march
                    "april", // april
                    "mei", // may
                    "juni", // june
                    "juli", // july
                    "augustus", // august
                    "september", // september
                    "oktober", // october
                    "november", // november
                    "december", // december
                    "" // month 13 if applicable
                }
            },
            { "MonthAbbreviations",
                new String[] {
                    "jan", // abb january
                    "feb", // abb february
                    "mrt", // abb march
                    "apr", // abb april
                    "mei", // abb may
                    "jun", // abb june
                    "jul", // abb july
                    "aug", // abb august
                    "sep", // abb september
                    "okt", // abb october
                    "nov", // abb november
                    "dec", // abb december
                    "" // abb month 13 if applicable
                }
            },
            { "DayNames",
                new String[] {
                    "zondag", // Sunday
                    "maandag", // Monday
                    "dinsdag", // Tuesday
                    "woensdag", // Wednesday
                    "donderdag", // Thursday
                    "vrijdag", // Friday
                    "zaterdag" // Saturday
                }
            },
            { "DayAbbreviations",
                new String[] {
                    "zo", // abb Sunday
                    "ma", // abb Monday
                    "di", // abb Tuesday
                    "wo", // abb Wednesday
                    "do", // abb Thursday
                    "vr", // abb Friday
                    "za" // abb Saturday
                }
            },
            { "NumberElements",
                new String[] {
                    ",", // decimal separator
                    ".", // group (thousandsnds) separator
                    ";", // list separator
                    "%", // percent sign
                    "0", // native 0 digit
                    "#", // pattern digit
                    "-", // minus sign
                    "E", // exponential
                    "\u2030", // per mille
                    "\u221e", // infinity
                    "\ufffd" // NaN
                }
            },
            { "DateTimePatterns",
                new String[] {
                    "H:mm:ss' uur' z", // full time pattern
                    "H:mm:ss z", // long time pattern
                    "H:mm:ss", // medium time pattern
                    "H:mm", // short time pattern
                    "EEEE d MMMM yyyy", // full date pattern
                    "d MMMM yyyy", // long date pattern
                    "d-MMM-yyyy", // medium date pattern
                    "d-M-yy", // short date pattern
                    "{1} {0}" // date-time pattern
                }
            },
            { "DateTimeElements",
                new String[] {
                    "2", // first day of week
                    "4" // min days in first week
                }
            }
        };
    }
}
