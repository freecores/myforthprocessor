MODULENAME equation.NullPointerException
(
* @(#)NullPointerException.java	1.18 03/01/23
*
* Copyright 2003 Sun Microsystems, Inc. All rights reserved.
* SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
)  ( *
* Thrown when an application attempts to use <code>null</code> in a
* case where an object is required. These include:
* <ul>
* <li>Calling the instance method of a <code>null</code> object.
* <li>Accessing or modifying the field of a <code>null</code> object.
* <li>Taking the length of <code>null</code> as if it were an array.
* <li>Accessing or modifying the slots of <code>null</code> as if it
*     were an array.
* <li>Throwing <code>null</code> as if it were a <code>Throwable</code>
*     value.
* </ul>
* <p>
* Applications should throw instances of this class to indicate
* other illegal uses of the <code>null</code> object.
*
* @author  unascribed
* @version 1.18, 01/23/03
* @since   JDK1.0
)
( *
* Constructs a <code>NullPointerException</code> with the specified
* detail message.
*
* @param   s   the detail message.
)

:LOCAL NullPointerException.NullPointerException§-938062336
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
   NullPointerException§561335808.table OVER 12 + !
   561315840 OVER 20 + !
   " NullPointerException " OVER 16 + !
   0 OVER 24 + ! DROP
   
   \ new statement
   §this DUP 0 V! ( return object )    
   \ new statement
   0 §break28665 LABEL
   
   \ new statement
   
   §base0 SETVTOP
   PURGE 3
   
   \ new statement
   DROP
;
( *
* Constructs a <code>NullPointerException</code> with no detail message.
)

:LOCAL NullPointerException.NullPointerException§966675968
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
   NullPointerException§561335808.table OVER 12 + !
   561315840 OVER 20 + !
   " NullPointerException " OVER 16 + !
   0 OVER 24 + ! DROP
   
   \ new statement
   §this DUP 0 V! ( return object )    
   \ new statement
   0 §break28664 LABEL
   
   \ new statement
   §base0 SETVTOP
   PURGE 2
   
   \ new statement
   DROP
;
VARIABLE NullPointerException._staticBlocking
VARIABLE NullPointerException._staticThread  HERE 4 - SALLOCATE

A:HERE VARIABLE NullPointerException§561335808.table 2 DUP 2* CELLS ALLOT R@ ! A:CELL+
-938062336 R@ ! A:CELL+ NullPointerException.NullPointerException§-938062336 VAL R@ ! A:CELL+
966675968 R@ ! A:CELL+ NullPointerException.NullPointerException§966675968 VAL R@ ! A:CELL+
A:DROP
