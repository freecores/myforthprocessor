MODULENAME equation.ThreadDeath
(
* @(#)ThreadDeath.java	1.14 03/01/23
*
* Copyright 2003 Sun Microsystems, Inc. All rights reserved.
* SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
)  ( *
* An instance of <code>ThreadDeath</code> is thrown in the victim
* thread when the <code>stop</code> method with zero arguments in
* class <code>Thread</code> is called.
* <p>
* An application should catch instances of this class only if it
* must clean up after being terminated asynchronously. If
* <code>ThreadDeath</code> is caught by a method, it is important
* that it be rethrown so that the thread actually dies.
* <p>
* The top-level error handler does not print out a message if
* <code>ThreadDeath</code> is never caught.
* <p>
* The class <code>ThreadDeath</code> is specifically a subclass of
* <code>Error</code> rather than <code>Exception</code>, even though
* it is a "normal occurrence", because many applications
* catch all occurrences of <code>Exception</code> and then discard
* the exception.
*
* @author unascribed
* @version 1.14, 01/23/03
* @see     java.lang.Thread#stop()
* @since   JDK1.0
)


:LOCAL ThreadDeath.ThreadDeath§-1686612992
   1 VALLOCATE LOCAL §base0
   
   \ new statement
   0
   LOCALS §this |
   
   \ new statement
   Error§-32422656.table -503364352 EXECUTE-NEW
   
   \ new statement
   28 MALLOC DROP DUP DUP §base0 V! TO §this OVER
   IF
      OVER CELL+ @ 4 + OVER OVER ! DROP
   ENDIF
   CELL+ @ OVER
   OVER ! NIP
   ThreadDeath§-477015040.table OVER 12 + !
   -477036544 OVER 20 + !
   " ThreadDeath " OVER 16 + !
   0 OVER 24 + ! DROP
   
   \ new statement
   §this DUP 0 V! ( return object )    
   \ new statement
   0 §break30234 LABEL
   
   \ new statement
   §base0 SETVTOP
   PURGE 2
   
   \ new statement
   DROP
;
VARIABLE ThreadDeath._staticBlocking
VARIABLE ThreadDeath._staticThread  HERE 4 - SALLOCATE

A:HERE VARIABLE ThreadDeath§-477015040.table 1 DUP 2* CELLS ALLOT R@ ! A:CELL+
-1686612992 R@ ! A:CELL+ ThreadDeath.ThreadDeath§-1686612992 VAL R@ ! A:CELL+
A:DROP
