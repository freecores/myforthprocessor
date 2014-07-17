/*
 * @(#)PlatformFont.java	1.48 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package sun.awt;

import java.awt.Font;
import java.awt.GraphicsEnvironment;
import java.awt.peer.FontPeer;
import java.util.Hashtable;
import java.util.Locale;
import java.util.Vector;
import sun.io.CharToByteConverter;
import sun.io.ConversionBufferFullException;
import sun.io.MalformedInputException;
import sun.java2d.FontSupport;

public abstract class PlatformFont implements FontPeer {

    static {
        NativeLibLoader.loadLibraries();
        initIDs();
    }

    protected FontDescriptor[] componentFonts;
    protected char defaultChar;
    protected FontProperties props;

    protected FontDescriptor defaultFont;

    protected static Hashtable charsetRegistry = new Hashtable(5);
    protected String familyName;
    protected String styleString;

    private Object[] fontCache;

    // Maybe this should be a property that is set based
    // on the locale?
    protected static int FONTCACHESIZE = 256;
    protected static int FONTCACHEMASK = PlatformFont.FONTCACHESIZE - 1;
    protected static String osVersion;

    public PlatformFont(String name, int style){
	GraphicsEnvironment ge = GraphicsEnvironment.getLocalGraphicsEnvironment();
	if (ge instanceof FontSupport) {
	    props = ((FontSupport)ge).getFontProperties();
        }
        if (props == null && (props = fprops) == null) {
            return;
        }

        // map given font name to a valid logical font family name
        familyName = name.toLowerCase(Locale.ENGLISH);
        if (!FontProperties.isLogicalFontFamilyName(familyName)) {
            String aliasName = props.getAliasedFamilyName(familyName);
            if (aliasName != null) {
                familyName = aliasName;
            } else {
                familyName = props.getFallbackFamilyName(familyName, "sansserif");
            }
        }

        // build font descriptors
	styleString = styleStr(style);

	Vector compFonts = new Vector(5);

	int numOfFonts = 0;
	for (;; numOfFonts++){
	    String index = String.valueOf(numOfFonts);

	    // search native font name
	    //
	    String nativeName = props.getProperty
		(familyName + "." + styleString + "." + index);
	    if (nativeName == null){
		nativeName = props.getProperty(familyName + "." + index);
		if (nativeName == null){
		    break;
		}
	    }

	    // search font charset
	    //
	    String fcName = props.getProperty
		("fontcharset." + familyName + "." + styleString + "." + index);
	    if (fcName == null){
		fcName = props.getProperty
		    ("fontcharset." + familyName + "." + index);
		if (fcName == null){
		    fcName = "default";
		}
	    }

	    CharToByteConverter
		fontCharset = getFontCharset(fcName.trim(), nativeName);

	    // search exclusion range for this font
	    //
	    String exString = props.getProperty
		("exclusion." + familyName + "." + styleString +  "." + index);
	    if (exString == null){
		exString = props.getProperty
		    ("exclusion." + familyName + "." + index);
		if (exString == null){
		    exString = "none";
		}
	    }

	    int[] exRange;
	    if (exString.equals("none")){
		exRange = new int[0];
	    } else {
		/*
		 * range format is xxxx-XXXX,yyyy-YYYY,.....
		 */
		int numRange = 1, idx = 0;
		for (;; numRange++){
		    idx = exString.indexOf(',', idx);
		    if (idx == -1){
			break;
		    }
		    idx++;
		}
		exRange = new int[numRange];
		for (int j = 0; j < numRange; j++){
		    String lower, upper;
		    int lo = 0, up =0;
		    try {
			lower = exString.substring(j * 10, j * 10 + 4);
			upper = exString.substring(j * 10 + 5, j * 10 + 9);
		    } catch(StringIndexOutOfBoundsException e){
			exRange = new int[0];
			break;
		    }
		    try {
			lo = Integer.parseInt(lower, 16);
			up = Integer.parseInt(upper, 16);
		    } catch(NumberFormatException e){
			exRange = new int[0];
			break;
		    }
		    exRange[j] = lo << 16 | up;
		}
	    }

	    compFonts.addElement(new FontDescriptor(nativeName,
						    fontCharset,
						    exRange));
	}

	componentFonts = new FontDescriptor[numOfFonts];
	for (int i = 0; i < numOfFonts; i++){
	    componentFonts[i] = (FontDescriptor)compFonts.elementAt(i);
	}

	// search default character
	//
	int dfChar;
	try {
	    dfChar = Integer.parseInt
		(props.getProperty("default.char", "003f"), 16);
	} catch (NumberFormatException e){
	    dfChar = 0x3f;
	}

	defaultChar = 0x3f;
	if (componentFonts.length > 0)
	    defaultFont = componentFonts[0];

	for (int i = 0; i < componentFonts.length; i++){
	    if (componentFonts[i].isExcluded((char)dfChar)){
		continue;
	    }

	    if (componentFonts[i].fontCharset.canConvert((char)dfChar)){
		defaultFont = componentFonts[i];
		defaultChar = (char)dfChar;
		break;
	    }
	}
    }

    /*
     * make default font properties.
     */
    protected static FontProperties fprops;

    static {
	GraphicsEnvironment ge;
	ge = GraphicsEnvironment.getLocalGraphicsEnvironment();
	if (ge instanceof FontSupport)
	    fprops = ((FontSupport)ge).getFontProperties();
    }

    /**
     * make a array of CharsetString with given String.
     */
    public CharsetString[] makeMultiCharsetString(String str){
	return makeMultiCharsetString(str.toCharArray(), 0, str.length(), true);
    }

    /**
     * make a array of CharsetString with given String.
     */
    public CharsetString[] makeMultiCharsetString(String str, boolean allowdefault){
	return makeMultiCharsetString(str.toCharArray(), 0, str.length(), allowdefault);
    }

    /**
     * make a array of CharsetString with given char array.
     * @param str The char array to convert.
     * @param offset offset of first character of interest
     * @param len number of characters to convert
     */
    public CharsetString[] makeMultiCharsetString(char str[], int offset, int len) {
	return makeMultiCharsetString(str, offset, len, true);
    }

    /**
     * make a array of CharsetString with given char array.
     * @param str The char array to convert.
     * @param offset offset of first character of interest
     * @param len number of characters to convert
     * @param allowDefault whether to allow the default char.
     * Setting this to true overloads the meaning of this method to
     * return non-null only if all chars can be converted.
     * @return array of CharsetString or if allowDefault is false and any
     * of the returned chars would have been converted to a default char,
     * then return null.
     * This is used to choose alternative means of displaying the text.
     */
    public CharsetString[] makeMultiCharsetString(char str[], int offset, int len,
						  boolean allowDefault) {

	if (len < 1) {
	    return new CharsetString[0];
	}
	Vector mcs = null;
	char[] tmpStr = new char[len];
	char tmpChar = defaultChar;

	FontDescriptor currentFont = defaultFont;


	for (int i = 0; i < componentFonts.length; i++) {
	    if (componentFonts[i].isExcluded(str[offset])){
		continue;
	    }

	    if (componentFonts[i].fontCharset.canConvert(str[offset])){
		currentFont = componentFonts[i];
		tmpChar = str[offset];
		break;
	    }
	}
	if (!allowDefault && tmpChar == defaultChar) {
	    return null;
	} else {
	    tmpStr[0] = tmpChar;
	}

	int lastIndex = 0;
	for (int i = 1; i < len; i++){
	    char ch = str[offset + i];
	    FontDescriptor fd = defaultFont;
	    tmpChar = defaultChar;
	    for (int j = 0; j < componentFonts.length; j++){
		if (componentFonts[j].isExcluded(ch)){
		    continue;
		}

		if (componentFonts[j].fontCharset.canConvert(ch)){
		    fd = componentFonts[j];
		    tmpChar = ch;
		    break;
		}
	    }
	    if (!allowDefault && tmpChar == defaultChar) {
		return null;
	    } else {
		tmpStr[i] = tmpChar;
	    }
	    if (currentFont != fd){
		if (mcs == null) {
		    mcs = new Vector(3);
		}
		mcs.addElement(new CharsetString(tmpStr, lastIndex,
						 i-lastIndex, currentFont));
		currentFont = fd;
		fd = defaultFont;
		lastIndex = i;
	    }
	}
	CharsetString[] result;
	CharsetString cs = new CharsetString(tmpStr, lastIndex,
					    len-lastIndex, currentFont);
	if (mcs == null) {
	    result = new CharsetString[1];
	    result[0] = cs;
	} else {
	    mcs.addElement(cs);
	    result = new CharsetString[mcs.size()];
	    for (int i = 0; i < mcs.size(); i++){
		result[i] = (CharsetString)mcs.elementAt(i);
	    }
	}
	return result;
    }

    /**
     * Is it possible that this font's metrics require the multi-font calls?
     * This might be true, for example, if the font supports kerning.
    **/
    public boolean mightHaveMultiFontMetrics() {
	return props != null;
    }

    /**
     * Specialized fast path string conversion for AWT.
     */
    public Object[] makeConvertedMultiFontString(String str)
        throws MalformedInputException,ConversionBufferFullException {
        return makeConvertedMultiFontChars(str.toCharArray(),0,str.length());
    }

    public Object[] makeConvertedMultiFontChars(char[] data,
						int start, int len)
    throws MalformedInputException,ConversionBufferFullException
    {
        Object[] result = new Object[2];
        Object[] workingCache;
        byte[] convertedData = null;
        int stringIndex = start;
        int convertedDataIndex = 0;
        int resultIndex = 0;
        int cacheIndex;
        FontDescriptor currentFontDescriptor = null;
        FontDescriptor lastFontDescriptor = null;
        char currentDefaultChar;
        PlatformFontCache theChar;

        // Simple bounds check
        int end = start + len;
        if (start < 0 || end > data.length) {
            throw new ArrayIndexOutOfBoundsException();
        }

        if(stringIndex >= end) {
            return null;
        }

        // coversion loop
        while(stringIndex < end)
        {
            currentDefaultChar = data[stringIndex];

            // Note that cache sizes must be a power of two!
            cacheIndex = (int)(currentDefaultChar & this.FONTCACHEMASK);

            theChar = (PlatformFontCache)getFontCache()[cacheIndex];

            // Is the unicode char we want cached?
            if(theChar == null || theChar.uniChar != currentDefaultChar)
            {
                /* find a converter that can convert the current character */
                currentFontDescriptor = defaultFont;
                currentDefaultChar = defaultChar;
                char ch = (char)data[stringIndex];
                int componentCount = componentFonts.length;

                for (int j = 0; j < componentCount; j++) {

                    FontDescriptor fontDescriptor = componentFonts[j];

                    fontDescriptor.fontCharset.reset();
                    fontDescriptor.fontCharset.setSubstitutionMode(false);

                    if (fontDescriptor.isExcluded(ch)) {
                        continue;
                    }
                    if (fontDescriptor.fontCharset.canConvert(ch)) {
                        currentFontDescriptor = fontDescriptor;
                        currentDefaultChar = ch;
                        break;
                    }
                }
                try {
                    char[] input = new char[1];
                    input[0] = currentDefaultChar;

                    theChar = new PlatformFontCache();

                    if (currentFontDescriptor.useUnicode()) {
                        theChar.encodingSize = 
                            (byte)currentFontDescriptor.unicodeCharset.convert(input, 0, 1, theChar.encoding, 0, 4);
		    }
                    else  {
                        theChar.encodingSize =
                            (byte)currentFontDescriptor.fontCharset.convert(input, 0, 1, theChar.encoding, 0, 4);
                    }

                    theChar.fontDescriptor = currentFontDescriptor;
                    theChar.uniChar = data[stringIndex];
                    getFontCache()[cacheIndex] = theChar;
                } catch(Exception e){
                    // Should never happen!
                    System.err.println(e);
                    e.printStackTrace();

                    return null;
                }
            }

            // Check to see if we've changed fonts.
            if(lastFontDescriptor != theChar.fontDescriptor) {
                if(lastFontDescriptor != null) {
                    result[resultIndex++] = lastFontDescriptor;
                    result[resultIndex++] = convertedData;
		    //  Add the size to the converted data field.
		    if(convertedData != null) {
			convertedDataIndex -= 4;
			convertedData[0] = (byte)(convertedDataIndex >> 24);
			convertedData[1] = (byte)(convertedDataIndex >> 16);
			convertedData[2] = (byte)(convertedDataIndex >> 8);
			convertedData[3] = (byte)convertedDataIndex;
		    }

                    if(resultIndex >= result.length) {
                        Object[] newResult = new Object[result.length * 2];

                        System.arraycopy(result, 0, newResult, 0,
					 result.length);
                        result = newResult;
                    }
                }

                if (theChar.fontDescriptor.useUnicode()) {
                    convertedData = new byte[(end - stringIndex + 1) * 
                                        theChar.fontDescriptor.unicodeCharset.getMaxBytesPerChar()
                                        + 4];
                }
                else  {
                    convertedData = new byte[(end - stringIndex + 1) *
                                        theChar.fontDescriptor.fontCharset.getMaxBytesPerChar()
                                        + 4];
                }

                convertedDataIndex = 4;

                lastFontDescriptor = theChar.fontDescriptor;
            }

            if(theChar.encodingSize == 1)
                convertedData[convertedDataIndex++] = theChar.encoding[0];
            else if(theChar.encodingSize == 2)
            {
                convertedData[convertedDataIndex++] = theChar.encoding[0];
                convertedData[convertedDataIndex++] = theChar.encoding[1];
            }
            else if(theChar.encodingSize == 3)
            {
                convertedData[convertedDataIndex++] = theChar.encoding[0];
                convertedData[convertedDataIndex++] = theChar.encoding[1];
                convertedData[convertedDataIndex++] = theChar.encoding[2];
            }
            else if(theChar.encodingSize == 4)
            {
	        convertedData[convertedDataIndex++] = theChar.encoding[0];
                convertedData[convertedDataIndex++] = theChar.encoding[1];
                convertedData[convertedDataIndex++] = theChar.encoding[2];
                convertedData[convertedDataIndex++] = theChar.encoding[3];
            }
            stringIndex++;
        }

        result[resultIndex++] = lastFontDescriptor;
        result[resultIndex++] = convertedData;

        //  Add the size to the converted data field.
        if(convertedData != null) {
            convertedDataIndex -= 4;
            convertedData[0] = (byte)(convertedDataIndex >> 24);
            convertedData[1] = (byte)(convertedDataIndex >> 16);
            convertedData[2] = (byte)(convertedDataIndex >> 8);
            convertedData[3] = (byte)convertedDataIndex;
        }
        return result;
    }

    protected abstract CharToByteConverter getFontCharset(String charsetName,
							   String fontName);

    /*
     * Create fontCache on demand instead of during construction to
     * reduce overall memory consumption.
     *
     * This method is declared final so that its code can be inlined
     * by the compiler.
     */
    protected final Object[] getFontCache() {
        // This method is not MT-safe by design. Since this is just a
        // cache anyways, it's okay if we occasionally allocate the array
        // twice or return an array which will be dereferenced and gced
        // right away.
        if (fontCache == null) {
	    fontCache = new Object[this.FONTCACHESIZE];
	}

	return fontCache;
    }

    /*
     * return String representation of style
     */
    public static String styleStr(int num){
	switch(num){
	  case Font.BOLD:
	    return "bold";
	  case Font.ITALIC:
	    return "italic";
	  case Font.ITALIC+Font.BOLD:
	    return "bolditalic";
	  default:
	    return "plain";
	}
    }

    /**
     * Initialize JNI field and method IDs
     */
    private static native void initIDs();

    class PlatformFontCache
    {
        char uniChar;
        FontDescriptor fontDescriptor;
        byte encodingSize;
        byte[] encoding = new byte[4];
    }
}

