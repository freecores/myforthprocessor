/*
 * @(#)ConfigurationStandard.java	1.52 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.tools.doclets.standard;

import com.sun.tools.doclets.*;
import com.sun.javadoc.*;
import java.util.*;
import java.io.*;

/**
 * Configure the output based on the command line options.
 * <p>
 * Also determine the length of the command line option. For example,
 * for a option "-header" there will be a string argument associated, then the
 * the length of option "-header" is two. But for option "-nohelp" no argument
 * is needed so it's length is 1.
 * </p>
 * <p>
 * Also do the error checking on the options used. For example it is illegal to
 * use "-helpfile" option when already "-nohelp" option is used.
 * </p>
 *
 * @author Robert Field.
 * @author Atul Dambalkar.
 * @author Jamie Ho
 */
public class ConfigurationStandard extends Configuration {
    
    /**
     * The name of the constant values file.
     */
    public static final String CONSTANTS_FILE_NAME = "constant-values.html";
    
    /**
     * The taglet manager.
     */
    public TagletManager tagletManager;
    
    /**
     * The path to Taglets
     */
    public String tagletpath = "";
    
    /**
     * True if user wants to suppress descriptions and tags.
     */
    public boolean nocomment = false;
    
    /**
     * True if user wants to suppress time stamp in output.
     */
    public boolean notimestamp= false;
    
    /**
     * True if user want to add member names as meta keywords.
     * Set to false because meta keywords are ignored in general
     * by most Internet search engines.
     */
    public boolean keywords = false;

    /**
     * Argument for command line option "-header".
     */
    public String header = "";
    
    /**
     * Argument for command line option "-packagesheader".
     */
    public String packagesheader = "";
    
    /**
     * Argument for command line option "-footer".
     */
    public String footer = "";
    
    /**
     * Argument for command line option "-doctitle".
     */
    public String doctitle = "";
    
    /**
     * Argument for command line option "-windowtitle".
     */
    public String windowtitle = "";
    
    /**
     * Argument for command line option "-bottom".
     */
    public String bottom = "";
    
    /**
     * Argument for command line option "-helpfile".
     */
    public String helpfile = "";
    
    /**
     * Argument for command line option "-stylesheetfile".
     */
    public String stylesheetfile = "";
    
    /**
     * True if command line option "-nohelp" is used. Default value is false.
     */
    public boolean nohelp = false;
    
    /**
     * True if command line option "-splitindex" is used. Default value is
     * false.
     */
    public boolean splitindex = false;
    
    /**
     * False if command line option "-noindex" is used. Default value is true.
     */
    public boolean createindex = true;
    
    /**
     * True if command line option "-use" is used. Default value is false.
     */
    public boolean classuse = false;
    
    /**
     * False if command line option "-notree" is used. Default value is true.
     */
    public boolean createtree = true;
    
    /**
     * True if command line option "-nodeprecated" is used. Default value is
     * false.
     */
    public boolean nodeprecatedlist = false;
    
    /**
     * True if command line option "-nonavbar" is used. Default value is false.
     */
    public boolean nonavbar = false;
    
    /**
     * True if command line option "-nooverview" is used. Default value is
     * false
     */
    private boolean nooverview = false;
    
    /**
     * True if command line option "-overview" is used. Default value is false.
     */
    public boolean overview = false;
    
    /**
     * This is true if option "-overview" is used or option "-overview" is not
     * used and number of packages is more than one.
     */
    public boolean createoverview = false;
    
    /**
     * This is true if option "-serialwarn" is used. Defualt value is false to
     * supress excessive warnings about serial tag.
     */
    public boolean serialwarn = false;
    
    /**
     * Unique Resource Handler for this package.
     */
    public final MessageRetriever standardmessage;
    
    /**
     * First file to appear in the right-hand frame in the generated
     * documentation.
     */
    public String topFile = "";
    
    /**
     * The classdoc for the class file getting generated.
     */
    public ClassDoc currentcd = null;  // Set this classdoc in the
    // ClassWriter.
    
    /**
     * The tracker of external package links (sole-instance).
     */
    public final Extern extern = new Extern(this);
    
    /**
     * The package grouping sole-instance.
     */
    public final Group group = new Group(this);

    /**
     * The meta tag keywords sole-instance.
     */
    public final MetaKeywords metakeywords = new MetaKeywords(this);
    
    /**
     * True if running in quiet mode
     */
    public boolean quiet = false;
    
    /**
     * True if version id has been printed
     */
    public boolean printedVersion = false;
    /**
     * Constructor. Initialises resource for the
     * {@link com.sun.tools.doclets.MessageRetriever}.
     */
    public ConfigurationStandard() {
        standardmessage = new MessageRetriever(this,
                                               "com.sun.tools.doclets.standard.resources.standard");
    }
    
