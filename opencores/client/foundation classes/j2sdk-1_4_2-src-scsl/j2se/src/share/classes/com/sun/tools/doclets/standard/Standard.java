/*
 * @(#)Standard.java	1.81 03/01/23
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
 * The class with "start" method, calls individual Writers.
 *
 * @author Atul M Dambalkar
 * @author Robert Field
 */
public class Standard {
    
    /**
     * The build date.  Note: For now, we will use
     * a version number instead of a date.
     */
    public static final String BUILD_DATE = System.getProperty("java.version");
    
    /**
     * The global configuration information for this run.
     */
    public ConfigurationStandard configuration = configuration();
    
    /**
     * The "start" method as required by Javadoc.
     *
     * @param Root
     * @see com.sun.javadoc.RootDoc
     * @return boolean
     */
    public static boolean start(RootDoc root) throws IOException {
        try {
            (new Standard()).startGeneration(root);
        } catch (DocletAbortException exc) {
            return false;
        }
        return true;
    }
    
    /**
     * Create the configuration instance.
     * Override this method to use a different
     * configuration.
     */
    public static ConfigurationStandard configuration() {
        return new ConfigurationStandard();
    }
    
    /**
     * Start the generation of files. Call generate methods in the individual
     * writers, which will in turn genrate the documentation files. Call the
     * TreeWriter generation first to ensure the Class Hierarchy is built
     * first and then can be used in the later generation.
     *
     * For new format.
     *
     * @see com.sun.javadoc.RootDoc
     */
    protected void startGeneration(RootDoc root) {
        if (root.classes().length == 0) {
            configuration.standardmessage.
                error("doclet.No_Public_Classes_To_Document");
            return;
        }
        configuration.setOptions(root);
        if(!configuration.quiet) {
            configuration.standardmessage.notice("stddoclet.version", BUILD_DATE);
            configuration.printedVersion = true;
        }
        if (configuration.genSrc) {
            if (configuration.destdirname.length() > 0) {
                SourceToHTMLConverter.convertRoot(configuration,
                                                  root, configuration.destdirname + File.separator
                                                      + configuration.SOURCE_OUTPUT_DIR_NAME);
            } else {
                SourceToHTMLConverter.convertRoot(configuration,
                                                  root, configuration.SOURCE_OUTPUT_DIR_NAME);
            }
        }
        ConstantsSummaryWriter.generate(configuration);
        
        if (configuration.topFile.length() == 0) {
            configuration.standardmessage.
                error("doclet.No_Non_Deprecated_Classes_To_Document");
            return;
        }
        boolean nodeprecated = configuration.nodeprecated;
        String configdestdir = configuration.destdirname;
        String confighelpfile = configuration.helpfile;
        String configstylefile = configuration.stylesheetfile;
        performCopy(configdestdir, confighelpfile);
        performCopy(configdestdir, configstylefile);
        Util.copyResourceFile(configuration, "inherit.gif", false);
        ClassTree classtree = new ClassTree(configuration, nodeprecated);
        // do early to reduce memory footprint
        if (configuration.classuse) {
            ClassUseMapper.generate(configuration, classtree);
        }
        IndexBuilder indexbuilder = new IndexBuilder(configuration,
                                                     nodeprecated);
        PackageDoc[] inclPackages = configuration.packages;
        if (configuration.createtree) {
            TreeWriter.generate(configuration, classtree);
        }
        if (configuration.createindex) {
            if (configuration.splitindex) {
                SplitIndexWriter.generate(configuration, indexbuilder);
            } else {
                SingleIndexWriter.generate(configuration, indexbuilder);
            }
        }
        
        if (!(configuration.nodeprecatedlist || nodeprecated)) {
            DeprecatedListWriter.generate(configuration);
        }
        
        AllClassesFrameWriter.generate(configuration,
                                       new IndexBuilder(configuration, nodeprecated, true));
        
        FrameOutputWriter.generate(configuration);
        
        PackagesFileWriter.generate(configuration);
        
        if (configuration.createoverview) {
            PackageIndexWriter.generate(configuration);
        }
        
        if (inclPackages.length > 1) {
            PackageIndexFrameWriter.generate(configuration);
        }
        
        if (! (inclPackages.length == 1 && inclPackages[0].name().equals(""))) {
            PackageDoc prev = null, next;
            for(int i = 0; i < inclPackages.length; i++) {
                PackageFrameWriter.generate(configuration, inclPackages[i]);
                //Don't generate package page for unnamed package
                if (inclPackages[i].name().length() == 0) {
                    continue;
                }
                next = (i + 1 < inclPackages.length && inclPackages[i+1].name().length() > 0) ?
                    inclPackages[i+1]:null;
                //If the next package is unnamed package, skip 2 ahead if possible
                next = (i + 2 < inclPackages.length && next == null) ?
                    inclPackages[i+2]: next;
                PackageWriter.generate(configuration, inclPackages[i], prev, next);
                if (configuration.createtree) {
                    PackageTreeWriter.generate(configuration,
                                               inclPackages[i], prev, next,
                                               nodeprecated);
                }
                prev = inclPackages[i];
            }
        }
        
        generateClassFiles(root, classtree);
        if (configuration.sourcepath != null &&
            configuration.sourcepath.length() > 0) {
            StringTokenizer pathTokens =
                new StringTokenizer(configuration.sourcepath, ":");
            boolean first = true;
            while(pathTokens.hasMoreTokens()){
                HtmlStandardWriter.copyDocFiles(configuration, pathTokens.nextToken()
                                                    + File.separator, HtmlStandardWriter.DOC_FILES_DIR_NAME, first);
                first = false;
            }
        }
        if (generateSerializedFormPage(root)) {
            SerializedFormWriter.generate(configuration);
        }
        
        PackageListWriter.generate(configuration);
        if (configuration.helpfile.length() == 0 &&
            !configuration.nohelp) {
            HelpWriter.generate(configuration);
        }
        if (configuration.stylesheetfile.length() == 0) {
            StylesheetWriter.generate(configuration);
        }
        configuration.tagletManager.printReport();
    }
    
