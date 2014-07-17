MODULENAME java.lang.Integer
(
* @(#)Integer.java	1.76 03/01/23
*
* Copyright 2003 Sun Microsystems, Inc. All rights reserved.
* SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
)  ( *
* The <code>Integer</code> class wraps a value of the primitive type
* <code>int</code> in an object. An object of type
* <code>Integer</code> contains a single field whose type is
* <code>int</code>.
*
*  <p>
*
* In addition, this class provides several methods for converting an
* <code>int</code> to a <code>String</code> and a <code>String</code>
* to an <code>int</code>, as well as other constants and methods
* useful when dealing with an <code>int</code>.
*
* @author  Lee Boynton
* @author  Arthur van Hoff
* @version 1.76, 01/23/03
* @since   JDK1.0
)


:LOCAL lang.Integer.Integer§-176207616
   1 VALLOCATE LOCAL §base0
   
   \ new statement
   0
   LOCALS §this |
   
   \ new statement
   0
   
   \ new statement
   32 MALLOC DROP DUP DUP §base0 V! TO §this OVER
   IF
      OVER CELL+ @ 4 + OVER OVER ! DROP
   ENDIF
   CELL+ @ OVER
   OVER ! NIP
   lang.Integer§-1073788672.table OVER 12 + !
   -1073807360 OVER 20 + !
   " Integer " OVER 16 + !
   0 OVER 24 + ! DROP
   
   \ new statement
   ( *
   * The value of the <code>Integer</code>.
   *
   * @serial
   )
   
   \ new statement
   §this DUP 0 V! ( return object )    
   \ new statement
   0 §break17008 LABEL
   
   \ new statement
   §base0 SETVTOP
   PURGE 2
   
   \ new statement
   DROP
;
( *
* Constructs a newly allocated <code>Integer</code> object that
* represents the <code>int</code> value indicated by the
* <code>String</code> parameter. The string is converted to an
* <code>int</code> value in exactly the manner used by the
* <code>parseInt</code> method for radix 10.
*
* @param      s   the <code>String</code> to be converted to an
*                 <code>Integer</code>.
* @exception  NumberFormatException  if the <code>String</code> does not
*               contain a parsable integer.
* @see        java.lang.Integer#parseInt(java.lang.String, int)
)

:LOCAL lang.Integer.Integer§-502511360
   2 VALLOCATE LOCAL §base0
   LOCAL §exception
   DUP 4 §base0 + V! LOCAL s
   
   \ new statement
   0
   LOCALS §this |
   
   \ new statement
   0
   
   \ new statement
   32 MALLOC DROP DUP DUP §base0 V! TO §this OVER
   IF
      OVER CELL+ @ 4 + OVER OVER ! DROP
   ENDIF
   CELL+ @ OVER
   OVER ! NIP
   lang.Integer§-1073788672.table OVER 12 + !
   -1073807360 OVER 20 + !
   " Integer " OVER 16 + !
   0 OVER 24 + ! DROP
   
   \ new statement
   ( *
   * The value of the <code>Integer</code>.
   *
   * @serial
   )
   
   \ new statement
   s
   10
   std21510 VAL
   lang.Integer.parseInt§537554944
   §this CELL+ @ 28 + ( java.lang.Integer.value )    ! FALSE DUP
   IF
      std21510 LABEL TRUE
   ENDIF
   IF
      §exception TO §return
      0 §break16986 BRANCH
   ENDIF
   
   \ new statement
   §this DUP 0 V! ( return object )    
   \ new statement
   0 §break16986 LABEL
   
   \ new statement
   
   
   §base0 SETVTOP
   PURGE 4
   
   \ new statement
   DROP
;
( *
* Constructs a newly allocated <code>Integer</code> object that
* represents the specified <code>int</code> value.
*
* @param   value   the value to be represented by the
*			<code>Integer</code> object.
)

:LOCAL lang.Integer.Integer§1561020672
   1 VALLOCATE LOCAL §base0
   LOCAL value
   
   \ new statement
   0
   LOCALS §this |
   
   \ new statement
   0
   
   \ new statement
   32 MALLOC DROP DUP DUP §base0 V! TO §this OVER
   IF
      OVER CELL+ @ 4 + OVER OVER ! DROP
   ENDIF
   CELL+ @ OVER
   OVER ! NIP
   lang.Integer§-1073788672.table OVER 12 + !
   -1073807360 OVER 20 + !
   " Integer " OVER 16 + !
   0 OVER 24 + ! DROP
   
   \ new statement
   ( *
   * The value of the <code>Integer</code>.
   *
   * @serial
   )
   
   \ new statement
   value
   §this CELL+ @ 28 + ( java.lang.Integer.value )    !
   
   \ new statement
   §this DUP 0 V! ( return object )    
   \ new statement
   0 §break16985 LABEL
   
   \ new statement
   
   §base0 SETVTOP
   PURGE 3
   
   \ new statement
   DROP
;


: lang.Integer.appendTo§-642555648
   2 VALLOCATE LOCAL §base0
   DUP 0 §base0 + V! LOCAL sb
   LOCAL i
   
   \ new statement
   0 DUP DUP
   LOCALS body length s |
   
   \ new statement
   BASE 10 TO BASE
   
   \ new statement
   i 0 ..R TO length TO body TO BASE
   
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
   0 §break16980 LABEL
   
   \ new statement
   
   
   
   
   
   §base0 SETVTOP
   PURGE 6
   
   \ new statement
   DROP
;
( *
* Returns the value of this <code>Integer</code> as a
* <code>byte</code>.
)

:LOCAL lang.Integer.byteValue§-1994235392
   1 VALLOCATE LOCAL §base0
   DUP 0 §base0 + V! LOCAL §this
   
   \ new statement
   
   \ new statement
   §this CELL+ @ 28 + ( java.lang.Integer.value )    @ A:R@  127 U>! NIP
   IF
      0FFFFFF00H OR
   ENDIF
   A:DROP
   0 §break16987 BRANCH
   
   \ new statement
   0 §break16987 LABEL
   
   \ new statement
   §base0 SETVTOP
   PURGE 2
   
   \ new statement
   DROP
;


: lang.Integer.classMonitorEnter§-433167616
   
   \ new statement
   
   \ new statement
   0 §break16972 LABEL
   
   \ new statement
   
   \ new statement
   DROP
;


: lang.Integer.classMonitorLeave§-1691393280
   
   \ new statement
   
   \ new statement
   0 §break16973 LABEL
   
   \ new statement
   
   \ new statement
   DROP
;
( *
* Compares this <code>Integer</code> object to another object.
* If the object is an <code>Integer</code>, this function behaves
* like <code>compareTo(Integer)</code>.  Otherwise, it throws a
* <code>ClassCastException</code> (as <code>Integer</code>
* objects are only comparable to other <code>Integer</code>
* objects).
*
* @param   o the <code>Object</code> to be compared.
* @return  the value <code>0</code> if the argument is a
*		<code>Integer</code> numerically equal to this
*		<code>Integer</code>; a value less than <code>0</code>
*		if the argument is a <code>Integer</code> numerically
*		greater than this <code>Integer</code>; and a value
*		greater than <code>0</code> if the argument is a
*		<code>Integer</code> numerically less than this
*		<code>Integer</code>.
* @exception <code>ClassCastException</code> if the argument is not an
*		  <code>Integer</code>.
* @see     java.lang.Comparable
* @since   1.2
)

:LOCAL lang.Integer.compareTo§-879140096
   2 VALLOCATE LOCAL §base0
   DUP 0 §base0 + V! LOCAL o
   DUP 4 §base0 + V! LOCAL §this
   
   \ new statement
   
   \ new statement
   §this A:R@
   o
   R> 32465664 TRUE ( java.lang.Integer.compareTo§32465664 ) EXECUTE-METHOD
   0 §break17007 BRANCH
   
   \ new statement
   0 §break17007 LABEL
   
   \ new statement
   
   §base0 SETVTOP
   PURGE 3
   
   \ new statement
   DROP
;
( *
* Compares two <code>Integer</code> objects numerically.
*
* @param   anotherInteger   the <code>Integer</code> to be compared.
* @return	the value <code>0</code> if this <code>Integer</code> is
* 		equal to the argument <code>Integer</code>; a value less than
* 		<code>0</code> if this <code>Integer</code> is numerically less
* 		than the argument <code>Integer</code>; and a value greater
* 		than <code>0</code> if this <code>Integer</code> is numerically
* 		 greater than the argument <code>Integer</code> (signed
* 		 comparison).
* @since   1.2
)

:LOCAL lang.Integer.compareTo§32465664
   2 VALLOCATE LOCAL §base0
   DUP 0 §base0 + V! LOCAL anotherInteger
   DUP 4 §base0 + V! LOCAL §this
   
   \ new statement
   0 DUP
   LOCALS anotherVal thisVal |
   
   \ new statement
   §this CELL+ @ 28 + ( java.lang.Integer.value )    @
   TO thisVal
   
   \ new statement
   anotherInteger CELL+ @ 28 + ( java.lang.Integer.value )    @
   TO anotherVal
   
   \ new statement
   thisVal
   anotherVal
   <
   IF
      -1
      
   ELSE
      thisVal
      anotherVal
      =
      IF
         0
         
      ELSE
         1
         
      ENDIF
      
   ENDIF
   0 §break17006 BRANCH
   
   \ new statement
   0 §break17006 LABEL
   
   \ new statement
   
   
   
   §base0 SETVTOP
   PURGE 5
   
   \ new statement
   DROP
;
( *
* Determines the integer value of the system property with the
* specified name.
* <p>
* The first argument is treated as the name of a system property.
* System properties are accessible through the
* {@link java.lang.System#getProperty(java.lang.String)} method. The
* string value of this property is then interpreted as an integer
* value and an <code>Integer</code> object representing this value is
* returned. Details of possible numeric formats can be found with
* the definition of <code>getProperty</code>.
* <p>
* If there is no property with the specified name, if the specified name
* is empty or <code>null</code>, or if the property does not have
* the correct numeric format, then <code>null</code> is returned.
* <p>
* In other words, this method returns an <code>Integer</code>
* object equal to the value of:
*
* <blockquote><code>
* getInteger(nm, null)
* </code></blockquote>
*
* @param   nm   property name.
* @return  the <code>Integer</code> value of the property.
* @see     java.lang.System#getProperty(java.lang.String)
* @see     java.lang.System#getProperty(java.lang.String, java.lang.String)
public static Integer getInteger(String nm) {
return getInteger(nm, null);
}
)  ( *
* Determines the integer value of the system property with the
* specified name.
* <p>
* The first argument is treated as the name of a system property.
* System properties are accessible through the {@link
* java.lang.System#getProperty(java.lang.String)} method. The
* string value of this property is then interpreted as an integer
* value and an <code>Integer</code> object representing this value is
* returned. Details of possible numeric formats can be found with
* the definition of <code>getProperty</code>.
* <p>
* The second argument is the default value. An <code>Integer</code> object
* that represents the value of the second argument is returned if there
* is no property of the specified name, if the property does not have
* the correct numeric format, or if the specified name is empty or
*  <code>null</code>.
* <p>
* In other words, this method returns an <code>Integer</code> object
* equal to the value of:
* <blockquote><code>
* getInteger(nm, new Integer(val))
* </code></blockquote>
* but in practice it may be implemented in a manner such as:
* <blockquote><pre>
* Integer result = getInteger(nm, null);
* return (result == null) ? new Integer(val) : result;
* </pre></blockquote>
* to avoid the unnecessary allocation of an <code>Integer</code>
* object when the default value is not needed.
*
* @param   nm   property name.
* @param   val   default value.
* @return  the <code>Integer</code> value of the property.
* @see     java.lang.System#getProperty(java.lang.String)
* @see     java.lang.System#getProperty(java.lang.String, java.lang.String)
public static Integer getInteger(String nm, int val) {
Integer result = getInteger(nm, null);
return (result == null) ? new Integer(val) : result;
}
)  ( *
* Returns the integer value of the system property with the
* specified name.  The first argument is treated as the name of a
* system property.  System properties are accessible through the
* {@link java.lang.System#getProperty(java.lang.String)} method.
* The string value of this property is then interpreted as an
* integer value, as per the <code>Integer.decode</code> method,
* and an <code>Integer</code> object representing this value is
* returned.
* <p>
* <ul><li>If the property value begins with the two ASCII characters
*         <code>0x</code> or the ASCII character <code>#</code>, not
*      followed by a minus sign, then the rest of it is parsed as a
*      hexadecimal integer exactly as by the method
*      {@link #valueOf(java.lang.String, int)} with radix 16.
* <li>If the property value begins with the ASCII character
*     <code>0</code> followed by another character, it is parsed as an
*     octal integer exactly as by the method
*     {@link #valueOf(java.lang.String, int)} with radix 8.
* <li>Otherwise, the property value is parsed as a decimal integer
* exactly as by the method {@link #valueOf(java.lang.String, int)}
* with radix 10.
* </ul><p>
* The second argument is the default value. The default value is
* returned if there is no property of the specified name, if the
* property does not have the correct numeric format, or if the
* specified name is empty or <code>null</code>.
*
* @param   nm   property name.
* @param   val   default value.
* @return  the <code>Integer</code> value of the property.
* @see     java.lang.System#getProperty(java.lang.String)
* @see java.lang.System#getProperty(java.lang.String, java.lang.String)
* @see java.lang.Integer#decode
public static Integer getInteger(String nm, Integer val) {
String v = null;
try {
v = System.getProperty(nm);
} catch (IllegalArgumentException e) {
} catch (NullPointerException e) {
}
if (v != null) {
try {
return Integer.decode(v);
} catch (NumberFormatException e) {
}
}
return val;
}
)  ( *
* Decodes a <code>String</code> into an <code>Integer</code>.
* Accepts decimal, hexadecimal, and octal numbers numbers given
* by the following grammar:
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
* leading zero) is parsed as by the <code>Integer.parseInt</code>
* method with the indicated radix (10, 16, or 8).  This sequence
* of characters must represent a positive value or a {@link
* NumberFormatException} will be thrown.  The result is negated
* if first character of the specified <code>String</code> is the
* minus sign.  No whitespace characters are permitted in the
* <code>String</code>.
*
* @param     nm the <code>String</code> to decode.
* @return    a <code>Integer</code> object holding the <code>int</code>
*		   value represented by <code>nm</code>
* @exception NumberFormatException  if the <code>String</code> does not
*            contain a parsable integer.
* @see java.lang.Integer#parseInt(java.lang.String, int)
* @since 1.2
)

: lang.Integer.decode§-1261280256
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
      0 §break16998 LABEL
      
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
   0=! or_21573 0BRANCH DROP
   nm A:R@
   U" 0X " COUNT lang.JavaArray.createUnicode§-675323136 DUP §tempvar 4 + V!
   index
   R> -953322752 TRUE ( java.lang.String.startsWith§-953322752 ) EXECUTE-METHOD
   or_21573 LABEL
   
   
   
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
      0 §break16999 LABEL
      
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
         0 §break17000 LABEL
         
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
         and_21592 0BRANCH! DROP
         nm A:R@
         R> 188050432 TRUE ( java.lang.String.length§188050432 ) EXECUTE-METHOD
         1
         index
         +
         >
         and_21592 LABEL
         
         
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
            0 §break17001 LABEL
            
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
      0 §break16997 BRANCH
      
      \ new statement
   ENDIF
   
   \ new statement
   
   \ new statement
   
   1 VALLOCATE LOCAL §tempvar
   nm A:R@
   index
   R> 18576128 TRUE ( java.lang.String.substring§18576128 ) EXECUTE-METHOD DUP §tempvar V!
   radix
   std21609 VAL
   lang.Integer.valueOf§-1472039424
   DUP 4 §base0 + V! TO result FALSE DUP
   IF
      std21609 LABEL TRUE
   ENDIF
   
   §tempvar SETVTOP
   
   PURGE 1
   
   IF
      0 §break17002 BRANCH
   ENDIF
   
   \ new statement
   negative
   IF
      result A:R@
      R> -370906880 TRUE ( java.lang.Integer.intValue§-370906880 ) EXECUTE-METHOD
      NEGATE
      lang.Integer§-1073788672.table 1561020672 EXECUTE-NEW
      
   ELSE
      result
      
   ENDIF
   DUP 4 §base0 + V! TO result
   
   \ new statement
   0 §except17003 LABEL
   
   \ new statement
   0 §break17002 LABEL
   
   \ new statement
   
   \ new statement
   0>!
   IF
      10001H - §break16997 BRANCH
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
      \  If number is Integer.MIN_VALUE, we'll end up here. The next line
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
      std21620 VAL
      lang.Integer.valueOf§-1472039424
      DUP 4 §base0 + V! TO result FALSE DUP
      IF
         std21620 LABEL TRUE
      ENDIF
      IF
         §exception TO §return
         65537 §break17005 BRANCH
      ENDIF
      
      \ new statement
      0 §break17005 LABEL
      
      \ new statement
      
      
      §base1 SETVTOP
      PURGE 3
      
      \ new statement
      0>!
      IF
         10001H - §break16997 BRANCH
      ENDIF
      DROP
      
      \ new statement
      DUP §out17004 BRANCH
   ENDIF
   
   \ new statement
   §out17004 LABEL DROP
   
   \ new statement
   result
   DUP 0 V!0 §break16997 BRANCH
   
   \ new statement
   0 §break16997 LABEL
   
   \ new statement
   
   
   
   
   
   
   §base0 SETVTOP
   PURGE 7
   
   \ new statement
   DROP
;
( *
* Returns the value of this <code>Integer</code> as a
* <code>double</code>.
)

:LOCAL lang.Integer.doubleValue§-330275840
   1 VALLOCATE LOCAL §base0
   DUP 0 §base0 + V! LOCAL §this
   
   \ new statement
   
   \ new statement
   §this CELL+ @ 28 + ( java.lang.Integer.value )    @ A:R@  S>D D>F
   A:DROP
   0 §break16992 BRANCH
   
   \ new statement
   0 §break16992 LABEL
   
   \ new statement
   §base0 SETVTOP
   PURGE 2
   
   \ new statement
   DROP
;
( *
* Compares this object to the specified object.  The result is
* <code>true</code> if and only if the argument is not
* <code>null</code> and is an <code>Integer</code> object that
* contains the same <code>int</code> value as this object.
*
* @param   obj   the object to compare with.
* @return  <code>true</code> if the objects are the same;
*          <code>false</code> otherwise.
)

:LOCAL lang.Integer.equals§-240098048
   2 VALLOCATE LOCAL §base0
   DUP 0 §base0 + V! LOCAL obj
   DUP 4 §base0 + V! LOCAL §this
   
   \ new statement
   
   \ new statement
   obj
   " Integer " INSTANCEOF
   
   \ new statement
   IF
      
      \ new statement
      
      \ new statement
      §this CELL+ @ 28 + ( java.lang.Integer.value )       @
      obj
      A:R@
      R> -370906880 TRUE ( java.lang.Integer.intValue§-370906880 ) EXECUTE-METHOD
      =
      65537 §break16996 BRANCH
      
      \ new statement
      0 §break16996 LABEL
      
      \ new statement
      
      \ new statement
      0>!
      IF
         10001H - §break16995 BRANCH
      ENDIF
      DROP
      
      \ new statement
   ENDIF
   
   \ new statement
   FALSE
   0 §break16995 BRANCH
   
   \ new statement
   0 §break16995 LABEL
   
   \ new statement
   
   §base0 SETVTOP
   PURGE 3
   
   \ new statement
   DROP
;
( *
* Returns the value of this <code>Integer</code> as a
* <code>float</code>.
)

:LOCAL lang.Integer.floatValue§1416259072
   1 VALLOCATE LOCAL §base0
   DUP 0 §base0 + V! LOCAL §this
   
   \ new statement
   
   \ new statement
   §this CELL+ @ 28 + ( java.lang.Integer.value )    @ A:R@  S>D D>F
   A:DROP
   0 §break16991 BRANCH
   
   \ new statement
   0 §break16991 LABEL
   
   \ new statement
   §base0 SETVTOP
   PURGE 2
   
   \ new statement
   DROP
;
( *
* Returns a hash code for this <code>Integer</code>.
*
* @return  a hash code value for this object, equal to the
*          primitive <code>int</code> value represented by this
*          <code>Integer</code> object.
)

:LOCAL lang.Integer.hashCode§-1604556800
   1 VALLOCATE LOCAL §base0
   DUP 0 §base0 + V! LOCAL §this
   
   \ new statement
   
   \ new statement
   §this CELL+ @ 28 + ( java.lang.Integer.value )    @
   0 §break16994 BRANCH
   
   \ new statement
   0 §break16994 LABEL
   
   \ new statement
   §base0 SETVTOP
   PURGE 2
   
   \ new statement
   DROP
;
( *
* Returns the value of this <code>Integer</code> as an
* <code>int</code>.
)

:LOCAL lang.Integer.intValue§-370906880
   1 VALLOCATE LOCAL §base0
   DUP 0 §base0 + V! LOCAL §this
   
   \ new statement
   
   \ new statement
   §this CELL+ @ 28 + ( java.lang.Integer.value )    @
   0 §break16989 BRANCH
   
   \ new statement
   0 §break16989 LABEL
   
   \ new statement
   §base0 SETVTOP
   PURGE 2
   
   \ new statement
   DROP
;
( *
* Returns the value of this <code>Integer</code> as a
* <code>long</code>.
)

:LOCAL lang.Integer.longValue§1506831360
   1 VALLOCATE LOCAL §base0
   DUP 0 §base0 + V! LOCAL §this
   
   \ new statement
   
   \ new statement
   §this CELL+ @ 28 + ( java.lang.Integer.value )    @ A:R@  S>D
   A:DROP
   0 §break16990 BRANCH
   
   \ new statement
   0 §break16990 LABEL
   
   \ new statement
   §base0 SETVTOP
   PURGE 2
   
   \ new statement
   DROP
;
( *
* Parses the string argument as a signed decimal integer. The
* characters in the string must all be decimal digits, except that
* the first character may be an ASCII minus sign <code>'-'</code>
* (<code>'&#92;u002D'</code>) to indicate a negative value. The resulting
* integer value is returned, exactly as if the argument and the radix
* 10 were given as arguments to the
* {@link #parseInt(java.lang.String, int)} method.
*
* @param s	   a <code>String</code> containing the <code>int</code>
*             representation to be parsed
* @return     the integer value represented by the argument in decimal.
* @exception  NumberFormatException  if the string does not contain a
*               parsable integer.
)

: lang.Integer.parseInt§-1920897024
   1 VALLOCATE LOCAL §base0
   LOCAL §exception
   DUP 0 §base0 + V! LOCAL s
   
   \ new statement
   
   \ new statement
   s
   10
   std21669 VAL
   lang.Integer.parseInt§537554944
   0 §break16982 BRANCH
   FALSE DUP
   IF
      std21669 LABEL TRUE
   ENDIF
   IF
      §exception TO §return
      0 §break16982 BRANCH
   ENDIF
   
   \ new statement
   0 §break16982 LABEL
   
   \ new statement
   
   
   §base0 SETVTOP
   PURGE 3
   
   \ new statement
   DROP
;
( *
* Parses the string argument as a signed integer in the radix
* specified by the second argument. The characters in the string
* must all be digits of the specified radix (as determined by
* whether {@link java.lang.Character#digit(char, int)} returns a
* nonnegative value), except that the first character may be an
* ASCII minus sign <code>'-'</code> (<code>'&#92;u002D'</code>) to
* indicate a negative value. The resulting integer value is returned.
* <p>
* An exception of type <code>NumberFormatException</code> is
* thrown if any of the following situations occurs:
* <ul>
* <li>The first argument is <code>null</code> or is a string of
* length zero.
* <li>The radix is either smaller than
* {@link java.lang.Character#MIN_RADIX} or
* larger than {@link java.lang.Character#MAX_RADIX}.
* <li>Any character of the string is not a digit of the specified
* radix, except that the first character may be a minus sign
* <code>'-'</code> (<code>'&#92;u002D'</code>) provided that the
* string is longer than length 1.
* <li>The value represented by the string is not a value of type
* <code>int</code>.
* </ul><p>
* Examples:
* <blockquote><pre>
* parseInt("0", 10) returns 0
* parseInt("473", 10) returns 473
* parseInt("-0", 10) returns 0
* parseInt("-FF", 16) returns -255
* parseInt("1100110", 2) returns 102
* parseInt("2147483647", 10) returns 2147483647
* parseInt("-2147483648", 10) returns -2147483648
* parseInt("2147483648", 10) throws a NumberFormatException
* parseInt("99", 8) throws a NumberFormatException
* parseInt("Kona", 10) throws a NumberFormatException
* parseInt("Kona", 27) returns 411787
* </pre></blockquote>
*
* @param      s   the <code>String</code> containing the integer
* 			representation to be parsed
* @param      radix   the radix to be used while parsing <code>s</code>.
* @return     the integer represented by the string argument in the
*             specified radix.
* @exception  NumberFormatException if the <code>String</code>
* 		   does not contain a parsable <code>int</code>.
)

: lang.Integer.parseInt§537554944
   1 VALLOCATE LOCAL §base0
   LOCAL §exception
   LOCAL radix
   DUP 0 §base0 + V! LOCAL s
   
   \ new statement
   0 DUP
   LOCALS body length |
   
   \ new statement
   s A:R@
   R> -1067748352 TRUE ( java.lang.String.trim§-1067748352 ) EXECUTE-METHOD
   DUP §base0 V! TO s
   
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
   0 DUP body CELL+ @ length >NUMBER 2DROP ROT TO BASE D>S
   
   \ new statement
   0 §break16981 LABEL
   
   \ new statement
   
   
   
   
   
   §base0 SETVTOP
   PURGE 6
   
   \ new statement
   DROP
;
( *
* Returns the value of this <code>Integer</code> as a
* <code>short</code>.
)

:LOCAL lang.Integer.shortValue§715813632
   1 VALLOCATE LOCAL §base0
   DUP 0 §base0 + V! LOCAL §this
   
   \ new statement
   
   \ new statement
   §this CELL+ @ 28 + ( java.lang.Integer.value )    @ A:R@  32767 U>! NIP
   IF
      0FFFF0000H OR
   ENDIF
   A:DROP
   0 §break16988 BRANCH
   
   \ new statement
   0 §break16988 LABEL
   
   \ new statement
   §base0 SETVTOP
   PURGE 2
   
   \ new statement
   DROP
;
( *
* Returns a string representation of the integer argument as an
* unsigned integer in base&nbsp;2.
* <p>
* The unsigned integer value is the argument plus 2<sup>32</sup>
* if the argument is negative; otherwise it is equal to the
* argument.  This value is converted to a string of ASCII digits
* in binary (base&nbsp;2) with no extra leading <code>0</code>s.
* If the unsigned magnitude is zero, it is represented by a
* single zero character <code>'0'</code>
* (<code>'&#92;u0030'</code>); otherwise, the first character of
* the representation of the unsigned magnitude will not be the
* zero character. The characters <code>'0'</code>
* (<code>'&#92;u0030'</code>) and <code>'1'</code>
* (<code>'&#92;u0031'</code>) are used as binary digits.
*
* @param   i   an integer to be converted to a string.
* @return  the string representation of the unsigned integer value
*          represented by the argument in binary (base&nbsp;2).
* @since   JDK1.0.2
)

: lang.Integer.toBinaryString§-288787456
   LOCAL i
   
   \ new statement
   
   \ new statement
   i
   1
   lang.Integer.toUnsignedString§1788703744
   DUP 0 V!0 §break16977 BRANCH
   
   \ new statement
   0 §break16977 LABEL
   
   \ new statement
   
   PURGE 1
   
   \ new statement
   DROP
;
( *
* Returns a string representation of the integer argument as an
* unsigned integer in base&nbsp;16.
* <p>
* The unsigned integer value is the argument plus 2<sup>32</sup>
* if the argument is negative; otherwise, it is equal to the
* argument.  This value is converted to a string of ASCII digits
* in hexadecimal (base&nbsp;16) with no extra leading
* <code>0</code>s. If the unsigned magnitude is zero, it is
* represented by a single zero character <code>'0'</code>
* (<code>'&#92;u0030'</code>); otherwise, the first character of
* the representation of the unsigned magnitude will not be the
* zero character. The following characters are used as
* hexadecimal digits:
* <blockquote><pre>
* 0123456789abcdef
* </pre></blockquote>
* These are the characters <code>'&#92;u0030'</code> through
* <code>'&#92;u0039'</code> and <code>'&#92;u0061'</code> through
* <code>'&#92;u0066'</code>. If uppercase letters are
* desired, the {@link java.lang.String#toUpperCase()} method may
* be called on the result:
* <blockquote><pre>
* Integer.toHexString(n).toUpperCase()
* </pre></blockquote>
*
* @param   i   an integer to be converted to a string.
* @return  the string representation of the unsigned integer value
*          represented by the argument in hexadecimal (base&nbsp;16).
* @since   JDK1.0.2
)

: lang.Integer.toHexString§1337684992
   LOCAL i
   
   \ new statement
   
   \ new statement
   i
   4
   lang.Integer.toUnsignedString§1788703744
   DUP 0 V!0 §break16975 BRANCH
   
   \ new statement
   0 §break16975 LABEL
   
   \ new statement
   
   PURGE 1
   
   \ new statement
   DROP
;
( *
* Returns a string representation of the integer argument as an
* unsigned integer in base&nbsp;8.
* <p>
* The unsigned integer value is the argument plus 2<sup>32</sup>
* if the argument is negative; otherwise, it is equal to the
* argument.  This value is converted to a string of ASCII digits
* in octal (base&nbsp;8) with no extra leading <code>0</code>s.
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
* @param   i   an integer to be converted to a string.
* @return  the string representation of the unsigned integer value
*          represented by the argument in octal (base&nbsp;8).
* @since   JDK1.0.2
)

: lang.Integer.toOctalString§-1590070272
   LOCAL i
   
   \ new statement
   
   \ new statement
   i
   3
   lang.Integer.toUnsignedString§1788703744
   DUP 0 V!0 §break16976 BRANCH
   
   \ new statement
   0 §break16976 LABEL
   
   \ new statement
   
   PURGE 1
   
   \ new statement
   DROP
;
\  I use the "invariant division by multiplication" trick to
\  accelerate Integer.toString.  In particular we want to
\  avoid division by 10.
\
\  The "trick" has roughly the same performance characterists
\  as the "classic" Integer.toString code on a non-JIT VM.
\  The trick avoids .rem and .div calls but has a longer code
\  path and is thus dominated by dispatch overhead.  In the
\  JIT case the dispatch overhead doesn't exist and the
\  "trick" is considerably faster than the classic code.
\
\  TODO-FIXME: convert (x * 52429) into the equiv shift-add
\  sequence.
\
\  RE:  Division by Invariant Integers using Multiplication
\       T Gralund, P Montgomery
\       ACM PLDI 1994
\
( *
* Returns a <code>String</code> object representing the
* specified integer. The argument is converted to signed decimal
* representation and returned as a string, exactly as if the
* argument and radix 10 were given as arguments to the {@link
* #toString(int, int)} method.
*
* @param   i   an integer to be converted.
* @return  a string representation of the argument in base&nbsp;10.
)

: lang.Integer.toString§1050637312
   LOCAL i
   
   \ new statement
   0 DUP
   LOCALS body length |
   
   \ new statement
   BASE 10 TO BASE
   
   \ new statement
   i 0 ..R TO length TO body TO BASE
   
   \ new statement
   length
   body
   lang.JavaArray.createString§-105880832
   DUP 0 V!0 §break16979 BRANCH
   
   \ new statement
   0 §break16979 LABEL
   
   \ new statement
   
   
   
   PURGE 3
   
   \ new statement
   DROP
;
( *
* Returns a <code>String</code> object representing this
* <code>Integer</code>'s value. The value is converted to signed
* decimal representation and returned as a string, exactly as if
* the integer value were given as an argument to the {@link
* java.lang.Integer#toString(int)} method.
*
* @return  a string representation of the value of this object in
*          base&nbsp;10.
)

:LOCAL lang.Integer.toString§1621718016
   1 VALLOCATE LOCAL §base0
   DUP 0 §base0 + V! LOCAL §this
   
   \ new statement
   
   \ new statement
   §this CELL+ @ 28 + ( java.lang.Integer.value )    @
   lang.String.valueOf§-166496768
   DUP 0 V!0 §break16993 BRANCH
   
   \ new statement
   0 §break16993 LABEL
   
   \ new statement
   §base0 SETVTOP
   PURGE 2
   
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
* result is the ASCII minus character <code>'-'</code>
* (<code>'&#92;u002D'</code>). If the first argument is not
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
* <code>'&#92;u007A'</code>. If <code>radix</code> is
* <var>N</var>, then the first <var>N</var> of these characters
* are used as radix-<var>N</var> digits in the order shown. Thus,
* the digits for hexadecimal (radix 16) are
* <code>0123456789abcdef</code>. If uppercase letters are
* desired, the {@link java.lang.String#toUpperCase()} method may
* be called on the result:
* <blockquote><pre>
* Integer.toString(n, 16).toUpperCase()
* </pre></blockquote>
*
* @param   i       an integer to be converted to a string.
* @param   radix   the radix to use in the string representation.
* @return  a string representation of the argument in the specified radix.
* @see     java.lang.Character#MAX_RADIX
* @see     java.lang.Character#MIN_RADIX
)

: lang.Integer.toString§502887424
   LOCAL radix
   LOCAL i
   
   \ new statement
   0 DUP
   LOCALS body length |
   
   \ new statement
   BASE radix TO BASE
   
   \ new statement
   i 0 ..R TO length TO body TO BASE
   
   \ new statement
   length
   body
   lang.JavaArray.createString§-105880832
   DUP 0 V!0 §break16974 BRANCH
   
   \ new statement
   0 §break16974 LABEL
   
   \ new statement
   
   
   
   
   PURGE 4
   
   \ new statement
   DROP
;
( *
* Convert the integer to an unsigned number.
)

: lang.Integer.toUnsignedString§1788703744
   LOCAL shift
   LOCAL i
   
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
   i 0 U..R TO length TO body TO BASE
   
   \ new statement
   length
   body
   lang.JavaArray.createString§-105880832
   DUP 0 V!0 §break16978 BRANCH
   
   \ new statement
   0 §break16978 LABEL
   
   \ new statement
   
   
   
   
   
   PURGE 5
   
   \ new statement
   DROP
;
( *
* Returns an <code>Integer</code> object holding the value
* extracted from the specified <code>String</code> when parsed
* with the radix given by the second argument. The first argument
* is interpreted as representing a signed integer in the radix
* specified by the second argument, exactly as if the arguments
* were given to the {@link #parseInt(java.lang.String, int)}
* method. The result is an <code>Integer</code> object that
* represents the integer value specified by the string.
* <p>
* In other words, this method returns an <code>Integer</code>
* object equal to the value of:
*
* <blockquote><code>
* new Integer(Integer.parseInt(s, radix))
* </code></blockquote>
*
* @param      s   the string to be parsed.
* @param      radix the radix to be used in interpreting <code>s</code>
* @return     an <code>Integer</code> object holding the value
*             represented by the string argument in the specified
*             radix.
* @exception NumberFormatException if the <code>String</code>
* 		  does not contain a parsable <code>int</code>.
)

: lang.Integer.valueOf§-1472039424
   1 VALLOCATE LOCAL §base0
   LOCAL §exception
   LOCAL radix
   DUP 0 §base0 + V! LOCAL s
   
   \ new statement
   
   \ new statement
   s
   radix
   std21730 VAL
   lang.Integer.parseInt§537554944
   lang.Integer§-1073788672.table 1561020672 EXECUTE-NEW
   DUP 0 V!0 §break16983 BRANCH
   FALSE DUP
   IF
      std21730 LABEL TRUE
   ENDIF
   IF
      §exception TO §return
      0 §break16983 BRANCH
   ENDIF
   
   \ new statement
   0 §break16983 LABEL
   
   \ new statement
   
   
   
   §base0 SETVTOP
   PURGE 4
   
   \ new statement
   DROP
;
( *
* Returns an <code>Integer</code> object holding the
* value of the specified <code>String</code>. The argument is
* interpreted as representing a signed decimal integer, exactly
* as if the argument were given to the {@link
* #parseInt(java.lang.String)} method. The result is an
* <code>Integer</code> object that represents the integer value
* specified by the string.
* <p>
* In other words, this method returns an <code>Integer</code>
* object equal to the value of:
*
* <blockquote><code>
* new Integer(Integer.parseInt(s))
* </code></blockquote>
*
* @param      s   the string to be parsed.
* @return     an <code>Integer</code> object holding the value
*             represented by the string argument.
* @exception  NumberFormatException  if the string cannot be parsed
*             as an integer.
)

: lang.Integer.valueOf§712209920
   1 VALLOCATE LOCAL §base0
   LOCAL §exception
   DUP 0 §base0 + V! LOCAL s
   
   \ new statement
   
   \ new statement
   s
   10
   std21735 VAL
   lang.Integer.parseInt§537554944
   lang.Integer§-1073788672.table 1561020672 EXECUTE-NEW
   DUP 0 V!0 §break16984 BRANCH
   FALSE DUP
   IF
      std21735 LABEL TRUE
   ENDIF
   IF
      §exception TO §return
      0 §break16984 BRANCH
   ENDIF
   
   \ new statement
   0 §break16984 LABEL
   
   \ new statement
   
   
   §base0 SETVTOP
   PURGE 3
   
   \ new statement
   DROP
;
-1 VALLOCATE CONSTANT lang.Integer.DigitOnes
-1 VALLOCATE CONSTANT lang.Integer.DigitTens
VARIABLE lang.Integer.MAX_VALUE
VARIABLE lang.Integer.MIN_VALUE
VARIABLE lang.Integer._staticBlocking
VARIABLE lang.Integer._staticThread
-1 VALLOCATE CONSTANT lang.Integer.digits
2VARIABLE lang.Integer.serialVersionUID

A:HERE VARIABLE lang.Integer§-1073788672.table 14 DUP 2* CELLS ALLOT R@ ! A:CELL+
-176207616 R@ ! A:CELL+ lang.Integer.Integer§-176207616 VAL R@ ! A:CELL+
-502511360 R@ ! A:CELL+ lang.Integer.Integer§-502511360 VAL R@ ! A:CELL+
1561020672 R@ ! A:CELL+ lang.Integer.Integer§1561020672 VAL R@ ! A:CELL+
-1994235392 R@ ! A:CELL+ lang.Integer.byteValue§-1994235392 VAL R@ ! A:CELL+
-879140096 R@ ! A:CELL+ lang.Integer.compareTo§-879140096 VAL R@ ! A:CELL+
32465664 R@ ! A:CELL+ lang.Integer.compareTo§32465664 VAL R@ ! A:CELL+
-330275840 R@ ! A:CELL+ lang.Integer.doubleValue§-330275840 VAL R@ ! A:CELL+
-240098048 R@ ! A:CELL+ lang.Integer.equals§-240098048 VAL R@ ! A:CELL+
1416259072 R@ ! A:CELL+ lang.Integer.floatValue§1416259072 VAL R@ ! A:CELL+
-1604556800 R@ ! A:CELL+ lang.Integer.hashCode§-1604556800 VAL R@ ! A:CELL+
-370906880 R@ ! A:CELL+ lang.Integer.intValue§-370906880 VAL R@ ! A:CELL+
1506831360 R@ ! A:CELL+ lang.Integer.longValue§1506831360 VAL R@ ! A:CELL+
715813632 R@ ! A:CELL+ lang.Integer.shortValue§715813632 VAL R@ ! A:CELL+
1621718016 R@ ! A:CELL+ lang.Integer.toString§1621718016 VAL R@ ! A:CELL+
A:DROP
