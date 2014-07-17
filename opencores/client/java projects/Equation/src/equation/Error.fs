MODULENAME equation.Error
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

:LOCAL Error.Error§-2086058752
   2 VALLOCATE LOCAL §base0
   DUP 4 §base0 + V! LOCAL cause
   
   \ new statement
   0
   LOCALS §this |
   
   \ new statement
   cause
   Throwable§1745900544.table -226929664 EXECUTE-NEW
   
   \ new statement
   28 MALLOC DROP DUP DUP §base0 V! TO §this OVER
   IF
      OVER CELL+ @ 4 + OVER OVER ! DROP
   ENDIF
   CELL+ @ OVER
   OVER ! NIP
   Error§-32422656.table OVER 12 + !
   -32440320 OVER 20 + !
   " Error " OVER 16 + !
   0 OVER 24 + ! DROP
   
   \ new statement
   §this DUP 0 V! ( return object )    
   \ new statement
   0 §break30154 LABEL
   
   \ new statement
   
   §base0 SETVTOP
   PURGE 3
   
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

:LOCAL Error.Error§-393329408
   3 VALLOCATE LOCAL §base0
   DUP 4 §base0 + V! LOCAL cause
   DUP 8 §base0 + V! LOCAL message
   
   \ new statement
   0
   LOCALS §this |
   
   \ new statement
   message
   cause
   Throwable§1745900544.table 1503810560 EXECUTE-NEW
   
   \ new statement
   28 MALLOC DROP DUP DUP §base0 V! TO §this OVER
   IF
      OVER CELL+ @ 4 + OVER OVER ! DROP
   ENDIF
   CELL+ @ OVER
   OVER ! NIP
   Error§-32422656.table OVER 12 + !
   -32440320 OVER 20 + !
   " Error " OVER 16 + !
   0 OVER 24 + ! DROP
   
   \ new statement
   §this DUP 0 V! ( return object )    
   \ new statement
   0 §break30153 LABEL
   
   \ new statement
   
   
   §base0 SETVTOP
   PURGE 4
   
   \ new statement
   DROP
;
( *
* Constructs a new error with <code>null</code> as its detail message.
* The cause is not initialized, and may subsequently be initialized by a
* call to {@link #initCause}.
)

:LOCAL Error.Error§-503364352
   1 VALLOCATE LOCAL §base0
   
   \ new statement
   0
   LOCALS §this |
   
   \ new statement
   Throwable§1745900544.table -862497792 EXECUTE-NEW
   
   \ new statement
   28 MALLOC DROP DUP DUP §base0 V! TO §this OVER
   IF
      OVER CELL+ @ 4 + OVER OVER ! DROP
   ENDIF
   CELL+ @ OVER
   OVER ! NIP
   Error§-32422656.table OVER 12 + !
   -32440320 OVER 20 + !
   " Error " OVER 16 + !
   0 OVER 24 + ! DROP
   
   \ new statement
   §this DUP 0 V! ( return object )    
   \ new statement
   0 §break30151 LABEL
   
   \ new statement
   §base0 SETVTOP
   PURGE 2
   
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

:LOCAL Error.Error§-906083072
   2 VALLOCATE LOCAL §base0
   DUP 4 §base0 + V! LOCAL message
   
   \ new statement
   0
   LOCALS §this |
   
   \ new statement
   message
   Throwable§1745900544.table 861099008 EXECUTE-NEW
   
   \ new statement
   28 MALLOC DROP DUP DUP §base0 V! TO §this OVER
   IF
      OVER CELL+ @ 4 + OVER OVER ! DROP
   ENDIF
   CELL+ @ OVER
   OVER ! NIP
   Error§-32422656.table OVER 12 + !
   -32440320 OVER 20 + !
   " Error " OVER 16 + !
   0 OVER 24 + ! DROP
   
   \ new statement
   §this DUP 0 V! ( return object )    
   \ new statement
   0 §break30152 LABEL
   
   \ new statement
   
   §base0 SETVTOP
   PURGE 3
   
   \ new statement
   DROP
;
VARIABLE Error._staticBlocking
VARIABLE Error._staticThread  HERE 4 - SALLOCATE
2VARIABLE Error.serialVersionUID

A:HERE VARIABLE Error§-32422656.table 4 DUP 2* CELLS ALLOT R@ ! A:CELL+
-2086058752 R@ ! A:CELL+ Error.Error§-2086058752 VAL R@ ! A:CELL+
-393329408 R@ ! A:CELL+ Error.Error§-393329408 VAL R@ ! A:CELL+
-503364352 R@ ! A:CELL+ Error.Error§-503364352 VAL R@ ! A:CELL+
-906083072 R@ ! A:CELL+ Error.Error§-906083072 VAL R@ ! A:CELL+
A:DROP
