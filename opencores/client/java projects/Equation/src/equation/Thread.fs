MODULENAME equation.Thread
(
* @(#)Thread.java	1.127 03/01/23
*
* Copyright 2003 Sun Microsystems, Inc. All rights reserved.
* SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
)  ( *
* A <i>thread</i> is a thread of execution in a program. The Java
* Virtual Machine allows an application to have multiple threads of
* execution running concurrently.
* <p>
* Every thread has a priority. Threads with higher priority are
* executed in preference to threads with lower priority. Each thread
* may or may not also be marked as a daemon. When code running in
* some thread creates a new <code>Thread</code> object, the new
* thread has its priority initially set equal to the priority of the
* creating thread, and is a daemon thread if and only if the
* creating thread is a daemon.
* <p>
* When a Java Virtual Machine starts up, there is usually a single
* non-daemon thread (which typically calls the method named
* <code>main</code> of some designated class). The Java Virtual
* Machine continues to execute threads until either of the following
* occurs:
* <ul>
* <li>The <code>exit</code> method of class <code>Runtime</code> has been
*     called and the security manager has permitted the exit operation
*     to take place.
* <li>All threads that are not daemon threads have died, either by
*     returning from the call to the <code>run</code> method or by
*     throwing an exception that propagates beyond the <code>run</code>
*     method.
* </ul>
* <p>
* There are two ways to create a new thread of execution. One is to
* declare a class to be a subclass of <code>Thread</code>. This
* subclass should override the <code>run</code> method of class
* <code>Thread</code>. An instance of the subclass can then be
* allocated and started. For example, a thread that computes primes
* larger than a stated value could be written as follows:
* <p><hr><blockquote><pre>
*     class PrimeThread extends Thread {
*         long minPrime;
*         PrimeThread(long minPrime) {
*             this.minPrime = minPrime;
*         }
*
*         public void run() {
*             // compute primes larger than minPrime
*             &nbsp;.&nbsp;.&nbsp;.
*         }
*     }
* </pre></blockquote><hr>
* <p>
* The following code would then create a thread and start it running:
* <p><blockquote><pre>
*     PrimeThread p = new PrimeThread(143);
*     p.start();
* </pre></blockquote>
* <p>
* The other way to create a thread is to declare a class that
* implements the <code>Runnable</code> interface. That class then
* implements the <code>run</code> method. An instance of the class can
* then be allocated, passed as an argument when creating
* <code>Thread</code>, and started. The same example in this other
* style looks like the following:
* <p><hr><blockquote><pre>
*     class PrimeRun implements Runnable {
*         long minPrime;
*         PrimeRun(long minPrime) {
*             this.minPrime = minPrime;
*         }
*
*         public void run() {
*             // compute primes larger than minPrime
*             &nbsp;.&nbsp;.&nbsp;.
*         }
*     }
* </pre></blockquote><hr>
* <p>
* The following code would then create a thread and start it running:
* <p><blockquote><pre>
*     PrimeRun p = new PrimeRun(143);
*     new Thread(p).start();
* </pre></blockquote>
* <p>
* Every thread has a name for identification purposes. More than
* one thread may have the same name. If a name is not specified when
* a thread is created, a new name is generated for it.
*
* @author  unascribed
* @version 1.127, 01/23/03
* @see     java.lang.Runnable
* @see     java.lang.Runtime#exit(int)
* @see     java.lang.Thread#run()
* @see     java.lang.Thread#stop()
* @since   JDK1.0
)
( *
* Allocates a new <code>Thread</code> object. This constructor has
* the same effect as <code>Thread(null, null,</code>
* <i>gname</i><code>)</code>, where <b><i>gname</i></b> is
* a newly generated name. Automatically generated names are of the
* form <code>"Thread-"+</code><i>n</i>, where <i>n</i> is an integer.
*
* @see     java.lang.Thread#Thread(java.lang.ThreadGroup,
*          java.lang.Runnable, java.lang.String)
)

:LOCAL Thread.Thread§-1360112640
   1 VALLOCATE LOCAL §base0
   
   \ new statement
   0
   LOCALS §this |
   
   \ new statement
   Object§-1890496768.table 1827294976 EXECUTE-NEW
   
   \ new statement
   68 MALLOC DROP DUP DUP §base0 V! TO §this OVER
   IF
      OVER CELL+ @ 4 + OVER OVER ! DROP
   ENDIF
   CELL+ @ OVER
   OVER ! NIP
   Thread§-1457236992.table OVER 12 + !
   -1457258496 OVER 20 + !
   " Thread " OVER 16 + !
   4 OVER 24 + ! DROP
   
   \ new statement
   (  Whether or not to single_step this thread.  )
   
   \ new statement
   (  Whether or not the thread is a daemon thread.  )
   
   \ new statement
   FALSE
   §this CELL+ @ 52 + ( equation.Thread.daemon )    !
   
   \ new statement
   (  Whether or not this thread was asked to exit before it runs. )
   
   \ new statement
   TRUE
   §this CELL+ @ 56 + ( equation.Thread.stillborn )    !
   
   \ new statement
   (  What will be run.  )
   
   \ new statement
   (  The group of this thread  )
   
   \ new statement
   (  The interrupt flag of this thread  )
   
   \ new statement
   FALSE
   §this CELL+ @ 60 + ( equation.Thread.interrupt )    !
   
   \ new statement
   (  ThreadLocal values pertaining to this thread. This map is maintained
   * by the ThreadLocal class.  )  \ ThreadLocal.ThreadLocalMap threadLocals = null;
   (
   * InheritableThreadLocal values pertaining to this thread. This map is
   * maintained by the InheritableThreadLocal class.
   )  \ ThreadLocal.ThreadLocalMap inheritableThreadLocals = null;
   (
   * The requested stack size for this thread, or 0 if the creator did
   * not specify a stack size.  It is up to the VM to do whatever it
   * likes with this number; some VMs will ignore it.
   )
   
   \ new statement
   (  The object in which this thread is blocked in an interruptible I/O
   * operation, if any.  The blocker's interrupt method() should be invoked
   * before setting this thread's interrupt status.
   )
   
   \ new statement
   
   2 VALLOCATE LOCAL §tempvar
   §this >R
   0
   0
   U" Thread- " COUNT JavaArray.createUnicode§-675323136 DUP §tempvar V!
   Thread.nextThreadNum§79719936
   <# 0 ..R JavaArray.createString§-105880832 DUP §tempvar 4 + V!
   SWAP  -1461427456 TRUE ( equation.String.concat§-1461427456 ) EXECUTE-METHOD
   0 S>D
   R> -1783011072 TRUE ( equation.Thread.init§-1783011072 ) EXECUTE-METHOD
   
   
   
   §tempvar SETVTOP
   
   PURGE 1
   
   
   \ new statement
   §this DUP 0 V! ( return object )    
   \ new statement
   0 §break28422 LABEL
   
   \ new statement
   §base0 SETVTOP
   PURGE 2
   
   \ new statement
   DROP
;
( *
* Allocates a new <code>Thread</code> object. This constructor has
* the same effect as <code>Thread(null, null, name)</code>.
*
* @param   name   the name of the new thread.
* @see     java.lang.Thread#Thread(java.lang.ThreadGroup,
*          java.lang.Runnable, java.lang.String)
)

:LOCAL Thread.Thread§-1380166656
   2 VALLOCATE LOCAL §base0
   DUP 4 §base0 + V! LOCAL name
   
   \ new statement
   0
   LOCALS §this |
   
   \ new statement
   Object§-1890496768.table 1827294976 EXECUTE-NEW
   
   \ new statement
   68 MALLOC DROP DUP DUP §base0 V! TO §this OVER
   IF
      OVER CELL+ @ 4 + OVER OVER ! DROP
   ENDIF
   CELL+ @ OVER
   OVER ! NIP
   Thread§-1457236992.table OVER 12 + !
   -1457258496 OVER 20 + !
   " Thread " OVER 16 + !
   4 OVER 24 + ! DROP
   
   \ new statement
   (  Whether or not to single_step this thread.  )
   
   \ new statement
   (  Whether or not the thread is a daemon thread.  )
   
   \ new statement
   FALSE
   §this CELL+ @ 52 + ( equation.Thread.daemon )    !
   
   \ new statement
   (  Whether or not this thread was asked to exit before it runs. )
   
   \ new statement
   TRUE
   §this CELL+ @ 56 + ( equation.Thread.stillborn )    !
   
   \ new statement
   (  What will be run.  )
   
   \ new statement
   (  The group of this thread  )
   
   \ new statement
   (  The interrupt flag of this thread  )
   
   \ new statement
   FALSE
   §this CELL+ @ 60 + ( equation.Thread.interrupt )    !
   
   \ new statement
   (  ThreadLocal values pertaining to this thread. This map is maintained
   * by the ThreadLocal class.  )  \ ThreadLocal.ThreadLocalMap threadLocals = null;
   (
   * InheritableThreadLocal values pertaining to this thread. This map is
   * maintained by the InheritableThreadLocal class.
   )  \ ThreadLocal.ThreadLocalMap inheritableThreadLocals = null;
   (
   * The requested stack size for this thread, or 0 if the creator did
   * not specify a stack size.  It is up to the VM to do whatever it
   * likes with this number; some VMs will ignore it.
   )
   
   \ new statement
   (  The object in which this thread is blocked in an interruptible I/O
   * operation, if any.  The blocker's interrupt method() should be invoked
   * before setting this thread's interrupt status.
   )
   
   \ new statement
   §this >R
   0
   0
   name
   0 S>D
   R> -1783011072 TRUE ( equation.Thread.init§-1783011072 ) EXECUTE-METHOD
   
   \ new statement
   §this DUP 0 V! ( return object )    
   \ new statement
   0 §break28425 LABEL
   
   \ new statement
   
   §base0 SETVTOP
   PURGE 3
   
   \ new statement
   DROP
;
( *
* Allocates a new <code>Thread</code> object. This constructor has
* the same effect as <code>Thread(group, null, name)</code>
*
* @param      group   the thread group.
* @param      name    the name of the new thread.
* @exception  SecurityException  if the current thread cannot create a
*               thread in the specified thread group.
* @see        java.lang.Thread#Thread(java.lang.ThreadGroup,
*          java.lang.Runnable, java.lang.String)
)

:LOCAL Thread.Thread§-1408805888
   3 VALLOCATE LOCAL §base0
   DUP 4 §base0 + V! LOCAL name
   DUP 8 §base0 + V! LOCAL group
   
   \ new statement
   0
   LOCALS §this |
   
   \ new statement
   Object§-1890496768.table 1827294976 EXECUTE-NEW
   
   \ new statement
   68 MALLOC DROP DUP DUP §base0 V! TO §this OVER
   IF
      OVER CELL+ @ 4 + OVER OVER ! DROP
   ENDIF
   CELL+ @ OVER
   OVER ! NIP
   Thread§-1457236992.table OVER 12 + !
   -1457258496 OVER 20 + !
   " Thread " OVER 16 + !
   4 OVER 24 + ! DROP
   
   \ new statement
   (  Whether or not to single_step this thread.  )
   
   \ new statement
   (  Whether or not the thread is a daemon thread.  )
   
   \ new statement
   FALSE
   §this CELL+ @ 52 + ( equation.Thread.daemon )    !
   
   \ new statement
   (  Whether or not this thread was asked to exit before it runs. )
   
   \ new statement
   TRUE
   §this CELL+ @ 56 + ( equation.Thread.stillborn )    !
   
   \ new statement
   (  What will be run.  )
   
   \ new statement
   (  The group of this thread  )
   
   \ new statement
   (  The interrupt flag of this thread  )
   
   \ new statement
   FALSE
   §this CELL+ @ 60 + ( equation.Thread.interrupt )    !
   
   \ new statement
   (  ThreadLocal values pertaining to this thread. This map is maintained
   * by the ThreadLocal class.  )  \ ThreadLocal.ThreadLocalMap threadLocals = null;
   (
   * InheritableThreadLocal values pertaining to this thread. This map is
   * maintained by the InheritableThreadLocal class.
   )  \ ThreadLocal.ThreadLocalMap inheritableThreadLocals = null;
   (
   * The requested stack size for this thread, or 0 if the creator did
   * not specify a stack size.  It is up to the VM to do whatever it
   * likes with this number; some VMs will ignore it.
   )
   
   \ new statement
   (  The object in which this thread is blocked in an interruptible I/O
   * operation, if any.  The blocker's interrupt method() should be invoked
   * before setting this thread's interrupt status.
   )
   
   \ new statement
   §this >R
   group
   0
   name
   0 S>D
   R> -1783011072 TRUE ( equation.Thread.init§-1783011072 ) EXECUTE-METHOD
   
   \ new statement
   §this DUP 0 V! ( return object )    
   \ new statement
   0 §break28426 LABEL
   
   \ new statement
   
   
   §base0 SETVTOP
   PURGE 4
   
   \ new statement
   DROP