    /**
     * Depending upon the command line options provided by the user, set
     * configure the output generation environment.
     *
     * @param root Used to retrieve used comand line options.
     */
    public void setSpecificDocletOptions(RootDoc root) {
        String[][] options = root.options();
        LinkedHashSet customTagStrs = new LinkedHashSet();
        for (int oi = 0; oi < options.length; ++oi) {
            String[] os = options[oi];
            String opt = os[0].toLowerCase();
            if (opt.equals("-footer")) {
                footer =  os[1];
            } else  if (opt.equals("-header")) {
                header =  os[1];
            } else  if (opt.equals("-packagesheader")) {
                packagesheader =  os[1];
            } else  if (opt.equals("-doctitle")) {
                doctitle =  os[1];
            } else  if (opt.equals("-windowtitle")) {
                windowtitle =  os[1];
            } else  if (opt.equals("-bottom")) {
                bottom =  os[1];
            } else  if (opt.equals("-helpfile")) {
                helpfile =  os[1];
            } else  if (opt.equals("-stylesheetfile")) {
                stylesheetfile =  os[1];
            } else  if (opt.equals("-charset")) {
                charset =  os[1];
            } else  if (opt.equals("-nohelp")) {
                nohelp = true;
            } else  if (opt.equals("-splitindex")) {
                splitindex = true;
            } else  if (opt.equals("-noindex")) {
                createindex = false;
            } else  if (opt.equals("-use")) {
                classuse = true;
            } else  if (opt.equals("-notree")) {
                createtree = false;
            } else  if (opt.equals("-nodeprecatedlist")) {
                nodeprecatedlist = true;
            } else  if (opt.equals("-nocomment")) {
                nocomment = true;
            } else  if (opt.equals("-notimestamp")) {
                notimestamp = true;
            } else  if (opt.equals("-keywords")) {
                keywords = true;
            } else  if (opt.equals("-nosince")) {
                nosince = true;
            } else  if (opt.equals("-nonavbar")) {
                nonavbar = true;
            } else  if (opt.equals("-nooverview")) {
                nooverview = true;
            } else  if (opt.equals("-overview")) {
                overview = true;
            } else  if (opt.equals("-serialwarn")) {
                serialwarn = true;
            } else if (opt.equals("-group")) {
                group.checkPackageGroups(os[1], os[2], root);
            } else if (opt.equals("-link")) {
                String url = os[1];
                extern.url(url, url, root, false);
            } else if (opt.equals("-linkoffline")) {
                String url = os[1];
                String pkglisturl = os[2];
                extern.url(url, pkglisturl, root, true);
            } else if (opt.equals("-quiet")) {
                message.setQuiet();
                standardmessage.setQuiet();
                quiet = true;
            } else if (opt.equals("-tag") || opt.equals("-taglet")) {
                customTagStrs.add(os);
            } else if (opt.equals("-tagletpath")) {
                tagletpath = os[1];
            }
        }
        if (root.specifiedClasses().length > 0) {
            Map map = new HashMap();
            PackageDoc pd;
            ClassDoc[] classes = root.classes();
            for (int i = 0; i < classes.length; i++) {
                pd = classes[i].containingPackage();
                if(! map.containsKey(pd.name())) {
                    map.put(pd.name(), pd);
                }
            }
            packages = new PackageDoc[map.size()];
            Iterator it = map.values().iterator();
            for (int i = 0; i < map.size(); i++) {
                packages[i] = (PackageDoc) it.next();
            }
            Arrays.sort(packages);
        }
        setCreateOverview();
        setTopFile(root);
        initTagletManager(customTagStrs);
    }
    
