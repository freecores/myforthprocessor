/*
 * @(#)DebugServer.java	1.3 03/01/23 11:13:51
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot;

import sun.jvm.hotspot.debugger.*;
import sun.jvm.hotspot.debugger.dbx.*;
import sun.jvm.hotspot.runtime.*;
import sun.jvm.hotspot.oops.*;

public class DebugServer {
  private void usage() {
    System.out.println("usage: java " + getClass().getName() + " [pid]");
    System.out.println("   or: java " + getClass().getName() + " [executable] [core]");
    System.out.println("\"pid\" must be the process ID of a HotSpot process.");
    System.out.println("If reading a core file, \"executable\" must (currently) be the");
    System.out.println("full path name to the precise java executable which generated");
    System.out.println("the core file (not, on Solaris, the \"java\" wrapper script in");
    System.out.println("the \"bin\" subdirectory of the JDK.)");
    System.exit(1);
  }

  public static void main(String[] args) {
    new DebugServer().run(args);
  }

  private void run(String[] args) {
    if ((args.length < 1) || (args.length > 2)) {
      usage();
    }

    // Attempt to handle "-h" or "-help"
    if (args[0].startsWith("-")) {
      usage();
    }

    int pid = 0;
    boolean usePid = false;
    String coreFileName = null;
    // FIXME: would be nice to pick this up from the core file
    // somehow, but that doesn't look possible. Should at least figure
    // it out from a path to the JDK.
    String javaExecutableName = null;

    if (args.length == 1) {
      try {
        pid = Integer.parseInt(args[0]);
        usePid = true;
      }
      catch (NumberFormatException e) {
        usage();
      }
    } else {
      javaExecutableName = args[0];
      coreFileName = args[1];
    }

    HotSpotAgent agent = new HotSpotAgent();
    try {
      if (usePid) {
        System.err.println("Attaching to process ID " + pid + " and starting RMI services, please wait...");
        agent.startServer(pid);
      } else {
        System.err.println("Attaching to core " + coreFileName +
                           " from executable " + javaExecutableName + " and starting RMI services, please wait...");
        agent.startServer(javaExecutableName, coreFileName);
      }
    }
    catch (DebuggerException e) {
      if (usePid) {
        System.err.print("Error attaching to process or starting server: ");
      } else {
        System.err.print("Error attaching to core file or starting server: ");
      }
      if (e.getMessage() != null) {
        System.err.print(e.getMessage());
      }
      System.err.println();
      System.exit(1);
    }

    System.err.println("Debugger attached and RMI services started.");
  }
}
