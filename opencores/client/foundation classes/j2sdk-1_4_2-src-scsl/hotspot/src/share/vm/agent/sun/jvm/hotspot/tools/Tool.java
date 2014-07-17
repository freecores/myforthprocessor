package sun.jvm.hotspot.tools;

import sun.jvm.hotspot.*;
import sun.jvm.hotspot.runtime.*;
import sun.jvm.hotspot.debugger.*;

// generic command line or GUI tool. 
// override run & code main as shown below.

public abstract class Tool implements Runnable {
   private HotSpotAgent agent;

   protected void usage() {
      System.out.println("usage: java " + getClass().getName() + " [pid]");
      System.out.println("   or: java " + getClass().getName() + " [executable] [core]");
      System.out.println("\"pid\" must be the process ID of a HotSpot process.");
      System.exit(1);
   }

   /*
      Derived class main should be of the following form:
  
      public static void main(String[] args) {
         <derived class> obj = new <derived class>;
         obj.start(args);
      }

   */

   protected void stop() {
      if (agent != null) {
         agent.detach();
         System.exit(1);
      }
   }

   protected void start(String[] args) {
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

      agent = new HotSpotAgent();
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
        throw new RuntimeException("Fatal error: " +
                                 "should have been able to detect core/C1/C2 build");
      }

      run();
   }

   public String getName() {
      return getClass().getName();
   }
}

