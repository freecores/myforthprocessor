/*
 * @(#)HtmlStandardWriter.java	1.137 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.tools.doclets.standard;

import com.sun.tools.doclets.*;
import com.sun.tools.doclets.standard.tags.*;
import com.sun.javadoc.*;
import java.io.*;
import java.util.*;
import java.lang.*;
import java.text.MessageFormat;


/**
 * Class for the Html Format Code Generation specific to JavaDoc.
 * This Class contains methods related to the Html Code Generation which
 * are used extensively while generating the entire documentation.
 *
 * @since JDK1.2
 * @author Atul M Dambalkar
 * @author Robert Field
 */
public class HtmlStandardWriter extends HtmlDocWriter {
    
    /**
     * Relative path from the file getting generated to the destination
     * directory. For example, if the file getting generated is
     * "java/lang/Object.html", then the relative path string is "../../".
     * This string can be empty if the file getting generated is in
     * the destination directory.
     */
    public String relativepath = "";
    
    /**
     * Same as relativepath, but normalized to never be empty or
     * end with a slash.
     */
    public String relativepathNoSlash = "";
    
    /**
     * Platform-dependent directory path from the current or the
     * destination directory to the file getting generated.
     * Used when creating the file.
     * For example, if the file getting generated is
     * "java/lang/Object.html", then the path string is "java/lang".
     */
    public String path = "";
    
    /**
     * Name of the file getting generated. If the file getting generated is
     * "java/lang/Object.html", then the filename is "Object.html".
     */
    public String filename = "";
    
    /**
     * Relative path from the destination directory to the current directory.
     * For example if the destination directory is "core/api/docs", then the
     * backpath string will be "../../".
     */
    public final String backpath;
    
    /**
     * The display length used for indentation while generating the class page.
     */
    public int displayLength = 0;
    
    /**
     * The global configuration information for this run.
     */
    public ConfigurationStandard configuration;
    
    /**
     * The name of the doc files directory.
     */
    public static final String DOC_FILES_DIR_NAME = "doc-files";
    
    /**
     * Constructor to construct the HtmlStandardWriter object.
     *
     * @param filename File to be generated.
     */
    public HtmlStandardWriter(ConfigurationStandard configuration,
                              String filename) throws IOException {
        super(configuration, filename);
        this.configuration = configuration;
        this.backpath = DirectoryManager.getBackPath(configuration.destdirname);
        this.filename = filename;
    }
    
    /**
     * Constructor to construct the HtmlStandardWriter object.
     *
     * @param path         Platform-dependent {@link #path} used when
     *                     creating file.
     * @param filename     Name of file to be generated.
     * @param relativepath Value for the variable {@link #relativepath}.
     */
    public HtmlStandardWriter(ConfigurationStandard configuration,
                              String path, String filename,
                              String relativepath) throws IOException {
        super(configuration, path, filename);
        this.configuration = configuration;
        this.backpath = DirectoryManager.getBackPath(configuration.destdirname);
        this.path = path;
        this.relativepath = relativepath;
        this.relativepathNoSlash =
            DirectoryManager.getPathNoTrailingSlash(this.relativepath);
        this.filename = filename;
    }
    
    /**
     * Copy the given directory contents from the source package directory
     * to the generated documentation directory. For example for a package
     * java.lang this method find out the source location of the package using
     * {@link SourcePath} and if given directory is found in the source
     * directory structure, copy the entire directory, to the generated
     * documentation hierarchy.
     *
     * @param srcDir The original directory to copy from.
     * @param path The relative path to the directory to be copied.
     * @param overwrite Overwrite files if true.
     * @throws DocletAbortException
     */
    public static void copyDocFiles(ConfigurationStandard configuration,
            String srcDirName, String path, boolean overwrite) {
        
        String destname = configuration.destdirname;
        File sourcePath, destPath = new File(destname);
        StringTokenizer pathTokens = new StringTokenizer(
            configuration.sourcepath == null || configuration.sourcepath.length() == 0 ?
            ".": configuration.sourcepath, ":");
        try {
            while(pathTokens.hasMoreTokens()){
                sourcePath = new File(pathTokens.nextToken());
                if(destPath.getCanonicalPath().equals(sourcePath.getCanonicalPath())){
                    return;
                }
            }
        } catch (IOException e) {
            e.printStackTrace();
        }
        File srcdir = new File(srcDirName + path);
        if (! srcdir.exists()) {
            return;
        }
        if (destname.length() > 0 && !destname.endsWith(fileseparator)) {
            destname += fileseparator;
        }
        String dest = destname + path;
        try {
            File destdir = new File(dest);
            DirectoryManager.createDirectory(configuration, dest);
            String[] files = srcdir.list();
            for (int i = 0; i < files.length; i++) {
                File srcfile = new File(srcdir, files[i]);
                File destfile = new File(destdir, files[i]);
                if (srcfile.isFile()) {
                    if(destfile.exists() && ! overwrite) {
                        configuration.standardmessage.warning((SourcePosition) null,
                                                              "doclet.Copy_Overwrite_warning",
                                                              srcfile.toString(), destdir.toString());
                    } else {
                        configuration.standardmessage.notice("doclet.Copying_File_0_To_Dir_1",
                                                             srcfile.toString(), destdir.toString());
                        Util.copyFile(destfile, srcfile);
                    }
                } else if(srcfile.isDirectory()) {
                    if(configuration.copydocfilesubdirs
                       && ! configuration.shouldExcludeDocFileDir(srcfile.getName())){
                        copyDocFiles(configuration, srcDirName,
                                     path + fileseparator + srcfile.getName(), overwrite);
                    }
                }
            }
        } catch (SecurityException exc) {
            throw new DocletAbortException();
        } catch (IOException exc) {
            throw new DocletAbortException();
        }
    }
    
    /**
     * Given a PackageDoc, return the source path for that package.
     * @param pkgDoc The package to seach the path for.
     * @return A string representing the path to the given package.
     */
    protected static String getSourcePath(Configuration configuration,
                                          PackageDoc pkgDoc){
        
        try{
            String pkgPath = DirectoryManager.getDirectoryPath(pkgDoc);
            String completePath = new SourcePath(configuration.sourcepath).getDirectory(pkgPath)
                + fileseparator;
            return completePath.substring(0, completePath.indexOf(pkgPath));
        } catch (Exception e){
            return "";
        }
    }
    
    /**
     * Replace {&#064;docRoot} tag used in options that accept HTML text, such
     * as -header, -footer and -bottom, and when converting a relative
     * HREF where commentTagsToString inserts a {&#064;docRoot} where one was
     * missing.  (Also see DocRootTaglet for {&#064;docRoot} tags in doc
     * comments.)
     * <p>
     * Replace {&#064;docRoot} tag in htmlstr with the relative path to the
     * destination directory from the directory where the file is being
     * written, looping to handle all such tags in htmlstr.
     * <p>
     * For example, for "-d docs" and -header containing {&#064;docRoot}, when
     * the HTML page for source file p/C1.java is being generated, the
     * {&#064;docRoot} tag would be inserted into the header as "../",
     * the relative path from docs/p/ to docs/ (the document root).
     * <p>
     * Note: This doc comment was written with '&amp;#064;' representing '@'
     * to prevent the inline tag from being interpreted.
     */
    public String replaceDocRootDir(String htmlstr) {
        // Return if no inline tags exist
        int index = htmlstr.indexOf("{@");
        if (index < 0) {
            return htmlstr;
        }
        String lowerHtml = htmlstr.toLowerCase();
        // Return index of first occurrence of {@docroot}
        // Note: {@docRoot} is not case sensitive when passed in w/command line option
        index = lowerHtml.indexOf("{@docroot}", index);
        if (index < 0) {
            return htmlstr;
        }
        StringBuffer buf = new StringBuffer();
        int previndex = 0;
        while (true) {
            // Search for lowercase version of {@docRoot}
            index = lowerHtml.indexOf("{@docroot}", previndex);
            // If next {@docRoot} tag not found, append rest of htmlstr and exit loop
            if (index < 0) {
                buf.append(htmlstr.substring(previndex));
                break;
            }
            // If next {@docroot} tag found, append htmlstr up to start of tag
            buf.append(htmlstr.substring(previndex, index));
            previndex = index + 10;  // length for {@docroot} string
            // Insert relative path where {@docRoot} was located
            buf.append(relativepathNoSlash);
            // Append slash if next character is not a slash
            if (previndex < htmlstr.length() && htmlstr.charAt(previndex) != '/') {
                buf.append(DirectoryManager.urlfileseparator);
            }
        }
        return buf.toString();
    }
    
