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

:LOCAL java.lang.Exception.Exception§-1996143360
   
   \ new statement
   0
   LOCALS §this |
   
   \ new statement
   0
   java.lang.Exception§-1574681344.table 264127744 EXECUTE-NEW
   TO §this
   
   \ new statement
   §this ( return object )    
   \ new statement
   0 §break218 LABEL
   
   \ new statement
   PURGE 1
   
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

:LOCAL java.lang.Exception.Exception§264127744
   LOCAL message
   
   \ new statement
   0
   LOCALS §this |
   
   \ new statement
   sort.Object§-1890496768.table 1827294976 EXECUTE-NEW
   
   \ new statement
   32 MALLOC DROP DUP TO §this OVER
   IF
      OVER CELL+ @ 4 + OVER OVER ! DROP
   ENDIF
   CELL+ @ OVER
   OVER ! NIP
   java.lang.Exception§-1574681344.table OVER 12 + !
   -1574699008 OVER 20 + !
   " Exception " OVER 16 + ! DROP
   
   \ new statement
   \ extends Throwable
   
   
   \ new statement
   0
   §this CELL+ @ 24 + ( java.lang.Exception._dynamicBlocking )    !
   
   \ new statement
   message DUP INCREFERENCE
   §this CELL+ @ 28 + ( java.lang.Exception.message )    DUP @ TRUE equation.JavaArray.kill§1620077312 !
   
   \ new statement
   §this ( return object )    
   \ new statement
   0 §break219 LABEL
   
   \ new statement
   message TRUE equation.JavaArray.kill§1620077312
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

:LOCAL java.lang.Exception.getLocalizedMessage§1223845632
   LOCAL §this
   
   \ new statement
   
   \ new statement
   §this A:R@
   R> 1695049472 TRUE ( java.lang.Exception.getMessage§1695049472 ) EXECUTE-METHOD
   0 §break221 BRANCH
   
   \ new statement
   0 §break221 LABEL
   
   \ new statement
   PURGE 1
   
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
public Exception(String message, Throwable cause) {
super(message, cause);
}
)  ( *
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
public Exception(Throwable cause) {
super(cause);
}
)  ( *
* Returns the detail message string of this throwable.
*
* @return  the detail message string of this <tt>Throwable</tt> instance
*          (which may be <tt>null</tt>).
)

:LOCAL java.lang.Exception.getMessage§1695049472
   LOCAL §this
   
   \ new statement
   
   \ new statement
   §this CELL+ @ 28 + ( java.lang.Exception.message )    @ DUP INCREFERENCE
   0 §break220 BRANCH
   
   \ new statement
   0 §break220 LABEL
   
   \ new statement
   PURGE 1
   
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

:LOCAL java.lang.Exception.toString§1621718016
   LOCAL §this
   
   \ new statement
   0 DUP
   LOCALS message s |
   
   \ new statement
   §this CELL+ @ A:R@
   R> 1621718016 FALSE ( sort.Object.toString§1621718016 ) EXECUTE-METHOD
   s TRUE equation.JavaArray.kill§1620077312 TO s
   
   \ new statement
   §this A:R@
   R> 1223845632 TRUE ( java.lang.Exception.getLocalizedMessage§1223845632 ) EXECUTE-METHOD
   message TRUE equation.JavaArray.kill§1620077312 TO message
   
   \ new statement
   
   0 DUP
   LOCALS 1§ 0§ |
   
   message
   0<>
   
   IF
      s
      U" :  " COUNT equation.JavaArray.createUnicode§-675323136 DUP TO 0§
      DUP INCREFERENCE  OVER  -1461427456 TRUE ( equation.String.concat§-1461427456 ) EXECUTE-METHOD DUP TO 1§
      message
      DUP INCREFERENCE  OVER  -1461427456 TRUE ( equation.String.concat§-1461427456 ) EXECUTE-METHOD
      
   ELSE
      s
      DUP INCREFERENCE
   ENDIF
   1§ TRUE equation.JavaArray.kill§1620077312
   0§ TRUE equation.JavaArray.kill§1620077312
   
   PURGE 2
   
   0 §break222 BRANCH
   
   \ new statement
   0 §break222 LABEL
   
   \ new statement
   message TRUE equation.JavaArray.kill§1620077312
   s TRUE equation.JavaArray.kill§1620077312
   PURGE 3
   
   \ new statement
   DROP
;


:LOCAL java.lang.Exception.~destructor§-1086882304
   LOCAL §this
   
   \ new statement
   0
   LOCALS obj |
   
   \ new statement
   §this @ 1- 0=
   IF
      §this CELL+ @  @ TO obj
      
      \ new statement
      obj
      0
      equation.JavaArray.kill§1620077312
      
      \ new statement
      §this CELL+ @ 8 +  @ TO obj
      
      \ new statement
      obj
      0
      equation.JavaArray.kill§1620077312
      
      \ new statement
      §this CELL+ @ 28 + ( java.lang.Exception.message )       @ A:R@
      A:DROP
      TO obj
      
      \ new statement
      obj
      -1
      equation.JavaArray.kill§1620077312
      
      \ new statement
   ENDIF
   §this DECREFERENCE
   
   \ new statement
   0 §break223 LABEL
   
   \ new statement
   PURGE 2
   
   \ new statement
   DROP
;
VARIABLE java.lang.Exception._staticBlocking
2VARIABLE java.lang.Exception.serialVersionUID

A:HERE VARIABLE java.lang.Exception§-1574681344.table 6 DUP 2* CELLS ALLOT R@ ! A:CELL+
-1996143360 R@ ! A:CELL+ java.lang.Exception.Exception§-1996143360 VAL R@ ! A:CELL+
264127744 R@ ! A:CELL+ java.lang.Exception.Exception§264127744 VAL R@ ! A:CELL+
1223845632 R@ ! A:CELL+ java.lang.Exception.getLocalizedMessage§1223845632 VAL R@ ! A:CELL+
1695049472 R@ ! A:CELL+ java.lang.Exception.getMessage§1695049472 VAL R@ ! A:CELL+
1621718016 R@ ! A:CELL+ java.lang.Exception.toString§1621718016 VAL R@ ! A:CELL+
-1086882304 R@ ! A:CELL+ java.lang.Exception.~destructor§-1086882304 VAL R@ ! A:CELL+
A:DROP
