/*
 * @(#)ThreadReference.java	1.41 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.jdi;
import java.util.List;

/**
 * A thread object from the target VM.
 * A ThreadReference is an {@link ObjectReference} with additional
 * access to thread-specific information from the target VM.
 *
 * @author Robert Field
 * @author Gordon Hirsch
 * @author James McIlree
 * @since  1.3
 */
public interface ThreadReference extends ObjectReference
{
    /** Thread status is unknown */
    public final int THREAD_STATUS_UNKNOWN  =-1;
    /** Thread has completed execution */
    public final int THREAD_STATUS_ZOMBIE = 0;
    /** Thread is runnable */
    public final int THREAD_STATUS_RUNNING = 1;
    /** Thread is sleeping - Thread.sleep() or JVM_Sleep() was called */
    public final int THREAD_STATUS_SLEEPING = 2;
    /** Thread is waiting on a java monitor */
    public final int THREAD_STATUS_MONITOR = 3;
    /** Thread is waiting - Thread.wait() or JVM_MonitorWait() was called */
    public final int THREAD_STATUS_WAIT = 4;
    /** Thread has not yet been started */
    public final int THREAD_STATUS_NOT_STARTED = 5;

    /**
     * Returns the name of this thread.
     *
     * @return the string containing the thread name.
     */
    String name();

    /**
     * Suspends this thread. The thread can be resumed through
     * {@link #resume} or resumed with other threads through
     * {@link VirtualMachine#resume}.
     * <p>
     * Unlike {@link java.lang.Thread#suspend},
     * suspends of both the virtual machine and individual threads are
     * counted. Before a thread will run again, it must be resumed
     * (through {@link #resume} or {@link ThreadReference#resume})
     * the same number of times it has been suspended.
     * <p>
     * Suspending single threads with this method has the same dangers
     * as {@link java.lang.Thread#suspend()}. If the suspended thread
     * holds a monitor needed by another running thread, deadlock is
     * possible in the target VM (at least until the suspended thread
     * is resumed again).
     * <p>
     * The suspended thread is guaranteed to remain suspended until
     * resumed through one of the JDI resume methods mentioned above;
     * the application in the target VM cannot resume the suspended thread
     * through {@link java.lang.Thread#resume}.
     */
    void suspend();

    /**
     * Resumes this thread. If this thread was not previously suspended
     * through {@link #suspend} or through {@link VirtualMachine#suspend},
     * or because of a SUSPEND_ALL or SUSPEND_EVENT_THREAD event, then
     * invoking this method has no effect. Otherwise, the count of pending
     * suspends on this thread is decremented. If it is decremented to 0,
     * the thread will continue to execute.
     * Note: the normal way to resume from an event related suspension is
     * via {@link com.sun.jdi.event.EventSet#resume}.
     */
    void resume();

    /**
     * Returns the number of pending suspends for this thread. See
     * {@link #suspend} for an explanation of counted suspends.
     * @return pending suspend count as an integer
     */
    int suspendCount();

    /**
     * Stops this thread with an asynchronous exception.
     * A debugger thread in the target VM will stop this thread
     * with the given {@link java.lang.Throwable} object.
     *
     * @param throwable the asynchronous exception to throw.
     * @throws InvalidTypeException if <code>throwable</code> is not
     * an instance of java.lang.Throwable in the target VM.
     * @see java.lang.Thread#stop(Throwable)
     */
    void stop(ObjectReference throwable) throws InvalidTypeException;

    /**
     * Interrupts this thread unless the thread has been suspended by the 
     * debugger.
     *
     * @see java.lang.Thread#interrupt()
     */
    void interrupt();

    /**
     * Returns the thread's status. If the thread is not suspended the
     * thread's current status is returned. If the thread is suspended, the
     * thread's status before the suspension is returned (or
     * {@link #THREAD_STATUS_UNKNOWN} if this information is not available.
     * {@link #isSuspended} can be used to determine if the thread has been
     * suspended.
     *
     * @return one of
     * {@link #THREAD_STATUS_UNKNOWN},
     * {@link #THREAD_STATUS_ZOMBIE},
     * {@link #THREAD_STATUS_RUNNING},
     * {@link #THREAD_STATUS_SLEEPING},
     * {@link #THREAD_STATUS_MONITOR},
     * {@link #THREAD_STATUS_WAIT},
     * {@link #THREAD_STATUS_NOT_STARTED},
     */
    int status();

