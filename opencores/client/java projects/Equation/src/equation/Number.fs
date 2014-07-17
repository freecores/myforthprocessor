MODULENAME equation.Number
(
* @(#)Number.java	1.28 03/01/23
*
* Copyright 2003 Sun Microsystems, Inc. All rights reserved.
* SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
)  ( *
* The abstract class <code>Number</code> is the superclass of classes
* <code>BigDecimal</code>, <code>BigInteger</code>,
* <code>Byte</code>, <code>Double</code>, <code>Float</code>,
* <code>Integer</code>, <code>Long</code>, and <code>Short</code>.
* <p>
* Subclasses of <code>Number</code> must provide methods to convert
* the represented numeric value to <code>byte</code>, <code>double</code>,
* <code>float</code>, <code>int</code>, <code>long</code>, and
* <code>short</code>.
*
* @author	Lee Boynton
* @author	Arthur van Hoff
* @version 1.28, 01/23/03
* @see     java.lang.Byte
* @see     java.lang.Double
* @see     java.lang.Float
* @see     java.lang.Integer
* @see     java.lang.Long
* @see     java.lang.Short
* @since   JDK1.0
)
( *
* Returns the value of the specified number as a <code>byte</code>.
* This may involve rounding or truncation.
*
* @return  the numeric value represented by this object after conversion
*          to type <code>byte</code>.
* @since   JDK1.1
)

:LOCAL Number.byteValue§-1994235392
   1 VALLOCATE LOCAL §base0
   DUP 0 §base0 + V! LOCAL §this
   
   \ new statement
   
   \ new statement
   §this >R
   R> -370906880 TRUE ( equation.Number.intValue§-370906880 ) EXECUTE-METHOD
   127 U>! NIP
   IF
      0FFFFFF00H OR
   ENDIF
   0 §break33089 BRANCH
   
   \ new statement
   0 §break33089 LABEL
   
   \ new statement
   §base0 SETVTOP
   PURGE 2
   
   \ new statement
   DROP
;
( *
* Returns the value of the specified number as a <code>short</code>.
* This may involve rounding or truncation.
*
* @return  the numeric value represented by this object after conversion
*          to type <code>short</code>.
* @since   JDK1.1
)

:LOCAL Number.shortValue§715813632
   1 VALLOCATE LOCAL §base0
   DUP 0 §base0 + V! LOCAL §this
   
   \ new statement
   
   \ new statement
   §this >R
   R> -370906880 TRUE ( equation.Number.intValue§-370906880 ) EXECUTE-METHOD
   32767 U>! NIP
   IF
      0FFFF0000H OR
   ENDIF
   0 §break33090 BRANCH
   
   \ new statement
   0 §break33090 LABEL
   
   \ new statement
   §base0 SETVTOP
   PURGE 2
   
   \ new statement
   DROP
;
2VARIABLE Number.serialVersionUID

A:HERE VARIABLE Number§-450605568.table 2 DUP 2* CELLS ALLOT R@ ! A:CELL+
-1994235392 R@ ! A:CELL+ Number.byteValue§-1994235392 VAL R@ ! A:CELL+
715813632 R@ ! A:CELL+ Number.shortValue§715813632 VAL R@ ! A:CELL+
A:DROP
