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
   
   \ new statement
   0
   LOCALS §this |
   
   \ new statement
   Object§-1890496768.table 1827294976 EXECUTE-NEW
   
   \ new statement
   44 MALLOC DROP DUP TO §this OVER
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
   0
   131073 JavaArray§1352878592.table -227194368 EXECUTE-NEW
   §this CELL+ @ 28 + ( equation.String.value )    DUP @ TRUE JavaArray.kill§1620077312 !
   
   \ new statement
   §this ( return object )    
   \ new statement
   0 §break64 LABEL
   
   \ new statement
   PURGE 1
   
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
   LOCAL length
   LOCAL offset
   LOCAL bytes
   
   \ new statement
   0
   LOCALS §this |
   
   \ new statement
   Object§-1890496768.table 1827294976 EXECUTE-NEW
   
   \ new statement
   44 MALLOC DROP DUP TO §this OVER
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
   length
   offset
   -
   1
   + 131073 JavaArray§1352878592.table -227194368 EXECUTE-NEW
   §this CELL+ @ 28 + ( equation.String.value )    DUP @ TRUE JavaArray.kill§1620077312 !
   
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
      bytes A:R@
      offset
      R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
      C@ A:R@
      A:DROP
      §this CELL+ @ 28 + ( equation.String.value ) @ A:R@
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
   0 §break81 LABEL
   
   \ new statement
   
   PURGE 1
   
   \ new statement
   DROP
   
   \ new statement
   §this ( return object )    
   \ new statement
   0 §break80 LABEL
   
   \ new statement
   bytes TRUE JavaArray.kill§1620077312
   
   
   PURGE 4
   
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
   LOCAL count
   LOCAL offset
   LOCAL hibyte
   LOCAL ascii
   
   \ new statement
   0 DUP
   LOCALS value §this |
   
   \ new statement
   Object§-1890496768.table 1827294976 EXECUTE-NEW
   
   \ new statement
   44 MALLOC DROP DUP TO §this OVER
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
   count 131073 JavaArray§1352878592.table -227194368 EXECUTE-NEW
   value TRUE JavaArray.kill§1620077312 TO value
   
   \ new statement
   count
   §this CELL+ @ 36 + ( equation.String.count )    !
   
   \ new statement
   value DUP INCREFERENCE
   §this CELL+ @ 28 + ( equation.String.value )    DUP @ TRUE JavaArray.kill§1620077312 !
   
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
         ascii A:R@
         i
         offset
         +
         R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
         C@
         255
         AND
         0FFFFH AND
         value A:R@
         0§
         R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
         H!
         
         PURGE 1
         
         
         \ new statement
         
         \ new statement
      REPEAT
      
      \ new statement
      0 §break74 LABEL
      
      \ new statement
      
      PURGE 1
      
      \ new statement
      DROP
      
      \ new statement
      0 §break73 LABEL
      
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
         ascii A:R@
         i
         offset
         +
         R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
         C@
         255
         AND
         OR
         0FFFFH AND
         value A:R@
         0§
         R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
         H!
         
         PURGE 1
         
         
         \ new statement
         
         \ new statement
      REPEAT
      
      \ new statement
      0 §break77 LABEL
      
      \ new statement
      
      PURGE 1
      
      \ new statement
      DROP
      
      \ new statement
      0 §break76 LABEL
      
      \ new statement
      
      \ new statement
      DROP
      
      \ new statement
   ENDIF
   
   \ new statement
   §this ( return object )    
   \ new statement
   0 §break72 LABEL
   
   \ new statement
   ascii TRUE JavaArray.kill§1620077312
   
   
   
   value TRUE JavaArray.kill§1620077312
   PURGE 6
   
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
   LOCAL bytes
   
   \ new statement
   0
   LOCALS §this |
   
   \ new statement
   bytes DUP INCREFERENCE
   0
   bytes CELL+ @ 28 + ( equation.JavaArray.length )    @
   1
   -
   String§1651856128.table -1626909952 EXECUTE-NEW
   TO §this
   
   \ new statement
   §this ( return object )    
   \ new statement
   0 §break82 LABEL
   
   \ new statement
   bytes TRUE JavaArray.kill§1620077312
   PURGE 2
   
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
   LOCAL value
   
   \ new statement
   0
   LOCALS §this |
   
   \ new statement
   Object§-1890496768.table 1827294976 EXECUTE-NEW
   
   \ new statement
   44 MALLOC DROP DUP TO §this OVER
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
   §this CELL+ @ 36 + ( equation.String.count )    @ 131073 JavaArray§1352878592.table -227194368 EXECUTE-NEW
   §this CELL+ @ 28 + ( equation.String.value )    DUP @ TRUE JavaArray.kill§1620077312 !
   
   \ new statement
   value DUP INCREFERENCE
   0
   §this CELL+ @ 28 + ( equation.String.value )    @ DUP INCREFERENCE
   0
   §this CELL+ @ 36 + ( equation.String.count )    @
   String.arraycopy§1134321920
   
   \ new statement
   §this ( return object )    
   \ new statement
   0 §break70 LABEL
   
   \ new statement
   value TRUE JavaArray.kill§1620077312
   PURGE 2
   
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
   LOCAL count
   LOCAL offset
   LOCAL value
   
   \ new statement
   0
   LOCALS §this |
   
   \ new statement
   Object§-1890496768.table 1827294976 EXECUTE-NEW
   
   \ new statement
   44 MALLOC DROP DUP TO §this OVER
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
   count 131073 JavaArray§1352878592.table -227194368 EXECUTE-NEW
   §this CELL+ @ 28 + ( equation.String.value )    DUP @ TRUE JavaArray.kill§1620077312 !
   
   \ new statement
   count
   §this CELL+ @ 36 + ( equation.String.count )    !
   
   \ new statement
   value DUP INCREFERENCE
   offset
   §this CELL+ @ 28 + ( equation.String.value )    @ DUP INCREFERENCE
   0
   count
   String.arraycopy§1134321920
   
   \ new statement
   §this ( return object )    
   \ new statement
   0 §break71 LABEL
   
   \ new statement
   
   
   value TRUE JavaArray.kill§1620077312
   PURGE 4
   
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
   LOCAL hibyte
   LOCAL ascii
   
   \ new statement
   0
   LOCALS §this |
   
   \ new statement
   ascii DUP INCREFERENCE
   hibyte
   0
   ascii CELL+ @ 28 + ( equation.JavaArray.length )    @
   String§1651856128.table -1936633088 EXECUTE-NEW
   TO §this
   
   \ new statement
   §this ( return object )    
   \ new statement
   0 §break79 LABEL
   
   \ new statement
   ascii TRUE JavaArray.kill§1620077312
   
   PURGE 3
   
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
   LOCAL value
   LOCAL count
   LOCAL offset
   
   \ new statement
   0
   LOCALS §this |
   
   \ new statement
   Object§-1890496768.table 1827294976 EXECUTE-NEW
   
   \ new statement
   44 MALLOC DROP DUP TO §this OVER
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
   value DUP INCREFERENCE
   §this CELL+ @ 28 + ( equation.String.value )    DUP @ TRUE JavaArray.kill§1620077312 !
   
   \ new statement
   offset
   §this CELL+ @ 32 + ( equation.String.offset )    !
   
   \ new statement
   count
   §this CELL+ @ 36 + ( equation.String.count )    !
   
   \ new statement
   §this ( return object )    
   \ new statement
   0 §break83 LABEL
   
   \ new statement
   
   
   value TRUE JavaArray.kill§1620077312
   PURGE 4
   
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
   LOCAL original
   
   \ new statement
   0
   LOCALS §this |
   
   \ new statement
   Object§-1890496768.table 1827294976 EXECUTE-NEW
   
   \ new statement
   44 MALLOC DROP DUP TO §this OVER
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
      §this CELL+ @ 36 + ( equation.String.count )       @ 131073 JavaArray§1352878592.table -227194368 EXECUTE-NEW
      §this CELL+ @ 28 + ( equation.String.value )       DUP @ TRUE JavaArray.kill§1620077312 !
      
      \ new statement
      original CELL+ @ 28 + ( equation.String.value )       @ DUP INCREFERENCE
      original CELL+ @ 32 + ( equation.String.offset )       @
      §this CELL+ @ 28 + ( equation.String.value )       @ DUP INCREFERENCE
      0
      §this CELL+ @ 36 + ( equation.String.count )       @
      String.arraycopy§1134321920
      
      \ new statement
      0 §break66 LABEL
      
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
      original CELL+ @ 28 + ( equation.String.value )       @ DUP INCREFERENCE
      §this CELL+ @ 28 + ( equation.String.value )       DUP @ TRUE JavaArray.kill§1620077312 !
      
      \ new statement
      0 §break67 LABEL
      
      \ new statement
      
      \ new statement
      DROP
      
      \ new statement
   ENDIF
   
   \ new statement
   §this ( return object )    
   \ new statement
   0 §break65 LABEL
   
   \ new statement
   original TRUE JavaArray.kill§1620077312
   PURGE 2
   
   \ new statement
   DROP