;
( *
* Allocates a new <code>Thread</code> object. This constructor has
* the same effect as <code>Thread(null, target,</code>
* <i>gname</i><code>)</code>, where <i>gname</i> is
* a newly generated name. Automatically generated names are of the
* form <code>"Thread-"+</code><i>n</i>, where <i>n</i> is an integer.
*
* @param   target   the object whose <code>run</code> method is called.
* @see     java.lang.Thread#Thread(java.lang.ThreadGroup,
*          java.lang.Runnable, java.lang.String)
)

:LOCAL Thread.Thread§-1768598528
   2 VALLOCATE LOCAL §base0
   DUP 4 §base0 + V! LOCAL target
   
   \ new statement
   0
   LOCALS §this |
   
   \ new statement
   Object§-1890496768.table 1827294976 EXECUTE-NEW
   
   \ new statement
   68 MALLOC DROP DUP DUP §base0 V! TO §this OVER
   IF
      OVER CELL+ @ 4 + OVER OVER ! DROP
   ENDIF
   CELL+ @ OVER
   OVER ! NIP
   Thread§-1457236992.table OVER 12 + !
   -1457258496 OVER 20 + !
   " Thread " OVER 16 + !
   4 OVER 24 + ! DROP
   
   \ new statement
   (  Whether or not to single_step this thread.  )
   
   \ new statement
   (  Whether or not the thread is a daemon thread.  )
   
   \ new statement
   FALSE
   §this CELL+ @ 52 + ( equation.Thread.daemon )    !
   
   \ new statement
   (  Whether or not this thread was asked to exit before it runs. )
   
   \ new statement
   TRUE
   §this CELL+ @ 56 + ( equation.Thread.stillborn )    !
   
   \ new statement
   (  What will be run.  )
   
   \ new statement
   (  The group of this thread  )
   
   \ new statement
   (  The interrupt flag of this thread  )
   
   \ new statement
   FALSE
   §this CELL+ @ 60 + ( equation.Thread.interrupt )    !
   
   \ new statement
   (  ThreadLocal values pertaining to this thread. This map is maintained
   * by the ThreadLocal class.  )  \ ThreadLocal.ThreadLocalMap threadLocals = null;
   (
   * InheritableThreadLocal values pertaining to this thread. This map is
   * maintained by the InheritableThreadLocal class.
   )  \ ThreadLocal.ThreadLocalMap inheritableThreadLocals = null;
   (
   * The requested stack size for this thread, or 0 if the creator did
   * not specify a stack size.  It is up to the VM to do whatever it
   * likes with this number; some VMs will ignore it.
   )
   
   \ new statement
   (  The object in which this thread is blocked in an interruptible I/O
   * operation, if any.  The blocker's interrupt method() should be invoked
   * before setting this thread's interrupt status.
   )
   
   \ new statement
   
   2 VALLOCATE LOCAL §tempvar
   §this >R
   0
   target
   U" Thread- " COUNT JavaArray.createUnicode§-675323136 DUP §tempvar V!
   Thread.nextThreadNum§79719936
   <# 0 ..R JavaArray.createString§-105880832 DUP §tempvar 4 + V!
   SWAP  -1461427456 TRUE ( equation.String.concat§-1461427456 ) EXECUTE-METHOD
   0 S>D
   R> -1783011072 TRUE ( equation.Thread.init§-1783011072 ) EXECUTE-METHOD
   
   
   
   §tempvar SETVTOP
   
   PURGE 1
   
   
   \ new statement
   §this DUP 0 V! ( return object )    
   \ new statement
   0 §break28423 LABEL
   
   \ new statement
   
   §base0 SETVTOP
   PURGE 3
   
   \ new statement
   DROP
;
( *
* Allocates a new <code>Thread</code> object. This constructor has
* the same effect as <code>Thread(null, target, name)</code>.
*
* @param   target   the object whose <code>run</code> method is called.
* @param   name     the name of the new thread.
* @see     java.lang.Thread#Thread(java.lang.ThreadGroup,
*          java.lang.Runnable, java.lang.String)
)

:LOCAL Thread.Thread§1142903808
   3 VALLOCATE LOCAL §base0
   DUP 4 §base0 + V! LOCAL name
   DUP 8 §base0 + V! LOCAL target
   
   \ new statement
   0
   LOCALS §this |
   
   \ new statement
   Object§-1890496768.table 1827294976 EXECUTE-NEW
   
   \ new statement
   68 MALLOC DROP DUP DUP §base0 V! TO §this OVER
   IF
      OVER CELL+ @ 4 + OVER OVER ! DROP
   ENDIF
   CELL+ @ OVER
   OVER ! NIP
   Thread§-1457236992.table OVER 12 + !
   -1457258496 OVER 20 + !
   " Thread " OVER 16 + !
   4 OVER 24 + ! DROP
   
   \ new statement
   (  Whether or not to single_step this thread.  )
   
   \ new statement
   (  Whether or not the thread is a daemon thread.  )
   
   \ new statement
   FALSE
   §this CELL+ @ 52 + ( equation.Thread.daemon )    !
   
   \ new statement
   (  Whether or not this thread was asked to exit before it runs. )
   
   \ new statement
   TRUE
   §this CELL+ @ 56 + ( equation.Thread.stillborn )    !
   
   \ new statement
   (  What will be run.  )
   
   \ new statement
   (  The group of this thread  )
   
   \ new statement
   (  The interrupt flag of this thread  )
   
   \ new statement
   FALSE
   §this CELL+ @ 60 + ( equation.Thread.interrupt )    !
   
   \ new statement
   (  ThreadLocal values pertaining to this thread. This map is maintained
   * by the ThreadLocal class.  )  \ ThreadLocal.ThreadLocalMap threadLocals = null;
   (
   * InheritableThreadLocal values pertaining to this thread. This map is
   * maintained by the InheritableThreadLocal class.
   )  \ ThreadLocal.ThreadLocalMap inheritableThreadLocals = null;
   (
   * The requested stack size for this thread, or 0 if the creator did
   * not specify a stack size.  It is up to the VM to do whatever it
   * likes with this number; some VMs will ignore it.
   )
   
   \ new statement
   (  The object in which this thread is blocked in an interruptible I/O
   * operation, if any.  The blocker's interrupt method() should be invoked
   * before setting this thread's interrupt status.
   )
   
   \ new statement
   §this >R
   0
   target
   name
   0 S>D
   R> -1783011072 TRUE ( equation.Thread.init§-1783011072 ) EXECUTE-METHOD
   
   \ new statement
   §this DUP 0 V! ( return object )    
   \ new statement
   0 §break28427 LABEL
   
   \ new statement
   
   
   §base0 SETVTOP
   PURGE 4
   
   \ new statement
   DROP
;
( *
* Allocates a new <code>Thread</code> object. This constructor has
* the same effect as <code>Thread(group, target,</code>
* <i>gname</i><code>)</code>, where <i>gname</i> is
* a newly generated name. Automatically generated names are of the
* form <code>"Thread-"+</code><i>n</i>, where <i>n</i> is an integer.
*
* @param      group    the thread group.
* @param      target   the object whose <code>run</code> method is called.
* @exception  SecurityException  if the current thread cannot create a
*             thread in the specified thread group.
* @see        java.lang.Thread#Thread(java.lang.ThreadGroup,
*             java.lang.Runnable, java.lang.String)
)

:LOCAL Thread.Thread§1507480576
   3 VALLOCATE LOCAL §base0
   DUP 4 §base0 + V! LOCAL target
   DUP 8 §base0 + V! LOCAL group
   
   \ new statement
   0
   LOCALS §this |
   
   \ new statement
   Object§-1890496768.table 1827294976 EXECUTE-NEW
   
   \ new statement
   68 MALLOC DROP DUP DUP §base0 V! TO §this OVER
   IF
      OVER CELL+ @ 4 + OVER OVER ! DROP
   ENDIF
   CELL+ @ OVER
   OVER ! NIP
   Thread§-1457236992.table OVER 12 + !
   -1457258496 OVER 20 + !
   " Thread " OVER 16 + !
   4 OVER 24 + ! DROP
   
   \ new statement
   (  Whether or not to single_step this thread.  )
   
   \ new statement
   (  Whether or not the thread is a daemon thread.  )
   
   \ new statement
   FALSE
   §this CELL+ @ 52 + ( equation.Thread.daemon )    !
   
   \ new statement
   (  Whether or not this thread was asked to exit before it runs. )
   
   \ new statement
   TRUE
   §this CELL+ @ 56 + ( equation.Thread.stillborn )    !
   
   \ new statement
   (  What will be run.  )
   
   \ new statement
   (  The group of this thread  )
   
   \ new statement
   (  The interrupt flag of this thread  )
   
   \ new statement
   FALSE
   §this CELL+ @ 60 + ( equation.Thread.interrupt )    !
   
   \ new statement
   (  ThreadLocal values pertaining to this thread. This map is maintained
   * by the ThreadLocal class.  )  \ ThreadLocal.ThreadLocalMap threadLocals = null;
   (
   * InheritableThreadLocal values pertaining to this thread. This map is
   * maintained by the InheritableThreadLocal class.
   )  \ ThreadLocal.ThreadLocalMap inheritableThreadLocals = null;
   (
   * The requested stack size for this thread, or 0 if the creator did
   * not specify a stack size.  It is up to the VM to do whatever it
   * likes with this number; some VMs will ignore it.
   )
   
   \ new statement
   (  The object in which this thread is blocked in an interruptible I/O
   * operation, if any.  The blocker's interrupt method() should be invoked
   * before setting this thread's interrupt status.
   )
   
   \ new statement
   
   2 VALLOCATE LOCAL §tempvar
   §this >R
   group
   target
   U" Thread- " COUNT JavaArray.createUnicode§-675323136 DUP §tempvar V!
   Thread.nextThreadNum§79719936
   <# 0 ..R JavaArray.createString§-105880832 DUP §tempvar 4 + V!
   SWAP  -1461427456 TRUE ( equation.String.concat§-1461427456 ) EXECUTE-METHOD
   0 S>D
   R> -1783011072 TRUE ( equation.Thread.init§-1783011072 ) EXECUTE-METHOD
   
   
   
   §tempvar SETVTOP
   
   PURGE 1
   
   
   \ new statement
   §this DUP 0 V! ( return object )    
   \ new statement
   0 §break28424 LABEL
   
   \ new statement
   
   
   §base0 SETVTOP
   PURGE 4
   
   \ new statement
   DROP
;
( *
* Allocates a new <code>Thread</code> object so that it has
* <code>target</code> as its run object, has the specified
* <code>name</code> as its name, and belongs to the thread group
* referred to by <code>group</code>.
* <p>
* If <code>group</code> is <code>null</code> and there is a
* security manager, the group is determined by the security manager's
* <code>getThreadGroup</code> method. If <code>group</code> is
* <code>null</code> and there is not a security manager, or the
* security manager's <code>getThreadGroup</code> method returns
* <code>null</code>, the group is set to be the same ThreadGroup
* as the thread that is creating the new thread.
*
* <p>If there is a security manager, its <code>checkAccess</code>
* method is called with the ThreadGroup as its argument.
* This may result in a SecurityException.
* <p>
* If the <code>target</code> argument is not <code>null</code>, the
* <code>run</code> method of the <code>target</code> is called when
* this thread is started. If the target argument is
* <code>null</code>, this thread's <code>run</code> method is called
* when this thread is started.
* <p>
* The priority of the newly created thread is set equal to the
* priority of the thread creating it, that is, the currently running
* thread. The method <code>setPriority</code> may be used to
* change the priority to a new value.
* <p>
* The newly created thread is initially marked as being a daemon
* thread if and only if the thread creating it is currently marked
* as a daemon thread. The method <code>setDaemon </code> may be used
* to change whether or not a thread is a daemon.
*
* @param      group     the thread group.
* @param      target   the object whose <code>run</code> method is called.
* @param      name     the name of the new thread.
* @exception  SecurityException  if the current thread cannot create a
*               thread in the specified thread group.
* @see        java.lang.Runnable#run()
* @see        java.lang.Thread#run()
* @see        java.lang.Thread#setDaemon(boolean)
* @see        java.lang.Thread#setPriority(int)
* @see        java.lang.ThreadGroup#checkAccess()
* @see        SecurityManager#checkAccess
)

