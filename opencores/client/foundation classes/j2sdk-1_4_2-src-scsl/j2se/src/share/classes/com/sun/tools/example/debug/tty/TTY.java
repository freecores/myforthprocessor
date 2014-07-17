/*
 * @(#)TTY.java	1.54 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
/*
 * Copyright (c) 1997-1999 by Sun Microsystems, Inc. All Rights Reserved.
 * 
 * Sun grants you ("Licensee") a non-exclusive, royalty free, license to use,
 * modify and redistribute this software in source and binary code form,
 * provided that i) this copyright notice and license appear on all copies of
 * the software; and ii) Licensee does not utilize the software in a manner
 * which is disparaging to Sun.
 * 
 * This software is provided "AS IS," without a warranty of any kind. ALL
 * EXPRESS OR IMPLIED CONDITIONS, REPRESENTATIONS AND WARRANTIES, INCLUDING ANY
 * IMPLIED WARRANTY OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NON-INFRINGEMENT, ARE HEREBY EXCLUDED. SUN AND ITS LICENSORS SHALL NOT BE
 * LIABLE FOR ANY DAMAGES SUFFERED BY LICENSEE AS A RESULT OF USING, MODIFYING
 * OR DISTRIBUTING THE SOFTWARE OR ITS DERIVATIVES. IN NO EVENT WILL SUN OR ITS
 * LICENSORS BE LIABLE FOR ANY LOST REVENUE, PROFIT OR DATA, OR FOR DIRECT,
 * INDIRECT, SPECIAL, CONSEQUENTIAL, INCIDENTAL OR PUNITIVE DAMAGES, HOWEVER
 * CAUSED AND REGARDLESS OF THE THEORY OF LIABILITY, ARISING OUT OF THE USE OF
 * OR INABILITY TO USE SOFTWARE, EVEN IF SUN HAS BEEN ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGES.
 * 
 * This software is not designed or intended for use in on-line control of
 * aircraft, air traffic, aircraft navigation or aircraft communications; or in
 * the design, construction, operation or maintenance of any nuclear
 * facility. Licensee represents and warrants that it will not use or
 * redistribute the Software for such purposes.
 */

package com.sun.tools.example.debug.tty;

import com.sun.jdi.*;
import com.sun.jdi.event.*;
import com.sun.jdi.connect.*;

import java.util.*;
import java.io.*;

public class TTY implements EventNotifier {
    EventHandler handler = null;

    /**
     * List of Strings to execute at each stop.
     */
    private List monitorCommands = new ArrayList();
    private int monitorCount = 0;

    /**
     * The name of this tool.
     */
    private static final String progname = "jdb";

    /**
     * The version date of this tool.  Formats to:
     *      April 19, 2002 7:42:13 PM PDT (when locale=en_US.ISO8859-1)
     */
    private static final long version = 1019270533724L;

    public void vmStartEvent(VMStartEvent se)  {
        Thread.yield();  // fetch output
        MessageOutput.lnprint("VM Started:");
    }

    public void vmDeathEvent(VMDeathEvent e)  {
    }

    public void vmDisconnectEvent(VMDisconnectEvent e)  {
    }

    public void threadStartEvent(ThreadStartEvent e)  {
    }

    public void threadDeathEvent(ThreadDeathEvent e)  {
    }

    public void classPrepareEvent(ClassPrepareEvent e)  {
    }

    public void classUnloadEvent(ClassUnloadEvent e)  {
    }

    public void breakpointEvent(BreakpointEvent be)  {
        Thread.yield();  // fetch output
        MessageOutput.lnprint("Breakpoint hit:");
    }

    public void fieldWatchEvent(WatchpointEvent fwe)  {
        Field field = fwe.field();
        ObjectReference obj = fwe.object();
        Thread.yield();  // fetch output

        if (fwe instanceof ModificationWatchpointEvent) {
            MessageOutput.lnprint("Field access encountered before after",
                                  new Object [] {field,
                                                 fwe.valueCurrent(),
                                                 ((ModificationWatchpointEvent)fwe).valueToBe()});
        } else {
            MessageOutput.lnprint("Field access encountered", field.toString());
        }
    }