;


: String.arraycopy§1134321920
   LOCAL count
   LOCAL doffset
   LOCAL dvalue
   LOCAL soffset
   LOCAL svalue
   
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
      A:R@
      soffset
      R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
      TO source
      
      \ new statement
      dvalue
      A:R@
      doffset
      R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
      TO dest
      
      \ new statement
      source dest count 2* MOVE
      
      \ new statement
      0 §break69 LABEL
      
      \ new statement
      
      
      PURGE 2
      
      \ new statement
      DROP
      
      \ new statement
   ENDIF
   
   \ new statement
   0 §break68 LABEL
   
   \ new statement
   
   
   dvalue TRUE JavaArray.kill§1620077312
   
   svalue TRUE JavaArray.kill§1620077312
   PURGE 5
   
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
   LOCAL index
   LOCAL §this
   
   \ new statement
   
   \ new statement
   ( if ((index < 0) || (index >= count)) {
   throw new StringIndexOutOfBoundsException(index);
   } )
   
   \ new statement
   §this CELL+ @ 28 + ( equation.String.value ) @ A:R@
   index
   §this CELL+ @ 32 + ( equation.String.offset )    @
   +
   R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
   H@
   0 §break85 BRANCH
   
   \ new statement
   0 §break85 LABEL
   
   \ new statement
   
   PURGE 2
   
   \ new statement
   DROP
;


: String.classMonitorEnter§-433167616
   
   \ new statement
   
   \ new statement
   0 §break62 LABEL
   
   \ new statement
   
   \ new statement
   DROP
;


