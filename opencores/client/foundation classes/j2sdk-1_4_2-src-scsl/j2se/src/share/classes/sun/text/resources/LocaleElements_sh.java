/*
 * @(#)LocaleElements_sh.java	1.16 03/01/23
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

public class LocaleElements_sh extends ListResourceBundle {
    /**
     * Overrides ListResourceBundle
     */
    public Object[][] getContents() {
        return new Object[][] {
            { "Languages", // language names
                new String[][] {
                    { "sh", "Srpski" }
                }
            },
            { "Countries", // country names
                new String[][] {
                    { "YU", "Jugoslavija" }
                }
            },
            { "MonthNames",
                new String[] {
                    "Januar", // january
                    "Februar", // february
                    "Mart", // march
                    "April", // april
                    "Maj", // may
                    "Juni", // june
                    "Juli", // july
                    "Avgust", // august
                    "Septembar", // september
                    "Oktobar", // october
                    "Novembar", // november
                    "Decembar", // december
                    "" // month 13 if applicable
                }
            },
            { "MonthAbbreviations",
                new String[] {
                    "Jan", // abb january
                    "Feb", // abb february
                    "Mar", // abb march
                    "Apr", // abb april
                    "Maj", // abb may
                    "Jun", // abb june
                    "Jul", // abb july
                    "Avg", // abb august
                    "Sep", // abb september
                    "Okt", // abb october
                    "Nov", // abb november
                    "Dec", // abb december
                    "" // abb month 13 if applicable
                }
            },
            { "DayNames",
                new String[] {
                    "Nedelja", // Sunday
                    "Ponedeljak", // Monday
                    "Utorak", // Tuesday
                    "Sreda", // Wednesday
                    "\u010cetvrtak", // Thursday
                    "Petak", // Friday
                    "Subota" // Saturday
                }
            },
            { "DayAbbreviations",
                new String[] {
                    "Ned", // abb Sunday
                    "Pon", // abb Monday
                    "Uto", // abb Tuesday
                    "Sre", // abb Wednesday
                    "\u010cet", // abb Thursday
                    "Pet", // abb Friday
                    "Sub" // abb Saturday
                }
            },
            { "Eras",
                new String[] { // era strings
                    "p. n. e.",
                    "n. e."
                }
            },
            { "NumberElements",
                new String[] {
                    ",", // decimal separator
                    ".", // group (thousands) separator
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
                    "HH.mm.ss z", // full time pattern
                    "HH.mm.ss z", // long time pattern
                    "H.mm.ss", // medium time pattern
                    "H.mm", // short time pattern
                    "EEEE, yyyy, MMMM d", // full date pattern
                    "EEEE, yyyy, MMMM d", // long date pattern
                    "yyyy.M.d", // medium date pattern
                    "yy.M.d", // short date pattern
                    "{1} {0}" // date-time pattern
                }
            },
            { "CollationElements",
                /* for sh_SP, default sorting except for the following: */

                /* add dz "ligature" between d and d<stroke>. */
                /* add d<stroke> between d and e. */
                /* add lj "ligature" between l and l<stroke>. */
                /* add l<stroke> between l and m. */
                /* add nj "ligature" between n and o. */
                /* add z<abovedot> after z.       */

                "& D < dz, Dz, dZ, DZ < \u0111, \u0110"      /* dz + d<stk> */
                +"& L < lj, Lj, lJ, LJ < \u0142, \u0141"     /* lj + l<stk> */
                +"& N < nj, Nj, nJ, NJ"                      /* nj ligature */
                +"& Z < \u017c, \u017b"                      /* z<abovedot> */
            }
        };
    }
}
