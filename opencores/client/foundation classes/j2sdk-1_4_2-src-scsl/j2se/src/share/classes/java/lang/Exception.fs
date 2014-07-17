MODULENAME java.lang.Exception
(
* @(#)Exception.java	1.30 03/01/23
*
* Copyright 2003 Sun Microsystems, Inc. All rights reserved.
* SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
)  ( *
* The class <code>Exception</code> and its subclasses are a form of
* <code>Throwable</code> that indicates conditions that a reasonable
* application might want to catch.
*
* @author  Frank Yellin
* @version 1.30, 01/23/03
* @see     java.lang.Error
* @since   JDK1.0
)
( *
* Constructs a new exception with <code>null</code> as its detail message.
* The cause is not initialized, and may subsequently be initialized by a
* call to {@link #initCause}.
)

:LOCAL lang.Exception.Exception§-1996143360
   1 VALLOCATE LOCAL §base0
   
   \ new statement
   0
   LOCALS §this |
   
   \ new statement
   lang.Throwable§1745900544.table -862497792 EXECUTE-NEW
   
   \ new statement
   28 MALLOC DROP DUP DUP §base0 V! TO §this OVER
   IF
      OVER CELL+ @ 4 + OVER OVER ! DROP
   ENDIF
   CELL+ @ OVER
   OVER ! NIP
   lang.Exception§-1574681344.table OVER 12 + !
   -1574699008 OVER 20 + !
   " Exception " OVER 16 + !
   0 OVER 24 + ! DROP
   
   \ new statement
   §this DUP 0 V! ( return object )    
   \ new statement
   0 §break17040 LABEL
   
   \ new statement
   §base0 SETVTOP
   PURGE 2
   
   \ new statement
   DROP
;
( *
* Constructs a new exception with the specified detail message.  The
* cause is not initialized, and may subsequently be initialized by
* a call to {@link #initCause}.
*
* @param   message   the detail message. The detail message is saved for
*          later retrieval by the {@link #getMessage()} method.
)

:LOCAL lang.Exception.Exception§264127744
   2 VALLOCATE LOCAL §base0
   DUP 4 §base0 + V! LOCAL message
   
   \ new statement
   0
   LOCALS §this |
   
   \ new statement
   message
   lang.Throwable§1745900544.table 861099008 EXECUTE-NEW
   
   \ new statement
   28 MALLOC DROP DUP DUP §base0 V! TO §this OVER
   IF
      OVER CELL+ @ 4 + OVER OVER ! DROP
   ENDIF
   CELL+ @ OVER
   OVER ! NIP
   lang.Exception§-1574681344.table OVER 12 + !
   -1574699008 OVER 20 + !
   " Exception " OVER 16 + !
   0 OVER 24 + ! DROP
   
   \ new statement
   §this DUP 0 V! ( return object )    
   \ new statement
   0 §break17041 LABEL
   
   \ new statement
   
   §base0 SETVTOP
   PURGE 3
   
   \ new statement
   DROP
;
( *
* Constructs a new exception with the specified cause and a detail
* message of <tt>(cause==null ? null : cause.toString())</tt> (which
* typically contains the class and detail message of <tt>cause</tt>).
* This constructor is useful for exceptions that are little more than
* wrappers for other throwables (for example, {@link
* java.security.PrivilegedActionException}).
*
* @param  cause the cause (which is saved for later retrieval by the
*         {@link #getCause()} method).  (A <tt>null</tt> value is
*         permitted, and indicates that the cause is nonexistent or
*         unknown.)
* @since  1.4
)

:LOCAL lang.Exception.Exception§463815936
   2 VALLOCATE LOCAL §base0
   DUP 4 §base0 + V! LOCAL cause
   
   \ new statement
   0
   LOCALS §this |
   
   \ new statement
   cause
   lang.Throwable§1745900544.table -226929664 EXECUTE-NEW
   
   \ new statement
   28 MALLOC DROP DUP DUP §base0 V! TO §this OVER
   IF
      OVER CELL+ @ 4 + OVER OVER ! DROP
   ENDIF
   CELL+ @ OVER
   OVER ! NIP
   lang.Exception§-1574681344.table OVER 12 + !
   -1574699008 OVER 20 + !
   " Exception " OVER 16 + !
   0 OVER 24 + ! DROP
   
   \ new statement
   §this DUP 0 V! ( return object )    
   \ new statement
   0 §break17043 LABEL
   
   \ new statement
   
   §base0 SETVTOP
   PURGE 3
   
   \ new statement
   DROP
;
( *
* Constructs a new exception with the specified detail message and
* cause.  <p>Note that the detail message associated with
* <code>cause</code> is <i>not</i> automatically incorporated in
* this exception's detail message.
*
* @param  message the detail message (which is saved for later retrieval
*         by the {@link #getMessage()} method).
* @param  cause the cause (which is saved for later retrieval by the
*         {@link #getCause()} method).  (A <tt>null</tt> value is
*         permitted, and indicates that the cause is nonexistent or
*         unknown.)
* @since  1.4
)

:LOCAL lang.Exception.Exception§624903424
   3 VALLOCATE LOCAL §base0
   DUP 4 §base0 + V! LOCAL cause
   DUP 8 §base0 + V! LOCAL message
   
   \ new statement
   0
   LOCALS §this |
   
   \ new statement
   message
   cause
   lang.Throwable§1745900544.table 1503810560 EXECUTE-NEW
   
   \ new statement
   28 MALLOC DROP DUP DUP §base0 V! TO §this OVER
   IF
      OVER CELL+ @ 4 + OVER OVER ! DROP
   ENDIF
   CELL+ @ OVER
   OVER ! NIP
   lang.Exception§-1574681344.table OVER 12 + !
   -1574699008 OVER 20 + !
   " Exception " OVER 16 + !
   0 OVER 24 + ! DROP
   
   \ new statement
   §this DUP 0 V! ( return object )    
   \ new statement
   0 §break17042 LABEL
   
   \ new statement
   
   
   §base0 SETVTOP
   PURGE 4
   
   \ new statement
   DROP
;


: lang.Exception.classMonitorEnter§-433167616
   
   \ new statement
   
   \ new statement
   0 §break17038 LABEL
   
   \ new statement
   
   \ new statement
   DROP
;


: lang.Exception.classMonitorLeave§-1691393280
   
   \ new statement
   
   \ new statement
   0 §break17039 LABEL
   
   \ new statement
   
   \ new statement
   DROP
;
VARIABLE lang.Exception._staticBlocking
VARIABLE lang.Exception._staticThread
2VARIABLE lang.Exception.serialVersionUID

A:HERE VARIABLE lang.Exception§-1574681344.table 4 DUP 2* CELLS ALLOT R@ ! A:CELL+
-1996143360 R@ ! A:CELL+ lang.Exception.Exception§-1996143360 VAL R@ ! A:CELL+
264127744 R@ ! A:CELL+ lang.Exception.Exception§264127744 VAL R@ ! A:CELL+
463815936 R@ ! A:CELL+ lang.Exception.Exception§463815936 VAL R@ ! A:CELL+
624903424 R@ ! A:CELL+ lang.Exception.Exception§624903424 VAL R@ ! A:CELL+
A:DROP