    /**
     * Print Html Hyper Link, with target frame.
     *
     * @param link String name of the file.
     * @param where Position in the file
     * @param target Name of the target frame.
     * @param label Tag for the link.
     * @param bold Whether the label should be bold or not?
     */
    public void printTargetHyperLink(String link, String where,
                                     String target, String label,
                                     boolean bold, String title) {
        print(getHyperLink(link, where, label, bold, "", title, target));
    }
    
    /**
     * Get Html Hyper Link, with target frame.  This
     * link will only appear if page is not in a frame.
     *
     * @param link String name of the file.
     * @param where Position in the file
     * @param target Name of the target frame.
     * @param label Tag for the link.
     * @param bold Whether the label should be bold or not?
     */
    public void printNoFramesTargetHyperLink(String link, String where,
                                               String target, String label,
                                               boolean bold) {
        script();
        println("  <!--");
        println("  if(window==top) {");
        println("    document.writeln('" + getHyperLink(link, where, label, bold, "", "", target) + "');");
        println("  }");
        println("  //-->");
        scriptEnd();
        println("<NOSCRIPT>");
        println("  " + getHyperLink(link, where, label, bold, "", "", target));
        println("</NOSCRIPT>\n");
    }
    
    /**
     * Print Html Hyper Link, with target frame.
     *
     * @param link String name of the file.
     * @param target Name of the target frame.
     * @param label Tag for the link.
     * @param bold Whether the label should be bold or not?
     */
    public void printTargetHyperLink(String link, String target,
                                     String label, boolean bold) {
        printTargetHyperLink(link, "", target, label, bold, "");
    }
    
    
    /**
     * Print bold Html Hyper Link, with target frame. The label will be bold.
     *
     * @param link String name of the file.
     * @param target Name of the target frame.
     * @param label Tag for the link.
     */
    public void printBoldTargetHyperLink(String link, String target,
                                         String label) {
        printTargetHyperLink(link, target, label, true);
    }
    
    /**
     * Print bold Html Hyper Link, with target frame. The label will be bold
     * and the link will only show up if the page is not in a frame.
     *
     * @param link String name of the file.
     * @param target Name of the target frame.
     * @param label Tag for the link.
     */
    public void printNoFramesBoldTargetHyperLink(String link, String target,
                                                 String label) {
        printNoFramesTargetHyperLink(link, "", target, label, true);
    }
    
    
    /**
     * Print Html Hyper Link, with target frame.
     *
     * @param link String name of the file.
     * @param target Name of the target frame.
     * @param label Tag for the link.
     */
    public void printTargetHyperLink(String link, String target,
                                     String label) {
        printTargetHyperLink(link, "", target, label, false, "");
    }
    
    /**
     * Print Class link, with target frame.
     *
     * @param cd The class to which link is.
     * @param target Name of the target frame.
     */
    public void printTargetClassLink(ClassDoc cd, String target) {
        String filename = cd.name() + ".html";
        String title = getText(cd.isInterface() ?
                                   "doclet.Href_Interface_Title" : "doclet.Href_Class_Title", cd.containingPackage().name());
        printTargetHyperLink(filename, "", target,
                                 (cd.isInterface())?
                                 italicsText(cd.name()): cd.name(), false, title);
    }
    
    /**
     * Print Package link, with target frame.
     *
     * @param pd The link will be to the "package-summary.html" page for this
     * package.
     * @param target Name of the target frame.
     * @param label Tag for the link.
     */
    public void printTargetPackageLink(PackageDoc pd, String target,
                                       String label) {
        printTargetHyperLink(pathString(pd, "package-summary.html"),
                             target, label);
    }
    
    /**
     * Print the html file header using no keywords for the META tag.
     *
     * @param title String title for the generated html file.
     */
    public void printHtmlHeader(String title) {
        printHtmlHeader(title, null, true);
    }
    
    /**
     * Print the html file header. Also print Html page title and stylesheet
     * default properties.
     *
     * @param title         String window title to go in the &lt;TITLE&gt; tag
     * @param metakeywords  Array of String keywords for META tag.  Each element
     *                      of the array is assigned to a separate META tag.
     *                      Pass in null for no array.
     */
    public void printHtmlHeader(String title, String[] metakeywords) {
        printHtmlHeader(title, metakeywords, true);
    }

    /**
     * Print the html file header. Also print Html page title and stylesheet
     * default properties.
     *
     * @param title         String window title to go in the &lt;TITLE&gt; tag
     * @param metakeywords  Array of String keywords for META tag.  Each element
     *                      of the array is assigned to a separate META tag.
     *                      Pass in null for no array.
     * @param includeScript boolean true if printing windowtitle script.  
     *             False for files that appear in the left-hand frames.
     */
    public void printHtmlHeader(String title, String[] metakeywords, boolean includeScript) {
        println("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 " +
                    "Transitional//EN\" " +
                    "\"http://www.w3.org/TR/html4/loose.dtd\">");
        println("<!--NewPage-->");
        html();
        head();
        if (! configuration.notimestamp) {
            print("<!-- Generated by javadoc (build " + Standard.BUILD_DATE + ") on ");
            print(today());
            println(" -->");
        }
        if (configuration.charset.length() > 0) {
            println("<META http-equiv=\"Content-Type\" content=\"text/html; "
                        + "charset=" + configuration.charset + "\">");
        }
        if ( configuration.windowtitle.length() > 0 ) {
            title += " (" + configuration.windowtitle  + ")";
        }
        title(title);
        println(title);
        titleEnd();
        println("");
        if ( metakeywords != null ) {
            for ( int i=0; i < metakeywords.length; i++ ) {
                println("<META NAME=\"keywords\" "
                            + "CONTENT=\"" + metakeywords[i] + "\">");
            }
        }
        println("");
        printStyleSheetProperties();
        println("");
        // Don't print windowtitle script for overview-frame, allclasses-frame 
        // and package-frame
        if (includeScript) {
            printWinTitleScript(title);
        }
        println("");
        headEnd();
        println("");
        body("white", includeScript);
    }
    
    /**
     * Print user specified header and the footer.
     *
     * @param header if true print the user provided header else print the
     * user provided footer.
     */
    public void printUserHeaderFooter(boolean header) {
        em();
        if (header) {
            print(replaceDocRootDir(configuration.header));
        } else {
            if (configuration.footer.length() != 0) {
                print(replaceDocRootDir(configuration.footer));
            } else {
                print(replaceDocRootDir(configuration.header));
            }
        }
        emEnd();
    }
    
    /**
     * Print the user specified bottom.
     */
    public void printBottom() {
        hr();
        print(replaceDocRootDir(configuration.bottom));
    }
    
