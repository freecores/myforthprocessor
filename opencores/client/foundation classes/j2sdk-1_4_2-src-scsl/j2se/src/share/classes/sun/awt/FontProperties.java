/*
 * @(#)FontProperties.java	1.10 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package sun.awt;

import java.io.File;
import java.io.FileInputStream;
import java.util.Locale;
import java.util.Properties;
import sun.io.CharacterEncoding;

public abstract class FontProperties extends Properties {

    protected static String osVersion;
    protected static String osName;

    public FontProperties() {
	super(getDefaultFontProperties());
	setOsNameAndVersion();
	initializeProperties();
    }

    private void initializeProperties() {

	// Find property file
        // give preference to lang_country_encoding within (user, jhome)
	String uhome = System.getProperty("user.home");
	String jhome = System.getProperty("java.home");
	if (jhome == null){
	    throw new Error("java.home property not set");
	}
	Locale startupLocale = SunToolkit.getStartupLocale();
	String language = startupLocale.getLanguage();
	String country = startupLocale.getCountry();

        // Translate the raw encoding name returned by the VM to the canonical
        // name from the alias table in CharacterEncoding. Map unlisted raw
        // encoding names to themselves. - liu 8/10/98 bug 4163038
	String rawEncoding = System.getProperty("file.encoding");
        String encoding = CharacterEncoding.aliasName(rawEncoding);
        if (encoding == null) {
            encoding = rawEncoding;
        }

	try {
	    File f = null;

            if (country != null) {
                f = tryOpeningFontProp(
                        f, uhome, language, country + "_" + encoding);
                f = tryOpeningFontProp(
                        f, jhome, language, country + "_" + encoding);
                f = tryOpeningFontProp(f, uhome, language, country);
                f = tryOpeningFontProp(f, jhome, language, country);
            }

            f = tryOpeningFontProp(f, uhome, language, encoding);
            f = tryOpeningFontProp(f, jhome, language, encoding);
            f = tryOpeningFontProp(f, uhome, language, null);
            f = tryOpeningFontProp(f, jhome, language, null);
            f = tryOpeningFontProp(f, uhome, encoding, null);
            f = tryOpeningFontProp(f, jhome, encoding, null);
            f = tryOpeningFontProp(f, uhome, null, null);
            f = tryOpeningFontProp(f, jhome, null, null);

            if (f != null) {
	        // Load property file
	        FileInputStream in = new FileInputStream(f.getPath());
	        load(in);
	        in.close();

                //PLSF support will not be activated if _PLSF_LANGUAGE has not been set
                if (("ja".equals(language) ||
                     "zh".equals(language) ||
                     "ko".equals(language) ||
                     "iw".equals(language) ||
                     "th".equals(language) ||
                     "hi".equals(language)) &&
                     isOriginalFP(f)) {
                    put("_PLSF_LANGUAGE", language);
                }

            }
        } catch (Exception e){
	}
    }

    private File tryOpeningFontProp(File f, String homedir,
				    String language, String ext) {
        if (f != null) {
            return f;       // already validated
        }
        String filename = homedir + File.separator
                            + "lib" + File.separator
                            + "font.properties";
        if (language != null) {
            filename += "." + language;
            if (ext != null) {
                filename += "_" + ext;
            }
        }

        File propsFile;
        // Try the os-specific prop file first
        if (osVersion != null && osName != null){
            propsFile = new File(filename+"."+ osName + osVersion);
            if ((propsFile != null) && propsFile.canRead()) {
                return propsFile;
            }
	}
        if (osName != null){
            propsFile = new File(filename+"." + osName);
            if ((propsFile != null) && propsFile.canRead()) {
                return propsFile;
            }
	}
        if (osVersion != null){
            propsFile = new File(filename+"."+ osVersion);
            if ((propsFile != null) && propsFile.canRead()) {
                return propsFile;
            }
	}

        propsFile = new File(filename);
        if ((propsFile != null) && propsFile.canRead()) {
            return propsFile;
        }
        
        return null;
    }

    private static Properties getDefaultFontProperties() {
	// set default props to prevent crashing
	// with corrupted or missing font.properties
	Properties defaultProps = new Properties();
	defaultProps.put("serif.0", "unknown");
	defaultProps.put("sansserif.0", "unknown");
	defaultProps.put("monospaced.0", "unknown");
	defaultProps.put("dialog.0", "unknown");
	defaultProps.put("dialoginput.0", "unknown");

	defaultProps.put("default.0", "sansserif");
	return defaultProps;
    }

    protected void setOsNameAndVersion() {
        osName = System.getProperty("os.name");
        osVersion = System.getProperty("os.version");
    }

    /* methods for handling font and style names ******************************/

    private static final String[] fontNames
            = {"serif", "sansserif", "monospaced", "dialog", "dialoginput"};
    
    /**
     * Checks whether the given font family name is a valid logical font name.
     * The check is case insensitive.
     */
    public static boolean isLogicalFontFamilyName(String fontName) {
        fontName = fontName.toLowerCase(Locale.ENGLISH);
        for (int i = 0; i < fontNames.length; i++) {
            if (fontName.equals(fontNames[i])) {
                return true;
            }
        }
        return false;
    }
    
    /**
     * Returns the alias for fontName if a valid alias has been defined,
     * null otherwise. A valid alias is the family name of a logical
     * font in lowercase.
     * REMIND: remove this method and references to it from the next feature release.
     */
    public String getAliasedFamilyName(String fontName) {
        assert !isLogicalFontFamilyName(fontName);
        fontName = fontName.toLowerCase(Locale.ENGLISH);
        String aliasName = getProperty("alias." + fontName);
        if (aliasName != null) {
            for (int i = 0; i < fontNames.length; i++) {
                if (aliasName.equals(fontNames[i])) {
                    return aliasName;
                }
            }
        }
        return null;
    }
    
    /**
     * Returns a fallback name for the given font name. For a few known
     * font names, matching logical font names are returned. For all
     * other font names, defaultFallback is returned.
     * defaultFallback differs between AWT and 2D.
     * REMIND: remove this method and references to it from the next feature release.
     */
    public abstract String getFallbackFamilyName(String fontName, String defaultFallback);
    
    /**
     * Returns the 1.1 equivalent for some old 1.0 font family names for which
     * we need to maintain compatibility in some configurations.
     * Returns null for other font names.
     * REMIND: remove this method and references to it from the next feature release.
     */
    protected String getCompatibilityFamilyName(String fontName) {
        fontName = fontName.toLowerCase(Locale.ENGLISH);
        if (fontName.equals("timesroman")) {
            return "serif";
        } else if (fontName.equals("helvetica")) {
            return "sansserif";
        } else if (fontName.equals("courier")) {
            return "monospaced";
        }
        return null;
    }

    protected boolean isOriginalFP(File f) {
        return false;
    }

    public boolean supportPLSF() {
        return false;
    }

    public FontProperties applyPreferLocaleSpecificFonts(FontProperties fprops) {
        return this;
    }
}