    public void stepEvent(StepEvent se)  {
        Thread.yield();  // fetch output
        MessageOutput.lnprint("Step completed:");
    }

    public void exceptionEvent(ExceptionEvent ee) {
        Thread.yield();  // fetch output
        Location catchLocation = ee.catchLocation();
        if (catchLocation == null) {
            MessageOutput.lnprint("Exception occurred uncaught",
                                  ee.exception().referenceType().name());
        } else {
            MessageOutput.lnprint("Exception occurred caught",
                                  new Object [] {ee.exception().referenceType().name(),
                                                 Commands.locationString(catchLocation)});
        }
    }

    public void methodEntryEvent(MethodEntryEvent me) {
        Thread.yield();  // fetch output
        /*
         * These can be very numerous, so be as efficient as possible.
         */
        MessageOutput.lnprint("Method entered:",
                              new Object [] {me.method().declaringType().name(),
                                             me.method().name()});
    }

    public void methodExitEvent(MethodExitEvent me) {
        Thread.yield();  // fetch output
        /*
         * These can be very numerous, so be as efficient as possible.
         */
        MessageOutput.lnprint("Method exited:",
                              new Object [] {me.method().declaringType().name(),
                                             me.method().name()});
    }

    public void vmInterrupted() {
        Thread.yield();  // fetch output
        printCurrentLocation();
        Iterator it = monitorCommands.iterator();
        while (it.hasNext()) {
            StringTokenizer t = new StringTokenizer((String)it.next());
            t.nextToken();  // get rid of monitor number
            executeCommand(t);
        }
        MessageOutput.printPrompt();
    }

    public void receivedEvent(Event event) {
    }

    private void printCurrentLocation() {
        ThreadInfo threadInfo = ThreadInfo.getCurrentThreadInfo();
        StackFrame frame;
        try {
            frame = threadInfo.getCurrentFrame();
        } catch (IncompatibleThreadStateException exc) {
            MessageOutput.println("<location unavailable>");
            return;
        }
        if (frame == null) {
            MessageOutput.println("No frames on the current call stack");
        } else {
            Location loc = frame.location();
            MessageOutput.println("location",
                                  new Object [] {threadInfo.getThread().name(),
                                                 Commands.locationString(loc)});
            // Output the current source line, if possible
            if (loc.lineNumber() != -1) {
                String line;
                try {
                    line = Env.sourceLine(loc, loc.lineNumber());
                } catch (java.io.IOException e) {
                    line = null;
                }
                if (line != null) {
                    MessageOutput.println("source line number and line",
                                          new Object [] {new Integer(loc.lineNumber()),
                                                         line});
                }
            }
        }
        MessageOutput.println();
    }

    void help() {
        MessageOutput.println("zz help text");
    }

    /*
     * These commands do not require a connected VM
     */
    private String[]  disconnectCmds = {
        "run", "catch", "ignore", "stop", "clear", "watch", "unwatch",
        "use", "sourcepath", "quit", "exit", "help", "?", "read",
        "version", "exclude"
    };

    private boolean isDisconnectCmd(String cmd) {
        for (int i = 0; i < disconnectCmds.length; i++) {
            if (disconnectCmds[i].equals(cmd)) {
                return true;
            }
        }
        return false;
    }

