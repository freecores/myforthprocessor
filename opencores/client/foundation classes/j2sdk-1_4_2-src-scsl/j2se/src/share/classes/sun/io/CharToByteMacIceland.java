/*
 * @(#)CharToByteMacIceland.java	1.11 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.io;

/**
 * Tables and data to convert Unicode to MacIceland
 *
 * @author  ConverterGenerator tool
 * @version >= JDK1.1.6
 */

public class CharToByteMacIceland extends CharToByteSingleByte {

    public String getCharacterEncoding() {
        return "MacIceland";
    }

    public CharToByteMacIceland() {
        super.mask1 = 0xFF00;
        super.mask2 = 0x00FF;
        super.shift = 8;
        super.index1 = index1;
        super.index2 = index2;
    }

    private final static String index2 =

        "\u0000\u0001\u0002\u0003\u0004\u0005\u0006\u0007" + 
        "\b\t\n\u000B\f\r\u000E\u000F" + 
        "\u0010\u0011\u0012\u0013\u0014\u0015\u0016\u0017" + 
        "\u0018\u0019\u001A\u001B\u001C\u001D\u001E\u001F" + 
        "\u0020\u0021\"\u0023\u0024\u0025\u0026\'" + 
        "\u0028\u0029\u002A\u002B\u002C\u002D\u002E\u002F" + 
        "\u0030\u0031\u0032\u0033\u0034\u0035\u0036\u0037" + 
        "\u0038\u0039\u003A\u003B\u003C\u003D\u003E\u003F" + 
        "\u0040\u0041\u0042\u0043\u0044\u0045\u0046\u0047" + 
        "\u0048\u0049\u004A\u004B\u004C\u004D\u004E\u004F" + 
        "\u0050\u0051\u0052\u0053\u0054\u0055\u0056\u0057" + 
        "\u0058\u0059\u005A\u005B\\\u005D\u005E\u005F" + 
        "\u0060\u0061\u0062\u0063\u0064\u0065\u0066\u0067" + 
        "\u0068\u0069\u006A\u006B\u006C\u006D\u006E\u006F" + 
        "\u0070\u0071\u0072\u0073\u0074\u0075\u0076\u0077" + 
        "\u0078\u0079\u007A\u007B\u007C\u007D\u007E\u007F" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u00CA\u00C1\u00A2\u00A3\u00DB\u00B4\u0000\u00A4" + 
        "\u00AC\u00A9\u00BB\u00C7\u00C2\u0000\u00A8\u00F8" + 
        "\u00A1\u00B1\u0000\u0000\u00AB\u00B5\u00A6\u00E1" + 
        "\u00FC\u0000\u00BC\u00C8\u0000\u0000\u0000\u00C0" + 
        "\u00CB\u00E7\u00E5\u00CC\u0080\u0081\u00AE\u0082" + 
        "\u00E9\u0083\u00E6\u00E8\u00ED\u00EA\u00EB\u00EC" + 
        "\u00DC\u0084\u00F1\u00EE\u00EF\u00CD\u0085\u0000" + 
        "\u00AF\u00F4\u00F2\u00F3\u0086\u00A0\u00DE\u00A7" + 
        "\u0088\u0087\u0089\u008B\u008A\u008C\u00BE\u008D" + 
        "\u008F\u008E\u0090\u0091\u0093\u0092\u0094\u0095" + 
        "\u00DD\u0096\u0098\u0097\u0099\u009B\u009A\u00D6" + 
        "\u00BF\u009D\u009C\u009E\u009F\u00E0\u00DF\u00D8" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u00F5\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u00CE\u00CF\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u00D9\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u00C4\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u00F6\u00FF\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u00F9\u00FA\u00FB" + 
        "\u00FE\u00F7\u00FD\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u00B9\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u00D0\u00D1\u0000\u0000\u0000" + 
        "\u00D4\u00D5\u00E2\u0000\u00D2\u00D3\u00E3\u0000" + 
        "\u0000\u0000\u00A5\u0000\u0000\u0000\u00C9\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u00E4\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u00DA\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u00AA\u0000\u0000\u0000\u00BD\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u00B6\u0000" + 
        "\u0000\u0000\u00C6\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u00B8\u0000\u00B7\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u00C3\u0000" + 
        "\u0000\u0000\u00B0\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u00BA" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u00C5\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u00AD\u0000\u0000\u0000" + 
        "\u00B2\u00B3\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u00D7\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000" + 
        "\u0000\u0000\u0000\u0000\u00F0"; 

    private final static short index1[] = {
        0, 256, 461, 683, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 
        403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 
        920, 1142, 1396, 403, 403, 1498, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 
        403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 
        403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 
        403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 
        403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 
        403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 
        403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 
        403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 
        403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 
        403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 
        403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 
        403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 
        403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 
        403, 403, 403, 403, 403, 403, 403, 403, 1701, 403, 403, 403, 403, 403, 403, 403, 
    };

}
