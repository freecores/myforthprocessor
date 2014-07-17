MODULENAME equation.InterruptedException
(
* @(#)InterruptedException.java	1.14 03/01/23
*
* Copyright 2003 Sun Microsystems, Inc. All rights reserved.
* SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
)  ( *
* Thrown when a thread is waiting, sleeping, or otherwise paused for
* a long time and another thread interrupts it using the
* <code>interrupt</code>  method in class <code>Thread</code>.
*
* @author  Frank Yellin
* @version 1.14, 01/23/03
* @see     java.lang.Object#wait()
* @see     java.lang.Object#wait(long)
* @see     java.lang.Object#wait(long, int)
* @see     java.lang.Thread#sleep(long)
* @see     java.lang.Thread#interrupt()
* @see     java.lang.Thread#interrupted()
* @since   JDK1.0
)
( *
* Constructs an <code>InterruptedException</code> with no detail  message.
)

:LOCAL InterruptedException.InterruptedException§1322010880
   1 VALLOCATE LOCAL §base0
   
   \ new statement
   0
   LOCALS §this |
   
   \ new statement
   Exception§-1574681344.table -1996143360 EXECUTE-NEW
   
   \ new statement
   28 MALLOC DROP DUP DUP §base0 V! TO §this OVER
   IF
      OVER CELL+ @ 4 + OVER OVER ! DROP
   ENDIF
   CELL+ @ OVER
   OVER ! NIP
   InterruptedException§-1721546496.table OVER 12 + !
   -1721565184 OVER 20 + !
   " InterruptedException " OVER 16 + !
   0 OVER 24 + ! DROP
   
   \ new statement
   §this DUP 0 V! ( return object )    
   \ new statement
   0 §break30127 LABEL
   
   \ new statement
   §base0 SETVTOP
   PURGE 2
   
   \ new statement
   DROP
;
( *
* Constructs an <code>InterruptedException</code> with the
* specified detail message.
*
* @param   s   the detail message.
)

:LOCAL InterruptedException.InterruptedException§601573632
   2 VALLOCATE LOCAL §base0
   DUP 4 §base0 + V! LOCAL s
   
   \ new statement
   0
   LOCALS §this |
   
   \ new statement
   s
   Exception§-1574681344.table 264127744 EXECUTE-NEW
   
   \ new statement
   28 MALLOC DROP DUP DUP §base0 V! TO §this OVER
   IF
      OVER CELL+ @ 4 + OVER OVER ! DROP
   ENDIF
   CELL+ @ OVER
   OVER ! NIP
   InterruptedException§-1721546496.table OVER 12 + !
   -1721565184 OVER 20 + !
   " InterruptedException " OVER 16 + !
   0 OVER 24 + ! DROP
   
   \ new statement
   §this DUP 0 V! ( return object )    
   \ new statement
   0 §break30128 LABEL
   
   \ new statement
   
   §base0 SETVTOP
   PURGE 3
   
   \ new statement
   DROP
;
VARIABLE InterruptedException._staticBlocking
VARIABLE InterruptedException._staticThread  HERE 4 - SALLOCATE

A:HERE VARIABLE InterruptedException§-1721546496.table 2 DUP 2* CELLS ALLOT R@ ! A:CELL+
1322010880 R@ ! A:CELL+ InterruptedException.InterruptedException§1322010880 VAL R@ ! A:CELL+
601573632 R@ ! A:CELL+ InterruptedException.InterruptedException§601573632 VAL R@ ! A:CELL+
A:DROP