:LOCAL Thread.Thread§353588224
   4 VALLOCATE LOCAL §base0
   DUP 4 §base0 + V! LOCAL name
   DUP 8 §base0 + V! LOCAL target
   DUP 12 §base0 + V! LOCAL group
   
   \ new statement
   0
   LOCALS §this |
   
   \ new statement
   Object§-1890496768.table 1827294976 EXECUTE-NEW
   
   \ new statement
   68 MALLOC DROP DUP DUP §base0 V! TO §this OVER
   IF
      OVER CELL+ @ 4 + OVER OVER ! DROP
   ENDIF
   CELL+ @ OVER
   OVER ! NIP
   Thread§-1457236992.table OVER 12 + !
   -1457258496 OVER 20 + !
   " Thread " OVER 16 + !
   4 OVER 24 + ! DROP
   
   \ new statement
   (  Whether or not to single_step this thread.  )
   
   \ new statement
   (  Whether or not the thread is a daemon thread.  )
   
   \ new statement
   FALSE
   §this CELL+ @ 52 + ( equation.Thread.daemon )    !
   
   \ new statement
   (  Whether or not this thread was asked to exit before it runs. )
   
   \ new statement
   TRUE
   §this CELL+ @ 56 + ( equation.Thread.stillborn )    !
   
   \ new statement
   (  What will be run.  )
   
   \ new statement
   (  The group of this thread  )
   
   \ new statement
   (  The interrupt flag of this thread  )
   
   \ new statement
   FALSE
   §this CELL+ @ 60 + ( equation.Thread.interrupt )    !
   
   \ new statement
   (  ThreadLocal values pertaining to this thread. This map is maintained
   * by the ThreadLocal class.  )  \ ThreadLocal.ThreadLocalMap threadLocals = null;
   (
   * InheritableThreadLocal values pertaining to this thread. This map is
   * maintained by the InheritableThreadLocal class.
   )  \ ThreadLocal.ThreadLocalMap inheritableThreadLocals = null;
   (
   * The requested stack size for this thread, or 0 if the creator did
   * not specify a stack size.  It is up to the VM to do whatever it
   * likes with this number; some VMs will ignore it.
   )
   
   \ new statement
   (  The object in which this thread is blocked in an interruptible I/O
   * operation, if any.  The blocker's interrupt method() should be invoked
   * before setting this thread's interrupt status.
   )
   
   \ new statement
   §this >R
   group
   target
   name
   0 S>D
   R> -1783011072 TRUE ( equation.Thread.init§-1783011072 ) EXECUTE-METHOD
   
   \ new statement
   §this DUP 0 V! ( return object )    
   \ new statement
   0 §break28428 LABEL
   
   \ new statement
   
   
   
   §base0 SETVTOP
   PURGE 5
   
   \ new statement
   DROP
