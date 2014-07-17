MODULENAME equation.LinkageError
(
* @(#)LinkageError.java	1.12 03/01/23
*
* Copyright 2003 Sun Microsystems, Inc. All rights reserved.
* SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
)  ( *
* Subclasses of <code>LinkageError</code> indicate that a class has
* some dependency on another class; however, the latter class has
* incompatibly changed after the compilation of the former class.
*
*
* @author  Frank Yellin
* @version 1.12, 01/23/03
* @since   JDK1.0
)
( *
* Constructs a <code>LinkageError</code> with the specified detail
* message.
*
* @param   s   the detail message.
)

:LOCAL LinkageError.LinkageError§-1828828160
   2 VALLOCATE LOCAL §base0
   DUP 4 §base0 + V! LOCAL s
   
   \ new statement
   0
   LOCALS §this |
   
   \ new statement
   s
   Error§-32422656.table -906083072 EXECUTE-NEW
   
   \ new statement
   28 MALLOC DROP DUP DUP §base0 V! TO §this OVER
   IF
      OVER CELL+ @ 4 + OVER OVER ! DROP
   ENDIF
   CELL+ @ OVER
   OVER ! NIP
   LinkageError§-2065150976.table OVER 12 + !
   -2065170432 OVER 20 + !
   " LinkageError " OVER 16 + !
   0 OVER 24 + ! DROP
   
   \ new statement
   §this DUP 0 V! ( return object )    
   \ new statement
   0 §break30150 LABEL
   
   \ new statement
   
   §base0 SETVTOP
   PURGE 3
   
   \ new statement
   DROP
;
( *
* Constructs a <code>LinkageError</code> with no detail message.
)

:LOCAL LinkageError.LinkageError§1875594240
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
   LinkageError§-2065150976.table OVER 12 + !
   -2065170432 OVER 20 + !
   " LinkageError " OVER 16 + !
   0 OVER 24 + ! DROP
   
   \ new statement
   §this DUP 0 V! ( return object )    
   \ new statement
   0 §break30149 LABEL
   
   \ new statement
   §base0 SETVTOP
   PURGE 2
   
   \ new statement
   DROP
;
VARIABLE LinkageError._staticBlocking
VARIABLE LinkageError._staticThread  HERE 4 - SALLOCATE

A:HERE VARIABLE LinkageError§-2065150976.table 2 DUP 2* CELLS ALLOT R@ ! A:CELL+
-1828828160 R@ ! A:CELL+ LinkageError.LinkageError§-1828828160 VAL R@ ! A:CELL+
1875594240 R@ ! A:CELL+ LinkageError.LinkageError§1875594240 VAL R@ ! A:CELL+
A:DROP