: String.classMonitorLeave§-1691393280
   
   \ new statement
   
   \ new statement
   0 §break63 LABEL
   
   \ new statement
   
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
   LOCAL o
   LOCAL §this
   
   \ new statement
   
   \ new statement
   §this A:R@
   o
   DUP INCREFERENCE
   R> 1883005696 TRUE ( equation.String.compareTo§1883005696 ) EXECUTE-METHOD
   0 §break108 BRANCH
   
   \ new statement
   0 §break108 LABEL
   
   \ new statement
   o TRUE JavaArray.kill§1620077312
   PURGE 2
   
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
   LOCAL anotherString
   LOCAL §this
   
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
   §this CELL+ @ 28 + ( equation.String.value )    @ DUP INCREFERENCE
   v1 TRUE JavaArray.kill§1620077312 TO v1
   
   \ new statement
   anotherString CELL+ @ 28 + ( equation.String.value )    @ DUP INCREFERENCE
   v2 TRUE JavaArray.kill§1620077312 TO v2
   
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
         v1 A:R@
         k
         R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
         H@
         TO c1
         
         \ new statement
         v2 A:R@
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
            196611 §break103 BRANCH
            
            \ new statement
            0 §break103 LABEL
            
            \ new statement
            
            \ new statement
            0>!
            IF
               10001H - 0<! §break101 0BRANCH
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
      0 §break101 LABEL
      
      \ new statement
      
      
      PURGE 2
      
      \ new statement
      0>!
      IF
         10001H - §break100 BRANCH
      ENDIF
      DROP
      
      \ new statement
      0 §break100 LABEL
      
      \ new statement
      
      
      PURGE 2
      
      \ new statement
      0>!
      IF
         10001H - §break99 BRANCH
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
         v1 A:R@
         i DUP 1+ TO i
         R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
         H@
         TO c1
         
         \ new statement
         v2 A:R@
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
            196611 §break107 BRANCH
            
            \ new statement
            0 §break107 LABEL
            
            \ new statement
            
            \ new statement
            0>!
            IF
               10001H - 0<! §break105 0BRANCH
            ENDIF
            DROP
            
            \ new statement
         ENDIF
         
         \ new statement
         
         \ new statement
      REPEAT
      
      \ new statement
      0 §break105 LABEL
      
      \ new statement
      
      
      PURGE 2
      
      \ new statement
      0>!
      IF
         10001H - §break104 BRANCH
      ENDIF
      DROP
      
      \ new statement
      0 §break104 LABEL
      
      \ new statement
      
      \ new statement
      0>!
      IF
         10001H - §break99 BRANCH
      ENDIF
      DROP
      
      \ new statement
   ENDIF
   
   \ new statement
   len1
   len2
   -
   0 §break99 BRANCH
   
   \ new statement
   0 §break99 LABEL
   
   \ new statement
   anotherString TRUE JavaArray.kill§1620077312
   
   
   
   
   
   v1 TRUE JavaArray.kill§1620077312
   v2 TRUE JavaArray.kill§1620077312
   PURGE 9
   
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
   LOCAL str
   LOCAL §this
   
   \ new statement
   
   \ new statement
   String.CASE_INSENSITIVE_ORDER  @ A:R@
   §this
   str DUP INCREFERENCE
   R> 2079286016 TRUE ( equation.Comparator.compare§2079286016 ) EXECUTE-METHOD
   0 §break121 BRANCH
   
   \ new statement
   0 §break121 LABEL
   
   \ new statement
   str TRUE JavaArray.kill§1620077312
   PURGE 2
   
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
   LOCAL str
   LOCAL §this
   
   \ new statement
   0 DUP
   LOCALS buf otherLen |
   
   \ new statement
   str A:R@
   R> 188050432 TRUE ( equation.String.length§188050432 ) EXECUTE-METHOD
   TO otherLen
   
   \ new statement
   otherLen
   0=
   
   \ new statement
   IF
      
      \ new statement
      
      \ new statement
      §this DUP INCREFERENCE
      65537 §break190 BRANCH
      
      \ new statement
      0 §break190 LABEL
      
      \ new statement
      
      \ new statement
      0>!
      IF
         10001H - §break189 BRANCH
      ENDIF
      DROP
      
      \ new statement
   ENDIF
   
   \ new statement
   §this CELL+ @ 36 + ( equation.String.count )    @
   otherLen
   + 131073 JavaArray§1352878592.table -227194368 EXECUTE-NEW
   buf TRUE JavaArray.kill§1620077312 TO buf
   
   \ new statement
   §this A:R@
   0
   §this CELL+ @ 36 + ( equation.String.count )    @
   buf DUP INCREFERENCE
   0
   R> -53713152 TRUE ( equation.String.getChars§-53713152 ) EXECUTE-METHOD
   
   \ new statement
   str A:R@
   0
   otherLen
   buf DUP INCREFERENCE
   §this CELL+ @ 36 + ( equation.String.count )    @
   R> -53713152 TRUE ( equation.String.getChars§-53713152 ) EXECUTE-METHOD
   
   \ new statement
   buf DUP INCREFERENCE
   0
   §this CELL+ @ 36 + ( equation.String.count )    @
   otherLen
   +
   String§1651856128.table 1127437056 EXECUTE-NEW
   0 §break189 BRANCH
   
   \ new statement
   0 §break189 LABEL
   
   \ new statement
   buf TRUE JavaArray.kill§1620077312
   
   str TRUE JavaArray.kill§1620077312
   PURGE 4
   
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
   LOCAL data
   
   \ new statement
   
   \ new statement
   data DUP INCREFERENCE
   0
   data CELL+ @ 28 + ( equation.JavaArray.length )    @
   String.copyValueOf§471425792
   0 §break228 BRANCH
   
   \ new statement
   0 §break228 LABEL
   
   \ new statement
   data TRUE JavaArray.kill§1620077312
   PURGE 1
   
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
   LOCAL count
   LOCAL offset
   LOCAL data
   
   \ new statement
   
   \ new statement
   \  All public String constructors now copy the data.
   
   
   \ new statement
   data DUP INCREFERENCE
   offset
   count
   String§1651856128.table 1127437056 EXECUTE-NEW
   0 §break227 BRANCH
   
   \ new statement
   0 §break227 LABEL
   
   \ new statement
   
   data TRUE JavaArray.kill§1620077312
   
   PURGE 3
   
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
   LOCAL suffix
   LOCAL §this
   
   \ new statement
   
   \ new statement
   §this A:R@
   suffix DUP INCREFERENCE
   §this CELL+ @ 36 + ( equation.String.count )    @
   suffix CELL+ @ 36 + ( equation.String.count )    @
   -
   R> -953322752 TRUE ( equation.String.startsWith§-953322752 ) EXECUTE-METHOD
   0 §break141 BRANCH
   
   \ new statement
   0 §break141 LABEL
   
   \ new statement
   suffix TRUE JavaArray.kill§1620077312
   PURGE 2
   
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
   LOCAL anObject
   LOCAL §this
   
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
      65537 §break93 BRANCH
      
      \ new statement
      0 §break93 LABEL
      
      \ new statement
      
      \ new statement
      0>!
      IF
         10001H - §break92 BRANCH
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
      0 DUP
      LOCALS anotherString n |
      
      \ new statement
      anObject
      DUP INCREFERENCE
      anotherString TRUE JavaArray.kill§1620077312 TO anotherString
      
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
         0 DUP DUP DUP
         LOCALS i j v1 v2 |
         
         \ new statement
         §this CELL+ @ 28 + ( equation.String.value )          @ DUP INCREFERENCE
         v1 TRUE JavaArray.kill§1620077312 TO v1
         
         \ new statement
         anotherString CELL+ @ 28 + ( equation.String.value )          @ DUP INCREFERENCE
         v2 TRUE JavaArray.kill§1620077312 TO v2
         
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
            v1 A:R@
            i DUP 1+ TO i
            R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
            H@
            v2 A:R@
            j DUP 1+ TO j
            R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
            H@
            <>
            
            \ new statement
            IF
               
               \ new statement
               FALSE
               196611 §break96 BRANCH
               
               \ new statement
            ENDIF
            
            \ new statement
            
            \ new statement
         REPEAT
         
         \ new statement
         0 §break96 LABEL
         
         \ new statement
         
         \ new statement
         0>!
         IF
            10001H - §break95 BRANCH
         ENDIF
         DROP
         
         \ new statement
         TRUE
         131074 §break95 BRANCH
         
         \ new statement
         0 §break95 LABEL
         
         \ new statement
         
         
         v1 TRUE JavaArray.kill§1620077312
         v2 TRUE JavaArray.kill§1620077312
         PURGE 4
         
         \ new statement
         0>!
         IF
            10001H - §break94 BRANCH
         ENDIF
         DROP
         
         \ new statement
      ENDIF
      
      \ new statement
      0 §break94 LABEL
      
      \ new statement
      anotherString TRUE JavaArray.kill§1620077312
      
      PURGE 2
      
      \ new statement
      0>!
      IF
         10001H - §break92 BRANCH
      ENDIF
      DROP
      
      \ new statement
   ENDIF
   
   \ new statement
   FALSE
   0 §break92 BRANCH
   
   \ new statement
   0 §break92 LABEL
   
   \ new statement
   anObject TRUE JavaArray.kill§1620077312
   PURGE 2
   
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
   LOCAL anotherString
   LOCAL §this
   
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
      and_838 0BRANCH! DROP
      anotherString CELL+ @ 36 + ( equation.String.count )       @
      §this CELL+ @ 36 + ( equation.String.count )       @
      =
      and_838 0BRANCH! DROP
      §this A:R@
      TRUE
      0
      anotherString DUP INCREFERENCE
      0
      §this CELL+ @ 36 + ( equation.String.count )       @
      R> 2060349952 TRUE ( equation.String.regionMatches§2060349952 ) EXECUTE-METHOD
      and_838 LABEL
      
   ENDIF
   0 §break98 BRANCH
   
   \ new statement
   0 §break98 LABEL
   
   \ new statement
   anotherString TRUE JavaArray.kill§1620077312
   PURGE 2
   
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
   LOCAL dstBegin
   LOCAL dst
   LOCAL srcEnd
   LOCAL srcBegin
   LOCAL §this
   
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
   §this CELL+ @ 28 + ( equation.String.value )    @ DUP INCREFERENCE
   val TRUE JavaArray.kill§1620077312 TO val
   
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
      val A:R@
      i DUP 1+ TO i
      R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
      H@ A:R@  127 U>! NIP
      IF
         0FFFFFF00H OR
      ENDIF
      A:DROP
      dst A:R@
      0§
      R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
      C!
      
      PURGE 1
      
      
      \ new statement
      
      \ new statement
   REPEAT
   
   \ new statement
   0 §break88 LABEL
   
   \ new statement
   
   \ new statement
   DROP
   
   \ new statement
   0 §break87 LABEL
   
   \ new statement
   dst TRUE JavaArray.kill§1620077312
   
   
   
   
   
   
   val TRUE JavaArray.kill§1620077312
   PURGE 9
   
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
   LOCAL §this
   
   \ new statement
   0
   LOCALS b |
   
   \ new statement
   §this CELL+ @ 36 + ( equation.String.count )    @ 65537 JavaArray§1352878592.table -227194368 EXECUTE-NEW
   b TRUE JavaArray.kill§1620077312 TO b
   
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
      §this CELL+ @ 28 + ( equation.String.value ) @ A:R@
      i
      R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
      H@ A:R@  127 U>! NIP
      IF
         0FFFFFF00H OR
      ENDIF
      A:DROP
      b A:R@
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
   0 §break91 LABEL
   
   \ new statement
   
   PURGE 1
   
   \ new statement
   DROP
   
   \ new statement
   b DUP INCREFERENCE
   0 §break90 BRANCH
   
   \ new statement
   0 §break90 LABEL
   
   \ new statement
   b TRUE JavaArray.kill§1620077312
   PURGE 2
   
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
   LOCAL dstBegin
   LOCAL dst
   LOCAL srcEnd
   LOCAL srcBegin
   LOCAL §this
   
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
   §this CELL+ @ 28 + ( equation.String.value )    @ DUP INCREFERENCE
   §this CELL+ @ 32 + ( equation.String.offset )    @
   srcBegin
   +
   dst DUP INCREFERENCE
   dstBegin
   srcEnd
   srcBegin
   -
   String.arraycopy§1134321920
   
   \ new statement
   0 §break86 LABEL
   
   \ new statement
   dst TRUE JavaArray.kill§1620077312
   
   
   
   PURGE 5
   
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
   LOCAL §this
   
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
      0 DUP DUP
      LOCALS len off val |
      
      \ new statement
      §this CELL+ @ 32 + ( equation.String.offset )       @
      TO off
      
      \ new statement
      §this CELL+ @ 28 + ( equation.String.value )       @ DUP INCREFERENCE
      val TRUE JavaArray.kill§1620077312 TO val
      
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
         val A:R@
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
      0 §break144 LABEL
      
      \ new statement
      
      PURGE 1
      
      \ new statement
      DROP
      
      \ new statement
      h
      §this CELL+ @ 40 + ( equation.String.hash )       !
      
      \ new statement
      0 §break143 LABEL
      
      \ new statement
      
      
      val TRUE JavaArray.kill§1620077312
      PURGE 3
      
      \ new statement
      DROP
      
      \ new statement
   ENDIF
   
   \ new statement
   h
   0 §break142 BRANCH
   
   \ new statement
   0 §break142 LABEL
   
   \ new statement
   
   PURGE 2
   
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
   LOCAL str
   LOCAL §this
   
   \ new statement
   
   \ new statement
   §this A:R@
   str DUP INCREFERENCE
   0
   R> 517761280 TRUE ( equation.String.indexOf§517761280 ) EXECUTE-METHOD
   0 §break158 BRANCH
   
   \ new statement
   0 §break158 LABEL
   
   \ new statement
   str TRUE JavaArray.kill§1620077312
   PURGE 2
   
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
   LOCAL fromIndex
   LOCAL targetCount
   LOCAL targetOffset
   LOCAL target
   LOCAL sourceCount
   LOCAL sourceOffset
   LOCAL source
   
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
      65537 §break161 BRANCH
      
      \ new statement
      0 §break161 LABEL
      
      \ new statement
      
      \ new statement
      0>!
      IF
         10001H - §break160 BRANCH
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
      0 §break162 LABEL
      
      \ new statement
      
      \ new statement
      0>!
      IF
         10001H - §break160 BRANCH
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
      65537 §break163 BRANCH
      
      \ new statement
      0 §break163 LABEL
      
      \ new statement
      
      \ new statement
      0>!
      IF
         10001H - §break160 BRANCH
      ENDIF
      DROP
      
      \ new statement
   ENDIF
   
   \ new statement
   target A:R@
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
         > INVERT and_954 0BRANCH! DROP
         source A:R@
         i
         R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
         H@
         first
         <>
         and_954 LABEL
         
         \ new statement
      WHILE
         
         \ new statement
         i 1+ TO i
         
         \ new statement
         
         \ new statement
      REPEAT
      
      \ new statement
      0 §break166 LABEL
      
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
         131074 §break168 BRANCH
         
         \ new statement
         0 §break168 LABEL
         
         \ new statement
         
         \ new statement
         0>!
         IF
            10001H - 0<! §break164 0BRANCH
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
         source A:R@
         j DUP 1+ TO j
         R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
         H@
         target A:R@
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
            65538 §break171 BRANCH
            
            \ new statement
            0 §break171 LABEL
            
            \ new statement
            
            \ new statement
            0>!
            IF
               10001H - 0<! §break169 0BRANCH
            ENDIF
            DROP
            
            \ new statement
         ENDIF
         
         \ new statement
         
         \ new statement
      REPEAT
      
      \ new statement
      0 §break169 LABEL
      
      \ new statement
      
      \ new statement
      0>!
      IF
         10001H - 0<! §break164 0BRANCH
      ENDIF
      DROP
      
      \ new statement
      i
      sourceOffset
      -
      65537 §break164 BRANCH
      
      \ new statement
      (  Found whole string.  )
      
      \ new statement
      
      \ new statement
   AGAIN
   
   \ new statement
   0 §break164 LABEL
   
   \ new statement
   
   
   
   PURGE 3
   
   \ new statement
   0>!
   IF
      10001H - §break160 BRANCH
   ENDIF
   DROP
   
   \ new statement
   0 DROP
   
   \ new statement
   0 §break160 LABEL
   
   \ new statement
   
   
   
   
   source TRUE JavaArray.kill§1620077312
   
   
   target TRUE JavaArray.kill§1620077312
   
   
   PURGE 10
   
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
   LOCAL fromIndex
   LOCAL str
   LOCAL §this
   
   \ new statement
   
   \ new statement
   §this CELL+ @ 28 + ( equation.String.value )    @ DUP INCREFERENCE
   §this CELL+ @ 32 + ( equation.String.offset )    @
   §this CELL+ @ 36 + ( equation.String.count )    @
   str CELL+ @ 28 + ( equation.String.value )    @ DUP INCREFERENCE
   str CELL+ @ 32 + ( equation.String.offset )    @
   str CELL+ @ 36 + ( equation.String.count )    @
   fromIndex
   String.indexOf§-412522240
   0 §break159 BRANCH
   
   \ new statement
   0 §break159 LABEL
   
   \ new statement
   
   str TRUE JavaArray.kill§1620077312
   PURGE 3
   
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
   LOCAL fromIndex
   LOCAL ch
   LOCAL §this
   
   \ new statement
   0 DUP
   LOCALS max v |
   
   \ new statement
   §this CELL+ @ 32 + ( equation.String.offset )    @
   §this CELL+ @ 36 + ( equation.String.count )    @
   +
   TO max
   
   \ new statement
   §this CELL+ @ 28 + ( equation.String.value )    @ DUP INCREFERENCE
   v TRUE JavaArray.kill§1620077312 TO v
   
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
      0 §break148 LABEL
      
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
         65537 §break149 BRANCH
         
         \ new statement
         0 §break149 LABEL
         
         \ new statement
         
         \ new statement
         0>!
         IF
            10001H - §break147 BRANCH
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
      v A:R@
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
         131074 §break152 BRANCH
         
         \ new statement
         0 §break152 LABEL
         
         \ new statement
         
         \ new statement
         0>!
         IF
            10001H - 0<! §break150 0BRANCH
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
   0 §break150 LABEL
   
   \ new statement
   
   PURGE 1
   
   \ new statement
   0>!
   IF
      10001H - §break147 BRANCH
   ENDIF
   DROP
   
   \ new statement
   -1
   0 §break147 BRANCH
   
   \ new statement
   0 §break147 LABEL
   
   \ new statement
   
   
   
   v TRUE JavaArray.kill§1620077312
   PURGE 5
   
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
   LOCAL ch
   LOCAL §this
   
   \ new statement
   
   \ new statement
   §this A:R@
   ch
   0
   R> 731343104 TRUE ( equation.String.indexOf§731343104 ) EXECUTE-METHOD
   0 §break146 BRANCH
   
   \ new statement
   0 §break146 LABEL
   
   \ new statement
   
   PURGE 2
   
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
   LOCAL str
   LOCAL §this
   
   \ new statement
   
   \ new statement
   §this A:R@
   str DUP INCREFERENCE
   §this CELL+ @ 36 + ( equation.String.count )    @
   R> -465605632 TRUE ( equation.String.lastIndexOf§-465605632 ) EXECUTE-METHOD
   0 §break172 BRANCH
   
   \ new statement
   0 §break172 LABEL
   
   \ new statement
   str TRUE JavaArray.kill§1620077312
   PURGE 2
   
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
   LOCAL fromIndex
   LOCAL ch
   LOCAL §this
   
   \ new statement
   0 DUP
   LOCALS min v |
   
   \ new statement
   §this CELL+ @ 32 + ( equation.String.offset )    @
   TO min
   
   \ new statement
   §this CELL+ @ 28 + ( equation.String.value )    @ DUP INCREFERENCE
   v TRUE JavaArray.kill§1620077312 TO v
   
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
      v A:R@
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
         131074 §break157 BRANCH
         
         \ new statement
         0 §break157 LABEL
         
         \ new statement
         
         \ new statement
         0>!
         IF
            10001H - 0<! §break155 0BRANCH
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
   0 §break155 LABEL
   
   \ new statement
   
   PURGE 1
   
   \ new statement
   0>!
   IF
      10001H - §break154 BRANCH
   ENDIF
   DROP
   
   \ new statement
   -1
   0 §break154 BRANCH
   
   \ new statement
   0 §break154 LABEL
   
   \ new statement
   
   
   
   v TRUE JavaArray.kill§1620077312
   PURGE 5
   
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
   LOCAL fromIndex
   LOCAL str
   LOCAL §this
   
   \ new statement
   
   \ new statement
   §this CELL+ @ 28 + ( equation.String.value )    @ DUP INCREFERENCE
   §this CELL+ @ 32 + ( equation.String.offset )    @
   §this CELL+ @ 36 + ( equation.String.count )    @
   str CELL+ @ 28 + ( equation.String.value )    @ DUP INCREFERENCE
   str CELL+ @ 32 + ( equation.String.offset )    @
   str CELL+ @ 36 + ( equation.String.count )    @
   fromIndex
   String.lastIndexOf§831351808
   0 §break173 BRANCH
   
   \ new statement
   0 §break173 LABEL
   
   \ new statement
   
   str TRUE JavaArray.kill§1620077312
   PURGE 3
   
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
   LOCAL ch
   LOCAL §this
   
   \ new statement
   
   \ new statement
   §this A:R@
   ch
   §this CELL+ @ 36 + ( equation.String.count )    @
   1
   -
   R> -1735627776 TRUE ( equation.String.lastIndexOf§-1735627776 ) EXECUTE-METHOD
   0 §break153 BRANCH
   
   \ new statement
   0 §break153 LABEL
   
   \ new statement
   
   PURGE 2
   
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
   LOCAL fromIndex
   LOCAL targetCount
   LOCAL targetOffset
   LOCAL target
   LOCAL sourceCount
   LOCAL sourceOffset
   LOCAL source
   
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
      65537 §break175 BRANCH
      
      \ new statement
      0 §break175 LABEL
      
      \ new statement
      
      \ new statement
      0>!
      IF
         10001H - §break174 BRANCH
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
      0 §break176 LABEL
      
      \ new statement
      
      \ new statement
      0>!
      IF
         10001H - §break174 BRANCH
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
      65537 §break177 BRANCH
      
      \ new statement
      0 §break177 LABEL
      
      \ new statement
      
      \ new statement
      0>!
      IF
         10001H - §break174 BRANCH
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
   target A:R@
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
         < INVERT and_1136 0BRANCH! DROP
         source A:R@
         i
         R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
         H@
         strLastChar
         <>
         and_1136 LABEL
         
         \ new statement
      WHILE
         
         \ new statement
         i 1- TO i
         
         \ new statement
         
         \ new statement
      REPEAT
      
      \ new statement
      0 §break180 LABEL
      
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
         131074 §break182 BRANCH
         
         \ new statement
         0 §break182 LABEL
         
         \ new statement
         
         \ new statement
         0>!
         IF
            10001H - 0<! §break178 0BRANCH
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
         source A:R@
         j DUP 1- TO j
         R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
         H@
         target A:R@
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
            65538 §break185 BRANCH
            
            \ new statement
            0 §break185 LABEL
            
            \ new statement
            
            \ new statement
            0>!
            IF
               10001H - 0<! §break183 0BRANCH
            ENDIF
            DROP
            
            \ new statement
         ENDIF
         
         \ new statement
         
         \ new statement
      REPEAT
      
      \ new statement
      0 §break183 LABEL
      
      \ new statement
      
      \ new statement
      0>!
      IF
         10001H - 0<! §break178 0BRANCH
      ENDIF
      DROP
      
      \ new statement
      start
      sourceOffset
      -
      1
      +
      65537 §break178 BRANCH
      
      \ new statement
      
      \ new statement
   AGAIN
   
   \ new statement
   0 §break178 LABEL
   
   \ new statement
   
   
   
   PURGE 3
   
   \ new statement
   0>!
   IF
      10001H - §break174 BRANCH
   ENDIF
   DROP
   
   \ new statement
   0 DROP
   
   \ new statement
   0 §break174 LABEL
   
   \ new statement
   
   
   
   
   source TRUE JavaArray.kill§1620077312
   
   
   
   
   target TRUE JavaArray.kill§1620077312
   
   
   PURGE 12
   
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
   LOCAL §this
   
   \ new statement
   
   \ new statement
   §this CELL+ @ 36 + ( equation.String.count )    @
   0 §break84 BRANCH
   
   \ new statement
   0 §break84 LABEL
   
   \ new statement
   PURGE 1
   
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
   LOCAL len
   LOCAL ooffset
   LOCAL other
   LOCAL toffset
   LOCAL §this
   
   \ new statement
   0 DUP DUP DUP
   LOCALS pa po ta to |
   
   \ new statement
   §this CELL+ @ 28 + ( equation.String.value )    @ DUP INCREFERENCE
   ta TRUE JavaArray.kill§1620077312 TO ta
   
   \ new statement
   §this CELL+ @ 32 + ( equation.String.offset )    @
   toffset
   +
   TO to
   
   \ new statement
   other CELL+ @ 28 + ( equation.String.value )    @ DUP INCREFERENCE
   pa TRUE JavaArray.kill§1620077312 TO pa
   
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
   0=! or_1195 0BRANCH DROP
   toffset
   0<
   0=! or_1195 0BRANCH DROP
   toffset  S>D
   §this CELL+ @ 36 + ( equation.String.count )    @ A:R@  S>D
   A:DROP
   len  S>D
   D-
   D>
   0=! or_1195 0BRANCH DROP
   ooffset  S>D
   other CELL+ @ 36 + ( equation.String.count )    @ A:R@  S>D
   A:DROP
   len  S>D
   D-
   D>
   or_1195 LABEL
   
   \ new statement
   IF
      
      \ new statement
      
      \ new statement
      FALSE
      65537 §break123 BRANCH
      
      \ new statement
      0 §break123 LABEL
      
      \ new statement
      
      \ new statement
      0>!
      IF
         10001H - §break122 BRANCH
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
      ta A:R@
      to DUP 1+ TO to
      R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
      H@
      pa A:R@
      po DUP 1+ TO po
      R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
      H@
      <>
      
      \ new statement
      IF
         
         \ new statement
         
         \ new statement
         FALSE
         131074 §break126 BRANCH
         
         \ new statement
         0 §break126 LABEL
         
         \ new statement
         
         \ new statement
         0>!
         IF
            10001H - 0<! §break124 0BRANCH
         ENDIF
         DROP
         
         \ new statement
      ENDIF
      
      \ new statement
      
      \ new statement
   REPEAT
   
   \ new statement
   0 §break124 LABEL
   
   \ new statement
   
   \ new statement
   0>!
   IF
      10001H - §break122 BRANCH
   ENDIF
   DROP
   
   \ new statement
   TRUE
   0 §break122 BRANCH
   
   \ new statement
   0 §break122 LABEL
   
   \ new statement
   
   
   other TRUE JavaArray.kill§1620077312
   pa TRUE JavaArray.kill§1620077312
   
   ta TRUE JavaArray.kill§1620077312
   
   
   PURGE 9
   
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
   LOCAL len
   LOCAL ooffset
   LOCAL other
   LOCAL toffset
   LOCAL ignoreCase
   LOCAL §this
   
   \ new statement
   0 DUP DUP DUP
   LOCALS pa po ta to |
   
   \ new statement
   §this CELL+ @ 28 + ( equation.String.value )    @ DUP INCREFERENCE
   ta TRUE JavaArray.kill§1620077312 TO ta
   
   \ new statement
   §this CELL+ @ 32 + ( equation.String.offset )    @
   toffset
   +
   TO to
   
   \ new statement
   other CELL+ @ 28 + ( equation.String.value )    @ DUP INCREFERENCE
   pa TRUE JavaArray.kill§1620077312 TO pa
   
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
   0=! or_1231 0BRANCH DROP
   toffset
   0<
   0=! or_1231 0BRANCH DROP
   toffset  S>D
   §this CELL+ @ 36 + ( equation.String.count )    @ A:R@  S>D
   A:DROP
   len  S>D
   D-
   D>
   0=! or_1231 0BRANCH DROP
   ooffset  S>D
   other CELL+ @ 36 + ( equation.String.count )    @ A:R@  S>D
   A:DROP
   len  S>D
   D-
   D>
   or_1231 LABEL
   
   \ new statement
   IF
      
      \ new statement
      
      \ new statement
      FALSE
      65537 §break128 BRANCH
      
      \ new statement
      0 §break128 LABEL
      
      \ new statement
      
      \ new statement
      0>!
      IF
         10001H - §break127 BRANCH
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
      ta A:R@
      to DUP 1+ TO to
      R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
      H@
      TO c1
      
      \ new statement
      pa A:R@
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
         1 §break131 BRANCH
         
         \ new statement
         0 §break131 LABEL
         
         \ new statement
         
         \ new statement
         0>!
         IF
            10001H - 0<! §break129 0BRANCH §continue129 BRANCH
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
            65538 §break133 BRANCH
            
            \ new statement
            0 §break133 LABEL
            
            \ new statement
            
            \ new statement
            0>!
            IF
               10001H - §break132 BRANCH
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
            65538 §break134 BRANCH
            
            \ new statement
            0 §break134 LABEL
            
            \ new statement
            
            \ new statement
            0>!
            IF
               10001H - §break132 BRANCH
            ENDIF
            DROP
            
            \ new statement
         ENDIF
         
         \ new statement
         0 §break132 LABEL
         
         \ new statement
         
         
         PURGE 2
         
         \ new statement
         0>!
         IF
            10001H - 0<! §break129 0BRANCH §continue129 BRANCH
         ENDIF
         DROP
         
         \ new statement
      ENDIF
      
      \ new statement
      FALSE
      65537 §break129 BRANCH
      
      \ new statement
      DUP §continue129 LABEL DROP
      
      \ new statement
   REPEAT
   
   \ new statement
   0 §break129 LABEL
   
   \ new statement
   
   
   PURGE 2
   
   \ new statement
   0>!
   IF
      10001H - §break127 BRANCH
   ENDIF
   DROP
   
   \ new statement
   TRUE
   0 §break127 BRANCH
   
   \ new statement
   0 §break127 LABEL
   
   \ new statement
   
   
   
   other TRUE JavaArray.kill§1620077312
   pa TRUE JavaArray.kill§1620077312
   
   ta TRUE JavaArray.kill§1620077312
   
   
   PURGE 10
   
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
   LOCAL newChar
   LOCAL oldChar
   LOCAL §this
   
   \ new statement
   
   \ new statement
   oldChar
   newChar
   <>
   
   \ new statement
   IF
      
      \ new statement
      0 DUP DUP DUP
      LOCALS i len off val |
      
      \ new statement
      §this CELL+ @ 36 + ( equation.String.count )       @
      TO len
      
      \ new statement
      -1
      TO i
      
      \ new statement
      §this CELL+ @ 28 + ( equation.String.value )       @ DUP INCREFERENCE
      val TRUE JavaArray.kill§1620077312 TO val
      
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
         val A:R@
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
            65537 §break195 BRANCH
            
            \ new statement
            0 §break195 LABEL
            
            \ new statement
            
            \ new statement
            0>!
            IF
               10001H - 0<! §break193 0BRANCH
            ENDIF
            DROP
            
            \ new statement
         ENDIF
         
         \ new statement
         
         \ new statement
      REPEAT
      
      \ new statement
      0 §break193 LABEL
      
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
         0
         LOCALS buf |
         
         \ new statement
         len 131073 JavaArray§1352878592.table -227194368 EXECUTE-NEW
         buf TRUE JavaArray.kill§1620077312 TO buf
         
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
            val A:R@
            off
            j
            +
            R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
            H@
            buf A:R@
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
         0 §break197 LABEL
         
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
            val A:R@
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
            buf A:R@
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
         0 §break199 LABEL
         
         \ new statement
         
         PURGE 1
         
         \ new statement
         DROP
         
         \ new statement
         buf DUP INCREFERENCE
         0
         len
         String§1651856128.table 1127437056 EXECUTE-NEW
         131074 §break196 BRANCH
         
         \ new statement
         0 §break196 LABEL
         
         \ new statement
         buf TRUE JavaArray.kill§1620077312
         PURGE 1
         
         \ new statement
         0>!
         IF
            10001H - §break192 BRANCH
         ENDIF
         DROP
         
         \ new statement
      ENDIF
      
      \ new statement
      0 §break192 LABEL
      
      \ new statement
      
      
      
      val TRUE JavaArray.kill§1620077312
      PURGE 4
      
      \ new statement
      0>!
      IF
         10001H - §break191 BRANCH
      ENDIF
      DROP
      
      \ new statement
   ENDIF
   
   \ new statement
   §this DUP INCREFERENCE
   0 §break191 BRANCH
   
   \ new statement
   0 §break191 LABEL
   
   \ new statement
   
   
   PURGE 3
   
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
   LOCAL toffset
   LOCAL prefix
   LOCAL §this
   
   \ new statement
   0 DUP DUP DUP DUP
   LOCALS pa pc po ta to |
   
   \ new statement
   §this CELL+ @ 28 + ( equation.String.value )    @ DUP INCREFERENCE
   ta TRUE JavaArray.kill§1620077312 TO ta
   
   \ new statement
   §this CELL+ @ 32 + ( equation.String.offset )    @
   toffset
   +
   TO to
   
   \ new statement
   prefix CELL+ @ 28 + ( equation.String.value )    @ DUP INCREFERENCE
   pa TRUE JavaArray.kill§1620077312 TO pa
   
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
   0=! or_1366 0BRANCH DROP
   toffset
   §this CELL+ @ 36 + ( equation.String.count )    @
   pc
   -
   >
   or_1366 LABEL
   
   \ new statement
   IF
      
      \ new statement
      
      \ new statement
      FALSE
      65537 §break136 BRANCH
      
      \ new statement
      0 §break136 LABEL
      
      \ new statement
      
      \ new statement
      0>!
      IF
         10001H - §break135 BRANCH
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
      ta A:R@
      to DUP 1+ TO to
      R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
      H@
      pa A:R@
      po DUP 1+ TO po
      R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
      H@
      <>
      
      \ new statement
      IF
         
         \ new statement
         
         \ new statement
         FALSE
         131074 §break139 BRANCH
         
         \ new statement
         0 §break139 LABEL
         
         \ new statement
         
         \ new statement
         0>!
         IF
            10001H - 0<! §break137 0BRANCH
         ENDIF
         DROP
         
         \ new statement
      ENDIF
      
      \ new statement
      
      \ new statement
   REPEAT
   
   \ new statement
   0 §break137 LABEL
   
   \ new statement
   
   \ new statement
   0>!
   IF
      10001H - §break135 BRANCH
   ENDIF
   DROP
   
   \ new statement
   TRUE
   0 §break135 BRANCH
   
   \ new statement
   0 §break135 LABEL
   
   \ new statement
   pa TRUE JavaArray.kill§1620077312
   
   
   prefix TRUE JavaArray.kill§1620077312
   ta TRUE JavaArray.kill§1620077312
   
   
   PURGE 8
   
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
   LOCAL prefix
   LOCAL §this
   
   \ new statement
   
   \ new statement
   §this A:R@
   prefix DUP INCREFERENCE
   0
   R> -953322752 TRUE ( equation.String.startsWith§-953322752 ) EXECUTE-METHOD
   0 §break140 BRANCH
   
   \ new statement
   0 §break140 LABEL
   
   \ new statement
   prefix TRUE JavaArray.kill§1620077312
   PURGE 2
   
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
   LOCAL endIndex
   LOCAL beginIndex
   LOCAL §this
   
   \ new statement
   
   \ new statement
   §this A:R@
   beginIndex
   endIndex
   R> -424054016 TRUE ( equation.String.substring§-424054016 ) EXECUTE-METHOD
   DUP INCREFERENCE
   0 §break188 BRANCH
   
   \ new statement
   0 §break188 LABEL
   
   \ new statement
   
   
   PURGE 3
   
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
   LOCAL endIndex
   LOCAL beginIndex
   LOCAL §this
   
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
   and_1408 0BRANCH! DROP
   endIndex
   §this CELL+ @ 36 + ( equation.String.count )    @
   =
   and_1408 LABEL
   
   IF
      §this DUP INCREFERENCE
   ELSE
      §this CELL+ @ 28 + ( equation.String.value )       @ DUP INCREFERENCE
      §this CELL+ @ 32 + ( equation.String.offset )       @
      beginIndex
      +
      endIndex
      beginIndex
      -
      String§1651856128.table 1127437056 EXECUTE-NEW
      
   ENDIF
   0 §break187 BRANCH
   
   \ new statement
   0 §break187 LABEL
   
   \ new statement
   
   
   PURGE 3
   
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
   LOCAL beginIndex
   LOCAL §this
   
   \ new statement
   
   \ new statement
   §this A:R@
   beginIndex
   §this CELL+ @ 36 + ( equation.String.count )    @
   R> -424054016 TRUE ( equation.String.substring§-424054016 ) EXECUTE-METHOD
   0 §break186 BRANCH
   
   \ new statement
   0 §break186 LABEL
   
   \ new statement
   
   PURGE 2
   
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
   LOCAL §this
   
   \ new statement
   0
   LOCALS result |
   
   \ new statement
   §this CELL+ @ 36 + ( equation.String.count )    @ 131073 JavaArray§1352878592.table -227194368 EXECUTE-NEW
   result TRUE JavaArray.kill§1620077312 TO result
   
   \ new statement
   §this A:R@
   0
   §this CELL+ @ 36 + ( equation.String.count )    @
   result DUP INCREFERENCE
   0
   R> -53713152 TRUE ( equation.String.getChars§-53713152 ) EXECUTE-METHOD
   
   \ new statement
   result DUP INCREFERENCE
   0 §break223 BRANCH
   
   \ new statement
   0 §break223 LABEL
   
   \ new statement
   result TRUE JavaArray.kill§1620077312
   PURGE 2
   
   \ new statement
   DROP
