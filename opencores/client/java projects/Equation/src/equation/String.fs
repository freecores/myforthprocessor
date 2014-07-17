MODULENAME equation.String
(
* @(#)String.java	1.159 03/01/23
*
* Copyright 2003 Sun Microsystems, Inc. All rights reserved.
* SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
)  \ import java.io.ObjectStreamClass;
\ import java.io.ObjectStreamField;
\ import java.io.UnsupportedEncodingException;
\ import java.util.ArrayList;
\ import java.util.Comparator;
\ import java.util.regex.Matcher;
\ import java.util.regex.Pattern;
\ import java.util.regex.PatternSyntaxException;
( *
* The <code>String</code> class represents character strings. All
* string literals in Java programs, such as <code>"abc"</code>, are
* implemented as instances of this class.
* <p>
* Strings are constant; their values cannot be changed after they
* are created. String buffers support mutable strings.
* Because String objects are immutable they can be shared. For example:
* <p><blockquote><pre>
*     String str = "abc";
* </pre></blockquote><p>
* is equivalent to:
* <p><blockquote><pre>
*     char data[] = {'a', 'b', 'c'};
*     String str = new String(data);
* </pre></blockquote><p>
* Here are some more examples of how strings can be used:
* <p><blockquote><pre>
*     System.out.println("abc");
*     String cde = "cde";
*     System.out.println("abc" + cde);
*     String c = "abc".substring(2,3);
*     String d = cde.substring(1, 2);
* </pre></blockquote>
* <p>
* The class <code>String</code> includes methods for examining
* individual characters of the sequence, for comparing strings, for
* searching strings, for extracting substrings, and for creating a
* copy of a string with all characters translated to uppercase or to
* lowercase. Case mapping relies heavily on the information provided
* by the Unicode Consortium's Unicode 3.0 specification. The
* specification's UnicodeData.txt and SpecialCasing.txt files are
* used extensively to provide case mapping.
* <p>
* The Java language provides special support for the string
* concatenation operator (&nbsp;+&nbsp;), and for conversion of
* other objects to strings. String concatenation is implemented
* through the <code>StringBuffer</code> class and its
* <code>append</code> method.
* String conversions are implemented through the method
* <code>toString</code>, defined by <code>Object</code> and
* inherited by all classes in Java. For additional information on
* string concatenation and conversion, see Gosling, Joy, and Steele,
* <i>The Java Language Specification</i>.
*
* <p> Unless otherwise noted, passing a <tt>null</tt> argument to a constructor
* or method in this class will cause a {@link NullPointerException} to be
* thrown.
*
* @author  Lee Boynton
* @author  Arthur van Hoff
* @version 1.152, 02/01/03
* @see     java.lang.Object#toString()
* @see     java.lang.StringBuffer
* @see     java.lang.StringBuffer#append(boolean)
* @see     java.lang.StringBuffer#append(char)
* @see     java.lang.StringBuffer#append(char[])
* @see     java.lang.StringBuffer#append(char[], int, int)
* @see     java.lang.StringBuffer#append(double)
* @see     java.lang.StringBuffer#append(float)
* @see     java.lang.StringBuffer#append(int)
* @see     java.lang.StringBuffer#append(long)
* @see     java.lang.StringBuffer#append(java.lang.Object)
* @see     java.lang.StringBuffer#append(java.lang.String)
* @see     java.nio.charset.Charset
* @since   JDK1.0
)
( *
* Class String is special cased within the Serialization Stream Protocol.
*
* A String instance is written intially into an ObjectOutputStream in the
* following format:
* <pre>
*      <code>TC_STRING</code> (utf String)
* </pre>
* The String is written by method <code>DataOutput.writeUTF</code>.
* A new handle is generated to  refer to all future references to the
* string instance within the stream.
private static final ObjectStreamField[] serialPersistentFields =
new ObjectStreamField[0];
)  ( *
* Initializes a newly created <code>String</code> object so that it
* represents an empty character sequence.  Note that use of this
* constructor is unnecessary since Strings are immutable.
)

:LOCAL String.String§-1584442624
   1 VALLOCATE LOCAL §base0
   
   \ new statement
   0
   LOCALS §this |
   
   \ new statement
   Object§-1890496768.table 1827294976 EXECUTE-NEW
   
   \ new statement
   44 MALLOC DROP DUP DUP §base0 V! TO §this OVER
   IF
      OVER CELL+ @ 4 + OVER OVER ! DROP
   ENDIF
   CELL+ @ OVER
   OVER ! NIP
   String§1651856128.table OVER 12 + !
   1651834880 OVER 20 + !
   " String " OVER 16 + !
   1 OVER 24 + ! DROP
   
   \ new statement
   ( * The value is used for character storage.  )
   
   \ new statement
   ( * The offset is the first index of the storage that is used.  )
   
   \ new statement
   ( * The count is the number of characters in the String.  )
   
   \ new statement
   ( * Cache the hash code for the string  )
   
   \ new statement
   0
   §this CELL+ @ 40 + ( equation.String.hash )    !
   
   \ new statement
   
   1 VALLOCATE LOCAL §tempvar
   0
   131073 JavaArray§1352878592.table -227194368 EXECUTE-NEW
   §this CELL+ @ 28 + ( equation.String.value )    !
   
   §tempvar SETVTOP
   
   PURGE 1
   
   
   \ new statement
   §this DUP 0 V! ( return object )    
   \ new statement
   0 §break28210 LABEL
   
   \ new statement
   §base0 SETVTOP
   PURGE 2
   
   \ new statement
   DROP
;
(  Common private utility method used to bounds check the byte array
* and requested offset & length values used by the String(byte[],..)
* constructors.
)  (
private static void checkBounds(byte[] bytes, int offset, int length) {
if (length < 0)
throw new StringIndexOutOfBoundsException(length);
if (offset < 0)
throw new StringIndexOutOfBoundsException(offset);
if (offset > bytes.length - length)
throw new StringIndexOutOfBoundsException(offset + length);
} )  ( *
* Constructs a new <tt>String</tt> by decoding the specified subarray of
* bytes using the specified charset.  The length of the new
* <tt>String</tt> is a function of the charset, and hence may not be equal
* to the length of the subarray.
*
* <p> The behavior of this constructor when the given bytes are not valid
* in the given charset is unspecified.  The {@link
* java.nio.charset.CharsetDecoder} class should be used when more control
* over the decoding process is required.
*
* @param  bytes   the bytes to be decoded into characters
* @param  offset  the index of the first byte to decode
* @param  length  the number of bytes to decode
* @param  charsetName  the name of a supported
*                 {@link java.nio.charset.Charset </code>charset<code>}
* @throws  UnsupportedEncodingException
*          if the named charset is not supported
* @throws  IndexOutOfBoundsException
*          if the <tt>offset</tt> and <tt>length</tt> arguments
*          index characters outside the bounds of the <tt>bytes</tt>
*          array
* @since JDK1.1
public String(byte bytes[], int offset, int length, String charsetName)
throws UnsupportedEncodingException
{
if (charsetName == null)
throw new NullPointerException("charsetName");
checkBounds(bytes, offset, length);
value = StringCoding.decode(charsetName, bytes, offset, length);
count = value.length;
}
)  ( *
* Constructs a new <tt>String</tt> by decoding the specified array of
* bytes using the specified charset.  The length of the new
* <tt>String</tt> is a function of the charset, and hence may not be equal
* to the length of the byte array.
*
* <p> The behavior of this constructor when the given bytes are not valid
* in the given charset is unspecified.  The {@link
* java.nio.charset.CharsetDecoder} class should be used when more control
* over the decoding process is required.
*
* @param  bytes   the bytes to be decoded into characters
* @param  charsetName  the name of a supported
*                 {@link java.nio.charset.Charset </code>charset<code>}
*
* @exception  UnsupportedEncodingException
*             If the named charset is not supported
* @since      JDK1.1
public String(byte bytes[], String charsetName)
throws UnsupportedEncodingException
{
this(bytes, 0, bytes.length, charsetName);
}
)  ( *
* Constructs a new <tt>String</tt> by decoding the specified subarray of
* bytes using the platform's default charset.  The length of the new
* <tt>String</tt> is a function of the charset, and hence may not be equal
* to the length of the subarray.
*
* <p> The behavior of this constructor when the given bytes are not valid
* in the default charset is unspecified.  The {@link
* java.nio.charset.CharsetDecoder} class should be used when more control
* over the decoding process is required.
*
* @param  bytes   the bytes to be decoded into characters
* @param  offset  the index of the first byte to decode
* @param  length  the number of bytes to decode
* @throws IndexOutOfBoundsException
*         if the <code>offset</code> and the <code>length</code>
*         arguments index characters outside the bounds of the
*         <code>bytes</code> array
* @since  JDK1.1
)

:LOCAL String.String§-1626909952
   2 VALLOCATE LOCAL §base0
   LOCAL length
   LOCAL offset
   DUP 4 §base0 + V! LOCAL bytes
   
   \ new statement
   0
   LOCALS §this |
   
   \ new statement
   Object§-1890496768.table 1827294976 EXECUTE-NEW
   
   \ new statement
   44 MALLOC DROP DUP DUP §base0 V! TO §this OVER
   IF
      OVER CELL+ @ 4 + OVER OVER ! DROP
   ENDIF
   CELL+ @ OVER
   OVER ! NIP
   String§1651856128.table OVER 12 + !
   1651834880 OVER 20 + !
   " String " OVER 16 + !
   1 OVER 24 + ! DROP
   
   \ new statement
   ( * The value is used for character storage.  )
   
   \ new statement
   ( * The offset is the first index of the storage that is used.  )
   
   \ new statement
   ( * The count is the number of characters in the String.  )
   
   \ new statement
   ( * Cache the hash code for the string  )
   
   \ new statement
   0
   §this CELL+ @ 40 + ( equation.String.hash )    !
   
   \ new statement
   \ checkBounds(bytes, offset, length);
   
   
   \ new statement
   
   1 VALLOCATE LOCAL §tempvar
   length
   offset
   -
   1
   + 131073 JavaArray§1352878592.table -227194368 EXECUTE-NEW
   §this CELL+ @ 28 + ( equation.String.value )    !
   
   §tempvar SETVTOP
   
   PURGE 1
   
   
   \ new statement
   §this CELL+ @ 28 + ( equation.String.value ) @ CELL+ @ 28 + ( equation.JavaArray.length )    @
   §this CELL+ @ 36 + ( equation.String.count )    !
   
   \ new statement
   0
   LOCALS i |
   
   \ new statement
   0
   TO i
   
   \ new statement
   BEGIN
      
      \ new statement
      i
      §this CELL+ @ 36 + ( equation.String.count )       @
      <
      
      \ new statement
   WHILE
      
      \ new statement
      
      0
      LOCALS 0§ |
      
      i TO 0§
      bytes >R
      offset
      R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
      A:R@ C@
      A:DROP
      §this CELL+ @ 28 + ( equation.String.value ) @ >R
      0§
      R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
      H!
      
      PURGE 1
      
      
      \ new statement
      
      \ new statement
      i 1+ TO i
      
      \ new statement
      offset 1+ TO offset
      
      \ new statement
   REPEAT
   
   \ new statement
   0 §break28227 LABEL
   
   \ new statement
   
   PURGE 1
   
   \ new statement
   DROP
   
   \ new statement
   §this DUP 0 V! ( return object )    
   \ new statement
   0 §break28226 LABEL
   
   \ new statement
   
   
   
   §base0 SETVTOP
   PURGE 5
   
   \ new statement
   DROP
;
( *
* Allocates a new <code>String</code> constructed from a subarray
* of an array of 8-bit integer values.
* <p>
* The <code>offset</code> argument is the index of the first byte
* of the subarray, and the <code>count</code> argument specifies the
* length of the subarray.
* <p>
* Each <code>byte</code> in the subarray is converted to a
* <code>char</code> as specified in the method above.
*
* @deprecated This method does not properly convert bytes into characters.
* As of JDK&nbsp;1.1, the preferred way to do this is via the
* <code>String</code> constructors that take a charset name or that use
* the platform's default charset.
*
* @param      ascii     the bytes to be converted to characters.
* @param      hibyte    the top 8 bits of each 16-bit Unicode character.
* @param      offset    the initial offset.
* @param      count     the length.
* @exception  IndexOutOfBoundsException  if the <code>offset</code>
*               or <code>count</code> argument is invalid.
* @see        java.lang.String#String(byte[], int)
* @see        java.lang.String#String(byte[], int, int, java.lang.String)
* @see        java.lang.String#String(byte[], int, int)
* @see        java.lang.String#String(byte[], java.lang.String)
* @see        java.lang.String#String(byte[])
)

:LOCAL String.String§-1936633088
   3 VALLOCATE LOCAL §base0
   LOCAL count
   LOCAL offset
   LOCAL hibyte
   DUP 4 §base0 + V! LOCAL ascii
   
   \ new statement
   0 DUP
   LOCALS value §this |
   
   \ new statement
   Object§-1890496768.table 1827294976 EXECUTE-NEW
   
   \ new statement
   44 MALLOC DROP DUP DUP §base0 V! TO §this OVER
   IF
      OVER CELL+ @ 4 + OVER OVER ! DROP
   ENDIF
   CELL+ @ OVER
   OVER ! NIP
   String§1651856128.table OVER 12 + !
   1651834880 OVER 20 + !
   " String " OVER 16 + !
   1 OVER 24 + ! DROP
   
   \ new statement
   ( * The value is used for character storage.  )
   
   \ new statement
   ( * The offset is the first index of the storage that is used.  )
   
   \ new statement
   ( * The count is the number of characters in the String.  )
   
   \ new statement
   ( * Cache the hash code for the string  )
   
   \ new statement
   0
   §this CELL+ @ 40 + ( equation.String.hash )    !
   
   \ new statement
   \ checkBounds(ascii, offset, count);
   
   
   \ new statement
   
   1 VALLOCATE LOCAL §tempvar
   count 131073 JavaArray§1352878592.table -227194368 EXECUTE-NEW
   DUP 8 §base0 + V! TO value
   
   §tempvar SETVTOP
   
   PURGE 1
   
   
   \ new statement
   count
   §this CELL+ @ 36 + ( equation.String.count )    !
   
   \ new statement
   value
   §this CELL+ @ 28 + ( equation.String.value )    !
   
   \ new statement
   hibyte
   0=
   
   \ new statement
   IF
      
      \ new statement
      
      \ new statement
      0
      LOCALS i |
      
      \ new statement
      count
      TO i
      
      \ new statement
      BEGIN
         
         \ new statement
         i DUP 1- TO i
         0>
         
         \ new statement
      WHILE
         
         \ new statement
         
         0
         LOCALS 0§ |
         
         i TO 0§
         ascii >R
         i
         offset
         +
         R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
         C@
         255
         AND
         0FFFFH AND
         value >R
         0§
         R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
         H!
         
         PURGE 1
         
         
         \ new statement
         
         \ new statement
      REPEAT
      
      \ new statement
      0 §break28220 LABEL
      
      \ new statement
      
      PURGE 1
      
      \ new statement
      DROP
      
      \ new statement
      0 §break28219 LABEL
      
      \ new statement
      
      \ new statement
      DROP
      
      \ new statement
   ELSE
      
      \ new statement
      
      \ new statement
      hibyte  S>D
      8
      SHIFTL  D>S
      TO hibyte
      
      \ new statement
      0
      LOCALS i |
      
      \ new statement
      count
      TO i
      
      \ new statement
      BEGIN
         
         \ new statement
         i DUP 1- TO i
         0>
         
         \ new statement
      WHILE
         
         \ new statement
         
         0
         LOCALS 0§ |
         
         i TO 0§
         hibyte
         ascii >R
         i
         offset
         +
         R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
         C@
         255
         AND
         OR
         0FFFFH AND
         value >R
         0§
         R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
         H!
         
         PURGE 1
         
         
         \ new statement
         
         \ new statement
      REPEAT
      
      \ new statement
      0 §break28223 LABEL
      
      \ new statement
      
      PURGE 1
      
      \ new statement
      DROP
      
      \ new statement
      0 §break28222 LABEL
      
      \ new statement
      
      \ new statement
      DROP
      
      \ new statement
   ENDIF
   
   \ new statement
   §this DUP 0 V! ( return object )    
   \ new statement
   0 §break28218 LABEL
   
   \ new statement
   
   
   
   
   
   §base0 SETVTOP
   PURGE 7
   
   \ new statement
   DROP
;
( *
* Constructs a new <tt>String</tt> by decoding the specified array of
* bytes using the platform's default charset.  The length of the new
* <tt>String</tt> is a function of the charset, and hence may not be equal
* to the length of the byte array.
*
* <p> The behavior of this constructor when the given bytes are not valid
* in the default charset is unspecified.  The {@link
* java.nio.charset.CharsetDecoder} class should be used when more control
* over the decoding process is required.
*
* @param  bytes   the bytes to be decoded into characters
* @since  JDK1.1
)

:LOCAL String.String§-2016390400
   2 VALLOCATE LOCAL §base0
   DUP 4 §base0 + V! LOCAL bytes
   
   \ new statement
   0
   LOCALS §this |
   
   \ new statement
   bytes
   0
   bytes CELL+ @ 28 + ( equation.JavaArray.length )    @
   1
   -
   String§1651856128.table -1626909952 EXECUTE-NEW
   DUP §base0 V! TO §this
   
   \ new statement
   §this DUP 0 V! ( return object )    
   \ new statement
   0 §break28228 LABEL
   
   \ new statement
   
   §base0 SETVTOP
   PURGE 3
   
   \ new statement
   DROP
;
( *
* Allocates a new <code>String</code> so that it represents the
* sequence of characters currently contained in the character array
* argument. The contents of the character array are copied; subsequent
* modification of the character array does not affect the newly created
* string.
*
* @param  value   the initial value of the string.
)

:LOCAL String.String§-99003648
   2 VALLOCATE LOCAL §base0
   DUP 4 §base0 + V! LOCAL value
   
   \ new statement
   0
   LOCALS §this |
   
   \ new statement
   Object§-1890496768.table 1827294976 EXECUTE-NEW
   
   \ new statement
   44 MALLOC DROP DUP DUP §base0 V! TO §this OVER
   IF
      OVER CELL+ @ 4 + OVER OVER ! DROP
   ENDIF
   CELL+ @ OVER
   OVER ! NIP
   String§1651856128.table OVER 12 + !
   1651834880 OVER 20 + !
   " String " OVER 16 + !
   1 OVER 24 + ! DROP
   
   \ new statement
   ( * The value is used for character storage.  )
   
   \ new statement
   ( * The offset is the first index of the storage that is used.  )
   
   \ new statement
   ( * The count is the number of characters in the String.  )
   
   \ new statement
   ( * Cache the hash code for the string  )
   
   \ new statement
   0
   §this CELL+ @ 40 + ( equation.String.hash )    !
   
   \ new statement
   value CELL+ @ 28 + ( equation.JavaArray.length )    @
   §this CELL+ @ 36 + ( equation.String.count )    !
   
   \ new statement
   
   1 VALLOCATE LOCAL §tempvar
   §this CELL+ @ 36 + ( equation.String.count )    @ 131073 JavaArray§1352878592.table -227194368 EXECUTE-NEW
   §this CELL+ @ 28 + ( equation.String.value )    !
   
   §tempvar SETVTOP
   
   PURGE 1
   
   
   \ new statement
   value
   0
   §this CELL+ @ 28 + ( equation.String.value )    @
   0
   §this CELL+ @ 36 + ( equation.String.count )    @
   String.arraycopy§1134321920
   
   \ new statement
   §this DUP 0 V! ( return object )    
   \ new statement
   0 §break28216 LABEL
   
   \ new statement
   
   §base0 SETVTOP
   PURGE 3
   
   \ new statement
   DROP
;
( *
* Allocates a new <code>String</code> that contains characters from
* a subarray of the character array argument. The <code>offset</code>
* argument is the index of the first character of the subarray and
* the <code>count</code> argument specifies the length of the
* subarray. The contents of the subarray are copied; subsequent
* modification of the character array does not affect the newly
* created string.
*
* @param      value    array that is the source of characters.
* @param      offset   the initial offset.
* @param      count    the length.
* @exception  IndexOutOfBoundsException  if the <code>offset</code>
*               and <code>count</code> arguments index characters outside
*               the bounds of the <code>value</code> array.
)

:LOCAL String.String§1127437056
   2 VALLOCATE LOCAL §base0
   LOCAL count
   LOCAL offset
   DUP 4 §base0 + V! LOCAL value
   
   \ new statement
   0
   LOCALS §this |
   
   \ new statement
   Object§-1890496768.table 1827294976 EXECUTE-NEW
   
   \ new statement
   44 MALLOC DROP DUP DUP §base0 V! TO §this OVER
   IF
      OVER CELL+ @ 4 + OVER OVER ! DROP
   ENDIF
   CELL+ @ OVER
   OVER ! NIP
   String§1651856128.table OVER 12 + !
   1651834880 OVER 20 + !
   " String " OVER 16 + !
   1 OVER 24 + ! DROP
   
   \ new statement
   ( * The value is used for character storage.  )
   
   \ new statement
   ( * The offset is the first index of the storage that is used.  )
   
   \ new statement
   ( * The count is the number of characters in the String.  )
   
   \ new statement
   ( * Cache the hash code for the string  )
   
   \ new statement
   0
   §this CELL+ @ 40 + ( equation.String.hash )    !
   
   \ new statement
   ( if (offset < 0) {
   throw new StringIndexOutOfBoundsException(offset);
   }
   if (count < 0) {
   throw new StringIndexOutOfBoundsException(count);
   }
   // Note: offset or count might be near -1>>>1.
   if (offset > value.length - count) {
   throw new StringIndexOutOfBoundsException(offset + count);
   } )
   
   \ new statement
   
   1 VALLOCATE LOCAL §tempvar
   count 131073 JavaArray§1352878592.table -227194368 EXECUTE-NEW
   §this CELL+ @ 28 + ( equation.String.value )    !
   
   §tempvar SETVTOP
   
   PURGE 1
   
   
   \ new statement
   count
   §this CELL+ @ 36 + ( equation.String.count )    !
   
   \ new statement
   value
   offset
   §this CELL+ @ 28 + ( equation.String.value )    @
   0
   count
   String.arraycopy§1134321920
   
   \ new statement
   §this DUP 0 V! ( return object )    
   \ new statement
   0 §break28217 LABEL
   
   \ new statement
   
   
   
   §base0 SETVTOP
   PURGE 5
   
   \ new statement
   DROP
;
( *
* Allocates a new <code>String</code> containing characters
* constructed from an array of 8-bit integer values. Each character
* <i>c</i>in the resulting string is constructed from the
* corresponding component <i>b</i> in the byte array such that:
* <p><blockquote><pre>
*     <b><i>c</i></b> == (char)(((hibyte &amp; 0xff) &lt;&lt; 8)
*                         | (<b><i>b</i></b> &amp; 0xff))
* </pre></blockquote>
*
* @deprecated This method does not properly convert bytes into characters.
* As of JDK&nbsp;1.1, the preferred way to do this is via the
* <code>String</code> constructors that take a charset name or
* that use the platform's default charset.
*
* @param      ascii    the bytes to be converted to characters.
* @param      hibyte   the top 8 bits of each 16-bit Unicode character.
* @see        java.lang.String#String(byte[], int, int, java.lang.String)
* @see        java.lang.String#String(byte[], int, int)
* @see        java.lang.String#String(byte[], java.lang.String)
* @see        java.lang.String#String(byte[])
)

:LOCAL String.String§1136743168
   2 VALLOCATE LOCAL §base0
   LOCAL hibyte
   DUP 4 §base0 + V! LOCAL ascii
   
   \ new statement
   0
   LOCALS §this |
   
   \ new statement
   ascii
   hibyte
   0
   ascii CELL+ @ 28 + ( equation.JavaArray.length )    @
   String§1651856128.table -1936633088 EXECUTE-NEW
   DUP §base0 V! TO §this
   
   \ new statement
   §this DUP 0 V! ( return object )    
   \ new statement
   0 §break28225 LABEL
   
   \ new statement
   
   
   §base0 SETVTOP
   PURGE 4
   
   \ new statement
   DROP
;
( *
* Allocates a new string that contains the sequence of characters
* currently contained in the string buffer argument. The contents of
* the string buffer are copied; subsequent modification of the string
* buffer does not affect the newly created string.
*
* @param   buffer   a <code>StringBuffer</code>.
public String (StringBuffer buffer) {
synchronized(buffer) {
buffer.setShared();
this.value = buffer.getValue();
this.offset = 0;
this.count = buffer.length();
}
}
)  \  Package private constructor which shares value array for speed.


:LOCAL String.String§1508463360
   2 VALLOCATE LOCAL §base0
   DUP 4 §base0 + V! LOCAL value
   LOCAL count
   LOCAL offset
   
   \ new statement
   0
   LOCALS §this |
   
   \ new statement
   Object§-1890496768.table 1827294976 EXECUTE-NEW
   
   \ new statement
   44 MALLOC DROP DUP DUP §base0 V! TO §this OVER
   IF
      OVER CELL+ @ 4 + OVER OVER ! DROP
   ENDIF
   CELL+ @ OVER
   OVER ! NIP
   String§1651856128.table OVER 12 + !
   1651834880 OVER 20 + !
   " String " OVER 16 + !
   1 OVER 24 + ! DROP
   
   \ new statement
   ( * The value is used for character storage.  )
   
   \ new statement
   ( * The offset is the first index of the storage that is used.  )
   
   \ new statement
   ( * The count is the number of characters in the String.  )
   
   \ new statement
   ( * Cache the hash code for the string  )
   
   \ new statement
   0
   §this CELL+ @ 40 + ( equation.String.hash )    !
   
   \ new statement
   value
   §this CELL+ @ 28 + ( equation.String.value )    !
   
   \ new statement
   offset
   §this CELL+ @ 32 + ( equation.String.offset )    !
   
   \ new statement
   count
   §this CELL+ @ 36 + ( equation.String.count )    !
   
   \ new statement
   §this DUP 0 V! ( return object )    
   \ new statement
   0 §break28229 LABEL
   
   \ new statement
   
   
   
   §base0 SETVTOP
   PURGE 5
   
   \ new statement
   DROP
;
( *
* Initializes a newly created <code>String</code> object so that it
* represents the same sequence of characters as the argument; in other
* words, the newly created string is a copy of the argument string. Unless
* an explicit copy of <code>original</code> is needed, use of this
* constructor is unnecessary since Strings are immutable.
*
* @param   original   a <code>String</code>.
)

:LOCAL String.String§1844794112
   2 VALLOCATE LOCAL §base0
   DUP 4 §base0 + V! LOCAL original
   
   \ new statement
   0
   LOCALS §this |
   
   \ new statement
   Object§-1890496768.table 1827294976 EXECUTE-NEW
   
   \ new statement
   44 MALLOC DROP DUP DUP §base0 V! TO §this OVER
   IF
      OVER CELL+ @ 4 + OVER OVER ! DROP
   ENDIF
   CELL+ @ OVER
   OVER ! NIP
   String§1651856128.table OVER 12 + !
   1651834880 OVER 20 + !
   " String " OVER 16 + !
   1 OVER 24 + ! DROP
   
   \ new statement
   ( * The value is used for character storage.  )
   
   \ new statement
   ( * The offset is the first index of the storage that is used.  )
   
   \ new statement
   ( * The count is the number of characters in the String.  )
   
   \ new statement
   ( * Cache the hash code for the string  )
   
   \ new statement
   0
   §this CELL+ @ 40 + ( equation.String.hash )    !
   
   \ new statement
   original CELL+ @ 36 + ( equation.String.count )    @
   §this CELL+ @ 36 + ( equation.String.count )    !
   
   \ new statement
   original CELL+ @ 28 + ( equation.String.value ) @ CELL+ @ 28 + ( equation.JavaArray.length )    @
   §this CELL+ @ 36 + ( equation.String.count )    @
   >
   
   \ new statement
   IF
      
      \ new statement
      
      \ new statement
      \  The array representing the String is bigger than the new
      \  String itself.  Perhaps this constructor is being called
      \  in order to trim the baggage, so make a copy of the array.
      
      
      \ new statement
      
      1 VALLOCATE LOCAL §tempvar
      §this CELL+ @ 36 + ( equation.String.count )       @ 131073 JavaArray§1352878592.table -227194368 EXECUTE-NEW
      §this CELL+ @ 28 + ( equation.String.value )       !
      
      §tempvar SETVTOP
      
      PURGE 1
      
      
      \ new statement
      original CELL+ @ 28 + ( equation.String.value )       @
      original CELL+ @ 32 + ( equation.String.offset )       @
      §this CELL+ @ 28 + ( equation.String.value )       @
      0
      §this CELL+ @ 36 + ( equation.String.count )       @
      String.arraycopy§1134321920
      
      \ new statement
      0 §break28212 LABEL
      
      \ new statement
      
      \ new statement
      DROP
      
      \ new statement
   ELSE
      
      \ new statement
      
      \ new statement
      \  The array representing the String is the same
      \  size as the String, so no point in making a copy.
      
      
      \ new statement
      original CELL+ @ 28 + ( equation.String.value )       @
      §this CELL+ @ 28 + ( equation.String.value )       !
      
      \ new statement
      0 §break28213 LABEL
      
      \ new statement
      
      \ new statement
      DROP
      
      \ new statement
   ENDIF
   
   \ new statement
   §this DUP 0 V! ( return object )    
   \ new statement
   0 §break28211 LABEL
   
   \ new statement
   
   §base0 SETVTOP
   PURGE 3
   
   \ new statement
   DROP
;


: String.arraycopy§1134321920
   2 VALLOCATE LOCAL §base0
   LOCAL count
   LOCAL doffset
   DUP 0 §base0 + V! LOCAL dvalue
   LOCAL soffset
   DUP 4 §base0 + V! LOCAL svalue
   
   \ new statement
   
   \ new statement
   count
   0>
   
   \ new statement
   IF
      
      \ new statement
      0 DUP
      LOCALS dest source |
      
      \ new statement
      svalue
      >R
      soffset
      R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
      TO source
      
      \ new statement
      dvalue
      >R
      doffset
      R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
      TO dest
      
      \ new statement
      source dest count 2* MOVE
      
      \ new statement
      0 §break28215 LABEL
      
      \ new statement
      
      
      PURGE 2
      
      \ new statement
      DROP
      
      \ new statement
   ENDIF
   
   \ new statement
   0 §break28214 LABEL
   
   \ new statement
   
   
   
   
   
   §base0 SETVTOP
   PURGE 6
   
   \ new statement
   DROP
;
( *
* Returns the character at the specified index. An index ranges
* from <code>0</code> to <code>length() - 1</code>. The first character
* of the sequence is at index <code>0</code>, the next at index
* <code>1</code>, and so on, as for array indexing.
*
* @param      index   the index of the character.
* @return     the character at the specified index of this string.
*             The first character is at index <code>0</code>.
* @exception  IndexOutOfBoundsException  if the <code>index</code>
*             argument is negative or not less than the length of this
*             string.
)

:LOCAL String.charAt§-1014275328
   1 VALLOCATE LOCAL §base0
   DUP 0 §base0 + V! LOCAL §this
   LOCAL index
   
   \ new statement
   
   \ new statement
   ( if ((index < 0) || (index >= count)) {
   throw new StringIndexOutOfBoundsException(index);
   } )
   
   \ new statement
   §this CELL+ @ 28 + ( equation.String.value ) @ >R
   index
   §this CELL+ @ 32 + ( equation.String.offset )    @
   +
   R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
   H@
   0 §break28231 BRANCH
   
   \ new statement
   0 §break28231 LABEL
   
   \ new statement
   
   §base0 SETVTOP
   PURGE 3
   
   \ new statement
   DROP
;
( *
* Compares this String to another Object.  If the Object is a String,
* this function behaves like <code>compareTo(String)</code>.  Otherwise,
* it throws a <code>ClassCastException</code> (as Strings are comparable
* only to other Strings).
*
* @param   o the <code>Object</code> to be compared.
* @return  the value <code>0</code> if the argument is a string
*		lexicographically equal to this string; a value less than
*		<code>0</code> if the argument is a string lexicographically
*		greater than this string; and a value greater than
*		<code>0</code> if the argument is a string lexicographically
*		less than this string.
* @exception <code>ClassCastException</code> if the argument is not a
*		  <code>String</code>.
* @see     java.lang.Comparable
* @since   1.2
)

:LOCAL String.compareTo§-879140096
   2 VALLOCATE LOCAL §base0
   DUP 4 §base0 + V! LOCAL §this
   DUP 0 §base0 + V! LOCAL o
   
   \ new statement
   
   \ new statement
   §this >R
   o
   R> 1883005696 TRUE ( equation.String.compareTo§1883005696 ) EXECUTE-METHOD
   0 §break28254 BRANCH
   
   \ new statement
   0 §break28254 LABEL
   
   \ new statement
   
   §base0 SETVTOP
   PURGE 3
   
   \ new statement
   DROP
;
( *
* Compares two strings lexicographically.
* The comparison is based on the Unicode value of each character in
* the strings. The character sequence represented by this
* <code>String</code> object is compared lexicographically to the
* character sequence represented by the argument string. The result is
* a negative integer if this <code>String</code> object
* lexicographically precedes the argument string. The result is a
* positive integer if this <code>String</code> object lexicographically
* follows the argument string. The result is zero if the strings
* are equal; <code>compareTo</code> returns <code>0</code> exactly when
* the {@link #equals(Object)} method would return <code>true</code>.
* <p>
* This is the definition of lexicographic ordering. If two strings are
* different, then either they have different characters at some index
* that is a valid index for both strings, or their lengths are different,
* or both. If they have different characters at one or more index
* positions, let <i>k</i> be the smallest such index; then the string
* whose character at position <i>k</i> has the smaller value, as
* determined by using the &lt; operator, lexicographically precedes the
* other string. In this case, <code>compareTo</code> returns the
* difference of the two character values at position <code>k</code> in
* the two string -- that is, the value:
* <blockquote><pre>
* this.charAt(k)-anotherString.charAt(k)
* </pre></blockquote>
* If there is no index position at which they differ, then the shorter
* string lexicographically precedes the longer string. In this case,
* <code>compareTo</code> returns the difference of the lengths of the
* strings -- that is, the value:
* <blockquote><pre>
* this.length()-anotherString.length()
* </pre></blockquote>
*
* @param   anotherString   the <code>String</code> to be compared.
* @return  the value <code>0</code> if the argument string is equal to
*          this string; a value less than <code>0</code> if this string
*          is lexicographically less than the string argument; and a
*          value greater than <code>0</code> if this string is
*          lexicographically greater than the string argument.
)

:LOCAL String.compareTo§1883005696
   4 VALLOCATE LOCAL §base0
   DUP 4 §base0 + V! LOCAL §this
   DUP 0 §base0 + V! LOCAL anotherString
   
   \ new statement
   0 DUP DUP DUP DUP DUP DUP
   LOCALS i j len1 len2 n v1 v2 |
   
   \ new statement
   §this CELL+ @ 36 + ( equation.String.count )    @
   TO len1
   
   \ new statement
   anotherString CELL+ @ 36 + ( equation.String.count )    @
   TO len2
   
   \ new statement
   len1
   len2
   lang.Math.min§61172992
   TO n
   
   \ new statement
   §this CELL+ @ 28 + ( equation.String.value )    @
   DUP 8 §base0 + V! TO v1
   
   \ new statement
   anotherString CELL+ @ 28 + ( equation.String.value )    @
   DUP 12 §base0 + V! TO v2
   
   \ new statement
   §this CELL+ @ 32 + ( equation.String.offset )    @
   TO i
   
   \ new statement
   anotherString CELL+ @ 32 + ( equation.String.offset )    @
   TO j
   
   \ new statement
   i
   j
   =
   
   \ new statement
   IF
      
      \ new statement
      0 DUP
      LOCALS k lim |
      
      \ new statement
      i
      TO k
      
      \ new statement
      n
      i
      +
      TO lim
      
      \ new statement
      0 DUP
      LOCALS c1 c2 |
      
      \ new statement
      BEGIN
         
         \ new statement
         k
         lim
         <
         
         \ new statement
      WHILE
         
         \ new statement
         v1 >R
         k
         R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
         H@
         TO c1
         
         \ new statement
         v2 >R
         k
         R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
         H@
         TO c2
         
         \ new statement
         c1
         c2
         <>
         
         \ new statement
         IF
            
            \ new statement
            
            \ new statement
            c1
            c2
            -
            196611 §break28249 BRANCH
            
            \ new statement
            0 §break28249 LABEL
            
            \ new statement
            
            \ new statement
            0>!
            IF
               10001H - 0<! §break28247 0BRANCH
            ENDIF
            DROP
            
            \ new statement
         ENDIF
         
         \ new statement
         k 1+ TO k
         
         \ new statement
         
         \ new statement
      REPEAT
      
      \ new statement
      0 §break28247 LABEL
      
      \ new statement
      
      
      PURGE 2
      
      \ new statement
      0>!
      IF
         10001H - §break28246 BRANCH
      ENDIF
      DROP
      
      \ new statement
      0 §break28246 LABEL
      
      \ new statement
      
      
      PURGE 2
      
      \ new statement
      0>!
      IF
         10001H - §break28245 BRANCH
      ENDIF
      DROP
      
      \ new statement
   ELSE
      
      \ new statement
      
      \ new statement
      0 DUP
      LOCALS c1 c2 |
      
      \ new statement
      BEGIN
         
         \ new statement
         n DUP 1- TO n
         0<>
         
         \ new statement
      WHILE
         
         \ new statement
         v1 >R
         i DUP 1+ TO i
         R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
         H@
         TO c1
         
         \ new statement
         v2 >R
         j DUP 1+ TO j
         R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
         H@
         TO c2
         
         \ new statement
         c1
         c2
         <>
         
         \ new statement
         IF
            
            \ new statement
            
            \ new statement
            c1
            c2
            -
            196611 §break28253 BRANCH
            
            \ new statement
            0 §break28253 LABEL
            
            \ new statement
            
            \ new statement
            0>!
            IF
               10001H - 0<! §break28251 0BRANCH
            ENDIF
            DROP
            
            \ new statement
         ENDIF
         
         \ new statement
         
         \ new statement
      REPEAT
      
      \ new statement
      0 §break28251 LABEL
      
      \ new statement
      
      
      PURGE 2
      
      \ new statement
      0>!
      IF
         10001H - §break28250 BRANCH
      ENDIF
      DROP
      
      \ new statement
      0 §break28250 LABEL
      
      \ new statement
      
      \ new statement
      0>!
      IF
         10001H - §break28245 BRANCH
      ENDIF
      DROP
      
      \ new statement
   ENDIF
   
   \ new statement
   len1
   len2
   -
   0 §break28245 BRANCH
   
   \ new statement
   0 §break28245 LABEL
   
   \ new statement
   
   
   
   
   
   
   
   
   §base0 SETVTOP
   PURGE 10
   
   \ new statement
   DROP
;
( *
* Compares two strings lexicographically, ignoring case
* differences. This method returns an integer whose sign is that of
* calling <code>compareTo</code> with normalized versions of the strings
* where case differences have been eliminated by calling
* <code>Character.toLowerCase(Character.toUpperCase(character))</code> on
* each character.
* <p>
* Note that this method does <em>not</em> take locale into account,
* and will result in an unsatisfactory ordering for certain locales.
* The java.text package provides <em>collators</em> to allow
* locale-sensitive ordering.
*
* @param   str   the <code>String</code> to be compared.
* @return  a negative integer, zero, or a positive integer as the
*		the specified String is greater than, equal to, or less
*		than this String, ignoring case considerations.
* @see     java.text.Collator#compare(String, String)
* @since   1.2
)

:LOCAL String.compareToIgnoreCase§-466132224
   2 VALLOCATE LOCAL §base0
   DUP 4 §base0 + V! LOCAL §this
   DUP 0 §base0 + V! LOCAL str
   
   \ new statement
   
   \ new statement
   String.CASE_INSENSITIVE_ORDER  @ >R
   §this
   str
   R> 2079286016 TRUE ( equation.Comparator.compare§2079286016 ) EXECUTE-METHOD
   0 §break28264 BRANCH
   
   \ new statement
   0 §break28264 LABEL
   
   \ new statement
   
   §base0 SETVTOP
   PURGE 3
   
   \ new statement
   DROP
;
( *
* Concatenates the specified string to the end of this string.
* <p>
* If the length of the argument string is <code>0</code>, then this
* <code>String</code> object is returned. Otherwise, a new
* <code>String</code> object is created, representing a character
* sequence that is the concatenation of the character sequence
* represented by this <code>String</code> object and the character
* sequence represented by the argument string.<p>
* Examples:
* <blockquote><pre>
* "cares".concat("s") returns "caress"
* "to".concat("get").concat("her") returns "together"
* </pre></blockquote>
*
* @param   str   the <code>String</code> that is concatenated to the end
*                of this <code>String</code>.
* @return  a string that represents the concatenation of this object's
*          characters followed by the string argument's characters.
)

:LOCAL String.concat§-1461427456
   3 VALLOCATE LOCAL §base0
   DUP 4 §base0 + V! LOCAL §this
   DUP 0 §base0 + V! LOCAL str
   
   \ new statement
   0 DUP
   LOCALS buf otherLen |
   
   \ new statement
   str >R
   R> 188050432 TRUE ( equation.String.length§188050432 ) EXECUTE-METHOD
   TO otherLen
   
   \ new statement
   otherLen
   0=
   
   \ new statement
   IF
      
      \ new statement
      
      \ new statement
      §this
      DUP 0 V!
      65537 §break28333 BRANCH
      
      \ new statement
      0 §break28333 LABEL
      
      \ new statement
      
      \ new statement
      0>!
      IF
         10001H - §break28332 BRANCH
      ENDIF
      DROP
      
      \ new statement
   ENDIF
   
   \ new statement
   
   1 VALLOCATE LOCAL §tempvar
   §this CELL+ @ 36 + ( equation.String.count )    @
   otherLen
   + 131073 JavaArray§1352878592.table -227194368 EXECUTE-NEW
   DUP 8 §base0 + V! TO buf
   
   §tempvar SETVTOP
   
   PURGE 1
   
   
   \ new statement
   §this >R
   0
   §this CELL+ @ 36 + ( equation.String.count )    @
   buf
   0
   R> -53713152 TRUE ( equation.String.getChars§-53713152 ) EXECUTE-METHOD
   
   \ new statement
   str >R
   0
   otherLen
   buf
   §this CELL+ @ 36 + ( equation.String.count )    @
   R> -53713152 TRUE ( equation.String.getChars§-53713152 ) EXECUTE-METHOD
   
   \ new statement
   buf
   0
   §this CELL+ @ 36 + ( equation.String.count )    @
   otherLen
   +
   String§1651856128.table 1127437056 EXECUTE-NEW
   DUP 0 V!
   0 §break28332 BRANCH
   
   \ new statement
   0 §break28332 LABEL
   
   \ new statement
   
   
   
   §base0 SETVTOP
   PURGE 5
   
   \ new statement
   DROP
;
( *
* Returns a String that represents the character sequence in the
* array specified.
*
* @param   data   the character array.
* @return  a <code>String</code> that contains the characters of the
*          character array.
)

: String.copyValueOf§1162502912
   1 VALLOCATE LOCAL §base0
   DUP 0 §base0 + V! LOCAL data
   
   \ new statement
   
   \ new statement
   data
   0
   data CELL+ @ 28 + ( equation.JavaArray.length )    @
   String.copyValueOf§471425792
   DUP 0 V!
   0 §break28371 BRANCH
   
   \ new statement
   0 §break28371 LABEL
   
   \ new statement
   
   §base0 SETVTOP
   PURGE 2
   
   \ new statement
   DROP
;
( *
* Returns a String that represents the character sequence in the
* array specified.
*
* @param   data     the character array.
* @param   offset   initial offset of the subarray.
* @param   count    length of the subarray.
* @return  a <code>String</code> that contains the characters of the
*          specified subarray of the character array.
)

: String.copyValueOf§471425792
   1 VALLOCATE LOCAL §base0
   LOCAL count
   LOCAL offset
   DUP 0 §base0 + V! LOCAL data
   
   \ new statement
   
   \ new statement
   \  All public String constructors now copy the data.
   
   
   \ new statement
   data
   offset
   count
   String§1651856128.table 1127437056 EXECUTE-NEW
   DUP 0 V!
   0 §break28370 BRANCH
   
   \ new statement
   0 §break28370 LABEL
   
   \ new statement
   
   
   
   §base0 SETVTOP
   PURGE 4
   
   \ new statement
   DROP
;
( *
* Tests if this string ends with the specified suffix.
*
* @param   suffix   the suffix.
* @return  <code>true</code> if the character sequence represented by the
*          argument is a suffix of the character sequence represented by
*          this object; <code>false</code> otherwise. Note that the
*          result will be <code>true</code> if the argument is the
*          empty string or is equal to this <code>String</code> object
*          as determined by the {@link #equals(Object)} method.
)

:LOCAL String.endsWith§-676764416
   2 VALLOCATE LOCAL §base0
   DUP 4 §base0 + V! LOCAL §this
   DUP 0 §base0 + V! LOCAL suffix
   
   \ new statement
   
   \ new statement
   §this >R
   suffix
   §this CELL+ @ 36 + ( equation.String.count )    @
   suffix CELL+ @ 36 + ( equation.String.count )    @
   -
   R> -953322752 TRUE ( equation.String.startsWith§-953322752 ) EXECUTE-METHOD
   0 §break28284 BRANCH
   
   \ new statement
   0 §break28284 LABEL
   
   \ new statement
   
   §base0 SETVTOP
   PURGE 3
   
   \ new statement
   DROP
;
( *
* Compares this string to the specified object.
* The result is <code>true</code> if and only if the argument is not
* <code>null</code> and is a <code>String</code> object that represents
* the same sequence of characters as this object.
*
* @param   anObject   the object to compare this <code>String</code>
*                     against.
* @return  <code>true</code> if the <code>String </code>are equal;
*          <code>false</code> otherwise.
* @see     java.lang.String#compareTo(java.lang.String)
* @see     java.lang.String#equalsIgnoreCase(java.lang.String)
)

:LOCAL String.equals§-240098048
   2 VALLOCATE LOCAL §base0
   DUP 4 §base0 + V! LOCAL §this
   DUP 0 §base0 + V! LOCAL anObject
   
   \ new statement
   
   \ new statement
   §this
   anObject
   =
   
   \ new statement
   IF
      
      \ new statement
      
      \ new statement
      TRUE
      65537 §break28239 BRANCH
      
      \ new statement
      0 §break28239 LABEL
      
      \ new statement
      
      \ new statement
      0>!
      IF
         10001H - §break28238 BRANCH
      ENDIF
      DROP
      
      \ new statement
   ENDIF
   
   \ new statement
   anObject
   " String " INSTANCEOF
   
   \ new statement
   IF
      
      \ new statement
      1 VALLOCATE LOCAL §base1
      0 DUP
      LOCALS anotherString n |
      
      \ new statement
      anObject
      DUP §base1 V! TO anotherString
      
      \ new statement
      §this CELL+ @ 36 + ( equation.String.count )       @
      TO n
      
      \ new statement
      n
      anotherString CELL+ @ 36 + ( equation.String.count )       @
      =
      
      \ new statement
      IF
         
         \ new statement
         2 VALLOCATE LOCAL §base2
         0 DUP DUP DUP
         LOCALS i j v1 v2 |
         
         \ new statement
         §this CELL+ @ 28 + ( equation.String.value )          @
         DUP §base2 V! TO v1
         
         \ new statement
         anotherString CELL+ @ 28 + ( equation.String.value )          @
         DUP 4 §base2 + V! TO v2
         
         \ new statement
         §this CELL+ @ 32 + ( equation.String.offset )          @
         TO i
         
         \ new statement
         anotherString CELL+ @ 32 + ( equation.String.offset )          @
         TO j
         
         \ new statement
         
         \ new statement
         BEGIN
            
            \ new statement
            n DUP 1- TO n
            0<>
            
            \ new statement
         WHILE
            
            \ new statement
            v1 >R
            i DUP 1+ TO i
            R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
            H@
            v2 >R
            j DUP 1+ TO j
            R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
            H@
            <>
            
            \ new statement
            IF
               
               \ new statement
               FALSE
               196611 §break28242 BRANCH
               
               \ new statement
            ENDIF
            
            \ new statement
            
            \ new statement
         REPEAT
         
         \ new statement
         0 §break28242 LABEL
         
         \ new statement
         
         \ new statement
         0>!
         IF
            10001H - §break28241 BRANCH
         ENDIF
         DROP
         
         \ new statement
         TRUE
         131074 §break28241 BRANCH
         
         \ new statement
         0 §break28241 LABEL
         
         \ new statement
         
         
         
         
         §base2 SETVTOP
         PURGE 5
         
         \ new statement
         0>!
         IF
            10001H - §break28240 BRANCH
         ENDIF
         DROP
         
         \ new statement
      ENDIF
      
      \ new statement
      0 §break28240 LABEL
      
      \ new statement
      
      
      §base1 SETVTOP
      PURGE 3
      
      \ new statement
      0>!
      IF
         10001H - §break28238 BRANCH
      ENDIF
      DROP
      
      \ new statement
   ENDIF
   
   \ new statement
   FALSE
   0 §break28238 BRANCH
   
   \ new statement
   0 §break28238 LABEL
   
   \ new statement
   
   §base0 SETVTOP
   PURGE 3
   
   \ new statement
   DROP
;
( *
* Returns <tt>true</tt> if and only if this <tt>String</tt> represents
* the same sequence of characters as the specified <tt>StringBuffer</tt>.
*
* @param   sb         the <tt>StringBuffer</tt> to compare to.
* @return  <tt>true</tt> if and only if this <tt>String</tt> represents
*          the same sequence of characters as the specified
*          <tt>StringBuffer</tt>, otherwise <tt>false</tt>.
* @since 1.4
public boolean contentEquals(StringBuffer sb) {
synchronized(sb) {
if (count != sb.length())
return false;
char v1[] = value;
char v2[] = sb.getValue();
int i = offset;
int j = 0;
int n = count;
while (n-- != 0) {
if (v1[i++] != v2[j++])
return false;
}
}
return true;
}
)  ( *
* Compares this <code>String</code> to another <code>String</code>,
* ignoring case considerations.  Two strings are considered equal
* ignoring case if they are of the same length, and corresponding
* characters in the two strings are equal ignoring case.
* <p>
* Two characters <code>c1</code> and <code>c2</code> are considered
* the same, ignoring case if at least one of the following is true:
* <ul><li>The two characters are the same (as compared by the
* <code>==</code> operator).
* <li>Applying the method {@link java.lang.Character#toUpperCase(char)}
* to each character produces the same result.
* <li>Applying the method {@link java.lang.Character#toLowerCase(char)}
* to each character produces the same result.</ul>
*
* @param   anotherString   the <code>String</code> to compare this
*                          <code>String</code> against.
* @return  <code>true</code> if the argument is not <code>null</code>
*          and the <code>String</code>s are equal,
*          ignoring case; <code>false</code> otherwise.
* @see     #equals(Object)
* @see     java.lang.Character#toLowerCase(char)
* @see java.lang.Character#toUpperCase(char)
)

:LOCAL String.equalsIgnoreCase§-518953728
   2 VALLOCATE LOCAL §base0
   DUP 4 §base0 + V! LOCAL §this
   DUP 0 §base0 + V! LOCAL anotherString
   
   \ new statement
   
   \ new statement
   §this
   anotherString
   =
   
   IF
      TRUE
      
   ELSE
      anotherString
      0<>
      and_31407 0BRANCH! DROP
      anotherString CELL+ @ 36 + ( equation.String.count )       @
      §this CELL+ @ 36 + ( equation.String.count )       @
      =
      and_31407 0BRANCH! DROP
      §this >R
      TRUE
      0
      anotherString
      0
      §this CELL+ @ 36 + ( equation.String.count )       @
      R> 2060349952 TRUE ( equation.String.regionMatches§2060349952 ) EXECUTE-METHOD
      and_31407 LABEL
      
   ENDIF
   0 §break28244 BRANCH
   
   \ new statement
   0 §break28244 LABEL
   
   \ new statement
   
   §base0 SETVTOP
   PURGE 3
   
   \ new statement
   DROP
;
( *
* Copies characters from this string into the destination byte
* array. Each byte receives the 8 low-order bits of the
* corresponding character. The eight high-order bits of each character
* are not copied and do not participate in the transfer in any way.
* <p>
* The first character to be copied is at index <code>srcBegin</code>;
* the last character to be copied is at index <code>srcEnd-1</code>.
* The total number of characters to be copied is
* <code>srcEnd-srcBegin</code>. The characters, converted to bytes,
* are copied into the subarray of <code>dst</code> starting at index
* <code>dstBegin</code> and ending at index:
* <p><blockquote><pre>
*     dstbegin + (srcEnd-srcBegin) - 1
* </pre></blockquote>
*
* @deprecated This method does not properly convert characters into bytes.
* As of JDK&nbsp;1.1, the preferred way to do this is via the
* the <code>getBytes()</code> method, which uses the platform's default
* charset.
*
* @param      srcBegin   index of the first character in the string
*                        to copy.
* @param      srcEnd     index after the last character in the string
*                        to copy.
* @param      dst        the destination array.
* @param      dstBegin   the start offset in the destination array.
* @exception IndexOutOfBoundsException if any of the following
*            is true:
*           <ul><li><code>srcBegin</code> is negative
*           <li><code>srcBegin</code> is greater than <code>srcEnd</code>
*           <li><code>srcEnd</code> is greater than the length of this
*            String
*           <li><code>dstBegin</code> is negative
*           <li><code>dstBegin+(srcEnd-srcBegin)</code> is larger than
*            <code>dst.length</code></ul>
)

:LOCAL String.getBytes§-457480448
   3 VALLOCATE LOCAL §base0
   DUP 4 §base0 + V! LOCAL §this
   LOCAL dstBegin
   DUP 0 §base0 + V! LOCAL dst
   LOCAL srcEnd
   LOCAL srcBegin
   
   \ new statement
   0 DUP DUP DUP
   LOCALS i j n val |
   
   \ new statement
   ( if (srcBegin < 0) {
   throw new StringIndexOutOfBoundsException(srcBegin);
   }
   if (srcEnd > count) {
   throw new StringIndexOutOfBoundsException(srcEnd);
   }
   if (srcBegin > srcEnd) {
   throw new StringIndexOutOfBoundsException(srcEnd - srcBegin);
   } )
   
   \ new statement
   dstBegin
   TO j
   
   \ new statement
   §this CELL+ @ 32 + ( equation.String.offset )    @
   srcEnd
   +
   TO n
   
   \ new statement
   §this CELL+ @ 32 + ( equation.String.offset )    @
   srcBegin
   +
   TO i
   
   \ new statement
   §this CELL+ @ 28 + ( equation.String.value )    @
   DUP 8 §base0 + V! TO val
   
   \ new statement
   (  avoid getfield opcode  )
   
   \ new statement
   
   \ new statement
   BEGIN
      
      \ new statement
      i
      n
      <
      
      \ new statement
   WHILE
      
      \ new statement
      
      0
      LOCALS 0§ |
      
      j DUP 1+ TO j
      TO 0§
      val >R
      i DUP 1+ TO i
      R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
      A:R@ H@  127 U>! NIP
      IF
         0FFFFFF00H OR
      ENDIF
      A:DROP
      dst >R
      0§
      R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
      C!
      
      PURGE 1
      
      
      \ new statement
      
      \ new statement
   REPEAT
   
   \ new statement
   0 §break28234 LABEL
   
   \ new statement
   
   \ new statement
   DROP
   
   \ new statement
   0 §break28233 LABEL
   
   \ new statement
   
   
   
   
   
   
   
   
   §base0 SETVTOP
   PURGE 10
   
   \ new statement
   DROP
;
( *
* Encodes this <tt>String</tt> into a sequence of bytes using the
* named charset, storing the result into a new byte array.
*
* <p> The behavior of this method when this string cannot be encoded in
* the given charset is unspecified.  The {@link
* java.nio.charset.CharsetEncoder} class should be used when more control
* over the encoding process is required.
*
* @param  charsetName
*         the name of a supported
*         {@link java.nio.charset.Charset </code>charset<code>}
*
* @return  The resultant byte array
*
* @exception  UnsupportedEncodingException
*             If the named charset is not supported
*
* @since      JDK1.1
public byte[] getBytes(String charsetName)
throws UnsupportedEncodingException
{
return StringCoding.encode(charsetName, value, offset, count);
}
)  ( *
* Encodes this <tt>String</tt> into a sequence of bytes using the
* platform's default charset, storing the result into a new byte array.
*
* <p> The behavior of this method when this string cannot be encoded in
* the default charset is unspecified.  The {@link
* java.nio.charset.CharsetEncoder} class should be used when more control
* over the encoding process is required.
*
* @return  The resultant byte array
*
* @since      JDK1.1
)

:LOCAL String.getBytes§-601004288
   2 VALLOCATE LOCAL §base0
   DUP 0 §base0 + V! LOCAL §this
   
   \ new statement
   0
   LOCALS b |
   
   \ new statement
   
   1 VALLOCATE LOCAL §tempvar
   §this CELL+ @ 36 + ( equation.String.count )    @ 65537 JavaArray§1352878592.table -227194368 EXECUTE-NEW
   DUP 4 §base0 + V! TO b
   
   §tempvar SETVTOP
   
   PURGE 1
   
   
   \ new statement
   0
   LOCALS i |
   
   \ new statement
   0
   TO i
   
   \ new statement
   BEGIN
      
      \ new statement
      i
      §this CELL+ @ 36 + ( equation.String.count )       @
      <
      
      \ new statement
   WHILE
      
      \ new statement
      
      0
      LOCALS 0§ |
      
      i TO 0§
      §this CELL+ @ 28 + ( equation.String.value ) @ >R
      i
      R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
      A:R@ H@  127 U>! NIP
      IF
         0FFFFFF00H OR
      ENDIF
      A:DROP
      b >R
      0§
      R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
      C!
      
      PURGE 1
      
      
      \ new statement
      
      \ new statement
      i 1+ TO i
      
      \ new statement
   REPEAT
   
   \ new statement
   0 §break28237 LABEL
   
   \ new statement
   
   PURGE 1
   
   \ new statement
   DROP
   
   \ new statement
   b
   DUP 0 V!
   0 §break28236 BRANCH
   
   \ new statement
   0 §break28236 LABEL
   
   \ new statement
   
   §base0 SETVTOP
   PURGE 3
   
   \ new statement
   DROP
;
( *
* Copies characters from this string into the destination character
* array.
* <p>
* The first character to be copied is at index <code>srcBegin</code>;
* the last character to be copied is at index <code>srcEnd-1</code>
* (thus the total number of characters to be copied is
* <code>srcEnd-srcBegin</code>). The characters are copied into the
* subarray of <code>dst</code> starting at index <code>dstBegin</code>
* and ending at index:
* <p><blockquote><pre>
*     dstbegin + (srcEnd-srcBegin) - 1
* </pre></blockquote>
*
* @param      srcBegin   index of the first character in the string
*                        to copy.
* @param      srcEnd     index after the last character in the string
*                        to copy.
* @param      dst        the destination array.
* @param      dstBegin   the start offset in the destination array.
* @exception IndexOutOfBoundsException If any of the following
*            is true:
*            <ul><li><code>srcBegin</code> is negative.
*            <li><code>srcBegin</code> is greater than <code>srcEnd</code>
*            <li><code>srcEnd</code> is greater than the length of this
*                string
*            <li><code>dstBegin</code> is negative
*            <li><code>dstBegin+(srcEnd-srcBegin)</code> is larger than
*                <code>dst.length</code></ul>
)

:LOCAL String.getChars§-53713152
   2 VALLOCATE LOCAL §base0
   DUP 4 §base0 + V! LOCAL §this
   LOCAL dstBegin
   DUP 0 §base0 + V! LOCAL dst
   LOCAL srcEnd
   LOCAL srcBegin
   
   \ new statement
   
   \ new statement
   ( if (srcBegin < 0) {
   throw new StringIndexOutOfBoundsException(srcBegin);
   }
   if (srcEnd > count) {
   throw new StringIndexOutOfBoundsException(srcEnd);
   }
   if (srcBegin > srcEnd) {
   throw new StringIndexOutOfBoundsException(srcEnd - srcBegin);
   } )
   
   \ new statement
   §this CELL+ @ 28 + ( equation.String.value )    @
   §this CELL+ @ 32 + ( equation.String.offset )    @
   srcBegin
   +
   dst
   dstBegin
   srcEnd
   srcBegin
   -
   String.arraycopy§1134321920
   
   \ new statement
   0 §break28232 LABEL
   
   \ new statement
   
   
   
   
   §base0 SETVTOP
   PURGE 6
   
   \ new statement
   DROP
;
( *
* Returns a hash code for this string. The hash code for a
* <code>String</code> object is computed as
* <blockquote><pre>
* s[0]*31^(n-1) + s[1]*31^(n-2) + ... + s[n-1]
* </pre></blockquote>
* using <code>int</code> arithmetic, where <code>s[i]</code> is the
* <i>i</i>th character of the string, <code>n</code> is the length of
* the string, and <code>^</code> indicates exponentiation.
* (The hash value of the empty string is zero.)
*
* @return  a hash code value for this object.
)

:LOCAL String.hashCode§-1604556800
   1 VALLOCATE LOCAL §base0
   DUP 0 §base0 + V! LOCAL §this
   
   \ new statement
   0
   LOCALS h |
   
   \ new statement
   §this CELL+ @ 40 + ( equation.String.hash )    @
   TO h
   
   \ new statement
   h
   0=
   
   \ new statement
   IF
      
      \ new statement
      1 VALLOCATE LOCAL §base1
      0 DUP DUP
      LOCALS len off val |
      
      \ new statement
      §this CELL+ @ 32 + ( equation.String.offset )       @
      TO off
      
      \ new statement
      §this CELL+ @ 28 + ( equation.String.value )       @
      DUP §base1 V! TO val
      
      \ new statement
      §this CELL+ @ 36 + ( equation.String.count )       @
      TO len
      
      \ new statement
      0
      LOCALS i |
      
      \ new statement
      0
      TO i
      
      \ new statement
      BEGIN
         
         \ new statement
         i
         len
         <
         
         \ new statement
      WHILE
         
         \ new statement
         h
         31
         *
         val >R
         off DUP 1+ TO off
         R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
         H@
         +
         TO h
         
         \ new statement
         
         \ new statement
         i 1+ TO i
         
         \ new statement
      REPEAT
      
      \ new statement
      0 §break28287 LABEL
      
      \ new statement
      
      PURGE 1
      
      \ new statement
      DROP
      
      \ new statement
      h
      §this CELL+ @ 40 + ( equation.String.hash )       !
      
      \ new statement
      0 §break28286 LABEL
      
      \ new statement
      
      
      
      §base1 SETVTOP
      PURGE 4
      
      \ new statement
      DROP
      
      \ new statement
   ENDIF
   
   \ new statement
   h
   0 §break28285 BRANCH
   
   \ new statement
   0 §break28285 LABEL
   
   \ new statement
   
   §base0 SETVTOP
   PURGE 3
   
   \ new statement
   DROP
;
( *
* Returns the index within this string of the first occurrence of the
* specified substring. The integer returned is the smallest value
* <i>k</i> such that:
* <blockquote><pre>
* this.startsWith(str, <i>k</i>)
* </pre></blockquote>
* is <code>true</code>.
*
* @param   str   any string.
* @return  if the string argument occurs as a substring within this
*          object, then the index of the first character of the first
*          such substring is returned; if it does not occur as a
*          substring, <code>-1</code> is returned.
)

:LOCAL String.indexOf§-337549056
   2 VALLOCATE LOCAL §base0
   DUP 4 §base0 + V! LOCAL §this
   DUP 0 §base0 + V! LOCAL str
   
   \ new statement
   
   \ new statement
   §this >R
   str
   0
   R> 517761280 TRUE ( equation.String.indexOf§517761280 ) EXECUTE-METHOD
   0 §break28301 BRANCH
   
   \ new statement
   0 §break28301 LABEL
   
   \ new statement
   
   §base0 SETVTOP
   PURGE 3
   
   \ new statement
   DROP
;
( *
* Code shared by String and StringBuffer to do searches. The
* source is the character array being searched, and the target
* is the string being searched for.
*
* @param   source       the characters being searched.
* @param   sourceOffset offset of the source string.
* @param   sourceCount  count of the source string.
* @param   target       the characters being searched for.
* @param   targetOffset offset of the target string.
* @param   targetCount  count of the target string.
* @param   fromIndex    the index to begin searching from.
)

: String.indexOf§-412522240
   2 VALLOCATE LOCAL §base0
   LOCAL fromIndex
   LOCAL targetCount
   LOCAL targetOffset
   DUP 0 §base0 + V! LOCAL target
   LOCAL sourceCount
   LOCAL sourceOffset
   DUP 4 §base0 + V! LOCAL source
   
   \ new statement
   0 DUP DUP
   LOCALS first i max |
   
   \ new statement
   fromIndex
   sourceCount
   < INVERT
   
   \ new statement
   IF
      
      \ new statement
      
      \ new statement
      targetCount
      0=
      IF
         sourceCount
      ELSE
         -1
         
      ENDIF
      65537 §break28304 BRANCH
      
      \ new statement
      0 §break28304 LABEL
      
      \ new statement
      
      \ new statement
      0>!
      IF
         10001H - §break28303 BRANCH
      ENDIF
      DROP
      
      \ new statement
   ENDIF
   
   \ new statement
   fromIndex
   0<
   
   \ new statement
   IF
      
      \ new statement
      
      \ new statement
      0
      TO fromIndex
      
      \ new statement
      0 §break28305 LABEL
      
      \ new statement
      
      \ new statement
      0>!
      IF
         10001H - §break28303 BRANCH
      ENDIF
      DROP
      
      \ new statement
   ENDIF
   
   \ new statement
   targetCount
   0=
   
   \ new statement
   IF
      
      \ new statement
      
      \ new statement
      fromIndex
      65537 §break28306 BRANCH
      
      \ new statement
      0 §break28306 LABEL
      
      \ new statement
      
      \ new statement
      0>!
      IF
         10001H - §break28303 BRANCH
      ENDIF
      DROP
      
      \ new statement
   ENDIF
   
   \ new statement
   target >R
   targetOffset
   R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
   H@
   TO first
   
   \ new statement
   sourceOffset
   fromIndex
   +
   TO i
   
   \ new statement
   sourceOffset
   sourceCount
   targetCount
   -
   +
   TO max
   
   \ new statement
   0 DUP DUP
   LOCALS end j k |
   
   \ new statement
   BEGIN
      
      \ new statement
      (  Look for first character.  )
      
      \ new statement
      
      \ new statement
      BEGIN
         
         \ new statement
         i
         max
         > INVERT and_31523 0BRANCH! DROP
         source >R
         i
         R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
         H@
         first
         <>
         and_31523 LABEL
         
         \ new statement
      WHILE
         
         \ new statement
         i 1+ TO i
         
         \ new statement
         
         \ new statement
      REPEAT
      
      \ new statement
      0 §break28309 LABEL
      
      \ new statement
      
      \ new statement
      DROP
      
      \ new statement
      i
      max
      >
      
      \ new statement
      IF
         
         \ new statement
         
         \ new statement
         -1
         131074 §break28311 BRANCH
         
         \ new statement
         0 §break28311 LABEL
         
         \ new statement
         
         \ new statement
         0>!
         IF
            10001H - 0<! §break28307 0BRANCH
         ENDIF
         DROP
         
         \ new statement
      ENDIF
      
      \ new statement
      (  Found first character, now look at the rest of v2  )
      
      \ new statement
      i
      1
      +
      TO j
      
      \ new statement
      j
      targetCount
      +
      1
      -
      TO end
      
      \ new statement
      targetOffset
      1
      +
      TO k
      
      \ new statement
      
      \ new statement
      BEGIN
         
         \ new statement
         j
         end
         <
         
         \ new statement
      WHILE
         
         \ new statement
         source >R
         j DUP 1+ TO j
         R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
         H@
         target >R
         k DUP 1+ TO k
         R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
         H@
         <>
         
         \ new statement
         IF
            
            \ new statement
            
            \ new statement
            i 1+ TO i
            
            \ new statement
            (  Look for str's first char again.  )
            
            \ new statement
            65538 §break28314 BRANCH
            
            \ new statement
            0 §break28314 LABEL
            
            \ new statement
            
            \ new statement
            0>!
            IF
               10001H - 0<! §break28312 0BRANCH
            ENDIF
            DROP
            
            \ new statement
         ENDIF
         
         \ new statement
         
         \ new statement
      REPEAT
      
      \ new statement
      0 §break28312 LABEL
      
      \ new statement
      
      \ new statement
      0>!
      IF
         10001H - 0<! §break28307 0BRANCH
      ENDIF
      DROP
      
      \ new statement
      i
      sourceOffset
      -
      65537 §break28307 BRANCH
      
      \ new statement
      (  Found whole string.  )
      
      \ new statement
      
      \ new statement
   AGAIN
   
   \ new statement
   0 §break28307 LABEL
   
   \ new statement
   
   
   
   PURGE 3
   
   \ new statement
   0>!
   IF
      10001H - §break28303 BRANCH
   ENDIF
   DROP
   
   \ new statement
   0 DROP
   
   \ new statement
   0 §break28303 LABEL
   
   \ new statement
   
   
   
   
   
   
   
   
   
   
   §base0 SETVTOP
   PURGE 11
   
   \ new statement
   DROP
;
( *
* Returns the index within this string of the first occurrence of the
* specified substring, starting at the specified index.  The integer
* returned is the smallest value <tt>k</tt> for which:
* <blockquote><pre>
*     k &gt;= Math.min(fromIndex, str.length()) && this.startsWith(str, k)
* </pre></blockquote>
* If no such value of <i>k</i> exists, then -1 is returned.
*
* @param   str         the substring for which to search.
* @param   fromIndex   the index from which to start the search.
* @return  the index within this string of the first occurrence of the
*          specified substring, starting at the specified index.
)

:LOCAL String.indexOf§517761280
   2 VALLOCATE LOCAL §base0
   DUP 4 §base0 + V! LOCAL §this
   LOCAL fromIndex
   DUP 0 §base0 + V! LOCAL str
   
   \ new statement
   
   \ new statement
   §this CELL+ @ 28 + ( equation.String.value )    @
   §this CELL+ @ 32 + ( equation.String.offset )    @
   §this CELL+ @ 36 + ( equation.String.count )    @
   str CELL+ @ 28 + ( equation.String.value )    @
   str CELL+ @ 32 + ( equation.String.offset )    @
   str CELL+ @ 36 + ( equation.String.count )    @
   fromIndex
   String.indexOf§-412522240
   0 §break28302 BRANCH
   
   \ new statement
   0 §break28302 LABEL
   
   \ new statement
   
   
   §base0 SETVTOP
   PURGE 4
   
   \ new statement
   DROP
;
( *
* Returns the index within this string of the first occurrence of the
* specified character, starting the search at the specified index.
* <p>
* If a character with value <code>ch</code> occurs in the character
* sequence represented by this <code>String</code> object at an index
* no smaller than <code>fromIndex</code>, then the index of the first
* such occurrence is returned--that is, the smallest value <i>k</i>
* such that:
* <blockquote><pre>
* (this.charAt(<i>k</i>) == ch) && (<i>k</i> &gt;= fromIndex)
* </pre></blockquote>
* is true. If no such character occurs in this string at or after
* position <code>fromIndex</code>, then <code>-1</code> is returned.
* <p>
* There is no restriction on the value of <code>fromIndex</code>. If it
* is negative, it has the same effect as if it were zero: this entire
* string may be searched. If it is greater than the length of this
* string, it has the same effect as if it were equal to the length of
* this string: <code>-1</code> is returned.
*
* @param   ch          a character.
* @param   fromIndex   the index to start the search from.
* @return  the index of the first occurrence of the character in the
*          character sequence represented by this object that is greater
*          than or equal to <code>fromIndex</code>, or <code>-1</code>
*          if the character does not occur.
)

:LOCAL String.indexOf§731343104
   2 VALLOCATE LOCAL §base0
   DUP 0 §base0 + V! LOCAL §this
   LOCAL fromIndex
   LOCAL ch
   
   \ new statement
   0 DUP
   LOCALS max v |
   
   \ new statement
   §this CELL+ @ 32 + ( equation.String.offset )    @
   §this CELL+ @ 36 + ( equation.String.count )    @
   +
   TO max
   
   \ new statement
   §this CELL+ @ 28 + ( equation.String.value )    @
   DUP 4 §base0 + V! TO v
   
   \ new statement
   fromIndex
   0<
   
   \ new statement
   IF
      
      \ new statement
      
      \ new statement
      0
      TO fromIndex
      
      \ new statement
      0 §break28291 LABEL
      
      \ new statement
      
      \ new statement
      DROP
      
      \ new statement
   ELSE
      
      \ new statement
      fromIndex
      §this CELL+ @ 36 + ( equation.String.count )       @
      < INVERT
      
      \ new statement
      IF
         
         \ new statement
         
         \ new statement
         \  Note: fromIndex might be near -1>>>1.
         
         
         \ new statement
         -1
         65537 §break28292 BRANCH
         
         \ new statement
         0 §break28292 LABEL
         
         \ new statement
         
         \ new statement
         0>!
         IF
            10001H - §break28290 BRANCH
         ENDIF
         DROP
         
         \ new statement
      ENDIF
      
      \ new statement
   ENDIF
   
   \ new statement
   0
   LOCALS i |
   
   \ new statement
   §this CELL+ @ 32 + ( equation.String.offset )    @
   fromIndex
   +
   TO i
   
   \ new statement
   BEGIN
      
      \ new statement
      i
      max
      <
      
      \ new statement
   WHILE
      
      \ new statement
      v >R
      i
      R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
      H@
      ch
      =
      
      \ new statement
      IF
         
         \ new statement
         
         \ new statement
         i
         §this CELL+ @ 32 + ( equation.String.offset )          @
         -
         131074 §break28295 BRANCH
         
         \ new statement
         0 §break28295 LABEL
         
         \ new statement
         
         \ new statement
         0>!
         IF
            10001H - 0<! §break28293 0BRANCH
         ENDIF
         DROP
         
         \ new statement
      ENDIF
      
      \ new statement
      
      \ new statement
      i 1+ TO i
      
      \ new statement
   REPEAT
   
   \ new statement
   0 §break28293 LABEL
   
   \ new statement
   
   PURGE 1
   
   \ new statement
   0>!
   IF
      10001H - §break28290 BRANCH
   ENDIF
   DROP
   
   \ new statement
   -1
   0 §break28290 BRANCH
   
   \ new statement
   0 §break28290 LABEL
   
   \ new statement
   
   
   
   
   §base0 SETVTOP
   PURGE 6
   
   \ new statement
   DROP
;
( *
* Returns the index within this string of the first occurrence of the
* specified character. If a character with value <code>ch</code> occurs
* in the character sequence represented by this <code>String</code>
* object, then the index of the first such occurrence is returned --
* that is, the smallest value <i>k</i> such that:
* <blockquote><pre>
* this.charAt(<i>k</i>) == ch
* </pre></blockquote>
* is <code>true</code>. If no such character occurs in this string,
* then <code>-1</code> is returned.
*
* @param   ch   a character.
* @return  the index of the first occurrence of the character in the
*          character sequence represented by this object, or
*          <code>-1</code> if the character does not occur.
)

:LOCAL String.indexOf§996698368
   1 VALLOCATE LOCAL §base0
   DUP 0 §base0 + V! LOCAL §this
   LOCAL ch
   
   \ new statement
   
   \ new statement
   §this >R
   ch
   0
   R> 731343104 TRUE ( equation.String.indexOf§731343104 ) EXECUTE-METHOD
   0 §break28289 BRANCH
   
   \ new statement
   0 §break28289 LABEL
   
   \ new statement
   
   §base0 SETVTOP
   PURGE 3
   
   \ new statement
   DROP
;
( *
* Returns the index within this string of the rightmost occurrence
* of the specified substring.  The rightmost empty string "" is
* considered to occur at the index value <code>this.length()</code>.
* The returned index is the largest value <i>k</i> such that
* <blockquote><pre>
* this.startsWith(str, k)
* </pre></blockquote>
* is true.
*
* @param   str   the substring to search for.
* @return  if the string argument occurs one or more times as a substring
*          within this object, then the index of the first character of
*          the last such substring is returned. If it does not occur as
*          a substring, <code>-1</code> is returned.
)

:LOCAL String.lastIndexOf§-1299878912
   2 VALLOCATE LOCAL §base0
   DUP 4 §base0 + V! LOCAL §this
   DUP 0 §base0 + V! LOCAL str
   
   \ new statement
   
   \ new statement
   §this >R
   str
   §this CELL+ @ 36 + ( equation.String.count )    @
   R> -465605632 TRUE ( equation.String.lastIndexOf§-465605632 ) EXECUTE-METHOD
   0 §break28315 BRANCH
   
   \ new statement
   0 §break28315 LABEL
   
   \ new statement
   
   §base0 SETVTOP
   PURGE 3
   
   \ new statement
   DROP
;
( *
* Returns the index within this string of the last occurrence of the
* specified character, searching backward starting at the specified
* index. That is, the index returned is the largest value <i>k</i>
* such that:
* <blockquote><pre>
* this.charAt(k) == ch) && (k &lt;= fromIndex)
* </pre></blockquote>
* is true.
*
* @param   ch          a character.
* @param   fromIndex   the index to start the search from. There is no
*          restriction on the value of <code>fromIndex</code>. If it is
*          greater than or equal to the length of this string, it has
*          the same effect as if it were equal to one less than the
*          length of this string: this entire string may be searched.
*          If it is negative, it has the same effect as if it were -1:
*          -1 is returned.
* @return  the index of the last occurrence of the character in the
*          character sequence represented by this object that is less
*          than or equal to <code>fromIndex</code>, or <code>-1</code>
*          if the character does not occur before that point.
)

:LOCAL String.lastIndexOf§-1735627776
   2 VALLOCATE LOCAL §base0
   DUP 0 §base0 + V! LOCAL §this
   LOCAL fromIndex
   LOCAL ch
   
   \ new statement
   0 DUP
   LOCALS min v |
   
   \ new statement
   §this CELL+ @ 32 + ( equation.String.offset )    @
   TO min
   
   \ new statement
   §this CELL+ @ 28 + ( equation.String.value )    @
   DUP 4 §base0 + V! TO v
   
   \ new statement
   0
   LOCALS i |
   
   \ new statement
   §this CELL+ @ 32 + ( equation.String.offset )    @
   fromIndex
   §this CELL+ @ 36 + ( equation.String.count )    @
   < INVERT
   
   IF
      §this CELL+ @ 36 + ( equation.String.count )       @
      1
      -
   ELSE
      fromIndex
      
   ENDIF
   +
   TO i
   
   \ new statement
   BEGIN
      
      \ new statement
      i
      min
      < INVERT
      
      \ new statement
   WHILE
      
      \ new statement
      v >R
      i
      R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
      H@
      ch
      =
      
      \ new statement
      IF
         
         \ new statement
         
         \ new statement
         i
         §this CELL+ @ 32 + ( equation.String.offset )          @
         -
         131074 §break28300 BRANCH
         
         \ new statement
         0 §break28300 LABEL
         
         \ new statement
         
         \ new statement
         0>!
         IF
            10001H - 0<! §break28298 0BRANCH
         ENDIF
         DROP
         
         \ new statement
      ENDIF
      
      \ new statement
      
      \ new statement
      i 1- TO i
      
      \ new statement
   REPEAT
   
   \ new statement
   0 §break28298 LABEL
   
   \ new statement
   
   PURGE 1
   
   \ new statement
   0>!
   IF
      10001H - §break28297 BRANCH
   ENDIF
   DROP
   
   \ new statement
   -1
   0 §break28297 BRANCH
   
   \ new statement
   0 §break28297 LABEL
   
   \ new statement
   
   
   
   
   §base0 SETVTOP
   PURGE 6
   
   \ new statement
   DROP
;
( *
* Returns the index within this string of the last occurrence of the
* specified substring, searching backward starting at the specified index.
* The integer returned is the largest value <i>k</i> such that:
* <blockquote><pre>
*     k &lt;= Math.min(fromIndex, str.length()) && this.startsWith(str, k)
* </pre></blockquote>
* If no such value of <i>k</i> exists, then -1 is returned.
*
* @param   str         the substring to search for.
* @param   fromIndex   the index to start the search from.
* @return  the index within this string of the last occurrence of the
*          specified substring.
)

:LOCAL String.lastIndexOf§-465605632
   2 VALLOCATE LOCAL §base0
   DUP 4 §base0 + V! LOCAL §this
   LOCAL fromIndex
   DUP 0 §base0 + V! LOCAL str
   
   \ new statement
   
   \ new statement
   §this CELL+ @ 28 + ( equation.String.value )    @
   §this CELL+ @ 32 + ( equation.String.offset )    @
   §this CELL+ @ 36 + ( equation.String.count )    @
   str CELL+ @ 28 + ( equation.String.value )    @
   str CELL+ @ 32 + ( equation.String.offset )    @
   str CELL+ @ 36 + ( equation.String.count )    @
   fromIndex
   String.lastIndexOf§831351808
   0 §break28316 BRANCH
   
   \ new statement
   0 §break28316 LABEL
   
   \ new statement
   
   
   §base0 SETVTOP
   PURGE 4
   
   \ new statement
   DROP
;
( *
* Returns the index within this string of the last occurrence of the
* specified character. That is, the index returned is the largest
* value <i>k</i> such that:
* <blockquote><pre>
* this.charAt(<i>k</i>) == ch
* </pre></blockquote>
* is true.
* The String is searched backwards starting at the last character.
*
* @param   ch   a character.
* @return  the index of the last occurrence of the character in the
*          character sequence represented by this object, or
*          <code>-1</code> if the character does not occur.
)

:LOCAL String.lastIndexOf§147155968
   1 VALLOCATE LOCAL §base0
   DUP 0 §base0 + V! LOCAL §this
   LOCAL ch
   
   \ new statement
   
   \ new statement
   §this >R
   ch
   §this CELL+ @ 36 + ( equation.String.count )    @
   1
   -
   R> -1735627776 TRUE ( equation.String.lastIndexOf§-1735627776 ) EXECUTE-METHOD
   0 §break28296 BRANCH
   
   \ new statement
   0 §break28296 LABEL
   
   \ new statement
   
   §base0 SETVTOP
   PURGE 3
   
   \ new statement
   DROP
;
( *
* Code shared by String and StringBuffer to do searches. The
* source is the character array being searched, and the target
* is the string being searched for.
*
* @param   source       the characters being searched.
* @param   sourceOffset offset of the source string.
* @param   sourceCount  count of the source string.
* @param   target       the characters being searched for.
* @param   targetOffset offset of the target string.
* @param   targetCount  count of the target string.
* @param   fromIndex    the index to begin searching from.
)

: String.lastIndexOf§831351808
   2 VALLOCATE LOCAL §base0
   LOCAL fromIndex
   LOCAL targetCount
   LOCAL targetOffset
   DUP 0 §base0 + V! LOCAL target
   LOCAL sourceCount
   LOCAL sourceOffset
   DUP 4 §base0 + V! LOCAL source
   
   \ new statement
   0 DUP DUP DUP DUP
   LOCALS i min rightIndex strLastChar strLastIndex |
   
   \ new statement
   (
   * Check arguments; return immediately where possible. For
   * consistency, don't check for null str.
   )
   
   \ new statement
   sourceCount
   targetCount
   -
   TO rightIndex
   
   \ new statement
   fromIndex
   0<
   
   \ new statement
   IF
      
      \ new statement
      
      \ new statement
      -1
      65537 §break28318 BRANCH
      
      \ new statement
      0 §break28318 LABEL
      
      \ new statement
      
      \ new statement
      0>!
      IF
         10001H - §break28317 BRANCH
      ENDIF
      DROP
      
      \ new statement
   ENDIF
   
   \ new statement
   fromIndex
   rightIndex
   >
   
   \ new statement
   IF
      
      \ new statement
      
      \ new statement
      rightIndex
      TO fromIndex
      
      \ new statement
      0 §break28319 LABEL
      
      \ new statement
      
      \ new statement
      0>!
      IF
         10001H - §break28317 BRANCH
      ENDIF
      DROP
      
      \ new statement
   ENDIF
   
   \ new statement
   (  Empty string always matches.  )
   
   \ new statement
   targetCount
   0=
   
   \ new statement
   IF
      
      \ new statement
      
      \ new statement
      fromIndex
      65537 §break28320 BRANCH
      
      \ new statement
      0 §break28320 LABEL
      
      \ new statement
      
      \ new statement
      0>!
      IF
         10001H - §break28317 BRANCH
      ENDIF
      DROP
      
      \ new statement
   ENDIF
   
   \ new statement
   targetOffset
   targetCount
   +
   1
   -
   TO strLastIndex
   
   \ new statement
   target >R
   strLastIndex
   R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
   H@
   TO strLastChar
   
   \ new statement
   sourceOffset
   targetCount
   +
   1
   -
   TO min
   
   \ new statement
   min
   fromIndex
   +
   TO i
   
   \ new statement
   0 DUP DUP
   LOCALS j k start |
   
   \ new statement
   BEGIN
      
      \ new statement
      
      \ new statement
      BEGIN
         
         \ new statement
         i
         min
         < INVERT and_31705 0BRANCH! DROP
         source >R
         i
         R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
         H@
         strLastChar
         <>
         and_31705 LABEL
         
         \ new statement
      WHILE
         
         \ new statement
         i 1- TO i
         
         \ new statement
         
         \ new statement
      REPEAT
      
      \ new statement
      0 §break28323 LABEL
      
      \ new statement
      
      \ new statement
      DROP
      
      \ new statement
      i
      min
      <
      
      \ new statement
      IF
         
         \ new statement
         
         \ new statement
         -1
         131074 §break28325 BRANCH
         
         \ new statement
         0 §break28325 LABEL
         
         \ new statement
         
         \ new statement
         0>!
         IF
            10001H - 0<! §break28321 0BRANCH
         ENDIF
         DROP
         
         \ new statement
      ENDIF
      
      \ new statement
      i
      1
      -
      TO j
      
      \ new statement
      j
      targetCount
      1
      -
      -
      TO start
      
      \ new statement
      strLastIndex
      1
      -
      TO k
      
      \ new statement
      
      \ new statement
      BEGIN
         
         \ new statement
         j
         start
         >
         
         \ new statement
      WHILE
         
         \ new statement
         source >R
         j DUP 1- TO j
         R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
         H@
         target >R
         k DUP 1- TO k
         R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
         H@
         <>
         
         \ new statement
         IF
            
            \ new statement
            
            \ new statement
            i 1- TO i
            
            \ new statement
            65538 §break28328 BRANCH
            
            \ new statement
            0 §break28328 LABEL
            
            \ new statement
            
            \ new statement
            0>!
            IF
               10001H - 0<! §break28326 0BRANCH
            ENDIF
            DROP
            
            \ new statement
         ENDIF
         
         \ new statement
         
         \ new statement
      REPEAT
      
      \ new statement
      0 §break28326 LABEL
      
      \ new statement
      
      \ new statement
      0>!
      IF
         10001H - 0<! §break28321 0BRANCH
      ENDIF
      DROP
      
      \ new statement
      start
      sourceOffset
      -
      1
      +
      65537 §break28321 BRANCH
      
      \ new statement
      
      \ new statement
   AGAIN
   
   \ new statement
   0 §break28321 LABEL
   
   \ new statement
   
   
   
   PURGE 3
   
   \ new statement
   0>!
   IF
      10001H - §break28317 BRANCH
   ENDIF
   DROP
   
   \ new statement
   0 DROP
   
   \ new statement
   0 §break28317 LABEL
   
   \ new statement
   
   
   
   
   
   
   
   
   
   
   
   
   §base0 SETVTOP
   PURGE 13
   
   \ new statement
   DROP
;
( *
* Returns the length of this string.
* The length is equal to the number of 16-bit
* Unicode characters in the string.
*
* @return  the length of the sequence of characters represented by this
*          object.
)

:LOCAL String.length§188050432
   1 VALLOCATE LOCAL §base0
   DUP 0 §base0 + V! LOCAL §this
   
   \ new statement
   
   \ new statement
   §this CELL+ @ 36 + ( equation.String.count )    @
   0 §break28230 BRANCH
   
   \ new statement
   0 §break28230 LABEL
   
   \ new statement
   §base0 SETVTOP
   PURGE 2
   
   \ new statement
   DROP
;
( *
* Tests if two string regions are equal.
* <p>
* A substring of this <tt>String</tt> object is compared to a substring
* of the argument other. The result is true if these substrings
* represent identical character sequences. The substring of this
* <tt>String</tt> object to be compared begins at index <tt>toffset</tt>
* and has length <tt>len</tt>. The substring of other to be compared
* begins at index <tt>ooffset</tt> and has length <tt>len</tt>. The
* result is <tt>false</tt> if and only if at least one of the following
* is true:
* <ul><li><tt>toffset</tt> is negative.
* <li><tt>ooffset</tt> is negative.
* <li><tt>toffset+len</tt> is greater than the length of this
* <tt>String</tt> object.
* <li><tt>ooffset+len</tt> is greater than the length of the other
* argument.
* <li>There is some nonnegative integer <i>k</i> less than <tt>len</tt>
* such that:
* <tt>this.charAt(toffset+<i>k</i>)&nbsp;!=&nbsp;other.charAt(ooffset+<i>k</i>)</tt>
* </ul>
*
* @param   toffset   the starting offset of the subregion in this string.
* @param   other     the string argument.
* @param   ooffset   the starting offset of the subregion in the string
*                    argument.
* @param   len       the number of characters to compare.
* @return  <code>true</code> if the specified subregion of this string
*          exactly matches the specified subregion of the string argument;
*          <code>false</code> otherwise.
)

:LOCAL String.regionMatches§1793290752
   4 VALLOCATE LOCAL §base0
   DUP 4 §base0 + V! LOCAL §this
   LOCAL len
   LOCAL ooffset
   DUP 0 §base0 + V! LOCAL other
   LOCAL toffset
   
   \ new statement
   0 DUP DUP DUP
   LOCALS pa po ta to |
   
   \ new statement
   §this CELL+ @ 28 + ( equation.String.value )    @
   DUP 8 §base0 + V! TO ta
   
   \ new statement
   §this CELL+ @ 32 + ( equation.String.offset )    @
   toffset
   +
   TO to
   
   \ new statement
   other CELL+ @ 28 + ( equation.String.value )    @
   DUP 12 §base0 + V! TO pa
   
   \ new statement
   other CELL+ @ 32 + ( equation.String.offset )    @
   ooffset
   +
   TO po
   
   \ new statement
   \  Note: toffset, ooffset, or len might be near -1>>>1.
   
   
   \ new statement
   ooffset
   0<
   0=! or_31764 0BRANCH DROP
   toffset
   0<
   0=! or_31764 0BRANCH DROP
   toffset  S>D
   §this CELL+ @ 36 + ( equation.String.count )    A:R@ @  S>D
   A:DROP
   len  S>D
   D-
   D>
   0=! or_31764 0BRANCH DROP
   ooffset  S>D
   other CELL+ @ 36 + ( equation.String.count )    A:R@ @  S>D
   A:DROP
   len  S>D
   D-
   D>
   or_31764 LABEL
   
   \ new statement
   IF
      
      \ new statement
      
      \ new statement
      FALSE
      65537 §break28266 BRANCH
      
      \ new statement
      0 §break28266 LABEL
      
      \ new statement
      
      \ new statement
      0>!
      IF
         10001H - §break28265 BRANCH
      ENDIF
      DROP
      
      \ new statement
   ENDIF
   
   \ new statement
   
   \ new statement
   BEGIN
      
      \ new statement
      len DUP 1- TO len
      0>
      
      \ new statement
   WHILE
      
      \ new statement
      ta >R
      to DUP 1+ TO to
      R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
      H@
      pa >R
      po DUP 1+ TO po
      R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
      H@
      <>
      
      \ new statement
      IF
         
         \ new statement
         
         \ new statement
         FALSE
         131074 §break28269 BRANCH
         
         \ new statement
         0 §break28269 LABEL
         
         \ new statement
         
         \ new statement
         0>!
         IF
            10001H - 0<! §break28267 0BRANCH
         ENDIF
         DROP
         
         \ new statement
      ENDIF
      
      \ new statement
      
      \ new statement
   REPEAT
   
   \ new statement
   0 §break28267 LABEL
   
   \ new statement
   
   \ new statement
   0>!
   IF
      10001H - §break28265 BRANCH
   ENDIF
   DROP
   
   \ new statement
   TRUE
   0 §break28265 BRANCH
   
   \ new statement
   0 §break28265 LABEL
   
   \ new statement
   
   
   
   
   
   
   
   
   §base0 SETVTOP
   PURGE 10
   
   \ new statement
   DROP
;
( *
* Tests if two string regions are equal.
* <p>
* A substring of this <tt>String</tt> object is compared to a substring
* of the argument <tt>other</tt>. The result is <tt>true</tt> if these
* substrings represent character sequences that are the same, ignoring
* case if and only if <tt>ignoreCase</tt> is true. The substring of
* this <tt>String</tt> object to be compared begins at index
* <tt>toffset</tt> and has length <tt>len</tt>. The substring of
* <tt>other</tt> to be compared begins at index <tt>ooffset</tt> and
* has length <tt>len</tt>. The result is <tt>false</tt> if and only if
* at least one of the following is true:
* <ul><li><tt>toffset</tt> is negative.
* <li><tt>ooffset</tt> is negative.
* <li><tt>toffset+len</tt> is greater than the length of this
* <tt>String</tt> object.
* <li><tt>ooffset+len</tt> is greater than the length of the other
* argument.
* <li><tt>ignoreCase</tt> is <tt>false</tt> and there is some nonnegative
* integer <i>k</i> less than <tt>len</tt> such that:
* <blockquote><pre>
* this.charAt(toffset+k) != other.charAt(ooffset+k)
* </pre></blockquote>
* <li><tt>ignoreCase</tt> is <tt>true</tt> and there is some nonnegative
* integer <i>k</i> less than <tt>len</tt> such that:
* <blockquote><pre>
* Character.toLowerCase(this.charAt(toffset+k)) !=
Character.toLowerCase(other.charAt(ooffset+k))
* </pre></blockquote>
* and:
* <blockquote><pre>
* Character.toUpperCase(this.charAt(toffset+k)) !=
*         Character.toUpperCase(other.charAt(ooffset+k))
* </pre></blockquote>
* </ul>
*
* @param   ignoreCase   if <code>true</code>, ignore case when comparing
*                       characters.
* @param   toffset      the starting offset of the subregion in this
*                       string.
* @param   other        the string argument.
* @param   ooffset      the starting offset of the subregion in the string
*                       argument.
* @param   len          the number of characters to compare.
* @return  <code>true</code> if the specified subregion of this string
*          matches the specified subregion of the string argument;
*          <code>false</code> otherwise. Whether the matching is exact
*          or case insensitive depends on the <code>ignoreCase</code>
*          argument.
)

:LOCAL String.regionMatches§2060349952
   4 VALLOCATE LOCAL §base0
   DUP 4 §base0 + V! LOCAL §this
   LOCAL len
   LOCAL ooffset
   DUP 0 §base0 + V! LOCAL other
   LOCAL toffset
   LOCAL ignoreCase
   
   \ new statement
   0 DUP DUP DUP
   LOCALS pa po ta to |
   
   \ new statement
   §this CELL+ @ 28 + ( equation.String.value )    @
   DUP 8 §base0 + V! TO ta
   
   \ new statement
   §this CELL+ @ 32 + ( equation.String.offset )    @
   toffset
   +
   TO to
   
   \ new statement
   other CELL+ @ 28 + ( equation.String.value )    @
   DUP 12 §base0 + V! TO pa
   
   \ new statement
   other CELL+ @ 32 + ( equation.String.offset )    @
   ooffset
   +
   TO po
   
   \ new statement
   \  Note: toffset, ooffset, or len might be near -1>>>1.
   
   
   \ new statement
   ooffset
   0<
   0=! or_31800 0BRANCH DROP
   toffset
   0<
   0=! or_31800 0BRANCH DROP
   toffset  S>D
   §this CELL+ @ 36 + ( equation.String.count )    A:R@ @  S>D
   A:DROP
   len  S>D
   D-
   D>
   0=! or_31800 0BRANCH DROP
   ooffset  S>D
   other CELL+ @ 36 + ( equation.String.count )    A:R@ @  S>D
   A:DROP
   len  S>D
   D-
   D>
   or_31800 LABEL
   
   \ new statement
   IF
      
      \ new statement
      
      \ new statement
      FALSE
      65537 §break28271 BRANCH
      
      \ new statement
      0 §break28271 LABEL
      
      \ new statement
      
      \ new statement
      0>!
      IF
         10001H - §break28270 BRANCH
      ENDIF
      DROP
      
      \ new statement
   ENDIF
   
   \ new statement
   0 DUP
   LOCALS c1 c2 |
   
   \ new statement
   BEGIN
      
      \ new statement
      len DUP 1- TO len
      0>
      
      \ new statement
   WHILE
      
      \ new statement
      ta >R
      to DUP 1+ TO to
      R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
      H@
      TO c1
      
      \ new statement
      pa >R
      po DUP 1+ TO po
      R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
      H@
      TO c2
      
      \ new statement
      c1
      c2
      =
      
      \ new statement
      IF
         
         \ new statement
         
         \ new statement
         1 §break28274 BRANCH
         
         \ new statement
         0 §break28274 LABEL
         
         \ new statement
         
         \ new statement
         0>!
         IF
            10001H - 0<! §break28272 0BRANCH §continue28272 BRANCH
         ENDIF
         DROP
         
         \ new statement
      ENDIF
      
      \ new statement
      ignoreCase
      
      \ new statement
      IF
         
         \ new statement
         0 DUP
         LOCALS u1 u2 |
         
         \ new statement
         \  If characters don't match but case may be ignored,
         \  try converting both characters to uppercase.
         \  If the results match, then the comparison scan should
         \  continue.
         
         
         \ new statement
         c1
         String.toUpperCase§1730376704
         TO u1
         
         \ new statement
         c2
         String.toUpperCase§1730376704
         TO u2
         
         \ new statement
         u1
         u2
         =
         
         \ new statement
         IF
            
            \ new statement
            
            \ new statement
            65538 §break28276 BRANCH
            
            \ new statement
            0 §break28276 LABEL
            
            \ new statement
            
            \ new statement
            0>!
            IF
               10001H - §break28275 BRANCH
            ENDIF
            DROP
            
            \ new statement
         ENDIF
         
         \ new statement
         \  Unfortunately, conversion to uppercase does not work properly
         \  for the Georgian alphabet, which has strange rules about case
         \  conversion.  So we need to make one last check before
         \  exiting.
         
         
         \ new statement
         u1
         String.toLowerCase§-1264749568
         u2
         String.toLowerCase§-1264749568
         =
         
         \ new statement
         IF
            
            \ new statement
            
            \ new statement
            65538 §break28277 BRANCH
            
            \ new statement
            0 §break28277 LABEL
            
            \ new statement
            
            \ new statement
            0>!
            IF
               10001H - §break28275 BRANCH
            ENDIF
            DROP
            
            \ new statement
         ENDIF
         
         \ new statement
         0 §break28275 LABEL
         
         \ new statement
         
         
         PURGE 2
         
         \ new statement
         0>!
         IF
            10001H - 0<! §break28272 0BRANCH §continue28272 BRANCH
         ENDIF
         DROP
         
         \ new statement
      ENDIF
      
      \ new statement
      FALSE
      65537 §break28272 BRANCH
      
      \ new statement
      DUP §continue28272 LABEL DROP
      
      \ new statement
   REPEAT
   
   \ new statement
   0 §break28272 LABEL
   
   \ new statement
   
   
   PURGE 2
   
   \ new statement
   0>!
   IF
      10001H - §break28270 BRANCH
   ENDIF
   DROP
   
   \ new statement
   TRUE
   0 §break28270 BRANCH
   
   \ new statement
   0 §break28270 LABEL
   
   \ new statement
   
   
   
   
   
   
   
   
   
   §base0 SETVTOP
   PURGE 11
   
   \ new statement
   DROP
;
( *
* Returns a new string resulting from replacing all occurrences of
* <code>oldChar</code> in this string with <code>newChar</code>.
* <p>
* If the character <code>oldChar</code> does not occur in the
* character sequence represented by this <code>String</code> object,
* then a reference to this <code>String</code> object is returned.
* Otherwise, a new <code>String</code> object is created that
* represents a character sequence identical to the character sequence
* represented by this <code>String</code> object, except that every
* occurrence of <code>oldChar</code> is replaced by an occurrence
* of <code>newChar</code>.
* <p>
* Examples:
* <blockquote><pre>
* "mesquite in your cellar".replace('e', 'o')
*         returns "mosquito in your collar"
* "the war of baronets".replace('r', 'y')
*         returns "the way of bayonets"
* "sparring with a purple porpoise".replace('p', 't')
*         returns "starring with a turtle tortoise"
* "JonL".replace('q', 'x') returns "JonL" (no change)
* </pre></blockquote>
*
* @param   oldChar   the old character.
* @param   newChar   the new character.
* @return  a string derived from this string by replacing every
*          occurrence of <code>oldChar</code> with <code>newChar</code>.
)

:LOCAL String.replace§-1850510848
   1 VALLOCATE LOCAL §base0
   DUP 0 §base0 + V! LOCAL §this
   LOCAL newChar
   LOCAL oldChar
   
   \ new statement
   
   \ new statement
   oldChar
   newChar
   <>
   
   \ new statement
   IF
      
      \ new statement
      1 VALLOCATE LOCAL §base1
      0 DUP DUP DUP
      LOCALS i len off val |
      
      \ new statement
      §this CELL+ @ 36 + ( equation.String.count )       @
      TO len
      
      \ new statement
      -1
      TO i
      
      \ new statement
      §this CELL+ @ 28 + ( equation.String.value )       @
      DUP §base1 V! TO val
      
      \ new statement
      (  avoid getfield opcode  )
      
      \ new statement
      §this CELL+ @ 32 + ( equation.String.offset )       @
      TO off
      
      \ new statement
      (  avoid getfield opcode  )
      
      \ new statement
      
      \ new statement
      BEGIN
         
         \ new statement
         i 1+ DUP TO i
         len
         <
         
         \ new statement
      WHILE
         
         \ new statement
         val >R
         off
         i
         +
         R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
         H@
         oldChar
         =
         
         \ new statement
         IF
            
            \ new statement
            
            \ new statement
            65537 §break28338 BRANCH
            
            \ new statement
            0 §break28338 LABEL
            
            \ new statement
            
            \ new statement
            0>!
            IF
               10001H - 0<! §break28336 0BRANCH
            ENDIF
            DROP
            
            \ new statement
         ENDIF
         
         \ new statement
         
         \ new statement
      REPEAT
      
      \ new statement
      0 §break28336 LABEL
      
      \ new statement
      
      \ new statement
      DROP
      
      \ new statement
      i
      len
      <
      
      \ new statement
      IF
         
         \ new statement
         1 VALLOCATE LOCAL §base2
         0
         LOCALS buf |
         
         \ new statement
         
         1 VALLOCATE LOCAL §tempvar
         len 131073 JavaArray§1352878592.table -227194368 EXECUTE-NEW
         DUP §base2 V! TO buf
         
         §tempvar SETVTOP
         
         PURGE 1
         
         
         \ new statement
         0
         LOCALS j |
         
         \ new statement
         0
         TO j
         
         \ new statement
         BEGIN
            
            \ new statement
            j
            i
            <
            
            \ new statement
         WHILE
            
            \ new statement
            
            0
            LOCALS 0§ |
            
            j TO 0§
            val >R
            off
            j
            +
            R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
            H@
            buf >R
            0§
            R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
            H!
            
            PURGE 1
            
            
            \ new statement
            
            \ new statement
            j 1+ TO j
            
            \ new statement
         REPEAT
         
         \ new statement
         0 §break28340 LABEL
         
         \ new statement
         
         PURGE 1
         
         \ new statement
         DROP
         
         \ new statement
         0
         LOCALS c |
         
         \ new statement
         BEGIN
            
            \ new statement
            i
            len
            <
            
            \ new statement
         WHILE
            
            \ new statement
            val >R
            off
            i
            +
            R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
            H@
            TO c
            
            \ new statement
            
            0
            LOCALS 0§ |
            
            i TO 0§
            c
            oldChar
            =
            
            IF
               newChar
            ELSE
               c
               
            ENDIF
            buf >R
            0§
            R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
            H!
            
            PURGE 1
            
            
            \ new statement
            i 1+ TO i
            
            \ new statement
            
            \ new statement
         REPEAT
         
         \ new statement
         0 §break28342 LABEL
         
         \ new statement
         
         PURGE 1
         
         \ new statement
         DROP
         
         \ new statement
         buf
         0
         len
         String§1651856128.table 1127437056 EXECUTE-NEW
         DUP 0 V!
         131074 §break28339 BRANCH
         
         \ new statement
         0 §break28339 LABEL
         
         \ new statement
         
         §base2 SETVTOP
         PURGE 2
         
         \ new statement
         0>!
         IF
            10001H - §break28335 BRANCH
         ENDIF
         DROP
         
         \ new statement
      ENDIF
      
      \ new statement
      0 §break28335 LABEL
      
      \ new statement
      
      
      
      
      §base1 SETVTOP
      PURGE 5
      
      \ new statement
      0>!
      IF
         10001H - §break28334 BRANCH
      ENDIF
      DROP
      
      \ new statement
   ENDIF
   
   \ new statement
   §this
   DUP 0 V!
   0 §break28334 BRANCH
   
   \ new statement
   0 §break28334 LABEL
   
   \ new statement
   
   
   §base0 SETVTOP
   PURGE 4
   
   \ new statement
   DROP
;
( *
* Tests if this string starts with the specified prefix beginning
* a specified index.
*
* @param   prefix    the prefix.
* @param   toffset   where to begin looking in the string.
* @return  <code>true</code> if the character sequence represented by the
*          argument is a prefix of the substring of this object starting
*          at index <code>toffset</code>; <code>false</code> otherwise.
*          The result is <code>false</code> if <code>toffset</code> is
*          negative or greater than the length of this
*          <code>String</code> object; otherwise the result is the same
*          as the result of the expression
*          <pre>
*          this.subString(toffset).startsWith(prefix)
*          </pre>
)

:LOCAL String.startsWith§-953322752
   4 VALLOCATE LOCAL §base0
   DUP 4 §base0 + V! LOCAL §this
   LOCAL toffset
   DUP 0 §base0 + V! LOCAL prefix
   
   \ new statement
   0 DUP DUP DUP DUP
   LOCALS pa pc po ta to |
   
   \ new statement
   §this CELL+ @ 28 + ( equation.String.value )    @
   DUP 8 §base0 + V! TO ta
   
   \ new statement
   §this CELL+ @ 32 + ( equation.String.offset )    @
   toffset
   +
   TO to
   
   \ new statement
   prefix CELL+ @ 28 + ( equation.String.value )    @
   DUP 12 §base0 + V! TO pa
   
   \ new statement
   prefix CELL+ @ 32 + ( equation.String.offset )    @
   TO po
   
   \ new statement
   prefix CELL+ @ 36 + ( equation.String.count )    @
   TO pc
   
   \ new statement
   \  Note: toffset might be near -1>>>1.
   
   
   \ new statement
   toffset
   0<
   0=! or_31935 0BRANCH DROP
   toffset
   §this CELL+ @ 36 + ( equation.String.count )    @
   pc
   -
   >
   or_31935 LABEL
   
   \ new statement
   IF
      
      \ new statement
      
      \ new statement
      FALSE
      65537 §break28279 BRANCH
      
      \ new statement
      0 §break28279 LABEL
      
      \ new statement
      
      \ new statement
      0>!
      IF
         10001H - §break28278 BRANCH
      ENDIF
      DROP
      
      \ new statement
   ENDIF
   
   \ new statement
   
   \ new statement
   BEGIN
      
      \ new statement
      pc 1- DUP TO pc
      0< INVERT
      
      \ new statement
   WHILE
      
      \ new statement
      ta >R
      to DUP 1+ TO to
      R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
      H@
      pa >R
      po DUP 1+ TO po
      R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
      H@
      <>
      
      \ new statement
      IF
         
         \ new statement
         
         \ new statement
         FALSE
         131074 §break28282 BRANCH
         
         \ new statement
         0 §break28282 LABEL
         
         \ new statement
         
         \ new statement
         0>!
         IF
            10001H - 0<! §break28280 0BRANCH
         ENDIF
         DROP
         
         \ new statement
      ENDIF
      
      \ new statement
      
      \ new statement
   REPEAT
   
   \ new statement
   0 §break28280 LABEL
   
   \ new statement
   
   \ new statement
   0>!
   IF
      10001H - §break28278 BRANCH
   ENDIF
   DROP
   
   \ new statement
   TRUE
   0 §break28278 BRANCH
   
   \ new statement
   0 §break28278 LABEL
   
   \ new statement
   
   
   
   
   
   
   
   §base0 SETVTOP
   PURGE 9
   
   \ new statement
   DROP
;
( *
* Tests if this string starts with the specified prefix.
*
* @param   prefix   the prefix.
* @return  <code>true</code> if the character sequence represented by the
*          argument is a prefix of the character sequence represented by
*          this string; <code>false</code> otherwise.
*          Note also that <code>true</code> will be returned if the
*          argument is an empty string or is equal to this
*          <code>String</code> object as determined by the
*          {@link #equals(Object)} method.
* @since   1. 0
)

:LOCAL String.startsWith§970093312
   2 VALLOCATE LOCAL §base0
   DUP 4 §base0 + V! LOCAL §this
   DUP 0 §base0 + V! LOCAL prefix
   
   \ new statement
   
   \ new statement
   §this >R
   prefix
   0
   R> -953322752 TRUE ( equation.String.startsWith§-953322752 ) EXECUTE-METHOD
   0 §break28283 BRANCH
   
   \ new statement
   0 §break28283 LABEL
   
   \ new statement
   
   §base0 SETVTOP
   PURGE 3
   
   \ new statement
   DROP
;
( *
* Returns a new character sequence that is a subsequence of this sequence.
*
* <p> An invocation of this method of the form
*
* <blockquote><pre>
* str.subSequence(begin,&nbsp;end)</pre></blockquote>
*
* behaves in exactly the same way as the invocation
*
* <blockquote><pre>
* str.substring(begin,&nbsp;end)</pre></blockquote>
*
* This method is defined so that the <tt>String</tt> class can implement
* the {@link CharSequence} interface. </p>
*
* @param      beginIndex   the begin index, inclusive.
* @param      endIndex     the end index, exclusive.
* @return     the specified subsequence.
*
* @throws  IndexOutOfBoundsException
*          if <tt>beginIndex</tt> or <tt>endIndex</tt> are negative,
*          if <tt>endIndex</tt> is greater than <tt>length()</tt>,
*          or if <tt>beginIndex</tt> is greater than <tt>startIndex</tt>
*
* @since 1.4
* @spec JSR-51
)

:LOCAL String.subSequence§602239744
   1 VALLOCATE LOCAL §base0
   DUP 0 §base0 + V! LOCAL §this
   LOCAL endIndex
   LOCAL beginIndex
   
   \ new statement
   
   \ new statement
   §this >R
   beginIndex
   endIndex
   R> -424054016 TRUE ( equation.String.substring§-424054016 ) EXECUTE-METHOD
   DUP 0 V!
   0 §break28331 BRANCH
   
   \ new statement
   0 §break28331 LABEL
   
   \ new statement
   
   
   §base0 SETVTOP
   PURGE 4
   
   \ new statement
   DROP
;
( *
* Returns a new string that is a substring of this string. The
* substring begins at the specified <code>beginIndex</code> and
* extends to the character at index <code>endIndex - 1</code>.
* Thus the length of the substring is <code>endIndex-beginIndex</code>.
* <p>
* Examples:
* <blockquote><pre>
* "hamburger".substring(4, 8) returns "urge"
* "smiles".substring(1, 5) returns "mile"
* </pre></blockquote>
*
* @param      beginIndex   the beginning index, inclusive.
* @param      endIndex     the ending index, exclusive.
* @return     the specified substring.
* @exception  IndexOutOfBoundsException  if the
*             <code>beginIndex</code> is negative, or
*             <code>endIndex</code> is larger than the length of
*             this <code>String</code> object, or
*             <code>beginIndex</code> is larger than
*             <code>endIndex</code>.
)

:LOCAL String.substring§-424054016
   1 VALLOCATE LOCAL §base0
   DUP 0 §base0 + V! LOCAL §this
   LOCAL endIndex
   LOCAL beginIndex
   
   \ new statement
   
   \ new statement
   ( if (beginIndex < 0) {
   throw new StringIndexOutOfBoundsException(beginIndex);
   }
   if (endIndex > count) {
   throw new StringIndexOutOfBoundsException(endIndex);
   }
   if (beginIndex > endIndex) {
   throw new StringIndexOutOfBoundsException(endIndex - beginIndex);
   } )
   
   \ new statement
   beginIndex
   0=
   and_31977 0BRANCH! DROP
   endIndex
   §this CELL+ @ 36 + ( equation.String.count )    @
   =
   and_31977 LABEL
   
   IF
      §this
   ELSE
      §this CELL+ @ 28 + ( equation.String.value )       @
      §this CELL+ @ 32 + ( equation.String.offset )       @
      beginIndex
      +
      endIndex
      beginIndex
      -
      String§1651856128.table 1127437056 EXECUTE-NEW
      
   ENDIF
   DUP 0 V!
   0 §break28330 BRANCH
   
   \ new statement
   0 §break28330 LABEL
   
   \ new statement
   
   
   §base0 SETVTOP
   PURGE 4
   
   \ new statement
   DROP
;
( *
* Returns a new string that is a substring of this string. The
* substring begins with the character at the specified index and
* extends to the end of this string. <p>
* Examples:
* <blockquote><pre>
* "unhappy".substring(2) returns "happy"
* "Harbison".substring(3) returns "bison"
* "emptiness".substring(9) returns "" (an empty string)
* </pre></blockquote>
*
* @param      beginIndex   the beginning index, inclusive.
* @return     the specified substring.
* @exception  IndexOutOfBoundsException  if
*             <code>beginIndex</code> is negative or larger than the
*             length of this <code>String</code> object.
)

:LOCAL String.substring§18576128
   1 VALLOCATE LOCAL §base0
   DUP 0 §base0 + V! LOCAL §this
   LOCAL beginIndex
   
   \ new statement
   
   \ new statement
   §this >R
   beginIndex
   §this CELL+ @ 36 + ( equation.String.count )    @
   R> -424054016 TRUE ( equation.String.substring§-424054016 ) EXECUTE-METHOD
   DUP 0 V!
   0 §break28329 BRANCH
   
   \ new statement
   0 §break28329 LABEL
   
   \ new statement
   
   §base0 SETVTOP
   PURGE 3
   
   \ new statement
   DROP
;
( *
* Converts this string to a new character array.
*
* @return  a newly allocated character array whose length is the length
*          of this string and whose contents are initialized to contain
*          the character sequence represented by this string.
)

:LOCAL String.toCharArray§629044224
   2 VALLOCATE LOCAL §base0
   DUP 0 §base0 + V! LOCAL §this
   
   \ new statement
   0
   LOCALS result |
   
   \ new statement
   
   1 VALLOCATE LOCAL §tempvar
   §this CELL+ @ 36 + ( equation.String.count )    @ 131073 JavaArray§1352878592.table -227194368 EXECUTE-NEW
   DUP 4 §base0 + V! TO result
   
   §tempvar SETVTOP
   
   PURGE 1
   
   
   \ new statement
   §this >R
   0
   §this CELL+ @ 36 + ( equation.String.count )    @
   result
   0
   R> -53713152 TRUE ( equation.String.getChars§-53713152 ) EXECUTE-METHOD
   
   \ new statement
   result
   DUP 0 V!
   0 §break28366 BRANCH
   
   \ new statement
   0 §break28366 LABEL
   
   \ new statement
   
   §base0 SETVTOP
   PURGE 3
   
   \ new statement
   DROP
;


: String.toLowerCase§-1264749568
   LOCAL a
   
   \ new statement
   
   \ new statement
   a
   65
   < INVERT and_31995 0BRANCH! DROP
   a
   90
   > INVERT
   and_31995 LABEL
   
   \ new statement
   IF
      
      \ new statement
      a
      65
      -
      97
      +  0FFFFH AND
      TO a
      
      \ new statement
   ENDIF
   
   \ new statement
   a
   0 §break28263 BRANCH
   
   \ new statement
   0 §break28263 LABEL
   
   \ new statement
   
   PURGE 1
   
   \ new statement
   DROP
;
( *
* Tells whether or not this string matches the given <a
* href="../util/regex/Pattern.html#sum">regular expression</a>.
*
* <p> An invocation of this method of the form
* <i>str</i><tt>.matches(</tt><i>regex</i><tt>)</tt> yields exactly the
* same result as the expression
*
* <blockquote><tt> {@link java.util.regex.Pattern}.{@link
* java.util.regex.Pattern#matches(String,CharSequence)
* matches}(</tt><i>regex</i><tt>,</tt> <i>str</i><tt>)</tt></blockquote>
*
* @param   regex
*          the regular expression to which this string is to be matched
*
* @return  <tt>true</tt> if, and only if, this string matches the
*          given regular expression
*
* @throws  PatternSyntaxException
*          if the regular expression's syntax is invalid
*
* @see java.util.regex.Pattern
*
* @since 1.4
* @spec JSR-51
public boolean matches(String regex) {
return Pattern.matches(regex, this);
}
)  ( *
* Replaces the first substring of this string that matches the given <a
* href="../util/regex/Pattern.html#sum">regular expression</a> with the
* given replacement.
*
* <p> An invocation of this method of the form
* <i>str</i><tt>.replaceFirst(</tt><i>regex</i><tt>,</tt> <i>repl</i><tt>)</tt>
* yields exactly the same result as the expression
*
* <blockquote><tt>
* {@link java.util.regex.Pattern}.{@link java.util.regex.Pattern#compile
* compile}(</tt><i>regex</i><tt>).{@link
* java.util.regex.Pattern#matcher(java.lang.CharSequence)
* matcher}(</tt><i>str</i><tt>).{@link java.util.regex.Matcher#replaceFirst
* replaceFirst}(</tt><i>repl</i><tt>)</tt></blockquote>
*
* @param   regex
*          the regular expression to which this string is to be matched
*
* @return  The resulting <tt>String</tt>
*
* @throws  PatternSyntaxException
*          if the regular expression's syntax is invalid
*
* @see java.util.regex.Pattern
*
* @since 1.4
* @spec JSR-51
public String replaceFirst(String regex, String replacement) {
return Pattern.compile(regex).matcher(this).replaceFirst(replacement);
}
)  ( *
* Replaces each substring of this string that matches the given <a
* href="../util/regex/Pattern.html#sum">regular expression</a> with the
* given replacement.
*
* <p> An invocation of this method of the form
* <i>str</i><tt>.replaceAll(</tt><i>regex</i><tt>,</tt> <i>repl</i><tt>)</tt>
* yields exactly the same result as the expression
*
* <blockquote><tt>
* {@link java.util.regex.Pattern}.{@link java.util.regex.Pattern#compile
* compile}(</tt><i>regex</i><tt>).{@link
* java.util.regex.Pattern#matcher(java.lang.CharSequence)
* matcher}(</tt><i>str</i><tt>).{@link java.util.regex.Matcher#replaceAll
* replaceAll}(</tt><i>repl</i><tt>)</tt></blockquote>
*
* @param   regex
*          the regular expression to which this string is to be matched
*
* @return  The resulting <tt>String</tt>
*
* @throws  PatternSyntaxException
*          if the regular expression's syntax is invalid
*
* @see java.util.regex.Pattern
*
* @since 1.4
* @spec JSR-51
public String replaceAll(String regex, String replacement) {
return Pattern.compile(regex).matcher(this).replaceAll(replacement);
}
)  ( *
* Splits this string around matches of the given <a
* href="{@docRoot}/java/util/regex/Pattern.html#sum">regular expression</a>.
*
* <p> The array returned by this method contains each substring of this
* string that is terminated by another substring that matches the given
* expression or is terminated by the end of the string.  The substrings in
* the array are in the order in which they occur in this string.  If the
* expression does not match any part of the input then the resulting array
* has just one element, namely this string.
*
* <p> The <tt>limit</tt> parameter controls the number of times the
* pattern is applied and therefore affects the length of the resulting
* array.  If the limit <i>n</i> is greater than zero then the pattern
* will be applied at most <i>n</i>&nbsp;-&nbsp;1 times, the array's
* length will be no greater than <i>n</i>, and the array's last entry
* will contain all input beyond the last matched delimiter.  If <i>n</i>
* is non-positive then the pattern will be applied as many times as
* possible and the array can have any length.  If <i>n</i> is zero then
* the pattern will be applied as many times as possible, the array can
* have any length, and trailing empty strings will be discarded.
*
* <p> The string <tt>"boo:and:foo"</tt>, for example, yields the
* following results with these parameters:
*
* <blockquote><table cellpadding=1 cellspacing=0 summary="Split example showing regex, limit, and result">
* <tr>
*     <th>Regex</th>
*     <th>Limit</th>
*     <th>Result</th>
* </tr>
* <tr><td align=center>:</td>
*     <td align=center>2</td>
*     <td><tt>{ "boo", "and:foo" }</tt></td></tr>
* <tr><td align=center>:</td>
*     <td align=center>5</td>
*     <td><tt>{ "boo", "and", "foo" }</tt></td></tr>
* <tr><td align=center>:</td>
*     <td align=center>-2</td>
*     <td><tt>{ "boo", "and", "foo" }</tt></td></tr>
* <tr><td align=center>o</td>
*     <td align=center>5</td>
*     <td><tt>{ "b", "", ":and:f", "", "" }</tt></td></tr>
* <tr><td align=center>o</td>
*     <td align=center>-2</td>
*     <td><tt>{ "b", "", ":and:f", "", "" }</tt></td></tr>
* <tr><td align=center>o</td>
*     <td align=center>0</td>
*     <td><tt>{ "b", "", ":and:f" }</tt></td></tr>
* </table></blockquote>
*
* <p> An invocation of this method of the form
* <i>str.</i><tt>split(</tt><i>regex</i><tt>,</tt>&nbsp;<i>n</i><tt>)</tt>
* yields the same result as the expression
*
* <blockquote>
* {@link java.util.regex.Pattern}.{@link java.util.regex.Pattern#compile
* compile}<tt>(</tt><i>regex</i><tt>)</tt>.{@link
* java.util.regex.Pattern#split(java.lang.CharSequence,int)
* split}<tt>(</tt><i>str</i><tt>,</tt>&nbsp;<i>n</i><tt>)</tt>
* </blockquote>
*
*
* @param  regex
*         the delimiting regular expression
*
* @param  limit
*         the result threshold, as described above
*
* @return  the array of strings computed by splitting this string
*          around matches of the given regular expression
*
* @throws  PatternSyntaxException
*          if the regular expression's syntax is invalid
*
* @see java.util.regex.Pattern
*
* @since 1.4
* @spec JSR-51
public String[] split(String regex, int limit) {
return Pattern.compile(regex).split(this, limit);
}
)  ( *
* Splits this string around matches of the given <a
* href="{@docRoot}/java/util/regex/Pattern.html#sum">regular expression</a>.
*
* <p> This method works as if by invoking the two-argument {@link
* #split(String, int) split} method with the given expression and a limit
* argument of zero.  Trailing empty strings are therefore not included in
* the resulting array.
*
* <p> The string <tt>"boo:and:foo"</tt>, for example, yields the following
* results with these expressions:
*
* <blockquote><table cellpadding=1 cellspacing=0 summary="Split examples showing regex and result">
* <tr>
*  <th>Regex</th>
*  <th>Result</th>
* </tr>
* <tr><td align=center>:</td>
*     <td><tt>{ "boo", "and", "foo" }</tt></td></tr>
* <tr><td align=center>o</td>
*     <td><tt>{ "b", "", ":and:f" }</tt></td></tr>
* </table></blockquote>
*
*
* @param  regex
*         the delimiting regular expression
*
* @return  the array of strings computed by splitting this string
*          around matches of the given regular expression
*
* @throws  PatternSyntaxException
*          if the regular expression's syntax is invalid
*
* @see java.util.regex.Pattern
*
* @since 1.4
* @spec JSR-51
public String[] split(String regex) {
return split(regex, 0);
}
)

:LOCAL String.toLowerCase§20804608
   3 VALLOCATE LOCAL §base0
   DUP 0 §base0 + V! LOCAL §this
   
   \ new statement
   0 DUP DUP DUP DUP
   LOCALS firstUpper len off result val |
   
   \ new statement
   §this CELL+ @ 36 + ( equation.String.count )    @
   TO len
   
   \ new statement
   §this CELL+ @ 32 + ( equation.String.offset )    @
   TO off
   
   \ new statement
   §this CELL+ @ 28 + ( equation.String.value )    @
   DUP 4 §base0 + V! TO val
   
   \ new statement
   (  Now check if there are any characters that need to be changed.  )
   
   \ new statement
   
   \ new statement
   0
   LOCALS c |
   
   \ new statement
   0
   TO firstUpper
   
   \ new statement
   BEGIN
      
      \ new statement
      firstUpper
      len
      <
      
      \ new statement
   WHILE
      
      \ new statement
      §this CELL+ @ 28 + ( equation.String.value ) @ >R
      off
      firstUpper
      +
      R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
      H@
      TO c
      
      \ new statement
      c
      c
      String.toLowerCase§-1264749568
      <>
      
      \ new statement
      IF
         
         \ new statement
         
         \ new statement
         131074 §break28348 BRANCH
         
         \ new statement
         0 §break28348 LABEL
         
         \ new statement
         
         \ new statement
         0>!
         IF
            10001H - 0<! §break28346 0BRANCH
         ENDIF
         DROP
         
         \ new statement
      ENDIF
      
      \ new statement
      
      \ new statement
      firstUpper 1+ TO firstUpper
      
      \ new statement
   REPEAT
   
   \ new statement
   0 §break28346 LABEL
   
   \ new statement
   
   PURGE 1
   
   \ new statement
   0>!
   IF
      10001H - §break28345 BRANCH
   ENDIF
   DROP
   
   \ new statement
   §this
   DUP 0 V!
   65537 §break28345 BRANCH
   
   \ new statement
   0 §break28345 LABEL
   
   \ new statement
   
   \ new statement
   0>!
   IF
      10001H - §break28344 BRANCH
   ENDIF
   DROP
   
   \ new statement
   0 DROP
   
   \ new statement
   
   1 VALLOCATE LOCAL §tempvar
   §this CELL+ @ 36 + ( equation.String.count )    @ 131073 JavaArray§1352878592.table -227194368 EXECUTE-NEW
   DUP 8 §base0 + V! TO result
   
   §tempvar SETVTOP
   
   PURGE 1
   
   
   \ new statement
   (  Just copy the first few lowerCase characters.  )
   
   \ new statement
   val
   off
   result
   0
   firstUpper
   String.arraycopy§1134321920
   
   \ new statement
   \  normal, fast loop
   
   
   \ new statement
   0
   LOCALS i |
   
   \ new statement
   firstUpper
   TO i
   
   \ new statement
   BEGIN
      
      \ new statement
      i
      len
      <
      
      \ new statement
   WHILE
      
      \ new statement
      
      0
      LOCALS 0§ |
      
      i TO 0§
      val >R
      off
      i
      +
      R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
      H@
      String.toLowerCase§-1264749568
      result >R
      0§
      R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
      H!
      
      PURGE 1
      
      
      \ new statement
      
      \ new statement
      i 1+ TO i
      
      \ new statement
   REPEAT
   
   \ new statement
   0 §break28349 LABEL
   
   \ new statement
   
   PURGE 1
   
   \ new statement
   0>!
   IF
      10001H - §break28344 BRANCH
   ENDIF
   DROP
   
   \ new statement
   result
   0
   result CELL+ @ 28 + ( equation.JavaArray.length )    @
   String§1651856128.table 1127437056 EXECUTE-NEW
   DUP 0 V!
   0 §break28344 BRANCH
   
   \ new statement
   0 §break28344 LABEL
   
   \ new statement
   
   
   
   
   
   §base0 SETVTOP
   PURGE 7
   
   \ new statement
   DROP
;
( *
* This object (which is already a string!) is itself returned.
*
* @return  the string itself.
)

:LOCAL String.toString§1621718016
   1 VALLOCATE LOCAL §base0
   DUP 0 §base0 + V! LOCAL §this
   
   \ new statement
   
   \ new statement
   §this
   DUP 0 V!
   0 §break28365 BRANCH
   
   \ new statement
   0 §break28365 LABEL
   
   \ new statement
   §base0 SETVTOP
   PURGE 2
   
   \ new statement
   DROP
;
( *
* Converts all of the characters in this <code>String</code> to upper
* case using the rules of the given <code>Locale</code>. Case mappings rely
* heavily on the Unicode specification's character data. Since case mappings
* are not always 1:1 char mappings, the resulting <code>String</code> may
* be a different length than the original <code>String</code>.
* <p>
* Examples of locale-sensitive and 1:M case mappings are in the following table.
* <p>
* <table border="1" summary="Examples of locale-sensitive and 1:M case mappings. Shows Language code of locale, lower case, upper case, and description.">
* <tr>
*   <th>Language Code of Locale</th>
*   <th>Lower Case</th>
*   <th>Upper Case</th>
*   <th>Description</th>
* </tr>
* <tr>
*   <td>tr (Turkish)</td>
*   <td>&#92;u0069</td>
*   <td>&#92;u0130</td>
*   <td>small letter i -&gt; capital letter I with dot above</td>
* </tr>
* <tr>
*   <td>tr (Turkish)</td>
*   <td>&#92;u0131</td>
*   <td>&#92;u0049</td>
*   <td>small letter dotless i -&gt; capital letter I</td>
* </tr>
* <tr>
*   <td>(all)</td>
*   <td>&#92;u00df</td>
*   <td>&#92;u0053 &#92;u0053</td>
*   <td>small letter sharp s -&gt; two letters: SS</td>
* </tr>
* <tr>
*   <td>(all)</td>
*   <td>Fahrvergn&uuml;gen</td>
*   <td>FAHRVERGN&Uuml;GEN</td>
*   <td></td>
* </tr>
* </table>
* @param locale use the case transformation rules for this locale
* @return the <code>String</code>, converted to uppercase.
* @see     java.lang.String#toUpperCase()
* @see     java.lang.String#toLowerCase()
* @see     java.lang.String#toLowerCase(Locale)
* @since   1.1
)

:LOCAL String.toUpperCase§1674802176
   3 VALLOCATE LOCAL §base0
   DUP 0 §base0 + V! LOCAL §this
   
   \ new statement
   0 DUP DUP DUP DUP DUP
   LOCALS firstLower len off result resultOffset val |
   
   \ new statement
   §this CELL+ @ 36 + ( equation.String.count )    @
   TO len
   
   \ new statement
   §this CELL+ @ 32 + ( equation.String.offset )    @
   TO off
   
   \ new statement
   §this CELL+ @ 28 + ( equation.String.value )    @
   DUP 4 §base0 + V! TO val
   
   \ new statement
   (  Now check if there are any characters that need changing.  )
   
   \ new statement
   0 DUP
   LOCALS c upperCaseChar |
   
   \ new statement
   
   \ new statement
   0
   TO firstLower
   
   \ new statement
   BEGIN
      
      \ new statement
      firstLower
      len
      <
      
      \ new statement
   WHILE
      
      \ new statement
      §this CELL+ @ 28 + ( equation.String.value ) @ >R
      off
      firstLower
      +
      R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
      H@
      TO c
      
      \ new statement
      c
      String.toUpperCase§1730376704
      TO upperCaseChar
      
      \ new statement
      c
      upperCaseChar
      <>
      
      \ new statement
      IF
         
         \ new statement
         
         \ new statement
         131074 §break28355 BRANCH
         
         \ new statement
         0 §break28355 LABEL
         
         \ new statement
         
         \ new statement
         0>!
         IF
            10001H - 0<! §break28353 0BRANCH
         ENDIF
         DROP
         
         \ new statement
      ENDIF
      
      \ new statement
      
      \ new statement
      firstLower 1+ TO firstLower
      
      \ new statement
   REPEAT
   
   \ new statement
   0 §break28353 LABEL
   
   \ new statement
   
   \ new statement
   0>!
   IF
      10001H - §break28352 BRANCH
   ENDIF
   DROP
   
   \ new statement
   §this
   DUP 0 V!
   65537 §break28352 BRANCH
   
   \ new statement
   0 §break28352 LABEL
   
   \ new statement
   
   
   PURGE 2
   
   \ new statement
   0>!
   IF
      10001H - §break28351 BRANCH
   ENDIF
   DROP
   
   \ new statement
   0 DROP
   
   \ new statement
   
   1 VALLOCATE LOCAL §tempvar
   len 131073 JavaArray§1352878592.table -227194368 EXECUTE-NEW
   DUP 8 §base0 + V! TO result
   
   §tempvar SETVTOP
   
   PURGE 1
   
   
   \ new statement
   (  might grow!  )
   
   \ new statement
   0
   TO resultOffset
   
   \ new statement
   (  result grows, so i+resultOffset
   * is the write location in result  )  (  Just copy the first few upperCase characters.  )
   
   \ new statement
   val
   off
   result
   0
   firstLower
   String.arraycopy§1134321920
   
   \ new statement
   1 VALLOCATE LOCAL §base1
   0 DUP DUP
   LOCALS ch upperChar upperCharArray |
   
   \ new statement
   \  normal, fast loop
   
   
   \ new statement
   0
   LOCALS i |
   
   \ new statement
   firstLower
   TO i
   
   \ new statement
   BEGIN
      
      \ new statement
      i
      len
      <
      
      \ new statement
   WHILE
      
      \ new statement
      val >R
      off
      i
      +
      R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
      H@
      TO ch
      
      \ new statement
      ch
      String.toUpperCase§1730376704
      TO upperChar
      
      \ new statement
      
      \ new statement
      
      0
      LOCALS 0§ |
      
      i
      resultOffset
      + TO 0§
      upperChar
      result >R
      0§
      R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
      H!
      
      PURGE 1
      
      
      \ new statement
      0 §break28359 LABEL
      
      \ new statement
      
      \ new statement
      DROP
      
      \ new statement
      
      \ new statement
      i 1+ TO i
      
      \ new statement
   REPEAT
   
   \ new statement
   0 §break28357 LABEL
   
   \ new statement
   
   PURGE 1
   
   \ new statement
   DROP
   
   \ new statement
   0 §break28356 LABEL
   
   \ new statement
   
   
   
   §base1 SETVTOP
   PURGE 4
   
   \ new statement
   0>!
   IF
      10001H - §break28351 BRANCH
   ENDIF
   DROP
   
   \ new statement
   result
   0
   result CELL+ @ 28 + ( equation.JavaArray.length )    @
   String§1651856128.table 1127437056 EXECUTE-NEW
   DUP 0 V!
   0 §break28351 BRANCH
   
   \ new statement
   0 §break28351 LABEL
   
   \ new statement
   
   
   
   
   
   
   §base0 SETVTOP
   PURGE 8
   
   \ new statement
   DROP
;


: String.toUpperCase§1730376704
   LOCAL a
   
   \ new statement
   
   \ new statement
   a
   97
   < INVERT and_32125 0BRANCH! DROP
   a
   122
   > INVERT
   and_32125 LABEL
   
   \ new statement
   IF
      
      \ new statement
      a
      97
      -
      65
      +  0FFFFH AND
      TO a
      
      \ new statement
   ENDIF
   
   \ new statement
   a
   0 §break28262 BRANCH
   
   \ new statement
   0 §break28262 LABEL
   
   \ new statement
   
   PURGE 1
   
   \ new statement
   DROP
;
( *
* Returns a copy of the string, with leading and trailing whitespace
* omitted.
* <p>
* If this <code>String</code> object represents an empty character
* sequence, or the first and last characters of character sequence
* represented by this <code>String</code> object both have codes
* greater than <code>'&#92;u0020'</code> (the space character), then a
* reference to this <code>String</code> object is returned.
* <p>
* Otherwise, if there is no character with a code greater than
* <code>'&#92;u0020'</code> in the string, then a new
* <code>String</code> object representing an empty string is created
* and returned.
* <p>
* Otherwise, let <i>k</i> be the index of the first character in the
* string whose code is greater than <code>'&#92;u0020'</code>, and let
* <i>m</i> be the index of the last character in the string whose code
* is greater than <code>'&#92;u0020'</code>. A new <code>String</code>
* object is created, representing the substring of this string that
* begins with the character at index <i>k</i> and ends with the
* character at index <i>m</i>-that is, the result of
* <code>this.substring(<i>k</i>,&nbsp;<i>m</i>+1)</code>.
* <p>
* This method may be used to trim
* {@link Character#isSpace(char) whitespace} from the beginning and end
* of a string; in fact, it trims all ASCII control characters as well.
*
* @return  A copy of this string with leading and trailing white
*          space removed, or this string if it has no leading or
*          trailing white space.
)

:LOCAL String.trim§-1067748352
   2 VALLOCATE LOCAL §base0
   DUP 0 §base0 + V! LOCAL §this
   
   \ new statement
   0 DUP DUP DUP
   LOCALS len off st val |
   
   \ new statement
   §this CELL+ @ 36 + ( equation.String.count )    @
   TO len
   
   \ new statement
   0
   TO st
   
   \ new statement
   §this CELL+ @ 32 + ( equation.String.offset )    @
   TO off
   
   \ new statement
   (  avoid getfield opcode  )
   
   \ new statement
   §this CELL+ @ 28 + ( equation.String.value )    @
   DUP 4 §base0 + V! TO val
   
   \ new statement
   (  avoid getfield opcode  )
   
   \ new statement
   
   \ new statement
   BEGIN
      
      \ new statement
      st
      len
      <
      and_32143 0BRANCH! DROP
      val >R
      off
      st
      +
      R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
      H@
      32
      > INVERT
      and_32143 LABEL
      
      \ new statement
   WHILE
      
      \ new statement
      st 1+ TO st
      
      \ new statement
      
      \ new statement
   REPEAT
   
   \ new statement
   0 §break28361 LABEL
   
   \ new statement
   
   \ new statement
   DROP
   
   \ new statement
   
   \ new statement
   BEGIN
      
      \ new statement
      st
      len
      <
      and_32154 0BRANCH! DROP
      val >R
      off
      len
      +
      1
      -
      R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
      H@
      32
      > INVERT
      and_32154 LABEL
      
      \ new statement
   WHILE
      
      \ new statement
      len 1- TO len
      
      \ new statement
      
      \ new statement
   REPEAT
   
   \ new statement
   0 §break28363 LABEL
   
   \ new statement
   
   \ new statement
   DROP
   
   \ new statement
   st
   0>
   0=! or_32163 0BRANCH DROP
   len
   §this CELL+ @ 36 + ( equation.String.count )    @
   <
   or_32163 LABEL
   
   IF
      §this >R
      st
      len
      R> -424054016 TRUE ( equation.String.substring§-424054016 ) EXECUTE-METHOD
      
   ELSE
      §this
      
   ENDIF
   DUP 0 V!
   0 §break28360 BRANCH
   
   \ new statement
   0 §break28360 LABEL
   
   \ new statement
   
   
   
   
   §base0 SETVTOP
   PURGE 6
   
   \ new statement
   DROP
;
( *
* Returns the string representation of the <code>int</code> argument.
* <p>
* The representation is exactly the one returned by the
* <code>Integer.toString</code> method of one argument.
*
* @param   i   an <code>int</code>.
* @return  a string representation of the <code>int</code> argument.
* @see     java.lang.Integer#toString(int, int)
)

: String.valueOf§-166496768
   LOCAL i
   
   \ new statement
   
   \ new statement
   
   2 VALLOCATE LOCAL §tempvar
   i  <# 0 ..R JavaArray.createString§-105880832 DUP §tempvar 4 + V!
   U"  " COUNT JavaArray.createUnicode§-675323136 DUP §tempvar V!
   SWAP  -1461427456 TRUE ( equation.String.concat§-1461427456 ) EXECUTE-METHOD
   DUP 0 V!
   
   
   §tempvar SETVTOP
   
   PURGE 1
   
   0 §break28374 BRANCH
   
   \ new statement
   0 §break28374 LABEL
   
   \ new statement
   
   PURGE 1
   
   \ new statement
   DROP
;
( *
* Returns the string representation of the <code>float</code> argument.
* <p>
* The representation is exactly the one returned by the
* <code>Float.toString</code> method of one argument.
*
* @param   f   a <code>float</code>.
* @return  a string representation of the <code>float</code> argument.
* @see     java.lang.Float#toString(float)
public static String valueOf(float f) {
return Float.toString(f);
}
)  ( *
* Returns the string representation of the <code>double</code> argument.
* <p>
* The representation is exactly the one returned by the
* <code>Double.toString</code> method of one argument.
*
* @param   d   a <code>double</code>.
* @return  a  string representation of the <code>double</code> argument.
* @see     java.lang.Double#toString(double)
)

: String.valueOf§-721717760
   2LOCAL d
   
   \ new statement
   
   \ new statement
   
   2 VALLOCATE LOCAL §tempvar
   d  F.. JavaArray.createString§-105880832 DUP §tempvar 4 + V!
   U"  " COUNT JavaArray.createUnicode§-675323136 DUP §tempvar V!
   SWAP  -1461427456 TRUE ( equation.String.concat§-1461427456 ) EXECUTE-METHOD
   DUP 0 V!
   
   
   §tempvar SETVTOP
   
   PURGE 1
   
   0 §break28376 BRANCH
   
   \ new statement
   0 §break28376 LABEL
   
   \ new statement
   
   PURGE 1
   
   \ new statement
   DROP
;
( *
* Returns the string representation of the <code>boolean</code> argument.
*
* @param   b   a <code>boolean</code>.
* @return  if the argument is <code>true</code>, a string equal to
*          <code>"true"</code> is returned; otherwise, a string equal to
*          <code>"false"</code> is returned.
)

: String.valueOf§-838633984
   LOCAL b
   
   \ new statement
   
   \ new statement
   b
   IF
      U" true " COUNT JavaArray.createUnicode§-675323136
      
   ELSE
      U" false " COUNT JavaArray.createUnicode§-675323136
      
   ENDIF
   DUP 0 V!
   0 §break28372 BRANCH
   
   \ new statement
   0 §break28372 LABEL
   
   \ new statement
   
   PURGE 1
   
   \ new statement
   DROP
;
( *
* Returns the string representation of the <code>long</code> argument.
* <p>
* The representation is exactly the one returned by the
* <code>Long.toString</code> method of one argument.
*
* @param   l   a <code>long</code>.
* @return  a string representation of the <code>long</code> argument.
* @see     java.lang.Long#toString(long)
)

: String.valueOf§-85035520
   2LOCAL l
   
   \ new statement
   
   \ new statement
   
   2 VALLOCATE LOCAL §tempvar
   l  <# 0 D..R JavaArray.createString§-105880832 DUP §tempvar 4 + V!
   U"  " COUNT JavaArray.createUnicode§-675323136 DUP §tempvar V!
   SWAP  -1461427456 TRUE ( equation.String.concat§-1461427456 ) EXECUTE-METHOD
   DUP 0 V!
   
   
   §tempvar SETVTOP
   
   PURGE 1
   
   0 §break28375 BRANCH
   
   \ new statement
   0 §break28375 LABEL
   
   \ new statement
   
   PURGE 1
   
   \ new statement
   DROP
;
( *
* Returns the string representation of the <code>char</code>
* argument.
*
* @param   c   a <code>char</code>.
* @return  a string of length <code>1</code> containing
*          as its single character the argument <code>c</code>.
)

: String.valueOf§1048802816
   1 VALLOCATE LOCAL §base0
   LOCAL c
   
   \ new statement
   0
   LOCALS data |
   
   \ new statement
   
   1 VALLOCATE LOCAL §tempvar
   1
   131073 JavaArray§1352878592.table -227194368 EXECUTE-NEW DUP §tempvar V!
   c OVER 0  SWAP -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD  H!
   DUP §base0 V! TO data
   
   §tempvar SETVTOP
   
   PURGE 1
   
   
   \ new statement
   data
   0
   1
   String§1651856128.table 1127437056 EXECUTE-NEW
   DUP 0 V!
   0 §break28373 BRANCH
   
   \ new statement
   0 §break28373 LABEL
   
   \ new statement
   
   
   §base0 SETVTOP
   PURGE 3
   
   \ new statement
   DROP
;
( *
* Returns the string representation of the <code>Object</code> argument.
*
* @param   obj   an <code>Object</code>.
* @return  if the argument is <code>null</code>, then a string equal to
*          <code>"null"</code>; otherwise, the value of
*          <code>obj.toString()</code> is returned.
* @see     java.lang.Object#toString()
)

: String.valueOf§59340288
   1 VALLOCATE LOCAL §base0
   DUP 0 §base0 + V! LOCAL obj
   
   \ new statement
   
   \ new statement
   obj
   0=
   
   IF
      U" null " COUNT JavaArray.createUnicode§-675323136
      
   ELSE
      obj >R
      R> 1621718016 TRUE ( equation.Object.toString§1621718016 ) EXECUTE-METHOD
      
   ENDIF
   DUP 0 V!
   0 §break28367 BRANCH
   
   \ new statement
   0 §break28367 LABEL
   
   \ new statement
   
   §base0 SETVTOP
   PURGE 2
   
   \ new statement
   DROP
;
( *
* Returns the string representation of a specific subarray of the
* <code>char</code> array argument.
* <p>
* The <code>offset</code> argument is the index of the first
* character of the subarray. The <code>count</code> argument
* specifies the length of the subarray. The contents of the subarray
* are copied; subsequent modification of the character array does not
* affect the newly created string.
*
* @param   data     the character array.
* @param   offset   the initial offset into the value of the
*                  <code>String</code>.
* @param   count    the length of the value of the <code>String</code>.
* @return  a string representing the sequence of characters contained
*          in the subarray of the character array argument.
* @exception IndexOutOfBoundsException if <code>offset</code> is
*          negative, or <code>count</code> is negative, or
*          <code>offset+count</code> is larger than
*          <code>data.length</code>.
)

: String.valueOf§60651008
   1 VALLOCATE LOCAL §base0
   LOCAL count
   LOCAL offset
   DUP 0 §base0 + V! LOCAL data
   
   \ new statement
   
   \ new statement
   data
   offset
   count
   String§1651856128.table 1127437056 EXECUTE-NEW
   DUP 0 V!
   0 §break28369 BRANCH
   
   \ new statement
   0 §break28369 LABEL
   
   \ new statement
   
   
   
   §base0 SETVTOP
   PURGE 4
   
   \ new statement
   DROP
;
( *
* Returns the string representation of the <code>char</code> array
* argument. The contents of the character array are copied; subsequent
* modification of the character array does not affect the newly
* created string.
*
* @param   data   a <code>char</code> array.
* @return  a newly allocated string representing the same sequence of
*          characters contained in the character array argument.
)

: String.valueOf§789214720
   1 VALLOCATE LOCAL §base0
   DUP 0 §base0 + V! LOCAL data
   
   \ new statement
   
   \ new statement
   data
   String§1651856128.table -99003648 EXECUTE-NEW
   DUP 0 V!
   0 §break28368 BRANCH
   
   \ new statement
   0 §break28368 LABEL
   
   \ new statement
   
   §base0 SETVTOP
   PURGE 2
   
   \ new statement
   DROP
;
VARIABLE String.CASE_INSENSITIVE_ORDER  HERE 4 - SALLOCATE
VARIABLE String._staticBlocking
VARIABLE String._staticThread  HERE 4 - SALLOCATE
2VARIABLE String.serialVersionUID

A:HERE VARIABLE String§1651856128.table 43 DUP 2* CELLS ALLOT R@ ! A:CELL+
-1584442624 R@ ! A:CELL+ String.String§-1584442624 VAL R@ ! A:CELL+
-1626909952 R@ ! A:CELL+ String.String§-1626909952 VAL R@ ! A:CELL+
-1936633088 R@ ! A:CELL+ String.String§-1936633088 VAL R@ ! A:CELL+
-2016390400 R@ ! A:CELL+ String.String§-2016390400 VAL R@ ! A:CELL+
-99003648 R@ ! A:CELL+ String.String§-99003648 VAL R@ ! A:CELL+
1127437056 R@ ! A:CELL+ String.String§1127437056 VAL R@ ! A:CELL+
1136743168 R@ ! A:CELL+ String.String§1136743168 VAL R@ ! A:CELL+
1508463360 R@ ! A:CELL+ String.String§1508463360 VAL R@ ! A:CELL+
1844794112 R@ ! A:CELL+ String.String§1844794112 VAL R@ ! A:CELL+
-1014275328 R@ ! A:CELL+ String.charAt§-1014275328 VAL R@ ! A:CELL+
-879140096 R@ ! A:CELL+ String.compareTo§-879140096 VAL R@ ! A:CELL+
1883005696 R@ ! A:CELL+ String.compareTo§1883005696 VAL R@ ! A:CELL+
-466132224 R@ ! A:CELL+ String.compareToIgnoreCase§-466132224 VAL R@ ! A:CELL+
-1461427456 R@ ! A:CELL+ String.concat§-1461427456 VAL R@ ! A:CELL+
-676764416 R@ ! A:CELL+ String.endsWith§-676764416 VAL R@ ! A:CELL+
-240098048 R@ ! A:CELL+ String.equals§-240098048 VAL R@ ! A:CELL+
-518953728 R@ ! A:CELL+ String.equalsIgnoreCase§-518953728 VAL R@ ! A:CELL+
-457480448 R@ ! A:CELL+ String.getBytes§-457480448 VAL R@ ! A:CELL+
-601004288 R@ ! A:CELL+ String.getBytes§-601004288 VAL R@ ! A:CELL+
-53713152 R@ ! A:CELL+ String.getChars§-53713152 VAL R@ ! A:CELL+
-1604556800 R@ ! A:CELL+ String.hashCode§-1604556800 VAL R@ ! A:CELL+
-337549056 R@ ! A:CELL+ String.indexOf§-337549056 VAL R@ ! A:CELL+
517761280 R@ ! A:CELL+ String.indexOf§517761280 VAL R@ ! A:CELL+
731343104 R@ ! A:CELL+ String.indexOf§731343104 VAL R@ ! A:CELL+
996698368 R@ ! A:CELL+ String.indexOf§996698368 VAL R@ ! A:CELL+
-1299878912 R@ ! A:CELL+ String.lastIndexOf§-1299878912 VAL R@ ! A:CELL+
-1735627776 R@ ! A:CELL+ String.lastIndexOf§-1735627776 VAL R@ ! A:CELL+
-465605632 R@ ! A:CELL+ String.lastIndexOf§-465605632 VAL R@ ! A:CELL+
147155968 R@ ! A:CELL+ String.lastIndexOf§147155968 VAL R@ ! A:CELL+
188050432 R@ ! A:CELL+ String.length§188050432 VAL R@ ! A:CELL+
1793290752 R@ ! A:CELL+ String.regionMatches§1793290752 VAL R@ ! A:CELL+
2060349952 R@ ! A:CELL+ String.regionMatches§2060349952 VAL R@ ! A:CELL+
-1850510848 R@ ! A:CELL+ String.replace§-1850510848 VAL R@ ! A:CELL+
-953322752 R@ ! A:CELL+ String.startsWith§-953322752 VAL R@ ! A:CELL+
970093312 R@ ! A:CELL+ String.startsWith§970093312 VAL R@ ! A:CELL+
602239744 R@ ! A:CELL+ String.subSequence§602239744 VAL R@ ! A:CELL+
-424054016 R@ ! A:CELL+ String.substring§-424054016 VAL R@ ! A:CELL+
18576128 R@ ! A:CELL+ String.substring§18576128 VAL R@ ! A:CELL+
629044224 R@ ! A:CELL+ String.toCharArray§629044224 VAL R@ ! A:CELL+
20804608 R@ ! A:CELL+ String.toLowerCase§20804608 VAL R@ ! A:CELL+
1621718016 R@ ! A:CELL+ String.toString§1621718016 VAL R@ ! A:CELL+
1674802176 R@ ! A:CELL+ String.toUpperCase§1674802176 VAL R@ ! A:CELL+
-1067748352 R@ ! A:CELL+ String.trim§-1067748352 VAL R@ ! A:CELL+
A:DROP


:LOCAL String.CaseInsensitiveComparator.CaseInsensitiveComparator§-42253568
   2 VALLOCATE LOCAL §base0
   
   \ new statement
   0
   LOCALS §this |
   
   \ new statement
   Object§-1890496768.table 1827294976 EXECUTE-NEW
   
   \ new statement
   28 MALLOC DROP DUP DUP §base0 V! TO §this OVER
   IF
      OVER CELL+ @ 4 + OVER OVER ! DROP
   ENDIF
   CELL+ @ OVER
   OVER ! NIP
   String.CaseInsensitiveComparator§-2000076032.table OVER 12 + !
   -2000093184 OVER 20 + !
   " CaseInsensitiveComparator " OVER 16 + !
   0 OVER 24 + ! DROP
   
   \ new statement
   §this DUP 0 V! ( return object )    
   \ new statement
   0 §break28261 LABEL
   
   \ new statement
   §base0 SETVTOP
   PURGE 2
   
   \ new statement
   DROP
;


:LOCAL String.CaseInsensitiveComparator.compare§2079286016
   5 VALLOCATE LOCAL §base0
   DUP 8 §base0 + V! LOCAL §this
   DUP 0 §base0 + V! LOCAL o2
   DUP 4 §base0 + V! LOCAL o1
   
   \ new statement
   0 DUP DUP DUP
   LOCALS n1 n2 s1 s2 |
   
   \ new statement
   o1
   DUP 12 §base0 + V! TO s1
   
   \ new statement
   o2
   DUP 16 §base0 + V! TO s2
   
   \ new statement
   s1 >R
   R> 188050432 TRUE ( equation.String.length§188050432 ) EXECUTE-METHOD
   TO n1
   
   \ new statement
   s2 >R
   R> 188050432 TRUE ( equation.String.length§188050432 ) EXECUTE-METHOD
   TO n2
   
   \ new statement
   0 DUP DUP DUP
   LOCALS c1 c2 i1 i2 |
   
   \ new statement
   0
   TO i1
   
   \ new statement
   0
   TO i2
   
   \ new statement
   BEGIN
      
      \ new statement
      i1
      n1
      < and_32232 0BRANCH! DROP
      i2
      n2
      <
      and_32232 LABEL
      
      \ new statement
   WHILE
      
      \ new statement
      s1 >R
      i1
      R> -1014275328 TRUE ( equation.String.charAt§-1014275328 ) EXECUTE-METHOD
      TO c1
      
      \ new statement
      s2 >R
      i2
      R> -1014275328 TRUE ( equation.String.charAt§-1014275328 ) EXECUTE-METHOD
      TO c2
      
      \ new statement
      c1
      c2
      <>
      
      \ new statement
      IF
         
         \ new statement
         
         \ new statement
         c1
         String.toUpperCase§1730376704
         TO c1
         
         \ new statement
         c2
         String.toUpperCase§1730376704
         TO c2
         
         \ new statement
         c1
         c2
         <>
         
         \ new statement
         IF
            
            \ new statement
            
            \ new statement
            c1
            String.toLowerCase§-1264749568
            TO c1
            
            \ new statement
            c2
            String.toLowerCase§-1264749568
            TO c2
            
            \ new statement
            c1
            c2
            <>
            
            \ new statement
            IF
               
               \ new statement
               
               \ new statement
               c1
               c2
               -
               262148 §break28260 BRANCH
               
               \ new statement
               0 §break28260 LABEL
               
               \ new statement
               
               \ new statement
               0>!
               IF
                  10001H - §break28259 BRANCH
               ENDIF
               DROP
               
               \ new statement
            ENDIF
            
            \ new statement
            0 §break28259 LABEL
            
            \ new statement
            
            \ new statement
            0>!
            IF
               10001H - §break28258 BRANCH
            ENDIF
            DROP
            
            \ new statement
         ENDIF
         
         \ new statement
         0 §break28258 LABEL
         
         \ new statement
         
         \ new statement
         0>!
         IF
            10001H - 0<! §break28256 0BRANCH
         ENDIF
         DROP
         
         \ new statement
      ENDIF
      
      \ new statement
      
      \ new statement
      i1 1+ TO i1
      
      \ new statement
      i2 1+ TO i2
      
      \ new statement
   REPEAT
   
   \ new statement
   0 §break28256 LABEL
   
   \ new statement
   
   
   
   
   PURGE 4
   
   \ new statement
   0>!
   IF
      10001H - §break28255 BRANCH
   ENDIF
   DROP
   
   \ new statement
   n1
   n2
   -
   0 §break28255 BRANCH
   
   \ new statement
   0 §break28255 LABEL
   
   \ new statement
   
   
   
   
   
   
   §base0 SETVTOP
   PURGE 8
   
   \ new statement
   DROP
;
VARIABLE String.CaseInsensitiveComparator._staticBlocking
VARIABLE String.CaseInsensitiveComparator._staticThread  HERE 4 - SALLOCATE
2VARIABLE String.CaseInsensitiveComparator.serialVersionUID

A:HERE VARIABLE String.CaseInsensitiveComparator§-2000076032.table 2 DUP 2* CELLS ALLOT R@ ! A:CELL+
-42253568 R@ ! A:CELL+ String.CaseInsensitiveComparator.CaseInsensitiveComparator§-42253568 VAL R@ ! A:CELL+
2079286016 R@ ! A:CELL+ String.CaseInsensitiveComparator.compare§2079286016 VAL R@ ! A:CELL+
A:DROP
