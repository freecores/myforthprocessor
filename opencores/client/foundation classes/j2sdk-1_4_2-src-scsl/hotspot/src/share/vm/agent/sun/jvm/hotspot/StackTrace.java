/*
 * @(#)StackTrace.java	1.10 03/01/23 11:14:14
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot;

import sun.jvm.hotspot.debugger.*;
import sun.jvm.hotspot.runtime.*;
import sun.jvm.hotspot.oops.*;

/** Traverses and prints the stack traces for all Java threads in the
    remote VM */
public class StackTrace {
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
    new StackTrace().run(args);
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
        System.err.println("Attaching to process ID " + pid + ", please wait...");
        agent.attach(pid);
      } else {
        System.err.println("Attaching to core " + coreFileName +
                           " from executable " + javaExecutableName + ", please wait...");
        agent.attach(javaExecutableName, coreFileName);
      }
    }
    catch (DebuggerException e) {
      if (usePid) {
        System.err.print("Error attaching to process: ");
      } else {
        System.err.print("Error attaching to core file: ");
      }
      if (e.getMessage() != null) {
        System.err.print(e.getMessage());
      }
      System.err.println();
      System.exit(1);
    }

    System.err.println("Debugger attached successfully.");

    if (VM.getVM().isCore()) {
      System.err.println("Core build detected.");
    } else if (VM.getVM().isClientCompiler()) {
      System.err.println("Client compiler detected.");
    } else if (VM.getVM().isServerCompiler()) {
      System.err.println("Server compiler detected.");
    } else {
      throw new RuntimeException("Fatal error: should have been able to detect core/C1/C2 build");
    }

    // Ready to go with the database...
    try {
      Threads threads = VM.getVM().getThreads();
      int i = 1;
      for (JavaThread cur = threads.first(); cur != null; cur = cur.next(), i++) {
        if (cur.isJavaThread()) {
          Address sp = cur.getLastJavaSP();
          System.out.print("Thread ");
          cur.printThreadIDOn(System.out);
          System.out.print(": (state = " + cur.getThreadState());
          System.out.println(", current Java SP = " + sp + ")");
          try {
            for (JavaVFrame vf = getLastJavaVFrame(cur); vf != null; vf = vf.javaSender()) {
              Method method = vf.getMethod();
              System.out.print(" - " + method.externalNameAndSignature() + 
                               " @bci=" + vf.getBCI());

              int lineNumber = method.getLineNumberFromBCI(vf.getBCI());
              if (lineNumber != -1) {
                System.out.print(", line=" + lineNumber);
              }

              Address pc = vf.getFrame().getPC();
              if (pc != null) {
                System.out.print(", pc=" + pc);
              }

              System.out.print(", methodOop=" + method.getHandle());

              if (vf.isCompiledFrame()) {
                System.out.print(" (Compiled frame");
              }
              if (vf.isInterpretedFrame()) {
                System.out.print(" (Interpreted frame");
              }
              if (vf.mayBeImpreciseDbg()) {
                System.out.print("; information may be imprecise");
              }

              System.out.println(")");
            }
          } catch (Exception e) {
            System.out.println("Error occurred during stack walking:");
            e.printStackTrace();
          }
          System.out.println();
          System.out.println();
        }
      }
    }
    catch (AddressException e) {
      System.err.println("Error accessing address 0x" + Long.toHexString(e.getAddress()));
      e.printStackTrace();
    }

    agent.detach();
  }

  private static JavaVFrame getLastJavaVFrame(JavaThread cur) {
    RegisterMap regMap = cur.newRegisterMap(true);
    Frame f = cur.getCurrentFrameGuess();
    VFrame vf = VFrame.newVFrame(f, regMap, cur, true, true);
    if (vf == null) {
      System.err.println(" (Unable to create vframe for topmost frame guess)");
      return null;
    }
    if (vf.isJavaFrame()) {
      return (JavaVFrame) vf;
    }
    return (JavaVFrame) vf.javaSender();
  }
}
