/*
 * @(#)SourceToHTMLConverter.java	1.12 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.tools.doclets;

import com.sun.javadoc.*;
import java.io.*;
import java.util.*;

/**
 * Converts Java Source Code to HTML.
 * @author Jamie Ho
 * @since JDK 1.4
 */

public class SourceToHTMLConverter {

    /**
     * The background color.
     */
     protected static final String BGCOLOR = "white";

    /**
     * The line number color.
     */
     protected static final String LINE_NO_COLOR = "green";
    
    /**
     * The number of trailing blank lines at the end of the page.
     * This is inserted so that anchors at the bottom of small pages
     * can be reached.
     */
     protected static final int NUM_BLANK_LINES = 60;

    /**
     * Convert the Classes in the given RootDoc to an HTML.
     * @param configuration the configuration
     * @param rd the RootDoc to convert
     * @param outputdir the name of the directory to output to.
     */
    public static void convertRoot(Configuration configuration, RootDoc rd, String outputdir) {
        if (rd == null || outputdir == null) {
            return;
        }
        PackageDoc[] pds = rd.specifiedPackages();
        for (int i = 0; i < pds.length; i++) {
            convertPackage(configuration, pds[i], outputdir);
        }
        ClassDoc[] cds = rd.specifiedClasses();
        for (int i = 0; i < cds.length; i++) {
            convertClass(configuration, cds[i], outputdir
                + File.separator
                + getPackagePath(cds[i].containingPackage()));
        }
    }
    
    /**
     * Convert the Classes in the given Package to an HTML
     * @param configuration the configuration
     * @param pd the Package to convert
     * @param outputdir the name of the directory to output to.
     */
    public static void convertPackage(Configuration configuration, PackageDoc pd, String outputdir) {
        if (pd == null || outputdir == null) {
            return;
        }
        outputdir += File.separator + getPackagePath(pd);
        ClassDoc[] cds = pd.allClasses();
        for (int i = 0; i < cds.length; i++) {
            convertClass(configuration, cds[i], outputdir);
        }
    }


    /**
     * Given a string, replace all tabs with the appropriate
     * number of spaces.
     * @param configuration the current configuration of the doclet.
     * @param s the String to scan.
     */
    private static void replaceTabs(Configuration configuration, StringBuffer s) {
        int index;
        int col;
        StringBuffer whitespace;
        while ((index = s.indexOf("\t")) != -1) {
            whitespace = new StringBuffer();
            col = index;
            do {
                whitespace.append(" ");
                col++;
            } while ((col%configuration.linksourcetab) != 0);
            s.replace(index, index+1, whitespace.toString());
        }
    }

    /**
     * Convert the given Class to an HTML
     * @param configuration the configuration
     * @param cd the class to convert
     * @param outputdir the name of the directory to output to.
     */
    public static void convertClass(Configuration configuration, ClassDoc cd, String outputdir) {
        if (cd == null || outputdir == null) {
            return;
        }
        SourcePosition sp = cd.position();
        if (sp == null) {
            return;
        }
        File file = sp.file();
        if (file == null) {
            return;
        }
        try {
            LineNumberReader reader = new LineNumberReader(new FileReader(file));
            int lineno = 1;
            String line;
            StringBuffer sb = new StringBuffer();
            StringBuffer lineBuffer;
            while ((line = reader.readLine()) != null){
                lineBuffer = new StringBuffer();
                //Put the anchor one line above
                lineBuffer.append(format(line));
                lineBuffer.append("<a name=\"line." + Integer.toString(lineno) + "\"></a>");
                lineBuffer.append("\n");
                replaceTabs(configuration, lineBuffer);
                sb.append(lineBuffer.toString());
                lineno++;
            }
            sb = addLineNumbers(sb.toString());
            sb.insert(0, getHeader());
            sb.append(getFooter());
            File dir = new File(outputdir);
            dir.mkdirs();
            File newFile = new File(dir, cd.name() + ".html");
            configuration.message.notice("doclet.Generating_0", newFile.getPath());
            FileOutputStream fout = new FileOutputStream(newFile);
            BufferedWriter bw = new BufferedWriter(new OutputStreamWriter(fout));
            bw.write(sb.toString());
            bw.close();
            fout.close();
        } catch (Exception e){
            e.printStackTrace();
        }
    }