;


: String.toLowerCase§-1264749568
   LOCAL a
   
   \ new statement
   
   \ new statement
   a
   65
   < INVERT and_1426 0BRANCH! DROP
   a
   90
   > INVERT
   and_1426 LABEL
   
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
   0 §break120 BRANCH
   
   \ new statement
   0 §break120 LABEL
   
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
   LOCAL §this
   
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
   §this CELL+ @ 28 + ( equation.String.value )    @ DUP INCREFERENCE
   val TRUE JavaArray.kill§1620077312 TO val
   
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
      §this CELL+ @ 28 + ( equation.String.value ) @ A:R@
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
         131074 §break205 BRANCH
         
         \ new statement
         0 §break205 LABEL
         
         \ new statement
         
         \ new statement
         0>!
         IF
            10001H - 0<! §break203 0BRANCH
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
   0 §break203 LABEL
   
   \ new statement
   
   PURGE 1
   
   \ new statement
   0>!
   IF
      10001H - §break202 BRANCH
   ENDIF
   DROP
   
   \ new statement
   §this DUP INCREFERENCE
   65537 §break202 BRANCH
   
   \ new statement
   0 §break202 LABEL
   
   \ new statement
   
   \ new statement
   0>!
   IF
      10001H - §break201 BRANCH
   ENDIF
   DROP
   
   \ new statement
   0 DROP
   
   \ new statement
   §this CELL+ @ 36 + ( equation.String.count )    @ 131073 JavaArray§1352878592.table -227194368 EXECUTE-NEW
   result TRUE JavaArray.kill§1620077312 TO result
   
   \ new statement
   (  Just copy the first few lowerCase characters.  )
   
   \ new statement
   val DUP INCREFERENCE
   off
   result DUP INCREFERENCE
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
      val A:R@
      off
      i
      +
      R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
      H@
      String.toLowerCase§-1264749568
      result A:R@
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
   0 §break206 LABEL
   
   \ new statement
   
   PURGE 1
   
   \ new statement
   0>!
   IF
      10001H - §break201 BRANCH
   ENDIF
   DROP
   
   \ new statement
   result DUP INCREFERENCE
   0
   result CELL+ @ 28 + ( equation.JavaArray.length )    @
   String§1651856128.table 1127437056 EXECUTE-NEW
   0 §break201 BRANCH
   
   \ new statement
   0 §break201 LABEL
   
   \ new statement
   
   
   
   result TRUE JavaArray.kill§1620077312
   val TRUE JavaArray.kill§1620077312
   PURGE 6
   
   \ new statement
   DROP
