MODULENAME equation.IllegalThreadStateException
(
* @(#)IllegalThreadStateException.java	1.20 03/01/23
*
* Copyright 2003 Sun Microsystems, Inc. All rights reserved.
* SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
)  ( *
* Thrown to indicate that a thread is not in an appropriate state
* for the requested operation. See, for example, the
* <code>suspend</code> and <code>resume</code> methods in class
* <code>Thread</code>.
*
* @author  unascribed
* @version 1.20, 01/23/03
* @see     java.lang.Thread#resume()
* @see     java.lang.Thread#suspend()
* @since   JDK1.0
)
( *
* Constructs an <code>IllegalThreadStateException</code> with no
* detail message.
)

:LOCAL IllegalThreadStateException.IllegalThreadStateException§-1901442816
   1 VALLOCATE LOCAL §base0
   
   \ new statement
   0
   LOCALS §this |
   
   \ new statement
   IllegalArgumentException§-946714368.table -564049664 EXECUTE-NEW
   
   \ new statement
   28 MALLOC DROP DUP DUP §base0 V! TO §this OVER
   IF
      OVER CELL+ @ 4 + OVER OVER ! DROP
   ENDIF
   CELL+ @ OVER
   OVER ! NIP
   IllegalThreadStateException§-782350080.table OVER 12 + !
   -782368768 OVER 20 + !
   " IllegalThreadStateException " OVER 16 + !
   0 OVER 24 + ! DROP
   
   \ new statement
   §this DUP 0 V! ( return object )    
   \ new statement
   0 §break28682 LABEL
   
   \ new statement
   §base0 SETVTOP
   PURGE 2
   
   \ new statement
   DROP
;
( *
* Constructs an <code>IllegalThreadStateException</code> with the
* specified detail message.
*
* @param   s   the detail message.
)

:LOCAL IllegalThreadStateException.IllegalThreadStateException§418007296
   2 VALLOCATE LOCAL §base0
   DUP 4 §base0 + V! LOCAL s
   
   \ new statement
   0
   LOCALS §this |
   
   \ new statement
   s
   IllegalArgumentException§-946714368.table 737495296 EXECUTE-NEW
   
   \ new statement
   28 MALLOC DROP DUP DUP §base0 V! TO §this OVER
   IF
      OVER CELL+ @ 4 + OVER OVER ! DROP
   ENDIF
   CELL+ @ OVER
   OVER ! NIP
   IllegalThreadStateException§-782350080.table OVER 12 + !
   -782368768 OVER 20 + !
   " IllegalThreadStateException " OVER 16 + !
   0 OVER 24 + ! DROP
   
   \ new statement
   §this DUP 0 V! ( return object )    
   \ new statement
   0 §break28683 LABEL
   
   \ new statement
   
   §base0 SETVTOP
   PURGE 3
   
   \ new statement
   DROP
;
VARIABLE IllegalThreadStateException._staticBlocking
VARIABLE IllegalThreadStateException._staticThread  HERE 4 - SALLOCATE

A:HERE VARIABLE IllegalThreadStateException§-782350080.table 2 DUP 2* CELLS ALLOT R@ ! A:CELL+
-1901442816 R@ ! A:CELL+ IllegalThreadStateException.IllegalThreadStateException§-1901442816 VAL R@ ! A:CELL+
418007296 R@ ! A:CELL+ IllegalThreadStateException.IllegalThreadStateException§418007296 VAL R@ ! A:CELL+
A:DROP
