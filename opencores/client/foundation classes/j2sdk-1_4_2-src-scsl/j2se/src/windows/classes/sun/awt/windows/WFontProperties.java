/*
 * @(#)WFontProperties.java	1.6 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt.windows;

import java.util.Locale;
import sun.awt.FontProperties;
import java.util.zip.Adler32;
import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.util.Properties;

public class WFontProperties extends FontProperties {

    // whether compatibility fallbacks for TimesRoman and Co. are used
    // REMIND: remove this variable and references to it from the next feature release.
    private boolean useCompatibilityFallbacks;
    
    public WFontProperties() {
	super();
        useCompatibilityFallbacks = "cp1252".equals(System.getProperty("file.encoding").toLowerCase(Locale.ENGLISH));
    }

    protected void setOsNameAndVersion(){
        super.setOsNameAndVersion();
        if (osName.startsWith("Windows")){
            int p, q;
            p = osName.indexOf(' ');
            if (p == -1){
                osName = null;
            }
            else{
                q = osName.indexOf(' ', p + 1);
                if (q == -1){
                    osName = osName.substring(p + 1);
		}
                else{
                    osName = osName.substring(p + 1, q);
                }
	    }
            osVersion = null;
        }
    }

    // overrides FontProperties.getFallbackFamilyName
    // REMIND: remove this method and references to it from the next feature release.
    public String getFallbackFamilyName(String fontName, String defaultFallback) {
        // maintain compatibility with old font.properties files, where
        // default file had aliases for timesroman & Co, while others didn't.
        if (useCompatibilityFallbacks) {
            String compatibilityName = getCompatibilityFamilyName(fontName);
            if (compatibilityName != null) {
                return compatibilityName;
            }
        }
        return defaultFallback;
    }

    protected boolean isOriginalFP(File file) {
        // get expected checksum
        Properties checksums = new Properties();
        try {
            checksums.load(ClassLoader.getSystemResourceAsStream("sun/awt/windows/font.properties.checksums"));
        } catch (IOException e) {
            return false;
        }
        String checksumString = checksums.getProperty(file.getName());
        if (checksumString == null) {
            return false;
        }
        long expectedChecksum = Long.parseLong(checksumString);
        
        // get actual checksum
        Adler32 checksum = new Adler32();
        byte[] buffer = new byte[1024];
        int length = 0;
        try {
            FileInputStream in = new FileInputStream(file);
            while ((length = in.read(buffer)) != -1) {
                checksum.update(buffer, 0, length);
            }
            in.close();
        } catch (IOException e) {
            return false;
        }
        return expectedChecksum == checksum.getValue();
    }

    public boolean supportPLSF() {
        return containsKey("_PLSF_LANGUAGE");
    }

    public FontProperties applyPreferLocaleSpecificFonts(FontProperties fprops){
        String lang = getProperty("_PLSF_LANGUAGE");
        if (lang == null) {
            return this;
	} 

        String[] keys = (String[])fprops.keySet().toArray(new String[0]);
        String key = null;
        String value = null;

        //need a better way to get a new FontProperties TBD
        FontProperties fp = (FontProperties)this.clone();
        fp.clear();

        if (lang.equals("ja")) { 
            for (int i = 0; i < keys.length; i++){
                key = keys[i];
                value = getProperty(key);
                //do nothing for monospaced
                if (key.indexOf("monospaced") == -1){
                    //swap slot 0 and 1
                    if (key.endsWith(".0")){
                        key = key.replace('0', '1');
                    } else if (key.endsWith(".1")){
                        key = key.replace('1', '0');
                    }
                }
		//Dialoginput MS GOTHIC -> MS MINCHO
                if (key.startsWith("dialoginput") && 
                    value.startsWith("\uff2d\uff33 \u30b4\u30b7\u30c3\u30af")) {
                    value = "\uff2d\uff33 \u660e\u671d,SHIFTJIS_CHARSET";
                } else if (!key.startsWith("monospaced") && 
                    value.startsWith("\uff2d\uff33 \u30b4\u30b7\u30c3\u30af")){
                    //MS GOTHIC -> MS PGOTHIC (not for monospaced)
                    value = "\uff2d\uff33 \uff30\u30b4\u30b7\u30c3\u30af,SHIFTJIS_CHARSET";
                } else if (value.startsWith("\uff2d\uff33 \u660e\u671d")){
                    //MS MINCHO -> MS PMINCHO
                    value = "\uff2d\uff33 \uff30\u660e\u671d,SHIFTJIS_CHARSET";
                }
                fp.put(key, value);
	    }
	} else if (lang.equals("ko") || lang.equals("zh")) {
            for (int i = 0; i < keys.length; i++){
                key = keys[i];
                value = getProperty(key);
                //do nothing for monospaced
                if (key.indexOf("monospaced") == -1){
                    //swap slot 0 and 1
                    if (key.endsWith(".0")){
                        key = key.replace('0', '1');
                    } else if (key.endsWith(".1")){
                        key = key.replace('1', '0');
                    }
                }
                fp.put(key, value);
	    }
        } else if (lang.equals("iw") || lang.equals("th") || lang.equals("hi")){ 
            for (int i = 0; i < keys.length; i++){
                key = keys[i];
                value = getProperty(key);
                //swap slot 0 and 1
                if (key.endsWith(".0")){
                    key = key.replace('0', '1');
                } else if (key.endsWith(".1")){
                    key = key.replace('1', '0');
                }
                fp.put(key, value);
	    }
        }
        return fp;
    }
}