    /**
     * Print the navigation bar for the Html page at the top and and the bottom.
     *
     * @param header If true print navigation bar at the top of the page else
     * print the nevigation bar at the bottom.
     */
    protected void navLinks(boolean header) {
        println("");
        if (!configuration.nonavbar) {
            if (header) {
                println("\n<!-- ========= START OF TOP NAVBAR ======= -->");
                anchor("navbar_top");
                print("\n");
                print(getHyperLink("", "skip-navbar_top", "", false, "", 
                    getText("doclet.Skip_navigation_links"), ""));
            } else {
                println("\n<!-- ======= START OF BOTTOM NAVBAR ====== -->");
                anchor("navbar_bottom");
                print("\n");
                print(getHyperLink("", "skip-navbar_bottom", "", false, "", 
                    getText("doclet.Skip_navigation_links"), ""));
            }
            table(0, "100%", 1, 0);
            tr();
            tdColspanBgcolorStyle(3, "#EEEEFF", "NavBarCell1");
            println("");
            if (header) {
                anchor("navbar_top_firstrow");
            } else {
                anchor("navbar_bottom_firstrow");
            }
            table(0, 0, 3);
            print("  ");
            trAlignVAlign("center", "top");
            
            if (configuration.createoverview) {
                navLinkContents();
            }
            
            if (configuration.packages.length > 0) {
                navLinkPackage();
            }
            
            navLinkClass();
            
            if(configuration.classuse) {
                navLinkClassUse();
            }
            if(configuration.createtree) {
                navLinkTree();
            }
            if(!(configuration.nodeprecated ||
                     configuration.nodeprecatedlist)) {
                navLinkDeprecated();
            }
            if(configuration.createindex) {
                navLinkIndex();
            }
            if (!configuration.nohelp) {
                navLinkHelp();
            }
            print("  ");
            trEnd();
            tableEnd();
            tdEnd();
            
            tdAlignVAlignRowspan("right", "top", 3);
            
            printUserHeaderFooter(header);
            tdEnd();
            trEnd();
            println("");
            
            tr();
            tdBgcolorStyle("white", "NavBarCell2");
            font("-2");
            space();
            navLinkPrevious();
            space();
            println("");
            space();
            navLinkNext();
            fontEnd();
            tdEnd();
            
            tdBgcolorStyle("white", "NavBarCell2");
            font("-2");
            print("  ");
            navShowLists();
            print("  ");
            space();
            println("");
            space();
            navHideLists();
            print("  ");
            space();
            println("");
            space();
            navLinkClassIndex();
            fontEnd();
            tdEnd();
            
            trEnd();
            
            printSummaryDetailLinks();
            
            tableEnd();
            if (header) {
                aName("skip-navbar_top");
                aEnd();
                println("\n<!-- ========= END OF TOP NAVBAR ========= -->");
            } else {
                aName("skip-navbar_bottom");
                aEnd();
                println("\n<!-- ======== END OF BOTTOM NAVBAR ======= -->");
            }
            println("");
        }
    }
    
    /**
     * Do nothing. This is the default method.
     */
    protected void printSummaryDetailLinks() {
    }
    
    /**
     * Print link to the "overview-summary.html" page.
     */
    protected void navLinkContents() {
        navCellStart();
        printHyperLink(relativepath + "overview-summary.html", "",
                       getText("doclet.Overview"), true, "NavBarFont1");
        navCellEnd();
    }
    
    /**
     * Description for a cell in the navigation bar.
     */
    protected void navCellStart() {
        print("  ");
        tdBgcolorStyle("#EEEEFF", "NavBarCell1");
        print("    ");
    }
    
    /**
     * Description for a cell in the navigation bar, but with reverse
     * high-light effect.
     */
    protected void navCellRevStart() {
        print("  ");
        tdBgcolorStyle("#FFFFFF", "NavBarCell1Rev");
        print(" ");
        space();
    }
    
    /**
     * Closing tag for navigation bar cell.
     */
    protected void navCellEnd() {
        space();
        tdEnd();
    }
    
    /**
     * Print link to the "package-summary.html" page for the package passed.
     *
     * @param pkg Package to which link will be generated.
     */
    protected void navLinkPackage(PackageDoc pkg) {
        printPackageLink(pkg, getFontColor("NavBarFont1") + getBold() +
                             getText("doclet.Package") +
                             getBoldEnd() + getFontEnd());
    }
    
    /**
     * Print the word "Package" in the navigation bar cell, to indicate that
     * link is not available here.
     */
    protected void navLinkPackage() {
        navCellStart();
        fontStyle("NavBarFont1");
        printText("doclet.Package");
        fontEnd();
        navCellEnd();
    }
    
    /**
     * Print the word "Use" in the navigation bar cell, to indicate that link
     * is not available.
     */
    protected void navLinkClassUse() {
        navCellStart();
        fontStyle("NavBarFont1");
        printText("doclet.navClassUse");
        fontEnd();
        navCellEnd();
    }
    
    /**
     * Print link for previous file.
     *
     * @param prev File name for the prev link.
     */
    public void navLinkPrevious(String prev) {
        String tag = getText("doclet.Prev");
        if (prev != null) {
            printHyperLink(prev, "", tag, true) ;
        } else {
            print(tag);
        }
    }
    
    /**
     * Print the word "PREV" to indicate that no link is available.
     */
    protected void navLinkPrevious() {
        navLinkPrevious(null);
    }
    
    /**
     * Print link for next file.
     *
     * @param next File name for the next link.
     */
    public void navLinkNext(String next) {
        String tag = getText("doclet.Next");
        if (next != null) {
            printHyperLink(next, "", tag, true);
        } else {
            print(tag);
        }
    }
    
    /**
     * Print the word "NEXT" to indicate that no link is available.
     */
    protected void navLinkNext() {
        navLinkNext(null);
    }
    
    /**
     * Print "FRAMES" link, to switch to the frame version of the output.
     *
     * @param link File to be linked, "index.html".
     */
    protected void navShowLists(String link) {
        printBoldTargetHyperLink(link, "_top", getText("doclet.FRAMES"));
    }
    
    /**
     * Print "FRAMES" link, to switch to the frame version of the output.
     */
    protected void navShowLists() {
        navShowLists(relativepath + "index.html");
    }
    
    /**
     * Print "NO FRAMES" link, to switch to the non-frame version of the output.
     *
     * @param link File to be linked.
     */
    protected void navHideLists(String link) {
        printBoldTargetHyperLink(link, "_top", getText("doclet.NO_FRAMES"));
    }
    
    /**
     * Print "NO FRAMES" link, to switch to the non-frame version of the output.
     */
    protected void navHideLists() {
        navHideLists(filename);
    }
    
    /**
     * Print "Tree" link in the navigation bar. If there is only one package
     * specified on the command line, then the "Tree" link will be to the
     * only "package-tree.html" file otherwise it will be to the
     * "overview-tree.html" file.
     */
    protected void navLinkTree() {
        navCellStart();
        PackageDoc[] packages = configuration.root.specifiedPackages();
        if (packages.length == 1 && configuration.root.specifiedClasses().length == 0) {
            printHyperLink(pathString(packages[0], "package-tree.html"), "",
                           getText("doclet.Tree"), true, "NavBarFont1");
        } else {
            printHyperLink(relativepath + "overview-tree.html", "",
                           getText("doclet.Tree"), true, "NavBarFont1");
        }
        navCellEnd();
    }
    
    /**
     * Print "Tree" link to the "overview-tree.html" file.
     */
    protected void navLinkMainTree(String label) {
        printHyperLink(relativepath + "overview-tree.html", label);
    }
    
