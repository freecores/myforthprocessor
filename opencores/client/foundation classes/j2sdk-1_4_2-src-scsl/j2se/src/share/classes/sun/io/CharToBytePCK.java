/*
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.io;

/**
 * @author Limin Shi
 * @author Mark Son-Bell
 * @author Ian Little
 * @version 1.2, 03/01/23
 *
 * PCK char->byte converter for Solaris platform containing additional
 * mappings for vendor defined chars (NEC row 13 & IBM extension chars)
 * (bugID 4765370)
 */
public class CharToBytePCK extends CharToByteJIS0208_Solaris {
    CharToByteJIS0201 cbJIS0201 = new CharToByteJIS0201();
    CharToByteJIS_IBM_Extensions cbIBMExt = new CharToByteJIS_IBM_Extensions();

    public String getCharacterEncoding() {
        return "PCK";
    }

    protected int convSingleByte(char inputChar, byte[] outputByte) {
        byte b;

        // \u0000 - \u007F map straight through
        if ((inputChar & 0xFF80) == 0) {
            outputByte[0] = (byte)inputChar;
            return 1;
        }

        if ((b = cbJIS0201.getNative(inputChar)) == 0)
            return 0;

        outputByte[0] = b;
        return 1;
    }

    protected int getNative(char ch) {
        int offset = index1[ch >> 8] << 8;
        int pos = index2[offset >> 12].charAt((offset & 0xfff) + (ch & 0xff));

        if (pos != 0) {
            /*
             * This algorithm for converting from JIS to SJIS comes from Ken Lunde's
             * "Understanding Japanese Information Processing", pg 163.
             */
            int c1 = (pos >> 8) & 0xff;
            int c2 = pos & 0xff;
            int rowOffset = c1 < 0x5F ? 0x70 : 0xB0;
            int cellOffset = (c1 % 2 == 1) ? (c2 > 0x5F ? 0x20 : 0x1F) : 0x7E;
            return ((((c1 + 1 ) >> 1) + rowOffset) << 8) | (c2 + cellOffset);
        } else {
            int result = 0;
            return (((result = cbIBMExt.getNative(ch)) != 0xFFFD) ? result: 0);
        }
    }
}



