MODULENAME equation.IncompatibleClassChangeError
(
* @(#)IncompatibleClassChangeError.java	1.17 03/01/23
*
* Copyright 2003 Sun Microsystems, Inc. All rights reserved.
* SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
)  ( *
* Thrown when an incompatible class change has occurred to some class
* definition. The definition of some class, on which the currently
* executing method depends, has since changed.
*
* @author  unascribed
* @version 1.17, 01/23/03
* @since   JDK1.0
)
( *
* Constructs an <code>IncompatibleClassChangeError</code> with the
* specified detail message.
*
* @param   s   the detail message.
)

:LOCAL IncompatibleClassChangeError.IncompatibleClassChangeError§-1356183296
   2 VALLOCATE LOCAL §base0
   DUP 4 §base0 + V! LOCAL s
   
   \ new statement
   0
   LOCALS §this |
   
   \ new statement
   s
   LinkageError§-2065150976.table -1828828160 EXECUTE-NEW
   
   \ new statement
   28 MALLOC DROP DUP DUP §base0 V! TO §this OVER
   IF
      OVER CELL+ @ 4 + OVER OVER ! DROP
   ENDIF
   CELL+ @ OVER
   OVER ! NIP
   IncompatibleClassChangeError§97667328.table OVER 12 + !
   97648640 OVER 20 + !
   " IncompatibleClassChangeError " OVER 16 + !
   0 OVER 24 + ! DROP
   
   \ new statement
   §this DUP 0 V! ( return object )    
   \ new statement
   0 §break30148 LABEL
   
   \ new statement
   
   §base0 SETVTOP
   PURGE 3
   
   \ new statement
   DROP
;
( *
* Constructs an <code>IncompatibleClassChangeError</code> with no
* detail message.
)

:LOCAL IncompatibleClassChangeError.IncompatibleClassChangeError§290474240
   1 VALLOCATE LOCAL §base0
   
   \ new statement
   0
   LOCALS §this |
   
   \ new statement
   LinkageError§-2065150976.table 1875594240 EXECUTE-NEW
   
   \ new statement
   28 MALLOC DROP DUP DUP §base0 V! TO §this OVER
   IF
      OVER CELL+ @ 4 + OVER OVER ! DROP
   ENDIF
   CELL+ @ OVER
   OVER ! NIP
   IncompatibleClassChangeError§97667328.table OVER 12 + !
   97648640 OVER 20 + !
   " IncompatibleClassChangeError " OVER 16 + !
   0 OVER 24 + ! DROP
   
   \ new statement
   §this DUP 0 V! ( return object )    
   \ new statement
   0 §break30147 LABEL
   
   \ new statement
   §base0 SETVTOP
   PURGE 2
   
   \ new statement
   DROP
;
VARIABLE IncompatibleClassChangeError._staticBlocking
VARIABLE IncompatibleClassChangeError._staticThread  HERE 4 - SALLOCATE

A:HERE VARIABLE IncompatibleClassChangeError§97667328.table 2 DUP 2* CELLS ALLOT R@ ! A:CELL+
-1356183296 R@ ! A:CELL+ IncompatibleClassChangeError.IncompatibleClassChangeError§-1356183296 VAL R@ ! A:CELL+
290474240 R@ ! A:CELL+ IncompatibleClassChangeError.IncompatibleClassChangeError§290474240 VAL R@ ! A:CELL+
A:DROP
