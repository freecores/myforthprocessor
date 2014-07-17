/**
 * @(#)Main.java	1.55 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.tools.javac.v8;
import java.io.FileOutputStream;

import java.io.FileWriter;

import java.io.IOException;

import java.io.PrintWriter;

import java.text.MessageFormat;

import java.util.ResourceBundle;

import java.util.MissingResourceException;

import java.util.StringTokenizer;

import com.sun.tools.javac.v8.util.*;

import com.sun.tools.javac.v8.code.*;

import com.sun.tools.javac.v8.tree.*;

import com.sun.tools.javac.v8.comp.*;

import com.sun.tools.javac.v8.parser.*;


/**
 * This class provides a commandline interface to the GJC compiler.
 */
public class Main {

    /**
     * For testing: enter any options you want to be set implicitly
     *  here.
     */
    static String[] forcedOpts = {};

    /**
     * The name of the compiler, for use in diagnostics.
     */
    String ownName;

    /**
     * The writer to use for diagnostic output.
     */
    PrintWriter out;

    /**
     * Result codes.
     */
    static final int EXIT_OK = 0;

    /**
     * Result codes.
     */
    static final int EXIT_ERROR = 1;

    /**
     * Result codes.
     */
    static final int EXIT_CMDERR = 2;

    /**
     * Result codes.
     */
    static final int EXIT_SYSERR = 3;

    /**
     * Result codes.
     */
    static final int EXIT_ABNORMAL = 4;

    /**
     * This class represents an option recognized by the main program
     */
    private class Option {

        /**
         * Option string.
         */
        String name;

        /**
         * Documentation key for arguments.
         */
        String argsNameKey;

        /**
         * Documentation key for description.
         */
        String descrKey;

        Option(String name, String argsNameKey, String descrKey) {
            super();
            this.name = name;
            this.argsNameKey = argsNameKey;
            this.descrKey = descrKey;
        }

        Option(String name, String descrKey) {
            this(name, null, descrKey);
        }

        /**
          * Does this option take an operand?
          */
        boolean hasArg() {
            return argsNameKey != null;
        }

        /**
          * Does argument string match option pattern?
          *  @param arg        The command line argument string.
          */
        boolean matches(String arg) {
            return name.equals(arg);
        }

        /**
          * Print a line of documentation describing this standard option.
          */
        void help() {
            String s = "  " + helpSynopsis();
            out.print(s);
            for (int j = s.length(); j < 28; j++)
                out.print(" ");
            Log.printLines(out, getLocalizedString(descrKey));
        }

        String helpSynopsis() {
            String s = name + " ";
            if (argsNameKey != null)
                s += getLocalizedString(argsNameKey);
            return s;
        }

        /**
          * Print a line of documentation describing this standard or extended options.
          */
        void xhelp() {
            help();
        }

        /**
          * Process the option (with arg). Return true if error detected.
          */
        boolean process(String option, String arg) {
            options.put(option, arg);
            return false;
        }

        /**
          * Process the option (without arg). Return true if error detected.
          */
        boolean process(String option) {
            return process(option, option);
        }
    }
    {
    }

    /**
      * A nonstandard or extended (-X) option
      */
    private class XOption extends Option {

        XOption(String name, String argsNameKey, String descrKey) {
            super(name, argsNameKey, descrKey);
        }

        XOption(String name, String descrKey) {
            this(name, null, descrKey);
        }

        void help() {
        }

        void xhelp() {
            super.help();
        }
    }
    {
    }

    /**
      * A hidden (implementor) option
      */
    private class HiddenOption extends Option {

        HiddenOption(String name) {
            super(name, null, null);
        }

        void help() {
        }