;
( *
* This object (which is already a string!) is itself returned.
*
* @return  the string itself.
)

:LOCAL String.toString§1621718016
   LOCAL §this
   
   \ new statement
   
   \ new statement
   §this DUP INCREFERENCE
   0 §break222 BRANCH
   
   \ new statement
   0 §break222 LABEL
   
   \ new statement
   PURGE 1
   
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
   LOCAL §this
   
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
   §this CELL+ @ 28 + ( equation.String.value )    @ DUP INCREFERENCE
   val TRUE JavaArray.kill§1620077312 TO val
   
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
      §this CELL+ @ 28 + ( equation.String.value ) @ A:R@
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
         131074 §break212 BRANCH
         
         \ new statement
         0 §break212 LABEL
         
         \ new statement
         
         \ new statement
         0>!
         IF
            10001H - 0<! §break210 0BRANCH
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
   0 §break210 LABEL
   
   \ new statement
   
   \ new statement
   0>!
   IF
      10001H - §break209 BRANCH
   ENDIF
   DROP
   
   \ new statement
   §this DUP INCREFERENCE
   65537 §break209 BRANCH
   
   \ new statement
   0 §break209 LABEL
   
   \ new statement
   
   
   PURGE 2
   
   \ new statement
   0>!
   IF
      10001H - §break208 BRANCH
   ENDIF
   DROP
   
   \ new statement
   0 DROP
   
   \ new statement
   len 131073 JavaArray§1352878592.table -227194368 EXECUTE-NEW
   result TRUE JavaArray.kill§1620077312 TO result
   
   \ new statement
   (  might grow!  )
   
   \ new statement
   0
   TO resultOffset
   
   \ new statement
   (  result grows, so i+resultOffset
   * is the write location in result  )  (  Just copy the first few upperCase characters.  )
   
   \ new statement
   val DUP INCREFERENCE
   off
   result DUP INCREFERENCE
   0
   firstLower
   String.arraycopy§1134321920
   
   \ new statement
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
      val A:R@
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
      result A:R@
      0§
      R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
      H!
      
      PURGE 1
      
      
      \ new statement
      0 §break216 LABEL
      
      \ new statement
      
      \ new statement
      DROP
      
      \ new statement
      
      \ new statement
      i 1+ TO i
      
      \ new statement
   REPEAT
   
   \ new statement
   0 §break214 LABEL
   
   \ new statement
   
   PURGE 1
   
   \ new statement
   DROP
   
   \ new statement
   0 §break213 LABEL
   
   \ new statement
   
   
   upperCharArray TRUE JavaArray.kill§1620077312
   PURGE 3
   
   \ new statement
   0>!
   IF
      10001H - §break208 BRANCH
   ENDIF
   DROP
   
   \ new statement
   result DUP INCREFERENCE
   0
   result CELL+ @ 28 + ( equation.JavaArray.length )    @
   String§1651856128.table 1127437056 EXECUTE-NEW
   0 §break208 BRANCH
   
   \ new statement
   0 §break208 LABEL
   
   \ new statement
   
   
   
   result TRUE JavaArray.kill§1620077312
   
   val TRUE JavaArray.kill§1620077312
   PURGE 7
   
   \ new statement
   DROP
