MODULENAME equation.IllegalArgumentException
(
* @(#)IllegalArgumentException.java	1.19 03/01/23
*
* Copyright 2003 Sun Microsystems, Inc. All rights reserved.
* SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
)  ( *
* Thrown to indicate that a method has been passed an illegal or
* inappropriate argument.
*
* @author  unascribed
* @version 1.19, 01/23/03
* @see	    java.lang.Thread#setPriority(int)
* @since   JDK1.0
)
( *
* Constructs an <code>IllegalArgumentException</code> with no
* detail message.
)

:LOCAL IllegalArgumentException.IllegalArgumentException§-564049664
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
   IllegalArgumentException§-946714368.table OVER 12 + !
   -946733056 OVER 20 + !
   " IllegalArgumentException " OVER 16 + !
   0 OVER 24 + ! DROP
   
   \ new statement
   §this DUP 0 V! ( return object )    
   \ new statement
   0 §break28204 LABEL
   
   \ new statement
   §base0 SETVTOP
   PURGE 2
   
   \ new statement
   DROP
;
( *
* Constructs an <code>IllegalArgumentException</code> with the
* specified detail message.
*
* @param   s   the detail message.
)

:LOCAL IllegalArgumentException.IllegalArgumentException§737495296
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
   IllegalArgumentException§-946714368.table OVER 12 + !
   -946733056 OVER 20 + !
   " IllegalArgumentException " OVER 16 + !
   0 OVER 24 + ! DROP
   
   \ new statement
   §this DUP 0 V! ( return object )    
   \ new statement
   0 §break28205 LABEL
   
   \ new statement
   
   §base0 SETVTOP
   PURGE 3
   
   \ new statement
   DROP
;
VARIABLE IllegalArgumentException._staticBlocking
VARIABLE IllegalArgumentException._staticThread  HERE 4 - SALLOCATE

A:HERE VARIABLE IllegalArgumentException§-946714368.table 2 DUP 2* CELLS ALLOT R@ ! A:CELL+
-564049664 R@ ! A:CELL+ IllegalArgumentException.IllegalArgumentException§-564049664 VAL R@ ! A:CELL+
737495296 R@ ! A:CELL+ IllegalArgumentException.IllegalArgumentException§737495296 VAL R@ ! A:CELL+
A:DROP