    void executeCommand(StringTokenizer t) {
        String cmd = t.nextToken().toLowerCase();
        Commands evaluator = new Commands();

        // Normally, prompt for the next command after this one is done
        boolean showPrompt = true;

        /*
         * Do this check ahead of time so that it precedes any syntax
         * checking in the command itself. If the command is not
         * properly classified by isDisconnectCmd, the exception catch
         * below will still protect us. 
         */
        if (!Env.connection().isOpen() && !isDisconnectCmd(cmd)) {
            MessageOutput.println("Command not valid until the VM is started with the run command",
                         cmd);
        } else {
            try {
                if (cmd.equals("print")) {
                    evaluator.commandPrint(t, false);
                    showPrompt = false;        // asynchronous command
                } else if (cmd.equals("eval")) {
                    evaluator.commandPrint(t, false);
                    showPrompt = false;        // asynchronous command
                } else if (cmd.equals("set")) {
                    evaluator.commandSet(t);
                    showPrompt = false;        // asynchronous command
                } else if (cmd.equals("dump")) {
                    evaluator.commandPrint(t, true);
                    showPrompt = false;        // asynchronous command
                } else if (cmd.equals("locals")) {
                    evaluator.commandLocals();
                } else if (cmd.equals("classes")) {
                    evaluator.commandClasses();
                } else if (cmd.equals("class")) {
                    evaluator.commandClass(t);
                } else if (cmd.equals("methods")) {
                    evaluator.commandMethods(t);
                } else if (cmd.equals("fields")) {
                    evaluator.commandFields(t);
                } else if (cmd.equals("threads")) {
                    evaluator.commandThreads(t);
                } else if (cmd.equals("thread")) {
                    evaluator.commandThread(t);
                } else if (cmd.equals("suspend")) {
                    evaluator.commandSuspend(t);
                } else if (cmd.equals("resume")) {
                    evaluator.commandResume(t);
                } else if (cmd.equals("cont")) {
                    evaluator.commandCont();
                } else if (cmd.equals("threadgroups")) {
                    evaluator.commandThreadGroups();
                } else if (cmd.equals("threadgroup")) {
                    evaluator.commandThreadGroup(t);
                } else if (cmd.equals("catch")) {
                    evaluator.commandCatchException(t);
                } else if (cmd.equals("ignore")) {
                    evaluator.commandIgnoreException(t);
                } else if (cmd.equals("step")) {
                    evaluator.commandStep(t);
                } else if (cmd.equals("stepi")) {
                    evaluator.commandStepi();
                } else if (cmd.equals("next")) {
                    evaluator.commandNext();
                } else if (cmd.equals("kill")) {
                    evaluator.commandKill(t);
                } else if (cmd.equals("interrupt")) {
                    evaluator.commandInterrupt(t);
                } else if (cmd.equals("trace")) {
                    evaluator.commandTrace(t);
                } else if (cmd.equals("untrace")) {
                    evaluator.commandUntrace(t);
                } else if (cmd.equals("where")) {
                    evaluator.commandWhere(t, false);
                } else if (cmd.equals("wherei")) {
                    evaluator.commandWhere(t, true);
                } else if (cmd.equals("up")) {
                    evaluator.commandUp(t);
                } else if (cmd.equals("down")) {
                    evaluator.commandDown(t);
                } else if (cmd.equals("load")) {
                    evaluator.commandLoad(t);
                } else if (cmd.equals("run")) {
                    evaluator.commandRun(t);
                    /*
                     * Fire up an event handler, if the connection was just
                     * opened. Since this was done from the run command
                     * we don't stop the VM on its VM start event (so
                     * arg 2 is false).
                     */
                    if ((handler == null) && Env.connection().isOpen()) {
                        handler = new EventHandler(this, false);
                    }
                } else if (cmd.equals("memory")) {
                    evaluator.commandMemory();
                } else if (cmd.equals("gc")) {
                    evaluator.commandGC();
                } else if (cmd.equals("stop")) {
                    evaluator.commandStop(t);
                } else if (cmd.equals("clear")) {
                    evaluator.commandClear(t);
                } else if (cmd.equals("watch")) {
                    evaluator.commandWatch(t);
                } else if (cmd.equals("unwatch")) {
                    evaluator.commandUnwatch(t);
                } else if (cmd.equals("list")) {
                    evaluator.commandList(t);
                } else if (cmd.equals("lines")) { // Undocumented command: useful for testing.
                    evaluator.commandLines(t);
                } else if (cmd.equals("classpath")) {
                    evaluator.commandClasspath(t);
                } else if (cmd.equals("use") || cmd.equals("sourcepath")) {
                    evaluator.commandUse(t);
                } else if (cmd.equals("monitor")) {
                    monitorCommand(t);
                } else if (cmd.equals("unmonitor")) {
                    unmonitorCommand(t);
                } else if (cmd.equals("lock")) {
                    evaluator.commandLock(t);
                    showPrompt = false;        // asynchronous command
                } else if (cmd.equals("threadlocks")) {
                    evaluator.commandThreadlocks(t);
                } else if (cmd.equals("disablegc")) {
                    evaluator.commandDisableGC(t);
                    showPrompt = false;        // asynchronous command
                } else if (cmd.equals("enablegc")) {
                    evaluator.commandEnableGC(t);
                    showPrompt = false;        // asynchronous command
                } else if (cmd.equals("save")) { // Undocumented command: useful for testing.
                    evaluator.commandSave(t);
                    showPrompt = false;        // asynchronous command
                } else if (cmd.equals("bytecodes")) { // Undocumented command: useful for testing.
                    evaluator.commandBytecodes(t);
                } else if (cmd.equals("redefine")) {
                    evaluator.commandRedefine(t);
                } else if (cmd.equals("pop")) {
                    evaluator.commandPopFrames(t, false);
                } else if (cmd.equals("reenter")) {
                    evaluator.commandPopFrames(t, true);
                } else if (cmd.equals("extension")) {
                    evaluator.commandExtension(t);
                } else if (cmd.equals("exclude")) {
                    evaluator.commandExclude(t);
                } else if (cmd.equals("read")) {
                    readCommand(t);
                } else if (cmd.equals("help") || cmd.equals("?")) {
                    help();
                } else if (cmd.equals("version")) {
                    evaluator.commandVersion(progname, version);
                } else if (cmd.equals("quit") || cmd.equals("exit")) {
                    if (handler != null) {
                        handler.shutdown();
                    }
                    Env.shutdown(); 
                } else {
                    if (t.hasMoreTokens()) {
                        try {            
                            int repeat = Integer.parseInt(cmd);
                            String subcom = t.nextToken("");
                            while (repeat-- > 0) {
                                executeCommand(new StringTokenizer(subcom));
                            }
                            return;
                        } catch (NumberFormatException exc) {
                        }
                    }
                    MessageOutput.println("Unrecognized command.  Try help...");
                }
            } catch (UnsupportedOperationException uoe) {
                MessageOutput.println("Command is not supported on the target VM", cmd);
            } catch (VMNotConnectedException vmnse) {
                MessageOutput.println("Command not valid until the VM is started with the run command",
                             cmd);
            } catch (Exception e) {
                MessageOutput.printException("Internal exception:", e);
            }
        }

        if (showPrompt) {
            MessageOutput.printPrompt();
        }
    }

