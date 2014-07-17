MODULENAME equation.IllegalStateException
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

:LOCAL IllegalStateException.IllegalStateException§1042368768
   1 VALLOCATE LOCAL §base0
   
   \ new statement
   0
   LOCALS §this |
   
   \ new statement
   RuntimeException§-1151249920.table 1000231424 EXECUTE-NEW
   
   \ new statement
   28 MALLOC DROP DUP DUP §base0 V! TO §this OVER
   IF
      OVER CELL+ @ 4 + OVER OVER ! DROP
   ENDIF
   CELL+ @ OVER
   OVER ! NIP
   IllegalStateException§799688960.table OVER 12 + !
   799670272 OVER 20 + !
   " IllegalStateException " OVER 16 + !
   0 OVER 24 + ! DROP
   
   \ new statement
   §this DUP 0 V! ( return object )    
   \ new statement
   0 §break32325 LABEL
   
   \ new statement
   §base0 SETVTOP
   PURGE 2
   
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

:LOCAL IllegalStateException.IllegalStateException§1129793792
   2 VALLOCATE LOCAL §base0
   DUP 4 §base0 + V! LOCAL s
   
   \ new statement
   0
   LOCALS §this |
   
   \ new statement
   s
   RuntimeException§-1151249920.table -745188864 EXECUTE-NEW
   
   \ new statement
   28 MALLOC DROP DUP DUP §base0 V! TO §this OVER
   IF
      OVER CELL+ @ 4 + OVER OVER ! DROP
   ENDIF
   CELL+ @ OVER
   OVER ! NIP
   IllegalStateException§799688960.table OVER 12 + !
   799670272 OVER 20 + !
   " IllegalStateException " OVER 16 + !
   0 OVER 24 + ! DROP
   
   \ new statement
   §this DUP 0 V! ( return object )    
   \ new statement
   0 §break32326 LABEL
   
   \ new statement
   
   §base0 SETVTOP
   PURGE 3
   
   \ new statement
   DROP
;
VARIABLE IllegalStateException._staticBlocking
VARIABLE IllegalStateException._staticThread  HERE 4 - SALLOCATE

A:HERE VARIABLE IllegalStateException§799688960.table 2 DUP 2* CELLS ALLOT R@ ! A:CELL+
1042368768 R@ ! A:CELL+ IllegalStateException.IllegalStateException§1042368768 VAL R@ ! A:CELL+
1129793792 R@ ! A:CELL+ IllegalStateException.IllegalStateException§1129793792 VAL R@ ! A:CELL+
A:DROP
