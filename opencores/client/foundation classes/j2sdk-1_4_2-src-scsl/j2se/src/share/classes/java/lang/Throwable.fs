MODULENAME java.lang.Throwable
(
* @(#)Throwable.java	1.51 03/01/23
*
* Copyright 2003 Sun Microsystems, Inc. All rights reserved.
* SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
)  ( *
* The <code>Throwable</code> class is the superclass of all errors and
* exceptions in the Java language. Only objects that are instances of this
* class (or one of its subclasses) are thrown by the Java Virtual Machine or
* can be thrown by the Java <code>throw</code> statement. Similarly, only
* this class or one of its subclasses can be the argument type in a
* <code>catch</code> clause.
*
* <p>Instances of two subclasses, {@link java.lang.Error} and
* {@link java.lang.Exception}, are conventionally used to indicate
* that exceptional situations have occurred. Typically, these instances
* are freshly created in the context of the exceptional situation so
* as to include relevant information (such as stack trace data).
*
* <p>A throwable contains a snapshot of the execution stack of its thread at
* the time it was created. It can also contain a message string that gives
* more information about the error. Finally, it can contain a <i>cause</i>:
* another throwable that caused this throwable to get thrown.  The cause
* facility is new in release 1.4.  It is also known as the <i>chained
* exception</i> facility, as the cause can, itself, have a cause, and so on,
* leading to a "chain" of exceptions, each caused by another.
*
* <p>One reason that a throwable may have a cause is that the class that
* throws it is built atop a lower layered abstraction, and an operation on
* the upper layer fails due to a failure in the lower layer.  It would be bad
* design to let the throwable thrown by the lower layer propagate outward, as
* it is generally unrelated to the abstraction provided by the upper layer.
* Further, doing so would tie the API of the upper layer to the details of
* its implementation, assuming the lower layer's exception was a checked
* exception.  Throwing a "wrapped exception" (i.e., an exception containing a
* cause) allows the upper layer to communicate the details of the failure to
* its caller without incurring either of these shortcomings.  It preserves
* the flexibility to change the implementation of the upper layer without
* changing its API (in particular, the set of exceptions thrown by its
* methods).
*
* <p>A second reason that a throwable may have a cause is that the method
* that throws it must conform to a general-purpose interface that does not
* permit the method to throw the cause directly.  For example, suppose
* a persistent collection conforms to the {@link java.util.Collection
* Collection} interface, and that its persistence is implemented atop
* <tt>java.io</tt>.  Suppose the internals of the <tt>put</tt> method
* can throw an {@link java.io.IOException IOException}.  The implementation
* can communicate the details of the <tt>IOException</tt> to its caller
* while conforming to the <tt>Collection</tt> interface by wrapping the
* <tt>IOException</tt> in an appropriate unchecked exception.  (The
* specification for the persistent collection should indicate that it is
* capable of throwing such exceptions.)
*
* <p>A cause can be associated with a throwable in two ways: via a
* constructor that takes the cause as an argument, or via the
* {@link #initCause(Throwable)} method.  New throwable classes that
* wish to allow causes to be associated with them should provide constructors
* that take a cause and delegate (perhaps indirectly) to one of the
* <tt>Throwable</tt> constructors that takes a cause.  For example:
* <pre>
*     try {
*         lowLevelOp();
*     } catch (LowLevelException le) {
*         throw new HighLevelException(le);  // Chaining-aware constructor
*     }
* </pre>
* Because the <tt>initCause</tt> method is public, it allows a cause to be
* associated with any throwable, even a "legacy throwable" whose
* implementation predates the addition of the exception chaining mechanism to
* <tt>Throwable</tt>. For example:
* <pre>
*     try {
*         lowLevelOp();
*     } catch (LowLevelException le) {
*         throw (HighLevelException)
new HighLevelException().initCause(le);  // Legacy constructor
*     }
* </pre>
*
* <p>Prior to release 1.4, there were many throwables that had their own
* non-standard exception chaining mechanisms (
* {@link ExceptionInInitializerError}, {@link ClassNotFoundException},
* {@link java.lang.reflect.UndeclaredThrowableException},
* {@link java.lang.reflect.InvocationTargetException},
* {@link java.io.WriteAbortedException},
* {@link java.security.PrivilegedActionException},
* {@link java.awt.print.PrinterIOException} and
* {@link java.rmi.RemoteException}).
* As of release 1.4, all of these throwables have been retrofitted to
* use the standard exception chaining mechanism, while continuing to
* implement their "legacy" chaining mechanisms for compatibility.
*
* <p>Further, as of release 1.4, many general purpose <tt>Throwable</tt>
* classes (for example {@link Exception}, {@link RuntimeException},
* {@link Error}) have been retrofitted with constructors that take
* a cause.  This was not strictly necessary, due to the existence of the
* <tt>initCause</tt> method, but it is more convenient and expressive to
* delegate to a constructor that takes a cause.
*
* <p>By convention, class <code>Throwable</code> and its subclasses have two
* constructors, one that takes no arguments and one that takes a
* <code>String</code> argument that can be used to produce a detail message.
* Further, those subclasses that might likely have a cause associated with
* them should have two more constructors, one that takes a
* <code>Throwable</code> (the cause), and one that takes a
* <code>String</code> (the detail message) and a <code>Throwable</code> (the
* cause).
*
* <p>Also introduced in release 1.4 is the {@link #getStackTrace()} method,
* which allows programmatic access to the stack trace information that was
* previously available only in text form, via the various forms of the
* {@link #printStackTrace()} method.  This information has been added to the
* <i>serialized representation</i> of this class so <tt>getStackTrace</tt>
* and <tt>printStackTrace</tt> will operate properly on a throwable that
* was obtained by deserialization.
*
* @author  unascribed
* @author  Josh Bloch (Added exception chaining and programmatic access to
*          stack trace in 1.4.)
* @version 1.51, 01/23/03
* @since JDK1.0
)
( *
* Constructs a new throwable with the specified cause and a detail
* message of <tt>(cause==null ? null : cause.toString())</tt> (which
* typically contains the class and detail message of <tt>cause</tt>).
* This constructor is useful for throwables that are little more than
* wrappers for other throwables (for example, {@link
* java.security.PrivilegedActionException}).
*
* <p>The {@link #fillInStackTrace()} method is called to initialize
* the stack trace data in the newly created throwable.
*
* @param  cause the cause (which is saved for later retrieval by the
*         {@link #getCause()} method).  (A <tt>null</tt> value is
*         permitted, and indicates that the cause is nonexistent or
*         unknown.)
* @since  1.4
)

:LOCAL lang.Throwable.Throwable§-226929664
   2 VALLOCATE LOCAL §base0
   DUP 4 §base0 + V! LOCAL cause
   
   \ new statement
   0
   LOCALS §this |
   
   \ new statement
   lang.Object§-1890496768.table 1827294976 EXECUTE-NEW
   
   \ new statement
   44 MALLOC DROP DUP DUP §base0 V! TO §this OVER
   IF
      OVER CELL+ @ 4 + OVER OVER ! DROP
   ENDIF
   CELL+ @ OVER
   OVER ! NIP
   lang.Throwable§1745900544.table OVER 12 + !
   1745879040 OVER 20 + !
   " Throwable " OVER 16 + !
   4 OVER 24 + ! DROP
   
   \ new statement
   ( *
   * Native code saves some indication of the stack backtrace in this slot.
   )
   
   \ new statement
   ( *
   * Specific details about the Throwable.  For example, for
   * <tt>FileNotFoundException</tt>, this contains the name of
   * the file that could not be found.
   *
   * @serial
   )
   
   \ new statement
   ( *
   * The throwable that caused this throwable to get thrown, or null if this
   * throwable was not caused by another throwable, or if the causative
   * throwable is unknown.  If this field is equal to this throwable itself,
   * it indicates that the cause of this throwable has not yet been
   * initialized.
   *
   * @serial
   * @since 1.4
   )
   
   \ new statement
   §this
   DUP 4 §base0 + V! TO cause
   
   \ new statement
   ( *
   * The stack trace, as returned by {@link #getStackTrace()}.
   *
   * @serial
   * @since 1.4
   )
   
   \ new statement
   
   1 VALLOCATE LOCAL §tempvar
   §this A:R@
   R> -875207168 TRUE ( java.lang.Throwable.fillInStackTrace§-875207168 ) EXECUTE-METHOD DUP §tempvar V!
   DROP
   
   §tempvar SETVTOP
   
   PURGE 1
   
   
   \ new statement
   cause
   0=
   IF
      0
      
   ELSE
      cause A:R@
      R> 1621718016 TRUE ( java.lang.Throwable.toString§1621718016 ) EXECUTE-METHOD
      
   ENDIF
   §this CELL+ @ 32 + ( java.lang.Throwable.detailMessage )    OVER 0 V! !
   
   \ new statement
   cause
   §this CELL+ @ 36 + ( java.lang.Throwable.cause )    OVER 0 V! !
   
   \ new statement
   §this DUP 0 V! ( return object )    
   \ new statement
   0 §break16926 LABEL
   
   \ new statement
   
   §base0 SETVTOP
   PURGE 3
   
   \ new statement
   DROP
;
(
* This field is lazily initialized on first use or serialization and
* nulled out when fillInStackTrace is called.
)  ( *
* Constructs a new throwable with <code>null</code> as its detail message.
* The cause is not initialized, and may subsequently be initialized by a
* call to {@link #initCause}.
*
* <p>The {@link #fillInStackTrace()} method is called to initialize
* the stack trace data in the newly created throwable.
)

:LOCAL lang.Throwable.Throwable§-862497792
   1 VALLOCATE LOCAL §base0
   
   \ new statement
   0
   LOCALS §this |
   
   \ new statement
   lang.Object§-1890496768.table 1827294976 EXECUTE-NEW
   
   \ new statement
   44 MALLOC DROP DUP DUP §base0 V! TO §this OVER
   IF
      OVER CELL+ @ 4 + OVER OVER ! DROP
   ENDIF
   CELL+ @ OVER
   OVER ! NIP
   lang.Throwable§1745900544.table OVER 12 + !
   1745879040 OVER 20 + !
   " Throwable " OVER 16 + !
   4 OVER 24 + ! DROP
   
   \ new statement
   ( *
   * Native code saves some indication of the stack backtrace in this slot.
   )
   
   \ new statement
   ( *
   * Specific details about the Throwable.  For example, for
   * <tt>FileNotFoundException</tt>, this contains the name of
   * the file that could not be found.
   *
   * @serial
   )
   
   \ new statement
   ( *
   * The throwable that caused this throwable to get thrown, or null if this
   * throwable was not caused by another throwable, or if the causative
   * throwable is unknown.  If this field is equal to this throwable itself,
   * it indicates that the cause of this throwable has not yet been
   * initialized.
   *
   * @serial
   * @since 1.4
   )
   
   \ new statement
   §this
   §this CELL+ @ 36 + ( java.lang.Throwable.cause )    OVER 0 V! !
   
   \ new statement
   ( *
   * The stack trace, as returned by {@link #getStackTrace()}.
   *
   * @serial
   * @since 1.4
   )
   
   \ new statement
   
   1 VALLOCATE LOCAL §tempvar
   §this A:R@
   R> -875207168 TRUE ( java.lang.Throwable.fillInStackTrace§-875207168 ) EXECUTE-METHOD DUP §tempvar V!
   DROP
   
   §tempvar SETVTOP
   
   PURGE 1
   
   
   \ new statement
   §this DUP 0 V! ( return object )    
   \ new statement
   0 §break16923 LABEL
   
   \ new statement
   §base0 SETVTOP
   PURGE 2
   
   \ new statement
   DROP
;
( *
* Constructs a new throwable with the specified detail message and
* cause.  <p>Note that the detail message associated with
* <code>cause</code> is <i>not</i> automatically incorporated in
* this throwable's detail message.
*
* <p>The {@link #fillInStackTrace()} method is called to initialize
* the stack trace data in the newly created throwable.
*
* @param  message the detail message (which is saved for later retrieval
*         by the {@link #getMessage()} method).
* @param  cause the cause (which is saved for later retrieval by the
*         {@link #getCause()} method).  (A <tt>null</tt> value is
*         permitted, and indicates that the cause is nonexistent or
*         unknown.)
* @since  1.4
)

:LOCAL lang.Throwable.Throwable§1503810560
   3 VALLOCATE LOCAL §base0
   DUP 4 §base0 + V! LOCAL cause
   DUP 8 §base0 + V! LOCAL message
   
   \ new statement
   0
   LOCALS §this |
   
   \ new statement
   lang.Object§-1890496768.table 1827294976 EXECUTE-NEW
   
   \ new statement
   44 MALLOC DROP DUP DUP §base0 V! TO §this OVER
   IF
      OVER CELL+ @ 4 + OVER OVER ! DROP
   ENDIF
   CELL+ @ OVER
   OVER ! NIP
   lang.Throwable§1745900544.table OVER 12 + !
   1745879040 OVER 20 + !
   " Throwable " OVER 16 + !
   4 OVER 24 + ! DROP
   
   \ new statement
   ( *
   * Native code saves some indication of the stack backtrace in this slot.
   )
   
   \ new statement
   ( *
   * Specific details about the Throwable.  For example, for
   * <tt>FileNotFoundException</tt>, this contains the name of
   * the file that could not be found.
   *
   * @serial
   )
   
   \ new statement
   ( *
   * The throwable that caused this throwable to get thrown, or null if this
   * throwable was not caused by another throwable, or if the causative
   * throwable is unknown.  If this field is equal to this throwable itself,
   * it indicates that the cause of this throwable has not yet been
   * initialized.
   *
   * @serial
   * @since 1.4
   )
   
   \ new statement
   §this
   DUP 4 §base0 + V! TO cause
   
   \ new statement
   ( *
   * The stack trace, as returned by {@link #getStackTrace()}.
   *
   * @serial
   * @since 1.4
   )
   
   \ new statement
   
   1 VALLOCATE LOCAL §tempvar
   §this A:R@
   R> -875207168 TRUE ( java.lang.Throwable.fillInStackTrace§-875207168 ) EXECUTE-METHOD DUP §tempvar V!
   DROP
   
   §tempvar SETVTOP
   
   PURGE 1
   
   
   \ new statement
   message
   §this CELL+ @ 32 + ( java.lang.Throwable.detailMessage )    OVER 0 V! !
   
   \ new statement
   cause
   §this CELL+ @ 36 + ( java.lang.Throwable.cause )    OVER 0 V! !
   
   \ new statement
   §this DUP 0 V! ( return object )    
   \ new statement
   0 §break16925 LABEL
   
   \ new statement
   
   
   §base0 SETVTOP
   PURGE 4
   
   \ new statement
   DROP
;
( *
* Constructs a new throwable with the specified detail message.  The
* cause is not initialized, and may subsequently be initialized by
* a call to {@link #initCause}.
*
* <p>The {@link #fillInStackTrace()} method is called to initialize
* the stack trace data in the newly created throwable.
*
* @param   message   the detail message. The detail message is saved for
*          later retrieval by the {@link #getMessage()} method.
)

:LOCAL lang.Throwable.Throwable§861099008
   2 VALLOCATE LOCAL §base0
   DUP 4 §base0 + V! LOCAL message
   
   \ new statement
   0
   LOCALS §this |
   
   \ new statement
   lang.Object§-1890496768.table 1827294976 EXECUTE-NEW
   
   \ new statement
   44 MALLOC DROP DUP DUP §base0 V! TO §this OVER
   IF
      OVER CELL+ @ 4 + OVER OVER ! DROP
   ENDIF
   CELL+ @ OVER
   OVER ! NIP
   lang.Throwable§1745900544.table OVER 12 + !
   1745879040 OVER 20 + !
   " Throwable " OVER 16 + !
   4 OVER 24 + ! DROP
   
   \ new statement
   ( *
   * Native code saves some indication of the stack backtrace in this slot.
   )
   
   \ new statement
   ( *
   * Specific details about the Throwable.  For example, for
   * <tt>FileNotFoundException</tt>, this contains the name of
   * the file that could not be found.
   *
   * @serial
   )
   
   \ new statement
   ( *
   * The throwable that caused this throwable to get thrown, or null if this
   * throwable was not caused by another throwable, or if the causative
   * throwable is unknown.  If this field is equal to this throwable itself,
   * it indicates that the cause of this throwable has not yet been
   * initialized.
   *
   * @serial
   * @since 1.4
   )
   
   \ new statement
   §this
   §this CELL+ @ 36 + ( java.lang.Throwable.cause )    OVER 0 V! !
   
   \ new statement
   ( *
   * The stack trace, as returned by {@link #getStackTrace()}.
   *
   * @serial
   * @since 1.4
   )
   
   \ new statement
   
   1 VALLOCATE LOCAL §tempvar
   §this A:R@
   R> -875207168 TRUE ( java.lang.Throwable.fillInStackTrace§-875207168 ) EXECUTE-METHOD DUP §tempvar V!
   DROP
   
   §tempvar SETVTOP
   
   PURGE 1
   
   
   \ new statement
   message
   §this CELL+ @ 32 + ( java.lang.Throwable.detailMessage )    OVER 0 V! !
   
   \ new statement
   §this DUP 0 V! ( return object )    
   \ new statement
   0 §break16924 LABEL
   
   \ new statement
   
   §base0 SETVTOP
   PURGE 3
   
   \ new statement
   DROP
;


: lang.Throwable.classMonitorEnter§-433167616
   
   \ new statement
   
   \ new statement
   0 §break16921 LABEL
   
   \ new statement
   
   \ new statement
   DROP
;


: lang.Throwable.classMonitorLeave§-1691393280
   
   \ new statement
   
   \ new statement
   0 §break16922 LABEL
   
   \ new statement
   
   \ new statement
   DROP
;
( *
* Fills in the execution stack trace. This method records within this
* <code>Throwable</code> object information about the current state of
* the stack frames for the current thread.
*
* @return  a reference to this <code>Throwable</code> instance.
* @see     java.lang.Throwable#printStackTrace()
)

:LOCAL lang.Throwable.fillInStackTrace§-875207168
   1 VALLOCATE LOCAL §base0
   DUP 0 §base0 + V! LOCAL §this
   
   \ new statement
   
   \ new statement
   §this LOCAL §synchronized0
   DI
   §synchronized0 A:R@
   lang.Throwable._staticThread
   R> 1472163072 TRUE ( Object.monitorEnter ) EXECUTE-METHOD
   EI
   
   \ new statement
   0
   DUP 0 V!0 §break16947 BRANCH
   
   \ new statement
   0 §break16947 LABEL
   
   \ new statement
   §synchronized0 A:R@
   R> 1974299904 TRUE ( Object.monitorLeave ) EXECUTE-METHOD
   PURGE 1
   
   \ new statement
   §base0 SETVTOP
   PURGE 2
   
   \ new statement
   DROP
;
( *
* Returns the cause of this throwable or <code>null</code> if the
* cause is nonexistent or unknown.  (The cause is the throwable that
* caused this throwable to get thrown.)
*
* <p>This implementation returns the cause that was supplied via one of
* the constructors requiring a <tt>Throwable</tt>, or that was set after
* creation with the {@link #initCause(Throwable)} method.  While it is
* typically unnecessary to override this method, a subclass can override
* it to return a cause set by some other means.  This is appropriate for
* a "legacy chained throwable" that predates the addition of chained
* exceptions to <tt>Throwable</tt>.  Note that it is <i>not</i>
* necessary to override any of the <tt>PrintStackTrace</tt> methods,
* all of which invoke the <tt>getCause</tt> method to determine the
* cause of a throwable.
*
* @return  the cause of this throwable or <code>null</code> if the
*          cause is nonexistent or unknown.
* @since 1.4
)

:LOCAL lang.Throwable.getCause§-2052495616
   1 VALLOCATE LOCAL §base0
   DUP 0 §base0 + V! LOCAL §this
   
   \ new statement
   
   \ new statement
   §this CELL+ @ 36 + ( java.lang.Throwable.cause )    @
   §this
   =
   IF
      0
      
   ELSE
      §this CELL+ @ 36 + ( java.lang.Throwable.cause )       @
      
   ENDIF
   DUP 0 V!0 §break16929 BRANCH
   
   \ new statement
   0 §break16929 LABEL
   
   \ new statement
   §base0 SETVTOP
   PURGE 2
   
   \ new statement
   DROP
;
( *
* Creates a localized description of this throwable.
* Subclasses may override this method in order to produce a
* locale-specific message.  For subclasses that do not override this
* method, the default implementation returns the same result as
* <code>getMessage()</code>.
*
* @return  The localized description of this throwable.
* @since   JDK1.1
)

:LOCAL lang.Throwable.getLocalizedMessage§1223845632
   1 VALLOCATE LOCAL §base0
   DUP 0 §base0 + V! LOCAL §this
   
   \ new statement
   
   \ new statement
   §this A:R@
   R> 1695049472 TRUE ( java.lang.Throwable.getMessage§1695049472 ) EXECUTE-METHOD
   DUP 0 V!0 §break16928 BRANCH
   
   \ new statement
   0 §break16928 LABEL
   
   \ new statement
   §base0 SETVTOP
   PURGE 2
   
   \ new statement
   DROP
;
( *
* Returns the detail message string of this throwable.
*
* @return  the detail message string of this <tt>Throwable</tt> instance
*          (which may be <tt>null</tt>).
)

:LOCAL lang.Throwable.getMessage§1695049472
   1 VALLOCATE LOCAL §base0
   DUP 0 §base0 + V! LOCAL §this
   
   \ new statement
   
   \ new statement
   §this CELL+ @ 32 + ( java.lang.Throwable.detailMessage )    @
   DUP 0 V!0 §break16927 BRANCH
   
   \ new statement
   0 §break16927 LABEL
   
   \ new statement
   §base0 SETVTOP
   PURGE 2
   
   \ new statement
   DROP
;


:LOCAL lang.Throwable.getOurStackTrace§1804625664
   1 VALLOCATE LOCAL §base0
   DUP 0 §base0 + V! LOCAL §this
   
   \ new statement
   
   \ new statement
   §this LOCAL §synchronized0
   DI
   §synchronized0 A:R@
   lang.Throwable._staticThread
   R> 1472163072 TRUE ( Object.monitorEnter ) EXECUTE-METHOD
   EI
   
   \ new statement
   \  Initialize stack trace if this is the first call to this method
   
   
   \ new statement
   §this CELL+ @ 40 + ( java.lang.Throwable.stackTrace )    @
   0=
   
   \ new statement
   IF
      
      \ new statement
      0
      LOCALS depth |
      
      \ new statement
      §this A:R@
      R> -1151441152 TRUE ( java.lang.Throwable.getStackTraceDepth§-1151441152 ) EXECUTE-METHOD
      TO depth
      
      \ new statement
      
      1 VALLOCATE LOCAL §tempvar
      depth -1278148607 lang.JavaArray§1352878592.table -227194368 EXECUTE-NEW
      §this CELL+ @ 40 + ( java.lang.Throwable.stackTrace )       OVER 0 V! !
      
      §tempvar SETVTOP
      
      PURGE 1
      
      
      \ new statement
      0
      LOCALS i |
      
      \ new statement
      0
      TO i
      
      \ new statement
      BEGIN
         
         \ new statement
         i
         depth
         <
         
         \ new statement
      WHILE
         
         \ new statement
         
         0
         LOCALS 0§ |
         
         i TO 0§
         §this A:R@
         i
         R> 1675126528 TRUE ( java.lang.Throwable.getStackTraceElement§1675126528 ) EXECUTE-METHOD
         §this CELL+ @ 40 + ( java.lang.Throwable.stackTrace ) @ A:R@
         0§
         R> -942971136 TRUE ( java.lang.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
         OVER 0 V! !
         
         PURGE 1
         
         
         \ new statement
         
         \ new statement
         i 1+ TO i
         
         \ new statement
      REPEAT
      
      \ new statement
      0 §break16951 LABEL
      
      \ new statement
      
      PURGE 1
      
      \ new statement
      DROP
      
      \ new statement
      0 §break16950 LABEL
      
      \ new statement
      
      PURGE 1
      
      \ new statement
      DROP
      
      \ new statement
   ENDIF
   
   \ new statement
   §this CELL+ @ 40 + ( java.lang.Throwable.stackTrace )    @
   DUP 0 V!0 §break16949 BRANCH
   
   \ new statement
   0 §break16949 LABEL
   
   \ new statement
   §synchronized0 A:R@
   R> 1974299904 TRUE ( Object.monitorLeave ) EXECUTE-METHOD
   PURGE 1
   
   \ new statement
   §base0 SETVTOP
   PURGE 2
   
   \ new statement
   DROP
;
( *
* Provides programmatic access to the stack trace information printed by
* {@link #printStackTrace()}.  Returns an array of stack trace elements,
* each representing one stack frame.  The zeroth element of the array
* (assuming the array's length is non-zero) represents the top of the
* stack, which is the last method invocation in the sequence.  Typically,
* this is the point at which this throwable was created and thrown.
* The last element of the array (assuming the array's length is non-zero)
* represents the bottom of the stack, which is the first method invocation
* in the sequence.
*
* <p>Some virtual machines may, under some circumstances, omit one
* or more stack frames from the stack trace.  In the extreme case,
* a virtual machine that has no stack trace information concerning
* this throwable is permitted to return a zero-length array from this
* method.  Generally speaking, the array returned by this method will
* contain one element for every frame that would be printed by
* <tt>printStackTrace</tt>.
*
* @return an array of stack trace elements representing the stack trace
*         pertaining to this throwable.
* @since  1.4
)

:LOCAL lang.Throwable.getStackTrace§-492738816
   1 VALLOCATE LOCAL §base0
   DUP 0 §base0 + V! LOCAL §this
   
   \ new statement
   
   \ new statement
   
   1 VALLOCATE LOCAL §tempvar
   §this A:R@
   R> 1804625664 TRUE ( java.lang.Throwable.getOurStackTrace§1804625664 ) EXECUTE-METHOD DUP §tempvar V!
   A:R@
   R> 1043948288 TRUE ( java.lang.JavaArray.clone§1043948288 ) EXECUTE-METHOD
   DUP 0 V!0 §break16948 BRANCH
   
   
   §tempvar SETVTOP
   
   PURGE 1
   
   
   \ new statement
   0 §break16948 LABEL
   
   \ new statement
   §base0 SETVTOP
   PURGE 2
   
   \ new statement
   DROP
;
( *
* Returns the number of elements in the stack trace (or 0 if the stack
* trace is unavailable).
)

:LOCAL lang.Throwable.getStackTraceDepth§-1151441152
   1 VALLOCATE LOCAL §base0
   DUP 0 §base0 + V! LOCAL §this
   
   \ new statement
   
   \ new statement
   0
   0 §break16954 BRANCH
   
   \ new statement
   0 §break16954 LABEL
   
   \ new statement
   §base0 SETVTOP
   PURGE 2
   
   \ new statement
   DROP
;
( *
* Returns the specified element of the stack trace.
*
* @param index index of the element to return.
* @throws IndexOutOfBoundsException if <tt>index %lt; 0 ||
*         index &gt;= getStackTraceDepth() </tt>
)

:LOCAL lang.Throwable.getStackTraceElement§1675126528
   1 VALLOCATE LOCAL §base0
   LOCAL index
   DUP 0 §base0 + V! LOCAL §this
   
   \ new statement
   
   \ new statement
   0
   DUP 0 V!0 §break16955 BRANCH
   
   \ new statement
   0 §break16955 LABEL
   
   \ new statement
   
   §base0 SETVTOP
   PURGE 3
   
   \ new statement
   DROP
;
( *
* Initializes the <i>cause</i> of this throwable to the specified value.
* (The cause is the throwable that caused this throwable to get thrown.)
*
* <p>This method can be called at most once.  It is generally called from
* within the constructor, or immediately after creating the
* throwable.  If this throwable was created
* with {@link #Throwable(Throwable)} or
* {@link #Throwable(String,Throwable)}, this method cannot be called
* even once.
*
* @param  cause the cause (which is saved for later retrieval by the
*         {@link #getCause()} method).  (A <tt>null</tt> value is
*         permitted, and indicates that the cause is nonexistent or
*         unknown.)
* @return  a reference to this <code>Throwable</code> instance.
* @throws IllegalArgumentException if <code>cause</code> is this
*         throwable.  (A throwable cannot be its own cause.)
* @throws IllegalStateException if this throwable was
*         created with {@link #Throwable(Throwable)} or
*         {@link #Throwable(String,Throwable)}, or this method has already
*         been called on this throwable.
* @since  1.4
)

:LOCAL lang.Throwable.initCause§-621844224
   2 VALLOCATE LOCAL §base0
   DUP 0 §base0 + V! LOCAL cause
   DUP 4 §base0 + V! LOCAL §this
   
   \ new statement
   
   \ new statement
   §this LOCAL §synchronized0
   DI
   §synchronized0 A:R@
   lang.Throwable._staticThread
   R> 1472163072 TRUE ( Object.monitorEnter ) EXECUTE-METHOD
   EI
   
   \ new statement
   §this CELL+ @ 36 + ( java.lang.Throwable.cause )    @
   §this
   <>
   
   \ new statement
   IF
      
      \ new statement
      
      1 VALLOCATE LOCAL §tempvar
      U" Can't overwrite cause " COUNT lang.JavaArray.createUnicode§-675323136 DUP §tempvar V!
      IllegalStateException§799688960.table 1129793792 EXECUTE-NEW
      lang.JavaArray.handler§-1096259584
      
      §tempvar SETVTOP
      
      PURGE 1
      
      
      \ new statement
      
      \ new statement
   ENDIF
   
   \ new statement
   cause
   §this
   =
   
   \ new statement
   IF
      
      \ new statement
      
      1 VALLOCATE LOCAL §tempvar
      U" Self-causation not permitted " COUNT lang.JavaArray.createUnicode§-675323136 DUP §tempvar V!
      lang.IllegalArgumentException§-946714368.table 737495296 EXECUTE-NEW
      lang.JavaArray.handler§-1096259584
      
      §tempvar SETVTOP
      
      PURGE 1
      
      
      \ new statement
      
      \ new statement
   ENDIF
   
   \ new statement
   cause
   §this CELL+ @ 36 + ( java.lang.Throwable.cause )    OVER 0 V! !
   
   \ new statement
   §this
   DUP 0 V!0 §break16930 BRANCH
   
   \ new statement
   0 §break16930 LABEL
   
   \ new statement
   §synchronized0 A:R@
   R> 1974299904 TRUE ( Object.monitorLeave ) EXECUTE-METHOD
   PURGE 1
   
   \ new statement
   
   §base0 SETVTOP
   PURGE 3
   
   \ new statement
   DROP
;
( *
* Prints this throwable and its backtrace to the
* standard error stream. This method prints a stack trace for this
* <code>Throwable</code> object on the error output stream that is
* the value of the field <code>System.err</code>. The first line of
* output contains the result of the {@link #toString()} method for
* this object.  Remaining lines represent data previously recorded by
* the method {@link #fillInStackTrace()}. The format of this
* information depends on the implementation, but the following
* example may be regarded as typical:
* <blockquote><pre>
* java.lang.NullPointerException
*         at MyClass.mash(MyClass.java:9)
*         at MyClass.crunch(MyClass.java:6)
*         at MyClass.main(MyClass.java:3)
* </pre></blockquote>
* This example was produced by running the program:
* <pre>
* class MyClass {
*     public static void main(String[] args) {
*         crunch(null);
*     }
*     static void crunch(int[] a) {
*         mash(a);
*     }
*     static void mash(int[] b) {
*         System.out.println(b[0]);
*     }
* }
* </pre>
* The backtrace for a throwable with an initialized, non-null cause
* should generally include the backtrace for the cause.  The format
* of this information depends on the implementation, but the following
* example may be regarded as typical:
* <pre>
* HighLevelException: MidLevelException: LowLevelException
*         at Junk.a(Junk.java:13)
*         at Junk.main(Junk.java:4)
* Caused by: MidLevelException: LowLevelException
*         at Junk.c(Junk.java:23)
*         at Junk.b(Junk.java:17)
*         at Junk.a(Junk.java:11)
*         ... 1 more
* Caused by: LowLevelException
*         at Junk.e(Junk.java:30)
*         at Junk.d(Junk.java:27)
*         at Junk.c(Junk.java:21)
*         ... 3 more
* </pre>
* Note the presence of lines containing the characters <tt>"..."</tt>.
* These lines indicate that the remainder of the stack trace for this
* exception matches the indicated number of frames from the bottom of the
* stack trace of the exception that was caused by this exception (the
* "enclosing" exception).  This shorthand can greatly reduce the length
* of the output in the common case where a wrapped exception is thrown
* from same method as the "causative exception" is caught.  The above
* example was produced by running the program:
* <pre>
* public class Junk {
*     public static void main(String args[]) {
*         try {
*             a();
*         } catch(HighLevelException e) {
*             e.printStackTrace();
*         }
*     }
*     static void a() throws HighLevelException {
*         try {
*             b();
*         } catch(MidLevelException e) {
*             throw new HighLevelException(e);
*         }
*     }
*     static void b() throws MidLevelException {
*         c();
*     }
*     static void c() throws MidLevelException {
*         try {
*             d();
*         } catch(LowLevelException e) {
*             throw new MidLevelException(e);
*         }
*     }
*     static void d() throws LowLevelException {
*        e();
*     }
*     static void e() throws LowLevelException {
*         throw new LowLevelException();
*     }
* }
*
* class HighLevelException extends Exception {
*     HighLevelException(Throwable cause) { super(cause); }
* }
*
* class MidLevelException extends Exception {
*     MidLevelException(Throwable cause)  { super(cause); }
* }
*
* class LowLevelException extends Exception {
* }
* </pre>
)

:LOCAL lang.Throwable.printStackTrace§263417856
   1 VALLOCATE LOCAL §base0
   DUP 0 §base0 + V! LOCAL §this
   
   \ new statement
   
   \ new statement
   §this A:R@
   lang.System.err
   @
   R> 634155008 TRUE ( java.lang.Throwable.printStackTrace§634155008 ) EXECUTE-METHOD
   
   \ new statement
   0 §break16932 LABEL
   
   \ new statement
   §base0 SETVTOP
   PURGE 2
   
   \ new statement
   DROP
;
( *
* Prints this throwable and its backtrace to the specified
* print writer.
*
* @param s <code>PrintWriter</code> to use for output
* @since   JDK1.1
)

:LOCAL lang.Throwable.printStackTrace§574910464
   2 VALLOCATE LOCAL §base0
   DUP 0 §base0 + V! LOCAL s
   DUP 4 §base0 + V! LOCAL §this
   
   \ new statement
   
   \ new statement
   2 VALLOCATE LOCAL §base1
   0 DUP
   LOCALS ourCause trace |
   
   \ new statement
   s
   LOCAL §synchronized0
   DI
   §synchronized0 A:R@
   io.PrintWriter._staticThread  @
   R> 1472163072 TRUE ( Object.monitorEnter ) EXECUTE-METHOD
   EI
   
   \ new statement
   s A:R@
   §this
   R> -1712558080 TRUE ( java.io.PrintWriter.println§-1712558080 ) EXECUTE-METHOD
   
   \ new statement
   §this A:R@
   R> 1804625664 TRUE ( java.lang.Throwable.getOurStackTrace§1804625664 ) EXECUTE-METHOD
   DUP §base1 V! TO trace
   
   \ new statement
   0
   LOCALS i |
   
   \ new statement
   0
   TO i
   
   \ new statement
   BEGIN
      
      \ new statement
      i
      trace CELL+ @ 28 + ( java.lang.JavaArray.length )       @
      <
      
      \ new statement
   WHILE
      
      \ new statement
      
      2 VALLOCATE LOCAL §tempvar
      s A:R@
      U" \tat  " COUNT lang.JavaArray.createUnicode§-675323136 DUP §tempvar V!
      trace A:R@
      i
      R> -942971136 TRUE ( java.lang.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
      @  " StackTraceElement " CASTTO
      OVER  -1461427456 TRUE ( java.lang.String.concat§-1461427456 ) EXECUTE-METHOD
      R> 575238144 TRUE ( java.io.PrintWriter.println§575238144 ) EXECUTE-METHOD
      
      
      
      §tempvar SETVTOP
      
      PURGE 1
      
      
      \ new statement
      
      \ new statement
      i 1+ TO i
      
      \ new statement
   REPEAT
   
   \ new statement
   0 §break16942 LABEL
   
   \ new statement
   
   PURGE 1
   
   \ new statement
   DROP
   
   \ new statement
   §this A:R@
   R> -2052495616 TRUE ( java.lang.Throwable.getCause§-2052495616 ) EXECUTE-METHOD
   DUP 4 §base1 + V! TO ourCause
   
   \ new statement
   ourCause
   0<>
   
   \ new statement
   IF
      
      \ new statement
      ourCause A:R@
      s
      trace
      R> -396726272 TRUE ( java.lang.Throwable.printStackTraceAsCause§-396726272 ) EXECUTE-METHOD
      
      \ new statement
   ENDIF
   
   \ new statement
   0 §break16941 LABEL
   
   \ new statement
   §synchronized0 A:R@
   R> 1974299904 TRUE ( Object.monitorLeave ) EXECUTE-METHOD
   PURGE 1
   
   \ new statement
   
   
   §base1 SETVTOP
   PURGE 3
   
   \ new statement
   DROP
   
   \ new statement
   0 §break16940 LABEL
   
   \ new statement
   
   §base0 SETVTOP
   PURGE 3
   
   \ new statement
   DROP
;
( *
* Prints this throwable and its backtrace to the specified print stream.
*
* @param s <code>PrintStream</code> to use for output
)

:LOCAL lang.Throwable.printStackTrace§634155008
   2 VALLOCATE LOCAL §base0
   DUP 0 §base0 + V! LOCAL s
   DUP 4 §base0 + V! LOCAL §this
   
   \ new statement
   
   \ new statement
   2 VALLOCATE LOCAL §base1
   0 DUP
   LOCALS ourCause trace |
   
   \ new statement
   s
   LOCAL §synchronized0
   DI
   §synchronized0 A:R@
   io.PrintStream._staticThread  @
   R> 1472163072 TRUE ( Object.monitorEnter ) EXECUTE-METHOD
   EI
   
   \ new statement
   s A:R@
   §this
   R> -1712558080 TRUE ( java.io.PrintStream.println§-1712558080 ) EXECUTE-METHOD
   
   \ new statement
   §this A:R@
   R> 1804625664 TRUE ( java.lang.Throwable.getOurStackTrace§1804625664 ) EXECUTE-METHOD
   DUP §base1 V! TO trace
   
   \ new statement
   0
   LOCALS i |
   
   \ new statement
   0
   TO i
   
   \ new statement
   BEGIN
      
      \ new statement
      i
      trace CELL+ @ 28 + ( java.lang.JavaArray.length )       @
      <
      
      \ new statement
   WHILE
      
      \ new statement
      
      2 VALLOCATE LOCAL §tempvar
      s A:R@
      U" \tat  " COUNT lang.JavaArray.createUnicode§-675323136 DUP §tempvar V!
      trace A:R@
      i
      R> -942971136 TRUE ( java.lang.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
      @  " StackTraceElement " CASTTO
      OVER  -1461427456 TRUE ( java.lang.String.concat§-1461427456 ) EXECUTE-METHOD
      R> 575238144 TRUE ( java.io.PrintStream.println§575238144 ) EXECUTE-METHOD
      
      
      
      §tempvar SETVTOP
      
      PURGE 1
      
      
      \ new statement
      
      \ new statement
      i 1+ TO i
      
      \ new statement
   REPEAT
   
   \ new statement
   0 §break16935 LABEL
   
   \ new statement
   
   PURGE 1
   
   \ new statement
   DROP
   
   \ new statement
   §this A:R@
   R> -2052495616 TRUE ( java.lang.Throwable.getCause§-2052495616 ) EXECUTE-METHOD
   DUP 4 §base1 + V! TO ourCause
   
   \ new statement
   ourCause
   0<>
   
   \ new statement
   IF
      
      \ new statement
      ourCause A:R@
      s
      trace
      R> -1871286272 TRUE ( java.lang.Throwable.printStackTraceAsCause§-1871286272 ) EXECUTE-METHOD
      
      \ new statement
   ENDIF
   
   \ new statement
   0 §break16934 LABEL
   
   \ new statement
   §synchronized0 A:R@
   R> 1974299904 TRUE ( Object.monitorLeave ) EXECUTE-METHOD
   PURGE 1
   
   \ new statement
   
   
   §base1 SETVTOP
   PURGE 3
   
   \ new statement
   DROP
   
   \ new statement
   0 §break16933 LABEL
   
   \ new statement
   
   §base0 SETVTOP
   PURGE 3
   
   \ new statement
   DROP
;
( *
* Print our stack trace as a cause for the specified stack trace.
)

:LOCAL lang.Throwable.printStackTraceAsCause§-1871286272
   5 VALLOCATE LOCAL §base0
   DUP 0 §base0 + V! LOCAL causedTrace
   DUP 4 §base0 + V! LOCAL s
   DUP 8 §base0 + V! LOCAL §this
   
   \ new statement
   0 DUP DUP DUP DUP
   LOCALS framesInCommon m n ourCause trace |
   
   \ new statement
   \  assert Thread.holdsLock(s);
   \  Compute number of frames in common between this and caused
   
   
   \ new statement
   §this A:R@
   R> 1804625664 TRUE ( java.lang.Throwable.getOurStackTrace§1804625664 ) EXECUTE-METHOD
   DUP 12 §base0 + V! TO trace
   
   \ new statement
   trace CELL+ @ 28 + ( java.lang.JavaArray.length )    @
   1
   -
   TO m
   
   \ new statement
   causedTrace CELL+ @ 28 + ( java.lang.JavaArray.length )    @
   1
   -
   TO n
   
   \ new statement
   
   \ new statement
   BEGIN
      
      \ new statement
      m
      0< INVERT and_22258 0BRANCH! DROP
      n
      0< INVERT
      and_22258 0BRANCH! DROP
      trace A:R@
      m
      R> -942971136 TRUE ( java.lang.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
      @  " StackTraceElement " CASTTO A:R@
      causedTrace A:R@
      n
      R> -942971136 TRUE ( java.lang.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
      @  " StackTraceElement " CASTTO
      R> -240098048 TRUE ( java.lang.StackTraceElement.equals§-240098048 ) EXECUTE-METHOD
      and_22258 LABEL
      
      \ new statement
   WHILE
      
      \ new statement
      m 1- TO m
      
      \ new statement
      n 1- TO n
      
      \ new statement
      
      \ new statement
   REPEAT
   
   \ new statement
   0 §break16937 LABEL
   
   \ new statement
   
   \ new statement
   DROP
   
   \ new statement
   trace CELL+ @ 28 + ( java.lang.JavaArray.length )    @
   1
   -
   m
   -
   TO framesInCommon
   
   \ new statement
   
   2 VALLOCATE LOCAL §tempvar
   s A:R@
   U" Caused by:  " COUNT lang.JavaArray.createUnicode§-675323136 DUP §tempvar V!
   §this
   OVER  -1461427456 TRUE ( java.lang.String.concat§-1461427456 ) EXECUTE-METHOD
   R> 575238144 TRUE ( java.io.PrintStream.println§575238144 ) EXECUTE-METHOD
   
   
   
   §tempvar SETVTOP
   
   PURGE 1
   
   
   \ new statement
   0
   LOCALS i |
   
   \ new statement
   0
   TO i
   
   \ new statement
   BEGIN
      
      \ new statement
      i
      m
      > INVERT
      
      \ new statement
   WHILE
      
      \ new statement
      
      2 VALLOCATE LOCAL §tempvar
      s A:R@
      U" \tat  " COUNT lang.JavaArray.createUnicode§-675323136 DUP §tempvar V!
      trace A:R@
      i
      R> -942971136 TRUE ( java.lang.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
      @  " StackTraceElement " CASTTO
      OVER  -1461427456 TRUE ( java.lang.String.concat§-1461427456 ) EXECUTE-METHOD
      R> 575238144 TRUE ( java.io.PrintStream.println§575238144 ) EXECUTE-METHOD
      
      
      
      §tempvar SETVTOP
      
      PURGE 1
      
      
      \ new statement
      
      \ new statement
      i 1+ TO i
      
      \ new statement
   REPEAT
   
   \ new statement
   0 §break16939 LABEL
   
   \ new statement
   
   PURGE 1
   
   \ new statement
   DROP
   
   \ new statement
   framesInCommon
   0<>
   
   \ new statement
   IF
      
      \ new statement
      
      4 VALLOCATE LOCAL §tempvar
      s A:R@
      U" \t...  " COUNT lang.JavaArray.createUnicode§-675323136 DUP §tempvar V!
      framesInCommon  <# 0 ..R lang.JavaArray.createString§-105880832 DUP §tempvar 4 + V!
      OVER  -1461427456 TRUE ( java.lang.String.concat§-1461427456 ) EXECUTE-METHOD DUP §tempvar 8 + V!
      U"  more " COUNT lang.JavaArray.createUnicode§-675323136 DUP §tempvar 12 + V!
      OVER  -1461427456 TRUE ( java.lang.String.concat§-1461427456 ) EXECUTE-METHOD
      R> 575238144 TRUE ( java.io.PrintStream.println§575238144 ) EXECUTE-METHOD
      
      
      
      
      
      §tempvar SETVTOP
      
      PURGE 1
      
      
      \ new statement
   ENDIF
   
   \ new statement
   \  Recurse if we have a cause
   
   
   \ new statement
   §this A:R@
   R> -2052495616 TRUE ( java.lang.Throwable.getCause§-2052495616 ) EXECUTE-METHOD
   DUP 16 §base0 + V! TO ourCause
   
   \ new statement
   ourCause
   0<>
   
   \ new statement
   IF
      
      \ new statement
      ourCause A:R@
      s
      trace
      R> -1871286272 TRUE ( java.lang.Throwable.printStackTraceAsCause§-1871286272 ) EXECUTE-METHOD
      
      \ new statement
   ENDIF
   
   \ new statement
   0 §break16936 LABEL
   
   \ new statement
   
   
   
   
   
   
   
   §base0 SETVTOP
   PURGE 9
   
   \ new statement
   DROP
;
( *
* Print our stack trace as a cause for the specified stack trace.
)

:LOCAL lang.Throwable.printStackTraceAsCause§-396726272
   5 VALLOCATE LOCAL §base0
   DUP 0 §base0 + V! LOCAL causedTrace
   DUP 4 §base0 + V! LOCAL s
   DUP 8 §base0 + V! LOCAL §this
   
   \ new statement
   0 DUP DUP DUP DUP
   LOCALS framesInCommon m n ourCause trace |
   
   \ new statement
   \  assert Thread.holdsLock(s);
   \  Compute number of frames in common between this and caused
   
   
   \ new statement
   §this A:R@
   R> 1804625664 TRUE ( java.lang.Throwable.getOurStackTrace§1804625664 ) EXECUTE-METHOD
   DUP 12 §base0 + V! TO trace
   
   \ new statement
   trace CELL+ @ 28 + ( java.lang.JavaArray.length )    @
   1
   -
   TO m
   
   \ new statement
   causedTrace CELL+ @ 28 + ( java.lang.JavaArray.length )    @
   1
   -
   TO n
   
   \ new statement
   
   \ new statement
   BEGIN
      
      \ new statement
      m
      0< INVERT and_22302 0BRANCH! DROP
      n
      0< INVERT
      and_22302 0BRANCH! DROP
      trace A:R@
      m
      R> -942971136 TRUE ( java.lang.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
      @  " StackTraceElement " CASTTO A:R@
      causedTrace A:R@
      n
      R> -942971136 TRUE ( java.lang.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
      @  " StackTraceElement " CASTTO
      R> -240098048 TRUE ( java.lang.StackTraceElement.equals§-240098048 ) EXECUTE-METHOD
      and_22302 LABEL
      
      \ new statement
   WHILE
      
      \ new statement
      m 1- TO m
      
      \ new statement
      n 1- TO n
      
      \ new statement
      
      \ new statement
   REPEAT
   
   \ new statement
   0 §break16944 LABEL
   
   \ new statement
   
   \ new statement
   DROP
   
   \ new statement
   trace CELL+ @ 28 + ( java.lang.JavaArray.length )    @
   1
   -
   m
   -
   TO framesInCommon
   
   \ new statement
   
   2 VALLOCATE LOCAL §tempvar
   s A:R@
   U" Caused by:  " COUNT lang.JavaArray.createUnicode§-675323136 DUP §tempvar V!
   §this
   OVER  -1461427456 TRUE ( java.lang.String.concat§-1461427456 ) EXECUTE-METHOD
   R> 575238144 TRUE ( java.io.PrintWriter.println§575238144 ) EXECUTE-METHOD
   
   
   
   §tempvar SETVTOP
   
   PURGE 1
   
   
   \ new statement
   0
   LOCALS i |
   
   \ new statement
   0
   TO i
   
   \ new statement
   BEGIN
      
      \ new statement
      i
      m
      > INVERT
      
      \ new statement
   WHILE
      
      \ new statement
      
      2 VALLOCATE LOCAL §tempvar
      s A:R@
      U" \tat  " COUNT lang.JavaArray.createUnicode§-675323136 DUP §tempvar V!
      trace A:R@
      i
      R> -942971136 TRUE ( java.lang.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
      @  " StackTraceElement " CASTTO
      OVER  -1461427456 TRUE ( java.lang.String.concat§-1461427456 ) EXECUTE-METHOD
      R> 575238144 TRUE ( java.io.PrintWriter.println§575238144 ) EXECUTE-METHOD
      
      
      
      §tempvar SETVTOP
      
      PURGE 1
      
      
      \ new statement
      
      \ new statement
      i 1+ TO i
      
      \ new statement
   REPEAT
   
   \ new statement
   0 §break16946 LABEL
   
   \ new statement
   
   PURGE 1
   
   \ new statement
   DROP
   
   \ new statement
   framesInCommon
   0<>
   
   \ new statement
   IF
      
      \ new statement
      
      4 VALLOCATE LOCAL §tempvar
      s A:R@
      U" \t...  " COUNT lang.JavaArray.createUnicode§-675323136 DUP §tempvar V!
      framesInCommon  <# 0 ..R lang.JavaArray.createString§-105880832 DUP §tempvar 4 + V!
      OVER  -1461427456 TRUE ( java.lang.String.concat§-1461427456 ) EXECUTE-METHOD DUP §tempvar 8 + V!
      U"  more " COUNT lang.JavaArray.createUnicode§-675323136 DUP §tempvar 12 + V!
      OVER  -1461427456 TRUE ( java.lang.String.concat§-1461427456 ) EXECUTE-METHOD
      R> 575238144 TRUE ( java.io.PrintWriter.println§575238144 ) EXECUTE-METHOD
      
      
      
      
      
      §tempvar SETVTOP
      
      PURGE 1
      
      
      \ new statement
   ENDIF
   
   \ new statement
   \  Recurse if we have a cause
   
   
   \ new statement
   §this A:R@
   R> -2052495616 TRUE ( java.lang.Throwable.getCause§-2052495616 ) EXECUTE-METHOD
   DUP 16 §base0 + V! TO ourCause
   
   \ new statement
   ourCause
   0<>
   
   \ new statement
   IF
      
      \ new statement
      ourCause A:R@
      s
      trace
      R> -396726272 TRUE ( java.lang.Throwable.printStackTraceAsCause§-396726272 ) EXECUTE-METHOD
      
      \ new statement
   ENDIF
   
   \ new statement
   0 §break16943 LABEL
   
   \ new statement
   
   
   
   
   
   
   
   §base0 SETVTOP
   PURGE 9
   
   \ new statement
   DROP
;
( *
* Sets the stack trace elements that will be returned by
* {@link #getStackTrace()} and printed by {@link #printStackTrace()}
* and related methods.
*
* This method, which is designed for use by RPC frameworks and other
* advanced systems, allows the client to override the default
* stack trace that is either generated by {@link #fillInStackTrace()}
* when a throwable is constructed or deserialized when a throwable is
* read from a serialization stream.
*
* @param   stackTrace the stack trace elements to be associated with
* this <code>Throwable</code>.  The specified array is copied by this
* call; changes in the specified array after the method invocation
* returns will have no affect on this <code>Throwable</code>'s stack
* trace.
*
* @throws NullPointerException if <code>stackTrace</code> is
*         <code>null</code>, or if any of the elements of
*         <code>stackTrace</code> are <code>null</code>
*
* @since  1.4
)

:LOCAL lang.Throwable.setStackTrace§716927744
   3 VALLOCATE LOCAL §base0
   DUP 0 §base0 + V! LOCAL stackTrace
   DUP 4 §base0 + V! LOCAL §this
   
   \ new statement
   0
   LOCALS defensiveCopy |
   
   \ new statement
   stackTrace A:R@
   R> 1043948288 TRUE ( java.lang.JavaArray.clone§1043948288 ) EXECUTE-METHOD
   DUP 8 §base0 + V! TO defensiveCopy
   
   \ new statement
   0
   LOCALS i |
   
   \ new statement
   0
   TO i
   
   \ new statement
   BEGIN
      
      \ new statement
      i
      defensiveCopy CELL+ @ 28 + ( java.lang.JavaArray.length )       @
      <
      
      \ new statement
   WHILE
      
      \ new statement
      defensiveCopy A:R@
      i
      R> -942971136 TRUE ( java.lang.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
      @  " StackTraceElement " CASTTO
      0=
      
      \ new statement
      IF
         
         \ new statement
         
         4 VALLOCATE LOCAL §tempvar
         U" stackTrace[ " COUNT lang.JavaArray.createUnicode§-675323136 DUP §tempvar V!
         i  <# 0 ..R lang.JavaArray.createString§-105880832 DUP §tempvar 4 + V!
         OVER  -1461427456 TRUE ( java.lang.String.concat§-1461427456 ) EXECUTE-METHOD DUP §tempvar 8 + V!
         U" ] " COUNT lang.JavaArray.createUnicode§-675323136 DUP §tempvar 12 + V!
         OVER  -1461427456 TRUE ( java.lang.String.concat§-1461427456 ) EXECUTE-METHOD
         lang.NullPointerException§561335808.table -938062336 EXECUTE-NEW
         lang.JavaArray.handler§-1096259584
         
         
         
         
         §tempvar SETVTOP
         
         PURGE 1
         
         
         \ new statement
         
         \ new statement
      ENDIF
      
      \ new statement
      
      \ new statement
      i 1+ TO i
      
      \ new statement
   REPEAT
   
   \ new statement
   0 §break16953 LABEL
   
   \ new statement
   
   PURGE 1
   
   \ new statement
   0>!
   IF
      10001H - §break16952 BRANCH
   ENDIF
   DROP
   
   \ new statement
   defensiveCopy
   §this CELL+ @ 40 + ( java.lang.Throwable.stackTrace )    OVER 0 V! !
   
   \ new statement
   0 §break16952 LABEL
   
   \ new statement
   
   
   §base0 SETVTOP
   PURGE 4
   
   \ new statement
   DROP
;
( *
* Returns a short description of this throwable.
* If this <code>Throwable</code> object was created with a non-null detail
* message string, then the result is the concatenation of three strings:
* <ul>
* <li>The name of the actual class of this object
* <li>": " (a colon and a space)
* <li>The result of the {@link #getMessage} method for this object
* </ul>
* If this <code>Throwable</code> object was created with a <tt>null</tt>
* detail message string, then the name of the actual class of this object
* is returned.
*
* @return a string representation of this throwable.
)

:LOCAL lang.Throwable.toString§1621718016
   3 VALLOCATE LOCAL §base0
   DUP 0 §base0 + V! LOCAL §this
   
   \ new statement
   0 DUP
   LOCALS message s |
   
   \ new statement
   §this CELL+ @ A:R@
   R> 1621718016 FALSE ( java.lang.Object.toString§1621718016 ) EXECUTE-METHOD
   DUP 4 §base0 + V! TO s
   
   \ new statement
   §this A:R@
   R> 1223845632 TRUE ( java.lang.Throwable.getLocalizedMessage§1223845632 ) EXECUTE-METHOD
   DUP 8 §base0 + V! TO message
   
   \ new statement
   
   2 VALLOCATE LOCAL §tempvar
   message
   0<>
   
   IF
      s
      U" :  " COUNT lang.JavaArray.createUnicode§-675323136 DUP §tempvar V!
      OVER  -1461427456 TRUE ( java.lang.String.concat§-1461427456 ) EXECUTE-METHOD DUP §tempvar 4 + V!
      message
      OVER  -1461427456 TRUE ( java.lang.String.concat§-1461427456 ) EXECUTE-METHOD
      
   ELSE
      s
      
   ENDIF
   DUP 0 V!0 §break16931 BRANCH
   
   
   
   §tempvar SETVTOP
   
   PURGE 1
   
   
   \ new statement
   0 §break16931 LABEL
   
   \ new statement
   
   
   §base0 SETVTOP
   PURGE 4
   
   \ new statement
   DROP
;
VARIABLE lang.Throwable._staticBlocking
VARIABLE lang.Throwable._staticThread
2VARIABLE lang.Throwable.serialVersionUID

A:HERE VARIABLE lang.Throwable§1745900544.table 20 DUP 2* CELLS ALLOT R@ ! A:CELL+
-226929664 R@ ! A:CELL+ lang.Throwable.Throwable§-226929664 VAL R@ ! A:CELL+
-862497792 R@ ! A:CELL+ lang.Throwable.Throwable§-862497792 VAL R@ ! A:CELL+
1503810560 R@ ! A:CELL+ lang.Throwable.Throwable§1503810560 VAL R@ ! A:CELL+
861099008 R@ ! A:CELL+ lang.Throwable.Throwable§861099008 VAL R@ ! A:CELL+
-875207168 R@ ! A:CELL+ lang.Throwable.fillInStackTrace§-875207168 VAL R@ ! A:CELL+
-2052495616 R@ ! A:CELL+ lang.Throwable.getCause§-2052495616 VAL R@ ! A:CELL+
1223845632 R@ ! A:CELL+ lang.Throwable.getLocalizedMessage§1223845632 VAL R@ ! A:CELL+
1695049472 R@ ! A:CELL+ lang.Throwable.getMessage§1695049472 VAL R@ ! A:CELL+
1804625664 R@ ! A:CELL+ lang.Throwable.getOurStackTrace§1804625664 VAL R@ ! A:CELL+
-492738816 R@ ! A:CELL+ lang.Throwable.getStackTrace§-492738816 VAL R@ ! A:CELL+
-1151441152 R@ ! A:CELL+ lang.Throwable.getStackTraceDepth§-1151441152 VAL R@ ! A:CELL+
1675126528 R@ ! A:CELL+ lang.Throwable.getStackTraceElement§1675126528 VAL R@ ! A:CELL+
-621844224 R@ ! A:CELL+ lang.Throwable.initCause§-621844224 VAL R@ ! A:CELL+
263417856 R@ ! A:CELL+ lang.Throwable.printStackTrace§263417856 VAL R@ ! A:CELL+
574910464 R@ ! A:CELL+ lang.Throwable.printStackTrace§574910464 VAL R@ ! A:CELL+
634155008 R@ ! A:CELL+ lang.Throwable.printStackTrace§634155008 VAL R@ ! A:CELL+
-1871286272 R@ ! A:CELL+ lang.Throwable.printStackTraceAsCause§-1871286272 VAL R@ ! A:CELL+
-396726272 R@ ! A:CELL+ lang.Throwable.printStackTraceAsCause§-396726272 VAL R@ ! A:CELL+
716927744 R@ ! A:CELL+ lang.Throwable.setStackTrace§716927744 VAL R@ ! A:CELL+
1621718016 R@ ! A:CELL+ lang.Throwable.toString§1621718016 VAL R@ ! A:CELL+
A:DROP