    protected void generateClassFiles(RootDoc root, ClassTree classtree) {
        
        generateClassCycle(classtree);
        PackageDoc[] packages = configuration.root.specifiedPackages();
        for (int i = 0; i < packages.length; i++) {
            PackageDoc pkg = packages[i];
            generateClassCycle(pkg.interfaces(), classtree);
            generateClassCycle(pkg.ordinaryClasses(), classtree);
            generateClassCycle(pkg.exceptions(), classtree);
            generateClassCycle(pkg.errors(), classtree);
        }
    }
    
    protected String classFileName(ClassDoc cd) {
        return cd.qualifiedName() + ".html";
    }
    
    /**
     * Instantiate ClassWriter for each Class within the ClassDoc[]
     * passed to it and generate Documentation for that.
     */
    protected void generateClassCycle(ClassTree classtree) {
        String[] packageNames = configuration.classDocCatalog.packageNames();
        for (int packageNameIndex = 0; packageNameIndex < packageNames.length; packageNameIndex++) {
            ClassDoc[] arr = configuration.classDocCatalog.allClasses(packageNames[packageNameIndex]);
            Arrays.sort(arr);
            generateClassCycle(arr, classtree);
        }
    }
    
    protected void generateClassCycle(ClassDoc[] arr, ClassTree classtree) {
        Arrays.sort(arr);
        for(int i = 0; i < arr.length; i++) {
            if (!(configuration.isGeneratedDoc(arr[i]) && arr[i].isIncluded())) {
                continue;
            }
            ClassDoc prev = (i == 0)?
                null:
                arr[i-1];
            ClassDoc curr = arr[i];
            ClassDoc next = (i+1 == arr.length)?
                null:
                arr[i+1];
            
            ClassWriter.generate(configuration,
                                 curr, prev, next, classtree);
        }
    }
    
    /**
     * Check for doclet added options here.
     *
     * @return number of arguments to option. Zero return means
     * option not known.  Negative value means error occurred.
     */
    public static int optionLength(String option) {
        // Construct temporary configuration for check
        return (new ConfigurationStandard()).optionLength(option);
    }
    
    /**
     * Check that options have the correct arguments here.
     * <P>
     * This method is not required and will default gracefully
     * (to true) if absent.
     * <P>
     * Printing option related error messages (using the provided
     * DocErrorReporter) is the responsibility of this method.
     *
     * @return true if the options are valid.
     */
    public static boolean validOptions(String options[][],
                                       DocErrorReporter reporter)
        throws IOException {
        // Construct temporary configuration for check
        return (new ConfigurationStandard()).validOptions(options, reporter);
    }
    
    protected void performCopy(String configdestdir, String filename) {
        try {
            String destdir = (configdestdir.length() > 0)?
                configdestdir + File.separatorChar: "";
            if (filename.length() > 0) {
                File helpstylefile = new File(filename);
                String parent = helpstylefile.getParent();
                String helpstylefilename = (parent == null)?
                    filename:
                    filename.substring(parent.length() + 1);
                File desthelpfile = new File(destdir + helpstylefilename);
                if (!desthelpfile.getCanonicalPath().equals(
                        helpstylefile.getCanonicalPath())) {
                    configuration.standardmessage.
                        notice((SourcePosition) null, "doclet.Copying_File_0_To_File_1",
                               helpstylefile.toString(), desthelpfile.toString());
                    Util.copyFile(desthelpfile, helpstylefile);
                }
            }
        } catch (IOException exc) {
            configuration.standardmessage.
                error((SourcePosition) null, "doclet.perform_copy_exception_encountered",
                      exc.toString());
            throw new DocletAbortException();
        }
    }
    
    public boolean generateSerializedFormPage(RootDoc root) {
        PackageDoc[] packages = root.specifiedPackages();
        ClassDoc[] cmdlineClasses = root.specifiedClasses();
        for (int i = 0; i < packages.length; i++) {
            PackageDoc pkg = packages[i];
            boolean include = HtmlStandardWriter.serialInclude(pkg);
            if (include) {
                ClassDoc[] classes = pkg.allClasses();
                for (int j = 0; j < classes.length; j++) {
                    if (HtmlStandardWriter.serialInclude(classes[j])) {
                        return true;
                    }
                }
            }
        }
        for (int i = 0; i < cmdlineClasses.length; i++) {
            if (HtmlStandardWriter.serialInclude(cmdlineClasses[i])) {
                return true;
            }
        }
        return false;
    }
}