    /*
     * Maintain a list of commands to execute each time the VM is suspended.
     */
    void monitorCommand(StringTokenizer t) {
        if (t.hasMoreTokens()) {
            ++monitorCount;
            monitorCommands.add(monitorCount + ": " + t.nextToken(""));
        } else {
            Iterator it = monitorCommands.iterator();
            while (it.hasNext()) {
                MessageOutput.printDirectln((String)it.next());// Special case: use printDirectln()
            }
        }            
    }

    void unmonitorCommand(StringTokenizer t) {
        if (t.hasMoreTokens()) {
            String monTok = t.nextToken();
            int monNum;
            try {            
                monNum = Integer.parseInt(monTok);
            } catch (NumberFormatException exc) {
                MessageOutput.println("Not a monitor number:", monTok);
                return;
            }
            String monStr = monTok + ":";
            Iterator it = monitorCommands.iterator();
            while (it.hasNext()) {
                String cmd = (String)it.next();
                StringTokenizer ct = new StringTokenizer(cmd);
                if (ct.nextToken().equals(monStr)) {
                    monitorCommands.remove(cmd);
                    MessageOutput.println("Unmonitoring", cmd);
                    return;
                }
            }
            MessageOutput.println("No monitor numbered:", monTok);
        } else {
            MessageOutput.println("Usage: unmonitor <monitor#>");
        }
    }


    void readCommand(StringTokenizer t) {
        if (t.hasMoreTokens()) {
            String cmdfname = t.nextToken();
            if (!readCommandFile(cmdfname)) {
                MessageOutput.println("Could not open:", cmdfname);
            }
        } else {
            MessageOutput.println("Usage: read <command-filename>");
        }
    }