    /**
     * Print the word "Class" in the navigation bar cell, to indicate that
     * class link is not available.
     */
    protected void navLinkClass() {
        navCellStart();
        fontStyle("NavBarFont1");
        printText("doclet.Class");
        fontEnd();
        navCellEnd();
    }
    
    /**
     * Print "Deprecated" API link in the navigation bar.
     */
    protected void navLinkDeprecated() {
        navCellStart();
        printHyperLink(relativepath + "deprecated-list.html", "",
                       getText("doclet.navDeprecated"), true, "NavBarFont1");
        navCellEnd();
    }
    
    /**
     * Print link for generated index. If the user has used "-splitindex"
     * command line option, then link to file "index-files/index-1.html" is
     * generated otherwise link to file "index-all.html" is generated.
     */
    protected void navLinkClassIndex() {
        printNoFramesBoldTargetHyperLink(relativepath + AllClassesFrameWriter.OUTPUT_FILE_NAME_NOFRAMES,
                                         "", getText("doclet.All_Classes"));
    }
    /**
     * Print link for generated class index.
     */
    protected void navLinkIndex() {
        navCellStart();
        printHyperLink(relativepath +
                           (configuration.splitindex?
                                DirectoryManager.getPath("index-files") +
                                fileseparator: "") +
                           (configuration.splitindex?
                                "index-1.html" : "index-all.html"), "",
                       getText("doclet.Index"), true, "NavBarFont1");
        navCellEnd();
    }
    
    /**
     * Print help file link. If user has provided a help file, then generate a
     * link to the user given file, which is already copied to current or
     * destination directory.
     */
    protected void navLinkHelp() {
        String helpfilenm = configuration.helpfile;
        if (helpfilenm.equals("")) {
            helpfilenm = "help-doc.html";
        } else {
            int lastsep;
            if ((lastsep = helpfilenm.lastIndexOf(File.separatorChar)) != -1) {
                helpfilenm = helpfilenm.substring(lastsep + 1);
            }
        }
        navCellStart();
        printHyperLink(relativepath + helpfilenm, "",
                       getText("doclet.Help"), true, "NavBarFont1");
        navCellEnd();
    }
    
    /**
     * Print the word "Detail" in the navigation bar. No link is available.
     */
    protected void navDetail() {
        printText("doclet.Detail");
    }
    
    /**
     * Print the word "Summary" in the navigation bar. No link is available.
     */
    protected void navSummary() {
        printText("doclet.Summary");
    }
    
    /**
     * Print the Html table tag for the index summary tables. The table tag
     * printed is
     * &lt;TABLE BORDER="1" CELLPADDING="3" CELLSPACING="0" WIDTH="100%">
     */
    public void tableIndexSummary() {
        table(1, "100%", 3, 0);
    }
    
    /**
     * Same as {@link #tableIndexSummary()}.
     */
    public void tableIndexDetail() {
        table(1, "100%", 3, 0);
    }
    
    /**
     * Print Html tag for table elements. The tag printed is
     * &lt;TD ALIGN="right" VALIGN="top" WIDTH="1%"&gt;.
     */
    public void tdIndex() {
        print("<TD ALIGN=\"right\" VALIGN=\"top\" WIDTH=\"1%\">");
    }
    
    /**
     * Prine table header information about color, column span and the font.
     *
     * @param color Background color.
     * @param span  Column span.
     */
    public void tableHeaderStart(String color, int span) {
        trBgcolorStyle(color, "TableHeadingColor");
        tdColspan(span);
        font("+2");
    }
    
    /**
     * Print table header for the inherited members summary tables. Print the
     * background color information.
     *
     * @param color Background color.
     */
    public void tableInheritedHeaderStart(String color) {
        trBgcolorStyle(color, "TableSubHeadingColor");
        td();
    }
    
    /**
     * Print "Use" table header. Print the background color and the column span.
     *
     * @param color Background color.
     */
    public void tableUseInfoHeaderStart(String color) {
        trBgcolorStyle(color, "TableSubHeadingColor");
        tdColspan(2);
    }
    
    /**
     * Print table header with the background color with default column span 2.
     *
     * @param color Background color.
     */
    public void tableHeaderStart(String color) {
        tableHeaderStart(color, 2);
    }
    
    /**
     * Print table header with the column span, with the default color #CCCCFF.
     *
     * @param span Column span.
     */
    public void tableHeaderStart(int span) {
        tableHeaderStart("#CCCCFF", span);
    }
    
    /**
     * Print table header with default column span 2 and default color #CCCCFF.
     */
    public void tableHeaderStart() {
        tableHeaderStart(2);
    }
    
    /**
     * Print table header end tags for font, column and row.
     */
    public void tableHeaderEnd() {
        fontEnd();
        tdEnd();
        trEnd();
    }
    
    /**
     * Print table header end tags in inherited tables for column and row.
     */
    public void tableInheritedHeaderEnd() {
        tdEnd();
        trEnd();
    }
    
    /**
     * Print the summary table row cell attribute width.
     *
     * @param width Width of the table cell.
     */
    public void summaryRow(int width) {
        if (width != 0) {
            tdWidth(width + "%");
        } else {
            td();
        }
    }
    
    /**
     * Print the summary table row cell end tag.
     */
    public void summaryRowEnd() {
        tdEnd();
    }
    
    /**
     * Print the heading in Html &lt;H2> format.
     *
     * @param str The Header string.
     */
    public void printIndexHeading(String str) {
        h2();
        print(str);
        h2End();
    }
    
    /**
     * Print Html tag &lt;FRAMESET=arg&gt;.
     *
     * @param arg Argument for the tag.
     */
    public void frameSet(String arg) {
        println("<FRAMESET " + arg + ">");
    }
    
    /**
     * Print Html closing tag &lt;/FRAMESET&gt;.
     */
    public void frameSetEnd() {
        println("</FRAMESET>");
    }
    
    /**
     * Print Html tag &lt;FRAME=arg&gt;.
     *
     * @param arg Argument for the tag.
     */
    public void frame(String arg) {
        println("<FRAME " + arg + ">");
    }
    
    /**
     * Print Html closing tag &lt;/FRAME&gt;.
     */
    public void frameEnd() {
        println("</FRAME>");
    }
    
    /**
     * Return path to the class page for a classdoc. For example, the class
     * name is "java.lang.Object" and if the current file getting generated is
     * "java/io/File.html", then the path string to the class, returned is
     * "../../java/lang.Object.html".
     *
     * @param cd Class to which the path is requested.
     */
    protected String pathToClass(ClassDoc cd) {
        return pathString(cd.containingPackage(), cd.name() + ".html");
    }
    
    /**
     * Return the path to the class page for a classdoc. Works same as
     * {@link #pathToClass(ClassDoc)}.
     *
     * @param cd   Class to which the path is requested.
     * @param name Name of the file(doesn't include path).
     */
    protected String pathString(ClassDoc cd, String name) {
        return pathString(cd.containingPackage(), name);
    }
    
    /**
     * Return path to the given file name in the given package. So if the name
     * passed is "Object.html" and the name of the package is "java.lang", and
     * if the relative path is "../.." then returned string will be
     * "../../java/lang/Object.html"
     *
     * @param pd Package in which the file name is assumed to be.
     * @param name File name, to which path string is.
     */
    protected String pathString(PackageDoc pd, String name) {
        StringBuffer buf = new StringBuffer(relativepath);
        buf.append(DirectoryManager.getPathToPackage(pd, name));
        return buf.toString();
    }
    
