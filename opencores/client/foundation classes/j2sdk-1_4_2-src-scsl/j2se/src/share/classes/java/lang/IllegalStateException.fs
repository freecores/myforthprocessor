MODULENAME java.lang.IllegalStateException
(
* @(#)IllegalStateException.java	1.11 03/01/23
*
* Copyright 2003 Sun Microsystems, Inc. All rights reserved.
* SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
)  ( *
* Signals that a method has been invoked at an illegal or
* inappropriate time.  In other words, the Java environment or
* Java application is not in an appropriate state for the requested
* operation.
*
* @author  Jonni Kanerva
* @version 1.11, 01/23/03
* @since   JDK1.1
)
( *
* Constructs an IllegalStateException with no detail message.
* A detail message is a String that describes this particular exception.
)

:LOCAL lang.IllegalStateException.IllegalStateException§1042368768
   
   \ new statement
   0
   LOCALS §this |
   
   \ new statement
   lang.RuntimeException§-1151249920.table 1000231424 EXECUTE-NEW
   
   \ new statement
   28 MALLOC DROP DUP TO §this OVER
   IF
      OVER CELL+ @ 4 + OVER OVER ! DROP
   ENDIF
   CELL+ @ OVER
   OVER ! NIP
   lang.IllegalStateException§799688960.table OVER 12 + !
   799670272 OVER 20 + !
   " IllegalStateException " OVER 16 + !
   0 OVER 24 + ! DROP
   
   \ new statement
   §this DUP ( return object )    
   \ new statement
   0 §break10396 LABEL
   
   \ new statement
   PURGE 1
   
   \ new statement
   DROP
;
( *
* Constructs an IllegalStateException with the specified detail
* message.  A detail message is a String that describes this particular
* exception.
*
* @param s the String that contains a detailed message
)

:LOCAL lang.IllegalStateException.IllegalStateException§1129793792
   LOCAL s
   
   \ new statement
   0
   LOCALS §this |
   
   \ new statement
   s DUP INCREFERENCE
   lang.RuntimeException§-1151249920.table -745188864 EXECUTE-NEW
   
   \ new statement
   28 MALLOC DROP DUP TO §this OVER
   IF
      OVER CELL+ @ 4 + OVER OVER ! DROP
   ENDIF
   CELL+ @ OVER
   OVER ! NIP
   lang.IllegalStateException§799688960.table OVER 12 + !
   799670272 OVER 20 + !
   " IllegalStateException " OVER 16 + !
   0 OVER 24 + ! DROP
   
   \ new statement
   §this DUP ( return object )    
   \ new statement
   0 §break10397 LABEL
   
   \ new statement
   s TRUE lang.JavaArray.kill§1620077312
   PURGE 2
   
   \ new statement
   DROP
;


: lang.IllegalStateException.classMonitorEnter§-433167616
   
   \ new statement
   
   \ new statement
   0 §break10394 LABEL
   
   \ new statement
   
   \ new statement
   DROP
;


: lang.IllegalStateException.classMonitorLeave§-1691393280
   
   \ new statement
   
   \ new statement
   0 §break10395 LABEL
   
   \ new statement
   
   \ new statement
   DROP
;
VARIABLE lang.IllegalStateException._staticBlocking
VARIABLE lang.IllegalStateException._staticThread

A:HERE VARIABLE lang.IllegalStateException§799688960.table 2 DUP 2* CELLS ALLOT R@ ! A:CELL+
1042368768 R@ ! A:CELL+ lang.IllegalStateException.IllegalStateException§1042368768 VAL R@ ! A:CELL+
1129793792 R@ ! A:CELL+ lang.IllegalStateException.IllegalStateException§1129793792 VAL R@ ! A:CELL+
A:DROP
