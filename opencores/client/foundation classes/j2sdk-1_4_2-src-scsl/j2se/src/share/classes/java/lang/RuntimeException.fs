MODULENAME java.lang.RuntimeException
(
* @(#)RuntimeException.java	1.12 03/01/23
*
* Copyright 2003 Sun Microsystems, Inc. All rights reserved.
* SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
)  ( *
* <code>RuntimeException</code> is the superclass of those
* exceptions that can be thrown during the normal operation of the
* Java Virtual Machine.
* <p>
* A method is not required to declare in its <code>throws</code>
* clause any subclasses of <code>RuntimeException</code> that might
* be thrown during the execution of the method but not caught.
*
*
* @author  Frank Yellin
* @version 1.12, 01/23/03
* @since   JDK1.0
)
( * Constructs a new runtime exception with the specified detail message.
* The cause is not initialized, and may subsequently be initialized by a
* call to {@link #initCause}.
*
* @param   message   the detail message. The detail message is saved for
*          later retrieval by the {@link #getMessage()} method.
)

:LOCAL lang.RuntimeException.RuntimeException§-745188864
   2 VALLOCATE LOCAL §base0
   DUP 4 §base0 + V! LOCAL message
   
   \ new statement
   0
   LOCALS §this |
   
   \ new statement
   message
   lang.Exception§-1574681344.table 264127744 EXECUTE-NEW
   
   \ new statement
   28 MALLOC DROP DUP DUP §base0 V! TO §this OVER
   IF
      OVER CELL+ @ 4 + OVER OVER ! DROP
   ENDIF
   CELL+ @ OVER
   OVER ! NIP
   lang.RuntimeException§-1151249920.table OVER 12 + !
   -1151270912 OVER 20 + !
   " RuntimeException " OVER 16 + !
   0 OVER 24 + ! DROP
   
   \ new statement
   §this DUP 0 V! ( return object )    
   \ new statement
   0 §break17035 LABEL
   
   \ new statement
   
   §base0 SETVTOP
   PURGE 3
   
   \ new statement
   DROP
;
( *
* Constructs a new runtime exception with the specified detail message and
* cause.  <p>Note that the detail message associated with
* <code>cause</code> is <i>not</i> automatically incorporated in
* this runtime exception's detail message.
*
* @param  message the detail message (which is saved for later retrieval
*         by the {@link #getMessage()} method).
* @param  cause the cause (which is saved for later retrieval by the
*         {@link #getCause()} method).  (A <tt>null</tt> value is
*         permitted, and indicates that the cause is nonexistent or
*         unknown.)
* @since  1.4
)

:LOCAL lang.RuntimeException.RuntimeException§-883863040
   3 VALLOCATE LOCAL §base0
   DUP 4 §base0 + V! LOCAL cause
   DUP 8 §base0 + V! LOCAL message
   
   \ new statement
   0
   LOCALS §this |
   
   \ new statement
   message
   cause
   lang.Exception§-1574681344.table 624903424 EXECUTE-NEW
   
   \ new statement
   28 MALLOC DROP DUP DUP §base0 V! TO §this OVER
   IF
      OVER CELL+ @ 4 + OVER OVER ! DROP
   ENDIF
   CELL+ @ OVER
   OVER ! NIP
   lang.RuntimeException§-1151249920.table OVER 12 + !
   -1151270912 OVER 20 + !
   " RuntimeException " OVER 16 + !
   0 OVER 24 + ! DROP
   
   \ new statement
   §this DUP 0 V! ( return object )    
   \ new statement
   0 §break17036 LABEL
   
   \ new statement
   
   
   §base0 SETVTOP
   PURGE 4
   
   \ new statement
   DROP
;
( * Constructs a new runtime exception with the specified cause and a
* detail message of <tt>(cause==null ? null : cause.toString())</tt>
* (which typically contains the class and detail message of
* <tt>cause</tt>).  This constructor is useful for runtime exceptions
* that are little more than wrappers for other throwables.
*
* @param  cause the cause (which is saved for later retrieval by the
*         {@link #getCause()} method).  (A <tt>null</tt> value is
*         permitted, and indicates that the cause is nonexistent or
*         unknown.)
* @since  1.4
)

:LOCAL lang.RuntimeException.RuntimeException§-945597952
   2 VALLOCATE LOCAL §base0
   DUP 4 §base0 + V! LOCAL cause
   
   \ new statement
   0
   LOCALS §this |
   
   \ new statement
   cause
   lang.Exception§-1574681344.table 463815936 EXECUTE-NEW
   
   \ new statement
   28 MALLOC DROP DUP DUP §base0 V! TO §this OVER
   IF
      OVER CELL+ @ 4 + OVER OVER ! DROP
   ENDIF
   CELL+ @ OVER
   OVER ! NIP
   lang.RuntimeException§-1151249920.table OVER 12 + !
   -1151270912 OVER 20 + !
   " RuntimeException " OVER 16 + !
   0 OVER 24 + ! DROP
   
   \ new statement
   §this DUP 0 V! ( return object )    
   \ new statement
   0 §break17037 LABEL
   
   \ new statement
   
   §base0 SETVTOP
   PURGE 3
   
   \ new statement
   DROP
;
( * Constructs a new runtime exception with <code>null</code> as its
* detail message.  The cause is not initialized, and may subsequently be
* initialized by a call to {@link #initCause}.
)

:LOCAL lang.RuntimeException.RuntimeException§1000231424
   1 VALLOCATE LOCAL §base0
   
   \ new statement
   0
   LOCALS §this |
   
   \ new statement
   lang.Exception§-1574681344.table -1996143360 EXECUTE-NEW
   
   \ new statement
   28 MALLOC DROP DUP DUP §base0 V! TO §this OVER
   IF
      OVER CELL+ @ 4 + OVER OVER ! DROP
   ENDIF
   CELL+ @ OVER
   OVER ! NIP
   lang.RuntimeException§-1151249920.table OVER 12 + !
   -1151270912 OVER 20 + !
   " RuntimeException " OVER 16 + !
   0 OVER 24 + ! DROP
   
   \ new statement
   §this DUP 0 V! ( return object )    
   \ new statement
   0 §break17034 LABEL
   
   \ new statement
   §base0 SETVTOP
   PURGE 2
   
   \ new statement
   DROP
;


: lang.RuntimeException.classMonitorEnter§-433167616
   
   \ new statement
   
   \ new statement
   0 §break17032 LABEL
   
   \ new statement
   
   \ new statement
   DROP
;


: lang.RuntimeException.classMonitorLeave§-1691393280
   
   \ new statement
   
   \ new statement
   0 §break17033 LABEL
   
   \ new statement
   
   \ new statement
   DROP
;
VARIABLE lang.RuntimeException._staticBlocking
VARIABLE lang.RuntimeException._staticThread
2VARIABLE lang.RuntimeException.serialVersionUID

A:HERE VARIABLE lang.RuntimeException§-1151249920.table 4 DUP 2* CELLS ALLOT R@ ! A:CELL+
-745188864 R@ ! A:CELL+ lang.RuntimeException.RuntimeException§-745188864 VAL R@ ! A:CELL+
-883863040 R@ ! A:CELL+ lang.RuntimeException.RuntimeException§-883863040 VAL R@ ! A:CELL+
-945597952 R@ ! A:CELL+ lang.RuntimeException.RuntimeException§-945597952 VAL R@ ! A:CELL+
1000231424 R@ ! A:CELL+ lang.RuntimeException.RuntimeException§1000231424 VAL R@ ! A:CELL+
A:DROP