    /**
     * Print link to the "pacakge-summary.html" file, depending upon the
     * package name.
     */
    public void printPackageLink(PackageDoc pkg) {
        print(getPackageLink(pkg));
    }
    
    public void printPackageLink(PackageDoc pkg, boolean bold) {
        print(getPackageLink(pkg, bold));
    }
    
    public void printPackageLink(PackageDoc pkg, String linklabel) {
        print(getPackageLink(pkg, linklabel, false));
    }
    
    /**
     * Get link for individual package file.
     */
    public String getPackageLink(PackageDoc pkg) {
        return getPackageLink(pkg, pkg.name(), false);
    }
    
    public String getPackageLink(PackageDoc pkg, boolean bold) {
        return getPackageLink(pkg, pkg.name(), bold);
    }
    
    public String getPackageLink(PackageDoc pkg, String label) {
        return getPackageLink(pkg, label, false);
    }
    
    public String getPackageLink(PackageDoc pkg, String linklabel,
                                 boolean bold) {
        boolean included = pkg.isIncluded();
        if (! included) {
            PackageDoc[] packages = configuration.packages;
            for (int i = 0; i < packages.length; i++) {
                if (packages[i].equals(pkg)) {
                    included = true;
                    break;
                }
            }
        }
        if (included) {
            return getHyperLink(pathString(pkg, "package-summary.html"),
                                "", linklabel, bold);
        } else {
            String crossPkgLink = getCrossPackageLink(pkg.name());
            if (crossPkgLink != null) {
                return getHyperLink(crossPkgLink, "", linklabel, bold);
            } else {
                return linklabel;
            }
        }
    }
    
    public String italicsClassName(ClassDoc cd, boolean qual) {
        String name = (qual)? cd.qualifiedName(): cd.name();
        return (cd.isInterface())?  italicsText(name): name;
    }
    
    public void printSrcLink(ProgramElementDoc d, String label) {
        if (d == null) {
            return;
        }
        ClassDoc cd = d.containingClass();
        if (cd == null) {
            //d must be a class doc since in has no containing class.
            cd = (ClassDoc) d;
        }
        String href = relativepath + configuration.SOURCE_OUTPUT_DIR_NAME
            + SourceToHTMLConverter.getPackagePath(cd.containingPackage())
            + cd.name() + ".html"
            + "#" + SourceToHTMLConverter.getAnchorName(d);
        printHyperLink(href, "", label, true);
    }
    
    public void printClassLink(ClassDoc cd) {
        print(getClassLink(cd, false));
    }
    
    public String getClassLink(ClassDoc cd) {
        return getClassLink(cd, false);
    }
    
    public void printClassLink(ClassDoc cd, String label) {
        print(getClassLink(cd, "", label, false));
    }
    
    public String getClassLink(ClassDoc cd, String label) {
        return getClassLink(cd, "", label, false);
    }
    
    public void printClassLink(ClassDoc cd, String where, String label) {
        print(getClassLink(cd, where, label, false));
    }
    
    public void printClassLink(ClassDoc cd, String label, boolean bold) {
        print(getClassLink(cd, "", label, bold));
    }
    
    public void printClassLink(ClassDoc cd, String where, String label,
                               boolean bold, String color) {
        print(getClassLink(cd, where, label, bold, color, ""));
    }
    
    public String getClassLink(ClassDoc cd, String where, String label) {
        return getClassLink(cd, where, label, false);
    }
    
    public void printClassLink(ClassDoc cd, boolean bold) {
        print(getClassLink(cd, bold));
    }
    
    public String getClassLink(ClassDoc cd, boolean bold) {
        return getClassLink(cd, "", "", bold);
    }
    
    public void printClassLink(ClassDoc cd, String where,
                               String label, boolean bold) {
        print(getClassLink(cd, where, label, bold));
    }
    
    public String getClassLink(ClassDoc cd, String where,
                               String label, boolean bold, String color,
                               String target) {
        boolean nameUnspecified = label.length() == 0;
        if (nameUnspecified) {
            label = cd.name();
        }
        displayLength += label.length();
                                   
        //Create a tool tip if we are linking to a class or interface.  Don't
        //create one if we are linking to a member.
        String title = where == null || where.length() == 0 ?(
            getText(cd.isInterface() ?
                "doclet.Href_Interface_Title" :
                "doclet.Href_Class_Title", cd.containingPackage().name())) :
            "";
        if (cd.isIncluded()) {
            if (isGeneratedDoc(cd)) {
                String filename = pathToClass(cd);
                return getHyperLink(filename, where, label, bold, color, title, target);
            }
        } else {
            String crosslink = getCrossClassLink(cd.qualifiedName(), where,
                label, bold, color, true);
            if (crosslink != null) {
                return crosslink;
            }
        }
        if (nameUnspecified) {
            displayLength -= label.length();
            label = configuration.getClassName(cd);
            displayLength += label.length();
        }
        return label;
    }
    
    public String getClassLink(ClassDoc cd, String where,
                               String label, boolean bold) {
        return getClassLink(cd, where, label, bold, "", "");
    }
    
    /*************************************************************
     * Return a class cross link to external class documentation.
     * The name must be fully qualified to determine which package
     * the class is in.  The -link option does not allow users to
     * link to external classes in the "default" package.
     *
     * @param qualifiedClassName the qualified name of the external class.
     * @param refMemName the name of the member being referenced.  This should
     * be null or empty string if no member is being referenced.
     * @param label the label for the external link.
     * @param bold true if the link should be bold.
     * @param style the style of the link.
     * @param code true if the label should be code font.
     */
    public String getCrossClassLink(String qualifiedClassName, String refMemName,
                                    String label, boolean bold, String style,
                                    boolean code) {
        String className = "",
            packageName = qualifiedClassName == null ? "" : qualifiedClassName;
        int periodIndex;
        while((periodIndex = packageName.lastIndexOf('.')) != -1) {
            className = packageName.substring(periodIndex + 1, packageName.length()) +
                (className.length() > 0 ? "." + className : "");
            String defaultLabel = code ? getCode() + className + getCodeEnd() : className;
            packageName = packageName.substring(0, periodIndex);
            if (getCrossPackageLink(packageName) != null) {
                //The package exists in external documentation, so link to the external
                //class (assuming that it exists).  This is definitely a limitation of
                //the -link option.  There are ways to determine if an external package
                //exists, but no way to determine if the external class exists.  We just
                //have to assume that it does.
                return getHyperLink(
                    configuration.extern.getExternalLink(packageName, relativepath, className + ".html"),
                    refMemName == null ? "" : refMemName,
                    label == null || label.length() == 0 ? defaultLabel : label,
                    bold, style,
                    getText("doclet.Href_Class_Or_Interface_Title", packageName),
                    "");
            }
        }
        return null;
    }
    
    public boolean isCrossClassIncluded(ClassDoc cd) {
        if (cd.isIncluded()) {
            return isGeneratedDoc(cd);
        }
        return configuration.extern.isExternal(cd);
    }
    
    public String getCrossPackageLink(String pkgName) {
        return configuration.extern.getExternalLink(pkgName, relativepath,
            "package-summary.html");
    }
    
    public void printQualifiedClassLink(ClassDoc cd) {
        printClassLink(cd, "", configuration.getClassName(cd));
    }
    
    public String getQualifiedClassLink(ClassDoc cd) {
        return getClassLink(cd, "", configuration.getClassName(cd));
    }
    
    /**
     * Print Class link, with only class name as the link and prefixing
     * plain package name.
     */
    public void printPreQualifiedClassLink(ClassDoc cd) {
        print(getPreQualifiedClassLink(cd, false));
    }
    
