/*
 * @(#)Debug.java	1.22 03/01/23
 * 
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.javaws.debug;
import java.io.PrintStream;
import com.sun.javaws.ui.general.GeneralUtilities;
import javax.swing.JOptionPane;
import java.net.Socket;
import java.io.*;
import com.sun.javaws.Main;


public class Debug {
    static PrintStream _logSockStream = null;
    // the workaroud for TCP flushing problem on windows
    //  bug #4396040
    //  used to check that TCK receive a message
    static DataInputStream _logResponseStream = null;
     
    public static final String TCK_PREAMBLE = "##TCKHarnesRun##";
  
    static synchronized void openSocketStream() {
        if (Globals.LogToHost != null && Globals.RedirectErrors) {
            /**
             * First we parse the options, note that IPv6 literal addresses must be
             * enclosed in [] ex: [ffff::abcd]:80
             */
            String hNamePort = Globals.LogToHost;
            String host=null;
            int port = -1;
            //Check to see if we have a literal IPv6 address
            int hostStartidx=0;
            int hostEndidx=0;
            if (hNamePort.charAt(0) == '[' && (hostEndidx = hNamePort.indexOf(1,']')) != -1) {
                hostStartidx = 1;
            } else {
                hostEndidx = hNamePort.indexOf(":");
            }
            host = hNamePort.substring(hostStartidx, hostEndidx);
            if (host == null) {
                System.err.println("Warning:Logging to logSocket disabled invalid host "+host);
                return;
            }
            try {
                String portS = (hNamePort.substring(hNamePort.lastIndexOf(':')+1));
                port = Integer.parseInt(portS);
            } catch (NumberFormatException nfe) {
                port = -1;
            }
            
            if (port < 0) {
                System.err.println("Warning:Logging to logSocket disabled invalid port "+port);
                return;
            }
        
            //Now we open the tcpSocket and the stream
            try {
                Socket logsocket = new Socket(host,port);
                _logSockStream = new PrintStream(
                        logsocket.getOutputStream());
                if (Globals.TCKResponse) {
                     _logResponseStream = new DataInputStream(
                             logsocket.getInputStream());
		}
	    } catch (java.io.IOException ioe) {
                Debug.println("Warning:Exceptions occurred, Logging to logSocket disabled "+hNamePort);
		ignoredException(ioe);
            }
            return;
        }
    }

    /**
     * println for tck harness only
     * "##TCKHarnessState##:%l:%d:%d:%s",datetime,pid,tid,state
     */
    public  static synchronized void tckprintln(String msg) {
        long datetime = System.currentTimeMillis();
        println(TCK_PREAMBLE+":"+datetime+":" +
                    (Runtime.getRuntime()).hashCode() + ":" + Thread.currentThread() + ":"
                    + msg );
        if (_logResponseStream != null) {
            try {
                   while(_logResponseStream.readLong()<datetime);
            } catch (java.io.IOException ioe) {
                System.err.println("Warning:Exceptions occurred, while logging to logSocket");
                ioe.printStackTrace(System.err);
            }
        }

    }

    /** Debugging output. Could get redirected */
    public static synchronized void println(String msg) {
	String newmsg = (Globals.TraceThreads ? (Thread.currentThread().getName() + ": " + msg) : msg);
        System.err.println(newmsg);
       
        if (_logSockStream != null) {
            _logSockStream.println(newmsg);
            _logSockStream.flush();
        }
        if (Globals.DebugWindow) {
            GeneralUtilities.showMessageDialog(null, newmsg, "Debug Window", JOptionPane.ERROR_MESSAGE);
        }
    }
    
    public static synchronized void print(String msg) {
	String newmsg = (Globals.TraceThreads ? (Thread.currentThread().getName() + ": " + msg) : msg);
        System.err.print(newmsg);
        if (_logSockStream != null) {
            _logSockStream.println(newmsg);
            _logSockStream.flush();
        }
        if (Globals.DebugWindow) {
            GeneralUtilities.showMessageDialog(null, newmsg, "Debug Window", JOptionPane.ERROR_MESSAGE);
        }
    }
    
    public static void fatal(String msg)  {
        println("Fatal error: " + msg);
        dumpStack();
        Main.systemExit(-1);
    }
    
    
    /** Simple assert mechanism. */
    public static void jawsAssert(boolean cond, String msg) {
        if (!cond) {
            println("Assert failure: " + msg);
            dumpStack();
	    Main.systemExit(-1);
        }
    }
    
    /** Method to printout ignored exceptions */
    public static void ignoredException(Throwable t) {
        if (Globals.TraceIgnoredExceptions) {
            println("### Ignored Throwable");
	    OutputStream baos = new ByteArrayOutputStream();
	    PrintStream ps = new PrintStream(baos);
            t.printStackTrace(ps);
	    println(baos.toString());
        }
    }
    
    public static void shouldNotReachHere() { fatal("shouldNotReachHere"); }
    public static void unimplemented()      { fatal("unimplemented"); }
    
    public static void dumpStack() { new Throwable().printStackTrace(System.err); }
    
    public static PrintStream getSocketStream() { return _logSockStream; }
}




