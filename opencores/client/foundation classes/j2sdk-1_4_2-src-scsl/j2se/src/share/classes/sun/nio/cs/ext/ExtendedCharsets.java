/*
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * @(#)ExtendedCharsets.java	1.12 03/01/23
 */

package sun.nio.cs.ext;

import java.lang.ref.SoftReference;
import java.nio.charset.Charset;
import java.nio.charset.spi.CharsetProvider;
import sun.nio.cs.AbstractCharsetProvider;
import java.security.AccessController;
import sun.security.action.GetPropertyAction;


/**
 * Provider for extended charsets.
 *
 */

public class ExtendedCharsets
    extends AbstractCharsetProvider
{

    static volatile SoftReference instance = null;

    private static final boolean sjisIsMS932;

    static {
	GetPropertyAction a = new GetPropertyAction("sun.nio.cs.map");
	String map = ((String)AccessController.doPrivileged(a));
	if (map != null) {
	    sjisIsMS932 = map.equalsIgnoreCase("Windows-31J/Shift_JIS");
	} else {
	    sjisIsMS932 = false;
	}
    }

    public ExtendedCharsets() {

	super("sun.nio.cs.ext");  // identify provider pkg name.

	// Traditional Chinese

	charset("Big5", "Big5",
		new String[] {
		    // IANA aliases
		    "csBig5"
		});

	charset("x-MS950-HKSCS", "MS950_HKSCS",
		new String[] {
		    // IANA aliases
		    "MS950_HKSCS"  // JDK historical;
		});

	charset("x-windows-950", "MS950",
		new String[] {
		    "ms950",	// JDK historical
		    "windows-950"
		});

	charset("x-EUC-TW", "EUC_TW",
		new String[] {
		    "euc_tw", // JDK historical
		    "euctw",
		    "cns11643",
		    "EUC-TW"    
	        });

	charset("Big5-HKSCS", "Big5_HKSCS",
		new String[] {
		    "Big5_HKSCS", // JDK historical
		    "big5-hkscs",
		    "big5hkscs" // Linux alias
		});

	// Simplified Chinese
	charset("GBK", "GBK",
		new String[] {
		    "windows-936",
		    "CP936"
		});

	charset("GB18030", "GB18030",
		new String[] {
		    "gb18030-2000"
		});

	charset("x-EUC-CN", "EUC_CN",
		new String[] {
		    // IANA aliases
		    "gb2312",
		    "gb2312-80",
		    "gb2312-1980",
		    "euc-cn",
		    "euccn",
		    "EUC_CN" //JDK historical
		});

	charset("x-mswin-936", "MS936",
		new String[] {
		    "ms936", // historical
		    // IANA aliases
		    "ms_936"
		});


	// Japanese

	if (!sjisIsMS932) {

	    // JIS defined Shift JIS
	    // Uses IANA aliases (4556882)
	    charset("Shift_JIS", "SJIS",
		    new String[] {
			// IANA aliases
			"sjis", // historical
			// Japanese name of Shift-JIS omitted (java.nio.Charset)
			// currently rejects the syntax of this name
			// "\u30b7\u30d5\u30c8\u7b26\u53f7\u5316\u8868\u73fe",
			"pck",
			"shift_jis",
			"shift-jis",
			"ms_kanji",
			"x-sjis",    
			"csShiftJIS"    
		    });

	    charset("windows-31j", "MS932",
		    new String[] {
		    "MS932", // JDK historical
		    "windows-932",
		    "csWindows31J"
		    });
	}

	charset("JIS_X0201", "JIS_X_0201",
		new String[] {
		    "JIS0201", // JDK historical 
		    // IANA aliases
		    "JIS_X0201",
		    "X0201",
		    "csHalfWidthKatakana"
		});

	charset("x-JIS0208", "JIS_X_0208",
		new String[] {
		    "JIS0208", // JDK historical
		    // IANA aliases
		    "JIS_C6626-1983",
		    "iso-ir-87",
		    "x0208",
		    "JIS_X0208-1983",
		    "csISO87JISX0208"
	        });

	charset("JIS_X0212-1990", "JIS_X_0212",
		new String[] {
		    "JIS0212", // JDK historical
		    // IANA aliases
		    "jis_x0212-1990",
		    "x0212",
		    "iso-ir-159",
		    "csISO159JISX02121990"
		});

	charset("EUC-JP", "EUC_JP",
		new String[] {
		    "euc_jp", // JDK historical
		    // IANA aliases
		    "eucjis",
		    "eucjp",
		    "Extended_UNIX_Code_Packed_Format_for_Japanese",
		    "csEUCPkdFmtjapanese",
		    "x-euc-jp",
		    "x-eucjp"
		});

	charset("x-euc-jp-linux", "EUC_JP_LINUX",
		new String[] {
		    "euc_jp_linux", // JDK historical
		    "euc-jp-linux"
		});

	charset("ISO-2022-JP", "ISO2022_JP",
	    new String[] {
	    // IANA aliases
	    "iso2022jp", // historical
	    "jis",
	    "csISO2022JP",
	    "jis_encoding",          
	    "csjisencoding"
	});

	// Korean
	charset("EUC-KR", "EUC_KR",
		new String[] {
		    "euc_kr", // JDK historical
		    // IANA aliases
		    "ksc5601",
		    "euckr",
		    "ks_c_5601-1987",
		    "ksc5601-1987",
		    "ksc5601_1987",
		    "ksc_5601",
		    "csEUCKR",
		    "5601"
		});

	charset("x-windows-949", "MS949",
		new String[] {
		    "ms949",	// JDK historical
		    "windows949",
		    // IANA aliases
		    "ms_949"
		});

	charset("x-Johab", "Johab",
		new String[] {
			"ksc5601-1992",
			"ksc5601_1992",
			"ms1361",
			"johab" // JDK historical
		});

	charset("ISO-2022-KR", "ISO2022_KR",
		new String[] {
			"ISO2022KR", // JDK historical
			"csISO2022KR"
		});

	charset("x-ISCII91", "ISCII91",
		new String[] {
			"iscii",
			"ST_SEV_358-88",
			"iso-ir-153",
			"csISO153GOST1976874",
			"ISCII91" // JDK historical
		});

	charset("ISO-8859-3", "ISO_8859_3",
		new String[] {
		    "iso8859_3", // JDK historical
		    "ISO_8859-3:1988",
		    "iso-ir-109",
		    "ISO_8859-3",
		    "latin3",
		    "l3",
		    "csISOLatin3"
		});

	charset("ISO-8859-6", "ISO_8859_6",
		new String[] {
		    "iso8859_6", // JDK historical
		    "iso-ir-127",
		    "ISO_8859-6",
		    "ISO_8859-6:1987",
		    "ECMA-114",
		    "ASMO-708",
		    "arabic",
		    "csISOLatinArabic"
		});

	charset("ISO-8859-8", "ISO_8859_8",
		new String[] {
		    "iso8859_8", // JDK historical
		    "iso-ir-138",
		    "ISO_8859-8",
		    "ISO_8859-8:1988",
		    "hebrew",
		    "csISOLatinHebrew"
		});

	charset("TIS-620", "TIS_620",
		new String[] {
		    "tis620", // JDK historical
		    "tis620.2533"
		});

	// Various Microsoft Windows international codepages

	charset("windows-1255", "MS1255",
		new String[] {
		    "cp1255" // JDK historical
		});

	charset("windows-1256", "MS1256",
		new String[] {
		    "cp1256" // JDK historical
		});

	charset("windows-1258", "MS1258",
		new String[] {
		    "cp1258" // JDK historical
		});

	instance = new SoftReference(this);

    }

    public static String[] aliasesFor(String charsetName) {
	SoftReference sr = instance;
	ExtendedCharsets sc = null;
	if (sr != null)
	    sc = (ExtendedCharsets)sr.get();
	if (sc == null) {
	    sc = new ExtendedCharsets();
	    instance = new SoftReference(sc);
	}
	return sc.aliases(charsetName);
    }
}