    /**
     * Given a <code>String</code>, add line numbers.
     * @param sb the text to add line numbers to.
     */
     protected static StringBuffer addLineNumbers(String s) {
         StringBuffer sb = new StringBuffer();
         StringTokenizer st = new StringTokenizer(s, "\n", true);
         int lineno = 1;
         String current;
         while(st.hasMoreTokens()){
             current = st.nextToken();
             if (! current.equals("\n")) {
                 sb.append(getHTMLLineNo(lineno) + current + st.nextToken());
             } else {
                 sb.append(getHTMLLineNo(lineno) + current);
             }
             lineno++;
         }
         return sb;
    }
    
    /**
     * Get the package path
     * @param pd the package to get the path for.
     * @return a path for the package
     */
    public static String getPackagePath(PackageDoc pd) {
        if (pd == null) {
            return "";
        }
        StringTokenizer st = new StringTokenizer(pd.name(), ".");
        String result = "";
        while (st.hasMoreTokens()) {
            result += st.nextToken() + File.separator;
        }
        return result;
    }
    
    /**
     * Get the header
     * @return the header to the output file
     */
    protected static String getHeader() {
        String result = "<HTML>\n";
        result += "<BODY BGCOLOR=\""+ BGCOLOR + "\">\n";
        result += "<PRE>\n";
        return result;
    }
    
    /**
     * Get the footer
     * @return the footer to the output file
     */
    protected static String getFooter() {
        StringBuffer footer = new StringBuffer();
        for (int i = 0; i < NUM_BLANK_LINES; i++) {
            footer.append("\n");
        }
        footer.append("</PRE>\n</BODY>\n</HTML>\n");
        return footer.toString();
    }

    /**
     * Get the HTML for the lines.
     * @param lineno The line number
     * @return the HTML code for the line
     */
    protected static String getHTMLLineNo(int lineno) {
        StringBuffer result = new StringBuffer("<FONT color=\""
                                  + LINE_NO_COLOR + "\">");
        if (lineno < 10) {
            result.append("00" + ((new Integer(lineno)).toString()));
        } else if (lineno < 100) {
            result.append("0" + ((new Integer(lineno)).toString()));
        } else {
            result.append((new Integer(lineno)).toString());
        }
        result.append("</FONT>    ");
        return result.toString();
    }

    /**
     * Replace every '<' character in comments with "&lt;".
     * @param str the string to format.
     */
    protected static String format(String str) {
        if (str == null) {
            return null;
        }
        StringBuffer sb = new StringBuffer(str);
        int next = -1;
        while ((next = sb.indexOf("<", next + 1)) != -1) {
            sb.replace(next, next + 1, "&lt;");
        }
        return sb.toString();
    }
    
    /**
     * Given an array of <code>Doc</code>s, add to the given <code>HashMap</code> the
     * line numbers and anchors that should be inserted in the output at those lines.
     * @param docs the array of <code>Doc</code>s to add anchors for.
     * @param hash the <code>HashMap</code> to add to.
     */
    
    protected static void addToHash(Doc[] docs, HashMap hash) {
        if(docs == null) {
            return;
        }
        for(int i = 0; i < docs.length; i++) {
            hash.put(new Integer(docs[i].position().line()), getAnchor(docs[i]));
        }
    }
    
    /**
     * Given a <code>Doc</code>, return an anchor for it.
     * @param d the <code>Doc</code> to check.
     * @return an anchor of the form &lt;a name="my_name">&lt;/a>
     */
    protected static String getAnchor(Doc d) {
        return "    <a name=\"" +
               getAnchorName(d) +
               "\"></a>";
    }
        
    /**
     * Given a <code>Doc</code>, return an anchor name for it.
     * @param d the <code>Doc</code> to check.
     * @return the name of the anchor
     */
    public static String getAnchorName(Doc d) {
        return "line." + d.position().line();
    }
}