    public String getPreQualifiedClassLink(ClassDoc cd) {
        return getPreQualifiedClassLink(cd, false);
    }
    
    public String getPreQualifiedClassLink(ClassDoc cd, boolean bold) {
        String classlink = "";
        PackageDoc pd = cd.containingPackage();
        if(pd != null && ! configuration.shouldExcludeQualifier(pd.name())) {
            classlink = getPkgName(cd);
        }
        classlink += getClassLink(cd, "", cd.name(), bold);
        return classlink;
    }
    
    
    /**
     * Print Class link, with only class name as the bold link and prefixing
     * plain package name.
     */
    public void printPreQualifiedBoldClassLink(ClassDoc cd) {
        print(getPreQualifiedClassLink(cd, true));
    }
    
    public void printText(String key) {
        print(getText(key));
    }
    
    public void printText(String key, String a1) {
        print(getText(key, a1));
    }
    
    public void printText(String key, String a1, String a2) {
        print(getText(key, a1, a2));
    }
    
    public void boldText(String key) {
        bold(getText(key));
    }
    
    public void boldText(String key, String a1) {
        bold(getText(key, a1));
    }
    
    public void boldText(String key, String a1, String a2) {
        bold(getText(key, a1, a2));
    }
    
    public String getText(String key) {
        return msg(false).getText(key);
    }
    
    public String getText(String key, String a1) {
        return msg(false).getText(key, a1);
    }
    
    public String getText(String key, String a1, String a2) {
        return msg(false).getText(key, a1, a2);
    }
    
    public String getText(String key, String a1, String a2, String a3) {
        return msg(false).getText(key, a1,
                                  a2, a3);
    }
    
    public void notice(SourcePosition pos, String key, String a1) {
        msg(false).notice(key, a1);
    }
    
    public void notice(SourcePosition pos, String key, String a1, String a2) {
        msg(false).notice(key, a1, a2);
    }
    
    public void warning(SourcePosition pos, String key, String a1) {
        msg(true).warning(pos, key, a1);
    }
    
    public void warning(SourcePosition pos, String key, String a1, String a2) {
        msg(true).warning(pos, key, a1, a2);
    }
    
    public void warning(SourcePosition pos, String key, String a1, String a2, String a3) {
        msg(true).warning(pos, key, a1, a2, a3);
    }
    
    public void error(SourcePosition pos, String key, String a1) {
        msg(true).notice(pos, key, a1);
    }
    
    public void error(SourcePosition pos, String key, String a1, String a2) {
        msg(true).notice(pos, key, a1, a2);
    }
    
    /**
     * Print link for any doc element.
     */
    public void printDocLink(Doc doc, String label, boolean bold) {
        print(getDocLink(doc, label, bold));
    }
    
    public String getDocLink(Doc doc, String label, boolean bold) {
        if (doc instanceof PackageDoc) {
            return getPackageLink((PackageDoc)doc, label);
        } else if (doc instanceof ClassDoc) {
            return getClassLink((ClassDoc)doc, "", label, bold);
        } else if (doc instanceof ExecutableMemberDoc) {
            ExecutableMemberDoc emd = (ExecutableMemberDoc)doc;
            return getClassLink(emd.containingClass(),
                                emd.name()+emd.signature(), label, bold);
        } else if (doc instanceof MemberDoc) {
            MemberDoc md = (MemberDoc)doc;
            return getClassLink(md.containingClass(), md.name(), label, bold);
        } else if (doc instanceof RootDoc) {
            return getHyperLink("overview-summary.html", label);
        } else {
            return label;
        }
    }
    
    /**
     * Return true if the doc element is getting documented, depending upon
     * -nodeprecated option and @deprecated tag used. Return true if
     * -nodeprecated is not used or @deprecated tag is not used.
     */
    public boolean isGeneratedDoc(Doc doc) {
        return configuration.isGeneratedDoc(doc);
    }
    
    public void printDocLink(Doc doc, String label) {
        printDocLink(doc, label, false);
    }
    
    public String getDocLink(Doc doc, String label) {
        return getDocLink(doc, label, false);
    }
    
    public static boolean serialInclude(Doc doc) {
        if (doc == null) {
            return false;
        }
        if (doc.isClass()) {
            return serialClassInclude((ClassDoc)doc);
        }
        return serialDocInclude(doc);
    }
    
    public static boolean serialClassInclude(ClassDoc cd) {
        if (cd.isSerializable()) {
            if (cd.tags("serial").length > 0) {
                return serialDocInclude(cd);
            } else if (cd.isPublic() || cd.isProtected()) {
                return true;   // default is include
                // public/protected serializable class.
            } else {   // it is a package private/private class
                return false;  // default is exclude package private class.
            }
        }
        return false;
    }
    
    
    public static boolean serialDocInclude(Doc doc) {
        Tag[] serial = doc.tags("serial");
        if (serial.length > 0) {
            String serialtext = serial[0].text().toLowerCase();
            if (serialtext.indexOf("exclude") >= 0) {
                return false;
            } else if (serialtext.indexOf("include") >= 0) {
                return true;
            }
        }
        return true;
    }
    
    public String seeTagToString(SeeTag see) {
        String tagName = see.name();
        if (! (tagName.startsWith("@link") || tagName.equals("@see"))) {
            return "";
        }
        StringBuffer result = new StringBuffer();
        boolean isplaintext = tagName.toLowerCase().equals("@linkplain");
        String label = see.label();
        label = (label.length() > 0)?
            ((isplaintext) ? label :
                 getCode() + label + getCodeEnd()):"";
        String seetext = replaceDocRootDir(see.text());
        
        //Check if @see is an href or "string"
        if (seetext.startsWith("<") || seetext.startsWith("\"")) {
            result.append(seetext);
            return result.toString();
        }
        
        //The text from the @see tag.  We will output this text when a label is not specified.
        String text = (isplaintext) ? seetext : getCode() + seetext + getCodeEnd();
        ClassDoc refClass = see.referencedClass();
        String refClassName = see.referencedClassName();
        MemberDoc refMem = see.referencedMember();
        String refMemName = see.referencedMemberName();
        if (refClass == null) {
            //@see is not referencing an included class
            PackageDoc refPackage = see.referencedPackage();
            if (refPackage != null && refPackage.isIncluded()) {
                //@see is referencing an included package
                String packageName = isplaintext ? refPackage.name() :
                    getCode() + refPackage.name() + getCodeEnd();
                result.append(getPackageLink(refPackage,
                    label.length() == 0 ? packageName : label));
            } else {
                //@see is not referencing an included class or package.  Check for cross links.
                String classCrossLink, packageCrossLink = getCrossPackageLink(refClassName);
                if (packageCrossLink != null) {
                    //Package cross link found
                    result.append(getHyperLink(packageCrossLink, "",
                        (label.length() == 0)? text : label, false));
                } else if ((classCrossLink = getCrossClassLink(refClassName,
                        refMemName, label, false, "", ! isplaintext)) != null) {
                    //Class cross link found (possiblly to a member in the class)
                    result.append(classCrossLink);
                } else {
                    //No cross link found so print warning
                    warning(see.position(), "doclet.see.class_or_package_not_found",
                            tagName, seetext);
                    result.append((label.length() == 0)? text: label);
                }
            }
        } else if (refMemName == null) {
            // Must be a class reference since refClass is not null and refMemName is null.
            if (label.length() == 0) {
                label = (isplaintext) ? refClass.name() : getCode() + refClass.name() + getCodeEnd();
                result.append(getClassLink(refClass, label));
            } else {
                result.append(getClassLink(refClass, label));
            }
        } else if (refMem == null) {
            // Must be a member reference since refClass is not null and refMemName is not null.
            // However, refMem is null, so this referenced member does not exist.
            result.append((label.length() == 0)? text: label);
        } else {
            // Must be a member reference since refClass is not null and refMemName is not null.
            // refMem is not null, so this @see tag must be referencing a valid member.
            ClassDoc containing = refMem.containingClass();
            if (configuration.currentcd != containing) {
                refMemName = containing.name() + "." + refMemName;
            }
            if (refMem instanceof ExecutableMemberDoc) {
                if (refMemName.indexOf('(') < 0) {
                    refMemName += ((ExecutableMemberDoc)refMem).signature();
                }
            }
            text = (isplaintext) ? refMemName : getCode() + refMemName + getCodeEnd();
            result.append(getDocLink(refMem, (label.length() == 0)? text: label));
        }
        return result.toString();
    }
    