    /**
     * Read and execute a command file.  Return true if the
     * file could be opened.
     */
    boolean readCommandFile(String filename) {
        File f = new File(filename);
        BufferedReader inFile = null;
        try {
            if (f.canRead()) {
                MessageOutput.println("*** Reading commands from", f.getCanonicalPath());
                // Process initial commands.
                inFile = new BufferedReader(new FileReader(f));
                String ln;
                while ((ln = inFile.readLine()) != null) {
                    StringTokenizer t = new StringTokenizer(ln);
                    if (t.hasMoreTokens()) {
                        executeCommand(t);
                    }
                }
            }
        } catch (IOException e) {
        } finally {
            if (inFile != null) {
                try {
                    inFile.close();
                } catch (Exception exc) {
                }
            }
        }
        return inFile != null;
    }

    public TTY() throws Exception {

        MessageOutput.println("Initializing progname", progname);
       
        if (Env.connection().isOpen()) {
            /*
             * Connection opened on startup. Start event handler
             * immediately, telling it (through arg 2) to stop on the 
             * VM start event.
             */
            this.handler = new EventHandler(this, true);
        }
        try {
            BufferedReader in = 
                    new BufferedReader(new InputStreamReader(System.in));
    
            String lastLine = null;
    
            Thread.currentThread().setPriority(Thread.NORM_PRIORITY);
    
            /*
             * Try reading user's home startup file. Handle Unix and 
             * and Win32 conventions for the names of these files. 
             */
            if (!readCommandFile(System.getProperty("user.home") + 
                                 File.separator + "jdb.ini")) {
                readCommandFile(System.getProperty("user.home") + 
                                File.separator + ".jdbrc");
            }
    
            // Try startup file in local directory
            if (!readCommandFile(System.getProperty("user.dir") + 
                                 File.separator + "jdb.ini")) {
                readCommandFile(System.getProperty("user.dir") + 
                                File.separator + ".jdbrc");
            }
    
            // Process interactive commands.
            MessageOutput.printPrompt();
            while (true) {
                String ln = in.readLine();
                if (ln == null) {
                    MessageOutput.println("Input stream closed.");
                    return;
                }
    
                if (ln.startsWith("!!") && lastLine != null) {
                    ln = lastLine + ln.substring(2);
                    MessageOutput.printDirectln(ln);// Special case: use printDirectln()
                }
    
                StringTokenizer t = new StringTokenizer(ln);
                if (t.hasMoreTokens()) {
                    lastLine = ln;
                    executeCommand(t);
                } else {
                    MessageOutput.printPrompt();
                }
            }
        } catch (VMDisconnectedException e) {
            handler.handleDisconnectedException();
        }
    }

    private static void usage() {
        MessageOutput.println("zz usage text", new Object [] {progname,
                                                     File.pathSeparator});
        System.exit(1);
    }

    static void usageError(String messageKey) {
        MessageOutput.println(messageKey);
        MessageOutput.println();
        usage();
    }

    static void usageError(String messageKey, String argument) {
        MessageOutput.println(messageKey, argument);
        MessageOutput.println();
        usage();
    }

    private static Connector findConnector(String transportName, List availableConnectors) {
        Iterator iter = availableConnectors.iterator();
        while (iter.hasNext()) {
            Connector connector = (Connector)iter.next();
            if (connector.transport().name().equals(transportName)) {
                return connector;
            }
        }

        // not found
        throw new IllegalArgumentException(MessageOutput.format("Invalid transport name:",
                                                                transportName));
    }

    private static boolean supportsSharedMemory() {
        List connectors = Bootstrap.virtualMachineManager().allConnectors();
        Iterator iter = connectors.iterator();
        while (iter.hasNext()) {
            Connector connector = (Connector)iter.next();
            if (connector.transport().name().equals("dt_shmem")) {
                return true;
            }
        }
        return false;
    }

    private static String addressToSocketArgs(String address) {
        int index = address.indexOf(':');
        if (index != -1) {
            String hostString = address.substring(0, index);
            String portString = address.substring(index + 1);
            return "hostname=" + hostString + ",port=" + portString;
        } else {
            return "port=" + address;
        }
    }

    private static boolean hasWhitespace(String string) {
        int length = string.length();
        for (int i = 0; i < length; i++) {
            if (Character.isWhitespace(string.charAt(i))) {
                return true;
            }
        }
        return false;
    }

