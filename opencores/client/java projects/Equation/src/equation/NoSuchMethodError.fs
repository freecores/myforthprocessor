MODULENAME equation.NoSuchMethodError
(
* @(#)NoSuchMethodError.java	1.20 03/01/23
*
* Copyright 2003 Sun Microsystems, Inc. All rights reserved.
* SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
)  ( *
* Thrown if an application tries to call a specified method of a
* class (either static or instance), and that class no longer has a
* definition of that method.
* <p>
* Normally, this error is caught by the compiler; this error can
* only occur at run time if the definition of a class has
* incompatibly changed.
*
* @author  unascribed
* @version 1.20, 01/23/03
* @since   JDK1.0
)
( *
* Constructs a <code>NoSuchMethodError</code> with the
* specified detail message.
*
* @param   s   the detail message.
)

:LOCAL NoSuchMethodError.NoSuchMethodError§-1876144640
   2 VALLOCATE LOCAL §base0
   DUP 4 §base0 + V! LOCAL s
   
   \ new statement
   0
   LOCALS §this |
   
   \ new statement
   s
   IncompatibleClassChangeError§97667328.table -1356183296 EXECUTE-NEW
   
   \ new statement
   28 MALLOC DROP DUP DUP §base0 V! TO §this OVER
   IF
      OVER CELL+ @ 4 + OVER OVER ! DROP
   ENDIF
   CELL+ @ OVER
   OVER ! NIP
   NoSuchMethodError§-405778944.table OVER 12 + !
   -405798912 OVER 20 + !
   " NoSuchMethodError " OVER 16 + !
   0 OVER 24 + ! DROP
   
   \ new statement
   §this DUP 0 V! ( return object )    
   \ new statement
   0 §break30146 LABEL
   
   \ new statement
   
   §base0 SETVTOP
   PURGE 3
   
   \ new statement
   DROP
;
( *
* Constructs a <code>NoSuchMethodError</code> with no detail message.
)

:LOCAL NoSuchMethodError.NoSuchMethodError§1581272576
   1 VALLOCATE LOCAL §base0
   
   \ new statement
   0
   LOCALS §this |
   
   \ new statement
   IncompatibleClassChangeError§97667328.table 290474240 EXECUTE-NEW
   
   \ new statement
   28 MALLOC DROP DUP DUP §base0 V! TO §this OVER
   IF
      OVER CELL+ @ 4 + OVER OVER ! DROP
   ENDIF
   CELL+ @ OVER
   OVER ! NIP
   NoSuchMethodError§-405778944.table OVER 12 + !
   -405798912 OVER 20 + !
   " NoSuchMethodError " OVER 16 + !
   0 OVER 24 + ! DROP
   
   \ new statement
   §this DUP 0 V! ( return object )    
   \ new statement
   0 §break30145 LABEL
   
   \ new statement
   §base0 SETVTOP
   PURGE 2
   
   \ new statement
   DROP
;
VARIABLE NoSuchMethodError._staticBlocking
VARIABLE NoSuchMethodError._staticThread  HERE 4 - SALLOCATE

A:HERE VARIABLE NoSuchMethodError§-405778944.table 2 DUP 2* CELLS ALLOT R@ ! A:CELL+
-1876144640 R@ ! A:CELL+ NoSuchMethodError.NoSuchMethodError§-1876144640 VAL R@ ! A:CELL+
1581272576 R@ ! A:CELL+ NoSuchMethodError.NoSuchMethodError§1581272576 VAL R@ ! A:CELL+
A:DROP
