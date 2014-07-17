These are the Java-level sources for the Serviceability Agent (SA).

To build, type "make all".

The current top-level program using the Serviceability Agent APIs is
HSDB, the "HotSpot Debugger". To run it, type "java sun.jvm.hotspot.HSDB".

There are three modes for the debugger: attaching to a local process,
opening a core file, and attaching to a remote "debug server". The
remote case requires two programs to be running on the remote machine:
the rmiregistry (see the script "start-rmiregistry" in this directory;
run this in the background) and the debug server (see the script
"start-debug-server"), in that order. start-rmiregistry takes no
arguments; start-debug-server takes as argument the process ID or the
executable and core file names to allow remote debugging of. Make sure
you do NOT have a CLASSPATH environment variable set when you run
these scripts. (The classes put into the rmiregistry are in sun.*, and
there are permissions problems if they aren't placed on the boot
classpath.)

NOTE that the SA currently only has full functionality against VMs
hosted on Solaris/SPARC. Remote debugging of Solaris/SPARC VMs on
arbitrary platforms is possible using the debug server; select
"Connect to debug server..."  in HSDB. Win32 support is in its initial
stages; object histograms of VMs are possible, but stack traces are
not yet implemented. See src/os/win32/agent for Win32-specific
details.

Once the debugger has been launched, the threads list is displayed.
The current set of functionality allows:

 - browsing of the annotated stack memory ("Stack Memory" button). It
   is currently annotated with the following information:
   - method names of the Java frames and their extents (supporting
     inlined compiled methods)
   - locations and types of oops, found using the oop map information
     from compiled methods (interpreter oop maps coming soon)
   - if a Java frame was interrupted by a signal (e.g., because of a
     crash), annotates the frame with the signal name and number
   - interpreter codelet descriptions for interpreted frames
 - finding which thread or threads caused a crash (currently
   identified by the presence of a signal handler frame)
 - browsing of oops using the Oop Inspector.
 - browsing of the java.lang.Thread object's oop.
 - object histogram and inspection of objects therein.

More functionality is coming soon.

Please note that as of this writing (8/2000) the SA is still in its
early phases and not all of the possible states of target VMs have
been tested. For example, the SA will probably not work at all if it
freezes the target VM during certain phases of GC. In addition,
infrequent infinite loops have been found during testing. Please email
bug reports to kenneth.russell@eng. A pointer to a core file (see
gcore(1)) which the SA can not handle well is best for a bug report.

The SA currently relies on existing binaries on jano; see
sun/jvm/hotspot/HotSpotAgent.java. The libsvc_agent_dbx.so import
module (Solaris only) can be rebuilt from the sources in
src/os/solaris/agent at the root of the source tree. The version of
dbx mirrored on jano and used by the SA is a nightly build of dbx 7.0.