    private static String addArgument(String string, String argument) {
        if (hasWhitespace(argument) || argument.indexOf(',') != -1) {
            // Quotes were stripped out for this argument, add 'em back. 
            StringBuffer buffer = new StringBuffer(string);
            buffer.append('"');
            for (int i = 0; i < argument.length(); i++) {
                char c = argument.charAt(i);
                if (c == '"') {
                    buffer.append('\\');
                }
                buffer.append(c);
            }
            buffer.append("\" ");
            return buffer.toString();
        } else {
            return string + argument + ' ';
        }
    }

    public static void main(String argv[]) throws MissingResourceException {
        String cmdLine = "";
        String javaArgs = "";
        int traceFlags = VirtualMachine.TRACE_NONE;
        boolean launchImmediately = false;
        String connectSpec = null;
        
        MessageOutput.textResources = ResourceBundle.getBundle
            ("com.sun.tools.example.debug.tty.TTYResources",
             Locale.getDefault());

        for (int i = 0; i < argv.length; i++) {
            String token = argv[i];
            if (token.equals("-dbgtrace")) {
                if ((i == argv.length - 1) ||
                    ! Character.isDigit(argv[i+1].charAt(0))) {
                    traceFlags = VirtualMachine.TRACE_ALL;
                } else {
                    String flagStr = "";
                    try {
                        flagStr = argv[++i];
                        traceFlags = Integer.decode(flagStr).intValue();
                    } catch (NumberFormatException nfe) {
                        usageError("dbgtrace flag value must be an integer:",
                                   flagStr);
                        return;                
                    }
                }
            } else if (token.equals("-X")) {
                usageError("Use java minus X to see");
                return;
            } else if (
                   // Standard VM options passed on
                   token.equals("-v") || token.startsWith("-v:") ||  // -v[:...]
                   token.startsWith("-verbose") ||                  // -verbose[:...]
                   token.startsWith("-D") ||
                   // -classpath handled below
                   // NonStandard options passed on
                   token.startsWith("-X") ||
                   // Old-style options (These should remain in place as long as
                   //  the standard VM accepts them)
                   token.equals("-noasyncgc") || token.equals("-prof") ||
                   token.equals("-verify") || token.equals("-noverify") ||
                   token.equals("-verifyremote") ||
                   token.equals("-verbosegc") ||
                   token.startsWith("-ms") || token.startsWith("-mx") ||
                   token.startsWith("-ss") || token.startsWith("-oss") ) {

                javaArgs = addArgument(javaArgs, token);
            } else if (token.equals("-tclassic")) {
                usageError("Classic VM no longer supported.");
                return;
            } else if (token.equals("-tclient")) {
                // -client must be the first one
                javaArgs = "-client " + javaArgs;
            } else if (token.equals("-tserver")) {
                // -server must be the first one
                javaArgs = "-server " + javaArgs;
            } else if (token.equals("-sourcepath")) {
                if (i == (argv.length - 1)) {
                    usageError("No sourcepath specified.");
                    return;
                }
                Env.setSourcePath(argv[++i]);
            } else if (token.equals("-classpath")) {
                if (i == (argv.length - 1)) {
                    usageError("No classpath specified.");
                    return;
                }
                javaArgs = addArgument(javaArgs, token);
                javaArgs = addArgument(javaArgs, argv[++i]);
            } else if (token.equals("-attach")) {
                if (connectSpec != null) {
                    usageError("cannot redefine existing connection", token);
                    return;
                }
                if (i == (argv.length - 1)) {
                    usageError("No attach address specified.");
                    return;
                }
                String address = argv[++i];

                /*
                 * -attach is shorthand for one of the reference implementation's
                 * attaching connectors. Use the shared memory attach if it's
                 * available; otherwise, use sockets. Build a connect 
                 * specification string based on this decision.
                 */
                if (supportsSharedMemory()) {
                    connectSpec = "com.sun.jdi.SharedMemoryAttach:name=" + 
                                   address;
                } else {
                    String suboptions = addressToSocketArgs(address);
                    connectSpec = "com.sun.jdi.SocketAttach:" + suboptions;
                }
            } else if (token.equals("-listen") || token.equals("-listenany")) {
                if (connectSpec != null) {
                    usageError("cannot redefine existing connection", token);
                    return;
                }
                String address = null;
                if (token.equals("-listen")) {
                    if (i == (argv.length - 1)) {
                        usageError("No attach address specified.");
                        return;
                    }
                    address = argv[++i];
                }

                /*
                 * -listen[any] is shorthand for one of the reference implementation's
                 * listening connectors. Use the shared memory listen if it's
                 * available; otherwise, use sockets. Build a connect 
                 * specification string based on this decision.
                 */
                if (supportsSharedMemory()) {
                    connectSpec = "com.sun.jdi.SharedMemoryListen";
                    if (address != null) {
                        connectSpec += (":name=" + address);
                    }
                } else {
                    String suboptions = addressToSocketArgs(address);
                    connectSpec = "com.sun.jdi.SocketListen";
                    if (address != null) {
                        connectSpec += (":port=" + address);
                    }
                }
            } else if (token.equals("-launch")) {
                launchImmediately = true;
            } else if (token.equals("-connect")) {
                /*
                 * -connect allows the user to pick the connector
                 * used in bringing up the target VM. This allows 
                 * use of connectors other than those in the reference
                 * implementation.
                 */
                if (connectSpec != null) {
                    usageError("cannot redefine existing connection", token);
                    return;
                }
                if (i == (argv.length - 1)) {
                    usageError("No connect specification.");
                    return;
                }
                connectSpec = argv[++i];
            } else if (token.equals("-help")) {
                usage();
            } else if (token.equals("-version")) {
                Commands evaluator = new Commands();                
                evaluator.commandVersion(progname, version);
                System.exit(0);
            } else if (token.startsWith("-")) {
                usageError("invalid option", token);
                return;
            } else {
                // Everything from here is part of the command line
                cmdLine = addArgument("", token);
                for (i++; i < argv.length; i++) {
                    cmdLine = addArgument(cmdLine, argv[i]);
                }
                break;
            }
        }

        /*
         * Unless otherwise specified, set the default connect spec.
	 */

        /*
         * Here are examples of jdb command lines and how the options
	 * are interpreted as arguments to the program being debugged.
	 * arg1       arg2
	 * ----       ----
	 * jdb hello a b       a          b
	 * jdb hello "a b"     a b
	 * jdb hello a,b       a,b
	 * jdb hello a, b      a,         b
	 * jdb hello "a, b"    a, b
	 * jdb -connect "com.sun.jdi.CommandLineLaunch:main=hello  a,b"   illegal
	 * jdb -connect  com.sun.jdi.CommandLineLaunch:main=hello "a,b"   illegal
	 * jdb -connect 'com.sun.jdi.CommandLineLaunch:main=hello "a,b"'  arg1 = a,b
	 * jdb -connect 'com.sun.jdi.CommandLineLaunch:main=hello "a b"'  arg1 = a b
	 * jdb -connect 'com.sun.jdi.CommandLineLaunch:main=hello  a b'   arg1 = a  arg2 = b
	 * jdb -connect 'com.sun.jdi.CommandLineLaunch:main=hello "a," b' arg1 = a, arg2 = b
	 */
        if (connectSpec == null) {
            connectSpec = "com.sun.jdi.CommandLineLaunch:";
        } else if (!connectSpec.endsWith(",") && !connectSpec.endsWith(":")) {
            connectSpec += ","; // (Bug ID 4285874)
        } 

        cmdLine = cmdLine.trim();
        javaArgs = javaArgs.trim();

        if (cmdLine.length() > 0) {
            if (!connectSpec.startsWith("com.sun.jdi.CommandLineLaunch:")) {
                usageError("Cannot specify command line with connector:",
                           connectSpec);
                return;
            }
            connectSpec += "main=" + cmdLine + ",";
        } 

        if (javaArgs.length() > 0) {
            if (!connectSpec.startsWith("com.sun.jdi.CommandLineLaunch:")) {
                usageError("Cannot specify target vm arguments with connector:",
                           connectSpec);
                return;
            }
            connectSpec += "options=" + javaArgs + ",";
        } 

        try {
            if (! connectSpec.endsWith(",")) {
                connectSpec += ","; // (Bug ID 4285874)
            }
            Env.init(connectSpec, launchImmediately, traceFlags);
            new TTY();
        } catch(Exception e) {                
            MessageOutput.printException("Internal exception:", e);
        }
    }
}