    /**
     * Determines whether the thread has been suspended by the
     * the debugger.
     *
     * @return <code>true</code> if the thread is currently suspended;
     * <code>false</code> otherwise.
     */
    boolean isSuspended();

    /**
     * Determines whether the thread is suspended at a breakpoint.
     *
     * @return <code>true</code> if the thread is currently stopped at
     * a breakpoint; <code>false</code> otherwise.
     */
    boolean isAtBreakpoint();

    /**
     * Returns this thread's thread group.
     * @return a {@link ThreadGroupReference} that mirrors this thread's
     * thread group in the target VM.
     */
    ThreadGroupReference threadGroup();

    /**
     * Returns the number of stack frames in the thread's current
     * call stack.
     * The thread must be suspended (normally through an interruption
     * to the VM) to get this information, and
     * it is only valid until the thread is resumed again.
     *
     * @return an integer frame count
     * @throws IncompatibleThreadStateException if the thread is
     * not suspended in the target VM
     */
    int frameCount() throws IncompatibleThreadStateException;

    /**
     * Returns a List containing each {@link StackFrame} in the
     * thread's current call stack.
     * The thread must be suspended (normally through an interruption
     * to the VM) to get this information, and
     * it is only valid until the thread is resumed again.
     *
     * @return a List of {@link StackFrame} with the current frame first
     * followed by each caller's frame.
     * @throws IncompatibleThreadStateException if the thread is
     * not suspended in the target VM
     */
    List frames() throws IncompatibleThreadStateException;

    /**
     * Returns the {@link StackFrame} at the given index in the
     * thread's current call stack. Index 0 retrieves the current
     * frame; higher indices retrieve caller frames.
     * The thread must be suspended (normally through an interruption
     * to the VM) to get this information, and
     * it is only valid until the thread is resumed again.
     *
     * @param index the desired frame
     * @return the requested {@link StackFrame}
     * @throws IncompatibleThreadStateException if the thread is
     * not suspended in the target VM
     * @throws java.lang.IndexOutOfBoundsException if the index is greater than
     * or equal to {@link #frameCount} or is negative.
     */
    StackFrame frame(int index) throws IncompatibleThreadStateException;

    /**
     * Returns a List containing a range of {@link StackFrame} mirrors
     * from the thread's current call stack.
     * The thread must be suspended (normally through an interruption
     * to the VM) to get this information, and
     * it is only valid until the thread is resumed again.
     *
     * @param start the index of the first frame to retrieve.
     *       Index 0 represents the current frame.
     * @param length the number of frames to retrieve
     * @return a List of {@link StackFrame} with the current frame first
     * followed by each caller's frame.
     * @throws IncompatibleThreadStateException if the thread is
     * not suspended in the target VM
     * @throws IndexOutOfBoundsException if the specified range is not
     * within the range of stack frame indicies.
     * That is, the exception is thrown if any of the following are true:
     * <pre>    start &lt; 0
     *    start &gt;= {@link #frameCount}
     *    length &lt; 0
     *    (start+length) &gt; {@link #frameCount}</pre>
     */
    List frames(int start, int length) throws IncompatibleThreadStateException;

    /**
     * Returns a List containing an {@link ObjectReference} for
     * each monitor owned by the thread.
     * A monitor is owned by a thread if it has been entered
     * (via the synchronized statement or entry into a synchronized
     * method) and has not been relinquished through {@link Object#wait}.
     * <p>
     * Not all target virtual machines support this operation.
     * Use {@link VirtualMachine#canGetOwnedMonitorInfo()}
     * to determine if the operation is supported.
     *
     * @return a List of {@link ObjectReference} objects. The list
     * has zero length if no monitors are owned by this thread.
     * @throws java.lang.UnsupportedOperationException if
     * the target virtual machine does not support this
     * operation.
     * @throws IncompatibleThreadStateException if the thread is
     * not suspended in the target VM
     */
    List ownedMonitors() throws IncompatibleThreadStateException;