    /**
     * Print tag information
     */
    public void generateTagInfo(Doc doc) {
        configuration.tagletManager.checkTags(doc, doc.tags(), false);
        configuration.tagletManager.checkTags(doc, doc.inlineTags(), true);
        Taglet[] definedTags;
        if (doc instanceof ClassDoc) {
            definedTags = configuration.tagletManager.getTypeCustomTags();
        } else if (doc instanceof PackageDoc){
            definedTags = configuration.tagletManager.getPackageCustomTags();
        } else {
            //Must be root doc
            definedTags = configuration.tagletManager.getOverviewCustomTags();
        }
        Tag[][] tagsInDoc = new Tag[definedTags.length][];
        boolean found = false;
        for (int i = 0; i < definedTags.length; i++) {
            tagsInDoc[i] = doc.tags(definedTags[i].getName());
            if (tagsInDoc[i].length > 0) {
                found = true;
            }
        }
        if (found
            || (doc.isClass() && ((ClassDoc)doc).isSerializable())) {
            dl();
            String output;
            for (int i = 0; i < definedTags.length; i++) {
                if (definedTags[i] instanceof SimpleTaglet) {
                    output = ((SimpleTaglet) definedTags[i]).toString(tagsInDoc[i], this);
                } else if (definedTags[i] instanceof SeeTaglet) {
                    output = ((SeeTaglet) definedTags[i]).toString(doc, this);
                } else {
                    output = definedTags[i].toString(tagsInDoc[i]);
                }
                if (output != null && output.length() > 0) {
                    configuration.tagletManager.seenCustomTag(definedTags[i].getName());
                    print(output);
                }
            }
            dlEnd();
        }
    }
    
    public void printSinceTag(Doc doc) {
        Tag[] sinces = doc.tags("since");
        if (!configuration.nosince && sinces.length > 0) {
            // there is going to be only one since tag
            dt();
            boldText("doclet.Since");
            dd();
            printInlineComment(doc, sinces[0]);
            ddEnd();
        }
    }
    
    public void printInlineComment(Doc doc, Tag tag) {
        printCommentTags(doc, tag.inlineTags(), false, false);
    }
    
    public void printInlineDeprecatedComment(Doc doc, Tag tag) {
        printCommentTags(doc, tag.inlineTags(), true, false);
    }
    
    public void printSummaryComment(Doc doc) {
        printCommentTags(doc, doc.firstSentenceTags(), false, true);
    }
    
    public void printSummaryDeprecatedComment(Doc doc) {
        printCommentTags(doc, doc.firstSentenceTags(), true, true);
    }
    
    public void printSummaryDeprecatedComment(Doc doc, Tag tag) {
        printCommentTags(doc, tag.firstSentenceTags(), true, true);
    }
    
    public void printInlineComment(Doc doc) {
        printCommentTags(doc, doc.inlineTags(), false, false);
        p();
    }
    
    public void printInlineDeprecatedComment(Doc doc) {
        printCommentTags(doc, doc.inlineTags(), true, false);
    }
    
    private void printCommentTags(Doc doc, Tag[] tags, boolean depr, boolean first) {
        if(configuration.nocomment){
            return;
        }
        if (depr) {
            italic();
        }
        String result = commentTagsToString(doc, tags, depr, first);
        if (doc instanceof MethodDoc) {
            result = replaceInheritDoc((MethodDoc) doc, null, result.toString());
        }
        print(result);
        if (depr) {
            italicEnd();
        }
        if (tags.length == 0) {
            space();
        }
    }
    
    /**
     * Converts inline tags and text to text strings, expanding the
     * inline tags along the way.  Called wherever text can contain
     * an inline tag, such as in comments or in free-form text arguments
     * to non-inline tags.
     *
     * @param doc    specific doc where comment resides
     * @param tags   array of text tags and inline tags (often alternating)
     *               present in the text of interest for this doc
     * @param depr   true if deprecated comment (to format it differently)
     * @param first  true if text is first sentence
     */
    public String commentTagsToString(Doc doc, Tag[] tags, boolean depr, boolean first) {
        StringBuffer result = new StringBuffer();
        // Array of all possible inline tags for this javadoc run
        Taglet[] definedTags = configuration.tagletManager.getInlineCustomTags();
        configuration.tagletManager.checkTags(doc, tags, true);
        for (int i = 0; i < tags.length; i++) {
            Tag tagelem = tags[i];
            String tagName = tagelem.name();
            if (tagelem instanceof SeeTag) {
                result.append(seeTagToString((SeeTag)tagelem));
            } else if (! tagName.equals("Text")) {
                //This is a custom inline tag.
                for (int j = 0; j < definedTags.length; j++) {
                    if (("@"+definedTags[j].getName()).equals(tagName)) {
                        //Given a name of a seen custom tag, remove it from the
                        // set of unseen custom tags.
                        configuration.tagletManager.seenCustomTag(definedTags[j].getName());
                        if (definedTags[j] instanceof AbstractInlineTaglet) {
                            // Tag is inline: {@docRoot}, {@inheritDoc} or {@value}
                            result.append(((AbstractInlineTaglet) definedTags[j]).toString(tagelem, doc, this));
                        } else {
                            // Tag is not inline: @param, @return, @see, @throws or other simple tag
                            result.append(definedTags[j].toString(tagelem));
                        }
                    }
                }
            } else {
                //This is just a regular text tag.  The text may contain html links (<a>)
                //or inline tag {@docRoot}, which will be handled as special cases.
                String relativeLink, text = tagelem.text();
                int end, begin = text.toLowerCase().indexOf("<a ");
                if(doc != null && (doc instanceof ExecutableMemberDoc || doc instanceof ClassDoc)
                   && begin >= 0){
                    StringBuffer textBuff = new StringBuffer(text);
                    ClassDoc containingClass;
                    if(doc instanceof ExecutableMemberDoc){
                        containingClass = ((ExecutableMemberDoc) doc).containingClass();
                    } else {
                        containingClass = (ClassDoc) doc;
                    }
                    String className = containingClass.name();
                    String pkgPath = DirectoryManager.getDirectoryPath(containingClass.containingPackage());
                    while(begin >=0){
                        begin = textBuff.indexOf("=", begin) + 1;
                        end = textBuff.indexOf(">", begin +1);
                        if(begin == 0){
                            warning(doc.position(), "doclet.malformed_html_link_tag", text);
                            break;
                        }
                        if (end == -1) {
                            //Break without warning.  This <a> tag is not necessarily malformed.  The text
                            //might be missing '>' character because the href has an inline tag.
                            break;
                        }
                        if(textBuff.substring(begin, end).indexOf("\"") != -1){
                            begin = textBuff.indexOf("\"", begin) + 1;
                            end = textBuff.indexOf("\"", begin +1);
                            if(begin == 0 || end == -1){
                                break;
                            }
                        }
                        relativeLink = textBuff.substring(begin, end);
                        if(!(relativeLink.toLowerCase().startsWith("http:")
                                 || relativeLink.toLowerCase().startsWith("file:"))){
                            if(!filename.startsWith(className)){
                                relativeLink = "{@docRoot}"
                                    + DirectoryManager.urlfileseparator
                                    + pkgPath
                                    + DirectoryManager.urlfileseparator
                                    + relativeLink;
                                textBuff.replace(begin, end, relativeLink);
                            }
                        }
                        begin = textBuff.toString().toLowerCase().indexOf("<a ", begin + 1);
                    }
                    text = textBuff.toString();
                }
                // Replace @docRoot only if not represented by an instance of DocRootTaglet,
                // that is, only if it was not present in a source file doc comment.
                // This happens when inserted by the doclet (a few lines
                // above in this method).  [It might also happen when passed in on the command
                // line as a text argument to an option (like -header).]
                text = replaceDocRootDir(text);
                if (first) {
                    text = removeNonInlineHtmlTags(text);
                }
                result.append(text);
            }
        }
        return result.toString();
    }
    
