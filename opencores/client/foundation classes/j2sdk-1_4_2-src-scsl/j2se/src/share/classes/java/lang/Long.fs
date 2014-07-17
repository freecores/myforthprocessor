MODULENAME java.lang.Long
(
* @(#)Long.java	1.66 03/01/23
*
* Copyright 2003 Sun Microsystems, Inc. All rights reserved.
* SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
)  ( *
* The <code>Long</code> class wraps a value of the primitive type
* <code>long</code> in an object. An object of type <code>Long</code>
* contains a single field whose type is <code>long</code>.
*
* <p>
*
* In addition, this class provides several methods for converting a
* <code>long</code> to a <code>String</code> and a
* <code>String</code> to a <code>long</code>, as well as other
* constants and methods useful when dealing with a <code>long</code>.
*
* @author  Lee Boynton
* @author  Arthur van Hoff
* @version 1.66, 01/23/03
* @since   JDK1.0
)
( *
* Constructs a newly allocated <code>Long</code> object that
* represents the specified <code>long</code> argument.
*
* @param   value   the value to be represented by the
*          <code>Long</code> object.
)

:LOCAL lang.Long.Long§-43430912
   1 VALLOCATE LOCAL §base0
   2LOCAL value
   
   \ new statement
   0
   LOCALS §this |
   
   \ new statement
   0
   
   \ new statement
   36 MALLOC DROP DUP DUP §base0 V! TO §this OVER
   IF
      OVER CELL+ @ 4 + OVER OVER ! DROP
   ENDIF
   CELL+ @ OVER
   OVER ! NIP
   lang.Long§714296320.table OVER 12 + !
   714276864 OVER 20 + !
   " Long " OVER 16 + !
   0 OVER 24 + ! DROP
   
   \ new statement
   ( *
   * The value of the <code>Long</code>.
   *
   * @serial
   )
   
   \ new statement
   value
   §this CELL+ @ 28 + ( java.lang.Long.value )    2!
   
   \ new statement
   §this DUP 0 V! ( return object )    
   \ new statement
   0 §break18643 LABEL
   
   \ new statement
   
   §base0 SETVTOP
   PURGE 3
   
   \ new statement
   DROP
;


:LOCAL lang.Long.Long§-564311040
   1 VALLOCATE LOCAL §base0
   
   \ new statement
   0
   LOCALS §this |
   
   \ new statement
   0
   
   \ new statement
   36 MALLOC DROP DUP DUP §base0 V! TO §this OVER
   IF
      OVER CELL+ @ 4 + OVER OVER ! DROP
   ENDIF
   CELL+ @ OVER
   OVER ! NIP
   lang.Long§714296320.table OVER 12 + !
   714276864 OVER 20 + !
   " Long " OVER 16 + !
   0 OVER 24 + ! DROP
   
   \ new statement
   ( *
   * The value of the <code>Long</code>.
   *
   * @serial
   )
   
   \ new statement
   §this DUP 0 V! ( return object )    
   \ new statement
   0 §break18657 LABEL
   
   \ new statement
   §base0 SETVTOP
   PURGE 2
   
   \ new statement
   DROP
;
( *
* Constructs a newly allocated <code>Long</code> object that
* represents the <code>long</code> value indicated by the
* <code>String</code> parameter. The string is converted to a
* <code>long</code> value in exactly the manner used by the
* <code>parseLong</code> method for radix 10.
*
* @param      s   the <code>String</code> to be converted to a
*		   <code>Long</code>.
* @exception  NumberFormatException  if the <code>String</code> does not
*               contain a parsable <code>long</code>.
* @see        java.lang.Long#parseLong(java.lang.String, int)
)

:LOCAL lang.Long.Long§685263872
   2 VALLOCATE LOCAL §base0
   LOCAL §exception
   DUP 4 §base0 + V! LOCAL s
   
   \ new statement
   0
   LOCALS §this |
   
   \ new statement
   0
   
   \ new statement
   36 MALLOC DROP DUP DUP §base0 V! TO §this OVER
   IF
      OVER CELL+ @ 4 + OVER OVER ! DROP
   ENDIF
   CELL+ @ OVER
   OVER ! NIP
   lang.Long§714296320.table OVER 12 + !
   714276864 OVER 20 + !
   " Long " OVER 16 + !
   0 OVER 24 + ! DROP
   
   \ new statement
   ( *
   * The value of the <code>Long</code>.
   *
   * @serial
   )
   
   \ new statement
   s
   10
   std19719 VAL
   lang.Long.parseLong§-1655803904
   §this CELL+ @ 28 + ( java.lang.Long.value )    2! FALSE DUP
   IF
      std19719 LABEL TRUE
   ENDIF
   IF
      §exception TO §return
      0 §break18644 BRANCH
   ENDIF
   
   \ new statement
   §this DUP 0 V! ( return object )    
   \ new statement
   0 §break18644 LABEL
   
   \ new statement
   
   
   §base0 SETVTOP
   PURGE 4
   
   \ new statement
   DROP
;


: lang.Long.appendTo§-1166319360
   2 VALLOCATE LOCAL §base0
   DUP 0 §base0 + V! LOCAL sb
   2LOCAL i
   
   \ new statement
   0 DUP DUP
   LOCALS body length s |
   
   \ new statement
   BASE 10 TO BASE
   
   \ new statement
   i 0 D..R TO length TO body TO BASE
   
   \ new statement
   length
   body
   lang.JavaArray.createString§-105880832
   DUP 4 §base0 + V! TO s
   
   \ new statement
   
   1 VALLOCATE LOCAL §tempvar
   sb A:R@
   s
   R> -310157056 TRUE ( java.lang.StringBuffer.append§-310157056 ) EXECUTE-METHOD DUP §tempvar V!
   DROP
   
   §tempvar SETVTOP
   
   PURGE 1
   
   
   \ new statement
   0 §break18629 LABEL
   
   \ new statement
   
   
   
   
   
   §base0 SETVTOP
   PURGE 6
   
   \ new statement
   DROP
;
( *
* Returns the value of this <code>Long</code> as a
* <code>byte</code>.
)

:LOCAL lang.Long.byteValue§-1994235392
   1 VALLOCATE LOCAL §base0
   DUP 0 §base0 + V! LOCAL §this
   
   \ new statement
   
   \ new statement
   §this CELL+ @ 28 + ( java.lang.Long.value )    2@ A:R@  NIP 127 U>! NIP
   IF
      0FFFFFF00H OR
   ENDIF
   A:DROP
   0 §break18645 BRANCH
   
   \ new statement
   0 §break18645 LABEL
   
   \ new statement
   §base0 SETVTOP
   PURGE 2
   
   \ new statement
   DROP
;


: lang.Long.classMonitorEnter§-433167616
   
   \ new statement
   
   \ new statement
   0 §break18621 LABEL
   
   \ new statement
   
   \ new statement
   DROP
;


: lang.Long.classMonitorLeave§-1691393280
   
   \ new statement
   
   \ new statement
   0 §break18622 LABEL
   
   \ new statement
   
   \ new statement
   DROP
;
( *
* Compares this <code>Long</code> object to another object.  If
* the object is a <code>Long</code>, this function behaves like
* <code>compareTo(Long)</code>.  Otherwise, it throws a
* <code>ClassCastException</code> (as <code>Long</code> objects
* are comparable only to other <code>Long</code> objects).
*
* @param   o the <code>Object</code> to be compared.
* @return  the value <code>0</code> if the argument is a
*		<code>Long</code> numerically equal to this
*		<code>Long</code>; a value less than <code>0</code>
*		if the argument is a <code>Long</code> numerically
*		greater than this <code>Long</code>; and a value
*		greater than <code>0</code> if the argument is a
*		<code>Long</code> numerically less than this
*		<code>Long</code>.
* @exception <code>ClassCastException</code> if the argument is not a
*		  <code>Long</code>.
* @see     java.lang.Comparable
* @since   1.2
)

:LOCAL lang.Long.compareTo§-879140096
   2 VALLOCATE LOCAL §base0
   DUP 0 §base0 + V! LOCAL o
   DUP 4 §base0 + V! LOCAL §this
   
   \ new statement
   
   \ new statement
   §this A:R@
   o
   R> 229532416 TRUE ( java.lang.Long.compareTo§229532416 ) EXECUTE-METHOD
   0 §break18656 BRANCH
   
   \ new statement
   0 §break18656 LABEL
   
   \ new statement
   
   §base0 SETVTOP
   PURGE 3
   
   \ new statement
   DROP
;
( *
* Determines the <code>long</code> value of the system property
* with the specified name.
* <p>
* The first argument is treated as the name of a system property.
* System properties are accessible through the {@link
* java.lang.System#getProperty(java.lang.String)} method. The
* string value of this property is then interpreted as a
* <code>long</code> value and a <code>Long</code> object
* representing this value is returned.  Details of possible
* numeric formats can be found with the definition of
* <code>getProperty</code>.
* <p>
* If there is no property with the specified name, if the
* specified name is empty or <code>null</code>, or if the
* property does not have the correct numeric format, then
* <code>null</code> is returned.
* <p>
* In other words, this method returns a <code>Long</code> object equal to
* the value of:
* <blockquote><code>
* getLong(nm, null)
* </code></blockquote>
*
* @param   nm   property name.
* @return  the <code>Long</code> value of the property.
* @see     java.lang.System#getProperty(java.lang.String)
* @see     java.lang.System#getProperty(java.lang.String, java.lang.String)
public static Long getLong(String nm) {
return getLong(nm, null);
}
)  ( *
* Determines the <code>long</code> value of the system property
* with the specified name.
* <p>
* The first argument is treated as the name of a system property.
* System properties are accessible through the {@link
* java.lang.System#getProperty(java.lang.String)} method. The
* string value of this property is then interpreted as a
* <code>long</code> value and a <code>Long</code> object
* representing this value is returned.  Details of possible
* numeric formats can be found with the definition of
* <code>getProperty</code>.
* <p>
* The second argument is the default value. A <code>Long</code> object
* that represents the value of the second argument is returned if there
* is no property of the specified name, if the property does not have
* the correct numeric format, or if the specified name is empty or null.
* <p>
* In other words, this method returns a <code>Long</code> object equal
* to the value of:
* <blockquote><code>
* getLong(nm, new Long(val))
* </code></blockquote>
* but in practice it may be implemented in a manner such as:
* <blockquote><pre>
* Long result = getLong(nm, null);
* return (result == null) ? new Long(val) : result;
* </pre></blockquote>
* to avoid the unnecessary allocation of a <code>Long</code> object when
* the default value is not needed.
*
* @param   nm    property name.
* @param   val   default value.
* @return  the <code>Long</code> value of the property.
* @see     java.lang.System#getProperty(java.lang.String)
* @see     java.lang.System#getProperty(java.lang.String, java.lang.String)
public static Long getLong(String nm, long val) {
Long result = Long.getLong(nm, null);
return (result == null) ? new Long(val) : result;
}
)  ( *
* Returns the <code>long</code> value of the system property with
* the specified name.  The first argument is treated as the name
* of a system property.  System properties are accessible through
* the {@link java.lang.System#getProperty(java.lang.String)}
* method. The string value of this property is then interpreted
* as a <code>long</code> value, as per the
* <code>Long.decode</code> method, and a <code>Long</code> object
* representing this value is returned.
* <p><ul>
* <li>If the property value begins with the two ASCII characters
* <code>0x</code> or the ASCII character <code>#</code>, not followed by
* a minus sign, then the rest of it is parsed as a hexadecimal integer
* exactly as for the method {@link #valueOf(java.lang.String, int)}
* with radix 16.
* <li>If the property value begins with the ASCII character
* <code>0</code> followed by another character, it is parsed as
* an octal integer exactly as by the method {@link
* #valueOf(java.lang.String, int)} with radix 8.
* <li>Otherwise the property value is parsed as a decimal
* integer exactly as by the method
* {@link #valueOf(java.lang.String, int)} with radix 10.
* </ul>
* <p>
* Note that, in every case, neither <code>L</code>
* (<code>'&#92;u004C'</code>) nor <code>l</code>
* (<code>'&#92;u006C'</code>) is permitted to appear at the end
* of the property value as a type indicator, as would be
* permitted in Java programming language source code.
* <p>
* The second argument is the default value. The default value is
* returned if there is no property of the specified name, if the
* property does not have the correct numeric format, or if the
* specified name is empty or <code>null</code>.
*
* @param   nm   property name.
* @param   val   default value.
* @return  the <code>Long</code> value of the property.
* @see     java.lang.System#getProperty(java.lang.String)
* @see java.lang.System#getProperty(java.lang.String, java.lang.String)
* @see java.lang.Long#decode
public static Long getLong(String nm, Long val) {
String v = null;
try {
v = System.getProperty(nm);
} catch (IllegalArgumentException e) {
} catch (NullPointerException e) {
}
if (v != null) {
try {
return Long.decode(v);
} catch (NumberFormatException e) {
}
}
return val;
}
)  ( *
* Compares two <code>Long</code> objects numerically.
*
* @param   anotherLong   the <code>Long</code> to be compared.
* @return	the value <code>0</code> if this <code>Long</code> is
* 		equal to the argument <code>Long</code>; a value less than
* 		<code>0</code> if this <code>Long</code> is numerically less
* 		than the argument <code>Long</code>; and a value greater
* 		than <code>0</code> if this <code>Long</code> is numerically
* 		 greater than the argument <code>Long</code> (signed
* 		 comparison).
* @since   1.2
)

:LOCAL lang.Long.compareTo§229532416
   2 VALLOCATE LOCAL §base0
   DUP 0 §base0 + V! LOCAL anotherLong
   DUP 4 §base0 + V! LOCAL §this
   
   \ new statement
   0 DUP 2DUP
   2LOCALS anotherVal thisVal |
   
   \ new statement
   §this CELL+ @ 28 + ( java.lang.Long.value )    2@
   TO thisVal
   
   \ new statement
   anotherLong CELL+ @ 28 + ( java.lang.Long.value )    2@
   TO anotherVal
   
   \ new statement
   thisVal
   anotherVal
   D<
   IF
      -1
      
   ELSE
      thisVal
      anotherVal
      D=
      IF
         0
         
      ELSE
         1
         
      ENDIF
      
   ENDIF
   0 §break18655 BRANCH
   
   \ new statement
   0 §break18655 LABEL
   
   \ new statement
   
   
   
   §base0 SETVTOP
   PURGE 5
   
   \ new statement
   DROP
;
( *
* Decodes a <code>String</code> into a <code>Long</code>.
* Accepts decimal, hexadecimal, and octal numbers given by the
* following grammar:
*
* <blockquote>
* <dl>
* <dt><i>DecodableString:</i>
* <dd><i>Sign<sub>opt</sub> DecimalNumeral</i>
* <dd><i>Sign<sub>opt</sub></i> <code>0x</code> <i>HexDigits</i>
* <dd><i>Sign<sub>opt</sub></i> <code>0X</code> <i>HexDigits</i>
* <dd><i>Sign<sub>opt</sub></i> <code>#</code> <i>HexDigits</i>
* <dd><i>Sign<sub>opt</sub></i> <code>0</code> <i>OctalDigits</i>
* <p>
* <dt><i>Sign:</i>
* <dd><code>-</code>
* </dl>
* </blockquote>
*
* <i>DecimalNumeral</i>, <i>HexDigits</i>, and <i>OctalDigits</i>
* are defined in <a href="http://java.sun.com/docs/books/jls/second_edition/html/lexical.doc.html#48282">&sect;3.10.1</a>
* of the <a href="http://java.sun.com/docs/books/jls/html/">Java
* Language Specification</a>.
* <p>
* The sequence of characters following an (optional) negative
* sign and/or radix specifier (&quot;<code>0x</code>&quot;,
* &quot;<code>0X</code>&quot;, &quot;<code>#</code>&quot;, or
* leading zero) is parsed as by the <code>Long.parseLong</code>
* method with the indicated radix (10, 16, or 8).  This sequence
* of characters must represent a positive value or a {@link
* NumberFormatException} will be thrown.  The result is negated
* if first character of the specified <code>String</code> is the
* minus sign.  No whitespace characters are permitted in the
* <code>String</code>.
*
* @param     nm the <code>String</code> to decode.
* @return    a <code>Long</code> object holding the <code>long</code>
*		  value represented by <code>nm</code>
* @exception NumberFormatException  if the <code>String</code> does not
*            contain a parsable <code>long</code>.
* @see java.lang.Long#parseLong(String, int)
* @since 1.2
)

: lang.Long.decode§-644127744
   2 VALLOCATE LOCAL §base0
   LOCAL §exception
   DUP 0 §base0 + V! LOCAL nm
   
   \ new statement
   0 DUP DUP DUP
   LOCALS index negative radix result |
   
   \ new statement
   10
   TO radix
   
   \ new statement
   0
   TO index
   
   \ new statement
   FALSE
   TO negative
   
   \ new statement
   \  Handle minus sign, if present
   
   
   \ new statement
   
   1 VALLOCATE LOCAL §tempvar
   nm A:R@
   U" - " COUNT lang.JavaArray.createUnicode§-675323136 DUP §tempvar V!
   R> 970093312 TRUE ( java.lang.String.startsWith§970093312 ) EXECUTE-METHOD
   
   
   §tempvar SETVTOP
   
   PURGE 1
   
   
   \ new statement
   IF
      
      \ new statement
      
      \ new statement
      TRUE
      TO negative
      
      \ new statement
      index 1+ TO index
      
      \ new statement
      0 §break18635 LABEL
      
      \ new statement
      
      \ new statement
      DROP
      
      \ new statement
   ENDIF
   
   \ new statement
   \  Handle radix specifier, if present
   
   
   \ new statement
   
   2 VALLOCATE LOCAL §tempvar
   nm A:R@
   U" 0x " COUNT lang.JavaArray.createUnicode§-675323136 DUP §tempvar V!
   index
   R> -953322752 TRUE ( java.lang.String.startsWith§-953322752 ) EXECUTE-METHOD
   0=! or_19773 0BRANCH DROP
   nm A:R@
   U" 0X " COUNT lang.JavaArray.createUnicode§-675323136 DUP §tempvar 4 + V!
   index
   R> -953322752 TRUE ( java.lang.String.startsWith§-953322752 ) EXECUTE-METHOD
   or_19773 LABEL
   
   
   
   §tempvar SETVTOP
   
   PURGE 1
   
   
   \ new statement
   IF
      
      \ new statement
      
      \ new statement
      index
      2
      +
      TO index
      
      \ new statement
      16
      TO radix
      
      \ new statement
      0 §break18636 LABEL
      
      \ new statement
      
      \ new statement
      DROP
      
      \ new statement
   ELSE
      
      \ new statement
      
      1 VALLOCATE LOCAL §tempvar
      nm A:R@
      U" # " COUNT lang.JavaArray.createUnicode§-675323136 DUP §tempvar V!
      index
      R> -953322752 TRUE ( java.lang.String.startsWith§-953322752 ) EXECUTE-METHOD
      
      
      §tempvar SETVTOP
      
      PURGE 1
      
      
      \ new statement
      IF
         
         \ new statement
         
         \ new statement
         index 1+ TO index
         
         \ new statement
         16
         TO radix
         
         \ new statement
         0 §break18637 LABEL
         
         \ new statement
         
         \ new statement
         DROP
         
         \ new statement
      ELSE
         
         \ new statement
         
         1 VALLOCATE LOCAL §tempvar
         nm A:R@
         U" 0 " COUNT lang.JavaArray.createUnicode§-675323136 DUP §tempvar V!
         index
         R> -953322752 TRUE ( java.lang.String.startsWith§-953322752 ) EXECUTE-METHOD
         and_19792 0BRANCH! DROP
         nm A:R@
         R> 188050432 TRUE ( java.lang.String.length§188050432 ) EXECUTE-METHOD
         1
         index
         +
         >
         and_19792 LABEL
         
         
         §tempvar SETVTOP
         
         PURGE 1
         
         
         \ new statement
         IF
            
            \ new statement
            
            \ new statement
            index 1+ TO index
            
            \ new statement
            8
            TO radix
            
            \ new statement
            0 §break18638 LABEL
            
            \ new statement
            
            \ new statement
            DROP
            
            \ new statement
         ENDIF
         
         \ new statement
      ENDIF
      
      \ new statement
   ENDIF
   
   \ new statement
   
   1 VALLOCATE LOCAL §tempvar
   nm A:R@
   U" - " COUNT lang.JavaArray.createUnicode§-675323136 DUP §tempvar V!
   index
   R> -953322752 TRUE ( java.lang.String.startsWith§-953322752 ) EXECUTE-METHOD
   
   
   §tempvar SETVTOP
   
   PURGE 1
   
   
   \ new statement
   IF
      
      \ new statement
      
      1 VALLOCATE LOCAL §tempvar
      U" Negative sign in wrong position " COUNT lang.JavaArray.createUnicode§-675323136 DUP §tempvar V!
      lang.NumberFormatException§607604224.table -1790489088 EXECUTE-NEW
      §exception TO §return
      
      
      §tempvar SETVTOP
      
      PURGE 1
      
      
      \ new statement
      0 §break18634 BRANCH
      
      \ new statement
   ENDIF
   
   \ new statement
   
   \ new statement
   
   1 VALLOCATE LOCAL §tempvar
   nm A:R@
   index
   R> 18576128 TRUE ( java.lang.String.substring§18576128 ) EXECUTE-METHOD DUP §tempvar V!
   radix
   std19809 VAL
   lang.Long.valueOf§119436800
   DUP 4 §base0 + V! TO result FALSE DUP
   IF
      std19809 LABEL TRUE
   ENDIF
   
   §tempvar SETVTOP
   
   PURGE 1
   
   IF
      0 §break18639 BRANCH
   ENDIF
   
   \ new statement
   negative
   IF
      result A:R@
      R> 1506831360 TRUE ( java.lang.Long.longValue§1506831360 ) EXECUTE-METHOD
      DNEGATE
      lang.Long§714296320.table -43430912 EXECUTE-NEW
      
   ELSE
      result
      
   ENDIF
   DUP 4 §base0 + V! TO result
   
   \ new statement
   0 §except18640 LABEL
   
   \ new statement
   0 §break18639 LABEL
   
   \ new statement
   
   \ new statement
   0>!
   IF
      10001H - §break18634 BRANCH
   ENDIF
   DROP
   
   \ new statement
   DUP " NumberFormatException " INSTANCEOF
   IF
      
      \ new statement
      2 VALLOCATE LOCAL §base1
      0 DUP
      LOCALS constant e |
      
      \ new statement
      TO e
      
      \ new statement
      \  If number is Long.MIN_VALUE, we'll end up here. The next line
      \  handles this case, and causes any genuine format error to be
      \  rethrown.
      
      
      \ new statement
      
      2 VALLOCATE LOCAL §tempvar
      negative
      IF
         U" - " COUNT lang.JavaArray.createUnicode§-675323136 DUP §tempvar V!
         nm A:R@
         index
         R> 18576128 TRUE ( java.lang.String.substring§18576128 ) EXECUTE-METHOD DUP §tempvar 4 + V!
         OVER  -1461427456 TRUE ( java.lang.String.concat§-1461427456 ) EXECUTE-METHOD
         lang.String§1651856128.table 1844794112 EXECUTE-NEW
         
      ELSE
         nm A:R@
         index
         R> 18576128 TRUE ( java.lang.String.substring§18576128 ) EXECUTE-METHOD
         
      ENDIF
      DUP 4 §base1 + V! TO constant
      
      
      §tempvar SETVTOP
      
      PURGE 1
      
      
      \ new statement
      constant
      radix
      std19820 VAL
      lang.Long.valueOf§119436800
      DUP 4 §base0 + V! TO result FALSE DUP
      IF
         std19820 LABEL TRUE
      ENDIF
      IF
         §exception TO §return
         65537 §break18642 BRANCH
      ENDIF
      
      \ new statement
      0 §break18642 LABEL
      
      \ new statement
      
      
      §base1 SETVTOP
      PURGE 3
      
      \ new statement
      0>!
      IF
         10001H - §break18634 BRANCH
      ENDIF
      DROP
      
      \ new statement
      DUP §out18641 BRANCH
   ENDIF
   
   \ new statement
   §out18641 LABEL DROP
   
   \ new statement
   result
   DUP 0 V!0 §break18634 BRANCH
   
   \ new statement
   0 §break18634 LABEL
   
   \ new statement
   
   
   
   
   
   
   §base0 SETVTOP
   PURGE 7
   
   \ new statement
   DROP
;
( *
* Returns the value of this <code>Long</code> as a
* <code>double</code>.
)

:LOCAL lang.Long.doubleValue§-330275840
   1 VALLOCATE LOCAL §base0
   DUP 0 §base0 + V! LOCAL §this
   
   \ new statement
   
   \ new statement
   §this CELL+ @ 28 + ( java.lang.Long.value )    2@ A:R@  D>F
   A:DROP
   0 §break18650 BRANCH
   
   \ new statement
   0 §break18650 LABEL
   
   \ new statement
   §base0 SETVTOP
   PURGE 2
   
   \ new statement
   DROP
;
( *
* Compares this object to the specified object.  The result is
* <code>true</code> if and only if the argument is not
* <code>null</code> and is a <code>Long</code> object that
* contains the same <code>long</code> value as this object.
*
* @param   obj   the object to compare with.
* @return  <code>true</code> if the objects are the same;
*          <code>false</code> otherwise.
)

:LOCAL lang.Long.equals§-240098048
   2 VALLOCATE LOCAL §base0
   DUP 0 §base0 + V! LOCAL obj
   DUP 4 §base0 + V! LOCAL §this
   
   \ new statement
   
   \ new statement
   obj
   " Long " INSTANCEOF
   
   \ new statement
   IF
      
      \ new statement
      
      \ new statement
      §this CELL+ @ 28 + ( java.lang.Long.value )       2@
      obj
      A:R@
      R> 1506831360 TRUE ( java.lang.Long.longValue§1506831360 ) EXECUTE-METHOD
      D=
      65537 §break18654 BRANCH
      
      \ new statement
      0 §break18654 LABEL
      
      \ new statement
      
      \ new statement
      0>!
      IF
         10001H - §break18653 BRANCH
      ENDIF
      DROP
      
      \ new statement
   ENDIF
   
   \ new statement
   FALSE
   0 §break18653 BRANCH
   
   \ new statement
   0 §break18653 LABEL
   
   \ new statement
   
   §base0 SETVTOP
   PURGE 3
   
   \ new statement
   DROP
;
( *
* Returns the value of this <code>Long</code> as a
* <code>float</code>.
)

:LOCAL lang.Long.floatValue§1416259072
   1 VALLOCATE LOCAL §base0
   DUP 0 §base0 + V! LOCAL §this
   
   \ new statement
   
   \ new statement
   §this CELL+ @ 28 + ( java.lang.Long.value )    2@ A:R@  D>F
   A:DROP
   0 §break18649 BRANCH
   
   \ new statement
   0 §break18649 LABEL
   
   \ new statement
   §base0 SETVTOP
   PURGE 2
   
   \ new statement
   DROP
;
( *
* Returns a hash code for this <code>Long</code>. The result is
* the exclusive OR of the two halves of the primitive
* <code>long</code> value held by this <code>Long</code>
* object. That is, the hashcode is the value of the expression:
* <blockquote><pre>
* (int)(this.longValue()^(this.longValue()&gt;&gt;&gt;32))
* </pre></blockquote>
*
* @return  a hash code value for this object.
)

:LOCAL lang.Long.hashCode§-1604556800
   1 VALLOCATE LOCAL §base0
   DUP 0 §base0 + V! LOCAL §this
   
   \ new statement
   
   \ new statement
   §this CELL+ @ 28 + ( java.lang.Long.value )    2@
   §this CELL+ @ 28 + ( java.lang.Long.value )    2@
   32
   SHIFTR
   A:R> A:R1@ NIP XOR A:XOR R>
   D>S
   0 §break18652 BRANCH
   
   \ new statement
   0 §break18652 LABEL
   
   \ new statement
   §base0 SETVTOP
   PURGE 2
   
   \ new statement
   DROP
;
( *
* Returns the value of this <code>Long</code> as an
* <code>int</code>.
)

:LOCAL lang.Long.intValue§-370906880
   1 VALLOCATE LOCAL §base0
   DUP 0 §base0 + V! LOCAL §this
   
   \ new statement
   
   \ new statement
   §this CELL+ @ 28 + ( java.lang.Long.value )    2@ A:R@  D>S
   A:DROP
   0 §break18647 BRANCH
   
   \ new statement
   0 §break18647 LABEL
   
   \ new statement
   §base0 SETVTOP
   PURGE 2
   
   \ new statement
   DROP
;
( *
* Returns the value of this <code>Long</code> as a
* <code>long</code> value.
)

:LOCAL lang.Long.longValue§1506831360
   1 VALLOCATE LOCAL §base0
   DUP 0 §base0 + V! LOCAL §this
   
   \ new statement
   
   \ new statement
   §this CELL+ @ 28 + ( java.lang.Long.value )    2@ A:R@
   A:DROP
   0 §break18648 BRANCH
   
   \ new statement
   0 §break18648 LABEL
   
   \ new statement
   §base0 SETVTOP
   PURGE 2
   
   \ new statement
   DROP
;
( *
* Parses the string argument as a signed <code>long</code> in the
* radix specified by the second argument. The characters in the
* string must all be digits of the specified radix (as determined
* by whether {@link java.lang.Character#digit(char, int)} returns
* a nonnegative value), except that the first character may be an
* ASCII minus sign <code>'-'</code> (<code>'&#92;u002D'</code>) to
* indicate a negative value. The resulting <code>long</code>
* value is returned.
* <p>
* Note that neither the character <code>L</code>
* (<code>'&#92;u004C'</code>) nor <code>l</code>
* (<code>'&#92;u006C'</code>) is permitted to appear at the end
* of the string as a type indicator, as would be permitted in
* Java programming language source code - except that either
* <code>L</code> or <code>l</code> may appear as a digit for a
* radix greater than 22.
* <p>
* An exception of type <code>NumberFormatException</code> is
* thrown if any of the following situations occurs:
* <ul>
* <li>The first argument is <code>null</code> or is a string of
* length zero.
* <li>The <code>radix</code> is either smaller than {@link
* java.lang.Character#MIN_RADIX} or larger than {@link
* java.lang.Character#MAX_RADIX}.
* <li>Any character of the string is not a digit of the specified
* radix, except that the first character may be a minus sign
* <code>'-'</code> (<code>'&#92;u002d'</code>) provided that the
* string is longer than length 1.
* <li>The value represented by the string is not a value of type
*      <code>long</code>.
* </ul><p>
* Examples:
* <blockquote><pre>
* parseLong("0", 10) returns 0L
* parseLong("473", 10) returns 473L
* parseLong("-0", 10) returns 0L
* parseLong("-FF", 16) returns -255L
* parseLong("1100110", 2) returns 102L
* parseLong("99", 8) throws a NumberFormatException
* parseLong("Hazelnut", 10) throws a NumberFormatException
* parseLong("Hazelnut", 36) returns 1356099454469L
* </pre></blockquote>
*
* @param      s       the <code>String</code> containing the
*                     <code>long</code> representation to be parsed.
* @param      radix   the radix to be used while parsing <code>s</code>.
* @return     the <code>long</code> represented by the string argument in
*             the specified radix.
* @exception  NumberFormatException  if the string does not contain a
*               parsable <code>long</code>.
)

: lang.Long.parseLong§-1655803904
   1 VALLOCATE LOCAL §base0
   LOCAL §exception
   LOCAL radix
   DUP 0 §base0 + V! LOCAL s
   
   \ new statement
   0 DUP
   LOCALS body length |
   
   \ new statement
   
   1 VALLOCATE LOCAL §tempvar
   s A:R@
   R> -1067748352 TRUE ( java.lang.String.trim§-1067748352 ) EXECUTE-METHOD DUP §tempvar V!
   DROP
   
   §tempvar SETVTOP
   
   PURGE 1
   
   
   \ new statement
   s A:R@
   R> 188050432 TRUE ( java.lang.String.length§188050432 ) EXECUTE-METHOD
   TO length
   
   \ new statement
   s CELL+ @ 28 + ( java.lang.String.value )    @ A:R@
   A:DROP
   A:R@
   0
   R> -942971136 TRUE ( java.lang.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
   TO body
   
   \ new statement
   BASE radix TO BASE
   
   \ new statement
   0 DUP body length >NUMBER 2DROP
   
   \ new statement
   ROT TO BASE
   
   \ new statement
   0 §break18630 LABEL
   
   \ new statement
   
   
   
   
   
   §base0 SETVTOP
   PURGE 6
   
   \ new statement
   DROP
;
( *
* Parses the string argument as a signed decimal
* <code>long</code>.  The characters in the string must all be
* decimal digits, except that the first character may be an ASCII
* minus sign <code>'-'</code> (<code>&#92;u002D'</code>) to
* indicate a negative value. The resulting <code>long</code>
* value is returned, exactly as if the argument and the radix
* <code>10</code> were given as arguments to the {@link
* #parseLong(java.lang.String, int)} method.
* <p>
* Note that neither the character <code>L</code>
* (<code>'&#92;u004C'</code>) nor <code>l</code>
* (<code>'&#92;u006C'</code>) is permitted to appear at the end
* of the string as a type indicator, as would be permitted in
* Java programming language source code.
*
* @param      s   a <code>String</code> containing the <code>long</code>
*             representation to be parsed
* @return     the <code>long</code> represented by the argument in
*		   decimal.
* @exception  NumberFormatException  if the string does not contain a
*               parsable <code>long</code>.
)

: lang.Long.parseLong§-882348032
   1 VALLOCATE LOCAL §base0
   LOCAL §exception
   DUP 0 §base0 + V! LOCAL s
   
   \ new statement
   
   \ new statement
   s
   10
   std19879 VAL
   lang.Long.parseLong§-1655803904
   0 §break18631 BRANCH
   FALSE DUP
   IF
      std19879 LABEL TRUE
   ENDIF
   IF
      §exception TO §return
      0 §break18631 BRANCH
   ENDIF
   
   \ new statement
   0 §break18631 LABEL
   
   \ new statement
   
   
   §base0 SETVTOP
   PURGE 3
   
   \ new statement
   DROP
;
( *
* Returns the value of this <code>Long</code> as a
* <code>short</code>.
)

:LOCAL lang.Long.shortValue§715813632
   1 VALLOCATE LOCAL §base0
   DUP 0 §base0 + V! LOCAL §this
   
   \ new statement
   
   \ new statement
   §this CELL+ @ 28 + ( java.lang.Long.value )    2@ A:R@  NIP 32767 U>! NIP
   IF
      0FFFF0000H OR
   ENDIF
   A:DROP
   0 §break18646 BRANCH
   
   \ new statement
   0 §break18646 LABEL
   
   \ new statement
   §base0 SETVTOP
   PURGE 2
   
   \ new statement
   DROP
;
( *
* Returns a string representation of the <code>long</code>
* argument as an unsigned integer in base&nbsp;2.
* <p>
* The unsigned <code>long</code> value is the argument plus
* 2<sup>64</sup> if the argument is negative; otherwise, it is
* equal to the argument.  This value is converted to a string of
* ASCII digits in binary (base&nbsp;2) with no extra leading
* <code>0</code>s.  If the unsigned magnitude is zero, it is
* represented by a single zero character <code>'0'</code>
* (<code>'&#92;u0030'</code>); otherwise, the first character of
* the representation of the unsigned magnitude will not be the
* zero character. The characters <code>'0'</code>
* (<code>'&#92;u0030'</code>) and <code>'1'</code>
* (<code>'&#92;u0031'</code>) are used as binary digits.
*
* @param   i   a <code>long</code> to be converted to a string.
* @return  the string representation of the unsigned <code>long</code>
*          value represented by the argument in binary (base&nbsp;2).
* @since   JDK 1.0.2
)

: lang.Long.toBinaryString§-2094959616
   2LOCAL i
   
   \ new statement
   
   \ new statement
   i
   1
   lang.Long.toUnsignedString§-1694010368
   DUP 0 V!0 §break18626 BRANCH
   
   \ new statement
   0 §break18626 LABEL
   
   \ new statement
   
   PURGE 1
   
   \ new statement
   DROP
;
( *
* Returns a string representation of the <code>long</code>
* argument as an unsigned integer in base&nbsp;16.
* <p>
* The unsigned <code>long</code> value is the argument plus
* 2<sup>64</sup> if the argument is negative; otherwise, it is
* equal to the argument.  This value is converted to a string of
* ASCII digits in hexadecimal (base&nbsp;16) with no extra
* leading <code>0</code>s.  If the unsigned magnitude is zero, it
* is represented by a single zero character <code>'0'</code>
* (<code>'&#92;u0030'</code>); otherwise, the first character of
* the representation of the unsigned magnitude will not be the
* zero character. The following characters are used as
* hexadecimal digits:
* <blockquote><pre>
* 0123456789abcdef
* </pre></blockquote>
* These are the characters <code>'&#92;u0030'</code> through
* <code>'&#92;u0039'</code> and  <code>'&#92;u0061'</code> through
* <code>'&#92;u0066'</code>.  If uppercase letters are desired,
* the {@link java.lang.String#toUpperCase()} method may be called
* on the result:
* <blockquote><pre>
* Long.toHexString(n).toUpperCase()
* </pre></blockquote>
*
* @param   i   a <code>long</code> to be converted to a string.
* @return  the string representation of the unsigned <code>long</code>
* 		value represented by the argument in hexadecimal
*		(base&nbsp;16).
* @since   JDK 1.0.2
)

: lang.Long.toHexString§-753568768
   2LOCAL i
   
   \ new statement
   
   \ new statement
   i
   4
   lang.Long.toUnsignedString§-1694010368
   DUP 0 V!0 §break18624 BRANCH
   
   \ new statement
   0 §break18624 LABEL
   
   \ new statement
   
   PURGE 1
   
   \ new statement
   DROP
;
( *
* Returns a string representation of the <code>long</code>
* argument as an unsigned integer in base&nbsp;8.
* <p>
* The unsigned <code>long</code> value is the argument plus
* 2<sup>64</sup> if the argument is negative; otherwise, it is
* equal to the argument.  This value is converted to a string of
* ASCII digits in octal (base&nbsp;8) with no extra leading
* <code>0</code>s.
* <p>
* If the unsigned magnitude is zero, it is represented by a
* single zero character <code>'0'</code>
* (<code>'&#92;u0030'</code>); otherwise, the first character of
* the representation of the unsigned magnitude will not be the
* zero character. The following characters are used as octal
* digits:
* <blockquote><pre>
* 01234567
* </pre></blockquote>
* These are the characters <code>'&#92;u0030'</code> through
* <code>'&#92;u0037'</code>.
*
* @param   i   a <code>long</code> to be converted to a string.
* @return  the string representation of the unsigned <code>long</code>
*		value represented by the argument in octal (base&nbsp;8).
* @since   JDK 1.0.2
)

: lang.Long.toOctalString§1960932352
   2LOCAL i
   
   \ new statement
   
   \ new statement
   i
   3
   lang.Long.toUnsignedString§-1694010368
   DUP 0 V!0 §break18625 BRANCH
   
   \ new statement
   0 §break18625 LABEL
   
   \ new statement
   
   PURGE 1
   
   \ new statement
   DROP
;
( *
* Returns a string representation of the first argument in the
* radix specified by the second argument.
* <p>
* If the radix is smaller than <code>Character.MIN_RADIX</code>
* or larger than <code>Character.MAX_RADIX</code>, then the radix
* <code>10</code> is used instead.
* <p>
* If the first argument is negative, the first element of the
* result is the ASCII minus sign <code>'-'</code>
* (<code>'&#92;u002d'</code>). If the first argument is not
* negative, no sign character appears in the result.
* <p>
* The remaining characters of the result represent the magnitude
* of the first argument. If the magnitude is zero, it is
* represented by a single zero character <code>'0'</code>
* (<code>'&#92;u0030'</code>); otherwise, the first character of
* the representation of the magnitude will not be the zero
* character.  The following ASCII characters are used as digits:
* <blockquote><pre>
*   0123456789abcdefghijklmnopqrstuvwxyz
* </pre></blockquote>
* These are <code>'&#92;u0030'</code> through
* <code>'&#92;u0039'</code> and <code>'&#92;u0061'</code> through
* <code>'&#92;u007a'</code>. If <code>radix</code> is
* <var>N</var>, then the first <var>N</var> of these characters
* are used as radix-<var>N</var> digits in the order shown. Thus,
* the digits for hexadecimal (radix 16) are
* <code>0123456789abcdef</code>. If uppercase letters are
* desired, the {@link java.lang.String#toUpperCase()} method may
* be called on the result:
* <blockquote><pre>
* Long.toString(n, 16).toUpperCase()
* </pre></blockquote>
*
* @param   i       a <code>long</code>to be converted to a string.
* @param   radix   the radix to use in the string representation.
* @return  a string representation of the argument in the specified radix.
* @see     java.lang.Character#MAX_RADIX
* @see     java.lang.Character#MIN_RADIX
)

: lang.Long.toString§-1101892608
   LOCAL radix
   2LOCAL i
   
   \ new statement
   0 DUP
   LOCALS body length |
   
   \ new statement
   BASE radix TO BASE
   
   \ new statement
   i 0 D..R TO length TO body TO BASE
   
   \ new statement
   length
   body
   lang.JavaArray.createString§-105880832
   DUP 0 V!0 §break18623 BRANCH
   
   \ new statement
   0 §break18623 LABEL
   
   \ new statement
   
   
   
   
   PURGE 4
   
   \ new statement
   DROP
;
( *
* Returns a <code>String</code> object representing the specified
* <code>long</code>.  The argument is converted to signed decimal
* representation and returned as a string, exactly as if the
* argument and the radix 10 were given as arguments to the {@link
* #toString(long, int)} method.
*
* @param   i   a <code>long</code> to be converted.
* @return  a string representation of the argument in base&nbsp;10.
)

: lang.Long.toString§-1270713344
   2LOCAL i
   
   \ new statement
   0 DUP
   LOCALS body length |
   
   \ new statement
   BASE 10 TO BASE
   
   \ new statement
   i 0 D..R TO length TO body TO BASE
   
   \ new statement
   length
   body
   lang.JavaArray.createString§-105880832
   DUP 0 V!0 §break18628 BRANCH
   
   \ new statement
   0 §break18628 LABEL
   
   \ new statement
   
   
   
   PURGE 3
   
   \ new statement
   DROP
;
( *
* Returns a <code>String</code> object representing this
* <code>Long</code>'s value.  The value is converted to signed
* decimal representation and returned as a string, exactly as if
* the <code>long</code> value were given as an argument to the
* {@link java.lang.Long#toString(long)} method.
*
* @return  a string representation of the value of this object in
*		base&nbsp;10.
)

:LOCAL lang.Long.toString§1621718016
   1 VALLOCATE LOCAL §base0
   DUP 0 §base0 + V! LOCAL §this
   
   \ new statement
   
   \ new statement
   §this CELL+ @ 28 + ( java.lang.Long.value )    2@
   lang.String.valueOf§-85035520
   DUP 0 V!0 §break18651 BRANCH
   
   \ new statement
   0 §break18651 LABEL
   
   \ new statement
   §base0 SETVTOP
   PURGE 2
   
   \ new statement
   DROP
;
( *
* Convert the integer to an unsigned number.
)

: lang.Long.toUnsignedString§-1694010368
   LOCAL shift
   2LOCAL i
   
   \ new statement
   0 DUP DUP
   LOCALS body length radix |
   
   \ new statement
   1 S>D
   shift
   SHIFTL  D>S
   TO radix
   
   \ new statement
   BASE radix TO BASE
   
   \ new statement
   i 0 UD..R TO length TO body TO BASE
   
   \ new statement
   length
   body
   lang.JavaArray.createString§-105880832
   DUP 0 V!0 §break18627 BRANCH
   
   \ new statement
   0 §break18627 LABEL
   
   \ new statement
   
   
   
   
   
   PURGE 5
   
   \ new statement
   DROP
;
( *
* Returns a <code>Long</code> object holding the value
* of the specified <code>String</code>. The argument is
* interpreted as representing a signed decimal <code>long</code>,
* exactly as if the argument were given to the {@link
* #parseLong(java.lang.String)} method. The result is a
* <code>Long</code> object that represents the integer value
* specified by the string.
* <p>
* In other words, this method returns a <code>Long</code> object
* equal to the value of:
*
* <blockquote><pre>
* new Long(Long.parseLong(s))
* </pre></blockquote>
*
* @param      s   the string to be parsed.
* @return     a <code>Long</code> object holding the value
*             represented by the string argument.
* @exception  NumberFormatException  If the string cannot be parsed
*              as a <code>long</code>.
)

: lang.Long.valueOf§-170166784
   1 VALLOCATE LOCAL §base0
   LOCAL §exception
   DUP 0 §base0 + V! LOCAL s
   
   \ new statement
   
   \ new statement
   s
   10
   std19931 VAL
   lang.Long.parseLong§-1655803904
   lang.Long§714296320.table -43430912 EXECUTE-NEW
   DUP 0 V!0 §break18633 BRANCH
   FALSE DUP
   IF
      std19931 LABEL TRUE
   ENDIF
   IF
      §exception TO §return
      0 §break18633 BRANCH
   ENDIF
   
   \ new statement
   0 §break18633 LABEL
   
   \ new statement
   
   
   §base0 SETVTOP
   PURGE 3
   
   \ new statement
   DROP
;
( *
* Returns a <code>Long</code> object holding the value
* extracted from the specified <code>String</code> when parsed
* with the radix given by the second argument.  The first
* argument is interpreted as representing a signed
* <code>long</code> in the radix specified by the second
* argument, exactly as if the arguments were given to the {@link
* #parseLong(java.lang.String, int)} method. The result is a
* <code>Long</code> object that represents the <code>long</code>
* value specified by the string.
* <p>
* In other words, this method returns a <code>Long</code> object equal
* to the value of:
*
* <blockquote><code>
* new Long(Long.parseLong(s, radix))
* </code></blockquote>
*
* @param      s       the string to be parsed
* @param      radix   the radix to be used in interpreting <code>s</code>
* @return     a <code>Long</code> object holding the value
*             represented by the string argument in the specified
*             radix.
* @exception  NumberFormatException  If the <code>String</code> does not
*             contain a parsable <code>long</code>.
)

: lang.Long.valueOf§119436800
   1 VALLOCATE LOCAL §base0
   LOCAL §exception
   LOCAL radix
   DUP 0 §base0 + V! LOCAL s
   
   \ new statement
   
   \ new statement
   s
   radix
   std19936 VAL
   lang.Long.parseLong§-1655803904
   lang.Long§714296320.table -43430912 EXECUTE-NEW
   DUP 0 V!0 §break18632 BRANCH
   FALSE DUP
   IF
      std19936 LABEL TRUE
   ENDIF
   IF
      §exception TO §return
      0 §break18632 BRANCH
   ENDIF
   
   \ new statement
   0 §break18632 LABEL
   
   \ new statement
   
   
   
   §base0 SETVTOP
   PURGE 4
   
   \ new statement
   DROP
;
2VARIABLE lang.Long.MAX_VALUE
2VARIABLE lang.Long.MIN_VALUE
VARIABLE lang.Long._staticBlocking
VARIABLE lang.Long._staticThread
2VARIABLE lang.Long.serialVersionUID

A:HERE VARIABLE lang.Long§714296320.table 14 DUP 2* CELLS ALLOT R@ ! A:CELL+
-43430912 R@ ! A:CELL+ lang.Long.Long§-43430912 VAL R@ ! A:CELL+
-564311040 R@ ! A:CELL+ lang.Long.Long§-564311040 VAL R@ ! A:CELL+
685263872 R@ ! A:CELL+ lang.Long.Long§685263872 VAL R@ ! A:CELL+
-1994235392 R@ ! A:CELL+ lang.Long.byteValue§-1994235392 VAL R@ ! A:CELL+
-879140096 R@ ! A:CELL+ lang.Long.compareTo§-879140096 VAL R@ ! A:CELL+
229532416 R@ ! A:CELL+ lang.Long.compareTo§229532416 VAL R@ ! A:CELL+
-330275840 R@ ! A:CELL+ lang.Long.doubleValue§-330275840 VAL R@ ! A:CELL+
-240098048 R@ ! A:CELL+ lang.Long.equals§-240098048 VAL R@ ! A:CELL+
1416259072 R@ ! A:CELL+ lang.Long.floatValue§1416259072 VAL R@ ! A:CELL+
-1604556800 R@ ! A:CELL+ lang.Long.hashCode§-1604556800 VAL R@ ! A:CELL+
-370906880 R@ ! A:CELL+ lang.Long.intValue§-370906880 VAL R@ ! A:CELL+
1506831360 R@ ! A:CELL+ lang.Long.longValue§1506831360 VAL R@ ! A:CELL+
715813632 R@ ! A:CELL+ lang.Long.shortValue§715813632 VAL R@ ! A:CELL+
1621718016 R@ ! A:CELL+ lang.Long.toString§1621718016 VAL R@ ! A:CELL+
A:DROP