    /**
     * Initialize the taglet manager.  The strings to initialize the simple custom tags should
     * be in the following format:  "[tag name]:[location str]:[heading]".
     * @param customTagStrs the set two dimentional arrays of strings.  These arrays contain
     * either -tag or -taglet arguments.
     */
    private void initTagletManager(Set customTagStrs) {
        tagletManager = new TagletManager(nosince, showversion, showauthor, standardmessage);
        for (Iterator it = customTagStrs.iterator(); it.hasNext(); ) {
            String[] data = new String[3];
            String[] args = (String[]) it.next();
            if (args[0].equals("-taglet")) {
                tagletManager.addCustomTag(args[1], tagletpath);
                continue;
            }
            String seperator = args[1].indexOf(TagletManager.ALT_SIMPLE_TAGLET_OPT_SEPERATOR) == -1 ?
                TagletManager.SIMPLE_TAGLET_OPT_SEPERATOR :
                TagletManager.ALT_SIMPLE_TAGLET_OPT_SEPERATOR;
            StringTokenizer st = new StringTokenizer(args[1], seperator, true);
            if (st.countTokens() == 1) {
                String tagName = st.nextToken();
                if (tagletManager.isKnownCustomTag(tagName)) {
                    //reorder a standard tag
                    tagletManager.addNewSimpleCustomTag(tagName, null, "");
                } else {
                    //Create a simple tag with the heading that has the same name as the tag.
                    StringBuffer heading = new StringBuffer(tagName + ":");
                    heading.setCharAt(0, Character.toUpperCase(tagName.charAt(0)));
                    tagletManager.addNewSimpleCustomTag(tagName, heading.toString(), "a");
                }
            } else if (st.countTokens() == 3) {
                //Add simple taglet without heading, probably to excluding it in the output.
                tagletManager.addNewSimpleCustomTag(st.nextToken(), st.nextToken(), "");
            } else if (st.countTokens() >= 5) {
                //add a new simple tag
                data[0] = st.nextToken();
                st.nextToken();
                data[1] = st.nextToken();
                st.nextToken();
                data[2] = "";
                while (st.hasMoreTokens()) {
                    data[2] += st.nextToken();
                }
                tagletManager.addNewSimpleCustomTag(data[0], data[2], data[1]);
            } else {
                standardmessage.error("doclet.Error_invalid_custom_tag_argument", args[1]);
            }
        }
    }
    
    /**
     * Returns the "length" of a given option. If an option takes no
     * arguments, its length is one. If it takes one argument, it's
     * length is two, and so on. This method is called by JavaDoc to
     * parse the options it does not recognize. It then calls
     * {@link #validOptions(String[][], DocErrorReporter)} to
     * validate them.
     * <b>Note:</b><br>
     * The options arrive as case-sensitive strings. For options that
     * are not case-sensitive, use toLowerCase() on the option string
     * before comparing it.
     * </blockquote>
     *
     * @return number of arguments + 1 for a option. Zero return means
     * option not known.  Negative value means error occurred.
     */
    public int optionLength(String option) {
        int len = generalOptionLength(option);
        if (len != -1) {
            return len;
        }
        // otherwise look for the options we have added
        option = option.toLowerCase();
        if (option.equals("-nocomment") ||
            option.equals("-notimestamp") ||
            option.equals("-keywords") ||
            option.equals("-nodeprecatedlist") ||
            option.equals("-noindex") ||
            option.equals("-notree") ||
            option.equals("-nohelp") ||
            option.equals("-nosince") ||
            option.equals("-quiet") ||
            option.equals("-splitindex") ||
            option.equals("-use") ||
            option.equals("-nonavbar") ||
            option.equals("-serialwarn") ||
            option.equals("-nooverview")) {
            return 1;
        } else if (option.equals("-help") ) {
            standardmessage.notice("doclet.usage");
            return 1;
        } else if (option.equals("-x") ) {
            standardmessage.notice("doclet.xusage");
            return -1; // so run will end
        } else if (option.equals("-footer") ||
                   option.equals("-header") ||
                   option.equals("-packagesheader") ||
                   option.equals("-doctitle") ||
                   option.equals("-windowtitle") ||
                   option.equals("-bottom") ||
                   option.equals("-helpfile") ||
                   option.equals("-stylesheetfile") ||
                   option.equals("-link") ||
                   option.equals("-linksourcetab") ||
                   option.equals("-charset") ||
                   option.equals("-overview") ||
                   option.equals("-tag") ||
                   option.equals("-tagletpath") ||
                   option.equals("-taglet") ) {
            return 2;
        } else if (option.equals("-group") ||
                   option.equals("-linkoffline")) {
            return 3;
        } else {
            return 0;
        }
    }
    
