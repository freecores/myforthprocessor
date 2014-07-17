MODULENAME java.lang.StringBuffer
(
* @(#)StringBuffer.java	1.78 03/05/16
*
* Copyright 2003 Sun Microsystems, Inc. All rights reserved.
* SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
)  ( *
* A string buffer implements a mutable sequence of characters.
* A string buffer is like a {@link String}, but can be modified. At any
* point in time it contains some particular sequence of characters, but
* the length and content of the sequence can be changed through certain
* method calls.
* <p>
* String buffers are safe for use by multiple threads. The methods
* are synchronized where necessary so that all the operations on any
* particular instance behave as if they occur in some serial order
* that is consistent with the order of the method calls made by each of
* the individual threads involved.
* <p>
* String buffers are used by the compiler to implement the binary
* string concatenation operator <code>+</code>. For example, the code:
* <p><blockquote><pre>
*     x = "a" + 4 + "c"
* </pre></blockquote><p>
* is compiled to the equivalent of:
* <p><blockquote><pre>
*     x = new StringBuffer().append("a").append(4).append("c")
*                           .toString()
* </pre></blockquote>
* which creates a new string buffer (initially empty), appends the string
* representation of each operand to the string buffer in turn, and then
* converts the contents of the string buffer to a string. Overall, this avoids
* creating many temporary strings.
* <p>
* The principal operations on a <code>StringBuffer</code> are the
* <code>append</code> and <code>insert</code> methods, which are
* overloaded so as to accept data of any type. Each effectively
* converts a given datum to a string and then appends or inserts the
* characters of that string to the string buffer. The
* <code>append</code> method always adds these characters at the end
* of the buffer; the <code>insert</code> method adds the characters at
* a specified point.
* <p>
* For example, if <code>z</code> refers to a string buffer object
* whose current contents are "<code>start</code>", then
* the method call <code>z.append("le")</code> would cause the string
* buffer to contain "<code>startle</code>", whereas
* <code>z.insert(4, "le")</code> would alter the string buffer to
* contain "<code>starlet</code>".
* <p>
* In general, if sb refers to an instance of a <code>StringBuffer</code>,
* then <code>sb.append(x)</code> has the same effect as
* <code>sb.insert(sb.length(),&nbsp;x)</code>.
* <p>
* Every string buffer has a capacity. As long as the length of the
* character sequence contained in the string buffer does not exceed
* the capacity, it is not necessary to allocate a new internal
* buffer array. If the internal buffer overflows, it is
* automatically made larger.
*
* @author	Arthur van Hoff
* @version 	1.78, 05/16/03
* @see     java.io.ByteArrayOutputStream
* @see     java.lang.String
* @since   JDK1.0
)
( *
* Constructs a string buffer with no characters in it and an
* initial capacity specified by the <code>length</code> argument.
*
* @param      length   the initial capacity.
* @exception  NegativeArraySizeException  if the <code>length</code>
*               argument is less than <code>0</code>.
)

:LOCAL lang.StringBuffer.StringBuffer§-2134617344
   1 VALLOCATE LOCAL §base0
   LOCAL length
   
   \ new statement
   0
   LOCALS §this |
   
   \ new statement
   lang.Object§-1890496768.table 1827294976 EXECUTE-NEW
   
   \ new statement
   40 MALLOC DROP DUP DUP §base0 V! TO §this OVER
   IF
      OVER CELL+ @ 4 + OVER OVER ! DROP
   ENDIF
   CELL+ @ OVER
   OVER ! NIP
   lang.StringBuffer§426332928.table OVER 12 + !
   426311680 OVER 20 + !
   " StringBuffer " OVER 16 + !
   1 OVER 24 + ! DROP
   
   \ new statement
   ( *
   * The value is used for character storage.
   *
   * @serial
   )
   
   \ new statement
   ( *
   * The count is the number of characters in the buffer.
   *
   * @serial
   )
   
   \ new statement
   ( *
   * A flag indicating whether the buffer is shared
   *
   * @serial
   )
   
   \ new statement
   
   1 VALLOCATE LOCAL §tempvar
   length 131073 lang.JavaArray§1352878592.table -227194368 EXECUTE-NEW
   §this CELL+ @ 28 + ( java.lang.StringBuffer.value )    OVER 0 V! !
   
   §tempvar SETVTOP
   
   PURGE 1
   
   
   \ new statement
   FALSE
   §this CELL+ @ 36 + ( java.lang.StringBuffer.shared )    !
   
   \ new statement
   §this DUP 0 V! ( return object )    
   \ new statement
   0 §break18661 LABEL
   
   \ new statement
   
   §base0 SETVTOP
   PURGE 3
   
   \ new statement
   DROP
;
( *
* Constructs a string buffer so that it represents the same
* sequence of characters as the string argument; in other
* words, the initial contents of the string buffer is a copy of the
* argument string. The initial capacity of the string buffer is
* <code>16</code> plus the length of the string argument.
*
* @param   str   the initial contents of the buffer.
* @exception NullPointerException if <code>str</code> is <code>null</code>
)

:LOCAL lang.StringBuffer.StringBuffer§421942016
   2 VALLOCATE LOCAL §base0
   DUP 4 §base0 + V! LOCAL str
   
   \ new statement
   0
   LOCALS §this |
   
   \ new statement
   str A:R@
   R> 188050432 TRUE ( java.lang.String.length§188050432 ) EXECUTE-METHOD
   16
   +
   lang.StringBuffer§426332928.table -2134617344 EXECUTE-NEW
   DUP §base0 V! TO §this
   
   \ new statement
   
   1 VALLOCATE LOCAL §tempvar
   §this A:R@
   str
   R> -310157056 TRUE ( java.lang.StringBuffer.append§-310157056 ) EXECUTE-METHOD DUP §tempvar V!
   DROP
   
   §tempvar SETVTOP
   
   PURGE 1
   
   
   \ new statement
   §this DUP 0 V! ( return object )    
   \ new statement
   0 §break18662 LABEL
   
   \ new statement
   
   §base0 SETVTOP
   PURGE 3
   
   \ new statement
   DROP
;
( *
* Constructs a string buffer with no characters in it and an
* initial capacity of 16 characters.
)

:LOCAL lang.StringBuffer.StringBuffer§495342336
   1 VALLOCATE LOCAL §base0
   
   \ new statement
   0
   LOCALS §this |
   
   \ new statement
   16
   lang.StringBuffer§426332928.table -2134617344 EXECUTE-NEW
   DUP §base0 V! TO §this
   
   \ new statement
   §this DUP 0 V! ( return object )    
   \ new statement
   0 §break18660 LABEL
   
   \ new statement
   §base0 SETVTOP
   PURGE 2
   
   \ new statement
   DROP
;
( *
* Appends the string representation of the <code>long</code>
* argument to this string buffer.
* <p>
* The argument is converted to a string as if by the method
* <code>String.valueOf</code>, and the characters of that
* string are then appended to this string buffer.
*
* @param   l   a <code>long</code>.
* @return  a reference to this <code>StringBuffer</code> object.
* @see     java.lang.String#valueOf(long)
* @see     java.lang.StringBuffer#append(java.lang.String)
)

:LOCAL lang.StringBuffer.append§-1086627584
   1 VALLOCATE LOCAL §base0
   2LOCAL l
   DUP 0 §base0 + V! LOCAL §this
   
   \ new statement
   
   \ new statement
   §this LOCAL §synchronized0
   DI
   §synchronized0 A:R@
   lang.StringBuffer._staticThread
   R> 1472163072 TRUE ( Object.monitorEnter ) EXECUTE-METHOD
   EI
   
   \ new statement
   l
   §this
   lang.Long.appendTo§-1166319360
   
   \ new statement
   §this
   DUP 0 V!0 §break18701 BRANCH
   
   \ new statement
   0 §break18701 LABEL
   
   \ new statement
   §synchronized0 A:R@
   R> 1974299904 TRUE ( Object.monitorLeave ) EXECUTE-METHOD
   PURGE 1
   
   \ new statement
   
   §base0 SETVTOP
   PURGE 3
   
   \ new statement
   DROP
;
( *
* Appends the specified <tt>StringBuffer</tt> to this
* <tt>StringBuffer</tt>.
* <p>
* The characters of the <tt>StringBuffer</tt> argument are appended,
* in order, to the contents of this <tt>StringBuffer</tt>, increasing the
* length of this <tt>StringBuffer</tt> by the length of the argument.
* If <tt>sb</tt> is <tt>null</tt>, then the four characters
* <tt>"null"</tt> are appended to this <tt>StringBuffer</tt>.
* <p>
* Let <i>n</i> be the length of the old character sequence, the one
* contained in the <tt>StringBuffer</tt> just prior to execution of the
* <tt>append</tt> method. Then the character at index <i>k</i> in
* the new character sequence is equal to the character at index <i>k</i>
* in the old character sequence, if <i>k</i> is less than <i>n</i>;
* otherwise, it is equal to the character at index <i>k-n</i> in the
* argument <code>sb</code>.
* <p>
* The method <tt>ensureCapacity</tt> is first called on this
* <tt>StringBuffer</tt> with the new buffer length as its argument.
* (This ensures that the storage of this <tt>StringBuffer</tt> is
* adequate to contain the additional characters being appended.)
*
* @param   sb         the <tt>StringBuffer</tt> to append.
* @return  a reference to this <tt>StringBuffer</tt>.
* @since 1.4
)

:LOCAL lang.StringBuffer.append§-1912250112
   2 VALLOCATE LOCAL §base0
   DUP 0 §base0 + V! LOCAL sb
   DUP 4 §base0 + V! LOCAL §this
   
   \ new statement
   0 DUP
   LOCALS len newcount |
   
   \ new statement
   §this LOCAL §synchronized0
   DI
   §synchronized0 A:R@
   lang.StringBuffer._staticThread
   R> 1472163072 TRUE ( Object.monitorEnter ) EXECUTE-METHOD
   EI
   
   \ new statement
   sb
   0=
   
   \ new statement
   IF
      
      \ new statement
      
      \ new statement
      lang.StringBuffer.NULL
      @
      DUP §base0 V! TO sb
      
      \ new statement
      0 §break18693 LABEL
      
      \ new statement
      
      \ new statement
      DROP
      
      \ new statement
   ENDIF
   
   \ new statement
   sb A:R@
   R> 188050432 TRUE ( java.lang.StringBuffer.length§188050432 ) EXECUTE-METHOD
   TO len
   
   \ new statement
   §this CELL+ @ 32 + ( java.lang.StringBuffer.count )    @
   len
   +
   TO newcount
   
   \ new statement
   newcount
   §this CELL+ @ 28 + ( java.lang.StringBuffer.value ) @ CELL+ @ 28 + ( java.lang.JavaArray.length )    @
   >
   
   \ new statement
   IF
      
      \ new statement
      §this A:R@
      newcount
      R> -1659607808 TRUE ( java.lang.StringBuffer.expandCapacity§-1659607808 ) EXECUTE-METHOD
      
      \ new statement
   ENDIF
   
   \ new statement
   sb A:R@
   0
   len
   §this CELL+ @ 28 + ( java.lang.StringBuffer.value )    @
   §this CELL+ @ 32 + ( java.lang.StringBuffer.count )    @
   R> -53713152 TRUE ( java.lang.StringBuffer.getChars§-53713152 ) EXECUTE-METHOD
   
   \ new statement
   newcount
   §this CELL+ @ 32 + ( java.lang.StringBuffer.count )    !
   
   \ new statement
   §this
   DUP 0 V!0 §break18692 BRANCH
   
   \ new statement
   0 §break18692 LABEL
   
   \ new statement
   §synchronized0 A:R@
   R> 1974299904 TRUE ( Object.monitorLeave ) EXECUTE-METHOD
   PURGE 1
   
   \ new statement
   
   
   
   §base0 SETVTOP
   PURGE 5
   
   \ new statement
   DROP
;
( *
* Appends the string representation of a subarray of the
* <code>char</code> array argument to this string buffer.
* <p>
* Characters of the character array <code>str</code>, starting at
* index <code>offset</code>, are appended, in order, to the contents
* of this string buffer. The length of this string buffer increases
* by the value of <code>len</code>.
* <p>
* The overall effect is exactly as if the arguments were converted to
* a string by the method {@link String#valueOf(char[],int,int)} and the
* characters of that string were then {@link #append(String) appended}
* to this <code>StringBuffer</code> object.
*
* @param   str      the characters to be appended.
* @param   offset   the index of the first character to append.
* @param   len      the number of characters to append.
* @return  a reference to this <code>StringBuffer</code> object.
)

:LOCAL lang.StringBuffer.append§-1994301184
   2 VALLOCATE LOCAL §base0
   LOCAL len
   LOCAL offset
   DUP 0 §base0 + V! LOCAL str
   DUP 4 §base0 + V! LOCAL §this
   
   \ new statement
   0
   LOCALS newcount |
   
   \ new statement
   §this LOCAL §synchronized0
   DI
   §synchronized0 A:R@
   lang.StringBuffer._staticThread
   R> 1472163072 TRUE ( Object.monitorEnter ) EXECUTE-METHOD
   EI
   
   \ new statement
   §this CELL+ @ 32 + ( java.lang.StringBuffer.count )    @
   len
   +
   TO newcount
   
   \ new statement
   newcount
   §this CELL+ @ 28 + ( java.lang.StringBuffer.value ) @ CELL+ @ 28 + ( java.lang.JavaArray.length )    @
   >
   
   \ new statement
   IF
      
      \ new statement
      §this A:R@
      newcount
      R> -1659607808 TRUE ( java.lang.StringBuffer.expandCapacity§-1659607808 ) EXECUTE-METHOD
      
      \ new statement
   ENDIF
   
   \ new statement
   str
   offset
   §this CELL+ @ 28 + ( java.lang.StringBuffer.value )    @
   §this CELL+ @ 32 + ( java.lang.StringBuffer.count )    @
   len
   lang.System.arraycopy§1260740864
   
   \ new statement
   newcount
   §this CELL+ @ 32 + ( java.lang.StringBuffer.count )    !
   
   \ new statement
   §this
   DUP 0 V!0 §break18695 BRANCH
   
   \ new statement
   0 §break18695 LABEL
   
   \ new statement
   §synchronized0 A:R@
   R> 1974299904 TRUE ( Object.monitorLeave ) EXECUTE-METHOD
   PURGE 1
   
   \ new statement
   
   
   
   
   §base0 SETVTOP
   PURGE 6
   
   \ new statement
   DROP
;
( *
* Appends the string to this string buffer.
* <p>
* The characters of the <code>String</code> argument are appended, in
* order, to the contents of this string buffer, increasing the
* length of this string buffer by the length of the argument.
* If <code>str</code> is <code>null</code>, then the four characters
* <code>"null"</code> are appended to this string buffer.
* <p>
* Let <i>n</i> be the length of the old character sequence, the one
* contained in the string buffer just prior to execution of the
* <code>append</code> method. Then the character at index <i>k</i> in
* the new character sequence is equal to the character at index <i>k</i>
* in the old character sequence, if <i>k</i> is less than <i>n</i>;
* otherwise, it is equal to the character at index <i>k-n</i> in the
* argument <code>str</code>.
*
* @param   str   a string.
* @return  a reference to this <code>StringBuffer</code>.
)

:LOCAL lang.StringBuffer.append§-310157056
   2 VALLOCATE LOCAL §base0
   DUP 0 §base0 + V! LOCAL str
   DUP 4 §base0 + V! LOCAL §this
   
   \ new statement
   0 DUP
   LOCALS len newcount |
   
   \ new statement
   §this LOCAL §synchronized0
   DI
   §synchronized0 A:R@
   lang.StringBuffer._staticThread
   R> 1472163072 TRUE ( Object.monitorEnter ) EXECUTE-METHOD
   EI
   
   \ new statement
   str
   0=
   
   \ new statement
   IF
      
      \ new statement
      
      \ new statement
      str
      lang.String.valueOf§59340288
      DUP §base0 V! TO str
      
      \ new statement
      0 §break18691 LABEL
      
      \ new statement
      
      \ new statement
      DROP
      
      \ new statement
   ENDIF
   
   \ new statement
   str A:R@
   R> 188050432 TRUE ( java.lang.String.length§188050432 ) EXECUTE-METHOD
   TO len
   
   \ new statement
   §this CELL+ @ 32 + ( java.lang.StringBuffer.count )    @
   len
   +
   TO newcount
   
   \ new statement
   newcount
   §this CELL+ @ 28 + ( java.lang.StringBuffer.value ) @ CELL+ @ 28 + ( java.lang.JavaArray.length )    @
   >
   
   \ new statement
   IF
      
      \ new statement
      §this A:R@
      newcount
      R> -1659607808 TRUE ( java.lang.StringBuffer.expandCapacity§-1659607808 ) EXECUTE-METHOD
      
      \ new statement
   ENDIF
   
   \ new statement
   str A:R@
   0
   len
   §this CELL+ @ 28 + ( java.lang.StringBuffer.value )    @
   §this CELL+ @ 32 + ( java.lang.StringBuffer.count )    @
   R> -53713152 TRUE ( java.lang.String.getChars§-53713152 ) EXECUTE-METHOD
   
   \ new statement
   newcount
   §this CELL+ @ 32 + ( java.lang.StringBuffer.count )    !
   
   \ new statement
   §this
   DUP 0 V!0 §break18690 BRANCH
   
   \ new statement
   0 §break18690 LABEL
   
   \ new statement
   §synchronized0 A:R@
   R> 1974299904 TRUE ( Object.monitorLeave ) EXECUTE-METHOD
   PURGE 1
   
   \ new statement
   
   
   
   §base0 SETVTOP
   PURGE 5
   
   \ new statement
   DROP
;
( *
* Appends the string representation of the <code>float</code>
* argument to this string buffer.
* <p>
* The argument is converted to a string as if by the method
* <code>String.valueOf</code>, and the characters of that
* string are then appended to this string buffer.
*
* @param   f   a <code>float</code>.
* @return  a reference to this <code>StringBuffer</code> object.
* @see     java.lang.String#valueOf(float)
* @see     java.lang.StringBuffer#append(java.lang.String)
public synchronized StringBuffer append(float f) {
new FloatingDecimal(f).appendTo(this);
return this;
}
)  ( *
* Appends the string representation of the <code>double</code>
* argument to this string buffer.
* <p>
* The argument is converted to a string as if by the method
* <code>String.valueOf</code>, and the characters of that
* string are then appended to this string buffer.
*
* @param   d   a <code>double</code>.
* @return  a reference to this <code>StringBuffer</code> object.
* @see     java.lang.String#valueOf(double)
* @see     java.lang.StringBuffer#append(java.lang.String)
)

:LOCAL lang.StringBuffer.append§-523149056
   1 VALLOCATE LOCAL §base0
   2LOCAL d
   DUP 0 §base0 + V! LOCAL §this
   
   \ new statement
   
   \ new statement
   §this LOCAL §synchronized0
   DI
   §synchronized0 A:R@
   lang.StringBuffer._staticThread
   R> 1472163072 TRUE ( Object.monitorEnter ) EXECUTE-METHOD
   EI
   
   \ new statement
   d
   §this
   lang.Double.appendTo§-161455872
   
   \ new statement
   §this
   DUP 0 V!0 §break18702 BRANCH
   
   \ new statement
   0 §break18702 LABEL
   
   \ new statement
   §synchronized0 A:R@
   R> 1974299904 TRUE ( Object.monitorLeave ) EXECUTE-METHOD
   PURGE 1
   
   \ new statement
   
   §base0 SETVTOP
   PURGE 3
   
   \ new statement
   DROP
;
( *
* Appends the string representation of the <code>boolean</code>
* argument to the string buffer.
* <p>
* The argument is converted to a string as if by the method
* <code>String.valueOf</code>, and the characters of that
* string are then appended to this string buffer.
*
* @param   b   a <code>boolean</code>.
* @return  a reference to this <code>StringBuffer</code>.
* @see     java.lang.String#valueOf(boolean)
* @see     java.lang.StringBuffer#append(java.lang.String)
)

:LOCAL lang.StringBuffer.append§1081041152
   1 VALLOCATE LOCAL §base0
   LOCAL b
   DUP 0 §base0 + V! LOCAL §this
   
   \ new statement
   
   \ new statement
   §this LOCAL §synchronized0
   DI
   §synchronized0 A:R@
   lang.StringBuffer._staticThread
   R> 1472163072 TRUE ( Object.monitorEnter ) EXECUTE-METHOD
   EI
   
   \ new statement
   b
   
   \ new statement
   IF
      
      \ new statement
      0
      LOCALS newcount |
      
      \ new statement
      §this CELL+ @ 32 + ( java.lang.StringBuffer.count )       @
      4
      +
      TO newcount
      
      \ new statement
      newcount
      §this CELL+ @ 28 + ( java.lang.StringBuffer.value ) @ CELL+ @ 28 + ( java.lang.JavaArray.length )       @
      >
      
      \ new statement
      IF
         
         \ new statement
         §this A:R@
         newcount
         R> -1659607808 TRUE ( java.lang.StringBuffer.expandCapacity§-1659607808 ) EXECUTE-METHOD
         
         \ new statement
      ENDIF
      
      \ new statement
      
      0
      LOCALS 0§ |
      
      §this CELL+ @ 32 + ( java.lang.StringBuffer.count )       @ A:R@ DUP 1+ R@ !
      A:DROP
      TO 0§
      116
      §this CELL+ @ 28 + ( java.lang.StringBuffer.value ) @ A:R@
      0§
      R> -942971136 TRUE ( java.lang.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
      H!
      
      PURGE 1
      
      
      \ new statement
      
      0
      LOCALS 0§ |
      
      §this CELL+ @ 32 + ( java.lang.StringBuffer.count )       @ A:R@ DUP 1+ R@ !
      A:DROP
      TO 0§
      114
      §this CELL+ @ 28 + ( java.lang.StringBuffer.value ) @ A:R@
      0§
      R> -942971136 TRUE ( java.lang.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
      H!
      
      PURGE 1
      
      
      \ new statement
      
      0
      LOCALS 0§ |
      
      §this CELL+ @ 32 + ( java.lang.StringBuffer.count )       @ A:R@ DUP 1+ R@ !
      A:DROP
      TO 0§
      117
      §this CELL+ @ 28 + ( java.lang.StringBuffer.value ) @ A:R@
      0§
      R> -942971136 TRUE ( java.lang.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
      H!
      
      PURGE 1
      
      
      \ new statement
      
      0
      LOCALS 0§ |
      
      §this CELL+ @ 32 + ( java.lang.StringBuffer.count )       @ A:R@ DUP 1+ R@ !
      A:DROP
      TO 0§
      101
      §this CELL+ @ 28 + ( java.lang.StringBuffer.value ) @ A:R@
      0§
      R> -942971136 TRUE ( java.lang.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
      H!
      
      PURGE 1
      
      
      \ new statement
      0 §break18697 LABEL
      
      \ new statement
      
      PURGE 1
      
      \ new statement
      DROP
      
      \ new statement
   ELSE
      
      \ new statement
      0
      LOCALS newcount |
      
      \ new statement
      §this CELL+ @ 32 + ( java.lang.StringBuffer.count )       @
      5
      +
      TO newcount
      
      \ new statement
      newcount
      §this CELL+ @ 28 + ( java.lang.StringBuffer.value ) @ CELL+ @ 28 + ( java.lang.JavaArray.length )       @
      >
      
      \ new statement
      IF
         
         \ new statement
         §this A:R@
         newcount
         R> -1659607808 TRUE ( java.lang.StringBuffer.expandCapacity§-1659607808 ) EXECUTE-METHOD
         
         \ new statement
      ENDIF
      
      \ new statement
      
      0
      LOCALS 0§ |
      
      §this CELL+ @ 32 + ( java.lang.StringBuffer.count )       @ A:R@ DUP 1+ R@ !
      A:DROP
      TO 0§
      102
      §this CELL+ @ 28 + ( java.lang.StringBuffer.value ) @ A:R@
      0§
      R> -942971136 TRUE ( java.lang.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
      H!
      
      PURGE 1
      
      
      \ new statement
      
      0
      LOCALS 0§ |
      
      §this CELL+ @ 32 + ( java.lang.StringBuffer.count )       @ A:R@ DUP 1+ R@ !
      A:DROP
      TO 0§
      97
      §this CELL+ @ 28 + ( java.lang.StringBuffer.value ) @ A:R@
      0§
      R> -942971136 TRUE ( java.lang.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
      H!
      
      PURGE 1
      
      
      \ new statement
      
      0
      LOCALS 0§ |
      
      §this CELL+ @ 32 + ( java.lang.StringBuffer.count )       @ A:R@ DUP 1+ R@ !
      A:DROP
      TO 0§
      108
      §this CELL+ @ 28 + ( java.lang.StringBuffer.value ) @ A:R@
      0§
      R> -942971136 TRUE ( java.lang.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
      H!
      
      PURGE 1
      
      
      \ new statement
      
      0
      LOCALS 0§ |
      
      §this CELL+ @ 32 + ( java.lang.StringBuffer.count )       @ A:R@ DUP 1+ R@ !
      A:DROP
      TO 0§
      115
      §this CELL+ @ 28 + ( java.lang.StringBuffer.value ) @ A:R@
      0§
      R> -942971136 TRUE ( java.lang.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
      H!
      
      PURGE 1
      
      
      \ new statement
      
      0
      LOCALS 0§ |
      
      §this CELL+ @ 32 + ( java.lang.StringBuffer.count )       @ A:R@ DUP 1+ R@ !
      A:DROP
      TO 0§
      101
      §this CELL+ @ 28 + ( java.lang.StringBuffer.value ) @ A:R@
      0§
      R> -942971136 TRUE ( java.lang.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
      H!
      
      PURGE 1
      
      
      \ new statement
      0 §break18698 LABEL
      
      \ new statement
      
      PURGE 1
      
      \ new statement
      DROP
      
      \ new statement
   ENDIF
   
   \ new statement
   §this
   DUP 0 V!0 §break18696 BRANCH
   
   \ new statement
   0 §break18696 LABEL
   
   \ new statement
   §synchronized0 A:R@
   R> 1974299904 TRUE ( Object.monitorLeave ) EXECUTE-METHOD
   PURGE 1
   
   \ new statement
   
   §base0 SETVTOP
   PURGE 3
   
   \ new statement
   DROP
;
( *
* Appends the string representation of the <code>Object</code>
* argument to this string buffer.
* <p>
* The argument is converted to a string as if by the method
* <code>String.valueOf</code>, and the characters of that
* string are then appended to this string buffer.
*
* @param   obj   an <code>Object</code>.
* @return  a reference to this <code>StringBuffer</code> object.
* @see     java.lang.String#valueOf(java.lang.Object)
* @see     java.lang.StringBuffer#append(java.lang.String)
)

:LOCAL lang.StringBuffer.append§1445355776
   2 VALLOCATE LOCAL §base0
   DUP 0 §base0 + V! LOCAL obj
   DUP 4 §base0 + V! LOCAL §this
   
   \ new statement
   
   \ new statement
   §this LOCAL §synchronized0
   DI
   §synchronized0 A:R@
   lang.StringBuffer._staticThread
   R> 1472163072 TRUE ( Object.monitorEnter ) EXECUTE-METHOD
   EI
   
   \ new statement
   
   1 VALLOCATE LOCAL §tempvar
   §this A:R@
   obj
   lang.String.valueOf§59340288 DUP §tempvar V!
   R> -310157056 TRUE ( java.lang.StringBuffer.append§-310157056 ) EXECUTE-METHOD
   DUP 0 V!0 §break18689 BRANCH
   
   
   §tempvar SETVTOP
   
   PURGE 1
   
   
   \ new statement
   0 §break18689 LABEL
   
   \ new statement
   §synchronized0 A:R@
   R> 1974299904 TRUE ( Object.monitorLeave ) EXECUTE-METHOD
   PURGE 1
   
   \ new statement
   
   §base0 SETVTOP
   PURGE 3
   
   \ new statement
   DROP
;
( *
* Appends the string representation of the <code>char</code> array
* argument to this string buffer.
* <p>
* The characters of the array argument are appended, in order, to
* the contents of this string buffer. The length of this string
* buffer increases by the length of the argument.
* <p>
* The overall effect is exactly as if the argument were converted to
* a string by the method {@link String#valueOf(char[])} and the
* characters of that string were then {@link #append(String) appended}
* to this <code>StringBuffer</code> object.
*
* @param   str   the characters to be appended.
* @return  a reference to this <code>StringBuffer</code> object.
)

:LOCAL lang.StringBuffer.append§1793024256
   2 VALLOCATE LOCAL §base0
   DUP 0 §base0 + V! LOCAL str
   DUP 4 §base0 + V! LOCAL §this
   
   \ new statement
   0 DUP
   LOCALS len newcount |
   
   \ new statement
   §this LOCAL §synchronized0
   DI
   §synchronized0 A:R@
   lang.StringBuffer._staticThread
   R> 1472163072 TRUE ( Object.monitorEnter ) EXECUTE-METHOD
   EI
   
   \ new statement
   str CELL+ @ 28 + ( java.lang.JavaArray.length )    @
   TO len
   
   \ new statement
   §this CELL+ @ 32 + ( java.lang.StringBuffer.count )    @
   len
   +
   TO newcount
   
   \ new statement
   newcount
   §this CELL+ @ 28 + ( java.lang.StringBuffer.value ) @ CELL+ @ 28 + ( java.lang.JavaArray.length )    @
   >
   
   \ new statement
   IF
      
      \ new statement
      §this A:R@
      newcount
      R> -1659607808 TRUE ( java.lang.StringBuffer.expandCapacity§-1659607808 ) EXECUTE-METHOD
      
      \ new statement
   ENDIF
   
   \ new statement
   str
   0
   §this CELL+ @ 28 + ( java.lang.StringBuffer.value )    @
   §this CELL+ @ 32 + ( java.lang.StringBuffer.count )    @
   len
   lang.System.arraycopy§1260740864
   
   \ new statement
   newcount
   §this CELL+ @ 32 + ( java.lang.StringBuffer.count )    !
   
   \ new statement
   §this
   DUP 0 V!0 §break18694 BRANCH
   
   \ new statement
   0 §break18694 LABEL
   
   \ new statement
   §synchronized0 A:R@
   R> 1974299904 TRUE ( Object.monitorLeave ) EXECUTE-METHOD
   PURGE 1
   
   \ new statement
   
   
   
   §base0 SETVTOP
   PURGE 5
   
   \ new statement
   DROP
;
( *
* Appends the string representation of the <code>char</code>
* argument to this string buffer.
* <p>
* The argument is appended to the contents of this string buffer.
* The length of this string buffer increases by <code>1</code>.
* <p>
* The overall effect is exactly as if the argument were converted to
* a string by the method {@link String#valueOf(char)} and the character
* in that string were then {@link #append(String) appended} to this
* <code>StringBuffer</code> object.
*
* @param   c   a <code>char</code>.
* @return  a reference to this <code>StringBuffer</code> object.
)

:LOCAL lang.StringBuffer.append§2069258496
   1 VALLOCATE LOCAL §base0
   LOCAL c
   DUP 0 §base0 + V! LOCAL §this
   
   \ new statement
   0
   LOCALS newcount |
   
   \ new statement
   §this LOCAL §synchronized0
   DI
   §synchronized0 A:R@
   lang.StringBuffer._staticThread
   R> 1472163072 TRUE ( Object.monitorEnter ) EXECUTE-METHOD
   EI
   
   \ new statement
   §this CELL+ @ 32 + ( java.lang.StringBuffer.count )    @
   1
   +
   TO newcount
   
   \ new statement
   newcount
   §this CELL+ @ 28 + ( java.lang.StringBuffer.value ) @ CELL+ @ 28 + ( java.lang.JavaArray.length )    @
   >
   
   \ new statement
   IF
      
      \ new statement
      §this A:R@
      newcount
      R> -1659607808 TRUE ( java.lang.StringBuffer.expandCapacity§-1659607808 ) EXECUTE-METHOD
      
      \ new statement
   ENDIF
   
   \ new statement
   
   0
   LOCALS 0§ |
   
   §this CELL+ @ 32 + ( java.lang.StringBuffer.count )    @ A:R@ DUP 1+ R@ !
   A:DROP
   TO 0§
   c
   §this CELL+ @ 28 + ( java.lang.StringBuffer.value ) @ A:R@
   0§
   R> -942971136 TRUE ( java.lang.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
   H!
   
   PURGE 1
   
   
   \ new statement
   §this
   DUP 0 V!0 §break18699 BRANCH
   
   \ new statement
   0 §break18699 LABEL
   
   \ new statement
   §synchronized0 A:R@
   R> 1974299904 TRUE ( Object.monitorLeave ) EXECUTE-METHOD
   PURGE 1
   
   \ new statement
   
   
   §base0 SETVTOP
   PURGE 4
   
   \ new statement
   DROP
;
( *
* Appends the string representation of the <code>int</code>
* argument to this string buffer.
* <p>
* The argument is converted to a string as if by the method
* <code>String.valueOf</code>, and the characters of that
* string are then appended to this string buffer.
*
* @param   i   an <code>int</code>.
* @return  a reference to this <code>StringBuffer</code> object.
* @see     java.lang.String#valueOf(int)
* @see     java.lang.StringBuffer#append(java.lang.String)
)

:LOCAL lang.StringBuffer.append§522477824
   1 VALLOCATE LOCAL §base0
   LOCAL i
   DUP 0 §base0 + V! LOCAL §this
   
   \ new statement
   
   \ new statement
   §this LOCAL §synchronized0
   DI
   §synchronized0 A:R@
   lang.StringBuffer._staticThread
   R> 1472163072 TRUE ( Object.monitorEnter ) EXECUTE-METHOD
   EI
   
   \ new statement
   i
   §this
   lang.Integer.appendTo§-642555648
   
   \ new statement
   §this
   DUP 0 V!0 §break18700 BRANCH
   
   \ new statement
   0 §break18700 LABEL
   
   \ new statement
   §synchronized0 A:R@
   R> 1974299904 TRUE ( Object.monitorLeave ) EXECUTE-METHOD
   PURGE 1
   
   \ new statement
   
   §base0 SETVTOP
   PURGE 3
   
   \ new statement
   DROP
;
( *
* Returns the current capacity of the String buffer. The capacity
* is the amount of storage available for newly inserted
* characters; beyond which an allocation will occur.
*
* @return  the current capacity of this string buffer.
)

:LOCAL lang.StringBuffer.capacity§371417856
   1 VALLOCATE LOCAL §base0
   DUP 0 §base0 + V! LOCAL §this
   
   \ new statement
   
   \ new statement
   §this LOCAL §synchronized0
   DI
   §synchronized0 A:R@
   lang.StringBuffer._staticThread
   R> 1472163072 TRUE ( Object.monitorEnter ) EXECUTE-METHOD
   EI
   
   \ new statement
   §this CELL+ @ 28 + ( java.lang.StringBuffer.value ) @ CELL+ @ 28 + ( java.lang.JavaArray.length )    @
   0 §break18664 BRANCH
   
   \ new statement
   0 §break18664 LABEL
   
   \ new statement
   §synchronized0 A:R@
   R> 1974299904 TRUE ( Object.monitorLeave ) EXECUTE-METHOD
   PURGE 1
   
   \ new statement
   §base0 SETVTOP
   PURGE 2
   
   \ new statement
   DROP
;
( *
* The specified character of the sequence currently represented by
* the string buffer, as indicated by the <code>index</code> argument,
* is returned. The first character of a string buffer is at index
* <code>0</code>, the next at index <code>1</code>, and so on, for
* array indexing.
* <p>
* The index argument must be greater than or equal to
* <code>0</code>, and less than the length of this string buffer.
*
* @param      index   the index of the desired character.
* @return     the character at the specified index of this string buffer.
* @exception  IndexOutOfBoundsException  if <code>index</code> is
*             negative or greater than or equal to <code>length()</code>.
* @see        java.lang.StringBuffer#length()
)

:LOCAL lang.StringBuffer.charAt§-1014275328
   1 VALLOCATE LOCAL §base0
   LOCAL index
   DUP 0 §base0 + V! LOCAL §this
   
   \ new statement
   
   \ new statement
   §this LOCAL §synchronized0
   DI
   §synchronized0 A:R@
   lang.StringBuffer._staticThread
   R> 1472163072 TRUE ( Object.monitorEnter ) EXECUTE-METHOD
   EI
   
   \ new statement
   index
   0<
   0=! or_19136 0BRANCH DROP
   index
   §this CELL+ @ 32 + ( java.lang.StringBuffer.count )    @
   < INVERT
   or_19136 LABEL
   
   \ new statement
   IF
      
      \ new statement
      
      \ new statement
      index
      lang.StringIndexOutOfBoundsException§-945597696.table 1138905856 EXECUTE-NEW
      lang.JavaArray.handler§-1096259584
      
      \ new statement
      
      \ new statement
      0 §break18682 LABEL
      
      \ new statement
      
      \ new statement
      0>!
      IF
         10001H - §break18681 BRANCH
      ENDIF
      DROP
      
      \ new statement
   ENDIF
   
   \ new statement
   §this CELL+ @ 28 + ( java.lang.StringBuffer.value ) @ A:R@
   index
   R> -942971136 TRUE ( java.lang.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
   H@
   0 §break18681 BRANCH
   
   \ new statement
   0 §break18681 LABEL
   
   \ new statement
   §synchronized0 A:R@
   R> 1974299904 TRUE ( Object.monitorLeave ) EXECUTE-METHOD
   PURGE 1
   
   \ new statement
   
   §base0 SETVTOP
   PURGE 3
   
   \ new statement
   DROP
;


: lang.StringBuffer.classMonitorEnter§-433167616
   
   \ new statement
   
   \ new statement
   0 §break18658 LABEL
   
   \ new statement
   
   \ new statement
   DROP
;


: lang.StringBuffer.classMonitorLeave§-1691393280
   
   \ new statement
   
   \ new statement
   0 §break18659 LABEL
   
   \ new statement
   
   \ new statement
   DROP
;
( *
* Copies the buffer value.  This is normally only called when shared
* is true.  It should only be called from a synchronized method.
)

:LOCAL lang.StringBuffer.copy§1412784896
   2 VALLOCATE LOCAL §base0
   DUP 0 §base0 + V! LOCAL §this
   
   \ new statement
   0
   LOCALS newValue |
   
   \ new statement
   
   1 VALLOCATE LOCAL §tempvar
   §this CELL+ @ 28 + ( java.lang.StringBuffer.value ) @ CELL+ @ 28 + ( java.lang.JavaArray.length )    @ 131073 lang.JavaArray§1352878592.table -227194368 EXECUTE-NEW
   DUP 4 §base0 + V! TO newValue
   
   §tempvar SETVTOP
   
   PURGE 1
   
   
   \ new statement
   §this CELL+ @ 28 + ( java.lang.StringBuffer.value )    @
   0
   newValue
   0
   §this CELL+ @ 32 + ( java.lang.StringBuffer.count )    @
   lang.System.arraycopy§1260740864
   
   \ new statement
   newValue
   §this CELL+ @ 28 + ( java.lang.StringBuffer.value )    OVER 0 V! !
   
   \ new statement
   FALSE
   §this CELL+ @ 36 + ( java.lang.StringBuffer.shared )    !
   
   \ new statement
   0 §break18665 LABEL
   
   \ new statement
   
   §base0 SETVTOP
   PURGE 3
   
   \ new statement
   DROP
;
( *
* Removes the characters in a substring of this <code>StringBuffer</code>.
* The substring begins at the specified <code>start</code> and extends to
* the character at index <code>end - 1</code> or to the end of the
* <code>StringBuffer</code> if no such character exists. If
* <code>start</code> is equal to <code>end</code>, no changes are made.
*
* @param      start  The beginning index, inclusive.
* @param      end    The ending index, exclusive.
* @return     This string buffer.
* @exception  StringIndexOutOfBoundsException  if <code>start</code>
*             is negative, greater than <code>length()</code>, or
*		   greater than <code>end</code>.
* @since      1.2
)

:LOCAL lang.StringBuffer.delete§162817024
   1 VALLOCATE LOCAL §base0
   LOCAL end
   LOCAL start
   DUP 0 §base0 + V! LOCAL §this
   
   \ new statement
   0
   LOCALS len |
   
   \ new statement
   §this LOCAL §synchronized0
   DI
   §synchronized0 A:R@
   lang.StringBuffer._staticThread
   R> 1472163072 TRUE ( Object.monitorEnter ) EXECUTE-METHOD
   EI
   
   \ new statement
   start
   0<
   
   \ new statement
   IF
      
      \ new statement
      start
      lang.StringIndexOutOfBoundsException§-945597696.table 1138905856 EXECUTE-NEW
      lang.JavaArray.handler§-1096259584
      
      \ new statement
      
      \ new statement
   ENDIF
   
   \ new statement
   end
   §this CELL+ @ 32 + ( java.lang.StringBuffer.count )    @
   >
   
   \ new statement
   IF
      
      \ new statement
      §this CELL+ @ 32 + ( java.lang.StringBuffer.count )       @
      TO end
      
      \ new statement
   ENDIF
   
   \ new statement
   start
   end
   >
   
   \ new statement
   IF
      
      \ new statement
      lang.StringIndexOutOfBoundsException§-945597696.table 1355436800 EXECUTE-NEW
      lang.JavaArray.handler§-1096259584
      
      \ new statement
      
      \ new statement
   ENDIF
   
   \ new statement
   end
   start
   -
   TO len
   
   \ new statement
   len
   0>
   
   \ new statement
   IF
      
      \ new statement
      
      \ new statement
      §this CELL+ @ 36 + ( java.lang.StringBuffer.shared )       @
      
      \ new statement
      IF
         
         \ new statement
         §this A:R@
         R> 1412784896 TRUE ( java.lang.StringBuffer.copy§1412784896 ) EXECUTE-METHOD
         
         \ new statement
      ENDIF
      
      \ new statement
      §this CELL+ @ 28 + ( java.lang.StringBuffer.value )       @
      start
      len
      +
      §this CELL+ @ 28 + ( java.lang.StringBuffer.value )       @
      start
      §this CELL+ @ 32 + ( java.lang.StringBuffer.count )       @
      end
      -
      lang.System.arraycopy§1260740864
      
      \ new statement
      §this CELL+ @ 32 + ( java.lang.StringBuffer.count )       @
      len
      -
      §this CELL+ @ 32 + ( java.lang.StringBuffer.count )       !
      
      \ new statement
      0 §break18704 LABEL
      
      \ new statement
      
      \ new statement
      0>!
      IF
         10001H - §break18703 BRANCH
      ENDIF
      DROP
      
      \ new statement
   ENDIF
   
   \ new statement
   §this
   DUP 0 V!0 §break18703 BRANCH
   
   \ new statement
   0 §break18703 LABEL
   
   \ new statement
   §synchronized0 A:R@
   R> 1974299904 TRUE ( Object.monitorLeave ) EXECUTE-METHOD
   PURGE 1
   
   \ new statement
   
   
   
   §base0 SETVTOP
   PURGE 5
   
   \ new statement
   DROP
;
( *
* Removes the character at the specified position in this
* <code>StringBuffer</code> (shortening the <code>StringBuffer</code>
* by one character).
*
* @param       index  Index of character to remove
* @return      This string buffer.
* @exception   StringIndexOutOfBoundsException  if the <code>index</code>
*		    is negative or greater than or equal to
*		    <code>length()</code>.
* @since       1.2
)

:LOCAL lang.StringBuffer.deleteCharAt§1018258432
   1 VALLOCATE LOCAL §base0
   LOCAL index
   DUP 0 §base0 + V! LOCAL §this
   
   \ new statement
   
   \ new statement
   §this LOCAL §synchronized0
   DI
   §synchronized0 A:R@
   lang.StringBuffer._staticThread
   R> 1472163072 TRUE ( Object.monitorEnter ) EXECUTE-METHOD
   EI
   
   \ new statement
   index
   0<
   0=! or_19204 0BRANCH DROP
   index
   §this CELL+ @ 32 + ( java.lang.StringBuffer.count )    @
   < INVERT
   or_19204 LABEL
   
   \ new statement
   IF
      
      \ new statement
      lang.StringIndexOutOfBoundsException§-945597696.table 1355436800 EXECUTE-NEW
      lang.JavaArray.handler§-1096259584
      
      \ new statement
      
      \ new statement
   ENDIF
   
   \ new statement
   §this CELL+ @ 36 + ( java.lang.StringBuffer.shared )    @
   
   \ new statement
   IF
      
      \ new statement
      §this A:R@
      R> 1412784896 TRUE ( java.lang.StringBuffer.copy§1412784896 ) EXECUTE-METHOD
      
      \ new statement
   ENDIF
   
   \ new statement
   §this CELL+ @ 28 + ( java.lang.StringBuffer.value )    @
   index
   1
   +
   §this CELL+ @ 28 + ( java.lang.StringBuffer.value )    @
   index
   §this CELL+ @ 32 + ( java.lang.StringBuffer.count )    @
   index
   -
   1
   -
   lang.System.arraycopy§1260740864
   
   \ new statement
   §this CELL+ @ 32 + ( java.lang.StringBuffer.count )    @ A:R@ 1- R@ !
   A:DROP
   
   \ new statement
   §this
   DUP 0 V!0 §break18705 BRANCH
   
   \ new statement
   0 §break18705 LABEL
   
   \ new statement
   §synchronized0 A:R@
   R> 1974299904 TRUE ( Object.monitorLeave ) EXECUTE-METHOD
   PURGE 1
   
   \ new statement
   
   §base0 SETVTOP
   PURGE 3
   
   \ new statement
   DROP
;
( *
* Ensures that the capacity of the buffer is at least equal to the
* specified minimum.
* If the current capacity of this string buffer is less than the
* argument, then a new internal buffer is allocated with greater
* capacity. The new capacity is the larger of:
* <ul>
* <li>The <code>minimumCapacity</code> argument.
* <li>Twice the old capacity, plus <code>2</code>.
* </ul>
* If the <code>minimumCapacity</code> argument is nonpositive, this
* method takes no action and simply returns.
*
* @param   minimumCapacity   the minimum desired capacity.
)

:LOCAL lang.StringBuffer.ensureCapacity§-1584765696
   1 VALLOCATE LOCAL §base0
   LOCAL minimumCapacity
   DUP 0 §base0 + V! LOCAL §this
   
   \ new statement
   
   \ new statement
   §this LOCAL §synchronized0
   DI
   §synchronized0 A:R@
   lang.StringBuffer._staticThread
   R> 1472163072 TRUE ( Object.monitorEnter ) EXECUTE-METHOD
   EI
   
   \ new statement
   minimumCapacity
   §this CELL+ @ 28 + ( java.lang.StringBuffer.value ) @ CELL+ @ 28 + ( java.lang.JavaArray.length )    @
   >
   
   \ new statement
   IF
      
      \ new statement
      
      \ new statement
      §this A:R@
      minimumCapacity
      R> -1659607808 TRUE ( java.lang.StringBuffer.expandCapacity§-1659607808 ) EXECUTE-METHOD
      
      \ new statement
      0 §break18667 LABEL
      
      \ new statement
      
      \ new statement
      DROP
      
      \ new statement
   ENDIF
   
   \ new statement
   0 §break18666 LABEL
   
   \ new statement
   §synchronized0 A:R@
   R> 1974299904 TRUE ( Object.monitorLeave ) EXECUTE-METHOD
   PURGE 1
   
   \ new statement
   
   §base0 SETVTOP
   PURGE 3
   
   \ new statement
   DROP
;
( *
* This implements the expansion semantics of ensureCapacity but is
* unsynchronized for use internally by methods which are already
* synchronized.
*
* @see java.lang.StringBuffer#ensureCapacity(int)
)

:LOCAL lang.StringBuffer.expandCapacity§-1659607808
   2 VALLOCATE LOCAL §base0
   LOCAL minimumCapacity
   DUP 0 §base0 + V! LOCAL §this
   
   \ new statement
   0 DUP
   LOCALS newCapacity newValue |
   
   \ new statement
   §this CELL+ @ 28 + ( java.lang.StringBuffer.value ) @ CELL+ @ 28 + ( java.lang.JavaArray.length )    @
   1
   +
   2*
   TO newCapacity
   
   \ new statement
   newCapacity
   0<
   
   \ new statement
   IF
      
      \ new statement
      
      \ new statement
      lang.Integer.MAX_VALUE
      @
      TO newCapacity
      
      \ new statement
      0 §break18669 LABEL
      
      \ new statement
      
      \ new statement
      DROP
      
      \ new statement
   ELSE
      
      \ new statement
      minimumCapacity
      newCapacity
      >
      
      \ new statement
      IF
         
         \ new statement
         
         \ new statement
         minimumCapacity
         TO newCapacity
         
         \ new statement
         0 §break18670 LABEL
         
         \ new statement
         
         \ new statement
         DROP
         
         \ new statement
      ENDIF
      
      \ new statement
   ENDIF
   
   \ new statement
   
   1 VALLOCATE LOCAL §tempvar
   newCapacity 131073 lang.JavaArray§1352878592.table -227194368 EXECUTE-NEW
   DUP 4 §base0 + V! TO newValue
   
   §tempvar SETVTOP
   
   PURGE 1
   
   
   \ new statement
   §this CELL+ @ 28 + ( java.lang.StringBuffer.value )    @
   0
   newValue
   0
   §this CELL+ @ 32 + ( java.lang.StringBuffer.count )    @
   lang.System.arraycopy§1260740864
   
   \ new statement
   newValue
   §this CELL+ @ 28 + ( java.lang.StringBuffer.value )    OVER 0 V! !
   
   \ new statement
   FALSE
   §this CELL+ @ 36 + ( java.lang.StringBuffer.shared )    !
   
   \ new statement
   0 §break18668 LABEL
   
   \ new statement
   
   
   
   §base0 SETVTOP
   PURGE 5
   
   \ new statement
   DROP
;
( *
* Characters are copied from this string buffer into the
* destination character array <code>dst</code>. The first character to
* be copied is at index <code>srcBegin</code>; the last character to
* be copied is at index <code>srcEnd-1</code>. The total number of
* characters to be copied is <code>srcEnd-srcBegin</code>. The
* characters are copied into the subarray of <code>dst</code> starting
* at index <code>dstBegin</code> and ending at index:
* <p><blockquote><pre>
* dstbegin + (srcEnd-srcBegin) - 1
* </pre></blockquote>
*
* @param      srcBegin   start copying at this offset in the string buffer.
* @param      srcEnd     stop copying at this offset in the string buffer.
* @param      dst        the array to copy the data into.
* @param      dstBegin   offset into <code>dst</code>.
* @exception  NullPointerException if <code>dst</code> is
*             <code>null</code>.
* @exception  IndexOutOfBoundsException  if any of the following is true:
*             <ul>
*             <li><code>srcBegin</code> is negative
*             <li><code>dstBegin</code> is negative
*             <li>the <code>srcBegin</code> argument is greater than
*             the <code>srcEnd</code> argument.
*             <li><code>srcEnd</code> is greater than
*             <code>this.length()</code>, the current length of this
*             string buffer.
*             <li><code>dstBegin+srcEnd-srcBegin</code> is greater than
*             <code>dst.length</code>
*             </ul>
)

:LOCAL lang.StringBuffer.getChars§-53713152
   2 VALLOCATE LOCAL §base0
   LOCAL dstBegin
   DUP 0 §base0 + V! LOCAL dst
   LOCAL srcEnd
   LOCAL srcBegin
   DUP 4 §base0 + V! LOCAL §this
   
   \ new statement
   
   \ new statement
   §this LOCAL §synchronized0
   DI
   §synchronized0 A:R@
   lang.StringBuffer._staticThread
   R> 1472163072 TRUE ( Object.monitorEnter ) EXECUTE-METHOD
   EI
   
   \ new statement
   srcBegin
   0<
   
   \ new statement
   IF
      
      \ new statement
      
      \ new statement
      srcBegin
      lang.StringIndexOutOfBoundsException§-945597696.table 1138905856 EXECUTE-NEW
      lang.JavaArray.handler§-1096259584
      
      \ new statement
      
      \ new statement
      0 §break18684 LABEL
      
      \ new statement
      
      \ new statement
      0>!
      IF
         10001H - §break18683 BRANCH
      ENDIF
      DROP
      
      \ new statement
   ENDIF
   
   \ new statement
   srcEnd
   0<
   0=! or_19272 0BRANCH DROP
   srcEnd
   §this CELL+ @ 32 + ( java.lang.StringBuffer.count )    @
   >
   or_19272 LABEL
   
   \ new statement
   IF
      
      \ new statement
      
      \ new statement
      srcEnd
      lang.StringIndexOutOfBoundsException§-945597696.table 1138905856 EXECUTE-NEW
      lang.JavaArray.handler§-1096259584
      
      \ new statement
      
      \ new statement
      0 §break18685 LABEL
      
      \ new statement
      
      \ new statement
      0>!
      IF
         10001H - §break18683 BRANCH
      ENDIF
      DROP
      
      \ new statement
   ENDIF
   
   \ new statement
   srcBegin
   srcEnd
   >
   
   \ new statement
   IF
      
      \ new statement
      
      \ new statement
      
      1 VALLOCATE LOCAL §tempvar
      U" srcBegin > srcEnd " COUNT lang.JavaArray.createUnicode§-675323136 DUP §tempvar V!
      lang.StringIndexOutOfBoundsException§-945597696.table 1858294528 EXECUTE-NEW
      lang.JavaArray.handler§-1096259584
      
      §tempvar SETVTOP
      
      PURGE 1
      
      
      \ new statement
      
      \ new statement
      0 §break18686 LABEL
      
      \ new statement
      
      \ new statement
      0>!
      IF
         10001H - §break18683 BRANCH
      ENDIF
      DROP
      
      \ new statement
   ENDIF
   
   \ new statement
   §this CELL+ @ 28 + ( java.lang.StringBuffer.value )    @
   srcBegin
   dst
   dstBegin
   srcEnd
   srcBegin
   -
   lang.System.arraycopy§1260740864
   
   \ new statement
   0 §break18683 LABEL
   
   \ new statement
   §synchronized0 A:R@
   R> 1974299904 TRUE ( Object.monitorLeave ) EXECUTE-METHOD
   PURGE 1
   
   \ new statement
   
   
   
   
   §base0 SETVTOP
   PURGE 6
   
   \ new statement
   DROP
;


:LOCAL lang.StringBuffer.getValue§551249664
   1 VALLOCATE LOCAL §base0
   DUP 0 §base0 + V! LOCAL §this
   
   \ new statement
   
   \ new statement
   §this CELL+ @ 28 + ( java.lang.StringBuffer.value )    @
   DUP 0 V!0 §break18732 BRANCH
   
   \ new statement
   0 §break18732 LABEL
   
   \ new statement
   §base0 SETVTOP
   PURGE 2
   
   \ new statement
   DROP
;
( *
* Returns the index within this string of the first occurrence of the
* specified substring. The integer returned is the smallest value
* <i>k</i> such that:
* <blockquote><pre>
* this.toString().startsWith(str, <i>k</i>)
* </pre></blockquote>
* is <code>true</code>.
*
* @param   str   any string.
* @return  if the string argument occurs as a substring within this
*          object, then the index of the first character of the first
*          such substring is returned; if it does not occur as a
*          substring, <code>-1</code> is returned.
* @exception java.lang.NullPointerException if <code>str</code> is
*          <code>null</code>.
* @since   1.4
)

:LOCAL lang.StringBuffer.indexOf§-337549056
   2 VALLOCATE LOCAL §base0
   DUP 0 §base0 + V! LOCAL str
   DUP 4 §base0 + V! LOCAL §this
   
   \ new statement
   
   \ new statement
   §this A:R@
   str
   0
   R> 517761280 TRUE ( java.lang.StringBuffer.indexOf§517761280 ) EXECUTE-METHOD
   0 §break18723 BRANCH
   
   \ new statement
   0 §break18723 LABEL
   
   \ new statement
   
   §base0 SETVTOP
   PURGE 3
   
   \ new statement
   DROP
;
( *
* Returns the index within this string of the first occurrence of the
* specified substring, starting at the specified index.  The integer
* returned is the smallest value <tt>k</tt> for which:
* <blockquote><pre>
*     k >= Math.min(fromIndex, str.length()) &&
*                   this.toString().startsWith(str, k)
* </pre></blockquote>
* If no such value of <i>k</i> exists, then -1 is returned.
*
* @param   str         the substring for which to search.
* @param   fromIndex   the index from which to start the search.
* @return  the index within this string of the first occurrence of the
*          specified substring, starting at the specified index.
* @exception java.lang.NullPointerException if <code>str</code> is
*            <code>null</code>.
* @since   1.4
)

:LOCAL lang.StringBuffer.indexOf§517761280
   2 VALLOCATE LOCAL §base0
   LOCAL fromIndex
   DUP 0 §base0 + V! LOCAL str
   DUP 4 §base0 + V! LOCAL §this
   
   \ new statement
   
   \ new statement
   §this LOCAL §synchronized0
   DI
   §synchronized0 A:R@
   lang.StringBuffer._staticThread
   R> 1472163072 TRUE ( Object.monitorEnter ) EXECUTE-METHOD
   EI
   
   \ new statement
   
   1 VALLOCATE LOCAL §tempvar
   §this CELL+ @ 28 + ( java.lang.StringBuffer.value )    @
   0
   §this CELL+ @ 32 + ( java.lang.StringBuffer.count )    @
   str A:R@
   R> 629044224 TRUE ( java.lang.String.toCharArray§629044224 ) EXECUTE-METHOD DUP §tempvar V!
   0
   str A:R@
   R> 188050432 TRUE ( java.lang.String.length§188050432 ) EXECUTE-METHOD
   fromIndex
   lang.String.indexOf§-412522240
   0 §break18724 BRANCH
   
   
   §tempvar SETVTOP
   
   PURGE 1
   
   
   \ new statement
   0 §break18724 LABEL
   
   \ new statement
   §synchronized0 A:R@
   R> 1974299904 TRUE ( Object.monitorLeave ) EXECUTE-METHOD
   PURGE 1
   
   \ new statement
   
   
   §base0 SETVTOP
   PURGE 4
   
   \ new statement
   DROP
;
( *
* Inserts the string representation of the <code>char</code>
* argument into this string buffer.
* <p>
* The second argument is inserted into the contents of this string
* buffer at the position indicated by <code>offset</code>. The length
* of this string buffer increases by one.
* <p>
* The overall effect is exactly as if the argument were converted to
* a string by the method {@link String#valueOf(char)} and the character
* in that string were then {@link #insert(int, String) inserted} into
* this <code>StringBuffer</code> object at the position indicated by
* <code>offset</code>.
* <p>
* The offset argument must be greater than or equal to
* <code>0</code>, and less than or equal to the length of this
* string buffer.
*
* @param      offset   the offset.
* @param      c        a <code>char</code>.
* @return     a reference to this <code>StringBuffer</code> object.
* @exception  IndexOutOfBoundsException  if the offset is invalid.
* @see        java.lang.StringBuffer#length()
)

:LOCAL lang.StringBuffer.insert§-1255511808
   1 VALLOCATE LOCAL §base0
   LOCAL c
   LOCAL offset
   DUP 0 §base0 + V! LOCAL §this
   
   \ new statement
   0
   LOCALS newcount |
   
   \ new statement
   §this LOCAL §synchronized0
   DI
   §synchronized0 A:R@
   lang.StringBuffer._staticThread
   R> 1472163072 TRUE ( Object.monitorEnter ) EXECUTE-METHOD
   EI
   
   \ new statement
   §this CELL+ @ 32 + ( java.lang.StringBuffer.count )    @
   1
   +
   TO newcount
   
   \ new statement
   newcount
   §this CELL+ @ 28 + ( java.lang.StringBuffer.value ) @ CELL+ @ 28 + ( java.lang.JavaArray.length )    @
   >
   
   \ new statement
   IF
      
      \ new statement
      §this A:R@
      newcount
      R> -1659607808 TRUE ( java.lang.StringBuffer.expandCapacity§-1659607808 ) EXECUTE-METHOD
      
      \ new statement
   ELSE
      
      \ new statement
      §this CELL+ @ 36 + ( java.lang.StringBuffer.shared )       @
      
      \ new statement
      IF
         
         \ new statement
         §this A:R@
         R> 1412784896 TRUE ( java.lang.StringBuffer.copy§1412784896 ) EXECUTE-METHOD
         
         \ new statement
      ENDIF
      
      \ new statement
   ENDIF
   
   \ new statement
   §this CELL+ @ 28 + ( java.lang.StringBuffer.value )    @
   offset
   §this CELL+ @ 28 + ( java.lang.StringBuffer.value )    @
   offset
   1
   +
   §this CELL+ @ 32 + ( java.lang.StringBuffer.count )    @
   offset
   -
   lang.System.arraycopy§1260740864
   
   \ new statement
   
   0
   LOCALS 0§ |
   
   offset TO 0§
   c
   §this CELL+ @ 28 + ( java.lang.StringBuffer.value ) @ A:R@
   0§
   R> -942971136 TRUE ( java.lang.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
   H!
   
   PURGE 1
   
   
   \ new statement
   newcount
   §this CELL+ @ 32 + ( java.lang.StringBuffer.count )    !
   
   \ new statement
   §this
   DUP 0 V!0 §break18718 BRANCH
   
   \ new statement
   0 §break18718 LABEL
   
   \ new statement
   §synchronized0 A:R@
   R> 1974299904 TRUE ( Object.monitorLeave ) EXECUTE-METHOD
   PURGE 1
   
   \ new statement
   
   
   
   §base0 SETVTOP
   PURGE 5
   
   \ new statement
   DROP
;
( *
* Inserts the string representation of the <code>char</code> array
* argument into this string buffer.
* <p>
* The characters of the array argument are inserted into the
* contents of this string buffer at the position indicated by
* <code>offset</code>. The length of this string buffer increases by
* the length of the argument.
* <p>
* The overall effect is exactly as if the argument were converted to
* a string by the method {@link String#valueOf(char[])} and the
* characters of that string were then
* {@link #insert(int,String) inserted} into this
* <code>StringBuffer</code>  object at the position indicated by
* <code>offset</code>.
*
* @param      offset   the offset.
* @param      str      a character array.
* @return     a reference to this <code>StringBuffer</code> object.
* @exception  StringIndexOutOfBoundsException  if the offset is invalid.
)

:LOCAL lang.StringBuffer.insert§-1532794624
   2 VALLOCATE LOCAL §base0
   DUP 0 §base0 + V! LOCAL str
   LOCAL offset
   DUP 4 §base0 + V! LOCAL §this
   
   \ new statement
   0 DUP
   LOCALS len newcount |
   
   \ new statement
   §this LOCAL §synchronized0
   DI
   §synchronized0 A:R@
   lang.StringBuffer._staticThread
   R> 1472163072 TRUE ( Object.monitorEnter ) EXECUTE-METHOD
   EI
   
   \ new statement
   offset
   0<
   0=! or_19335 0BRANCH DROP
   offset
   §this CELL+ @ 32 + ( java.lang.StringBuffer.count )    @
   >
   or_19335 LABEL
   
   \ new statement
   IF
      
      \ new statement
      
      \ new statement
      lang.StringIndexOutOfBoundsException§-945597696.table 1355436800 EXECUTE-NEW
      lang.JavaArray.handler§-1096259584
      
      \ new statement
      
      \ new statement
      0 §break18716 LABEL
      
      \ new statement
      
      \ new statement
      0>!
      IF
         10001H - §break18715 BRANCH
      ENDIF
      DROP
      
      \ new statement
   ENDIF
   
   \ new statement
   str CELL+ @ 28 + ( java.lang.JavaArray.length )    @
   TO len
   
   \ new statement
   §this CELL+ @ 32 + ( java.lang.StringBuffer.count )    @
   len
   +
   TO newcount
   
   \ new statement
   newcount
   §this CELL+ @ 28 + ( java.lang.StringBuffer.value ) @ CELL+ @ 28 + ( java.lang.JavaArray.length )    @
   >
   
   \ new statement
   IF
      
      \ new statement
      §this A:R@
      newcount
      R> -1659607808 TRUE ( java.lang.StringBuffer.expandCapacity§-1659607808 ) EXECUTE-METHOD
      
      \ new statement
   ELSE
      
      \ new statement
      §this CELL+ @ 36 + ( java.lang.StringBuffer.shared )       @
      
      \ new statement
      IF
         
         \ new statement
         §this A:R@
         R> 1412784896 TRUE ( java.lang.StringBuffer.copy§1412784896 ) EXECUTE-METHOD
         
         \ new statement
      ENDIF
      
      \ new statement
   ENDIF
   
   \ new statement
   §this CELL+ @ 28 + ( java.lang.StringBuffer.value )    @
   offset
   §this CELL+ @ 28 + ( java.lang.StringBuffer.value )    @
   offset
   len
   +
   §this CELL+ @ 32 + ( java.lang.StringBuffer.count )    @
   offset
   -
   lang.System.arraycopy§1260740864
   
   \ new statement
   str
   0
   §this CELL+ @ 28 + ( java.lang.StringBuffer.value )    @
   offset
   len
   lang.System.arraycopy§1260740864
   
   \ new statement
   newcount
   §this CELL+ @ 32 + ( java.lang.StringBuffer.count )    !
   
   \ new statement
   §this
   DUP 0 V!0 §break18715 BRANCH
   
   \ new statement
   0 §break18715 LABEL
   
   \ new statement
   §synchronized0 A:R@
   R> 1974299904 TRUE ( Object.monitorLeave ) EXECUTE-METHOD
   PURGE 1
   
   \ new statement
   
   
   
   
   §base0 SETVTOP
   PURGE 6
   
   \ new statement
   DROP
;
( *
* Inserts the string into this string buffer.
* <p>
* The characters of the <code>String</code> argument are inserted, in
* order, into this string buffer at the indicated offset, moving up any
* characters originally above that position and increasing the length
* of this string buffer by the length of the argument. If
* <code>str</code> is <code>null</code>, then the four characters
* <code>"null"</code> are inserted into this string buffer.
* <p>
* The character at index <i>k</i> in the new character sequence is
* equal to:
* <ul>
* <li>the character at index <i>k</i> in the old character sequence, if
* <i>k</i> is less than <code>offset</code>
* <li>the character at index <i>k</i><code>-offset</code> in the
* argument <code>str</code>, if <i>k</i> is not less than
* <code>offset</code> but is less than <code>offset+str.length()</code>
* <li>the character at index <i>k</i><code>-str.length()</code> in the
* old character sequence, if <i>k</i> is not less than
* <code>offset+str.length()</code>
* </ul><p>
* The offset argument must be greater than or equal to
* <code>0</code>, and less than or equal to the length of this
* string buffer.
*
* @param      offset   the offset.
* @param      str      a string.
* @return     a reference to this <code>StringBuffer</code> object.
* @exception  StringIndexOutOfBoundsException  if the offset is invalid.
* @see        java.lang.StringBuffer#length()
)

:LOCAL lang.StringBuffer.insert§-1717409536
   2 VALLOCATE LOCAL §base0
   DUP 0 §base0 + V! LOCAL str
   LOCAL offset
   DUP 4 §base0 + V! LOCAL §this
   
   \ new statement
   0 DUP
   LOCALS len newcount |
   
   \ new statement
   §this LOCAL §synchronized0
   DI
   §synchronized0 A:R@
   lang.StringBuffer._staticThread
   R> 1472163072 TRUE ( Object.monitorEnter ) EXECUTE-METHOD
   EI
   
   \ new statement
   offset
   0<
   0=! or_19366 0BRANCH DROP
   offset
   §this CELL+ @ 32 + ( java.lang.StringBuffer.count )    @
   >
   or_19366 LABEL
   
   \ new statement
   IF
      
      \ new statement
      
      \ new statement
      lang.StringIndexOutOfBoundsException§-945597696.table 1355436800 EXECUTE-NEW
      lang.JavaArray.handler§-1096259584
      
      \ new statement
      
      \ new statement
      0 §break18713 LABEL
      
      \ new statement
      
      \ new statement
      0>!
      IF
         10001H - §break18712 BRANCH
      ENDIF
      DROP
      
      \ new statement
   ENDIF
   
   \ new statement
   str
   0=
   
   \ new statement
   IF
      
      \ new statement
      
      \ new statement
      str
      lang.String.valueOf§59340288
      DUP §base0 V! TO str
      
      \ new statement
      0 §break18714 LABEL
      
      \ new statement
      
      \ new statement
      0>!
      IF
         10001H - §break18712 BRANCH
      ENDIF
      DROP
      
      \ new statement
   ENDIF
   
   \ new statement
   str A:R@
   R> 188050432 TRUE ( java.lang.String.length§188050432 ) EXECUTE-METHOD
   TO len
   
   \ new statement
   §this CELL+ @ 32 + ( java.lang.StringBuffer.count )    @
   len
   +
   TO newcount
   
   \ new statement
   newcount
   §this CELL+ @ 28 + ( java.lang.StringBuffer.value ) @ CELL+ @ 28 + ( java.lang.JavaArray.length )    @
   >
   
   \ new statement
   IF
      
      \ new statement
      §this A:R@
      newcount
      R> -1659607808 TRUE ( java.lang.StringBuffer.expandCapacity§-1659607808 ) EXECUTE-METHOD
      
      \ new statement
   ELSE
      
      \ new statement
      §this CELL+ @ 36 + ( java.lang.StringBuffer.shared )       @
      
      \ new statement
      IF
         
         \ new statement
         §this A:R@
         R> 1412784896 TRUE ( java.lang.StringBuffer.copy§1412784896 ) EXECUTE-METHOD
         
         \ new statement
      ENDIF
      
      \ new statement
   ENDIF
   
   \ new statement
   §this CELL+ @ 28 + ( java.lang.StringBuffer.value )    @
   offset
   §this CELL+ @ 28 + ( java.lang.StringBuffer.value )    @
   offset
   len
   +
   §this CELL+ @ 32 + ( java.lang.StringBuffer.count )    @
   offset
   -
   lang.System.arraycopy§1260740864
   
   \ new statement
   str A:R@
   0
   len
   §this CELL+ @ 28 + ( java.lang.StringBuffer.value )    @
   offset
   R> -53713152 TRUE ( java.lang.String.getChars§-53713152 ) EXECUTE-METHOD
   
   \ new statement
   newcount
   §this CELL+ @ 32 + ( java.lang.StringBuffer.count )    !
   
   \ new statement
   §this
   DUP 0 V!0 §break18712 BRANCH
   
   \ new statement
   0 §break18712 LABEL
   
   \ new statement
   §synchronized0 A:R@
   R> 1974299904 TRUE ( Object.monitorLeave ) EXECUTE-METHOD
   PURGE 1
   
   \ new statement
   
   
   
   
   §base0 SETVTOP
   PURGE 6
   
   \ new statement
   DROP
;
( *
* Inserts the string representation of the <code>double</code>
* argument into this string buffer.
* <p>
* The second argument is converted to a string as if by the method
* <code>String.valueOf</code>, and the characters of that
* string are then inserted into this string buffer at the indicated
* offset.
* <p>
* The offset argument must be greater than or equal to
* <code>0</code>, and less than or equal to the length of this
* string buffer.
*
* @param      offset   the offset.
* @param      d        a <code>double</code>.
* @return     a reference to this <code>StringBuffer</code> object.
* @exception  StringIndexOutOfBoundsException  if the offset is invalid.
* @see        java.lang.String#valueOf(double)
* @see        java.lang.StringBuffer#insert(int, java.lang.String)
* @see        java.lang.StringBuffer#length()
)

:LOCAL lang.StringBuffer.insert§-1765316352
   1 VALLOCATE LOCAL §base0
   2LOCAL d
   LOCAL offset
   DUP 0 §base0 + V! LOCAL §this
   
   \ new statement
   
   \ new statement
   
   1 VALLOCATE LOCAL §tempvar
   §this A:R@
   offset
   d
   lang.String.valueOf§-721717760 DUP §tempvar V!
   R> -1717409536 TRUE ( java.lang.StringBuffer.insert§-1717409536 ) EXECUTE-METHOD
   DUP 0 V!0 §break18722 BRANCH
   
   
   §tempvar SETVTOP
   
   PURGE 1
   
   
   \ new statement
   0 §break18722 LABEL
   
   \ new statement
   
   
   §base0 SETVTOP
   PURGE 4
   
   \ new statement
   DROP
;
( *
* Inserts the string representation of the <code>float</code>
* argument into this string buffer.
* <p>
* The second argument is converted to a string as if by the method
* <code>String.valueOf</code>, and the characters of that
* string are then inserted into this string buffer at the indicated
* offset.
* <p>
* The offset argument must be greater than or equal to
* <code>0</code>, and less than or equal to the length of this
* string buffer.
*
* @param      offset   the offset.
* @param      f        a <code>float</code>.
* @return     a reference to this <code>StringBuffer</code> object.
* @exception  StringIndexOutOfBoundsException  if the offset is invalid.
* @see        java.lang.String#valueOf(float)
* @see        java.lang.StringBuffer#insert(int, java.lang.String)
* @see        java.lang.StringBuffer#length()
)

:LOCAL lang.StringBuffer.insert§-321427200
   1 VALLOCATE LOCAL §base0
   2LOCAL f
   LOCAL offset
   DUP 0 §base0 + V! LOCAL §this
   
   \ new statement
   
   \ new statement
   
   1 VALLOCATE LOCAL §tempvar
   §this A:R@
   offset
   f
   lang.String.valueOf§-721717760 DUP §tempvar V!
   R> -1717409536 TRUE ( java.lang.StringBuffer.insert§-1717409536 ) EXECUTE-METHOD
   DUP 0 V!0 §break18721 BRANCH
   
   
   §tempvar SETVTOP
   
   PURGE 1
   
   
   \ new statement
   0 §break18721 LABEL
   
   \ new statement
   
   
   §base0 SETVTOP
   PURGE 4
   
   \ new statement
   DROP
;
( *
* Inserts the string representation of the <code>long</code>
* argument into this string buffer.
* <p>
* The second argument is converted to a string as if by the method
* <code>String.valueOf</code>, and the characters of that
* string are then inserted into this string buffer at the position
* indicated by <code>offset</code>.
* <p>
* The offset argument must be greater than or equal to
* <code>0</code>, and less than or equal to the length of this
* string buffer.
*
* @param      offset   the offset.
* @param      l        a <code>long</code>.
* @return     a reference to this <code>StringBuffer</code> object.
* @exception  StringIndexOutOfBoundsException  if the offset is invalid.
* @see        java.lang.String#valueOf(long)
* @see        java.lang.StringBuffer#insert(int, java.lang.String)
* @see        java.lang.StringBuffer#length()
)

:LOCAL lang.StringBuffer.insert§1900505344
   1 VALLOCATE LOCAL §base0
   2LOCAL l
   LOCAL offset
   DUP 0 §base0 + V! LOCAL §this
   
   \ new statement
   
   \ new statement
   
   1 VALLOCATE LOCAL §tempvar
   §this A:R@
   offset
   l
   lang.String.valueOf§-85035520 DUP §tempvar V!
   R> -1717409536 TRUE ( java.lang.StringBuffer.insert§-1717409536 ) EXECUTE-METHOD
   DUP 0 V!0 §break18720 BRANCH
   
   
   §tempvar SETVTOP
   
   PURGE 1
   
   
   \ new statement
   0 §break18720 LABEL
   
   \ new statement
   
   
   §base0 SETVTOP
   PURGE 4
   
   \ new statement
   DROP
;
( *
* Inserts the string representation of a subarray of the <code>str</code>
* array argument into this string buffer. The subarray begins at the
* specified <code>offset</code> and extends <code>len</code> characters.
* The characters of the subarray are inserted into this string buffer at
* the position indicated by <code>index</code>. The length of this
* <code>StringBuffer</code> increases by <code>len</code> characters.
*
* @param      index    position at which to insert subarray.
* @param      str       A character array.
* @param      offset   the index of the first character in subarray to
*		   to be inserted.
* @param      len      the number of characters in the subarray to
*		   to be inserted.
* @return     This string buffer.
* @exception  StringIndexOutOfBoundsException  if <code>index</code>
*             is negative or greater than <code>length()</code>, or
*		   <code>offset</code> or <code>len</code> are negative, or
*		   <code>(offset+len)</code> is greater than
*		   <code>str.length</code>.
* @since 1.2
)

:LOCAL lang.StringBuffer.insert§2145478912
   2 VALLOCATE LOCAL §base0
   LOCAL len
   LOCAL offset
   DUP 0 §base0 + V! LOCAL str
   LOCAL index
   DUP 4 §base0 + V! LOCAL §this
   
   \ new statement
   0
   LOCALS newCount |
   
   \ new statement
   §this LOCAL §synchronized0
   DI
   §synchronized0 A:R@
   lang.StringBuffer._staticThread
   R> 1472163072 TRUE ( Object.monitorEnter ) EXECUTE-METHOD
   EI
   
   \ new statement
   index
   0<
   0=! or_19420 0BRANCH DROP
   index
   §this CELL+ @ 32 + ( java.lang.StringBuffer.count )    @
   >
   or_19420 LABEL
   
   \ new statement
   IF
      
      \ new statement
      lang.StringIndexOutOfBoundsException§-945597696.table 1355436800 EXECUTE-NEW
      lang.JavaArray.handler§-1096259584
      
      \ new statement
      
      \ new statement
   ENDIF
   
   \ new statement
   offset
   0<
   0=! or_19426 0BRANCH DROP
   offset
   len
   +
   0<
   0=! or_19426 0BRANCH DROP
   offset
   len
   +
   str CELL+ @ 28 + ( java.lang.JavaArray.length )    @
   >
   or_19426 LABEL
   
   \ new statement
   IF
      
      \ new statement
      offset
      lang.StringIndexOutOfBoundsException§-945597696.table 1138905856 EXECUTE-NEW
      lang.JavaArray.handler§-1096259584
      
      \ new statement
      
      \ new statement
   ENDIF
   
   \ new statement
   len
   0<
   
   \ new statement
   IF
      
      \ new statement
      len
      lang.StringIndexOutOfBoundsException§-945597696.table 1138905856 EXECUTE-NEW
      lang.JavaArray.handler§-1096259584
      
      \ new statement
      
      \ new statement
   ENDIF
   
   \ new statement
   §this CELL+ @ 32 + ( java.lang.StringBuffer.count )    @
   len
   +
   TO newCount
   
   \ new statement
   newCount
   §this CELL+ @ 28 + ( java.lang.StringBuffer.value ) @ CELL+ @ 28 + ( java.lang.JavaArray.length )    @
   >
   
   \ new statement
   IF
      
      \ new statement
      §this A:R@
      newCount
      R> -1659607808 TRUE ( java.lang.StringBuffer.expandCapacity§-1659607808 ) EXECUTE-METHOD
      
      \ new statement
   ELSE
      
      \ new statement
      §this CELL+ @ 36 + ( java.lang.StringBuffer.shared )       @
      
      \ new statement
      IF
         
         \ new statement
         §this A:R@
         R> 1412784896 TRUE ( java.lang.StringBuffer.copy§1412784896 ) EXECUTE-METHOD
         
         \ new statement
      ENDIF
      
      \ new statement
   ENDIF
   
   \ new statement
   §this CELL+ @ 28 + ( java.lang.StringBuffer.value )    @
   index
   §this CELL+ @ 28 + ( java.lang.StringBuffer.value )    @
   index
   len
   +
   §this CELL+ @ 32 + ( java.lang.StringBuffer.count )    @
   index
   -
   lang.System.arraycopy§1260740864
   
   \ new statement
   str
   offset
   §this CELL+ @ 28 + ( java.lang.StringBuffer.value )    @
   index
   len
   lang.System.arraycopy§1260740864
   
   \ new statement
   newCount
   §this CELL+ @ 32 + ( java.lang.StringBuffer.count )    !
   
   \ new statement
   §this
   DUP 0 V!0 §break18710 BRANCH
   
   \ new statement
   0 §break18710 LABEL
   
   \ new statement
   §synchronized0 A:R@
   R> 1974299904 TRUE ( Object.monitorLeave ) EXECUTE-METHOD
   PURGE 1
   
   \ new statement
   
   
   
   
   
   §base0 SETVTOP
   PURGE 7
   
   \ new statement
   DROP
;
( *
* Inserts the string representation of the <code>Object</code>
* argument into this string buffer.
* <p>
* The second argument is converted to a string as if by the method
* <code>String.valueOf</code>, and the characters of that
* string are then inserted into this string buffer at the indicated
* offset.
* <p>
* The offset argument must be greater than or equal to
* <code>0</code>, and less than or equal to the length of this
* string buffer.
*
* @param      offset   the offset.
* @param      obj      an <code>Object</code>.
* @return     a reference to this <code>StringBuffer</code> object.
* @exception  StringIndexOutOfBoundsException  if the offset is invalid.
* @see        java.lang.String#valueOf(java.lang.Object)
* @see        java.lang.StringBuffer#insert(int, java.lang.String)
* @see        java.lang.StringBuffer#length()
)

:LOCAL lang.StringBuffer.insert§570910976
   2 VALLOCATE LOCAL §base0
   DUP 0 §base0 + V! LOCAL obj
   LOCAL offset
   DUP 4 §base0 + V! LOCAL §this
   
   \ new statement
   
   \ new statement
   §this LOCAL §synchronized0
   DI
   §synchronized0 A:R@
   lang.StringBuffer._staticThread
   R> 1472163072 TRUE ( Object.monitorEnter ) EXECUTE-METHOD
   EI
   
   \ new statement
   
   1 VALLOCATE LOCAL §tempvar
   §this A:R@
   offset
   obj
   lang.String.valueOf§59340288 DUP §tempvar V!
   R> -1717409536 TRUE ( java.lang.StringBuffer.insert§-1717409536 ) EXECUTE-METHOD
   DUP 0 V!0 §break18711 BRANCH
   
   
   §tempvar SETVTOP
   
   PURGE 1
   
   
   \ new statement
   0 §break18711 LABEL
   
   \ new statement
   §synchronized0 A:R@
   R> 1974299904 TRUE ( Object.monitorLeave ) EXECUTE-METHOD
   PURGE 1
   
   \ new statement
   
   
   §base0 SETVTOP
   PURGE 4
   
   \ new statement
   DROP
;
( *
* Inserts the string representation of the second <code>int</code>
* argument into this string buffer.
* <p>
* The second argument is converted to a string as if by the method
* <code>String.valueOf</code>, and the characters of that
* string are then inserted into this string buffer at the indicated
* offset.
* <p>
* The offset argument must be greater than or equal to
* <code>0</code>, and less than or equal to the length of this
* string buffer.
*
* @param      offset   the offset.
* @param      i        an <code>int</code>.
* @return     a reference to this <code>StringBuffer</code> object.
* @exception  StringIndexOutOfBoundsException  if the offset is invalid.
* @see        java.lang.String#valueOf(int)
* @see        java.lang.StringBuffer#insert(int, java.lang.String)
* @see        java.lang.StringBuffer#length()
)

:LOCAL lang.StringBuffer.insert§795633920
   1 VALLOCATE LOCAL §base0
   LOCAL i
   LOCAL offset
   DUP 0 §base0 + V! LOCAL §this
   
   \ new statement
   
   \ new statement
   
   1 VALLOCATE LOCAL §tempvar
   §this A:R@
   offset
   i
   lang.String.valueOf§-166496768 DUP §tempvar V!
   R> -1717409536 TRUE ( java.lang.StringBuffer.insert§-1717409536 ) EXECUTE-METHOD
   DUP 0 V!0 §break18719 BRANCH
   
   
   §tempvar SETVTOP
   
   PURGE 1
   
   
   \ new statement
   0 §break18719 LABEL
   
   \ new statement
   
   
   §base0 SETVTOP
   PURGE 4
   
   \ new statement
   DROP
;
( *
* Inserts the string representation of the <code>boolean</code>
* argument into this string buffer.
* <p>
* The second argument is converted to a string as if by the method
* <code>String.valueOf</code>, and the characters of that
* string are then inserted into this string buffer at the indicated
* offset.
* <p>
* The offset argument must be greater than or equal to
* <code>0</code>, and less than or equal to the length of this
* string buffer.
*
* @param      offset   the offset.
* @param      b        a <code>boolean</code>.
* @return     a reference to this <code>StringBuffer</code> object.
* @exception  StringIndexOutOfBoundsException  if the offset is invalid.
* @see        java.lang.String#valueOf(boolean)
* @see        java.lang.StringBuffer#insert(int, java.lang.String)
* @see        java.lang.StringBuffer#length()
)

:LOCAL lang.StringBuffer.insert§900622592
   1 VALLOCATE LOCAL §base0
   LOCAL b
   LOCAL offset
   DUP 0 §base0 + V! LOCAL §this
   
   \ new statement
   
   \ new statement
   
   1 VALLOCATE LOCAL §tempvar
   §this A:R@
   offset
   b
   lang.String.valueOf§-838633984 DUP §tempvar V!
   R> -1717409536 TRUE ( java.lang.StringBuffer.insert§-1717409536 ) EXECUTE-METHOD
   DUP 0 V!0 §break18717 BRANCH
   
   
   §tempvar SETVTOP
   
   PURGE 1
   
   
   \ new statement
   0 §break18717 LABEL
   
   \ new statement
   
   
   §base0 SETVTOP
   PURGE 4
   
   \ new statement
   DROP
;
( *
* Returns the index within this string of the rightmost occurrence
* of the specified substring.  The rightmost empty string "" is
* considered to occur at the index value <code>this.length()</code>.
* The returned index is the largest value <i>k</i> such that
* <blockquote><pre>
* this.toString().startsWith(str, k)
* </pre></blockquote>
* is true.
*
* @param   str   the substring to search for.
* @return  if the string argument occurs one or more times as a substring
*          within this object, then the index of the first character of
*          the last such substring is returned. If it does not occur as
*          a substring, <code>-1</code> is returned.
* @exception java.lang.NullPointerException  if <code>str</code> is
*          <code>null</code>.
* @since   1.4
)

:LOCAL lang.StringBuffer.lastIndexOf§-1299878912
   2 VALLOCATE LOCAL §base0
   DUP 0 §base0 + V! LOCAL str
   DUP 4 §base0 + V! LOCAL §this
   
   \ new statement
   
   \ new statement
   §this LOCAL §synchronized0
   DI
   §synchronized0 A:R@
   lang.StringBuffer._staticThread
   R> 1472163072 TRUE ( Object.monitorEnter ) EXECUTE-METHOD
   EI
   
   \ new statement
   §this A:R@
   str
   §this CELL+ @ 32 + ( java.lang.StringBuffer.count )    @
   R> -465605632 TRUE ( java.lang.StringBuffer.lastIndexOf§-465605632 ) EXECUTE-METHOD
   0 §break18725 BRANCH
   
   \ new statement
   0 §break18725 LABEL
   
   \ new statement
   §synchronized0 A:R@
   R> 1974299904 TRUE ( Object.monitorLeave ) EXECUTE-METHOD
   PURGE 1
   
   \ new statement
   
   §base0 SETVTOP
   PURGE 3
   
   \ new statement
   DROP
;
( *
* Returns the index within this string of the last occurrence of the
* specified substring. The integer returned is the largest value <i>k</i>
* such that:
* <blockquote><pre>
*     k <= Math.min(fromIndex, str.length()) &&
*                   this.toString().startsWith(str, k)
* </pre></blockquote>
* If no such value of <i>k</i> exists, then -1 is returned.
*
* @param   str         the substring to search for.
* @param   fromIndex   the index to start the search from.
* @return  the index within this string of the last occurrence of the
*          specified substring.
* @exception java.lang.NullPointerException if <code>str</code> is
*          <code>null</code>.
* @since   1.4
)

:LOCAL lang.StringBuffer.lastIndexOf§-465605632
   2 VALLOCATE LOCAL §base0
   LOCAL fromIndex
   DUP 0 §base0 + V! LOCAL str
   DUP 4 §base0 + V! LOCAL §this
   
   \ new statement
   
   \ new statement
   §this LOCAL §synchronized0
   DI
   §synchronized0 A:R@
   lang.StringBuffer._staticThread
   R> 1472163072 TRUE ( Object.monitorEnter ) EXECUTE-METHOD
   EI
   
   \ new statement
   
   1 VALLOCATE LOCAL §tempvar
   §this CELL+ @ 28 + ( java.lang.StringBuffer.value )    @
   0
   §this CELL+ @ 32 + ( java.lang.StringBuffer.count )    @
   str A:R@
   R> 629044224 TRUE ( java.lang.String.toCharArray§629044224 ) EXECUTE-METHOD DUP §tempvar V!
   0
   str A:R@
   R> 188050432 TRUE ( java.lang.String.length§188050432 ) EXECUTE-METHOD
   fromIndex
   lang.String.lastIndexOf§831351808
   0 §break18726 BRANCH
   
   
   §tempvar SETVTOP
   
   PURGE 1
   
   
   \ new statement
   0 §break18726 LABEL
   
   \ new statement
   §synchronized0 A:R@
   R> 1974299904 TRUE ( Object.monitorLeave ) EXECUTE-METHOD
   PURGE 1
   
   \ new statement
   
   
   §base0 SETVTOP
   PURGE 4
   
   \ new statement
   DROP
;
( *
* Returns the length (character count) of this string buffer.
*
* @return  the length of the sequence of characters currently
*          represented by this string buffer.
)

:LOCAL lang.StringBuffer.length§188050432
   1 VALLOCATE LOCAL §base0
   DUP 0 §base0 + V! LOCAL §this
   
   \ new statement
   
   \ new statement
   §this LOCAL §synchronized0
   DI
   §synchronized0 A:R@
   lang.StringBuffer._staticThread
   R> 1472163072 TRUE ( Object.monitorEnter ) EXECUTE-METHOD
   EI
   
   \ new statement
   §this CELL+ @ 32 + ( java.lang.StringBuffer.count )    @
   0 §break18663 BRANCH
   
   \ new statement
   0 §break18663 LABEL
   
   \ new statement
   §synchronized0 A:R@
   R> 1974299904 TRUE ( Object.monitorLeave ) EXECUTE-METHOD
   PURGE 1
   
   \ new statement
   §base0 SETVTOP
   PURGE 2
   
   \ new statement
   DROP
;
( *
* Replaces the characters in a substring of this <code>StringBuffer</code>
* with characters in the specified <code>String</code>. The substring
* begins at the specified <code>start</code> and extends to the character
* at index <code>end - 1</code> or to the end of the
* <code>StringBuffer</code> if no such character exists. First the
* characters in the substring are removed and then the specified
* <code>String</code> is inserted at <code>start</code>. (The
* <code>StringBuffer</code> will be lengthened to accommodate the
* specified String if necessary.)
*
* @param      start    The beginning index, inclusive.
* @param      end      The ending index, exclusive.
* @param      str   String that will replace previous contents.
* @return     This string buffer.
* @exception  StringIndexOutOfBoundsException  if <code>start</code>
*             is negative, greater than <code>length()</code>, or
*		   greater than <code>end</code>.
* @since      1.2
)

:LOCAL lang.StringBuffer.replace§948597248
   2 VALLOCATE LOCAL §base0
   DUP 0 §base0 + V! LOCAL str
   LOCAL end
   LOCAL start
   DUP 4 §base0 + V! LOCAL §this
   
   \ new statement
   0 DUP
   LOCALS len newCount |
   
   \ new statement
   §this LOCAL §synchronized0
   DI
   §synchronized0 A:R@
   lang.StringBuffer._staticThread
   R> 1472163072 TRUE ( Object.monitorEnter ) EXECUTE-METHOD
   EI
   
   \ new statement
   start
   0<
   
   \ new statement
   IF
      
      \ new statement
      start
      lang.StringIndexOutOfBoundsException§-945597696.table 1138905856 EXECUTE-NEW
      lang.JavaArray.handler§-1096259584
      
      \ new statement
      
      \ new statement
   ENDIF
   
   \ new statement
   end
   §this CELL+ @ 32 + ( java.lang.StringBuffer.count )    @
   >
   
   \ new statement
   IF
      
      \ new statement
      §this CELL+ @ 32 + ( java.lang.StringBuffer.count )       @
      TO end
      
      \ new statement
   ENDIF
   
   \ new statement
   start
   end
   >
   
   \ new statement
   IF
      
      \ new statement
      lang.StringIndexOutOfBoundsException§-945597696.table 1355436800 EXECUTE-NEW
      lang.JavaArray.handler§-1096259584
      
      \ new statement
      
      \ new statement
   ENDIF
   
   \ new statement
   str A:R@
   R> 188050432 TRUE ( java.lang.String.length§188050432 ) EXECUTE-METHOD
   TO len
   
   \ new statement
   §this CELL+ @ 32 + ( java.lang.StringBuffer.count )    @
   len
   +
   end
   start
   -
   -
   TO newCount
   
   \ new statement
   newCount
   §this CELL+ @ 28 + ( java.lang.StringBuffer.value ) @ CELL+ @ 28 + ( java.lang.JavaArray.length )    @
   >
   
   \ new statement
   IF
      
      \ new statement
      §this A:R@
      newCount
      R> -1659607808 TRUE ( java.lang.StringBuffer.expandCapacity§-1659607808 ) EXECUTE-METHOD
      
      \ new statement
   ELSE
      
      \ new statement
      §this CELL+ @ 36 + ( java.lang.StringBuffer.shared )       @
      
      \ new statement
      IF
         
         \ new statement
         §this A:R@
         R> 1412784896 TRUE ( java.lang.StringBuffer.copy§1412784896 ) EXECUTE-METHOD
         
         \ new statement
      ENDIF
      
      \ new statement
   ENDIF
   
   \ new statement
   §this CELL+ @ 28 + ( java.lang.StringBuffer.value )    @
   end
   §this CELL+ @ 28 + ( java.lang.StringBuffer.value )    @
   start
   len
   +
   §this CELL+ @ 32 + ( java.lang.StringBuffer.count )    @
   end
   -
   lang.System.arraycopy§1260740864
   
   \ new statement
   str A:R@
   0
   len
   §this CELL+ @ 28 + ( java.lang.StringBuffer.value )    @
   start
   R> -53713152 TRUE ( java.lang.String.getChars§-53713152 ) EXECUTE-METHOD
   
   \ new statement
   newCount
   §this CELL+ @ 32 + ( java.lang.StringBuffer.count )    !
   
   \ new statement
   §this
   DUP 0 V!0 §break18706 BRANCH
   
   \ new statement
   0 §break18706 LABEL
   
   \ new statement
   §synchronized0 A:R@
   R> 1974299904 TRUE ( Object.monitorLeave ) EXECUTE-METHOD
   PURGE 1
   
   \ new statement
   
   
   
   
   
   §base0 SETVTOP
   PURGE 7
   
   \ new statement
   DROP
;
( *
* The character sequence contained in this string buffer is
* replaced by the reverse of the sequence.
* <p>
* Let <i>n</i> be the length of the old character sequence, the one
* contained in the string buffer just prior to execution of the
* <code>reverse</code> method. Then the character at index <i>k</i> in
* the new character sequence is equal to the character at index
* <i>n-k-1</i> in the old character sequence.
*
* @return  a reference to this <code>StringBuffer</code> object.
* @since   JDK1.0.2
)

:LOCAL lang.StringBuffer.reverse§-1963298304
   1 VALLOCATE LOCAL §base0
   DUP 0 §base0 + V! LOCAL §this
   
   \ new statement
   0
   LOCALS n |
   
   \ new statement
   §this LOCAL §synchronized0
   DI
   §synchronized0 A:R@
   lang.StringBuffer._staticThread
   R> 1472163072 TRUE ( Object.monitorEnter ) EXECUTE-METHOD
   EI
   
   \ new statement
   §this CELL+ @ 36 + ( java.lang.StringBuffer.shared )    @
   
   \ new statement
   IF
      
      \ new statement
      §this A:R@
      R> 1412784896 TRUE ( java.lang.StringBuffer.copy§1412784896 ) EXECUTE-METHOD
      
      \ new statement
   ENDIF
   
   \ new statement
   §this CELL+ @ 32 + ( java.lang.StringBuffer.count )    @
   1
   -
   TO n
   
   \ new statement
   0 DUP
   LOCALS j temp |
   
   \ new statement
   n
   1
   -
   S>D 2/
   TO j
   
   \ new statement
   BEGIN
      
      \ new statement
      j
      0< INVERT
      
      \ new statement
   WHILE
      
      \ new statement
      §this CELL+ @ 28 + ( java.lang.StringBuffer.value ) @ A:R@
      j
      R> -942971136 TRUE ( java.lang.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
      H@
      TO temp
      
      \ new statement
      
      0
      LOCALS 0§ |
      
      j TO 0§
      §this CELL+ @ 28 + ( java.lang.StringBuffer.value ) @ A:R@
      n
      j
      -
      R> -942971136 TRUE ( java.lang.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
      H@
      §this CELL+ @ 28 + ( java.lang.StringBuffer.value ) @ A:R@
      0§
      R> -942971136 TRUE ( java.lang.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
      H!
      
      PURGE 1
      
      
      \ new statement
      
      0
      LOCALS 0§ |
      
      n
      j
      - TO 0§
      temp
      §this CELL+ @ 28 + ( java.lang.StringBuffer.value ) @ A:R@
      0§
      R> -942971136 TRUE ( java.lang.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
      H!
      
      PURGE 1
      
      
      \ new statement
      
      \ new statement
      j 1- TO j
      
      \ new statement
   REPEAT
   
   \ new statement
   0 §break18728 LABEL
   
   \ new statement
   
   
   PURGE 2
   
   \ new statement
   DROP
   
   \ new statement
   §this
   DUP 0 V!0 §break18727 BRANCH
   
   \ new statement
   0 §break18727 LABEL
   
   \ new statement
   §synchronized0 A:R@
   R> 1974299904 TRUE ( Object.monitorLeave ) EXECUTE-METHOD
   PURGE 1
   
   \ new statement
   
   §base0 SETVTOP
   PURGE 3
   
   \ new statement
   DROP
;
( *
* The character at the specified index of this string buffer is set
* to <code>ch</code>. The string buffer is altered to represent a new
* character sequence that is identical to the old character sequence,
* except that it contains the character <code>ch</code> at position
* <code>index</code>.
* <p>
* The index argument must be greater than or equal to
* <code>0</code>, and less than the length of this string buffer.
*
* @param      index   the index of the character to modify.
* @param      ch      the new character.
* @exception  IndexOutOfBoundsException  if <code>index</code> is
*             negative or greater than or equal to <code>length()</code>.
* @see        java.lang.StringBuffer#length()
)

:LOCAL lang.StringBuffer.setCharAt§-497847552
   1 VALLOCATE LOCAL §base0
   LOCAL ch
   LOCAL index
   DUP 0 §base0 + V! LOCAL §this
   
   \ new statement
   
   \ new statement
   §this LOCAL §synchronized0
   DI
   §synchronized0 A:R@
   lang.StringBuffer._staticThread
   R> 1472163072 TRUE ( Object.monitorEnter ) EXECUTE-METHOD
   EI
   
   \ new statement
   index
   0<
   0=! or_19556 0BRANCH DROP
   index
   §this CELL+ @ 32 + ( java.lang.StringBuffer.count )    @
   < INVERT
   or_19556 LABEL
   
   \ new statement
   IF
      
      \ new statement
      
      \ new statement
      index
      lang.StringIndexOutOfBoundsException§-945597696.table 1138905856 EXECUTE-NEW
      lang.JavaArray.handler§-1096259584
      
      \ new statement
      
      \ new statement
      0 §break18688 LABEL
      
      \ new statement
      
      \ new statement
      0>!
      IF
         10001H - §break18687 BRANCH
      ENDIF
      DROP
      
      \ new statement
   ENDIF
   
   \ new statement
   §this CELL+ @ 36 + ( java.lang.StringBuffer.shared )    @
   
   \ new statement
   IF
      
      \ new statement
      §this A:R@
      R> 1412784896 TRUE ( java.lang.StringBuffer.copy§1412784896 ) EXECUTE-METHOD
      
      \ new statement
   ENDIF
   
   \ new statement
   
   0
   LOCALS 0§ |
   
   index TO 0§
   ch
   §this CELL+ @ 28 + ( java.lang.StringBuffer.value ) @ A:R@
   0§
   R> -942971136 TRUE ( java.lang.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
   H!
   
   PURGE 1
   
   
   \ new statement
   0 §break18687 LABEL
   
   \ new statement
   §synchronized0 A:R@
   R> 1974299904 TRUE ( Object.monitorLeave ) EXECUTE-METHOD
   PURGE 1
   
   \ new statement
   
   
   §base0 SETVTOP
   PURGE 4
   
   \ new statement
   DROP
;
( *
* Sets the length of this String buffer.
* This string buffer is altered to represent a new character sequence
* whose length is specified by the argument. For every nonnegative
* index <i>k</i> less than <code>newLength</code>, the character at
* index <i>k</i> in the new character sequence is the same as the
* character at index <i>k</i> in the old sequence if <i>k</i> is less
* than the length of the old character sequence; otherwise, it is the
* null character <code>'&#92;u0000'</code>.
*
* In other words, if the <code>newLength</code> argument is less than
* the current length of the string buffer, the string buffer is
* truncated to contain exactly the number of characters given by the
* <code>newLength</code> argument.
* <p>
* If the <code>newLength</code> argument is greater than or equal
* to the current length, sufficient null characters
* (<code>'&#92;u0000'</code>) are appended to the string buffer so that
* length becomes the <code>newLength</code> argument.
* <p>
* The <code>newLength</code> argument must be greater than or equal
* to <code>0</code>.
*
* @param      newLength   the new length of the buffer.
* @exception  IndexOutOfBoundsException  if the
*               <code>newLength</code> argument is negative.
* @see        java.lang.StringBuffer#length()
)

:LOCAL lang.StringBuffer.setLength§1089893120
   1 VALLOCATE LOCAL §base0
   LOCAL newLength
   DUP 0 §base0 + V! LOCAL §this
   
   \ new statement
   
   \ new statement
   §this LOCAL §synchronized0
   DI
   §synchronized0 A:R@
   lang.StringBuffer._staticThread
   R> 1472163072 TRUE ( Object.monitorEnter ) EXECUTE-METHOD
   EI
   
   \ new statement
   newLength
   0<
   
   \ new statement
   IF
      
      \ new statement
      
      \ new statement
      newLength
      lang.StringIndexOutOfBoundsException§-945597696.table 1138905856 EXECUTE-NEW
      lang.JavaArray.handler§-1096259584
      
      \ new statement
      
      \ new statement
      0 §break18672 LABEL
      
      \ new statement
      
      \ new statement
      0>!
      IF
         10001H - §break18671 BRANCH
      ENDIF
      DROP
      
      \ new statement
   ENDIF
   
   \ new statement
   newLength
   §this CELL+ @ 28 + ( java.lang.StringBuffer.value ) @ CELL+ @ 28 + ( java.lang.JavaArray.length )    @
   >
   
   \ new statement
   IF
      
      \ new statement
      
      \ new statement
      §this A:R@
      newLength
      R> -1659607808 TRUE ( java.lang.StringBuffer.expandCapacity§-1659607808 ) EXECUTE-METHOD
      
      \ new statement
      0 §break18673 LABEL
      
      \ new statement
      
      \ new statement
      0>!
      IF
         10001H - §break18671 BRANCH
      ENDIF
      DROP
      
      \ new statement
   ENDIF
   
   \ new statement
   §this CELL+ @ 32 + ( java.lang.StringBuffer.count )    @
   newLength
   <
   
   \ new statement
   IF
      
      \ new statement
      
      \ new statement
      §this CELL+ @ 36 + ( java.lang.StringBuffer.shared )       @
      
      \ new statement
      IF
         
         \ new statement
         §this A:R@
         R> 1412784896 TRUE ( java.lang.StringBuffer.copy§1412784896 ) EXECUTE-METHOD
         
         \ new statement
      ENDIF
      
      \ new statement
      
      \ new statement
      BEGIN
         
         \ new statement
         §this CELL+ @ 32 + ( java.lang.StringBuffer.count )          @
         newLength
         <
         
         \ new statement
      WHILE
         
         \ new statement
         
         0
         LOCALS 0§ |
         
         §this CELL+ @ 32 + ( java.lang.StringBuffer.count )          @ TO 0§
         0
         §this CELL+ @ 28 + ( java.lang.StringBuffer.value ) @ A:R@
         0§
         R> -942971136 TRUE ( java.lang.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
         H!
         
         PURGE 1
         
         
         \ new statement
         
         \ new statement
         §this CELL+ @ 32 + ( java.lang.StringBuffer.count )          @ A:R@ 1+ R@ !
         A:DROP
         
         \ new statement
      REPEAT
      
      \ new statement
      0 §break18675 LABEL
      
      \ new statement
      
      \ new statement
      DROP
      
      \ new statement
      0 §break18674 LABEL
      
      \ new statement
      
      \ new statement
      0>!
      IF
         10001H - §break18671 BRANCH
      ENDIF
      DROP
      
      \ new statement
   ELSE
      
      \ new statement
      
      \ new statement
      newLength
      §this CELL+ @ 32 + ( java.lang.StringBuffer.count )       !
      
      \ new statement
      §this CELL+ @ 36 + ( java.lang.StringBuffer.shared )       @
      
      \ new statement
      IF
         
         \ new statement
         
         \ new statement
         newLength
         0>
         
         \ new statement
         IF
            
            \ new statement
            
            \ new statement
            §this A:R@
            R> 1412784896 TRUE ( java.lang.StringBuffer.copy§1412784896 ) EXECUTE-METHOD
            
            \ new statement
            0 §break18679 LABEL
            
            \ new statement
            
            \ new statement
            DROP
            
            \ new statement
         ELSE
            
            \ new statement
            
            \ new statement
            \  If newLength is zero, assume the StringBuffer is being
            \  stripped for reuse; Make new buffer of default size
            
            
            \ new statement
            
            1 VALLOCATE LOCAL §tempvar
            16
            131073 lang.JavaArray§1352878592.table -227194368 EXECUTE-NEW
            §this CELL+ @ 28 + ( java.lang.StringBuffer.value )             OVER 0 V! !
            
            §tempvar SETVTOP
            
            PURGE 1
            
            
            \ new statement
            FALSE
            §this CELL+ @ 36 + ( java.lang.StringBuffer.shared )             !
            
            \ new statement
            0 §break18680 LABEL
            
            \ new statement
            
            \ new statement
            DROP
            
            \ new statement
         ENDIF
         
         \ new statement
         0 §break18678 LABEL
         
         \ new statement
         
         \ new statement
         DROP
         
         \ new statement
      ENDIF
      
      \ new statement
      0 §break18677 LABEL
      
      \ new statement
      
      \ new statement
      0>!
      IF
         10001H - §break18671 BRANCH
      ENDIF
      DROP
      
      \ new statement
   ENDIF
   
   \ new statement
   0 §break18671 LABEL
   
   \ new statement
   §synchronized0 A:R@
   R> 1974299904 TRUE ( Object.monitorLeave ) EXECUTE-METHOD
   PURGE 1
   
   \ new statement
   
   §base0 SETVTOP
   PURGE 3
   
   \ new statement
   DROP
;
\
\  The following two methods are needed by String to efficiently
\  convert a StringBuffer into a String.  They are not public.
\  They shouldn't be called by anyone but String.


:LOCAL lang.StringBuffer.setShared§1849258752
   1 VALLOCATE LOCAL §base0
   DUP 0 §base0 + V! LOCAL §this
   
   \ new statement
   
   \ new statement
   TRUE
   §this CELL+ @ 36 + ( java.lang.StringBuffer.shared )    !
   
   \ new statement
   0 §break18731 LABEL
   
   \ new statement
   §base0 SETVTOP
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
* sb.subSequence(begin,&nbsp;end)</pre></blockquote>
*
* behaves in exactly the same way as the invocation
*
* <blockquote><pre>
* sb.substring(begin,&nbsp;end)</pre></blockquote>
*
* This method is provided so that the <tt>StringBuffer</tt> class can
* implement the {@link CharSequence} interface. </p>
*
* @param      start   the start index, inclusive.
* @param      end     the end index, exclusive.
* @return     the specified subsequence.
*
* @throws  IndexOutOfBoundsException
*          if <tt>start</tt> or <tt>end</tt> are negative,
*          if <tt>end</tt> is greater than <tt>length()</tt>,
*          or if <tt>start</tt> is greater than <tt>end</tt>
*
* @since 1.4
* @spec JSR-51
)

:LOCAL lang.StringBuffer.subSequence§602239744
   1 VALLOCATE LOCAL §base0
   LOCAL end
   LOCAL start
   DUP 0 §base0 + V! LOCAL §this
   
   \ new statement
   
   \ new statement
   §this A:R@
   start
   end
   R> -424054016 TRUE ( java.lang.StringBuffer.substring§-424054016 ) EXECUTE-METHOD
   DUP 0 V!0 §break18708 BRANCH
   
   \ new statement
   0 §break18708 LABEL
   
   \ new statement
   
   
   §base0 SETVTOP
   PURGE 4
   
   \ new statement
   DROP
;
( *
* Returns a new <code>String</code> that contains a subsequence of
* characters currently contained in this <code>StringBuffer</code>. The
* substring begins at the specified <code>start</code> and
* extends to the character at index <code>end - 1</code>. An
* exception is thrown if
*
* @param      start    The beginning index, inclusive.
* @param      end      The ending index, exclusive.
* @return     The new string.
* @exception  StringIndexOutOfBoundsException  if <code>start</code>
*             or <code>end</code> are negative or greater than
*		   <code>length()</code>, or <code>start</code> is
*		   greater than <code>end</code>.
* @since      1.2
)

:LOCAL lang.StringBuffer.substring§-424054016
   1 VALLOCATE LOCAL §base0
   LOCAL end
   LOCAL start
   DUP 0 §base0 + V! LOCAL §this
   
   \ new statement
   
   \ new statement
   §this LOCAL §synchronized0
   DI
   §synchronized0 A:R@
   lang.StringBuffer._staticThread
   R> 1472163072 TRUE ( Object.monitorEnter ) EXECUTE-METHOD
   EI
   
   \ new statement
   start
   0<
   
   \ new statement
   IF
      
      \ new statement
      start
      lang.StringIndexOutOfBoundsException§-945597696.table 1138905856 EXECUTE-NEW
      lang.JavaArray.handler§-1096259584
      
      \ new statement
      
      \ new statement
   ENDIF
   
   \ new statement
   end
   §this CELL+ @ 32 + ( java.lang.StringBuffer.count )    @
   >
   
   \ new statement
   IF
      
      \ new statement
      end
      lang.StringIndexOutOfBoundsException§-945597696.table 1138905856 EXECUTE-NEW
      lang.JavaArray.handler§-1096259584
      
      \ new statement
      
      \ new statement
   ENDIF
   
   \ new statement
   start
   end
   >
   
   \ new statement
   IF
      
      \ new statement
      end
      start
      -
      lang.StringIndexOutOfBoundsException§-945597696.table 1138905856 EXECUTE-NEW
      lang.JavaArray.handler§-1096259584
      
      \ new statement
      
      \ new statement
   ENDIF
   
   \ new statement
   §this CELL+ @ 28 + ( java.lang.StringBuffer.value )    @
   start
   end
   start
   -
   lang.String§1651856128.table 1127437056 EXECUTE-NEW
   DUP 0 V!0 §break18709 BRANCH
   
   \ new statement
   0 §break18709 LABEL
   
   \ new statement
   §synchronized0 A:R@
   R> 1974299904 TRUE ( Object.monitorLeave ) EXECUTE-METHOD
   PURGE 1
   
   \ new statement
   
   
   §base0 SETVTOP
   PURGE 4
   
   \ new statement
   DROP
;
( *
* Returns a new <code>String</code> that contains a subsequence of
* characters currently contained in this <code>StringBuffer</code>.The
* substring begins at the specified index and extends to the end of the
* <code>StringBuffer</code>.
*
* @param      start    The beginning index, inclusive.
* @return     The new string.
* @exception  StringIndexOutOfBoundsException  if <code>start</code> is
*             less than zero, or greater than the length of this
*             <code>StringBuffer</code>.
* @since      1.2
)

:LOCAL lang.StringBuffer.substring§18576128
   1 VALLOCATE LOCAL §base0
   LOCAL start
   DUP 0 §base0 + V! LOCAL §this
   
   \ new statement
   
   \ new statement
   §this LOCAL §synchronized0
   DI
   §synchronized0 A:R@
   lang.StringBuffer._staticThread
   R> 1472163072 TRUE ( Object.monitorEnter ) EXECUTE-METHOD
   EI
   
   \ new statement
   §this A:R@
   start
   §this CELL+ @ 32 + ( java.lang.StringBuffer.count )    @
   R> -424054016 TRUE ( java.lang.StringBuffer.substring§-424054016 ) EXECUTE-METHOD
   DUP 0 V!0 §break18707 BRANCH
   
   \ new statement
   0 §break18707 LABEL
   
   \ new statement
   §synchronized0 A:R@
   R> 1974299904 TRUE ( Object.monitorLeave ) EXECUTE-METHOD
   PURGE 1
   
   \ new statement
   
   §base0 SETVTOP
   PURGE 3
   
   \ new statement
   DROP
;
( *
* Converts to a string representing the data in this string buffer.
* A new <code>String</code> object is allocated and initialized to
* contain the character sequence currently represented by this
* string buffer. This <code>String</code> is then returned. Subsequent
* changes to the string buffer do not affect the contents of the
* <code>String</code>.
* <p>
* Implementation advice: This method can be coded so as to create a new
* <code>String</code> object without allocating new memory to hold a
* copy of the character sequence. Instead, the string can share the
* memory used by the string buffer. Any subsequent operation that alters
* the content or capacity of the string buffer must then make a copy of
* the internal buffer at that time. This strategy is effective for
* reducing the amount of memory allocated by a string concatenation
* operation when it is implemented using a string buffer.
*
* @return  a string representation of the string buffer.
)

:LOCAL lang.StringBuffer.toString§1621718016
   1 VALLOCATE LOCAL §base0
   DUP 0 §base0 + V! LOCAL §this
   
   \ new statement
   
   \ new statement
   §this
   lang.String§1651856128.table -643083520 EXECUTE-NEW
   DUP 0 V!0 §break18730 BRANCH
   
   \ new statement
   0 §break18730 LABEL
   
   \ new statement
   §base0 SETVTOP
   PURGE 2
   
   \ new statement
   DROP
;
-1 VALLOCATE CONSTANT lang.StringBuffer.NULL
VARIABLE lang.StringBuffer._staticBlocking
VARIABLE lang.StringBuffer._staticThread
2VARIABLE lang.StringBuffer.serialVersionUID

A:HERE VARIABLE lang.StringBuffer§426332928.table 46 DUP 2* CELLS ALLOT R@ ! A:CELL+
-2134617344 R@ ! A:CELL+ lang.StringBuffer.StringBuffer§-2134617344 VAL R@ ! A:CELL+
421942016 R@ ! A:CELL+ lang.StringBuffer.StringBuffer§421942016 VAL R@ ! A:CELL+
495342336 R@ ! A:CELL+ lang.StringBuffer.StringBuffer§495342336 VAL R@ ! A:CELL+
-1086627584 R@ ! A:CELL+ lang.StringBuffer.append§-1086627584 VAL R@ ! A:CELL+
-1912250112 R@ ! A:CELL+ lang.StringBuffer.append§-1912250112 VAL R@ ! A:CELL+
-1994301184 R@ ! A:CELL+ lang.StringBuffer.append§-1994301184 VAL R@ ! A:CELL+
-310157056 R@ ! A:CELL+ lang.StringBuffer.append§-310157056 VAL R@ ! A:CELL+
-523149056 R@ ! A:CELL+ lang.StringBuffer.append§-523149056 VAL R@ ! A:CELL+
1081041152 R@ ! A:CELL+ lang.StringBuffer.append§1081041152 VAL R@ ! A:CELL+
1445355776 R@ ! A:CELL+ lang.StringBuffer.append§1445355776 VAL R@ ! A:CELL+
1793024256 R@ ! A:CELL+ lang.StringBuffer.append§1793024256 VAL R@ ! A:CELL+
2069258496 R@ ! A:CELL+ lang.StringBuffer.append§2069258496 VAL R@ ! A:CELL+
522477824 R@ ! A:CELL+ lang.StringBuffer.append§522477824 VAL R@ ! A:CELL+
371417856 R@ ! A:CELL+ lang.StringBuffer.capacity§371417856 VAL R@ ! A:CELL+
-1014275328 R@ ! A:CELL+ lang.StringBuffer.charAt§-1014275328 VAL R@ ! A:CELL+
1412784896 R@ ! A:CELL+ lang.StringBuffer.copy§1412784896 VAL R@ ! A:CELL+
162817024 R@ ! A:CELL+ lang.StringBuffer.delete§162817024 VAL R@ ! A:CELL+
1018258432 R@ ! A:CELL+ lang.StringBuffer.deleteCharAt§1018258432 VAL R@ ! A:CELL+
-1584765696 R@ ! A:CELL+ lang.StringBuffer.ensureCapacity§-1584765696 VAL R@ ! A:CELL+
-1659607808 R@ ! A:CELL+ lang.StringBuffer.expandCapacity§-1659607808 VAL R@ ! A:CELL+
-53713152 R@ ! A:CELL+ lang.StringBuffer.getChars§-53713152 VAL R@ ! A:CELL+
551249664 R@ ! A:CELL+ lang.StringBuffer.getValue§551249664 VAL R@ ! A:CELL+
-337549056 R@ ! A:CELL+ lang.StringBuffer.indexOf§-337549056 VAL R@ ! A:CELL+
517761280 R@ ! A:CELL+ lang.StringBuffer.indexOf§517761280 VAL R@ ! A:CELL+
-1255511808 R@ ! A:CELL+ lang.StringBuffer.insert§-1255511808 VAL R@ ! A:CELL+
-1532794624 R@ ! A:CELL+ lang.StringBuffer.insert§-1532794624 VAL R@ ! A:CELL+
-1717409536 R@ ! A:CELL+ lang.StringBuffer.insert§-1717409536 VAL R@ ! A:CELL+
-1765316352 R@ ! A:CELL+ lang.StringBuffer.insert§-1765316352 VAL R@ ! A:CELL+
-321427200 R@ ! A:CELL+ lang.StringBuffer.insert§-321427200 VAL R@ ! A:CELL+
1900505344 R@ ! A:CELL+ lang.StringBuffer.insert§1900505344 VAL R@ ! A:CELL+
2145478912 R@ ! A:CELL+ lang.StringBuffer.insert§2145478912 VAL R@ ! A:CELL+
570910976 R@ ! A:CELL+ lang.StringBuffer.insert§570910976 VAL R@ ! A:CELL+
795633920 R@ ! A:CELL+ lang.StringBuffer.insert§795633920 VAL R@ ! A:CELL+
900622592 R@ ! A:CELL+ lang.StringBuffer.insert§900622592 VAL R@ ! A:CELL+
-1299878912 R@ ! A:CELL+ lang.StringBuffer.lastIndexOf§-1299878912 VAL R@ ! A:CELL+
-465605632 R@ ! A:CELL+ lang.StringBuffer.lastIndexOf§-465605632 VAL R@ ! A:CELL+
188050432 R@ ! A:CELL+ lang.StringBuffer.length§188050432 VAL R@ ! A:CELL+
948597248 R@ ! A:CELL+ lang.StringBuffer.replace§948597248 VAL R@ ! A:CELL+
-1963298304 R@ ! A:CELL+ lang.StringBuffer.reverse§-1963298304 VAL R@ ! A:CELL+
-497847552 R@ ! A:CELL+ lang.StringBuffer.setCharAt§-497847552 VAL R@ ! A:CELL+
1089893120 R@ ! A:CELL+ lang.StringBuffer.setLength§1089893120 VAL R@ ! A:CELL+
1849258752 R@ ! A:CELL+ lang.StringBuffer.setShared§1849258752 VAL R@ ! A:CELL+
602239744 R@ ! A:CELL+ lang.StringBuffer.subSequence§602239744 VAL R@ ! A:CELL+
-424054016 R@ ! A:CELL+ lang.StringBuffer.substring§-424054016 VAL R@ ! A:CELL+
18576128 R@ ! A:CELL+ lang.StringBuffer.substring§18576128 VAL R@ ! A:CELL+
1621718016 R@ ! A:CELL+ lang.StringBuffer.toString§1621718016 VAL R@ ! A:CELL+
A:DROP