;


: String.toUpperCase§1730376704
   LOCAL a
   
   \ new statement
   
   \ new statement
   a
   97
   < INVERT and_1556 0BRANCH! DROP
   a
   122
   > INVERT
   and_1556 LABEL
   
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
   0 §break119 BRANCH
   
   \ new statement
   0 §break119 LABEL
   
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
   LOCAL §this
   
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
   §this CELL+ @ 28 + ( equation.String.value )    @ DUP INCREFERENCE
   val TRUE JavaArray.kill§1620077312 TO val
   
   \ new statement
   (  avoid getfield opcode  )
   
   \ new statement
   
   \ new statement
   BEGIN
      
      \ new statement
      st
      len
      <
      and_1574 0BRANCH! DROP
      val A:R@
      off
      st
      +
      R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
      H@
      32
      > INVERT
      and_1574 LABEL
      
      \ new statement
   WHILE
      
      \ new statement
      st 1+ TO st
      
      \ new statement
      
      \ new statement
   REPEAT
   
   \ new statement
   0 §break218 LABEL
   
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
      and_1585 0BRANCH! DROP
      val A:R@
      off
      len
      +
      1
      -
      R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
      H@
      32
      > INVERT
      and_1585 LABEL
      
      \ new statement
   WHILE
      
      \ new statement
      len 1- TO len
      
      \ new statement
      
      \ new statement
   REPEAT
   
   \ new statement
   0 §break220 LABEL
   
   \ new statement
   
   \ new statement
   DROP
   
   \ new statement
   st
   0>
   0=! or_1594 0BRANCH DROP
   len
   §this CELL+ @ 36 + ( equation.String.count )    @
   <
   or_1594 LABEL
   
   IF
      §this A:R@
      st
      len
      R> -424054016 TRUE ( equation.String.substring§-424054016 ) EXECUTE-METHOD
      
   ELSE
      §this
      DUP INCREFERENCE
   ENDIF
   0 §break217 BRANCH
   
   \ new statement
   0 §break217 LABEL
   
   \ new statement
   
   
   
   val TRUE JavaArray.kill§1620077312
   PURGE 5
   
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
   
   0 0
   LOCALS 1§ 0§ |
   
   i  <# 0 ..R JavaArray.createString§-105880832  DUP TO 1§
   U"  " COUNT JavaArray.createUnicode§-675323136 DUP TO 0§
   DUP INCREFERENCE  OVER  -1461427456 TRUE ( equation.String.concat§-1461427456 ) EXECUTE-METHOD
   1§ TRUE JavaArray.kill§1620077312
   0§ TRUE JavaArray.kill§1620077312
   
   PURGE 2
   
   0 §break231 BRANCH
   
   \ new statement
   0 §break231 LABEL
   
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
   
   0 0
   LOCALS 1§ 0§ |
   
   d  F.. JavaArray.createString§-105880832  DUP TO 1§
   U"  " COUNT JavaArray.createUnicode§-675323136 DUP TO 0§
   DUP INCREFERENCE  OVER  -1461427456 TRUE ( equation.String.concat§-1461427456 ) EXECUTE-METHOD
   1§ TRUE JavaArray.kill§1620077312
   0§ TRUE JavaArray.kill§1620077312
   
   PURGE 2
   
   0 §break233 BRANCH
   
   \ new statement
   0 §break233 LABEL
   
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
   0 §break229 BRANCH
   
   \ new statement
   0 §break229 LABEL
   
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
   
   0 0
   LOCALS 1§ 0§ |
   
   l  <# 0 D..R JavaArray.createString§-105880832  DUP TO 1§
   U"  " COUNT JavaArray.createUnicode§-675323136 DUP TO 0§
   DUP INCREFERENCE  OVER  -1461427456 TRUE ( equation.String.concat§-1461427456 ) EXECUTE-METHOD
   1§ TRUE JavaArray.kill§1620077312
   0§ TRUE JavaArray.kill§1620077312
   
   PURGE 2
   
   0 §break232 BRANCH
   
   \ new statement
   0 §break232 LABEL
   
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
   LOCAL c
   
   \ new statement
   0
   LOCALS data |
   
   \ new statement
   1
   131073 JavaArray§1352878592.table -227194368 EXECUTE-NEW
   c OVER 0  OVER -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD  H!
   data TRUE JavaArray.kill§1620077312 TO data
   
   \ new statement
   data DUP INCREFERENCE
   0
   1
   String§1651856128.table 1127437056 EXECUTE-NEW
   0 §break230 BRANCH
   
   \ new statement
   0 §break230 LABEL
   
   \ new statement
   
   data TRUE JavaArray.kill§1620077312
   PURGE 2
   
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
   LOCAL obj
   
   \ new statement
   
   \ new statement
   obj
   0=
   
   IF
      U" null " COUNT JavaArray.createUnicode§-675323136
      
   ELSE
      obj A:R@
      R> 1621718016 TRUE ( equation.Object.toString§1621718016 ) EXECUTE-METHOD
      
   ENDIF
   0 §break224 BRANCH
   
   \ new statement
   0 §break224 LABEL
   
   \ new statement
   obj TRUE JavaArray.kill§1620077312
   PURGE 1
   
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
   LOCAL count
   LOCAL offset
   LOCAL data
   
   \ new statement
   
   \ new statement
   data DUP INCREFERENCE
   offset
   count
   String§1651856128.table 1127437056 EXECUTE-NEW
   0 §break226 BRANCH
   
   \ new statement
   0 §break226 LABEL
   
   \ new statement
   
   data TRUE JavaArray.kill§1620077312
   
   PURGE 3
   
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
   LOCAL data
   
   \ new statement
   
   \ new statement
   data DUP INCREFERENCE
   String§1651856128.table -99003648 EXECUTE-NEW
   0 §break225 BRANCH
   
   \ new statement
   0 §break225 LABEL
   
   \ new statement
   data TRUE JavaArray.kill§1620077312
   PURGE 1
   
   \ new statement
   DROP