    /**
     * Returns an {@link ObjectReference} for the monitor, if any,
     * for which this thread is currently waiting.
     * The thread can be waiting for a monitor through entry into a
     * synchronized method, the synchronized statement, or
     * {@link Object#wait}.  The {@link #status} method can be used
     * to differentiate between the first two cases and the third.
     * <p>
     * Not all target virtual machines support this operation.
     * Use {@link VirtualMachine#canGetCurrentContendedMonitor()}
     * to determine if the operation is supported.
     *
     * @return the {@link ObjectReference} corresponding to the
     * contended monitor, or null if it is not waiting for a monitor.
     * @throws java.lang.UnsupportedOperationException if
     * the target virtual machine does not support this
     * operation.
     * @throws IncompatibleThreadStateException if the thread is
     * not suspended in the target VM
     */
    ObjectReference currentContendedMonitor() throws IncompatibleThreadStateException;

    /**
     * Pop stack frames.
     * <P>
     * All frames up to and including the <CODE>frame</FRAME> are
     * popped off the stack.  
     * The frame previous to the parameter <CODE>frame</FRAME>
     * will become the current frame.
     * <P>
     * After this operation, this thread will be
     * suspended at the invoke instruction of the target method
     * that created <CODE>frame</CODE>. 
     * The <CODE>frame</CODE>'s method can be reentered with a step into
     * the instruction.
     * <P>
     * The operand stack is restored, however, any changes
     * to the arguments that occurred in the called method, remain.
     * For example, if the method <CODE>foo</CODE>:
     * <PRE>
     *    void foo(int x) {
     *        System.out.println("Foo: " + x);
     *        x = 4;
     *        System.out.println("pop here");
     *    }
     * </PRE>
     * was called with <CODE>foo(7)</CODE> and <CODE>foo</CODE>
     * is popped at the second <CODE>println</CODE> and resumed,
     * it will print: <CODE>Foo: 4</CODE>.
     * <P>
     * Locks acquired by a popped frame are released when it 
     * is popped. This applies to synchronized methods that 
     * are popped, and to any synchronized blocks within them. 
     * <P>
     * Finally blocks are not executed. 
     * <P>
     * No aspect of state, other than this thread's execution point and
     * locks, is affected by this call.  Specifically, the values of
     * fields are unchanged, as are external resources such as
     * I/O streams.  Additionally, the target program might be
     * placed in a state that is impossible with normal program flow;
     * for example, order of lock acquisition might be perturbed.
     * Thus the target program may
     * proceed differently than the user would expect.
     * <P>
     * The specified thread must be suspended.
     * <P>
     * All <code>StackFrame</code> objects for this thread are
     * invalidated. 
     * <P>
     * No events are generated by this method. 
     * <P>
     * None of the frames through and including the frame for the caller 
     * of <i>frame</i> may be native.
     * <P>
     * Not all target virtual machines support this operation. 
     * Use {@link VirtualMachine#canPopFrames() VirtualMachine.canPopFrames()}
     * to determine if the operation is supported.
     *
     * @param frame Stack frame to pop.  <CODE>frame</CODE> is on this
     * thread's call stack.
     *
     * @throws java.lang.UnsupportedOperationException if 
     * the target virtual machine does not support this 
     * operation - see 
     * {@link VirtualMachine#canPopFrames() VirtualMachine.canPopFrames()}.
     *
     * @throws IncompatibleThreadStateException if this
     * thread is not suspended.
     *
     * @throws java.lang.IllegalArgumentException if <CODE>frame</CODE>
     * is not on this thread's call stack.
     *
     * @throws NativeMethodException if one of the frames that would be 
     * popped is that of a native method or if the frame previous to
     * <i>frame</i> is native.
     *
     * @throws InvalidStackFrameException if <CODE>frame</CODE> has become
     * invalid. Once this thread is resumed, the stack frame is
     * no longer valid.  This exception is also thrown if there are no
     * more frames.
     *
     * @since 1.4 */
    void popFrames(StackFrame frame) throws IncompatibleThreadStateException;
}