    /**
     * After parsing the available options using
     * {@link #optionLength(String)},
     * JavaDoc invokes this method with an array of options-arrays, where
     * the first item in any array is the option, and subsequent items in
     * that array are its arguments. So, if -print is an option that takes
     * no arguments, and -copies is an option that takes 1 argument, then
     * <pre>
     *     -print -copies 3
     * </pre>
     * produces an array of arrays that looks like:
     * <pre>
     *      option[0][0] = -print
     *      option[1][0] = -copies
     *      option[1][1] = 3
     * </pre>
     * (By convention, command line switches start with a "-", but
     * they don't have to.)
     * This method is not required to be written by sub-classes and will
     * default gracefully (to true) if absent.
     * <P>
     * Printing option related error messages (using the provided
     * DocErrorReporter) is the responsibility of this method.
     * <P>
     * Note: This is invoked on a temporary config, no side-effect
     * settings will persist.
     *
     * @param options  Options used on the command line.
     * @param reporter Error reporter to be used.
     * @return true if all the options are valid.
     */
    public boolean validOptions(String options[][],
                                DocErrorReporter reporter) {
        boolean helpfile = false;
        boolean nohelp = false;
        boolean overview = false;
        boolean nooverview = false;
        boolean splitindex = false;
        boolean noindex = false;
        // check shared options
        if (!generalValidOptions(options, reporter)) {
            return false;
        }
        // otherwise look at our options
        for (int oi = 0; oi < options.length; ++oi) {
            String[] os = options[oi];
            String opt = os[0].toLowerCase();
            if (opt.equals("-helpfile")) {
                if (nohelp == true) {
                    reporter.printError(standardmessage.getText(
                                            "doclet.Option_conflict", "-helpfile", "-nohelp"));
                    return false;
                }
                if (helpfile == true) {
                    reporter.printError(standardmessage.getText(
                                            "doclet.Option_reuse", "-helpfile"));
                    return false;
                }
                File help = new File(os[1]);
                if (!help.exists()) {
                    reporter.printError(standardmessage.getText(
                                            "doclet.File_not_found", os[1]));
                    return false;
                }
                helpfile = true;
            } else  if (opt.equals("-nohelp")) {
                if (helpfile == true) {
                    reporter.printError(standardmessage.getText(
                                            "doclet.Option_conflict", "-nohelp", "-helpfile"));
                    return false;
                }
                nohelp = true;
            } else if (opt.equals("-overview")) {
                if (nooverview == true) {
                    reporter.printError(standardmessage.getText(
                                            "doclet.Option_conflict", "-overview", "-nooverview"));
                    return false;
                }
                if (overview == true) {
                    reporter.printError(standardmessage.getText(
                                            "doclet.Option_reuse", "-overview"));
                    return false;
                }
                overview = true;
            } else  if (opt.equals("-nooverview")) {
                if (overview == true) {
                    reporter.printError(standardmessage.getText(
                                            "doclet.Option_conflict", "-nooverview", "-overview"));
                    return false;
                }
                nooverview = true;
            } else if (opt.equals("-splitindex")) {
                if (noindex == true) {
                    reporter.printError(standardmessage.getText(
                                            "doclet.Option_conflict", "-splitindex", "-noindex"));
                    return false;
                }
                splitindex = true;
            } else if (opt.equals("-noindex")) {
                if (splitindex == true) {
                    reporter.printError(standardmessage.getText(
                                            "doclet.Option_conflict", "-noindex", "-splitindex"));
                    return false;
                }
                noindex = true;
            } else if (opt.equals("-group")) {
                if (!group.checkPackageGroups(os[1], os[2], reporter)) {
                    return false;
                }
            }
        }
        return true;
    }
    
    /**
     * Decide the page which will appear first in the right-hand frame. It will
     * be "overview-summary.html" if "-overview" option is used or no
     * "-overview" but the number of packages is more than one. It will be
     * "package-summary.html" of the respective package if there is only one
     * package to document. It will be a class page(first in the sorted order),
     * if only classes are provided on the command line.
     *
     * @param root Root of the program structure.
     */
    protected void setTopFile(RootDoc root) {
        if (!checkForDeprecation(root)) {
            return;
        }
        if (createoverview) {
            topFile = "overview-summary.html";
        } else {
            if (packages.length == 1 && packages[0].name().equals("")) {
                if (root.classes().length > 0) {
                    ClassDoc[] classarr = root.classes();
                    Arrays.sort(classarr);
                    ClassDoc cd = getValidClass(classarr);
                    topFile = DirectoryManager.getPathToClass(cd);
                }
            } else {
                topFile = DirectoryManager.getPathToPackage(packages[0],
                                                            "package-summary.html");
            }
        }
    }
    
    protected ClassDoc getValidClass(ClassDoc[] classarr) {
        if (!nodeprecated) {
            return classarr[0];
        }
        for (int i = 0; i < classarr.length; i++) {
            if (classarr[i].tags("deprecated").length == 0) {
                return classarr[i];
            }
        }
        return null;
    }
    
    protected boolean checkForDeprecation(RootDoc root) {
        ClassDoc[] classarr = root.classes();
        for (int i = 0; i < classarr.length; i++) {
            if (isGeneratedDoc(classarr[i])) {
                return true;
            }
        }
        return false;
    }
    
    /**
     * Generate "overview.html" page if option "-overview" is used or number of
     * packages is more than one. Sets {@link #createoverview} field to true.
     */
    protected void setCreateOverview() {
        if ((overview || packages.length > 1) && !nooverview) {
            createoverview = true;
        }
    }
    
    /**
     * Return true if the doc element is getting documented, depending upon
     * -nodeprecated option and @deprecated tag used. Return true if
     * -nodeprecated is not used or @deprecated tag is not used.
     */
    public boolean isGeneratedDoc(Doc doc) {
        if (!nodeprecated) {
            return true;
        }
        return (doc.tags("deprecated")).length == 0;
    }
}