;


:LOCAL String.~destructor§-1086882304
   LOCAL §this
   
   \ new statement
   0
   LOCALS obj |
   
   \ new statement
   §this @ 1- 0=
   IF
      §this CELL+ @  @ TO obj
      
      \ new statement
      obj
      0
      JavaArray.kill§1620077312
      
      \ new statement
      §this CELL+ @ 8 +  @ TO obj
      
      \ new statement
      obj
      0
      JavaArray.kill§1620077312
      
      \ new statement
      §this CELL+ @ 28 + ( equation.String.value )       @ A:R@
      A:DROP
      TO obj
      
      \ new statement
      obj
      -1
      JavaArray.kill§1620077312
      
      \ new statement
   ENDIF
   §this DECREFERENCE
   
   \ new statement
   0 §break234 LABEL
   
   \ new statement
   PURGE 2
   
   \ new statement
   DROP
;
VARIABLE String.CASE_INSENSITIVE_ORDER
VARIABLE String._staticBlocking
VARIABLE String._staticThread
2VARIABLE String.serialVersionUID

A:HERE VARIABLE String§1651856128.table 44 DUP 2* CELLS ALLOT R@ ! A:CELL+
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
-1086882304 R@ ! A:CELL+ String.~destructor§-1086882304 VAL R@ ! A:CELL+
A:DROP


