MODULENAME java.lang.NumberFormatException
(
* @(#)NumberFormatException.java	1.19 03/01/23
*
* Copyright 2003 Sun Microsystems, Inc. All rights reserved.
* SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
)  ( *
* Thrown to indicate that the application has attempted to convert
* a string to one of the numeric types, but that the string does not
* have the appropriate format.
*
* @author  unascribed
* @version 1.19, 01/23/03
* @see     java.lang.Integer#toString()
* @since   JDK1.0
)
( *
* Constructs a <code>NumberFormatException</code> with the
* specified detail message.
*
* @param   s   the detail message.
)

:LOCAL lang.NumberFormatException.NumberFormatException§-1790489088
   2 VALLOCATE LOCAL §base0
   DUP 4 §base0 + V! LOCAL s
   
   \ new statement
   0
   LOCALS §this |
   
   \ new statement
   s
   lang.IllegalArgumentException§-946714368.table 737495296 EXECUTE-NEW
   
   \ new statement
   28 MALLOC DROP DUP DUP §base0 V! TO §this OVER
   IF
      OVER CELL+ @ 4 + OVER OVER ! DROP
   ENDIF
   CELL+ @ OVER
   OVER ! NIP
   lang.NumberFormatException§607604224.table OVER 12 + !
   607584256 OVER 20 + !
   " NumberFormatException " OVER 16 + !
   0 OVER 24 + ! DROP
   
   \ new statement
   §this DUP 0 V! ( return object )    
   \ new statement
   0 §break17026 LABEL
   
   \ new statement
   
   §base0 SETVTOP
   PURGE 3
   
   \ new statement
   DROP
;
( *
* Constructs a <code>NumberFormatException</code> with no detail message.
)

:LOCAL lang.NumberFormatException.NumberFormatException§1732201984
   1 VALLOCATE LOCAL §base0
   
   \ new statement
   0
   LOCALS §this |
   
   \ new statement
   lang.IllegalArgumentException§-946714368.table -564049664 EXECUTE-NEW
   
   \ new statement
   28 MALLOC DROP DUP DUP §base0 V! TO §this OVER
   IF
      OVER CELL+ @ 4 + OVER OVER ! DROP
   ENDIF
   CELL+ @ OVER
   OVER ! NIP
   lang.NumberFormatException§607604224.table OVER 12 + !
   607584256 OVER 20 + !
   " NumberFormatException " OVER 16 + !
   0 OVER 24 + ! DROP
   
   \ new statement
   §this DUP 0 V! ( return object )    
   \ new statement
   0 §break17025 LABEL
   
   \ new statement
   §base0 SETVTOP
   PURGE 2
   
   \ new statement
   DROP
;


: lang.NumberFormatException.classMonitorEnter§-433167616
   
   \ new statement
   
   \ new statement
   0 §break17023 LABEL
   
   \ new statement
   
   \ new statement
   DROP
;


: lang.NumberFormatException.classMonitorLeave§-1691393280
   
   \ new statement
   
   \ new statement
   0 §break17024 LABEL
   
   \ new statement
   
   \ new statement
   DROP
;
( *
* Factory method for making a <code>NumberFormatException</code>
* given the specified input which caused the error.
*
* @param   s   the input causing the error
)

: lang.NumberFormatException.forInputString§1916364288
   1 VALLOCATE LOCAL §base0
   DUP 0 §base0 + V! LOCAL s
   
   \ new statement
   
   \ new statement
   
   3 VALLOCATE LOCAL §tempvar
   U" For input string: \" " COUNT lang.JavaArray.createUnicode§-675323136 DUP §tempvar V!
   s
   OVER  -1461427456 TRUE ( java.lang.String.concat§-1461427456 ) EXECUTE-METHOD DUP §tempvar 4 + V!
   U" \" " COUNT lang.JavaArray.createUnicode§-675323136 DUP §tempvar 8 + V!
   OVER  -1461427456 TRUE ( java.lang.String.concat§-1461427456 ) EXECUTE-METHOD
   lang.NumberFormatException§607604224.table -1790489088 EXECUTE-NEW
   DUP 0 V!0 §break17027 BRANCH
   
   
   
   
   §tempvar SETVTOP
   
   PURGE 1
   
   
   \ new statement
   0 §break17027 LABEL
   
   \ new statement
   
   §base0 SETVTOP
   PURGE 2
   
   \ new statement
   DROP
;
VARIABLE lang.NumberFormatException._staticBlocking
VARIABLE lang.NumberFormatException._staticThread
2VARIABLE lang.NumberFormatException.serialVersionUID

A:HERE VARIABLE lang.NumberFormatException§607604224.table 2 DUP 2* CELLS ALLOT R@ ! A:CELL+
-1790489088 R@ ! A:CELL+ lang.NumberFormatException.NumberFormatException§-1790489088 VAL R@ ! A:CELL+
1732201984 R@ ! A:CELL+ lang.NumberFormatException.NumberFormatException§1732201984 VAL R@ ! A:CELL+
A:DROP