    public String removeNonInlineHtmlTags(String text) {
        if (text.indexOf('<') < 0) {
            return text;
        }
        String noninlinetags[] = { "<ul>", "</ul>", "<ol>", "</ol>",
                "<dl>", "</dl>", "<table>", "</table>",
                "<tr>", "</tr>", "<td>", "</td>",
                "<th>", "</th>", "<p>", "</p>",
                "<li>", "</li>", "<dd>", "</dd>",
                "<dir>", "</dir>", "<dt>", "</dt>",
                "<h1>", "</h1>", "<h2>", "</h2>",
                "<h3>", "</h3>", "<h4>", "</h4>",
                "<h5>", "</h5>", "<h6>", "</h6>",
                "<pre>", "</pre>", "<menu>", "</menu>",
                "<listing>", "</listing>", "<hr>",
                "<blockquote>", "</blockquote>",
                "<center>", "</center>",
                "<UL>", "</UL>", "<OL>", "</OL>",
                "<DL>", "</DL>", "<TABLE>", "</TABLE>",
                "<TR>", "</TR>", "<TD>", "</TD>",
                "<TH>", "</TH>", "<P>", "</P>",
                "<LI>", "</LI>", "<DD>", "</DD>",
                "<DIR>", "</DIR>", "<DT>", "</DT>",
                "<H1>", "</H1>", "<H2>", "</H2>",
                "<H3>", "</H3>", "<H4>", "</H4>",
                "<H5>", "</H5>", "<H6>", "</H6>",
                "<PRE>", "</PRE>", "<MENU>", "</MENU>",
                "<LISTING>", "</LISTING>", "<HR>",
                "<BLOCKQUOTE>", "</BLOCKQUOTE>",
                "<CENTER>", "</CENTER>"
        };
        for (int i = 0; i < noninlinetags.length; i++) {
            text = replace(text, noninlinetags[i], "");
        }
        return text;
    }
    
    public String replace(String text, String tobe, String by) {
        while (true) {
            int startindex = text.indexOf(tobe);
            if (startindex < 0) {
                return text;
            }
            int endindex = startindex + tobe.length();
            StringBuffer replaced = new StringBuffer();
            if (startindex > 0) {
                replaced.append(text.substring(0, startindex));
            }
            replaced.append(by);
            if (text.length() > endindex) {
                replaced.append(text.substring(endindex));
            }
            text = replaced.toString();
        }
    }
    
    public void printStyleSheetProperties() {
        String filename = configuration.stylesheetfile;
        if (filename.length() > 0) {
            File stylefile = new File(filename);
            String parent = stylefile.getParent();
            filename = (parent == null)?
                filename:
                filename.substring(parent.length() + 1);
        } else {
            filename = "stylesheet.css";
        }
        filename = relativepath + filename;
        link("REL =\"stylesheet\" TYPE=\"text/css\" HREF=\"" +
                 filename + "\" " + "TITLE=\"Style\"");
    }
    
    /**
     * According to the Java Language Specifications, all the outer classes
     * and static nested classes are core classes.
     */
    public boolean isCoreClass(ClassDoc cd) {
        return cd.containingClass() == null || cd.isStatic();
    }
    
    public Configuration configuration() {
        return configuration;
    }
    
    /**
     * If checkVersion is true, print the version number before return
     * the MessageRetriever.
     */
    public MessageRetriever msg(boolean checkVersion) {
        if(checkVersion && !configuration.printedVersion){
            configuration.standardmessage.notice("stddoclet.version", Standard.BUILD_DATE);
            configuration.printedVersion = true;
        }
        return configuration.standardmessage;
    }
    
    /**
     * Given a <code>MethodDoc</code> item, a <code>Tag</code> in the
     * <code>MethodDoc</code> item and a String, replace all occurances
     * of @inheritDoc with documentation from it's superclass or superinterface.
     */
    public String replaceInheritDoc(MethodDoc md, Tag tag, String text) {
        if (text == null || text.indexOf("{@inheritDoc}") == -1) {
            return text;
        }
        String replacement = "";
        MethodDoc omd = md.overriddenMethod();
        if (omd == null) {
            //This method does not override any method in the superclass.
            //Let's try to find a method that it implements from an interface.
            MethodDoc[] implementedMethods = (new ImplementedMethods(md)).build();
            omd = implementedMethods != null && implementedMethods.length > 0 ?
                implementedMethods[0] : null;
        }
        if (omd != null) {
            if (tag == null) {
                replacement = commentTagsToString(omd, omd.inlineTags(), false, false);
                replacement = replaceInheritDoc(omd, null, replacement);
            } else {
                if (tag instanceof ParamTag) {
                    ParamTag[] tags = omd.paramTags();
                    for (int i = 0; i < tags.length; i++) {
                        if (tags[i].parameterName().equals(((ParamTag) tag).parameterName())) {
                            replacement = commentTagsToString(omd, tags[i].inlineTags(), false, false);
                            replacement = replaceInheritDoc(omd, tags[i], replacement);
                            break;
                        }
                    }
                } else if (tag instanceof ThrowsTag) {
                    ThrowsTag[] tags = omd.throwsTags();
                    String targetExceptionName = ((ThrowsTag) tag).exception() == null ?
                        ((ThrowsTag) tag).exceptionName() : ((ThrowsTag) tag).exception().qualifiedName();
                    for (int i = 0; i < tags.length; i++) {
                        String currentExceptionName = tags[i].exception() == null ? tags[i].exceptionName() :
                            tags[i].exception().qualifiedName();
                        if (targetExceptionName.equals(currentExceptionName)) {
                            replacement = commentTagsToString(omd, tags[i].inlineTags(), false, false);
                            replacement = replaceInheritDoc(omd, tags[i], replacement);
                            break;
                        }
                    }
                } else if (tag.name().equals("@return")) {
                    Tag[] tags = omd.tags("return");
                    if (tags.length > 0) {
                        replacement = commentTagsToString(omd, tags[0].inlineTags(), false, false);
                        replacement = replaceInheritDoc(omd, tags[0], replacement);
                    }
                }
            }
        } else {
            warning(md.position(), "doclet.noInheritedDoc", md.name() + md.flatSignature());
            return text;
        }
        StringBuffer sb = new StringBuffer(text);
        int start;
        while ((start = sb.indexOf("{@inheritDoc}")) != -1 && replacement.indexOf("{@inheritDoc}") == -1) {
            sb.replace(start, start + 13, replacement);
        }
        return sb.toString();
    }
}