;
( *
* Allocates a new <code>Thread</code> object so that it has
* <code>target</code> as its run object, has the specified
* <code>name</code> as its name, belongs to the thread group referred to
* by <code>group</code>, and has the specified <i>stack size</i>.
*
* <p>This constructor is identical to {@link
* #Thread(ThreadGroup,Runnable,String)} with the exception of the fact
* that it allows the thread stack size to be specified.  The stack size
* is the approximate number of bytes of address space that the virtual
* machine is to allocate for this thread's stack.  <b>The effect of the
* <tt>stackSize</tt> parameter, if any, is highly platform dependent.</b>
*
* <p>On some platforms, specifying a higher value for the
* <tt>stackSize</tt> parameter may allow a thread to achieve greater
* recursion depth before throwing a {@link StackOverflowError}.
* Similarly, specifying a lower value may allow a greater number of
* threads to exist concurrently without throwing an an {@link
* OutOfMemoryError} (or other internal error).  The details of
* the relationship between the value of the <tt>stackSize</tt> parameter
* and the maximum recursion depth and concurrency level are
* platform-dependent.  <b>On some platforms, the value of the
* <tt>stackSize</tt> parameter may have no effect whatsoever.</b>
*
* <p>The virtual machine is free to treat the <tt>stackSize</tt>
* parameter as a suggestion.  If the specified value is unreasonably low
* for the platform, the virtual machine may instead use some
* platform-specific minimum value; if the specified value is unreasonably
* high, the virtual machine may instead use some platform-specific
* maximum.  Likewise, the virtual machine is free to round the specified
* value up or down as it sees fit (or to ignore it completely).
*
* <p>Specifying a value of zero for the <tt>stackSize</tt> parameter will
* cause this constructor to behave exactly like the
* <tt>Thread(ThreadGroup, Runnable, String)</tt> constructor.
*
* <p><i>Due to the platform-dependent nature of the behavior of this
* constructor, extreme care should be exercised in its use.
* The thread stack size necessary to perform a given computation will
* likely vary from one JRE implementation to another.  In light of this
* variation, careful tuning of the stack size parameter may be required,
* and the tuning may need to be repeated for each JRE implementation on
* which an application is to run.</i>
*
* <p>Implementation note: Java platform implementers are encouraged to
* document their implementation's behavior with respect to the
* <tt>stackSize parameter</tt>.
*
* @param      group    the thread group.
* @param      target   the object whose <code>run</code> method is called.
* @param      name     the name of the new thread.
* @param      stackSize the desired stack size for the new thread, or
*             zero to indicate that this parameter is to be ignored.
* @exception  SecurityException  if the current thread cannot create a
*               thread in the specified thread group.
)

:LOCAL Thread.Thread§708269056
   4 VALLOCATE LOCAL §base0
   2LOCAL stackSize
   DUP 4 §base0 + V! LOCAL name
   DUP 8 §base0 + V! LOCAL target
   DUP 12 §base0 + V! LOCAL group
   
   \ new statement
   0
   LOCALS §this |
   
   \ new statement
   Object§-1890496768.table 1827294976 EXECUTE-NEW
   
   \ new statement
   68 MALLOC DROP DUP DUP §base0 V! TO §this OVER
   IF
      OVER CELL+ @ 4 + OVER OVER ! DROP
   ENDIF
   CELL+ @ OVER
   OVER ! NIP
   Thread§-1457236992.table OVER 12 + !
   -1457258496 OVER 20 + !
   " Thread " OVER 16 + !
   4 OVER 24 + ! DROP
   
   \ new statement
   (  Whether or not to single_step this thread.  )
   
   \ new statement
   (  Whether or not the thread is a daemon thread.  )
   
   \ new statement
   FALSE
   §this CELL+ @ 52 + ( equation.Thread.daemon )    !
   
   \ new statement
   (  Whether or not this thread was asked to exit before it runs. )
   
   \ new statement
   TRUE
   §this CELL+ @ 56 + ( equation.Thread.stillborn )    !
   
   \ new statement
   (  What will be run.  )
   
   \ new statement
   (  The group of this thread  )
   
   \ new statement
   (  The interrupt flag of this thread  )
   
   \ new statement
   FALSE
   §this CELL+ @ 60 + ( equation.Thread.interrupt )    !
   
   \ new statement
   (  ThreadLocal values pertaining to this thread. This map is maintained
   * by the ThreadLocal class.  )  \ ThreadLocal.ThreadLocalMap threadLocals = null;
   (
   * InheritableThreadLocal values pertaining to this thread. This map is
   * maintained by the InheritableThreadLocal class.
   )  \ ThreadLocal.ThreadLocalMap inheritableThreadLocals = null;
   (
   * The requested stack size for this thread, or 0 if the creator did
   * not specify a stack size.  It is up to the VM to do whatever it
   * likes with this number; some VMs will ignore it.
   )
   
   \ new statement
   (  The object in which this thread is blocked in an interruptible I/O
   * operation, if any.  The blocker's interrupt method() should be invoked
   * before setting this thread's interrupt status.
   )
   
   \ new statement
   §this >R
   group
   target
   name
   stackSize
   R> -1783011072 TRUE ( equation.Thread.init§-1783011072 ) EXECUTE-METHOD
   
   \ new statement
   §this DUP 0 V! ( return object )    
   \ new statement
   0 §break28429 LABEL
   
   \ new statement
   
   
   
   
   §base0 SETVTOP
   PURGE 6
   
   \ new statement
   DROP
;
( *
* Returns the number of active threads in the current thread's thread
* group.
*
* @return  the number of active threads in the current thread's thread
*          group.
)

: Thread.activeCount§-483565312
   
   \ new statement
   
   \ new statement
   
   2 VALLOCATE LOCAL §tempvar
   Thread.currentThread§-1671470336 DUP §tempvar V!
   >R
   R> 409036544 TRUE ( equation.Thread.getThreadGroup§409036544 ) EXECUTE-METHOD DUP §tempvar 4 + V!
   >R
   R> -483565312 TRUE ( equation.ThreadGroup.activeCount§-483565312 ) EXECUTE-METHOD
   
   
   
   §tempvar SETVTOP
   
   PURGE 1
   
   0 §break28454 BRANCH
   
   \ new statement
   0 §break28454 LABEL
   
   \ new statement
   
   \ new statement
   DROP
;


: Thread.classMonitorEnter§-433167616
   1 VALLOCATE LOCAL §base0
   
   \ new statement
   0
   LOCALS t |
   
   \ new statement
   DI
   
   \ new statement
   Thread._staticThread
   @
   ThreadState.getState§366110464
   0=
   
   \ new statement
   IF
      
      \ new statement
      
      \ new statement
      0
      Thread._staticThread
      !
      
      \ new statement
      0
      Thread._staticBlocking
      !
      
      \ new statement
      0 §break28480 LABEL
      
      \ new statement
      
      \ new statement
      DROP
      
      \ new statement
   ENDIF
   
   \ new statement
   Thread.currentThread§-1671470336
   DUP §base0 V! TO t
   
   \ new statement
   
   \ new statement
   BEGIN
      
      \ new statement
      t
      Thread._staticThread
      @
      <> and_30416 0BRANCH! DROP
      Thread._staticThread
      @
      0<>
      and_30416 LABEL
      
      \ new statement
   WHILE
      
      \ new statement
      ThreadState.BLOCKED
      @
      -1457236992
      0 S>D
      ThreadState.switchTask§2070835968
      
      \ new statement
      
      \ new statement
   REPEAT
   
   \ new statement
   0 §break28481 LABEL
   
   \ new statement
   
   \ new statement
   DROP
   
   \ new statement
   t
   Thread._staticThread
   !
   
   \ new statement
   Thread._staticBlocking
   A:R@ @ 1+ R@ !
   A:DROP
   
   \ new statement
   EI
   
   \ new statement
   0 §break28479 LABEL
   
   \ new statement
   
   §base0 SETVTOP
   PURGE 2
   
   \ new statement
   DROP
;


: Thread.classMonitorLeave§-1691393280
   
   \ new statement
   
   \ new statement
   DI
   
   \ new statement
   Thread._staticBlocking
   A:R@ @ 1- DUP R@ !
   A:DROP
   0=
   
   \ new statement
   IF
      
      \ new statement
      
      \ new statement
      ThreadState.BLOCKED
      @
      -1457236992
      FALSE
      ThreadState.unblockTask§-2137950976
      
      \ new statement
      0
      Thread._staticThread
      !
      
      \ new statement
      0 §break28484 LABEL
      
      \ new statement
      
      \ new statement
      DROP
      
      \ new statement
   ENDIF
   
   \ new statement
   EI
   
   \ new statement
   0 §break28483 LABEL
   
   \ new statement
   
   \ new statement
   DROP
;
( *
* Counts the number of stack frames in this thread. The thread must
* be suspended.
*
* @return     the number of stack frames in this thread.
* @exception  IllegalThreadStateException  if this thread is not
*             suspended.
* @deprecated The definition of this call depends on {@link #suspend},
*		   which is deprecated.  Further, the results of this call
*		   were never well-defined.
)

:LOCAL Thread.countStackFrames§332423936
   1 VALLOCATE LOCAL §base0
   DUP 0 §base0 + V! LOCAL §this
   
   \ new statement
   
   \ new statement
   §this
   ThreadState.getState§366110464
   ThreadState.SUSPEND
   @
   <>
   
   \ new statement
   IF
      
      \ new statement
      IllegalThreadStateException§-782350080.table -1901442816 EXECUTE-NEW
      JavaArray.handler§-1096259584
      
      \ new statement
      
      \ new statement
   ENDIF
   
   \ new statement
   2
   0 §break28456 BRANCH
   
   \ new statement
   0 §break28456 LABEL
   
   \ new statement
   §base0 SETVTOP
   PURGE 2
   
   \ new statement
   DROP
;
( *
* Returns a reference to the currently executing thread object.
*
* @return  the currently executing thread.
)

: Thread.currentThread§-1671470336
   
   \ new statement
   
   \ new statement
   ThreadState.currentThread§-1671470336
   DUP 0 V!
   0 §break28413 BRANCH
   
   \ new statement
   0 §break28413 LABEL
   
   \ new statement
   
   \ new statement
   DROP
;
( *
* Destroys this thread, without any cleanup. Any monitors it has
* locked remain locked. (This method is not implemented.)
)

:LOCAL Thread.destroy§855073792
   1 VALLOCATE LOCAL §base0
   DUP 0 §base0 + V! LOCAL §this
   
   \ new statement
   
   \ new statement
   NoSuchMethodError§-405778944.table 1581272576 EXECUTE-NEW
   JavaArray.handler§-1096259584
   
   \ new statement
   
   \ new statement
   0 §break28443 LABEL
   
   \ new statement
   §base0 SETVTOP
   PURGE 2
   
   \ new statement
   DROP
;
( *
* Prints a stack trace of the current thread. This method is used
* only for debugging.
*
* @see     java.lang.Throwable#printStackTrace()
)

: Thread.dumpStack§1879073792
   
   \ new statement
   
   \ new statement
   \ new Exception("Stack trace").printStackTrace();
   
   
   \ new statement
   0 §break28471 LABEL
   
   \ new statement
   
   \ new statement
   DROP
;
( *
* Copies into the specified array every active thread in
* the current thread's thread group and its subgroups. This method simply
* calls the <code>enumerate</code> method of the current thread's thread
* group with the array argument.
* <p>
* First, if there is a security manager, that <code>enumerate</code>
* method calls the security
* manager's <code>checkAccess</code> method
* with the thread group as its argument. This may result
* in throwing a <code>SecurityException</code>.
*
* @param tarray an array of Thread objects to copy to
* @return  the number of threads put into the array
* @exception  SecurityException  if a security manager exists and its
*             <code>checkAccess</code> method doesn't allow the operation.
* @see     java.lang.ThreadGroup#enumerate(java.lang.Thread[])
* @see     java.lang.SecurityManager#checkAccess(java.lang.ThreadGroup)
)

: Thread.enumerate§-1458019072
   1 VALLOCATE LOCAL §base0
   DUP 0 §base0 + V! LOCAL tarray
   
   \ new statement
   
   \ new statement
   
   2 VALLOCATE LOCAL §tempvar
   Thread.currentThread§-1671470336 DUP §tempvar V!
   >R
   R> 409036544 TRUE ( equation.Thread.getThreadGroup§409036544 ) EXECUTE-METHOD DUP §tempvar 4 + V!
   >R
   tarray
   R> -1458019072 TRUE ( equation.ThreadGroup.enumerate§-1458019072 ) EXECUTE-METHOD
   
   
   
   §tempvar SETVTOP
   
   PURGE 1
   
   0 §break28455 BRANCH
   
   \ new statement
   0 §break28455 LABEL
   
   \ new statement
   
   §base0 SETVTOP
   PURGE 2
   
   \ new statement
   DROP
;
( *
* This method is called by the system to give a Thread
* a chance to clean up before it actually exits.
)

:LOCAL Thread.exit§1390306560
   1 VALLOCATE LOCAL §base0
   DUP 0 §base0 + V! LOCAL §this
   
   \ new statement
   
   \ new statement
   §this CELL+ @ 36 + ( equation.Thread.group )    @
   0<>
   
   \ new statement
   IF
      
      \ new statement
      
      \ new statement
      §this CELL+ @ 36 + ( equation.Thread.group ) @ >R
      §this
      R> 453997056 TRUE ( equation.ThreadGroup.remove§453997056 ) EXECUTE-METHOD
      
      \ new statement
      0
      §this CELL+ @ 36 + ( equation.Thread.group )       !
      
      \ new statement
      0 §break28434 LABEL
      
      \ new statement
      
      \ new statement
      DROP
      
      \ new statement
   ENDIF
   
   \ new statement
   (  Aggressively null object connected to Thread: see bug 4006245  )
   
   \ new statement
   0
   §this CELL+ @ 32 + ( equation.Thread.target )    !
   
   \ new statement
   §this
   ThreadState.exit§-1797888768
   
   \ new statement
   0 §break28433 LABEL
   
   \ new statement
   §base0 SETVTOP
   PURGE 2
   
   \ new statement
   DROP
;
( *
* Returns this thread's name.
*
* @return  this thread's name.
* @see     #setName
* @see     java.lang.Thread#setName(java.lang.String)
)

:LOCAL Thread.getName§-1704827136
   1 VALLOCATE LOCAL §base0
   DUP 0 §base0 + V! LOCAL §this
   
   \ new statement
   
   \ new statement
   §this CELL+ @ 28 + ( equation.Thread.name )    @
   String.valueOf§789214720
   DUP 0 V!
   0 §break28452 BRANCH
   
   \ new statement
   0 §break28452 LABEL
   
   \ new statement
   §base0 SETVTOP
   PURGE 2
   
   \ new statement
   DROP
;
( *
* Returns this thread's priority.
*
* @return  this thread's priority.
* @see     #setPriority
* @see     java.lang.Thread#setPriority(int)
)

:LOCAL Thread.getPriority§-487889152
   1 VALLOCATE LOCAL §base0
   DUP 0 §base0 + V! LOCAL §this
   
   \ new statement
   
   \ new statement
   §this CELL+ @ 44 + ( equation.Thread.priority )    @
   0 §break28450 BRANCH
   
   \ new statement
   0 §break28450 LABEL
   
   \ new statement
   §base0 SETVTOP
   PURGE 2
   
   \ new statement
   DROP
;
( *
* Returns the thread group to which this thread belongs.
* This method returns null if this thread has died
* (been stopped).
*
* @return  this thread's thread group.
)

:LOCAL Thread.getThreadGroup§409036544
   1 VALLOCATE LOCAL §base0
   DUP 0 §base0 + V! LOCAL §this
   
   \ new statement
   
   \ new statement
   §this CELL+ @ 36 + ( equation.Thread.group )    @
   DUP 0 V!
   0 §break28453 BRANCH
   
   \ new statement
   0 §break28453 LABEL
   
   \ new statement
   §base0 SETVTOP
   PURGE 2
   
   \ new statement
   DROP
;
( *
* Returns <tt>true</tt> if and only if the current thread holds the
* monitor lock on the specified object.
*
* <p>This method is designed to allow a program to assert that
* the current thread already holds a specified lock:
* <pre>
*     assert Thread.holdsLock(obj);
* </pre>
*
* @param  obj the object on which to test lock ownership
* @throws NullPointerException if obj is <tt>null</tt>
* @return <tt>true</tt> if the current thread holds the monitor lock on
*         the specified object.
* @since 1.4
)

: Thread.holdsLock§635660288
   1 VALLOCATE LOCAL §base0
   DUP 0 §base0 + V! LOCAL obj
   
   \ new statement
   
   \ new statement
   
   1 VALLOCATE LOCAL §tempvar
   obj CELL+ @ 28 + ( equation.Object._dynamicThread )    @
   0=  0=! or_30509 0BRANCH DROP
   obj CELL+ @ 28 + ( equation.Object._dynamicThread )    @
   Thread.currentThread§-1671470336 DUP §tempvar V!
   =
   or_30509 LABEL
   
   
   §tempvar SETVTOP
   
   PURGE 1
   
   0 §break28478 BRANCH
   
   \ new statement
   0 §break28478 LABEL
   
   \ new statement
   
   §base0 SETVTOP
   PURGE 2
   
   \ new statement
   DROP
;
( *
* Initialize a Thread.
*
* @param g the Thread group
* @param target the object whose run() method gets called
* @param name the name of the new Thread
* @param stackSize the desired stack size for the new thread, or
*        zero to indicate that this parameter is to be ignored.
)

:LOCAL Thread.init§-1783011072
   5 VALLOCATE LOCAL §base0
   DUP 12 §base0 + V! LOCAL §this
   2LOCAL stackSize
   DUP 0 §base0 + V! LOCAL name
   DUP 4 §base0 + V! LOCAL target
   DUP 8 §base0 + V! LOCAL g
   
   \ new statement
   0
   LOCALS parent |
   
   \ new statement
   Thread.currentThread§-1671470336
   DUP 16 §base0 + V! TO parent
   
   \ new statement
   g
   0= and_30516 0BRANCH! DROP
   parent
   0<>
   and_30516 LABEL
   
   \ new statement
   IF
      
      \ new statement
      
      \ new statement
      parent >R
      R> 409036544 TRUE ( equation.Thread.getThreadGroup§409036544 ) EXECUTE-METHOD
      DUP 8 §base0 + V! TO g
      
      \ new statement
      0 §break28421 LABEL
      
      \ new statement
      
      \ new statement
      DROP
      
      \ new statement
   ENDIF
   
   \ new statement
   g
   §this CELL+ @ 36 + ( equation.Thread.group )    !
   
   \ new statement
   parent
   0<>
   
   \ new statement
   IF
      
      \ new statement
      parent >R
      R> -1531156224 TRUE ( equation.Thread.isDaemon§-1531156224 ) EXECUTE-METHOD
      §this CELL+ @ 52 + ( equation.Thread.daemon )       !
      
      \ new statement
   ENDIF
   
   \ new statement
   parent
   0<>
   
   IF
      parent >R
      R> -487889152 TRUE ( equation.Thread.getPriority§-487889152 ) EXECUTE-METHOD
      
   ELSE
      Thread.MAX_PRIORITY
      @
      
   ENDIF
   §this CELL+ @ 44 + ( equation.Thread.priority )    !
   
   \ new statement
   name >R
   R> 629044224 TRUE ( equation.String.toCharArray§629044224 ) EXECUTE-METHOD
   §this CELL+ @ 28 + ( equation.Thread.name )    !
   
   \ new statement
   target
   §this CELL+ @ 32 + ( equation.Thread.target )    !
   
   \ new statement
   §this >R
   §this CELL+ @ 44 + ( equation.Thread.priority )    @
   R> 1035629312 TRUE ( equation.Thread.setPriority§1035629312 ) EXECUTE-METHOD
   
   \ new statement
   (  Stash the specified stack size in case the VM cares  )
   
   \ new statement
   stackSize  D>S
   §this CELL+ @ 64 + ( equation.Thread.stackSize )    !
   
   \ new statement
   g >R
   §this
   R> 335503616 TRUE ( equation.ThreadGroup.add§335503616 ) EXECUTE-METHOD
   
   \ new statement
   0 §break28420 LABEL
   
   \ new statement
   
   
   
   
   
   §base0 SETVTOP
   PURGE 7
   
   \ new statement
   DROP
;
( *
* Interrupts this thread.
*
* <p> First the {@link #checkAccess() checkAccess} method of this thread
* is invoked, which may cause a {@link SecurityException} to be thrown.
*
* <p> If this thread is blocked in an invocation of the {@link
* Object#wait() wait()}, {@link Object#wait(long) wait(long)}, or {@link
* Object#wait(long, int) wait(long, int)} methods of the {@link Object}
* class, or of the {@link #join()}, {@link #join(long)}, {@link
* #join(long, int)}, {@link #sleep(long)}, or {@link #sleep(long, int)},
* methods of this class, then its interrupt status will be cleared and it
* will receive an {@link InterruptedException}.
*
* <p> If this thread is blocked in an I/O operation upon an {@link
* java.nio.channels.InterruptibleChannel </code>interruptible
* channel<code>} then the channel will be closed, the thread's interrupt
* status will be set, and the thread will receive a {@link
* java.nio.channels.ClosedByInterruptException}.
*
* <p> If this thread is blocked in a {@link java.nio.channels.Selector}
* then the thread's interrupt status will be set and it will return
* immediately from the selection operation, possibly with a non-zero
* value, just as if the selector's {@link
* java.nio.channels.Selector#wakeup wakeup} method were invoked.
*
* <p> If none of the previous conditions hold then this thread's interrupt
* status will be set. </p>
*
* @throws  SecurityException
*          if the current thread cannot modify this thread
*
* @revised 1.4
* @spec JSR-51
)

:LOCAL Thread.interrupt§384592128
   2 VALLOCATE LOCAL §base0
   DUP 0 §base0 + V! LOCAL §this
   
   \ new statement
   0
   LOCALS b |
   
   \ new statement
   TRUE
   §this CELL+ @ 60 + ( equation.Thread.interrupt )    !
   
   \ new statement
   §this CELL+ @ 40 + ( equation.Thread.blocker )    @
   DUP 4 §base0 + V! TO b
   
   \ new statement
   b
   0<>
   
   \ new statement
   IF
      
      \ new statement
      
      \ new statement
      b >R
      R> 384592128 TRUE ( equation.Interruptible.interrupt§384592128 ) EXECUTE-METHOD
      
      \ new statement
      0 §break28439 LABEL
      
      \ new statement
      
      \ new statement
      DROP
      
      \ new statement
   ENDIF
   
   \ new statement
   0 §break28438 LABEL
   
   \ new statement
   
   §base0 SETVTOP
   PURGE 3
   
   \ new statement
   DROP
;
( *
* Tests whether the current thread has been interrupted.  The
* <i>interrupted status</i> of the thread is cleared by this method.  In
* other words, if this method were to be called twice in succession, the
* second call would return false (unless the current thread were
* interrupted again, after the first call had cleared its interrupted
* status and before the second call had examined it).
*
* @return  <code>true</code> if the current thread has been interrupted;
*          <code>false</code> otherwise.
* @see java.lang.Thread#isInterrupted()
)

: Thread.interrupted§-1804572416
   
   \ new statement
   
   \ new statement
   
   1 VALLOCATE LOCAL §tempvar
   Thread.currentThread§-1671470336 DUP §tempvar V!
   >R
   TRUE
   R> 992241920 TRUE ( equation.Thread.isInterrupted§992241920 ) EXECUTE-METHOD
   
   
   §tempvar SETVTOP
   
   PURGE 1
   
   0 §break28440 BRANCH
   
   \ new statement
   0 §break28440 LABEL
   
   \ new statement
   
   \ new statement
   DROP
;
( *
* Tests if this thread is alive. A thread is alive if it has
* been started and has not yet died.
*
* @return  <code>true</code> if this thread is alive;
*          <code>false</code> otherwise.
)

:LOCAL Thread.isAlive§-128489216
   1 VALLOCATE LOCAL §base0
   DUP 0 §base0 + V! LOCAL §this
   
   \ new statement
   
   \ new statement
   §this
   ThreadState.getState§366110464
   ThreadState.RUNNING
   @
   =
   0 §break28444 BRANCH
   
   \ new statement
   0 §break28444 LABEL
   
   \ new statement
   §base0 SETVTOP
   PURGE 2
   
   \ new statement
   DROP
;
( *
* Tests if this thread is a daemon thread.
*
* @return  <code>true</code> if this thread is a daemon thread;
*          <code>false</code> otherwise.
* @see     java.lang.Thread#setDaemon(boolean)
)

:LOCAL Thread.isDaemon§-1531156224
   1 VALLOCATE LOCAL §base0
   DUP 0 §base0 + V! LOCAL §this
   
   \ new statement
   
   \ new statement
   §this CELL+ @ 52 + ( equation.Thread.daemon )    @
   0 §break28474 BRANCH
   
   \ new statement
   0 §break28474 LABEL
   
   \ new statement
   §base0 SETVTOP
   PURGE 2
   
   \ new statement
   DROP
;
( *
* Tests whether this thread has been interrupted.  The <i>interrupted
* status</i> of the thread is unaffected by this method.
*
* @return  <code>true</code> if this thread has been interrupted;
*          <code>false</code> otherwise.
* @see     java.lang.Thread#interrupted()
)

:LOCAL Thread.isInterrupted§426797312
   1 VALLOCATE LOCAL §base0
   DUP 0 §base0 + V! LOCAL §this
   
   \ new statement
   
   \ new statement
   §this >R
   FALSE
   R> 992241920 TRUE ( equation.Thread.isInterrupted§992241920 ) EXECUTE-METHOD
   0 §break28441 BRANCH
   
   \ new statement
   0 §break28441 LABEL
   
   \ new statement
   §base0 SETVTOP
   PURGE 2
   
   \ new statement
   DROP
;
( *
* Tests if some Thread has been interrupted.  The interrupted state
* is reset or not based on the value of ClearInterrupted that is
* passed.
)

:LOCAL Thread.isInterrupted§992241920
   1 VALLOCATE LOCAL §base0
   DUP 0 §base0 + V! LOCAL §this
   LOCAL ClearInterrupted
   
   \ new statement
   0
   LOCALS f |
   
   \ new statement
   §this CELL+ @ 60 + ( equation.Thread.interrupt )    @
   TO f
   
   \ new statement
   §this CELL+ @ 60 + ( equation.Thread.interrupt )    @
   ClearInterrupted 0=
   AND
   §this CELL+ @ 60 + ( equation.Thread.interrupt )    !
   
   \ new statement
   f
   0 §break28442 BRANCH
   
   \ new statement
   0 §break28442 LABEL
   
   \ new statement
   
   
   §base0 SETVTOP
   PURGE 4
   
   \ new statement
   DROP
;
( *
* Waits for this thread to die.
*
* @exception  InterruptedException if another thread has interrupted
*             the current thread.  The <i>interrupted status</i> of the
*             current thread is cleared when this exception is thrown.
)

:LOCAL Thread.join§-1002214912
   1 VALLOCATE LOCAL §base0
   DUP 0 §base0 + V! LOCAL §this
   LOCAL §exception
   
   \ new statement
   
   \ new statement
   §this >R
   0 S>D
   std30581 VAL
   R> 1404332544 TRUE ( equation.Thread.join§1404332544 ) EXECUTE-METHOD
   FALSE DUP
   IF
      std30581 LABEL TRUE
   ENDIF
   IF
      §exception TO §return
      0 §break28470 BRANCH
   ENDIF
   
   \ new statement
   0 §break28470 LABEL
   
   \ new statement
   
   §base0 SETVTOP
   PURGE 3
   
   \ new statement
   DROP
;
( *
* Waits at most <code>millis</code> milliseconds for this thread to
* die. A timeout of <code>0</code> means to wait forever.
*
* @param      millis   the time to wait in milliseconds.
* @exception  InterruptedException if another thread has interrupted
*             the current thread.  The <i>interrupted status</i> of the
*             current thread is cleared when this exception is thrown.
)

:LOCAL Thread.join§1404332544
   1 VALLOCATE LOCAL §base0
   DUP 0 §base0 + V! LOCAL §this
   LOCAL §exception
   2LOCAL millis
   
   \ new statement
   0 DUP 2DUP
   2LOCALS base now |
   
   \ new statement
   §this LOCAL §synchronized0
   Thread._staticThread  @
   §synchronized0 801271040 TRUE ( Object.monitorEnter ) EXECUTE-METHOD
   
   \ new statement
   MILLISECONDS 2@ TO base
   
   \ new statement
   0 S>D
   TO now
   
   \ new statement
   millis
   D0<
   
   \ new statement
   IF
      
      \ new statement
      
      \ new statement
      
      1 VALLOCATE LOCAL §tempvar
      U" timeout value is negative " COUNT JavaArray.createUnicode§-675323136 DUP §tempvar V!
      IllegalArgumentException§-946714368.table 737495296 EXECUTE-NEW
      §exception TO §return
      
      
      §tempvar SETVTOP
      
      PURGE 1
      
      
      \ new statement
      65537 §break28458 BRANCH
      
      \ new statement
      0 §break28458 LABEL
      
      \ new statement
      
      \ new statement
      0>!
      IF
         10001H - §break28457 BRANCH
      ENDIF
      DROP
      
      \ new statement
   ENDIF
   
   \ new statement
   millis
   D0=
   
   \ new statement
   IF
      
      \ new statement
      
      \ new statement
      
      \ new statement
      BEGIN
         
         \ new statement
         §this >R
         R> -128489216 TRUE ( equation.Thread.isAlive§-128489216 ) EXECUTE-METHOD
         
         \ new statement
      WHILE
         
         \ new statement
         §this >R
         0 S>D
         std30605 VAL
         R> -1913293056 TRUE ( equation.Object.wait§-1913293056 ) EXECUTE-METHOD
         FALSE DUP
         IF
            std30605 LABEL TRUE
         ENDIF
         IF
            §exception TO §return
            131074 §break28460 BRANCH
         ENDIF
         
         \ new statement
         
         \ new statement
      REPEAT
      
      \ new statement
      0 §break28460 LABEL
      
      \ new statement
      
      \ new statement
      0>!
      IF
         10001H - §break28459 BRANCH
      ENDIF
      DROP
      
      \ new statement
      0 §break28459 LABEL
      
      \ new statement
      
      \ new statement
      0>!
      IF
         10001H - §break28457 BRANCH
      ENDIF
      DROP
      
      \ new statement
   ELSE
      
      \ new statement
      
      \ new statement
      0 DUP
      2LOCALS delay |
      
      \ new statement
      BEGIN
         
         \ new statement
         §this >R
         R> -128489216 TRUE ( equation.Thread.isAlive§-128489216 ) EXECUTE-METHOD
         
         \ new statement
      WHILE
         
         \ new statement
         millis
         now
         D-
         TO delay
         
         \ new statement
         delay
         DNEGATE DROP 0< INVERT
         
         \ new statement
         IF
            
            \ new statement
            
            \ new statement
            65537 §break28465 BRANCH
            
            \ new statement
            0 §break28465 LABEL
            
            \ new statement
            
            \ new statement
            0>!
            IF
               10001H - 0<! §break28463 0BRANCH
            ENDIF
            DROP
            
            \ new statement
         ENDIF
         
         \ new statement
         §this >R
         delay
         std30629 VAL
         R> -1913293056 TRUE ( equation.Object.wait§-1913293056 ) EXECUTE-METHOD
         FALSE DUP
         IF
            std30629 LABEL TRUE
         ENDIF
         IF
            §exception TO §return
            131074 §break28463 BRANCH
         ENDIF
         
         \ new statement
         MILLISECONDS 2@ base D- TO now
         
         \ new statement
         
         \ new statement
      REPEAT
      
      \ new statement
      0 §break28463 LABEL
      
      \ new statement
      
      PURGE 1
      
      \ new statement
      0>!
      IF
         10001H - §break28462 BRANCH
      ENDIF
      DROP
      
      \ new statement
      0 §break28462 LABEL
      
      \ new statement
      
      \ new statement
      0>!
      IF
         10001H - §break28457 BRANCH
      ENDIF
      DROP
      
      \ new statement
   ENDIF
   
   \ new statement
   0 §break28457 LABEL
   
   \ new statement
   §synchronized0 1974299904 TRUE ( Object.monitorLeave ) EXECUTE-METHOD
   PURGE 1
   
   \ new statement
   
   
   
   
   §base0 SETVTOP
   PURGE 6
   
   \ new statement
   DROP
;
( *
* Waits at most <code>millis</code> milliseconds plus
* <code>nanos</code> nanoseconds for this thread to die.
*
* @param      millis   the time to wait in milliseconds.
* @param      nanos    0-999999 additional nanoseconds to wait.
* @exception  IllegalArgumentException  if the value of millis is negative
*               the value of nanos is not in the range 0-999999.
* @exception  InterruptedException if another thread has interrupted
*             the current thread.  The <i>interrupted status</i> of the
*             current thread is cleared when this exception is thrown.
)

:LOCAL Thread.join§1573677568
   1 VALLOCATE LOCAL §base0
   DUP 0 §base0 + V! LOCAL §this
   LOCAL §exception
   LOCAL nanos
   2LOCAL millis
   
   \ new statement
   
   \ new statement
   §this LOCAL §synchronized0
   Thread._staticThread  @
   §synchronized0 801271040 TRUE ( Object.monitorEnter ) EXECUTE-METHOD
   
   \ new statement
   millis
   D0<
   
   \ new statement
   IF
      
      \ new statement
      
      \ new statement
      
      1 VALLOCATE LOCAL §tempvar
      U" timeout value is negative " COUNT JavaArray.createUnicode§-675323136 DUP §tempvar V!
      IllegalArgumentException§-946714368.table 737495296 EXECUTE-NEW
      §exception TO §return
      
      
      §tempvar SETVTOP
      
      PURGE 1
      
      
      \ new statement
      65537 §break28467 BRANCH
      
      \ new statement
      0 §break28467 LABEL
      
      \ new statement
      
      \ new statement
      0>!
      IF
         10001H - §break28466 BRANCH
      ENDIF
      DROP
      
      \ new statement
   ENDIF
   
   \ new statement
   nanos
   0<  0=! or_30656 0BRANCH DROP
   nanos
   999999
   >
   or_30656 LABEL
   
   \ new statement
   IF
      
      \ new statement
      
      \ new statement
      
      1 VALLOCATE LOCAL §tempvar
      U" nanosecond timeout value out of range " COUNT JavaArray.createUnicode§-675323136 DUP §tempvar V!
      IllegalArgumentException§-946714368.table 737495296 EXECUTE-NEW
      §exception TO §return
      
      
      §tempvar SETVTOP
      
      PURGE 1
      
      
      \ new statement
      65537 §break28468 BRANCH
      
      \ new statement
      0 §break28468 LABEL
      
      \ new statement
      
      \ new statement
      0>!
      IF
         10001H - §break28466 BRANCH
      ENDIF
      DROP
      
      \ new statement
   ENDIF
   
   \ new statement
   nanos
   500000
   < INVERT  0=! or_30666 0BRANCH DROP
   nanos
   0<> and_30667 0BRANCH! DROP
   millis
   D0=
   and_30667 LABEL
   or_30666 LABEL
   
   \ new statement
   IF
      
      \ new statement
      
      \ new statement
      millis 0 1 D+ TO millis
      
      \ new statement
      0 §break28469 LABEL
      
      \ new statement
      
      \ new statement
      0>!
      IF
         10001H - §break28466 BRANCH
      ENDIF
      DROP
      
      \ new statement
   ENDIF
   
   \ new statement
   §this >R
   millis
   std30675 VAL
   R> 1404332544 TRUE ( equation.Thread.join§1404332544 ) EXECUTE-METHOD
   FALSE DUP
   IF
      std30675 LABEL TRUE
   ENDIF
   IF
      §exception TO §return
      0 §break28466 BRANCH
   ENDIF
   
   \ new statement
   0 §break28466 LABEL
   
   \ new statement
   §synchronized0 1974299904 TRUE ( Object.monitorLeave ) EXECUTE-METHOD
   PURGE 1
   
   \ new statement
   
   
   
   §base0 SETVTOP
   PURGE 5
   
   \ new statement
   DROP
;


: Thread.nextThreadNum§79719936
   
   \ new statement
   
   \ new statement
   Thread.classMonitorEnter§-433167616
   
   \ new statement
   Thread.threadInitNumber
   A:R@ @ DUP 1+ R@ !
   A:DROP
   0 §break28412 BRANCH
   
   \ new statement
   0 §break28412 LABEL
   
   \ new statement
   Thread.classMonitorLeave§-1691393280
   
   \ new statement
   
   \ new statement
   DROP
;
( *
* Resumes a suspended thread.
* <p>
* First, the <code>checkAccess</code> method of this thread is called
* with no arguments. This may result in throwing a
* <code>SecurityException</code> (in the current thread).
* <p>
* If the thread is alive but suspended, it is resumed and is
* permitted to make progress in its execution.
*
* @exception  SecurityException  if the current thread cannot modify this
*               thread.
* @see        #checkAccess
* @see        java.lang.Thread#suspend()
* @deprecated This method exists solely for use with {@link #suspend},
*     which has been deprecated because it is deadlock-prone.
*     For more information, see
*     <a href="{@docRoot}/../guide/misc/threadPrimitiveDeprecation.html">Why
*     are Thread.stop, Thread.suspend and Thread.resume Deprecated?</a>.
)

:LOCAL Thread.resume§599945728
   1 VALLOCATE LOCAL §base0
   DUP 0 §base0 + V! LOCAL §this
   
   \ new statement
   
   \ new statement
   ThreadState.SUSPEND
   @
   §this
   FALSE
   ThreadState.unblockTask§1685746944
   
   \ new statement
   0 §break28446 LABEL
   
   \ new statement
   §base0 SETVTOP
   PURGE 2
   
   \ new statement
   DROP
;
( *
* If this thread was constructed using a separate
* <code>Runnable</code> run object, then that
* <code>Runnable</code> object's <code>run</code> method is called;
* otherwise, this method does nothing and returns.
* <p>
* Subclasses of <code>Thread</code> should override this method.
*
* @see     java.lang.Thread#start()
* @see     java.lang.Thread#stop()
* @see     java.lang.Thread#Thread(java.lang.ThreadGroup,
*          java.lang.Runnable, java.lang.String)
* @see     java.lang.Runnable#run()
)

:LOCAL Thread.run§706638336
   1 VALLOCATE LOCAL §base0
   DUP 0 §base0 + V! LOCAL §this
   
   \ new statement
   
   \ new statement
   §this CELL+ @ 32 + ( equation.Thread.target )    @
   0<>
   
   \ new statement
   IF
      
      \ new statement
      
      \ new statement
      §this CELL+ @ 32 + ( equation.Thread.target ) @ >R
      R> 706638336 TRUE ( equation.Runnable.run§706638336 ) EXECUTE-METHOD
      
      \ new statement
      0 §break28432 LABEL
      
      \ new statement
      
      \ new statement
      DROP
      
      \ new statement
   ENDIF
   
   \ new statement
   0 §break28431 LABEL
   
   \ new statement
   §base0 SETVTOP
   PURGE 2
   
   \ new statement
   DROP
;
( *
* Marks this thread as either a daemon thread or a user thread. The
* Java Virtual Machine exits when the only threads running are all
* daemon threads.
* <p>
* This method must be called before the thread is started.
* <p>
* This method first calls the <code>checkAccess</code> method
* of this thread
* with no arguments. This may result in throwing a
* <code>SecurityException </code>(in the current thread).
*
* @param      on   if <code>true</code>, marks this thread as a
*                  daemon thread.
* @exception  IllegalThreadStateException  if this thread is active.
* @exception  SecurityException  if the current thread cannot modify
*               this thread.
* @see        java.lang.Thread#isDaemon()
* @see          #checkAccess
)

:LOCAL Thread.setDaemon§-1235782912
   1 VALLOCATE LOCAL §base0
   DUP 0 §base0 + V! LOCAL §this
   LOCAL on
   
   \ new statement
   
   \ new statement
   §this >R
   R> -128489216 TRUE ( equation.Thread.isAlive§-128489216 ) EXECUTE-METHOD
   
   \ new statement
   IF
      
      \ new statement
      
      \ new statement
      IllegalThreadStateException§-782350080.table -1901442816 EXECUTE-NEW
      JavaArray.handler§-1096259584
      
      \ new statement
      
      \ new statement
      0 §break28473 LABEL
      
      \ new statement
      
      \ new statement
      0>!
      IF
         10001H - §break28472 BRANCH
      ENDIF
      DROP
      
      \ new statement
   ENDIF
   
   \ new statement
   on
   §this CELL+ @ 52 + ( equation.Thread.daemon )    !
   
   \ new statement
   0 §break28472 LABEL
   
   \ new statement
   
   §base0 SETVTOP
   PURGE 3
   
   \ new statement
   DROP
;
( *
* Changes the name of this thread to be equal to the argument
* <code>name</code>.
* <p>
* First the <code>checkAccess</code> method of this thread is called
* with no arguments. This may result in throwing a
* <code>SecurityException</code>.
*
* @param      name   the new name for this thread.
* @exception  SecurityException  if the current thread cannot modify this
*               thread.
* @see        #getName
* @see        java.lang.Thread#checkAccess()
* @see        java.lang.Thread#getName()
)

:LOCAL Thread.setName§-1752141056
   2 VALLOCATE LOCAL §base0
   DUP 4 §base0 + V! LOCAL §this
   DUP 0 §base0 + V! LOCAL name
   
   \ new statement
   
   \ new statement
   name >R
   R> 629044224 TRUE ( equation.String.toCharArray§629044224 ) EXECUTE-METHOD
   §this CELL+ @ 28 + ( equation.Thread.name )    !
   
   \ new statement
   0 §break28451 LABEL
   
   \ new statement
   
   §base0 SETVTOP
   PURGE 3
   
   \ new statement
   DROP
;
( *
* Changes the priority of this thread.
* <p>
* First the <code>checkAccess</code> method of this thread is called
* with no arguments. This may result in throwing a
* <code>SecurityException</code>.
* <p>
* Otherwise, the priority of this thread is set to the smaller of
* the specified <code>newPriority</code> and the maximum permitted
* priority of the thread's thread group.
*
* @param newPriority priority to set this thread to
* @exception  IllegalArgumentException  If the priority is not in the
*               range <code>MIN_PRIORITY</code> to
*               <code>MAX_PRIORITY</code>.
* @exception  SecurityException  if the current thread cannot modify
*               this thread.
* @see        #getPriority
* @see        java.lang.Thread#checkAccess()
* @see        java.lang.Thread#getPriority()
* @see        java.lang.Thread#getThreadGroup()
* @see        java.lang.Thread#MAX_PRIORITY
* @see        java.lang.Thread#MIN_PRIORITY
* @see        java.lang.ThreadGroup#getMaxPriority()
)

:LOCAL Thread.setPriority§1035629312
   1 VALLOCATE LOCAL §base0
   DUP 0 §base0 + V! LOCAL §this
   LOCAL newPriority
   
   \ new statement
   
   \ new statement
   newPriority
   Thread.MAX_PRIORITY
   @
   >  0=! or_30725 0BRANCH DROP
   newPriority
   Thread.MIN_PRIORITY
   @
   <
   or_30725 LABEL
   
   \ new statement
   IF
      
      \ new statement
      
      \ new statement
      IllegalArgumentException§-946714368.table -564049664 EXECUTE-NEW
      JavaArray.handler§-1096259584
      
      \ new statement
      
      \ new statement
      0 §break28448 LABEL
      
      \ new statement
      
      \ new statement
      0>!
      IF
         10001H - §break28447 BRANCH
      ENDIF
      DROP
      
      \ new statement
   ENDIF
   
   \ new statement
   newPriority
   §this CELL+ @ 36 + ( equation.Thread.group ) @ >R
   R> 836921088 TRUE ( equation.ThreadGroup.getMaxPriority§836921088 ) EXECUTE-METHOD
   >
   
   \ new statement
   IF
      
      \ new statement
      
      \ new statement
      §this CELL+ @ 36 + ( equation.Thread.group ) @ >R
      R> 836921088 TRUE ( equation.ThreadGroup.getMaxPriority§836921088 ) EXECUTE-METHOD
      TO newPriority
      
      \ new statement
      0 §break28449 LABEL
      
      \ new statement
      
      \ new statement
      0>!
      IF
         10001H - §break28447 BRANCH
      ENDIF
      DROP
      
      \ new statement
   ENDIF
   
   \ new statement
   newPriority
   §this CELL+ @ 44 + ( equation.Thread.priority )    !
   
   \ new statement
   0 §break28447 LABEL
   
   \ new statement
   
   §base0 SETVTOP
   PURGE 3
   
   \ new statement
   DROP
;
( *
* Causes the currently executing thread to sleep (cease execution)
* for the specified number of milliseconds plus the specified number
* of nanoseconds. The thread does not lose ownership of any monitors.
*
* @param      millis   the length of time to sleep in milliseconds.
* @param      nanos    0-999999 additional nanoseconds to sleep.
* @exception  IllegalArgumentException  if the value of millis is
*             negative or the value of nanos is not in the range
*             0-999999.
* @exception  InterruptedException if another thread has interrupted
*             the current thread.  The <i>interrupted status</i> of the
*             current thread is cleared when this exception is thrown.
* @see        java.lang.Object#notify()
)

: Thread.sleep§-2034666752
   LOCAL §exception
   LOCAL nanos
   2LOCAL millis
   
   \ new statement
   
   \ new statement
   millis
   D0<
   
   \ new statement
   IF
      
      \ new statement
      
      \ new statement
      
      1 VALLOCATE LOCAL §tempvar
      U" timeout value is negative " COUNT JavaArray.createUnicode§-675323136 DUP §tempvar V!
      IllegalArgumentException§-946714368.table 737495296 EXECUTE-NEW
      §exception TO §return
      
      
      §tempvar SETVTOP
      
      PURGE 1
      
      
      \ new statement
      65537 §break28417 BRANCH
      
      \ new statement
      0 §break28417 LABEL
      
      \ new statement
      
      \ new statement
      0>!
      IF
         10001H - §break28416 BRANCH
      ENDIF
      DROP
      
      \ new statement
   ENDIF
   
   \ new statement
   nanos
   0<  0=! or_30757 0BRANCH DROP
   nanos
   999999
   >
   or_30757 LABEL
   
   \ new statement
   IF
      
      \ new statement
      
      \ new statement
      
      1 VALLOCATE LOCAL §tempvar
      U" nanosecond timeout value out of range " COUNT JavaArray.createUnicode§-675323136 DUP §tempvar V!
      IllegalArgumentException§-946714368.table 737495296 EXECUTE-NEW
      §exception TO §return
      
      
      §tempvar SETVTOP
      
      PURGE 1
      
      
      \ new statement
      65537 §break28418 BRANCH
      
      \ new statement
      0 §break28418 LABEL
      
      \ new statement
      
      \ new statement
      0>!
      IF
         10001H - §break28416 BRANCH
      ENDIF
      DROP
      
      \ new statement
   ENDIF
   
   \ new statement
   nanos
   500000
   < INVERT  0=! or_30767 0BRANCH DROP
   nanos
   0<> and_30768 0BRANCH! DROP
   millis
   D0=
   and_30768 LABEL
   or_30767 LABEL
   
   \ new statement
   IF
      
      \ new statement
      
      \ new statement
      millis 0 1 D+ TO millis
      
      \ new statement
      0 §break28419 LABEL
      
      \ new statement
      
      \ new statement
      0>!
      IF
         10001H - §break28416 BRANCH
      ENDIF
      DROP
      
      \ new statement
   ENDIF
   
   \ new statement
   millis
   std30776 VAL
   Thread.sleep§359035648
   FALSE DUP
   IF
      std30776 LABEL TRUE
   ENDIF
   IF
      §exception TO §return
      0 §break28416 BRANCH
   ENDIF
   
   \ new statement
   0 §break28416 LABEL
   
   \ new statement
   
   
   
   PURGE 3
   
   \ new statement
   DROP
;
( *
* Causes the currently executing thread to sleep (temporarily cease
* execution) for the specified number of milliseconds. The thread
* does not lose ownership of any monitors.
*
* @param      millis   the length of time to sleep in milliseconds.
* @exception  InterruptedException if another thread has interrupted
*             the current thread.  The <i>interrupted status</i> of the
*             current thread is cleared when this exception is thrown.
* @see        java.lang.Object#notify()
)

: Thread.sleep§359035648
   LOCAL §exception
   2LOCAL millis
   
   \ new statement
   
   \ new statement
   Thread.interrupted§-1804572416
   
   \ new statement
   IF
      
      \ new statement
      InterruptedException§-1721546496.table 1322010880 EXECUTE-NEW
      §exception TO §return
      
      \ new statement
      0 §break28415 BRANCH
      
      \ new statement
   ENDIF
   
   \ new statement
   ThreadState.RUNNING
   @
   0
   millis
   ThreadState.switchTask§416117504
   
   \ new statement
   0 §break28415 LABEL
   
   \ new statement
   
   
   PURGE 2
   
   \ new statement
   DROP
;
( *
* Causes this thread to begin execution; the Java Virtual Machine
* calls the <code>run</code> method of this thread.
* <p>
* The result is that two threads are running concurrently: the
* current thread (which returns from the call to the
* <code>start</code> method) and the other thread (which executes its
* <code>run</code> method).
*
* @exception  IllegalThreadStateException  if the thread was already
*               started.
* @see        java.lang.Thread#run()
* @see        java.lang.Thread#stop()
)

:LOCAL Thread.start§164066048
   1 VALLOCATE LOCAL §base0
   DUP 0 §base0 + V! LOCAL §this
   
   \ new statement
   
   \ new statement
   §this LOCAL §synchronized0
   Thread._staticThread  @
   §synchronized0 801271040 TRUE ( Object.monitorEnter ) EXECUTE-METHOD
   
   \ new statement
   §this CELL+ @ 56 + ( equation.Thread.stillborn )    A:R@ @ 0=
   A:DROP
   
   \ new statement
   IF
      
      \ new statement
      IllegalThreadStateException§-782350080.table -1901442816 EXECUTE-NEW
      JavaArray.handler§-1096259584
      
      \ new statement
      
      \ new statement
   ENDIF
   
   \ new statement
   §this
   ThreadState.append§1533632768
   
   \ new statement
   0 §break28430 LABEL
   
   \ new statement
   §synchronized0 1974299904 TRUE ( Object.monitorLeave ) EXECUTE-METHOD
   PURGE 1
   
   \ new statement
   §base0 SETVTOP
   PURGE 2
   
   \ new statement
   DROP
;
( *
* Forces the thread to stop executing.
* <p>
* If there is a security manager installed, the <code>checkAccess</code>
* method of this thread is called, which may result in a
* <code>SecurityException</code> being raised (in the current thread).
* <p>
* If this thread is different from the current thread (that is, the current
* thread is trying to stop a thread other than itself) or
* <code>obj</code> is not an instance of <code>ThreadDeath</code>, the
* security manager's <code>checkPermission</code> method (with the
* <code>RuntimePermission("stopThread")</code> argument) is called in
* addition.
* Again, this may result in throwing a
* <code>SecurityException</code> (in the current thread).
* <p>
* If the argument <code>obj</code> is null, a
* <code>NullPointerException</code> is thrown (in the current thread).
* <p>
* The thread represented by this thread is forced to complete
* whatever it is doing abnormally and to throw the
* <code>Throwable</code> object <code>obj</code> as an exception. This
* is an unusual action to take; normally, the <code>stop</code> method
* that takes no arguments should be used.
* <p>
* It is permitted to stop a thread that has not yet been started.
* If the thread is eventually started, it immediately terminates.
*
* @param      obj   the Throwable object to be thrown.
* @exception  SecurityException  if the current thread cannot modify
*               this thread.
* @see        java.lang.Thread#interrupt()
* @see        java.lang.Thread#checkAccess()
* @see        java.lang.Thread#run()
* @see        java.lang.Thread#start()
* @see        java.lang.Thread#stop()
* @see        SecurityManager#checkAccess(Thread)
* @see        SecurityManager#checkPermission
* @deprecated This method is inherently unsafe.  See {@link #stop}
*        (with no arguments) for details.  An additional danger of this
*        method is that it may be used to generate exceptions that the
*        target thread is unprepared to handle (including checked
*        exceptions that the thread could not possibly throw, were it
*        not for this method).
*        For more information, see
*        <a href="{@docRoot}/../guide/misc/threadPrimitiveDeprecation.html">Why
*        are Thread.stop, Thread.suspend and Thread.resume Deprecated?</a>.
)

:LOCAL Thread.stop§-1725402368
   2 VALLOCATE LOCAL §base0
   DUP 4 §base0 + V! LOCAL §this
   DUP 0 §base0 + V! LOCAL obj
   
   \ new statement
   
   \ new statement
   §this LOCAL §synchronized0
   Thread._staticThread  @
   §synchronized0 801271040 TRUE ( Object.monitorEnter ) EXECUTE-METHOD
   
   \ new statement
   ThreadState.TERMINATED
   @
   §this
   ThreadState.switchTask§874672896
   
   \ new statement
   obj
   JavaArray.handler§-1096259584
   
   \ new statement
   
   \ new statement
   0 §break28437 LABEL
   
   \ new statement
   §synchronized0 1974299904 TRUE ( Object.monitorLeave ) EXECUTE-METHOD
   PURGE 1
   
   \ new statement
   
   §base0 SETVTOP
   PURGE 3
   
   \ new statement
   DROP
;
( *
* Forces the thread to stop executing.
* <p>
* If there is a security manager installed, its <code>checkAccess</code>
* method is called with <code>this</code>
* as its argument. This may result in a
* <code>SecurityException</code> being raised (in the current thread).
* <p>
* If this thread is different from the current thread (that is, the current
* thread is trying to stop a thread other than itself), the
* security manager's <code>checkPermission</code> method (with a
* <code>RuntimePermission("stopThread")</code> argument) is called in
* addition.
* Again, this may result in throwing a
* <code>SecurityException</code> (in the current thread).
* <p>
* The thread represented by this thread is forced to stop whatever
* it is doing abnormally and to throw a newly created
* <code>ThreadDeath</code> object as an exception.
* <p>
* It is permitted to stop a thread that has not yet been started.
* If the thread is eventually started, it immediately terminates.
* <p>
* An application should not normally try to catch
* <code>ThreadDeath</code> unless it must do some extraordinary
* cleanup operation (note that the throwing of
* <code>ThreadDeath</code> causes <code>finally</code> clauses of
* <code>try</code> statements to be executed before the thread
* officially dies).  If a <code>catch</code> clause catches a
* <code>ThreadDeath</code> object, it is important to rethrow the
* object so that the thread actually dies.
* <p>
* The top-level error handler that reacts to otherwise uncaught
* exceptions does not print out a message or otherwise notify the
* application if the uncaught exception is an instance of
* <code>ThreadDeath</code>.
*
* @exception  SecurityException  if the current thread cannot
*               modify this thread.
* @see        java.lang.Thread#interrupt()
* @see        java.lang.Thread#checkAccess()
* @see        java.lang.Thread#run()
* @see        java.lang.Thread#start()
* @see        java.lang.ThreadDeath
* @see        java.lang.ThreadGroup#uncaughtException(java.lang.Thread,
*             java.lang.Throwable)
* @see        SecurityManager#checkAccess(Thread)
* @see        SecurityManager#checkPermission
* @deprecated This method is inherently unsafe.  Stopping a thread with
*	     Thread.stop causes it to unlock all of the monitors that it
*	     has locked (as a natural consequence of the unchecked
*	     <code>ThreadDeath</code> exception propagating up the stack).  If
*       any of the objects previously protected by these monitors were in
*       an inconsistent state, the damaged objects become visible to
*       other threads, potentially resulting in arbitrary behavior.  Many
*       uses of <code>stop</code> should be replaced by code that simply
*       modifies some variable to indicate that the target thread should
*       stop running.  The target thread should check this variable
*       regularly, and return from its run method in an orderly fashion
*       if the variable indicates that it is to stop running.  If the
*       target thread waits for long periods (on a condition variable,
*       for example), the <code>interrupt</code> method should be used to
*       interrupt the wait.
*       For more information, see
*       <a href="{@docRoot}/../guide/misc/threadPrimitiveDeprecation.html">Why
*       are Thread.stop, Thread.suspend and Thread.resume Deprecated?</a>.
)

:LOCAL Thread.stop§-74747136
   1 VALLOCATE LOCAL §base0
   DUP 0 §base0 + V! LOCAL §this
   
   \ new statement
   
   \ new statement
   
   \ new statement
   §this
   LOCAL §synchronized0
   Thread._staticThread  @
   §synchronized0 801271040 TRUE ( Object.monitorEnter ) EXECUTE-METHOD
   
   \ new statement
   \ if the thread is alreay dead, return
   
   
   \ new statement
   §this >R
   R> -128489216 TRUE ( equation.Thread.isAlive§-128489216 ) EXECUTE-METHOD
   0=
   
   \ new statement
   IF
      
      \ new statement
      65537 §break28436 BRANCH
      
      \ new statement
   ENDIF
   
   \ new statement
   ThreadState.TERMINATED
   @
   §this
   ThreadState.switchTask§874672896
   
   \ new statement
   ThreadDeath§-477015040.table -1686612992 EXECUTE-NEW
   JavaArray.handler§-1096259584
   
   \ new statement
   
   \ new statement
   0 §break28436 LABEL
   
   \ new statement
   §synchronized0 1974299904 TRUE ( Object.monitorLeave ) EXECUTE-METHOD
   PURGE 1
   
   \ new statement
   
   \ new statement
   0>!
   IF
      10001H - §break28435 BRANCH
   ENDIF
   DROP
   
   \ new statement
   0 §break28435 LABEL
   
   \ new statement
   §base0 SETVTOP
   PURGE 2
   
   \ new statement
   DROP
;
( *
* Suspends this thread.
* <p>
* First, the <code>checkAccess</code> method of this thread is called
* with no arguments. This may result in throwing a
* <code>SecurityException </code>(in the current thread).
* <p>
* If the thread is alive, it is suspended and makes no further
* progress unless and until it is resumed.
*
* @exception  SecurityException  if the current thread cannot modify
*               this thread.
* @see #checkAccess
* @deprecated   This method has been deprecated, as it is
*   inherently deadlock-prone.  If the target thread holds a lock on the
*   monitor protecting a critical system resource when it is suspended, no
*   thread can access this resource until the target thread is resumed. If
*   the thread that would resume the target thread attempts to lock this
*   monitor prior to calling <code>resume</code>, deadlock results.  Such
*   deadlocks typically manifest themselves as "frozen" processes.
*   For more information, see
*   <a href="{@docRoot}/../guide/misc/threadPrimitiveDeprecation.html">Why
*   are Thread.stop, Thread.suspend and Thread.resume Deprecated?</a>.
)

:LOCAL Thread.suspend§-2031258880
   1 VALLOCATE LOCAL §base0
   DUP 0 §base0 + V! LOCAL §this
   
   \ new statement
   
   \ new statement
   ThreadState.SUSPEND
   @
   §this
   0 S>D
   ThreadState.switchTask§416117504
   
   \ new statement
   0 §break28445 LABEL
   
   \ new statement
   §base0 SETVTOP
   PURGE 2
   
   \ new statement
   DROP
;
( *
* Returns a string representation of this thread, including the
* thread's name, priority, and thread group.
*
* @return  a string representation of this thread.
)

:LOCAL Thread.toString§1621718016
   2 VALLOCATE LOCAL §base0
   DUP 0 §base0 + V! LOCAL §this
   
   \ new statement
   0
   LOCALS group |
   
   \ new statement
   §this >R
   R> 409036544 TRUE ( equation.Thread.getThreadGroup§409036544 ) EXECUTE-METHOD
   DUP 4 §base0 + V! TO group
   
   \ new statement
   group
   0<>
   
   \ new statement
   IF
      
      \ new statement
      
      \ new statement
      
      12 VALLOCATE LOCAL §tempvar
      U" Thread[ " COUNT JavaArray.createUnicode§-675323136 DUP §tempvar V!
      §this >R
      R> -1704827136 TRUE ( equation.Thread.getName§-1704827136 ) EXECUTE-METHOD DUP §tempvar 4 + V!
      SWAP  -1461427456 TRUE ( equation.String.concat§-1461427456 ) EXECUTE-METHOD DUP §tempvar 8 + V!
      U" , " COUNT JavaArray.createUnicode§-675323136 DUP §tempvar 12 + V!
      SWAP  -1461427456 TRUE ( equation.String.concat§-1461427456 ) EXECUTE-METHOD DUP §tempvar 16 + V!
      §this >R
      R> -487889152 TRUE ( equation.Thread.getPriority§-487889152 ) EXECUTE-METHOD
      <# 0 ..R JavaArray.createString§-105880832 DUP §tempvar 20 + V!
      SWAP  -1461427456 TRUE ( equation.String.concat§-1461427456 ) EXECUTE-METHOD DUP §tempvar 24 + V!
      U" , " COUNT JavaArray.createUnicode§-675323136 DUP §tempvar 28 + V!
      SWAP  -1461427456 TRUE ( equation.String.concat§-1461427456 ) EXECUTE-METHOD DUP §tempvar 32 + V!
      group >R
      R> -1704827136 TRUE ( equation.ThreadGroup.getName§-1704827136 ) EXECUTE-METHOD DUP §tempvar 36 + V!
      SWAP  -1461427456 TRUE ( equation.String.concat§-1461427456 ) EXECUTE-METHOD DUP §tempvar 40 + V!
      U" ] " COUNT JavaArray.createUnicode§-675323136 DUP §tempvar 44 + V!
      SWAP  -1461427456 TRUE ( equation.String.concat§-1461427456 ) EXECUTE-METHOD
      DUP 0 V!
      
      
      
      
      
      
      
      
      
      
      
      
      §tempvar SETVTOP
      
      PURGE 1
      
      65537 §break28476 BRANCH
      
      \ new statement
      0 §break28476 LABEL
      
      \ new statement
      
      \ new statement
      0>!
      IF
         10001H - §break28475 BRANCH
      ENDIF
      DROP
      
      \ new statement
   ELSE
      
      \ new statement
      
      \ new statement
      
      8 VALLOCATE LOCAL §tempvar
      U" Thread[ " COUNT JavaArray.createUnicode§-675323136 DUP §tempvar V!
      §this >R
      R> -1704827136 TRUE ( equation.Thread.getName§-1704827136 ) EXECUTE-METHOD DUP §tempvar 4 + V!
      SWAP  -1461427456 TRUE ( equation.String.concat§-1461427456 ) EXECUTE-METHOD DUP §tempvar 8 + V!
      U" , " COUNT JavaArray.createUnicode§-675323136 DUP §tempvar 12 + V!
      SWAP  -1461427456 TRUE ( equation.String.concat§-1461427456 ) EXECUTE-METHOD DUP §tempvar 16 + V!
      §this >R
      R> -487889152 TRUE ( equation.Thread.getPriority§-487889152 ) EXECUTE-METHOD
      <# 0 ..R JavaArray.createString§-105880832 DUP §tempvar 20 + V!
      SWAP  -1461427456 TRUE ( equation.String.concat§-1461427456 ) EXECUTE-METHOD DUP §tempvar 24 + V!
      U" ,] " COUNT JavaArray.createUnicode§-675323136 DUP §tempvar 28 + V!
      SWAP  -1461427456 TRUE ( equation.String.concat§-1461427456 ) EXECUTE-METHOD
      DUP 0 V!
      
      
      
      
      
      
      
      
      §tempvar SETVTOP
      
      PURGE 1
      
      65537 §break28477 BRANCH
      
      \ new statement
      0 §break28477 LABEL
      
      \ new statement
      
      \ new statement
      0>!
      IF
         10001H - §break28475 BRANCH
      ENDIF
      DROP
      
      \ new statement
   ENDIF
   
   \ new statement
   0 §break28475 LABEL
   
   \ new statement
   
   §base0 SETVTOP
   PURGE 3
   
   \ new statement
   DROP
;
( *
* Causes the currently executing thread object to temporarily pause
* and allow other threads to execute.
)

: Thread.yield§-1710327552
   
   \ new statement
   
   \ new statement
   0 S>D
   std30854 VAL
   Thread.sleep§359035648
   FALSE DUP
   IF
      std30854 LABEL TRUE
   ENDIF
   IF
      JavaArray.handler§-1096259584
   ENDIF
   
   \ new statement
   0 §break28414 LABEL
   
   \ new statement
   
   \ new statement
   DROP
;
VARIABLE Thread.MAX_PRIORITY
VARIABLE Thread.MIN_PRIORITY
VARIABLE Thread.NORM_PRIORITY
VARIABLE Thread._staticBlocking
VARIABLE Thread._staticThread  HERE 4 - SALLOCATE
VARIABLE Thread.threadInitNumber

A:HERE VARIABLE Thread§-1457236992.table 33 DUP 2* CELLS ALLOT R@ ! A:CELL+
-1360112640 R@ ! A:CELL+ Thread.Thread§-1360112640 VAL R@ ! A:CELL+
-1380166656 R@ ! A:CELL+ Thread.Thread§-1380166656 VAL R@ ! A:CELL+
-1408805888 R@ ! A:CELL+ Thread.Thread§-1408805888 VAL R@ ! A:CELL+
-1768598528 R@ ! A:CELL+ Thread.Thread§-1768598528 VAL R@ ! A:CELL+
1142903808 R@ ! A:CELL+ Thread.Thread§1142903808 VAL R@ ! A:CELL+
1507480576 R@ ! A:CELL+ Thread.Thread§1507480576 VAL R@ ! A:CELL+
353588224 R@ ! A:CELL+ Thread.Thread§353588224 VAL R@ ! A:CELL+
708269056 R@ ! A:CELL+ Thread.Thread§708269056 VAL R@ ! A:CELL+
332423936 R@ ! A:CELL+ Thread.countStackFrames§332423936 VAL R@ ! A:CELL+
855073792 R@ ! A:CELL+ Thread.destroy§855073792 VAL R@ ! A:CELL+
1390306560 R@ ! A:CELL+ Thread.exit§1390306560 VAL R@ ! A:CELL+
-1704827136 R@ ! A:CELL+ Thread.getName§-1704827136 VAL R@ ! A:CELL+
-487889152 R@ ! A:CELL+ Thread.getPriority§-487889152 VAL R@ ! A:CELL+
409036544 R@ ! A:CELL+ Thread.getThreadGroup§409036544 VAL R@ ! A:CELL+
-1783011072 R@ ! A:CELL+ Thread.init§-1783011072 VAL R@ ! A:CELL+
384592128 R@ ! A:CELL+ Thread.interrupt§384592128 VAL R@ ! A:CELL+
-128489216 R@ ! A:CELL+ Thread.isAlive§-128489216 VAL R@ ! A:CELL+
-1531156224 R@ ! A:CELL+ Thread.isDaemon§-1531156224 VAL R@ ! A:CELL+
426797312 R@ ! A:CELL+ Thread.isInterrupted§426797312 VAL R@ ! A:CELL+
992241920 R@ ! A:CELL+ Thread.isInterrupted§992241920 VAL R@ ! A:CELL+
-1002214912 R@ ! A:CELL+ Thread.join§-1002214912 VAL R@ ! A:CELL+
1404332544 R@ ! A:CELL+ Thread.join§1404332544 VAL R@ ! A:CELL+
1573677568 R@ ! A:CELL+ Thread.join§1573677568 VAL R@ ! A:CELL+
599945728 R@ ! A:CELL+ Thread.resume§599945728 VAL R@ ! A:CELL+
706638336 R@ ! A:CELL+ Thread.run§706638336 VAL R@ ! A:CELL+
-1235782912 R@ ! A:CELL+ Thread.setDaemon§-1235782912 VAL R@ ! A:CELL+
-1752141056 R@ ! A:CELL+ Thread.setName§-1752141056 VAL R@ ! A:CELL+
1035629312 R@ ! A:CELL+ Thread.setPriority§1035629312 VAL R@ ! A:CELL+
164066048 R@ ! A:CELL+ Thread.start§164066048 VAL R@ ! A:CELL+
-1725402368 R@ ! A:CELL+ Thread.stop§-1725402368 VAL R@ ! A:CELL+
-74747136 R@ ! A:CELL+ Thread.stop§-74747136 VAL R@ ! A:CELL+
-2031258880 R@ ! A:CELL+ Thread.suspend§-2031258880 VAL R@ ! A:CELL+
1621718016 R@ ! A:CELL+ Thread.toString§1621718016 VAL R@ ! A:CELL+
A:DROP
