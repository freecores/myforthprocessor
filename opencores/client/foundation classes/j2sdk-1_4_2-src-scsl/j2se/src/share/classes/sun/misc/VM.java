/*
 * @(#)VM.java	1.22 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.misc;

import java.util.Properties;

public class VM {

    /* The following methods used to be native methods that instruct
     * the VM to selectively suspend certain threads in low-memory
     * situations. They are inherently dangerous and not implementable
     * on native threads. We removed them in JDK 1.2. The skeletons
     * remain so that existing applications that use these methods
     * will still work.
     */
    private static boolean suspended = false;

    /** @deprecated */
    public static boolean threadsSuspended() {
        return suspended;
    }

    public static boolean allowThreadSuspension(ThreadGroup g, boolean b) {
	return g.allowThreadSuspension(b);
    }

    /** @deprecated */
    public static boolean suspendThreads() {
        suspended = true;
	return true;
    }

    // Causes any suspended threadgroups to be resumed.
    /** @deprecated */
    public static void unsuspendThreads() {
        suspended = false;
    }

    // Causes threadgroups no longer marked suspendable to be resumed.
    /** @deprecated */
    public static void unsuspendSomeThreads() {
    }

    /* Deprecated fields and methods -- Memory advice not supported in 1.2 */

    /** @deprecated */
    public static final int STATE_GREEN = 1;

    /** @deprecated */
    public static final int STATE_YELLOW = 2;

    /** @deprecated */
    public static final int STATE_RED = 3;

    /** @deprecated */
    public static final int getState() {
	return STATE_GREEN;
    }

    /** @deprecated */
    public static void registerVMNotification(VMNotification n) { }

    /** @deprecated */
    public static void asChange(int as_old, int as_new) { }

    /** @deprecated */
    public static void asChange_otherthread(int as_old, int as_new) { }

    /*
     * Not supported in 1.2 because these will have to be exported as
     * JVM functions, and we are not sure we want do that. Leaving
     * here so it can be easily resurrected -- just remove the //
     * comments.
     */

    /**
     * Resume Java profiling.  All profiling data is added to any
     * earlier profiling, unless <code>resetJavaProfiler</code> is
     * called in between.  If profiling was not started from the
     * command line, <code>resumeJavaProfiler</code> will start it.
     * <p>
     *
     * NOTE: Profiling must be enabled from the command line for a
     * java.prof report to be automatically generated on exit; if not,
     * writeJavaProfilerReport must be invoked to write a report.
     *
     * @see 	resetJavaProfiler
     * @see 	writeJavaProfilerReport
     */

    // public native static void resumeJavaProfiler();

    /**
     * Suspend Java profiling.
     */
    // public native static void suspendJavaProfiler();

    /**
     * Initialize Java profiling.  Any accumulated profiling
     * information is discarded.
     */
    // public native static void resetJavaProfiler();

    /**
     * Write the current profiling contents to the file "java.prof".
     * If the file already exists, it will be overwritten.
     */
    // public native static void writeJavaProfilerReport();


    private static volatile boolean booted = false;

    // Invoked by by System.initializeSystemClass just before returning.
    // Subsystems that are invoked during initialization can check this
    // property in order to avoid doing things that should wait until the
    // application class loader has been set up.
    //
    public static void booted() {
	booted = true;
    }

    public static boolean isBooted() {
	return booted;
    }

    // A user-settable upper limit on the maximum amount of allocatable
    // direct buffer memory.  This value may be changed during VM
    // initialization if "java" is launched with "-XX:MaxDirectMemorySize=<size>".
    private static long directMemory = 64 * 1024 * 1024;

    // If this method is invoked during VM initialization, it initializes the
    // maximum amount of allocatable direct buffer memory (in bytes) from the
    // system property sun.nio.MaxDirectMemorySize.  The system property will
    // be removed when it is accessed.
    //
    // If this method is invoked after the VM is booted, it returns the
    // maximum amount of allocatable direct buffer memory.
    public static long maxDirectMemory() {
	if (booted)
	    return directMemory;

	Properties p = System.getProperties();
	String s = (String)p.remove("sun.nio.MaxDirectMemorySize");
	System.setProperties(p);

	long l = (s != null ? Long.parseLong(s) : -1);
	if (l > -1)
	    directMemory = l;

	return directMemory;
    }
}