        void xhelp() {
        }
    }
    {
    }
    private Option[] recognizedOptions = {new Option("-g", "opt.g"),
    new Option("-g:none", "opt.g.none") {


                                              boolean process(String option) {
                                                  options.put("-g:", "none");
                                                  return false;
                                              }
                                          }
                                          , new Option("-g:{lines,vars,source}", "opt.g.lines.vars.source") {


                                              boolean matches(String s) {
                                                  return s.startsWith("-g:");
                                              }

                                              boolean process(String option) {
                                                  String suboptions =
                                                          option.substring(3);
                                                  options.put("-g:", suboptions);
                                              for (StringTokenizer t =
                                                      new StringTokenizer(
                                                      suboptions, ",");
                                                      t.hasMoreTokens();) {
                                                      String tok = t.nextToken();
                                                      String opt = "-g:" + tok;
                                                      options.put(opt, opt);
                                                  }
                                                  return false;
                                              }
                                          }
                                          , new Option("-nowarn", "opt.nowarn"),
                                          new Option("-verbose", "opt.verbose"),
                                          new Option("-deprecation",
                                                  "opt.deprecation"),
                                          new Option("-classpath", "opt.arg.path",
                                                  "opt.classpath"),
                                          new Option("-sourcepath", "opt.arg.path",
                                                  "opt.sourcepath"),
                                          new Option("-bootclasspath", "opt.arg.path",
                                                  "opt.bootclasspath"),
                                          new Option("-extdirs", "opt.arg.dirs",
                                                  "opt.extdirs"),
                                          new Option("-d", "opt.arg.directory",
                                                  "opt.d"),
                                          new Option("-encoding", "opt.arg.encoding",
                                                  "opt.encoding"),
                                          new Option("-source", "opt.arg.release",
                                                  "opt.source") {


                                              boolean process(String option,
                                                      String operand) {
                                                  Source source =
                                                          Source.lookup(operand);
                                              if (source == null) {
                                                      error("err.invalid.source",
                                                              operand);
                                                      return true;
                                                  }
                                                  boolean err = super.process(option,
                                                          operand);
                                                  String targetString =
                                                          (String) options.get("-target");
                                                  Target target = (targetString !=
                                                          null) ? Target.lookup(
                                                          targetString) : null;
                                              if (source.ordinal >=
                                                      Source.JDK1_4.ordinal) {
                                                  if (target != null) {
                                                      if (target.ordinal <
                                                              Target.JDK1_4.ordinal) {
                                                              error("err.source.target.conflict");
                                                              return true;
                                                          }
                                                  } else {
                                                          options.put("-target",
                                                                  Target.JDK1_4.name);
                                                      }
                                                  }
                                                  return err;
                                              }
                                          }
                                          , new Option("-target", "opt.arg.release",
                                                  "opt.target") {


                                              boolean process(String option,
                                                      String operand) {
                                                  Target target =
                                                          Target.lookup(operand);
                                              if (target == null) {
                                                      error("err.invalid.target",
                                                              operand);
                                                      return true;
                                                  }
                                                  boolean err = super.process(option,
                                                          operand);
                                                  String sourceString =
                                                          (String) options.get("-source");
                                                  Source source = (sourceString !=
                                                          null) ? Source.lookup(
                                                          sourceString) : null;
                                              if (source != null && target != null &&
                                                      source.ordinal >=
                                                      Source.JDK1_4.ordinal &&
                                                      target.ordinal <
                                                      Target.JDK1_4.ordinal) {
                                                      error("err.source.target.conflict");
                                                      return true;
                                                  }
                                                  return err;
                                              }
                                          }
                                          , new Option("-help", "opt.help") {


                                              boolean process(String option) {
                                                  Main.this.help();
                                                  return super.process(option);
                                              }
                                          }
                                          , new XOption("-Xbootclasspath/p:", "opt.arg.path",
                                                  "opt.Xbootclasspath.p") {


                                              boolean matches(String arg) {
                                                  return arg.startsWith(name);
                                              }

                                              String helpSynopsis() {
                                                  return name + getLocalizedString(
                                                          argsNameKey);
                                              }

                                              boolean hasArg() {
                                                  return false;
                                              }

                                              boolean process(String option) {
                                                  return super.process(name,
                                                          option.substring(
                                                          name.length()));
                                              }
                                          }
                                          , new XOption("-Xmaxerrs", "opt.arg.number",
                                                  "opt.maxerrs"),
                                          new XOption("-Xmaxwarns", "opt.arg.number",
                                                  "opt.maxwarns"),
                                          new XOption("-Xswitchcheck",
                                                  "opt.switchcheck"),
                                          new XOption("-Xstdout", "opt.arg.file",
                                                  "opt.Xstdout") {


                                              boolean process(String option,
                                                      String arg) {
                                              try {
                                                      out = new PrintWriter(
                                                              new FileWriter(arg)
                                                              , true);
                                              } catch (java.io.IOException e) {
                                                      error("err.error.writing.file",
                                                              arg, e.toString());
                                                      return true;
                                                  }
                                                  return super.process(option, arg);
                                              }
                                          }
                                          , new XOption("-X", "opt.X") {


                                              boolean process(String option) {
                                                  Main.this.xhelp();
                                                  return super.process(option);
                                              }
                                          }
                                          , new HiddenOption("-O"),
                                          new HiddenOption("-Xjcov"),
                                          new HiddenOption("-XD") {

                                              String s;

                                              boolean matches(String s) {
                                                  this.s = s;
                                                  return s.startsWith(name);
                                              }

                                              boolean process(String option) {
                                                  s = s.substring(name.length());
                                                  int eq = s.indexOf('=');
                                                  String key = (eq < 0) ? s :
                                                          s.substring(0, eq);
                                                  String value = (eq < 0) ? s :
                                                          s.substring(eq + 1);
                                                  options.put(key, value);
                                                  return false;
                                              }
                                          }
                                          , new HiddenOption("sourcefile") {

                                              String s;

                                              boolean matches(String s) {
                                                  this.s = s;
                                                  return s.endsWith(".java");
                                              }

                                              boolean process(String option) {
                                                  filenames.append(s);
                                                  return false;
                                              }
                                          }
                                         };

