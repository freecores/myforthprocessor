MODULENAME java.lang.StringIndexOutOfBoundsException
(
* @(#)StringIndexOutOfBoundsException.java	1.21 03/01/23
*
* Copyright 2003 Sun Microsystems, Inc. All rights reserved.
* SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
)  ( *
* Thrown by <code>String</code> methods to indicate that an index
* is either negative or greater than the size of the string.  For
* some methods such as the charAt method, this exception also is
* thrown when the index is equal to the size of the string.
*
* @author  unascribed
* @version 1.21, 01/23/03
* @see     java.lang.String#charAt(int)
* @since   JDK1.0
)
( *
* Constructs a new <code>StringIndexOutOfBoundsException</code>
* class with an argument indicating the illegal index.
*
* @param   index   the illegal index.
)

:LOCAL lang.StringIndexOutOfBoundsException.StringIndexOutOfBoundsException§1138905856
   1 VALLOCATE LOCAL §base0
   LOCAL index
   
   \ new statement
   0
   LOCALS §this |
   
   \ new statement
   
   2 VALLOCATE LOCAL §tempvar
   U" String index out of range:  " COUNT lang.JavaArray.createUnicode§-675323136 DUP §tempvar V!
   index  <# 0 ..R lang.JavaArray.createString§-105880832 DUP §tempvar 4 + V!
   OVER  -1461427456 TRUE ( java.lang.String.concat§-1461427456 ) EXECUTE-METHOD
   lang.IndexOutOfBoundsException§1771129088.table -360625920 EXECUTE-NEW
   
   
   
   §tempvar SETVTOP
   
   PURGE 1
   
   
   \ new statement
   28 MALLOC DROP DUP DUP §base0 V! TO §this OVER
   IF
      OVER CELL+ @ 4 + OVER OVER ! DROP
   ENDIF
   CELL+ @ OVER
   OVER ! NIP
   lang.StringIndexOutOfBoundsException§-945597696.table OVER 12 + !
   -945618944 OVER 20 + !
   " StringIndexOutOfBoundsException " OVER 16 + !
   0 OVER 24 + ! DROP
   
   \ new statement
   §this DUP 0 V! ( return object )    
   \ new statement
   0 §break18589 LABEL
   
   \ new statement
   
   §base0 SETVTOP
   PURGE 3
   
   \ new statement
   DROP
;
( *
* Constructs a <code>StringIndexOutOfBoundsException</code> with no
* detail message.
*
* @since   JDK1.0.
)

:LOCAL lang.StringIndexOutOfBoundsException.StringIndexOutOfBoundsException§1355436800
   1 VALLOCATE LOCAL §base0
   
   \ new statement
   0
   LOCALS §this |
   
   \ new statement
   lang.IndexOutOfBoundsException§1771129088.table -496219904 EXECUTE-NEW
   
   \ new statement
   28 MALLOC DROP DUP DUP §base0 V! TO §this OVER
   IF
      OVER CELL+ @ 4 + OVER OVER ! DROP
   ENDIF
   CELL+ @ OVER
   OVER ! NIP
   lang.StringIndexOutOfBoundsException§-945597696.table OVER 12 + !
   -945618944 OVER 20 + !
   " StringIndexOutOfBoundsException " OVER 16 + !
   0 OVER 24 + ! DROP
   
   \ new statement
   §this DUP 0 V! ( return object )    
   \ new statement
   0 §break18587 LABEL
   
   \ new statement
   §base0 SETVTOP
   PURGE 2
   
   \ new statement
   DROP
;
( *
* Constructs a <code>StringIndexOutOfBoundsException</code> with
* the specified detail message.
*
* @param   s   the detail message.
)

:LOCAL lang.StringIndexOutOfBoundsException.StringIndexOutOfBoundsException§1858294528
   2 VALLOCATE LOCAL §base0
   DUP 4 §base0 + V! LOCAL s
   
   \ new statement
   0
   LOCALS §this |
   
   \ new statement
   s
   lang.IndexOutOfBoundsException§1771129088.table -360625920 EXECUTE-NEW
   
   \ new statement
   28 MALLOC DROP DUP DUP §base0 V! TO §this OVER
   IF
      OVER CELL+ @ 4 + OVER OVER ! DROP
   ENDIF
   CELL+ @ OVER
   OVER ! NIP
   lang.StringIndexOutOfBoundsException§-945597696.table OVER 12 + !
   -945618944 OVER 20 + !
   " StringIndexOutOfBoundsException " OVER 16 + !
   0 OVER 24 + ! DROP
   
   \ new statement
   §this DUP 0 V! ( return object )    
   \ new statement
   0 §break18588 LABEL
   
   \ new statement
   
   §base0 SETVTOP
   PURGE 3
   
   \ new statement
   DROP
;


: lang.StringIndexOutOfBoundsException.classMonitorEnter§-433167616
   
   \ new statement
   
   \ new statement
   0 §break18585 LABEL
   
   \ new statement
   
   \ new statement
   DROP
;


: lang.StringIndexOutOfBoundsException.classMonitorLeave§-1691393280
   
   \ new statement
   
   \ new statement
   0 §break18586 LABEL
   
   \ new statement
   
   \ new statement
   DROP
;
VARIABLE lang.StringIndexOutOfBoundsException._staticBlocking
VARIABLE lang.StringIndexOutOfBoundsException._staticThread

A:HERE VARIABLE lang.StringIndexOutOfBoundsException§-945597696.table 3 DUP 2* CELLS ALLOT R@ ! A:CELL+
1138905856 R@ ! A:CELL+ lang.StringIndexOutOfBoundsException.StringIndexOutOfBoundsException§1138905856 VAL R@ ! A:CELL+
1355436800 R@ ! A:CELL+ lang.StringIndexOutOfBoundsException.StringIndexOutOfBoundsException§1355436800 VAL R@ ! A:CELL+
1858294528 R@ ! A:CELL+ lang.StringIndexOutOfBoundsException.StringIndexOutOfBoundsException§1858294528 VAL R@ ! A:CELL+
A:DROP
