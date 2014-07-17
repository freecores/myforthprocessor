MODULENAME java.lang.IndexOutOfBoundsException
(
* @(#)IndexOutOfBoundsException.java	1.10 03/01/23
*
* Copyright 2003 Sun Microsystems, Inc. All rights reserved.
* SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
)  ( *
* Thrown to indicate that an index of some sort (such as to an array, to a
* string, or to a vector) is out of range.
* <p>
* Applications can subclass this class to indicate similar exceptions.
*
* @author  Frank Yellin
* @version 1.10, 01/23/03
* @since   JDK1.0
)
( *
* Constructs an <code>IndexOutOfBoundsException</code> with the
* specified detail message.
*
* @param   s   the detail message.
)

:LOCAL lang.IndexOutOfBoundsException.IndexOutOfBoundsException§-360625920
   2 VALLOCATE LOCAL §base0
   DUP 4 §base0 + V! LOCAL s
   
   \ new statement
   0
   LOCALS §this |
   
   \ new statement
   s
   lang.RuntimeException§-1151249920.table -745188864 EXECUTE-NEW
   
   \ new statement
   28 MALLOC DROP DUP DUP §base0 V! TO §this OVER
   IF
      OVER CELL+ @ 4 + OVER OVER ! DROP
   ENDIF
   CELL+ @ OVER
   OVER ! NIP
   lang.IndexOutOfBoundsException§1771129088.table OVER 12 + !
   1771110400 OVER 20 + !
   " IndexOutOfBoundsException " OVER 16 + !
   0 OVER 24 + ! DROP
   
   \ new statement
   §this DUP 0 V! ( return object )    
   \ new statement
   0 §break17891 LABEL
   
   \ new statement
   
   §base0 SETVTOP
   PURGE 3
   
   \ new statement
   DROP
;
( *
* Constructs an <code>IndexOutOfBoundsException</code> with no
* detail message.
)

:LOCAL lang.IndexOutOfBoundsException.IndexOutOfBoundsException§-496219904
   1 VALLOCATE LOCAL §base0
   
   \ new statement
   0
   LOCALS §this |
   
   \ new statement
   lang.RuntimeException§-1151249920.table 1000231424 EXECUTE-NEW
   
   \ new statement
   28 MALLOC DROP DUP DUP §base0 V! TO §this OVER
   IF
      OVER CELL+ @ 4 + OVER OVER ! DROP
   ENDIF
   CELL+ @ OVER
   OVER ! NIP
   lang.IndexOutOfBoundsException§1771129088.table OVER 12 + !
   1771110400 OVER 20 + !
   " IndexOutOfBoundsException " OVER 16 + !
   0 OVER 24 + ! DROP
   
   \ new statement
   §this DUP 0 V! ( return object )    
   \ new statement
   0 §break17890 LABEL
   
   \ new statement
   §base0 SETVTOP
   PURGE 2
   
   \ new statement
   DROP
;


: lang.IndexOutOfBoundsException.classMonitorEnter§-433167616
   
   \ new statement
   
   \ new statement
   0 §break17888 LABEL
   
   \ new statement
   
   \ new statement
   DROP
;


: lang.IndexOutOfBoundsException.classMonitorLeave§-1691393280
   
   \ new statement
   
   \ new statement
   0 §break17889 LABEL
   
   \ new statement
   
   \ new statement
   DROP
;
VARIABLE lang.IndexOutOfBoundsException._staticBlocking
VARIABLE lang.IndexOutOfBoundsException._staticThread

A:HERE VARIABLE lang.IndexOutOfBoundsException§1771129088.table 2 DUP 2* CELLS ALLOT R@ ! A:CELL+
-360625920 R@ ! A:CELL+ lang.IndexOutOfBoundsException.IndexOutOfBoundsException§-360625920 VAL R@ ! A:CELL+
-496219904 R@ ! A:CELL+ lang.IndexOutOfBoundsException.IndexOutOfBoundsException§-496219904 VAL R@ ! A:CELL+
A:DROP
