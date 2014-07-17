MODULENAME java.lang.Object
(
* @(#)Object.java	1.61 03/01/23
*
* Copyright 2003 Sun Microsystems, Inc. All rights reserved.
* SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
)  ( *
* Class <code>Object</code> is the root of the class hierarchy.
* Every class has <code>Object</code> as a superclass. All objects,
* including arrays, implement the methods of this class.
*
* @author  unascribed
* @version 1.61, 01/23/03
* @see     java.lang.Class
* @since   JDK1.0
)


:LOCAL lang.Object.Object§1827294976
   1 VALLOCATE LOCAL §base0
   
   \ new statement
   0
   LOCALS §this |
   
   \ new statement
   0
   
   \ new statement
   28 MALLOC DROP DUP DUP §base0 V! TO §this OVER
   IF
      OVER CELL+ @ 4 + OVER OVER ! DROP
   ENDIF
   CELL+ @ OVER
   OVER ! NIP
   lang.Object§-1890496768.table OVER 12 + !
   -1890516992 OVER 20 + !
   " Object " OVER 16 + !
   0 OVER 24 + ! DROP
   
   \ new statement
   §this DUP 0 V! ( return object )    
   \ new statement
   0 §break16971 LABEL
   
   \ new statement
   §base0 SETVTOP
   PURGE 2
   
   \ new statement
   DROP
;


: lang.Object.classMonitorEnter§-433167616
   
   \ new statement
   
   \ new statement
   0 §break16956 LABEL
   
   \ new statement
   
   \ new statement
   DROP
;


: lang.Object.classMonitorLeave§-1691393280
   
   \ new statement
   
   \ new statement
   0 §break16957 LABEL
   
   \ new statement
   
   \ new statement
   DROP
;
( *
* Creates and returns a copy of this object.  The precise meaning
* of "copy" may depend on the class of the object. The general
* intent is that, for any object <tt>x</tt>, the expression:
* <blockquote>
* <pre>
* x.clone() != x</pre></blockquote>
* will be true, and that the expression:
* <blockquote>
* <pre>
* x.clone().getClass() == x.getClass()</pre></blockquote>
* will be <tt>true</tt>, but these are not absolute requirements.
* While it is typically the case that:
* <blockquote>
* <pre>
* x.clone().equals(x)</pre></blockquote>
* will be <tt>true</tt>, this is not an absolute requirement.
* <p>
* By convention, the returned object should be obtained by calling
* <tt>super.clone</tt>.  If a class and all of its superclasses (except
* <tt>Object</tt>) obey this convention, it will be the case that
* <tt>x.clone().getClass() == x.getClass()</tt>.
* <p>
* By convention, the object returned by this method should be independent
* of this object (which is being cloned).  To achieve this independence,
* it may be necessary to modify one or more fields of the object returned
* by <tt>super.clone</tt> before returning it.  Typically, this means
* copying any mutable objects that comprise the internal "deep structure"
* of the object being cloned and replacing the references to these
* objects with references to the copies.  If a class contains only
* primitive fields or references to immutable objects, then it is usually
* the case that no fields in the object returned by <tt>super.clone</tt>
* need to be modified.
* <p>
* The method <tt>clone</tt> for class <tt>Object</tt> performs a
* specific cloning operation. First, if the class of this object does
* not implement the interface <tt>Cloneable</tt>, then a
* <tt>CloneNotSupportedException</tt> is thrown. Note that all arrays
* are considered to implement the interface <tt>Cloneable</tt>.
* Otherwise, this method creates a new instance of the class of this
* object and initializes all its fields with exactly the contents of
* the corresponding fields of this object, as if by assignment; the
* contents of the fields are not themselves cloned. Thus, this method
* performs a "shallow copy" of this object, not a "deep copy" operation.
* <p>
* The class <tt>Object</tt> does not itself implement the interface
* <tt>Cloneable</tt>, so calling the <tt>clone</tt> method on an object
* whose class is <tt>Object</tt> will result in throwing an
* exception at run time.
*
* @return     a clone of this instance.
* @exception  CloneNotSupportedException  if the object's class does not
*               support the <code>Cloneable</code> interface. Subclasses
*               that override the <code>clone</code> method can also
*               throw this exception to indicate that an instance cannot
*               be cloned.
* @see java.lang.Cloneable
)

:LOCAL lang.Object.clone§-360160512
   1 VALLOCATE LOCAL §base0
   LOCAL §exception
   DUP 0 §base0 + V! LOCAL §this
   
   \ new statement
   
   \ new statement
   §this ALLOCATED-SIZE A:R@ ALLOCATE DROP A:R>
   
   \ new statement
   §this CELL+ @ R@ CELL+ @ R1@ MOVE
   
   \ new statement
   R@ CELL+ @ DUP INCREFERENCE
   
   \ new statement
   CELL+ DUP INCREFERENCE
   
   \ new statement
   CELL+ INCREFERENCE
   
   \ new statement
   R@ A:2DROP
   
   \ new statement
   0 §break16960 LABEL
   
   \ new statement
   
   §base0 SETVTOP
   PURGE 3
   
   \ new statement
   DROP
;
( *
* Indicates whether some other object is "equal to" this one.
* <p>
* The <code>equals</code> method implements an equivalence relation
* on non-null object references:
* <ul>
* <li>It is <i>reflexive</i>: for any non-null reference value
*     <code>x</code>, <code>x.equals(x)</code> should return
*     <code>true</code>.
* <li>It is <i>symmetric</i>: for any non-null reference values
*     <code>x</code> and <code>y</code>, <code>x.equals(y)</code>
*     should return <code>true</code> if and only if
*     <code>y.equals(x)</code> returns <code>true</code>.
* <li>It is <i>transitive</i>: for any non-null reference values
*     <code>x</code>, <code>y</code>, and <code>z</code>, if
*     <code>x.equals(y)</code> returns <code>true</code> and
*     <code>y.equals(z)</code> returns <code>true</code>, then
*     <code>x.equals(z)</code> should return <code>true</code>.
* <li>It is <i>consistent</i>: for any non-null reference values
*     <code>x</code> and <code>y</code>, multiple invocations of
*     <tt>x.equals(y)</tt> consistently return <code>true</code>
*     or consistently return <code>false</code>, provided no
*     information used in <code>equals</code> comparisons on the
*     objects is modified.
* <li>For any non-null reference value <code>x</code>,
*     <code>x.equals(null)</code> should return <code>false</code>.
* </ul>
* <p>
* The <tt>equals</tt> method for class <code>Object</code> implements
* the most discriminating possible equivalence relation on objects;
* that is, for any non-null reference values <code>x</code> and
* <code>y</code>, this method returns <code>true</code> if and only
* if <code>x</code> and <code>y</code> refer to the same object
* (<code>x == y</code> has the value <code>true</code>).
* <p>
* Note that it is generally necessary to override the <tt>hashCode</tt>
* method whenever this method is overridden, so as to maintain the
* general contract for the <tt>hashCode</tt> method, which states
* that equal objects must have equal hash codes.
*
* @param   obj   the reference object with which to compare.
* @return  <code>true</code> if this object is the same as the obj
*          argument; <code>false</code> otherwise.
* @see     #hashCode()
* @see     java.util.Hashtable
)

:LOCAL lang.Object.equals§-240098048
   2 VALLOCATE LOCAL §base0
   DUP 0 §base0 + V! LOCAL obj
   DUP 4 §base0 + V! LOCAL §this
   
   \ new statement
   
   \ new statement
   §this
   obj
   =
   0 §break16959 BRANCH
   
   \ new statement
   0 §break16959 LABEL
   
   \ new statement
   
   §base0 SETVTOP
   PURGE 3
   
   \ new statement
   DROP
;
( *
* Called by the garbage collector on an object when garbage collection
* determines that there are no more references to the object.
* A subclass overrides the <code>finalize</code> method to dispose of
* system resources or to perform other cleanup.
* <p>
* The general contract of <tt>finalize</tt> is that it is invoked
* if and when the Java<font size="-2"><sup>TM</sup></font> virtual
* machine has determined that there is no longer any
* means by which this object can be accessed by any thread that has
* not yet died, except as a result of an action taken by the
* finalization of some other object or class which is ready to be
* finalized. The <tt>finalize</tt> method may take any action, including
* making this object available again to other threads; the usual purpose
* of <tt>finalize</tt>, however, is to perform cleanup actions before
* the object is irrevocably discarded. For example, the finalize method
* for an object that represents an input/output connection might perform
* explicit I/O transactions to break the connection before the object is
* permanently discarded.
* <p>
* The <tt>finalize</tt> method of class <tt>Object</tt> performs no
* special action; it simply returns normally. Subclasses of
* <tt>Object</tt> may override this definition.
* <p>
* The Java programming language does not guarantee which thread will
* invoke the <tt>finalize</tt> method for any given object. It is
* guaranteed, however, that the thread that invokes finalize will not
* be holding any user-visible synchronization locks when finalize is
* invoked. If an uncaught exception is thrown by the finalize method,
* the exception is ignored and finalization of that object terminates.
* <p>
* After the <tt>finalize</tt> method has been invoked for an object, no
* further action is taken until the Java virtual machine has again
* determined that there is no longer any means by which this object can
* be accessed by any thread that has not yet died, including possible
* actions by other objects or classes which are ready to be finalized,
* at which point the object may be discarded.
* <p>
* The <tt>finalize</tt> method is never invoked more than once by a Java
* virtual machine for any given object.
* <p>
* Any exception thrown by the <code>finalize</code> method causes
* the finalization of this object to be halted, but is otherwise
* ignored.
*
* @throws Throwable the <code>Exception</code> raised by this method
)

:LOCAL lang.Object.finalize§997811712
   1 VALLOCATE LOCAL §base0
   LOCAL §exception
   DUP 0 §base0 + V! LOCAL §this
   
   \ new statement
   
   \ new statement
   0 §break16970 LABEL
   
   \ new statement
   
   §base0 SETVTOP
   PURGE 3
   
   \ new statement
   DROP
;
( *
* Returns a hash code value for the object. This method is
* supported for the benefit of hashtables such as those provided by
* <code>java.util.Hashtable</code>.
* <p>
* The general contract of <code>hashCode</code> is:
* <ul>
* <li>Whenever it is invoked on the same object more than once during
*     an execution of a Java application, the <tt>hashCode</tt> method
*     must consistently return the same integer, provided no information
*     used in <tt>equals</tt> comparisons on the object is modified.
*     This integer need not remain consistent from one execution of an
*     application to another execution of the same application.
* <li>If two objects are equal according to the <tt>equals(Object)</tt>
*     method, then calling the <code>hashCode</code> method on each of
*     the two objects must produce the same integer result.
* <li>It is <em>not</em> required that if two objects are unequal
*     according to the {@link java.lang.Object#equals(java.lang.Object)}
*     method, then calling the <tt>hashCode</tt> method on each of the
*     two objects must produce distinct integer results.  However, the
*     programmer should be aware that producing distinct integer results
*     for unequal objects may improve the performance of hashtables.
* </ul>
* <p>
* As much as is reasonably practical, the hashCode method defined by
* class <tt>Object</tt> does return distinct integers for distinct
* objects. (This is typically implemented by converting the internal
* address of the object into an integer, but this implementation
* technique is not required by the
* Java<font size="-2"><sup>TM</sup></font> programming language.)
*
* @return  a hash code value for this object.
* @see     java.lang.Object#equals(java.lang.Object)
* @see     java.util.Hashtable
)

:LOCAL lang.Object.hashCode§-1604556800
   1 VALLOCATE LOCAL §base0
   DUP 0 §base0 + V! LOCAL §this
   
   \ new statement
   
   \ new statement
   §this
   DUP 0 V!0 §break16958 BRANCH
   
   \ new statement
   0 §break16958 LABEL
   
   \ new statement
   §base0 SETVTOP
   PURGE 2
   
   \ new statement
   DROP
;
( *
* Wakes up a single thread that is waiting on this object's
* monitor. If any threads are waiting on this object, one of them
* is chosen to be awakened. The choice is arbitrary and occurs at
* the discretion of the implementation. A thread waits on an object's
* monitor by calling one of the <code>wait</code> methods.
* <p>
* The awakened thread will not be able to proceed until the current
* thread relinquishes the lock on this object. The awakened thread will
* compete in the usual manner with any other threads that might be
* actively competing to synchronize on this object; for example, the
* awakened thread enjoys no reliable privilege or disadvantage in being
* the next thread to lock this object.
* <p>
* This method should only be called by a thread that is the owner
* of this object's monitor. A thread becomes the owner of the
* object's monitor in one of three ways:
* <ul>
* <li>By executing a synchronized instance method of that object.
* <li>By executing the body of a <code>synchronized</code> statement
*     that synchronizes on the object.
* <li>For objects of type <code>Class,</code> by executing a
*     synchronized static method of that class.
* </ul>
* <p>
* Only one thread at a time can own an object's monitor.
*
* @exception  IllegalMonitorStateException  if the current thread is not
*               the owner of this object's monitor.
* @see        java.lang.Object#notifyAll()
* @see        java.lang.Object#wait()
)

:LOCAL lang.Object.notify§2134994432
   1 VALLOCATE LOCAL §base0
   DUP 0 §base0 + V! LOCAL §this
   
   \ new statement
   
   \ new statement
   0 §break16962 LABEL
   
   \ new statement
   §base0 SETVTOP
   PURGE 2
   
   \ new statement
   DROP
;
( *
* Wakes up all threads that are waiting on this object's monitor. A
* thread waits on an object's monitor by calling one of the
* <code>wait</code> methods.
* <p>
* The awakened threads will not be able to proceed until the current
* thread relinquishes the lock on this object. The awakened threads
* will compete in the usual manner with any other threads that might
* be actively competing to synchronize on this object; for example,
* the awakened threads enjoy no reliable privilege or disadvantage in
* being the next thread to lock this object.
* <p>
* This method should only be called by a thread that is the owner
* of this object's monitor. See the <code>notify</code> method for a
* description of the ways in which a thread can become the owner of
* a monitor.
*
* @exception  IllegalMonitorStateException  if the current thread is not
*               the owner of this object's monitor.
* @see        java.lang.Object#notify()
* @see        java.lang.Object#wait()
)

:LOCAL lang.Object.notifyAll§2130144768
   1 VALLOCATE LOCAL §base0
   DUP 0 §base0 + V! LOCAL §this
   
   \ new statement
   
   \ new statement
   0 §break16963 LABEL
   
   \ new statement
   §base0 SETVTOP
   PURGE 2
   
   \ new statement
   DROP
;
( *
* Returns a string representation of the object. In general, the
* <code>toString</code> method returns a string that
* "textually represents" this object. The result should
* be a concise but informative representation that is easy for a
* person to read.
* It is recommended that all subclasses override this method.
* <p>
* The <code>toString</code> method for class <code>Object</code>
* returns a string consisting of the name of the class of which the
* object is an instance, the at-sign character `<code>@</code>', and
* the unsigned hexadecimal representation of the hash code of the
* object. In other words, this method returns a string equal to the
* value of:
* <blockquote>
* <pre>
* getClass().getName() + '@' + Integer.toHexString(hashCode())
* </pre></blockquote>
*
* @return  a string representation of the object.
)

:LOCAL lang.Object.toString§1621718016
   2 VALLOCATE LOCAL §base0
   DUP 0 §base0 + V! LOCAL §this
   
   \ new statement
   0 DUP DUP
   LOCALS h length string |
   
   \ new statement
   §this CELL+ @ 16 + COUNT TO length TO string
   
   \ new statement
   
   4 VALLOCATE LOCAL §tempvar
   length
   string
   lang.JavaArray.createString§-105880832 DUP §tempvar V!
   U" @ " COUNT lang.JavaArray.createUnicode§-675323136 DUP §tempvar 4 + V!
   OVER  -1461427456 TRUE ( java.lang.String.concat§-1461427456 ) EXECUTE-METHOD DUP §tempvar 8 + V!
   §this A:R@
   R> -1604556800 TRUE ( java.lang.Object.hashCode§-1604556800 ) EXECUTE-METHOD
   lang.Integer.toHexString§1337684992 DUP §tempvar 12 + V!
   OVER  -1461427456 TRUE ( java.lang.String.concat§-1461427456 ) EXECUTE-METHOD
   DUP 0 V!0 §break16961 BRANCH
   
   
   
   
   
   §tempvar SETVTOP
   
   PURGE 1
   
   
   \ new statement
   0 §break16961 LABEL
   
   \ new statement
   
   
   
   §base0 SETVTOP
   PURGE 5
   
   \ new statement
   DROP
;
( *
* Causes current thread to wait until either another thread invokes the
* {@link java.lang.Object#notify()} method or the
* {@link java.lang.Object#notifyAll()} method for this object, or a
* specified amount of time has elapsed.
* <p>
* The current thread must own this object's monitor.
* <p>
* This method causes the current thread (call it <var>T</var>) to
* place itself in the wait set for this object and then to relinquish
* any and all synchronization claims on this object. Thread <var>T</var>
* becomes disabled for thread scheduling purposes and lies dormant
* until one of four things happens:
* <ul>
* <li>Some other thread invokes the <tt>notify</tt> method for this
* object and thread <var>T</var> happens to be arbitrarily chosen as
* the thread to be awakened.
* <li>Some other thread invokes the <tt>notifyAll</tt> method for this
* object.
* <li>Some other thread {@link java.lang.Thread#interrupt() interrupts}
* thread <var>T</var>.
* <li>The specified amount of real time has elapsed, more or less.  If
* <tt>timeout</tt> is zero, however, then real time is not taken into
* consideration and the thread simply waits until notified.
* </ul>
* The thread <var>T</var> is then removed from the wait set for this
* object and re-enabled for thread scheduling. It then competes in the
* usual manner with other threads for the right to synchronize on the
* object; once it has gained control of the object, all its
* synchronization claims on the object are restored to the status quo
* ante - that is, to the situation as of the time that the <tt>wait</tt>
* method was invoked. Thread <var>T</var> then returns from the
* invocation of the <tt>wait</tt> method. Thus, on return from the
* <tt>wait</tt> method, the synchronization state of the object and of
* thread <tt>T</tt> is exactly as it was when the <tt>wait</tt> method
* was invoked.
* <p>
* If the current thread is
* {@link java.lang.Thread#interrupt() interrupted} by another thread
* while it is waiting, then an <tt>InterruptedException</tt> is thrown.
* This exception is not thrown until the lock status of this object has
* been restored as described above.
* <p>
* Note that the <tt>wait</tt> method, as it places the current thread
* into the wait set for this object, unlocks only this object; any
* other objects on which the current thread may be synchronized remain
* locked while the thread waits.
* <p>
* This method should only be called by a thread that is the owner
* of this object's monitor. See the <code>notify</code> method for a
* description of the ways in which a thread can become the owner of
* a monitor.
*
* @param      timeout   the maximum time to wait in milliseconds.
* @exception  IllegalArgumentException      if the value of timeout is
*		     negative.
* @exception  IllegalMonitorStateException  if the current thread is not
*               the owner of the object's monitor.
* @exception  InterruptedException if another thread has interrupted
*             the current thread.  The <i>interrupted status</i> of the
*             current thread is cleared when this exception is thrown.
* @see        java.lang.Object#notify()
* @see        java.lang.Object#notifyAll()
)

:LOCAL lang.Object.wait§-1913293056
   1 VALLOCATE LOCAL §base0
   LOCAL §exception
   2LOCAL timeout
   DUP 0 §base0 + V! LOCAL §this
   
   \ new statement
   
   \ new statement
   BEGIN
      MILLISECONDS 2@
      BEGIN
         2DUP MILLISECONDS 2@ D- D0= INVERT
      UNTIL
      2DROP        timeout 0 1 D- 2DUP TO timeout D0=
   UNTIL
   
   \ new statement
   0 §break16964 LABEL
   
   \ new statement
   
   
   §base0 SETVTOP
   PURGE 4
   
   \ new statement
   DROP
;
( *
* Causes current thread to wait until another thread invokes the
* {@link java.lang.Object#notify()} method or the
* {@link java.lang.Object#notifyAll()} method for this object.
* In other words, this method behaves exactly as if it simply
* performs the call <tt>wait(0)</tt>.
* <p>
* The current thread must own this object's monitor. The thread
* releases ownership of this monitor and waits until another thread
* notifies threads waiting on this object's monitor to wake up
* either through a call to the <code>notify</code> method or the
* <code>notifyAll</code> method. The thread then waits until it can
* re-obtain ownership of the monitor and resumes execution.
* <p>
* This method should only be called by a thread that is the owner
* of this object's monitor. See the <code>notify</code> method for a
* description of the ways in which a thread can become the owner of
* a monitor.
*
* @exception  IllegalMonitorStateException  if the current thread is not
*               the owner of the object's monitor.
* @exception  InterruptedException if another thread has interrupted
*             the current thread.  The <i>interrupted status</i> of the
*             current thread is cleared when this exception is thrown.
* @see        java.lang.Object#notify()
* @see        java.lang.Object#notifyAll()
)

:LOCAL lang.Object.wait§-892045568
   1 VALLOCATE LOCAL §base0
   LOCAL §exception
   DUP 0 §base0 + V! LOCAL §this
   
   \ new statement
   
   \ new statement
   §this A:R@
   0 S>D
   std21810 VAL
   R> -1913293056 TRUE ( java.lang.Object.wait§-1913293056 ) EXECUTE-METHOD
   FALSE DUP
   IF
      std21810 LABEL TRUE
   ENDIF
   IF
      §exception TO §return
      0 §break16969 BRANCH
   ENDIF
   
   \ new statement
   0 §break16969 LABEL
   
   \ new statement
   
   §base0 SETVTOP
   PURGE 3
   
   \ new statement
   DROP
;
( *
* Causes current thread to wait until another thread invokes the
* {@link java.lang.Object#notify()} method or the
* {@link java.lang.Object#notifyAll()} method for this object, or
* some other thread interrupts the current thread, or a certain
* amount of real time has elapsed.
* <p>
* This method is similar to the <code>wait</code> method of one
* argument, but it allows finer control over the amount of time to
* wait for a notification before giving up. The amount of real time,
* measured in nanoseconds, is given by:
* <blockquote>
* <pre>
* 1000000*timeout+nanos</pre></blockquote>
* <p>
* In all other respects, this method does the same thing as the
* method {@link #wait(long)} of one argument. In particular,
* <tt>wait(0, 0)</tt> means the same thing as <tt>wait(0)</tt>.
* <p>
* The current thread must own this object's monitor. The thread
* releases ownership of this monitor and waits until either of the
* following two conditions has occurred:
* <ul>
* <li>Another thread notifies threads waiting on this object's monitor
*     to wake up either through a call to the <code>notify</code> method
*     or the <code>notifyAll</code> method.
* <li>The timeout period, specified by <code>timeout</code>
*     milliseconds plus <code>nanos</code> nanoseconds arguments, has
*     elapsed.
* </ul>
* <p>
* The thread then waits until it can re-obtain ownership of the
* monitor and resumes execution.
* <p>
* This method should only be called by a thread that is the owner
* of this object's monitor. See the <code>notify</code> method for a
* description of the ways in which a thread can become the owner of
* a monitor.
*
* @param      timeout   the maximum time to wait in milliseconds.
* @param      nanos      additional time, in nanoseconds range
*                       0-999999.
* @exception  IllegalArgumentException      if the value of timeout is
*			    negative or the value of nanos is
*			    not in the range 0-999999.
* @exception  IllegalMonitorStateException  if the current thread is not
*               the owner of this object's monitor.
* @exception  InterruptedException if another thread has interrupted
*             the current thread.  The <i>interrupted status</i> of the
*             current thread is cleared when this exception is thrown.
)

:LOCAL lang.Object.wait§2084337408
   1 VALLOCATE LOCAL §base0
   LOCAL §exception
   LOCAL nanos
   2LOCAL timeout
   DUP 0 §base0 + V! LOCAL §this
   
   \ new statement
   
   \ new statement
   timeout
   D0<
   
   \ new statement
   IF
      
      \ new statement
      
      \ new statement
      
      1 VALLOCATE LOCAL §tempvar
      U" timeout value is negative " COUNT lang.JavaArray.createUnicode§-675323136 DUP §tempvar V!
      lang.IllegalArgumentException§-946714368.table 737495296 EXECUTE-NEW
      §exception TO §return
      
      
      §tempvar SETVTOP
      
      PURGE 1
      
      
      \ new statement
      65537 §break16966 BRANCH
      
      \ new statement
      0 §break16966 LABEL
      
      \ new statement
      
      \ new statement
      0>!
      IF
         10001H - §break16965 BRANCH
      ENDIF
      DROP
      
      \ new statement
   ENDIF
   
   \ new statement
   nanos
   0<  0=! or_21825 0BRANCH DROP
   nanos
   999999
   >
   or_21825 LABEL
   
   \ new statement
   IF
      
      \ new statement
      
      \ new statement
      
      1 VALLOCATE LOCAL §tempvar
      U" nanosecond timeout value out of range " COUNT lang.JavaArray.createUnicode§-675323136 DUP §tempvar V!
      lang.IllegalArgumentException§-946714368.table 737495296 EXECUTE-NEW
      §exception TO §return
      
      
      §tempvar SETVTOP
      
      PURGE 1
      
      
      \ new statement
      65537 §break16967 BRANCH
      
      \ new statement
      0 §break16967 LABEL
      
      \ new statement
      
      \ new statement
      0>!
      IF
         10001H - §break16965 BRANCH
      ENDIF
      DROP
      
      \ new statement
   ENDIF
   
   \ new statement
   nanos
   500000
   < INVERT  0=! or_21835 0BRANCH DROP
   nanos
   0<> and_21836 0BRANCH! DROP
   timeout
   D0=
   and_21836 LABEL
   or_21835 LABEL
   
   \ new statement
   IF
      
      \ new statement
      
      \ new statement
      timeout 0 DUP 1+ D+ TO timeout
      
      \ new statement
      0 §break16968 LABEL
      
      \ new statement
      
      \ new statement
      0>!
      IF
         10001H - §break16965 BRANCH
      ENDIF
      DROP
      
      \ new statement
   ENDIF
   
   \ new statement
   §this A:R@
   timeout
   std21844 VAL
   R> -1913293056 TRUE ( java.lang.Object.wait§-1913293056 ) EXECUTE-METHOD
   FALSE DUP
   IF
      std21844 LABEL TRUE
   ENDIF
   IF
      §exception TO §return
      0 §break16965 BRANCH
   ENDIF
   
   \ new statement
   0 §break16965 LABEL
   
   \ new statement
   
   
   
   §base0 SETVTOP
   PURGE 5
   
   \ new statement
   DROP
;
VARIABLE lang.Object._staticBlocking
VARIABLE lang.Object._staticThread

A:HERE VARIABLE lang.Object§-1890496768.table 11 DUP 2* CELLS ALLOT R@ ! A:CELL+
1827294976 R@ ! A:CELL+ lang.Object.Object§1827294976 VAL R@ ! A:CELL+
-360160512 R@ ! A:CELL+ lang.Object.clone§-360160512 VAL R@ ! A:CELL+
-240098048 R@ ! A:CELL+ lang.Object.equals§-240098048 VAL R@ ! A:CELL+
997811712 R@ ! A:CELL+ lang.Object.finalize§997811712 VAL R@ ! A:CELL+
-1604556800 R@ ! A:CELL+ lang.Object.hashCode§-1604556800 VAL R@ ! A:CELL+
2134994432 R@ ! A:CELL+ lang.Object.notify§2134994432 VAL R@ ! A:CELL+
2130144768 R@ ! A:CELL+ lang.Object.notifyAll§2130144768 VAL R@ ! A:CELL+
1621718016 R@ ! A:CELL+ lang.Object.toString§1621718016 VAL R@ ! A:CELL+
-1913293056 R@ ! A:CELL+ lang.Object.wait§-1913293056 VAL R@ ! A:CELL+
-892045568 R@ ! A:CELL+ lang.Object.wait§-892045568 VAL R@ ! A:CELL+
2084337408 R@ ! A:CELL+ lang.Object.wait§2084337408 VAL R@ ! A:CELL+
A:DROP
