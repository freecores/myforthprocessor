/*
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * @(#)SpecialCaseMap.java	1.3 03/01/23
 */


import java.io.*;
import java.util.*;

/**
 * SpecialCaseMap has the responsibility of storing the
 * 1:M case mappings that occur when uppercasing Unicode 3
 * characters. This class can also read and parse the
 * SpecialCasing.txt file that contains those mappings.
 * A single SpecialCaseMap contains the mapping for one char
 * to 1 or more char values.
 * <p>
 * @author John O'Conner
 */
public class SpecialCaseMap implements Comparator {

    SpecialCaseMap() {
        chSource = '\uFFFF';
    }


    /**
     * Read and parse a Unicode special case map file.
     *
     * @param file   a file specifying the Unicode special case mappings
     * @return 	an array of SpecialCaseMap objects, one for each line of the
     *		special case map data file that could be successfully parsed
     */

    public static SpecialCaseMap[] readSpecFile(File file) throws FileNotFoundException {
        Vector caseMaps = new Vector(150);
        int count = 0;
        BufferedReader f = new BufferedReader(new FileReader(file));
        loop:
        while(true) {
            try {
                String line = f.readLine();
            	if (line == null) break loop;
            	SpecialCaseMap item = parse(line.trim());
            	if (item != null) {
                    caseMaps.addElement(item);
                    ++count;
                }
            }
            catch (IOException e) { break loop; }
        }
        caseMaps.trimToSize();
        Collections.sort(caseMaps, (Comparator)caseMaps.elementAt(0));
        SpecialCaseMap[] result = new SpecialCaseMap[caseMaps.size()];
        caseMaps.toArray(result);
        return result;
    }

    /**
    * Given one line of a Unicode special casing data file as a String, parse the line
    * and return a SpecialCaseMap object that contains the case mapping.
    *
    * @param s a line of the Unicode special case map data file to be parsed
    * @return a SpecialCaseMap object, or null if the parsing process failed for some reason
    */
    public static SpecialCaseMap parse(String s) {
        SpecialCaseMap spec = null;
        String[] tokens = new String[REQUIRED_FIELDS];
        if ( s != null && s.length() != 0 && s.charAt(0) != '#') {
            try {
                int x = 0, tokenStart = 0, tokenEnd = 0;
                for (x=0; x<REQUIRED_FIELDS-1; x++) {
                    tokenEnd = s.indexOf(';', tokenStart);
                    tokens[x] = s.substring(tokenStart, tokenEnd);
                    tokenStart = tokenEnd+1;
                }
                tokens[x] = s.substring(tokenStart);
                // for now we don't want to include locale-sensitive or
                // context sensitive mappings
                if(tokens[FIELD_CONDITIONS].indexOf(';') == -1) {
                    spec = new SpecialCaseMap();
                    spec.setCharSource(parseChar(tokens[FIELD_SOURCE]));
                    spec.setUpperCaseMap(parseCaseMap(tokens[FIELD_UPPERCASE]));
                    spec.setLowerCaseMap(parseCaseMap(tokens[FIELD_LOWERCASE]));
                    spec.setTitleCaseMap(parseCaseMap(tokens[FIELD_TITLECASE]));
                    spec.setLocale(parseLocale(tokens[FIELD_CONDITIONS]));
                    spec.setContext(parseContext(tokens[FIELD_CONDITIONS]));
                }
            }
            catch(Exception e) {
                spec = null;
                System.out.println("Error parsing spec line.");
            }
        }
        return spec;
    }

    static char parseChar(String token) throws NumberFormatException {
        return (char)Integer.parseInt(token, 16);
    }

    static char[] parseCaseMap(String token ) throws NumberFormatException {
        int pos = 0;
        StringBuffer buff = new StringBuffer();
        int start = 0, end = 0;
        while(pos < token.length() ){
            while(Character.isSpaceChar(token.charAt(pos++)));
            --pos;
            start = pos;
            while(pos < token.length() && !Character.isSpaceChar(token.charAt(pos))) pos++;
            end = pos;
            char ch = parseChar(token.substring(start,end));
            buff.append(ch);
        }
        char[] map = new char[buff.length()];
        buff.getChars(0, buff.length(), map, 0);
        return map;
    }

    static Locale parseLocale(String token) {
        return null;
    }

    static String[] parseContext(String token) {
        return null;
    }

    static  int find(char ch, SpecialCaseMap[] map) {
        int top, bottom, current;
        bottom = 0;
        top = map.length;
        current = top/2;
        // invariant: top > current >= bottom && ch >= map.chSource
        while (top - bottom > 1) {
            if (ch >= map[current].getCharSource()) {
                bottom = current;
            } else {
                top = current;
            }
            current = (top + bottom) / 2;
        }
        if (ch == map[current].getCharSource()) return current;
        else return -1;
    }


    void setCharSource(char ch) {
        chSource = ch;
    }

    void setLowerCaseMap(char[] map) {
        lowerCaseMap = map;
    }

    void setUpperCaseMap(char[] map) {
        upperCaseMap = map;
    }

    void setTitleCaseMap(char[] map) {
        titleCaseMap = map;
    }

    void setLocale(Locale locale) {
        this.locale = locale;
    }

    void setContext(String[] context) {
        this.context = context;
    }

    public char getCharSource() {
        return chSource;
    }

    public char[] getLowerCaseMap() {
        return lowerCaseMap;
    }

    public char[] getUpperCaseMap() {
        return upperCaseMap;
    }

    public char[] getTitleCaseMap() {
        return titleCaseMap;
    }

    public Locale getLocale() {
        return locale;
    }

    public String[] getContext() {
        return context;
    }


    char chSource;
    Locale locale;
    char[] lowerCaseMap;
    char[] upperCaseMap;
    char[] titleCaseMap;
    String[] context;

    /**
     * Fields that can be found in the SpecialCasing.txt file.
     */
    static int REQUIRED_FIELDS = 5;
    static int FIELD_SOURCE = 0;
    static int FIELD_LOWERCASE = 1;
    static int FIELD_TITLECASE = 2;
    static int FIELD_UPPERCASE = 3;
    static int FIELD_CONDITIONS = 4;

    /**
     * Context values
     */
    static String CONTEXT_FINAL = "FINAL";
    static String CONTEXT_NONFINAL = "NON_FINAL";
    static String CONTEXT_MODERN = "MODERN";
    static String CONTEXT_NONMODERN = "NON_MODERN";

    public int compare(Object o1, Object o2) {
        if (((SpecialCaseMap)o1).chSource < ((SpecialCaseMap)o2).chSource) {
            return -1;
        }
        else if (((SpecialCaseMap)o1).chSource > ((SpecialCaseMap)o2).chSource) {
            return 1;
        }
        else return 0;
    }

    public boolean equals(Object o1) {
        return super.equals(o1);
    }
}