:LOCAL String.CaseInsensitiveComparator.CaseInsensitiveComparator§-42253568
   
   \ new statement
   0
   LOCALS §this |
   
   \ new statement
   0
   
   \ new statement
   28 MALLOC DROP DUP TO §this OVER
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
   §this ( return object )    
   \ new statement
   0 §break117 LABEL
   
   \ new statement
   PURGE 1
   
   \ new statement
   DROP
;


: String.CaseInsensitiveComparator.classMonitorEnter§-433167616
   
   \ new statement
   
   \ new statement
   0 §break109 LABEL
   
   \ new statement
   
   \ new statement
   DROP
;


: String.CaseInsensitiveComparator.classMonitorLeave§-1691393280
   
   \ new statement
   
   \ new statement
   0 §break110 LABEL
   
   \ new statement
   
   \ new statement
   DROP
;


:LOCAL String.CaseInsensitiveComparator.compare§2079286016
   LOCAL o2
   LOCAL o1
   LOCAL §this
   
   \ new statement
   0 DUP DUP DUP
   LOCALS n1 n2 s1 s2 |
   
   \ new statement
   o1
   DUP INCREFERENCE
   s1 TRUE JavaArray.kill§1620077312 TO s1
   
   \ new statement
   o2
   DUP INCREFERENCE
   s2 TRUE JavaArray.kill§1620077312 TO s2
   
   \ new statement
   s1 A:R@
   R> 188050432 TRUE ( equation.String.length§188050432 ) EXECUTE-METHOD
   TO n1
   
   \ new statement
   s2 A:R@
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
      < and_1682 0BRANCH! DROP
      i2
      n2
      <
      and_1682 LABEL
      
      \ new statement
   WHILE
      
      \ new statement
      s1 A:R@
      i1
      R> -1014275328 TRUE ( equation.String.charAt§-1014275328 ) EXECUTE-METHOD
      TO c1
      
      \ new statement
      s2 A:R@
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
               262148 §break116 BRANCH
               
               \ new statement
               0 §break116 LABEL
               
               \ new statement
               
               \ new statement
               0>!
               IF
                  10001H - §break115 BRANCH
               ENDIF
               DROP
               
               \ new statement
            ENDIF
            
            \ new statement
            0 §break115 LABEL
            
            \ new statement
            
            \ new statement
            0>!
            IF
               10001H - §break114 BRANCH
            ENDIF
            DROP
            
            \ new statement
         ENDIF
         
         \ new statement
         0 §break114 LABEL
         
         \ new statement
         
         \ new statement
         0>!
         IF
            10001H - 0<! §break112 0BRANCH
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
   0 §break112 LABEL
   
   \ new statement
   
   
   
   
   PURGE 4
   
   \ new statement
   0>!
   IF
      10001H - §break111 BRANCH
   ENDIF
   DROP
   
   \ new statement
   n1
   n2
   -
   0 §break111 BRANCH
   
   \ new statement
   0 §break111 LABEL
   
   \ new statement
   
   
   o1 TRUE JavaArray.kill§1620077312
   o2 TRUE JavaArray.kill§1620077312
   s1 TRUE JavaArray.kill§1620077312
   s2 TRUE JavaArray.kill§1620077312
   PURGE 7
   
   \ new statement
   DROP
;


:LOCAL String.CaseInsensitiveComparator.~destructor§-1086882304
   LOCAL §this
   
   \ new statement
   0
   LOCALS obj |
   
   \ new statement
   §this @ 1- 0=
   IF
      §this CELL+ @  @ TO obj
      
      \ new statement
      obj
      0
      JavaArray.kill§1620077312
      
      \ new statement
      §this CELL+ @ 8 +  @ TO obj
      
      \ new statement
      obj
      0
      JavaArray.kill§1620077312
      
      \ new statement
   ENDIF
   §this DECREFERENCE
   
   \ new statement
   0 §break118 LABEL
   
   \ new statement
   PURGE 2
   
   \ new statement
   DROP
;
VARIABLE String.CaseInsensitiveComparator._staticBlocking
VARIABLE String.CaseInsensitiveComparator._staticThread
2VARIABLE String.CaseInsensitiveComparator.serialVersionUID

A:HERE VARIABLE String.CaseInsensitiveComparator§-2000076032.table 3 DUP 2* CELLS ALLOT R@ ! A:CELL+
-42253568 R@ ! A:CELL+ String.CaseInsensitiveComparator.CaseInsensitiveComparator§-42253568 VAL R@ ! A:CELL+
2079286016 R@ ! A:CELL+ String.CaseInsensitiveComparator.compare§2079286016 VAL R@ ! A:CELL+
-1086882304 R@ ! A:CELL+ String.CaseInsensitiveComparator.~destructor§-1086882304 VAL R@ ! A:CELL+
A:DROP