    /**
     * Construct a compiler instance.
     */
    public Main(String name) {
        this(name, new PrintWriter(System.err, true));
    }

    /**
      * Construct a compiler instance.
      */
    public Main(String name, PrintWriter out) {
        super();
        this.ownName = name;
        this.out = out;
    }

    /**
      * A table of all options that's passed to the JavaCompiler constructor.
      */
    private Options options = null;

    /**
     * The list of files to process
     */
    ListBuffer filenames = null;

    /**
     * Print a string that explains usage.
     */
    void help() {
        Log.printLines(out, getLocalizedString("msg.usage.header", ownName));
        for (int i = 0; i < recognizedOptions.length; i++) {
            recognizedOptions[i].help();
        }
        out.println();
    }

    /**
      * Print a string that explains usage for X options.
      */
    void xhelp() {
        for (int i = 0; i < recognizedOptions.length; i++) {
            recognizedOptions[i].xhelp();
        }
        out.println();
        Log.printLines(out, getLocalizedString("msg.usage.nonstandard.footer"));
    }

    /**
      * Report a usage error.
      */
    void error(String key) {
        Log.printLines(out, ownName + ": " + getLocalizedString(key, null, null));
        help();
    }

    /**
      * Report a usage error.
      */
    void error(String key, String arg) {
        Log.printLines(out, ownName + ": " + getLocalizedString(key, arg, null));
        help();
    }

    /**
      * Report a usage error.
      */
    void error(String key, String arg0, String arg1) {
        Log.printLines(out, ownName + ": " + getLocalizedString(key, arg0, arg1));
        help();
    }

    /**
      * Process command line arguments: store all command line options
      *  in `options' table and return all source filenames.
      *  @param args    The array of command line arguments.
      */
    protected List processArgs(String[] flags) {
        int ac = 0;
        while (ac < flags.length) {
            String flag = flags[ac];
            ac++;
            int j;
            for (j = 0; j < recognizedOptions.length; j++)
                if (recognizedOptions[j].matches(flag))
                    break;
            if (j == recognizedOptions.length) {
                error("err.invalid.flag", flag);
                return null;
            }
            Option option = recognizedOptions[j];
            if (option.hasArg()) {
                if (ac == flags.length) {
                    error("err.req.arg", flag);
                    return null;
                }
                String operand = flags[ac];
                ac++;
                if (option.process(flag, operand))
                    return null;
            } else {
                if (option.process(flag))
                    return null;
            }
        }
        return filenames.toList();
    }

