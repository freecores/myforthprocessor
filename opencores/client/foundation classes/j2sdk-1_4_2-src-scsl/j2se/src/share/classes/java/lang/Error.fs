MODULENAME java.lang.Error
(
* @(#)Error.java	1.15 03/01/23
*
* Copyright 2003 Sun Microsystems, Inc. All rights reserved.
* SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
)  ( *
* An <code>Error</code> is a subclass of <code>Throwable</code>
* that indicates serious problems that a reasonable application
* should not try to catch. Most such errors are abnormal conditions.
* The <code>ThreadDeath</code> error, though a "normal" condition,
* is also a subclass of <code>Error</code> because most applications
* should not try to catch it.
* <p>
* A method is not required to declare in its <code>throws</code>
* clause any subclasses of <code>Error</code> that might be thrown
* during the execution of the method but not caught, since these
* errors are abnormal conditions that should never occur.
*
* @author  Frank Yellin
* @version 1.15, 01/23/03
* @see     java.lang.ThreadDeath
* @since   JDK1.0
)
( *
* Constructs a new error with the specified cause and a detail
* message of <tt>(cause==null ? null : cause.toString())</tt> (which
* typically contains the class and detail message of <tt>cause</tt>).
* This constructor is useful for errors that are little more than
* wrappers for other throwables.
*
* @param  cause the cause (which is saved for later retrieval by the
*         {@link #getCause()} method).  (A <tt>null</tt> value is
*         permitted, and indicates that the cause is nonexistent or
*         unknown.)
* @since  1.4
)

:LOCAL lang.Error.Error§-2086058752
   LOCAL cause
   
   \ new statement
   0
   LOCALS §this |
   
   \ new statement
   cause DUP INCREFERENCE
   lang.Throwable§1745900544.table -226929664 EXECUTE-NEW
   
   \ new statement
   28 MALLOC DROP DUP TO §this OVER
   IF
      OVER CELL+ @ 4 + OVER OVER ! DROP
   ENDIF
   CELL+ @ OVER
   OVER ! NIP
   lang.Error§-32422656.table OVER 12 + !
   -32440320 OVER 20 + !
   " Error " OVER 16 + !
   0 OVER 24 + ! DROP
   
   \ new statement
   §this DUP ( return object )    
   \ new statement
   0 §break5975 LABEL
   
   \ new statement
   cause TRUE JavaArray.kill§1620077312
   PURGE 2
   
   \ new statement
   DROP
;
( *
* Constructs a new error with the specified detail message and
* cause.  <p>Note that the detail message associated with
* <code>cause</code> is <i>not</i> automatically incorporated in
* this error's detail message.
*
* @param  message the detail message (which is saved for later retrieval
*         by the {@link #getMessage()} method).
* @param  cause the cause (which is saved for later retrieval by the
*         {@link #getCause()} method).  (A <tt>null</tt> value is
*         permitted, and indicates that the cause is nonexistent or
*         unknown.)
* @since  1.4
)

:LOCAL lang.Error.Error§-393329408
   LOCAL cause
   LOCAL message
   
   \ new statement
   0
   LOCALS §this |
   
   \ new statement
   message DUP INCREFERENCE
   cause DUP INCREFERENCE
   lang.Throwable§1745900544.table 1503810560 EXECUTE-NEW
   
   \ new statement
   28 MALLOC DROP DUP TO §this OVER
   IF
      OVER CELL+ @ 4 + OVER OVER ! DROP
   ENDIF
   CELL+ @ OVER
   OVER ! NIP
   lang.Error§-32422656.table OVER 12 + !
   -32440320 OVER 20 + !
   " Error " OVER 16 + !
   0 OVER 24 + ! DROP
   
   \ new statement
   §this DUP ( return object )    
   \ new statement
   0 §break5974 LABEL
   
   \ new statement
   cause TRUE JavaArray.kill§1620077312
   message TRUE JavaArray.kill§1620077312
   PURGE 3
   
   \ new statement
   DROP
;
( *
* Constructs a new error with <code>null</code> as its detail message.
* The cause is not initialized, and may subsequently be initialized by a
* call to {@link #initCause}.
)

:LOCAL lang.Error.Error§-503364352
   
   \ new statement
   0
   LOCALS §this |
   
   \ new statement
   lang.Throwable§1745900544.table -862497792 EXECUTE-NEW
   
   \ new statement
   28 MALLOC DROP DUP TO §this OVER
   IF
      OVER CELL+ @ 4 + OVER OVER ! DROP
   ENDIF
   CELL+ @ OVER
   OVER ! NIP
   lang.Error§-32422656.table OVER 12 + !
   -32440320 OVER 20 + !
   " Error " OVER 16 + !
   0 OVER 24 + ! DROP
   
   \ new statement
   §this DUP ( return object )    
   \ new statement
   0 §break5972 LABEL
   
   \ new statement
   PURGE 1
   
   \ new statement
   DROP
;
( *
* Constructs a new error with the specified detail message.  The
* cause is not initialized, and may subsequently be initialized by
* a call to {@link #initCause}.
*
* @param   message   the detail message. The detail message is saved for
*          later retrieval by the {@link #getMessage()} method.
)

:LOCAL lang.Error.Error§-906083072
   LOCAL message
   
   \ new statement
   0
   LOCALS §this |
   
   \ new statement
   message DUP INCREFERENCE
   lang.Throwable§1745900544.table 861099008 EXECUTE-NEW
   
   \ new statement
   28 MALLOC DROP DUP TO §this OVER
   IF
      OVER CELL+ @ 4 + OVER OVER ! DROP
   ENDIF
   CELL+ @ OVER
   OVER ! NIP
   lang.Error§-32422656.table OVER 12 + !
   -32440320 OVER 20 + !
   " Error " OVER 16 + !
   0 OVER 24 + ! DROP
   
   \ new statement
   §this DUP ( return object )    
   \ new statement
   0 §break5973 LABEL
   
   \ new statement
   message TRUE JavaArray.kill§1620077312
   PURGE 2
   
   \ new statement
   DROP
;


: lang.Error.classMonitorEnter§-433167616
   
   \ new statement
   
   \ new statement
   0 §break5970 LABEL
   
   \ new statement
   
   \ new statement
   DROP
;


: lang.Error.classMonitorLeave§-1691393280
   
   \ new statement
   
   \ new statement
   0 §break5971 LABEL
   
   \ new statement
   
   \ new statement
   DROP
;
VARIABLE lang.Error._staticBlocking
VARIABLE lang.Error._staticThread
2VARIABLE lang.Error.serialVersionUID

A:HERE VARIABLE lang.Error§-32422656.table 4 DUP 2* CELLS ALLOT R@ ! A:CELL+
-2086058752 R@ ! A:CELL+ lang.Error.Error§-2086058752 VAL R@ ! A:CELL+
-393329408 R@ ! A:CELL+ lang.Error.Error§-393329408 VAL R@ ! A:CELL+
-503364352 R@ ! A:CELL+ lang.Error.Error§-503364352 VAL R@ ! A:CELL+
-906083072 R@ ! A:CELL+ lang.Error.Error§-906083072 VAL R@ ! A:CELL+
A:DROP