    /**
      * Programmatic interface for main function.
      * @param args    The command line parameters.
      */
    public int compile(String[] args) {
        Context context = new Context();
        options = Options.instance(context);
        filenames = new ListBuffer();
        JavaCompiler comp = null;
        try {
            if (args.length == 0) {
                help();
                return EXIT_CMDERR;
            }
            processArgs(forcedOpts);
            List filenames;
            try {
                filenames = processArgs(CommandLine.parse(args));
                if (filenames == null) {
                    return EXIT_CMDERR;
                } else if (filenames.isEmpty()) {
                    if (options.get("-help") != null || options.get("-X") != null)
                        return EXIT_OK;
                    error("err.no.source.files");
                    return EXIT_CMDERR;
                }
            } catch (java.io.FileNotFoundException e) {
                Log.printLines(out,
                        ownName + ": " +
                        getLocalizedString("err.file.not.found", e.getMessage()));
                return EXIT_SYSERR;
            }
            context.put(Log.outKey, out);
            comp = JavaCompiler.make(context);
            if (comp == null)
                return EXIT_SYSERR;
            List cs = comp.compile(filenames);
            if (comp.errorCount() != 0)
                return EXIT_ERROR;
        } catch (IOException ex) {
            ioMessage(ex);
            return EXIT_SYSERR;
        }
        catch (OutOfMemoryError ex) {
            resourceMessage(ex);
            return EXIT_SYSERR;
        }
        catch (StackOverflowError ex) {
            resourceMessage(ex);
            return EXIT_SYSERR;
        }
        catch (FatalError ex) {
            feMessage(ex);
            return EXIT_SYSERR;
        }
        catch (Throwable ex) {
            bugMessage(ex);
            return EXIT_ABNORMAL;
        }
        finally { if (comp != null)
                      comp.close();
                      filenames = null;
                      options = null;
                    } return EXIT_OK;
    }

    /**
      * Print a message reporting an internal error.
      */
    void bugMessage(Throwable ex) {
        Log.printLines(out, getLocalizedString("msg.bug", JavaCompiler.version()));
        ex.printStackTrace(out);
    }

    /**
      * Print a message reporting an fatal error.
      */
    void feMessage(Throwable ex) {
        Log.printLines(out, ex.getMessage());
    }

    /**
      * Print a message reporting an input/output error.
      */
    void ioMessage(Throwable ex) {
        Log.printLines(out, getLocalizedString("msg.io"));
        ex.printStackTrace(out);
    }

    /**
      * Print a message reporting an out-of-resources error.
      */
    void resourceMessage(Throwable ex) {
        Log.printLines(out, getLocalizedString("msg.resource"));
        ex.printStackTrace(out);
    }

    /**
      * Find a localized string in the resource bundle.
      *  @param key     The key for the localized string.
      */
    private static String getLocalizedString(String key) {
        return getText("javac." + key, null, null);
    }

    private static String getLocalizedString(String key, String arg0) {
        return getText("javac." + key, arg0, null);
    }

    private static String getLocalizedString(String key, String arg0, String arg1) {
        return getText("javac." + key, arg0, arg1);
    }
    private static final String javacRB = "com.sun.tools.javac.v8.resources.javac";
    private static ResourceBundle messageRB;

    /**
     * Initialize ResourceBundle.
     */
    private static void initResource() {
        try {
            messageRB = ResourceBundle.getBundle(javacRB);
        } catch (MissingResourceException e) {
            Error x = new FatalError("Fatal Error: Resource for javac is missing");
            x.initCause(e);
            throw x;
        }
    }

    /**
      * Get and format message string from resource.
      */
    private static String getText(String key, String arg0, String arg1) {
        if (messageRB == null)
            initResource();
        try {
            String[] args = {arg0, arg1};
            return MessageFormat.format(messageRB.getString(key), args);
        } catch (MissingResourceException e) {
            if (arg0 == null)
                arg0 = "null";
            if (arg1 == null)
                arg1 = "null";
            String[] args = {key, arg0, arg1};
            String msg = "javac message file broken: key={0} arguments={1}, {2}";
            return MessageFormat.format(msg, args);
        }
    }
}
