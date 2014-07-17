MODULENAME java.lang.Character
\ import gnu.java.lang.CharData;
\ import java.io.Serializable;
\ import java.text.Collator;
( *
* Wrapper class for the primitive char data type.  In addition, this class
* allows one to retrieve property information and perform transformations
* on the defined characters in the Unicode Standard, Version 4.0.0.
* java.lang.Character is designed to be very dynamic, and as such, it
* retrieves information on the Unicode character set from a separate
* database, gnu.java.lang.CharData, which can be easily upgraded.
*
* <p>For predicates, boundaries are used to describe
* the set of characters for which the method will return true.
* This syntax uses fairly normal regular expression notation.
* See 5.13 of the Unicode Standard, Version 4.0, for the
* boundary specification.
*
* <p>See <a href="http://www.unicode.org">http://www.unicode.org</a>
* for more information on the Unicode Standard.
*
* @author Tom Tromey (tromey@cygnus.com)
* @author Paul N. Fisher
* @author Jochen Hoenicke
* @author Eric Blake (ebb9@email.byu.edu)
* @see CharData
* @since 1.0
* @status updated to 1.4
)


:LOCAL lang.Character.Character§-1412545792
   1 VALLOCATE LOCAL §base0
   
   \ new statement
   0
   LOCALS §this |
   
   \ new statement
   lang.Object§-1890496768.table 1827294976 EXECUTE-NEW
   
   \ new statement
   32 MALLOC DROP DUP DUP §base0 V! TO §this OVER
   IF
      OVER CELL+ @ 4 + OVER OVER ! DROP
   ENDIF
   CELL+ @ OVER
   OVER ! NIP
   lang.Character§1114260224.table OVER 12 + !
   1114243072 OVER 20 + !
   " Character " OVER 16 + !
   0 OVER 24 + ! DROP
   
   \ new statement
   ( *
   * The immutable value of this Character.
   *
   * @serial the value of this Character
   )
   
   \ new statement
   §this DUP 0 V! ( return object )    
   \ new statement
   0 §break17887 LABEL
   
   \ new statement
   §base0 SETVTOP
   PURGE 2
   
   \ new statement
   DROP
;
( *
* Wraps up a character.
*
* @param value the character to wrap
)

:LOCAL lang.Character.Character§849625856
   1 VALLOCATE LOCAL §base0
   LOCAL value
   
   \ new statement
   0
   LOCALS §this |
   
   \ new statement
   lang.Object§-1890496768.table 1827294976 EXECUTE-NEW
   
   \ new statement
   32 MALLOC DROP DUP DUP §base0 V! TO §this OVER
   IF
      OVER CELL+ @ 4 + OVER OVER ! DROP
   ENDIF
   CELL+ @ OVER
   OVER ! NIP
   lang.Character§1114260224.table OVER 12 + !
   1114243072 OVER 20 + !
   " Character " OVER 16 + !
   0 OVER 24 + ! DROP
   
   \ new statement
   ( *
   * The immutable value of this Character.
   *
   * @serial the value of this Character
   )
   
   \ new statement
   value
   §this CELL+ @ 28 + ( java.lang.Character.value )    H!
   
   \ new statement
   §this DUP 0 V! ( return object )    
   \ new statement
   0 §break17824 LABEL
   
   \ new statement
   
   §base0 SETVTOP
   PURGE 3
   
   \ new statement
   DROP
;
( *
* Return number of 16-bit characters required to represent the given
* code point.
*
* @param codePoint a unicode code point
*
* @return 2 if codePoint >= 0x10000, 1 otherwise.
*
* @since 1.5
)

: lang.Character.charCount§-1036950784
   LOCAL codePoint
   
   \ new statement
   
   \ new statement
   codePoint
   lang.Character.MIN_SUPPLEMENTARY_CODE_POINT
   @
   < INVERT
   
   IF
      2
      
   ELSE
      1
      
   ENDIF
   0 §break17874 BRANCH
   
   \ new statement
   0 §break17874 LABEL
   
   \ new statement
   
   PURGE 1
   
   \ new statement
   DROP
;
( *
* Returns the character which has been wrapped by this class.
*
* @return the character wrapped
)

:LOCAL lang.Character.charValue§-63216896
   1 VALLOCATE LOCAL §base0
   DUP 0 §base0 + V! LOCAL §this
   
   \ new statement
   
   \ new statement
   §this CELL+ @ 28 + ( java.lang.Character.value )    H@
   0 §break17825 BRANCH
   
   \ new statement
   0 §break17825 LABEL
   
   \ new statement
   §base0 SETVTOP
   PURGE 2
   
   \ new statement
   DROP
;


: lang.Character.classMonitorEnter§-433167616
   
   \ new statement
   
   \ new statement
   0 §break17822 LABEL
   
   \ new statement
   
   \ new statement
   DROP
;


: lang.Character.classMonitorLeave§-1691393280
   
   \ new statement
   
   \ new statement
   0 §break17823 LABEL
   
   \ new statement
   
   \ new statement
   DROP
;
( *
* Get the code point at the specified index in the CharSequence.
* If the character is the start of a surrogate pair, and there is a
* following character within the specified range, and this
* character completes the pair, then the corresponding
* supplementary code point is returned.  Otherwise, the character
* at the index is returned.
*
* @param chars the character array in which to look
* @param index the index of the codepoint to get, starting at 0
* @param limit the limit past which characters should not be examined
* @return the codepoint at the specified index
* @throws IndexOutOfBoundsException if index is negative or &gt;=
* limit, or if limit is negative or &gt;= the length of the array
* @since 1.5
)

: lang.Character.codePointAt§-1236376832
   1 VALLOCATE LOCAL §base0
   LOCAL limit
   LOCAL index
   DUP 0 §base0 + V! LOCAL chars
   
   \ new statement
   0 DUP
   LOCALS high low |
   
   \ new statement
   index
   0<  0=! or_17953 0BRANCH DROP
   index
   limit
   < INVERT
   0=! or_17953 0BRANCH DROP
   limit
   0<
   0=! or_17953 0BRANCH DROP
   limit
   chars CELL+ @ 28 + ( java.lang.JavaArray.length )    @
   >
   or_17953 LABEL
   
   \ new statement
   IF
      
      \ new statement
      lang.IndexOutOfBoundsException§1771129088.table -496219904 EXECUTE-NEW
      lang.JavaArray.handler§-1096259584
      
      \ new statement
      
      \ new statement
   ENDIF
   
   \ new statement
   chars A:R@
   index
   R> -942971136 TRUE ( java.lang.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
   H@
   TO high
   
   \ new statement
   high
   lang.Character.isHighSurrogate§-311465728
   0=
   0=! or_17960 0BRANCH DROP
   index 1+ DUP TO index
   limit
   < INVERT
   or_17960 LABEL
   
   \ new statement
   IF
      
      \ new statement
      high
      0 §break17883 BRANCH
      
      \ new statement
   ENDIF
   
   \ new statement
   chars A:R@
   index
   R> -942971136 TRUE ( java.lang.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
   H@
   TO low
   
   \ new statement
   low
   lang.Character.isLowSurrogate§879388928
   0=
   
   \ new statement
   IF
      
      \ new statement
      high
      0 §break17883 BRANCH
      
      \ new statement
   ENDIF
   
   \ new statement
   high
   low
   lang.Character.toCodePoint§-2064157696
   0 §break17883 BRANCH
   
   \ new statement
   0 §break17883 LABEL
   
   \ new statement
   
   
   
   
   
   §base0 SETVTOP
   PURGE 6
   
   \ new statement
   DROP
;
( *
* Get the code point at the specified index in the CharSequence.
* This is like CharSequence#charAt(int), but if the character is
* the start of a surrogate pair, and there is a following
* character, and this character completes the pair, then the
* corresponding supplementary code point is returned.  Otherwise,
* the character at the index is returned.
*
* @param sequence the CharSequence
* @param index the index of the codepoint to get, starting at 0
* @return the codepoint at the specified index
* @throws IndexOutOfBoundsException if index is negative or &gt;= length()
* @since 1.5
)

: lang.Character.codePointAt§1204249344
   1 VALLOCATE LOCAL §base0
   LOCAL index
   DUP 0 §base0 + V! LOCAL sequence
   
   \ new statement
   0 DUP DUP
   LOCALS high len low |
   
   \ new statement
   sequence A:R@
   R> 188050432 TRUE ( java.lang.CharSequence.length§188050432 ) EXECUTE-METHOD
   TO len
   
   \ new statement
   index
   0<  0=! or_17976 0BRANCH DROP
   index
   len
   < INVERT
   or_17976 LABEL
   
   \ new statement
   IF
      
      \ new statement
      lang.IndexOutOfBoundsException§1771129088.table -496219904 EXECUTE-NEW
      lang.JavaArray.handler§-1096259584
      
      \ new statement
      
      \ new statement
   ENDIF
   
   \ new statement
   sequence A:R@
   index
   R> -1014275328 TRUE ( java.lang.CharSequence.charAt§-1014275328 ) EXECUTE-METHOD
   TO high
   
   \ new statement
   high
   lang.Character.isHighSurrogate§-311465728
   0=
   0=! or_17983 0BRANCH DROP
   index 1+ DUP TO index
   len
   < INVERT
   or_17983 LABEL
   
   \ new statement
   IF
      
      \ new statement
      high
      0 §break17881 BRANCH
      
      \ new statement
   ENDIF
   
   \ new statement
   sequence A:R@
   index
   R> -1014275328 TRUE ( java.lang.CharSequence.charAt§-1014275328 ) EXECUTE-METHOD
   TO low
   
   \ new statement
   low
   lang.Character.isLowSurrogate§879388928
   0=
   
   \ new statement
   IF
      
      \ new statement
      high
      0 §break17881 BRANCH
      
      \ new statement
   ENDIF
   
   \ new statement
   high
   low
   lang.Character.toCodePoint§-2064157696
   0 §break17881 BRANCH
   
   \ new statement
   0 §break17881 LABEL
   
   \ new statement
   
   
   
   
   
   §base0 SETVTOP
   PURGE 6
   
   \ new statement
   DROP
;
( *
* Get the code point at the specified index in the CharSequence.
* If the character is the start of a surrogate pair, and there is a
* following character, and this character completes the pair, then
* the corresponding supplementary code point is returned.
* Otherwise, the character at the index is returned.
*
* @param chars the character array in which to look
* @param index the index of the codepoint to get, starting at 0
* @return the codepoint at the specified index
* @throws IndexOutOfBoundsException if index is negative or &gt;= length()
* @since 1.5
)

: lang.Character.codePointAt§1772118784
   1 VALLOCATE LOCAL §base0
   LOCAL index
   DUP 0 §base0 + V! LOCAL chars
   
   \ new statement
   
   \ new statement
   chars
   index
   chars CELL+ @ 28 + ( java.lang.JavaArray.length )    @
   lang.Character.codePointAt§-1236376832
   0 §break17882 BRANCH
   
   \ new statement
   0 §break17882 LABEL
   
   \ new statement
   
   
   §base0 SETVTOP
   PURGE 3
   
   \ new statement
   DROP
;
( *
* Get the code point before the specified index.  This is like
* #codePointAt(char[], int), but checks the characters at
* <code>index-1</code> and <code>index-2</code> to see if they form
* a supplementary code point.  If they do not, the character at
* <code>index-1</code> is returned.
*
* @param chars the character array
* @param index the index just past the codepoint to get, starting at 0
* @return the codepoint at the specified index
* @throws IndexOutOfBoundsException if index is negative or &gt;= length()
* @since 1.5
)

: lang.Character.codePointBefore§485450496
   1 VALLOCATE LOCAL §base0
   LOCAL index
   DUP 0 §base0 + V! LOCAL chars
   
   \ new statement
   
   \ new statement
   chars
   index
   1
   lang.Character.codePointBefore§824009472
   0 §break17884 BRANCH
   
   \ new statement
   0 §break17884 LABEL
   
   \ new statement
   
   
   §base0 SETVTOP
   PURGE 3
   
   \ new statement
   DROP
;
( *
* Get the code point before the specified index.  This is like
* #codePointAt(CharSequence, int), but checks the characters at
* <code>index-1</code> and <code>index-2</code> to see if they form
* a supplementary code point.  If they do not, the character at
* <code>index-1</code> is returned.
*
* @param sequence the CharSequence
* @param index the index just past the codepoint to get, starting at 0
* @return the codepoint at the specified index
* @throws IndexOutOfBoundsException if index is negative or &gt;= length()
* @since 1.5
)

: lang.Character.codePointBefore§74736384
   1 VALLOCATE LOCAL §base0
   LOCAL index
   DUP 0 §base0 + V! LOCAL sequence
   
   \ new statement
   0 DUP DUP
   LOCALS high len low |
   
   \ new statement
   sequence A:R@
   R> 188050432 TRUE ( java.lang.CharSequence.length§188050432 ) EXECUTE-METHOD
   TO len
   
   \ new statement
   index
   1
   <  0=! or_18009 0BRANCH DROP
   index
   len
   >
   or_18009 LABEL
   
   \ new statement
   IF
      
      \ new statement
      lang.IndexOutOfBoundsException§1771129088.table -496219904 EXECUTE-NEW
      lang.JavaArray.handler§-1096259584
      
      \ new statement
      
      \ new statement
   ENDIF
   
   \ new statement
   index 1- TO index
   
   \ new statement
   sequence A:R@
   index
   R> -1014275328 TRUE ( java.lang.CharSequence.charAt§-1014275328 ) EXECUTE-METHOD
   TO low
   
   \ new statement
   low
   lang.Character.isLowSurrogate§879388928
   0=
   0=! or_18017 0BRANCH DROP
   index 1- DUP TO index
   0<
   or_18017 LABEL
   
   \ new statement
   IF
      
      \ new statement
      low
      0 §break17886 BRANCH
      
      \ new statement
   ENDIF
   
   \ new statement
   sequence A:R@
   index
   R> -1014275328 TRUE ( java.lang.CharSequence.charAt§-1014275328 ) EXECUTE-METHOD
   TO high
   
   \ new statement
   high
   lang.Character.isHighSurrogate§-311465728
   0=
   
   \ new statement
   IF
      
      \ new statement
      low
      0 §break17886 BRANCH
      
      \ new statement
   ENDIF
   
   \ new statement
   high
   low
   lang.Character.toCodePoint§-2064157696
   0 §break17886 BRANCH
   
   \ new statement
   0 §break17886 LABEL
   
   \ new statement
   
   
   
   
   
   §base0 SETVTOP
   PURGE 6
   
   \ new statement
   DROP
;
( *
* Get the code point before the specified index.  This is like
* #codePointAt(char[], int), but checks the characters at
* <code>index-1</code> and <code>index-2</code> to see if they form
* a supplementary code point.  If they do not, the character at
* <code>index-1</code> is returned.  The start parameter is used to
* limit the range of the array which may be examined.
*
* @param chars the character array
* @param index the index just past the codepoint to get, starting at 0
* @param start the index before which characters should not be examined
* @return the codepoint at the specified index
* @throws IndexOutOfBoundsException if index is &gt; start or &gt;
* the length of the array, or if limit is negative or &gt;= the
* length of the array
* @since 1.5
)

: lang.Character.codePointBefore§824009472
   1 VALLOCATE LOCAL §base0
   LOCAL start
   LOCAL index
   DUP 0 §base0 + V! LOCAL chars
   
   \ new statement
   0 DUP
   LOCALS high low |
   
   \ new statement
   index
   start
   <  0=! or_18032 0BRANCH DROP
   index
   chars CELL+ @ 28 + ( java.lang.JavaArray.length )    @
   >
   0=! or_18032 0BRANCH DROP
   start
   0<
   0=! or_18032 0BRANCH DROP
   start
   chars CELL+ @ 28 + ( java.lang.JavaArray.length )    @
   < INVERT
   or_18032 LABEL
   
   \ new statement
   IF
      
      \ new statement
      lang.IndexOutOfBoundsException§1771129088.table -496219904 EXECUTE-NEW
      lang.JavaArray.handler§-1096259584
      
      \ new statement
      
      \ new statement
   ENDIF
   
   \ new statement
   index 1- TO index
   
   \ new statement
   chars A:R@
   index
   R> -942971136 TRUE ( java.lang.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
   H@
   TO low
   
   \ new statement
   low
   lang.Character.isLowSurrogate§879388928
   0=
   0=! or_18040 0BRANCH DROP
   index 1- DUP TO index
   start
   <
   or_18040 LABEL
   
   \ new statement
   IF
      
      \ new statement
      low
      0 §break17885 BRANCH
      
      \ new statement
   ENDIF
   
   \ new statement
   chars A:R@
   index
   R> -942971136 TRUE ( java.lang.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
   H@
   TO high
   
   \ new statement
   high
   lang.Character.isHighSurrogate§-311465728
   0=
   
   \ new statement
   IF
      
      \ new statement
      low
      0 §break17885 BRANCH
      
      \ new statement
   ENDIF
   
   \ new statement
   high
   low
   lang.Character.toCodePoint§-2064157696
   0 §break17885 BRANCH
   
   \ new statement
   0 §break17885 LABEL
   
   \ new statement
   
   
   
   
   
   §base0 SETVTOP
   PURGE 6
   
   \ new statement
   DROP
;
( *
* Compares another Character to this Character, numerically.
*
* @param anotherCharacter Character to compare with this Character
* @return a negative integer if this Character is less than
*         anotherCharacter, zero if this Character is equal, and
*         a positive integer if this Character is greater
* @throws NullPointerException if anotherCharacter is null
* @since 1.2
)

:LOCAL lang.Character.compareTo§-1220123904
   2 VALLOCATE LOCAL §base0
   DUP 0 §base0 + V! LOCAL anotherCharacter
   DUP 4 §base0 + V! LOCAL §this
   
   \ new statement
   
   \ new statement
   §this CELL+ @ 28 + ( java.lang.Character.value )    H@
   anotherCharacter CELL+ @ 28 + ( java.lang.Character.value )    H@
   -
   0 §break17864 BRANCH
   
   \ new statement
   0 §break17864 LABEL
   
   \ new statement
   
   §base0 SETVTOP
   PURGE 3
   
   \ new statement
   DROP
;
( *
* Compares an object to this Character.  Assuming the object is a
* Character object, this method performs the same comparison as
* compareTo(Character).
*
* @param o object to compare
* @return the comparison value
* @throws ClassCastException if o is not a Character object
* @throws NullPointerException if o is null
* @see #compareTo(Character)
* @since 1.2
)

:LOCAL lang.Character.compareTo§-879140096
   2 VALLOCATE LOCAL §base0
   DUP 0 §base0 + V! LOCAL o
   DUP 4 §base0 + V! LOCAL §this
   
   \ new statement
   
   \ new statement
   §this A:R@
   o
   R> -1220123904 TRUE ( java.lang.Character.compareTo§-1220123904 ) EXECUTE-METHOD
   0 §break17865 BRANCH
   
   \ new statement
   0 §break17865 LABEL
   
   \ new statement
   
   §base0 SETVTOP
   PURGE 3
   
   \ new statement
   DROP
;
( *
* Converts a character into a digit of the specified radix. If the radix
* exceeds MIN_RADIX or MAX_RADIX, or if the result of getNumericValue(ch)
* exceeds the radix, or if ch is not a decimal digit or in the case
* insensitive set of 'a'-'z', the result is -1.
* <br>
* character argument boundary = [Nd]|U+0041-U+005A|U+0061-U+007A
*    |U+FF21-U+FF3A|U+FF41-U+FF5A
*
* @param ch character to convert into a digit
* @param radix radix in which ch is a digit
* @return digit which ch represents in radix, or -1 not a valid digit
* @see #MIN_RADIX
* @see #MAX_RADIX
* @see #forDigit(int, int)
* @see #isDigit(char)
* @see #getNumericValue(char)
)

: lang.Character.digit§-795843584
   LOCAL radix
   LOCAL ch
   
   \ new statement
   
   \ new statement
   radix
   lang.Character.MIN_RADIX
   @
   <  0=! or_18065 0BRANCH DROP
   radix
   lang.Character.MAX_RADIX
   @
   >
   or_18065 LABEL
   
   \ new statement
   IF
      
      \ new statement
      -1
      0 §break17856 BRANCH
      
      \ new statement
   ENDIF
   
   \ new statement
   ch
   lang.Character.isDigit§1337157888
   
   \ new statement
   IF
      
      \ new statement
      ch
      48
      -
      0 §break17856 BRANCH
      
      \ new statement
   ENDIF
   
   \ new statement
   ch
   lang.Character.isLowerCase§-543201024
   
   \ new statement
   IF
      
      \ new statement
      ch
      87
      -
      0 §break17856 BRANCH
      
      \ new statement
   ENDIF
   
   \ new statement
   ch
   lang.Character.isUpperCase§203516160
   
   \ new statement
   IF
      
      \ new statement
      ch
      55
      -
      0 §break17856 BRANCH
      
      \ new statement
   ENDIF
   
   \ new statement
   -1
   0 §break17856 BRANCH
   
   \ new statement
   0 §break17856 LABEL
   
   \ new statement
   
   
   PURGE 2
   
   \ new statement
   DROP
;
( *
* Determines if an object is equal to this object. This is only true for
* another Character object wrapping the same value.
*
* @param o object to compare
* @return true if o is a Character with the same value
)

:LOCAL lang.Character.equals§-240098048
   2 VALLOCATE LOCAL §base0
   DUP 0 §base0 + V! LOCAL o
   DUP 4 §base0 + V! LOCAL §this
   
   \ new statement
   
   \ new statement
   o
   " Character " INSTANCEOF and_18087 0BRANCH! DROP
   §this CELL+ @ 28 + ( java.lang.Character.value )    H@
   o
   CELL+ @ 28 + ( java.lang.Character.value )    H@
   =
   and_18087 LABEL
   0 §break17827 BRANCH
   
   \ new statement
   0 §break17827 LABEL
   
   \ new statement
   
   §base0 SETVTOP
   PURGE 3
   
   \ new statement
   DROP
;
( *
* Converts a digit into a character which represents that digit
* in a specified radix. If the radix exceeds MIN_RADIX or MAX_RADIX,
* or the digit exceeds the radix, then the null character <code>'\0'</code>
* is returned.  Otherwise the return value is in '0'-'9' and 'a'-'z'.
* <br>
* return value boundary = U+0030-U+0039|U+0061-U+007A
*
* @param digit digit to be converted into a character
* @param radix radix of digit
* @return character representing digit in radix, or '\0'
* @see #MIN_RADIX
* @see #MAX_RADIX
* @see #digit(char, int)
)

: lang.Character.forDigit§-1878432256
   LOCAL radix
   LOCAL digit
   
   \ new statement
   
   \ new statement
   radix
   lang.Character.MIN_RADIX
   @
   <  0=! or_18093 0BRANCH DROP
   radix
   lang.Character.MAX_RADIX
   @
   >
   0=! or_18093 0BRANCH DROP
   digit
   0<
   0=! or_18093 0BRANCH DROP
   digit
   radix
   < INVERT
   or_18093 LABEL
   
   \ new statement
   IF
      
      \ new statement
      0
      0 §break17863 BRANCH
      
      \ new statement
   ENDIF
   
   \ new statement
   digit
   10
   <
   
   \ new statement
   IF
      
      \ new statement
      digit
      48
      +  0FFFFH AND
      0 §break17863 BRANCH
      
      \ new statement
   ENDIF
   
   \ new statement
   digit
   -87
   -  0FFFFH AND
   0 §break17863 BRANCH
   
   \ new statement
   0 §break17863 LABEL
   
   \ new statement
   
   
   PURGE 2
   
   \ new statement
   DROP
;
( *
* Returns the numerical value (unsigned) of the wrapped character.
* Range of returned values: 0x0000-0xFFFF.
*
* @return the value of the wrapped character
)

:LOCAL lang.Character.hashCode§-1604556800
   1 VALLOCATE LOCAL §base0
   DUP 0 §base0 + V! LOCAL §this
   
   \ new statement
   
   \ new statement
   §this CELL+ @ 28 + ( java.lang.Character.value )    H@
   0 §break17826 BRANCH
   
   \ new statement
   0 §break17826 LABEL
   
   \ new statement
   §base0 SETVTOP
   PURGE 2
   
   \ new statement
   DROP
;
( *
* Determines if a character is a Unicode decimal digit. For example,
* <code>'0'</code> is a digit.  A character is a Unicode digit if
* getType() returns DECIMAL_DIGIT_NUMBER.
* <br>
* Unicode decimal digit = [Nd]
*
* @param ch character to test
* @return true if ch is a Unicode decimal digit, else false
* @see #digit(char, int)
* @see #forDigit(int, int)
* @see #getType(char)
)

: lang.Character.isDigit§1337157888
   LOCAL ch
   
   \ new statement
   
   \ new statement
   ch
   lang.Character.isDigit§1363241216
   0 §break17834 BRANCH
   
   \ new statement
   0 §break17834 LABEL
   
   \ new statement
   
   PURGE 1
   
   \ new statement
   DROP
;
( *
* Determines if a character is a Unicode decimal digit. For example,
* <code>'0'</code> is a digit. A character is a Unicode digit if
* getType() returns DECIMAL_DIGIT_NUMBER.
* <br>
* Unicode decimal digit = [Nd]
*
* @param codePoint character to test
* @return true if ch is a Unicode decimal digit, else false
* @see #digit(char, int)
* @see #forDigit(int, int)
* @see #getType(char)
*
* @since 1.5
)

: lang.Character.isDigit§1363241216
   LOCAL codePoint
   
   \ new statement
   
   \ new statement
   \ return getType(codePoint) == DECIMAL_DIGIT_NUMBER;
   
   
   \ new statement
   codePoint
   48
   < INVERT and_18118 0BRANCH! DROP
   codePoint
   57
   > INVERT
   and_18118 LABEL
   0 §break17835 BRANCH
   
   \ new statement
   0 §break17835 LABEL
   
   \ new statement
   
   PURGE 1
   
   \ new statement
   DROP
;
( *
* Return true if the given character is a high surrogate.
* @param ch the character
* @return true if the character is a high surrogate character
*
* @since 1.5
)

: lang.Character.isHighSurrogate§-311465728
   LOCAL ch
   
   \ new statement
   
   \ new statement
   ch
   lang.Character.MIN_HIGH_SURROGATE
   H@
   < INVERT and_18124 0BRANCH! DROP
   ch
   lang.Character.MAX_HIGH_SURROGATE
   H@
   > INVERT
   and_18124 LABEL
   0 §break17877 BRANCH
   
   \ new statement
   0 §break17877 LABEL
   
   \ new statement
   
   PURGE 1
   
   \ new statement
   DROP
;
( *
* Determines if the character is an ISO Control character.  This is true
* if the code point is in the range [0, 0x001F] or if it is in the range
* [0x007F, 0x009F].
* @param codePoint the character to check
* @return true if the character is in one of the above ranges
*
* @since 1.5
)

: lang.Character.isISOControl§1198614784
   LOCAL codePoint
   
   \ new statement
   
   \ new statement
   codePoint
   0< INVERT and_18131 0BRANCH! DROP
   codePoint
   31
   > INVERT
   and_18131 LABEL
   0=! or_18130 0BRANCH DROP
   codePoint
   127
   < INVERT and_18132 0BRANCH! DROP
   codePoint
   159
   > INVERT
   and_18132 LABEL
   or_18130 LABEL
   
   \ new statement
   IF
      
      \ new statement
      TRUE
      0 §break17862 BRANCH
      
      \ new statement
   ENDIF
   
   \ new statement
   FALSE
   0 §break17862 BRANCH
   
   \ new statement
   0 §break17862 LABEL
   
   \ new statement
   
   PURGE 1
   
   \ new statement
   DROP
;
( *
* Determines if a character has the ISO Control property.
* <br>
* ISO Control = [Cc]
*
* @param ch character to test
* @return true if ch is an ISO Control character, else false
* @see #isSpaceChar(char)
* @see #isWhitespace(char)
* @since 1.1
)

: lang.Character.isISOControl§2116446464
   LOCAL ch
   
   \ new statement
   
   \ new statement
   ch
   lang.Character.isISOControl§1198614784
   0 §break17861 BRANCH
   
   \ new statement
   0 §break17861 LABEL
   
   \ new statement
   
   PURGE 1
   
   \ new statement
   DROP
;
( *
* Determines if a character is ignorable in a Unicode identifier. This
* includes the non-whitespace ISO control characters (<code>'\u0000'</code>
* through <code>'\u0008'</code>, <code>'\u000E'</code> through
* <code>'\u001B'</code>, and <code>'\u007F'</code> through
* <code>'\u009F'</code>), and FORMAT characters.
* <br>
* Unicode identifier ignorable = [Cf]|U+0000-U+0008|U+000E-U+001B
*    |U+007F-U+009F
*
* @param codePoint character to test
* @return true if ch is ignorable in a Unicode or Java identifier
* @see #isJavaIdentifierPart(char)
* @see #isUnicodeIdentifierPart(char)
* @since 1.5
)

: lang.Character.isIdentifierIgnorable§-1970902784
   LOCAL codePoint
   
   \ new statement
   
   \ new statement
   codePoint
   0< INVERT and_18148 0BRANCH! DROP
   codePoint
   8
   > INVERT
   and_18148 LABEL
   0=! or_18147 0BRANCH DROP
   codePoint
   14
   < INVERT and_18149 0BRANCH! DROP
   codePoint
   27
   > INVERT
   and_18149 LABEL
   0=! or_18147 0BRANCH DROP
   codePoint
   127
   < INVERT and_18150 0BRANCH! DROP
   codePoint
   159
   > INVERT
   and_18150 LABEL
   or_18147 LABEL
   
   \ new statement
   IF
      
      \ new statement
      TRUE
      0 §break17851 BRANCH
      
      \ new statement
   ENDIF
   
   \ new statement
   ( || getType(codePoint) == FORMAT )
   
   \ new statement
   FALSE
   0 §break17851 BRANCH
   
   \ new statement
   0 §break17851 LABEL
   
   \ new statement
   
   PURGE 1
   
   \ new statement
   DROP
;
( *
* Determines if a character is ignorable in a Unicode identifier. This
* includes the non-whitespace ISO control characters (<code>'\u0000'</code>
* through <code>'\u0008'</code>, <code>'\u000E'</code> through
* <code>'\u001B'</code>, and <code>'\u007F'</code> through
* <code>'\u009F'</code>), and FORMAT characters.
* <br>
* Unicode identifier ignorable = [Cf]|U+0000-U+0008|U+000E-U+001B
*    |U+007F-U+009F
*
* @param ch character to test
* @return true if ch is ignorable in a Unicode or Java identifier
* @see #isJavaIdentifierPart(char)
* @see #isUnicodeIdentifierPart(char)
* @since 1.1
)

: lang.Character.isIdentifierIgnorable§-35952384
   LOCAL ch
   
   \ new statement
   
   \ new statement
   ch
   lang.Character.isIdentifierIgnorable§-1970902784
   0 §break17850 BRANCH
   
   \ new statement
   0 §break17850 LABEL
   
   \ new statement
   
   PURGE 1
   
   \ new statement
   DROP
;
( *
* Determines if a character can follow the first letter in
* a Java identifier.  This is the combination of isJavaLetter (isLetter,
* type of LETTER_NUMBER, currency, connecting punctuation) and digit,
* numeric letter (like Roman numerals), combining marks, non-spacing marks,
* or isIdentifierIgnorable.
* <br>
* Java identifier extender =
*   [Lu]|[Ll]|[Lt]|[Lm]|[Lo]|[Nl]|[Sc]|[Pc]|[Mn]|[Mc]|[Nd]|[Cf]
*   |U+0000-U+0008|U+000E-U+001B|U+007F-U+009F
*
* @param codePoint character to test
* @return true if ch can follow the first letter in a Java identifier
* @see #isIdentifierIgnorable(char)
* @see #isJavaIdentifierStart(char)
* @see #isLetterOrDigit(char)
* @see #isUnicodeIdentifierPart(char)
* @since 1.5
)

: lang.Character.isJavaIdentifierPart§-1727633152
   LOCAL codePoint
   
   \ new statement
   
   \ new statement
   \ int category = getType(codePoint);
   \ return ((1 << category)
   \         & ((1 << UPPERCASE_LETTER)
   \            | (1 << LOWERCASE_LETTER)
   \            | (1 << TITLECASE_LETTER)
   \            | (1 << MODIFIER_LETTER)
   \            | (1 << OTHER_LETTER)
   \            | (1 << NON_SPACING_MARK)
   \            | (1 << COMBINING_SPACING_MARK)
   \            | (1 << DECIMAL_DIGIT_NUMBER)
   \            | (1 << LETTER_NUMBER)
   \            | (1 << CURRENCY_SYMBOL)
   \            | (1 << CONNECTOR_PUNCTUATION)
   \            | (1 << FORMAT))) != 0
   \   || (category == CONTROL && isIdentifierIgnorable(codePoint));
   
   
   \ new statement
   codePoint
   lang.Character.isJavaIdentifierStart§-1180604160
   0=! or_18167 0BRANCH DROP
   codePoint
   lang.Character.isDigit§1363241216
   0=! or_18167 0BRANCH DROP
   codePoint
   46
   =
   or_18167 LABEL
   0 §break17845 BRANCH
   
   \ new statement
   0 §break17845 LABEL
   
   \ new statement
   
   PURGE 1
   
   \ new statement
   DROP
;
( *
* Determines if a character can follow the first letter in
* a Java identifier.  This is the combination of isJavaLetter (isLetter,
* type of LETTER_NUMBER, currency, connecting punctuation) and digit,
* numeric letter (like Roman numerals), combining marks, non-spacing marks,
* or isIdentifierIgnorable.
* <br>
* Java identifier extender =
*   [Lu]|[Ll]|[Lt]|[Lm]|[Lo]|[Nl]|[Sc]|[Pc]|[Mn]|[Mc]|[Nd]|[Cf]
*   |U+0000-U+0008|U+000E-U+001B|U+007F-U+009F
*
* @param ch character to test
* @return true if ch can follow the first letter in a Java identifier
* @see #isIdentifierIgnorable(char)
* @see #isJavaIdentifierStart(char)
* @see #isLetterOrDigit(char)
* @see #isUnicodeIdentifierPart(char)
* @since 1.1
)

: lang.Character.isJavaIdentifierPart§2042652928
   LOCAL ch
   
   \ new statement
   
   \ new statement
   ch
   lang.Character.isJavaIdentifierPart§-1727633152
   0 §break17844 BRANCH
   
   \ new statement
   0 §break17844 LABEL
   
   \ new statement
   
   PURGE 1
   
   \ new statement
   DROP
;
( *
* Determines if a character can start a Java identifier. This is the
* combination of isLetter, any character where getType returns
* LETTER_NUMBER, currency symbols (like '$'), and connecting punctuation
* (like '_').
* <br>
* Java identifier start = [Lu]|[Ll]|[Lt]|[Lm]|[Lo]|[Nl]|[Sc]|[Pc]
*
* @param codePoint character to test
* @return true if ch can start a Java identifier, else false
* @see #isJavaIdentifierPart(char)
* @see #isLetter(char)
* @see #isUnicodeIdentifierStart(char)
* @since 1.5
)

: lang.Character.isJavaIdentifierStart§-1180604160
   LOCAL codePoint
   
   \ new statement
   
   \ new statement
   \ return ((1 << getType(codePoint))
   \         & ((1 << UPPERCASE_LETTER)
   \            | (1 << LOWERCASE_LETTER)
   \            | (1 << TITLECASE_LETTER)
   \            | (1 << MODIFIER_LETTER)
   \            | (1 << OTHER_LETTER)
   \            | (1 << LETTER_NUMBER)
   \            | (1 << CURRENCY_SYMBOL)
   \            | (1 << CONNECTOR_PUNCTUATION))) != 0;
   
   
   \ new statement
   codePoint
   lang.Character.isLetter§-1400936192
   0=! or_18179 0BRANCH DROP
   codePoint
   36
   =
   0=! or_18179 0BRANCH DROP
   codePoint
   95
   =
   or_18179 LABEL
   0 §break17843 BRANCH
   
   \ new statement
   0 §break17843 LABEL
   
   \ new statement
   
   PURGE 1
   
   \ new statement
   DROP
;
( *
* Determines if a character can start a Java identifier. This is the
* combination of isLetter, any character where getType returns
* LETTER_NUMBER, currency symbols (like '$'), and connecting punctuation
* (like '_').
* <br>
* Java identifier start = [Lu]|[Ll]|[Lt]|[Lm]|[Lo]|[Nl]|[Sc]|[Pc]
*
* @param ch character to test
* @return true if ch can start a Java identifier, else false
* @see #isJavaIdentifierPart(char)
* @see #isLetter(char)
* @see #isUnicodeIdentifierStart(char)
* @since 1.1
)

: lang.Character.isJavaIdentifierStart§-1470797568
   LOCAL ch
   
   \ new statement
   
   \ new statement
   ch
   lang.Character.isJavaIdentifierStart§-1180604160
   0 §break17842 BRANCH
   
   \ new statement
   0 §break17842 LABEL
   
   \ new statement
   
   PURGE 1
   
   \ new statement
   DROP
;
( *
* Determines if a character can start a Java identifier. This is the
* combination of isLetter, any character where getType returns
* LETTER_NUMBER, currency symbols (like '$'), and connecting punctuation
* (like '_').
*
* @param ch character to test
* @return true if ch can start a Java identifier, else false
* @deprecated Replaced by {@link #isJavaIdentifierStart(char)}
* @see #isJavaLetterOrDigit(char)
* @see #isJavaIdentifierStart(char)
* @see #isJavaIdentifierPart(char)
* @see #isLetter(char)
* @see #isLetterOrDigit(char)
* @see #isUnicodeIdentifierStart(char)
)

: lang.Character.isJavaLetter§1510172928
   LOCAL ch
   
   \ new statement
   
   \ new statement
   ch
   lang.Character.isJavaIdentifierStart§-1470797568
   0 §break17840 BRANCH
   
   \ new statement
   0 §break17840 LABEL
   
   \ new statement
   
   PURGE 1
   
   \ new statement
   DROP
;
( *
* Determines if a character can follow the first letter in
* a Java identifier.  This is the combination of isJavaLetter (isLetter,
* type of LETTER_NUMBER, currency, connecting punctuation) and digit,
* numeric letter (like Roman numerals), combining marks, non-spacing marks,
* or isIdentifierIgnorable.
*
* @param ch character to test
* @return true if ch can follow the first letter in a Java identifier
* @deprecated Replaced by {@link #isJavaIdentifierPart(char)}
* @see #isJavaLetter(char)
* @see #isJavaIdentifierStart(char)
* @see #isJavaIdentifierPart(char)
* @see #isLetter(char)
* @see #isLetterOrDigit(char)
* @see #isUnicodeIdentifierPart(char)
* @see #isIdentifierIgnorable(char)
)

: lang.Character.isJavaLetterOrDigit§-16422656
   LOCAL ch
   
   \ new statement
   
   \ new statement
   ch
   lang.Character.isJavaIdentifierPart§2042652928
   0 §break17841 BRANCH
   
   \ new statement
   0 §break17841 LABEL
   
   \ new statement
   
   PURGE 1
   
   \ new statement
   DROP
;
( *
* Determines if a character is a Unicode letter. Not all letters have case,
* so this may return true when isLowerCase and isUpperCase return false.
* A character is a Unicode letter if getType() returns one of
* UPPERCASE_LETTER, LOWERCASE_LETTER, TITLECASE_LETTER, MODIFIER_LETTER,
* or OTHER_LETTER.
* <br>
* letter = [Lu]|[Ll]|[Lt]|[Lm]|[Lo]
*
* @param codePoint character to test
* @return true if ch is a Unicode letter, else false
* @see #isDigit(char)
* @see #isJavaIdentifierStart(char)
* @see #isJavaLetter(char)
* @see #isJavaLetterOrDigit(char)
* @see #isLetterOrDigit(char)
* @see #isLowerCase(char)
* @see #isTitleCase(char)
* @see #isUnicodeIdentifierStart(char)
* @see #isUpperCase(char)
*
* @since 1.5
)

: lang.Character.isLetter§-1400936192
   LOCAL codePoint
   
   \ new statement
   
   \ new statement
   \ return ((1 << getType(codePoint))
   \     & ((1 << UPPERCASE_LETTER)
   \         | (1 << LOWERCASE_LETTER)
   \       | (1 << TITLECASE_LETTER)
   \     | (1 << MODIFIER_LETTER)
   \   | (1 << OTHER_LETTER))) != 0;
   
   
   \ new statement
   codePoint
   lang.Character.isUpperCase§-821008128
   0=! or_18201 0BRANCH DROP
   codePoint
   lang.Character.isLowerCase§-728405760
   or_18201 LABEL
   0 §break17837 BRANCH
   
   \ new statement
   0 §break17837 LABEL
   
   \ new statement
   
   PURGE 1
   
   \ new statement
   DROP
;
( *
* Determines if a character is a Unicode letter. Not all letters have case,
* so this may return true when isLowerCase and isUpperCase return false.
* A character is a Unicode letter if getType() returns one of
* UPPERCASE_LETTER, LOWERCASE_LETTER, TITLECASE_LETTER, MODIFIER_LETTER,
* or OTHER_LETTER.
* <br>
* letter = [Lu]|[Ll]|[Lt]|[Lm]|[Lo]
*
* @param ch character to test
* @return true if ch is a Unicode letter, else false
* @see #isDigit(char)
* @see #isJavaIdentifierStart(char)
* @see #isJavaLetter(char)
* @see #isJavaLetterOrDigit(char)
* @see #isLetterOrDigit(char)
* @see #isLowerCase(char)
* @see #isTitleCase(char)
* @see #isUnicodeIdentifierStart(char)
* @see #isUpperCase(char)
)

: lang.Character.isLetter§-1749849856
   LOCAL ch
   
   \ new statement
   
   \ new statement
   ch
   lang.Character.isLetter§-1400936192
   0 §break17836 BRANCH
   
   \ new statement
   0 §break17836 LABEL
   
   \ new statement
   
   PURGE 1
   
   \ new statement
   DROP
;
( *
* Determines if a character is a Unicode letter or a Unicode digit. This
* is the combination of isLetter and isDigit.
* <br>
* letter or digit = [Lu]|[Ll]|[Lt]|[Lm]|[Lo]|[Nd]
*
* @param codePoint character to test
* @return true if ch is a Unicode letter or a Unicode digit, else false
* @see #isDigit(char)
* @see #isJavaIdentifierPart(char)
* @see #isJavaLetter(char)
* @see #isJavaLetterOrDigit(char)
* @see #isLetter(char)
* @see #isUnicodeIdentifierPart(char)
*
* @since 1.5
)

: lang.Character.isLetterOrDigit§-1311414016
   LOCAL codePoint
   
   \ new statement
   
   \ new statement
   \ return ((1 << getType(codePoint))
   \     & ((1 << UPPERCASE_LETTER)
   \        | (1 << LOWERCASE_LETTER)
   \        | (1 << TITLECASE_LETTER)
   \        | (1 << MODIFIER_LETTER)
   \        | (1 << OTHER_LETTER)
   \        | (1 << DECIMAL_DIGIT_NUMBER))) != 0;
   
   
   \ new statement
   codePoint
   lang.Character.isLetter§-1400936192
   0=! or_18213 0BRANCH DROP
   codePoint
   lang.Character.isDigit§1363241216
   or_18213 LABEL
   0 §break17839 BRANCH
   
   \ new statement
   0 §break17839 LABEL
   
   \ new statement
   
   PURGE 1
   
   \ new statement
   DROP
;
( *
* Determines if a character is a Unicode letter or a Unicode digit. This
* is the combination of isLetter and isDigit.
* <br>
* letter or digit = [Lu]|[Ll]|[Lt]|[Lm]|[Lo]|[Nd]
*
* @param ch character to test
* @return true if ch is a Unicode letter or a Unicode digit, else false
* @see #isDigit(char)
* @see #isJavaIdentifierPart(char)
* @see #isJavaLetter(char)
* @see #isJavaLetterOrDigit(char)
* @see #isLetter(char)
* @see #isUnicodeIdentifierPart(char)
)

: lang.Character.isLetterOrDigit§-1644271360
   LOCAL ch
   
   \ new statement
   
   \ new statement
   ch
   lang.Character.isLetterOrDigit§-1311414016
   0 §break17838 BRANCH
   
   \ new statement
   0 §break17838 LABEL
   
   \ new statement
   
   PURGE 1
   
   \ new statement
   DROP
;
( *
* Return true if the given character is a low surrogate.
* @param ch the character
* @return true if the character is a low surrogate character
*
* @since 1.5
)

: lang.Character.isLowSurrogate§879388928
   LOCAL ch
   
   \ new statement
   
   \ new statement
   ch
   lang.Character.MIN_LOW_SURROGATE
   H@
   < INVERT and_18224 0BRANCH! DROP
   ch
   lang.Character.MAX_LOW_SURROGATE
   H@
   > INVERT
   and_18224 LABEL
   0 §break17878 BRANCH
   
   \ new statement
   0 §break17878 LABEL
   
   \ new statement
   
   PURGE 1
   
   \ new statement
   DROP
;
( *
* Determines if a character is a Unicode lowercase letter. For example,
* <code>'a'</code> is lowercase.  Returns true if getType() returns
* LOWERCASE_LETTER.
* <br>
* lowercase = [Ll]
*
* @param ch character to test
* @return true if ch is a Unicode lowercase letter, else false
* @see #isUpperCase(char)
* @see #isTitleCase(char)
* @see #toLowerCase(char)
* @see #getType(char)
)

: lang.Character.isLowerCase§-543201024
   LOCAL ch
   
   \ new statement
   
   \ new statement
   ch
   lang.Character.isLowerCase§-728405760
   0 §break17830 BRANCH
   
   \ new statement
   0 §break17830 LABEL
   
   \ new statement
   
   PURGE 1
   
   \ new statement
   DROP
;
( *
* Determines if a character is a Unicode lowercase letter. For example,
* <code>'a'</code> is lowercase.  Returns true if getType() returns
* LOWERCASE_LETTER.
* <br>
* lowercase = [Ll]
*
* @param codePoint character to test
* @return true if ch is a Unicode lowercase letter, else false
* @see #isUpperCase(char)
* @see #isTitleCase(char)
* @see #toLowerCase(char)
* @see #getType(char)
*
* @since 1.5
)

: lang.Character.isLowerCase§-728405760
   LOCAL codePoint
   
   \ new statement
   
   \ new statement
   \ return getType(codePoint) == LOWERCASE_LETTER;
   
   
   \ new statement
   codePoint
   97
   < INVERT and_18236 0BRANCH! DROP
   codePoint
   122
   > INVERT
   and_18236 LABEL
   0 §break17831 BRANCH
   
   \ new statement
   0 §break17831 LABEL
   
   \ new statement
   
   PURGE 1
   
   \ new statement
   DROP
;
( *
* Determines if a character is a ISO-LATIN-1 space. This is only the five
* characters <code>'\t'</code>, <code>'\n'</code>, <code>'\f'</code>,
* <code>'\r'</code>, and <code>' '</code>.
* <br>
* Java space = U+0020|U+0009|U+000A|U+000C|U+000D
*
* @param ch character to test
* @return true if ch is a space, else false
* @deprecated Replaced by {@link #isWhitespace(char)}
* @see #isSpaceChar(char)
* @see #isWhitespace(char)
)

: lang.Character.isSpace§1874225408
   LOCAL ch
   
   \ new statement
   
   \ new statement
   \  Performing the subtraction up front alleviates need to compare longs.
   
   
   \ new statement
   ch DUP 1- TO ch
   32
   > INVERT and_18243 0BRANCH! DROP
   ch
   31
   =
   0=! or_18244 0BRANCH DROP
   ch
   8
   =
   0=! or_18244 0BRANCH DROP
   ch
   9
   =
   0=! or_18244 0BRANCH DROP
   ch
   12
   =
   0=! or_18244 0BRANCH DROP
   ch
   11
   =
   or_18244 LABEL
   and_18243 LABEL
   0 §break17857 BRANCH
   
   \ new statement
   0 §break17857 LABEL
   
   \ new statement
   
   PURGE 1
   
   \ new statement
   DROP
;
( *
* Determines if a character is a Unicode space character. This includes
* SPACE_SEPARATOR, LINE_SEPARATOR, and PARAGRAPH_SEPARATOR.
* <br>
* Unicode space = [Zs]|[Zp]|[Zl]
*
* @param ch character to test
* @return true if ch is a Unicode space, else false
* @see #isWhitespace(char)
* @since 1.1
)

: lang.Character.isSpaceChar§-1862309632
   LOCAL ch
   
   \ new statement
   
   \ new statement
   ch
   lang.Character.isSpace§1874225408
   0 §break17858 BRANCH
   
   \ new statement
   0 §break17858 LABEL
   
   \ new statement
   
   PURGE 1
   
   \ new statement
   DROP
;
( *
* Determines whether the specified code point is
* in the range 0x10000 .. 0x10FFFF, i.e. the character is within the Unicode
* supplementary character range.
*
* @param codePoint a Unicode code point
*
* @return <code>true</code> if code point is in supplementary range
*
* @since 1.5
)

: lang.Character.isSupplementaryCodePoint§2137155840
   LOCAL codePoint
   
   \ new statement
   
   \ new statement
   codePoint
   lang.Character.MIN_SUPPLEMENTARY_CODE_POINT
   @
   < INVERT and_18255 0BRANCH! DROP
   codePoint
   lang.Character.MAX_CODE_POINT
   @
   > INVERT
   and_18255 LABEL
   0 §break17875 BRANCH
   
   \ new statement
   0 §break17875 LABEL
   
   \ new statement
   
   PURGE 1
   
   \ new statement
   DROP
;
( *
* Return true if the given characters compose a surrogate pair.
* This is true if the first character is a high surrogate and the
* second character is a low surrogate.
* @param ch1 the first character
* @param ch2 the first character
* @return true if the characters compose a surrogate pair
*
* @since 1.5
)

: lang.Character.isSurrogatePair§-664639232
   LOCAL ch2
   LOCAL ch1
   
   \ new statement
   
   \ new statement
   ch1
   lang.Character.isHighSurrogate§-311465728
   and_18261 0BRANCH! DROP
   ch2
   lang.Character.isLowSurrogate§879388928
   and_18261 LABEL
   0 §break17879 BRANCH
   
   \ new statement
   0 §break17879 LABEL
   
   \ new statement
   
   
   PURGE 2
   
   \ new statement
   DROP
;
( *
* Determines if a character can follow the first letter in
* a Unicode identifier. This includes letters, connecting punctuation,
* digits, numeric letters, combining marks, non-spacing marks, and
* isIdentifierIgnorable.
* <br>
* Unicode identifier extender =
*   [Lu]|[Ll]|[Lt]|[Lm]|[Lo]|[Nl]|[Mn]|[Mc]|[Nd]|[Pc]|[Cf]|
*   |U+0000-U+0008|U+000E-U+001B|U+007F-U+009F
*
* @param codePoint character to test
* @return true if ch can follow the first letter in a Unicode identifier
* @see #isIdentifierIgnorable(char)
* @see #isJavaIdentifierPart(char)
* @see #isLetterOrDigit(char)
* @see #isUnicodeIdentifierStart(char)
* @since 1.5
)

: lang.Character.isUnicodeIdentifierPart§1354459392
   LOCAL codePoint
   
   \ new statement
   
   \ new statement
   \ int category = getType(codePoint);
   \ return ((1 << category)
   \         & ((1 << UPPERCASE_LETTER)
   \            | (1 << LOWERCASE_LETTER)
   \            | (1 << TITLECASE_LETTER)
   \            | (1 << MODIFIER_LETTER)
   \            | (1 << OTHER_LETTER)
   \            | (1 << NON_SPACING_MARK)
   \            | (1 << COMBINING_SPACING_MARK)
   \            | (1 << DECIMAL_DIGIT_NUMBER)
   \            | (1 << LETTER_NUMBER)
   \            | (1 << CONNECTOR_PUNCTUATION)
   \            | (1 << FORMAT))) != 0
   \   || (category == CONTROL && isIdentifierIgnorable(codePoint));
   
   
   \ new statement
   codePoint
   lang.Character.isJavaIdentifierPart§-1727633152
   0 §break17849 BRANCH
   
   \ new statement
   0 §break17849 LABEL
   
   \ new statement
   
   PURGE 1
   
   \ new statement
   DROP
;
( *
* Determines if a character can follow the first letter in
* a Unicode identifier. This includes letters, connecting punctuation,
* digits, numeric letters, combining marks, non-spacing marks, and
* isIdentifierIgnorable.
* <br>
* Unicode identifier extender =
*   [Lu]|[Ll]|[Lt]|[Lm]|[Lo]|[Nl]|[Mn]|[Mc]|[Nd]|[Pc]|[Cf]|
*   |U+0000-U+0008|U+000E-U+001B|U+007F-U+009F
*
* @param ch character to test
* @return true if ch can follow the first letter in a Unicode identifier
* @see #isIdentifierIgnorable(char)
* @see #isJavaIdentifierPart(char)
* @see #isLetterOrDigit(char)
* @see #isUnicodeIdentifierStart(char)
* @since 1.1
)

: lang.Character.isUnicodeIdentifierPart§392653056
   LOCAL ch
   
   \ new statement
   
   \ new statement
   ch
   lang.Character.isUnicodeIdentifierPart§1354459392
   0 §break17848 BRANCH
   
   \ new statement
   0 §break17848 LABEL
   
   \ new statement
   
   PURGE 1
   
   \ new statement
   DROP
;
( *
* Determines if a character can start a Unicode identifier.  Only
* letters can start a Unicode identifier, but this includes characters
* in LETTER_NUMBER.
* <br>
* Unicode identifier start = [Lu]|[Ll]|[Lt]|[Lm]|[Lo]|[Nl]
*
* @param codePoint character to test
* @return true if ch can start a Unicode identifier, else false
* @see #isJavaIdentifierStart(char)
* @see #isLetter(char)
* @see #isUnicodeIdentifierPart(char)
* @since 1.5
)

: lang.Character.isUnicodeIdentifierStart§-687445760
   LOCAL codePoint
   
   \ new statement
   
   \ new statement
   \ return ((1 << getType(codePoint))
   \         & ((1 << UPPERCASE_LETTER)
   \            | (1 << LOWERCASE_LETTER)
   \            | (1 << TITLECASE_LETTER)
   \            | (1 << MODIFIER_LETTER)
   \            | (1 << OTHER_LETTER)
   \            | (1 << LETTER_NUMBER))) != 0;
   
   
   \ new statement
   codePoint
   lang.Character.isLetter§-1400936192
   0 §break17847 BRANCH
   
   \ new statement
   0 §break17847 LABEL
   
   \ new statement
   
   PURGE 1
   
   \ new statement
   DROP
;
( *
* Determines if a character can start a Unicode identifier.  Only
* letters can start a Unicode identifier, but this includes characters
* in LETTER_NUMBER.
* <br>
* Unicode identifier start = [Lu]|[Ll]|[Lt]|[Lm]|[Lo]|[Nl]
*
* @param ch character to test
* @return true if ch can start a Unicode identifier, else false
* @see #isJavaIdentifierStart(char)
* @see #isLetter(char)
* @see #isUnicodeIdentifierPart(char)
* @since 1.1
)

: lang.Character.isUnicodeIdentifierStart§2039376128
   LOCAL ch
   
   \ new statement
   
   \ new statement
   ch
   lang.Character.isUnicodeIdentifierStart§-687445760
   0 §break17846 BRANCH
   
   \ new statement
   0 §break17846 LABEL
   
   \ new statement
   
   PURGE 1
   
   \ new statement
   DROP
;
( *
* Determines if a character is a Unicode uppercase letter. For example,
* <code>'A'</code> is uppercase.  Returns true if getType() returns
* UPPERCASE_LETTER.
* <br>
* uppercase = [Lu]
*
* @param codePoint character to test
* @return true if ch is a Unicode uppercase letter, else false
* @see #isLowerCase(char)
* @see #isTitleCase(char)
* @see #toUpperCase(char)
* @see #getType(char)
*
* @since 1.5
)

: lang.Character.isUpperCase§-821008128
   LOCAL codePoint
   
   \ new statement
   
   \ new statement
   \ return getType(codePoint) == UPPERCASE_LETTER;
   
   
   \ new statement
   codePoint
   65
   < INVERT and_18290 0BRANCH! DROP
   codePoint
   90
   > INVERT
   and_18290 LABEL
   0 §break17833 BRANCH
   
   \ new statement
   0 §break17833 LABEL
   
   \ new statement
   
   PURGE 1
   
   \ new statement
   DROP
;
( *
* Determines if a character is a Unicode uppercase letter. For example,
* <code>'A'</code> is uppercase.  Returns true if getType() returns
* UPPERCASE_LETTER.
* <br>
* uppercase = [Lu]
*
* @param ch character to test
* @return true if ch is a Unicode uppercase letter, else false
* @see #isLowerCase(char)
* @see #isTitleCase(char)
* @see #toUpperCase(char)
* @see #getType(char)
)

: lang.Character.isUpperCase§203516160
   LOCAL ch
   
   \ new statement
   
   \ new statement
   ch
   lang.Character.isUpperCase§-821008128
   0 §break17832 BRANCH
   
   \ new statement
   0 §break17832 LABEL
   
   \ new statement
   
   PURGE 1
   
   \ new statement
   DROP
;
( *
* Determines whether the specified code point is
* in the range 0x0000 .. 0x10FFFF, i.e. it is a valid Unicode code point.
*
* @param codePoint a Unicode code point
*
* @return <code>true</code> if code point is valid
*
* @since 1.5
)

: lang.Character.isValidCodePoint§-155031296
   LOCAL codePoint
   
   \ new statement
   
   \ new statement
   codePoint
   lang.Character.MIN_CODE_POINT
   @
   < INVERT and_18301 0BRANCH! DROP
   codePoint
   lang.Character.MAX_CODE_POINT
   @
   > INVERT
   and_18301 LABEL
   0 §break17876 BRANCH
   
   \ new statement
   0 §break17876 LABEL
   
   \ new statement
   
   PURGE 1
   
   \ new statement
   DROP
;
( *
* Determines if a character is Java whitespace. This includes Unicode
* space characters (SPACE_SEPARATOR, LINE_SEPARATOR, and
* PARAGRAPH_SEPARATOR) except the non-breaking spaces
* (<code>'\u00A0'</code>, <code>'\u2007'</code>, and <code>'\u202F'</code>);
* and these characters: <code>'\u0009'</code>, <code>'\u000A'</code>,
* <code>'\u000B'</code>, <code>'\u000C'</code>, <code>'\u000D'</code>,
* <code>'\u001C'</code>, <code>'\u001D'</code>, <code>'\u001E'</code>,
* and <code>'\u001F'</code>.
* <br>
* Java whitespace = ([Zs] not Nb)|[Zl]|[Zp]|U+0009-U+000D|U+001C-U+001F
*
* @param codePoint character to test
* @return true if ch is Java whitespace, else false
* @see #isSpaceChar(char)
* @since 1.5
)

: lang.Character.isWhitespace§-441685760
   LOCAL codePoint
   
   \ new statement
   
   \ new statement
   codePoint
   9
   =
   0=! or_18307 0BRANCH DROP
   codePoint
   10
   =
   0=! or_18307 0BRANCH DROP
   codePoint
   11
   =
   0=! or_18307 0BRANCH DROP
   codePoint
   12
   =
   0=! or_18307 0BRANCH DROP
   codePoint
   13
   =
   0=! or_18307 0BRANCH DROP
   codePoint
   28
   =
   0=! or_18307 0BRANCH DROP
   codePoint
   29
   =
   0=! or_18307 0BRANCH DROP
   codePoint
   30
   =
   0=! or_18307 0BRANCH DROP
   codePoint
   31
   =
   or_18307 LABEL
   0 §break17860 BRANCH
   
   \ new statement
   0 §break17860 LABEL
   
   \ new statement
   
   PURGE 1
   
   \ new statement
   DROP
;
( *
* Determines if a character is Java whitespace. This includes Unicode
* space characters (SPACE_SEPARATOR, LINE_SEPARATOR, and
* PARAGRAPH_SEPARATOR) except the non-breaking spaces
* (<code>'\u00A0'</code>, <code>'\u2007'</code>, and <code>'\u202F'</code>);
* and these characters: <code>'\u0009'</code>, <code>'\u000A'</code>,
* <code>'\u000B'</code>, <code>'\u000C'</code>, <code>'\u000D'</code>,
* <code>'\u001C'</code>, <code>'\u001D'</code>, <code>'\u001E'</code>,
* and <code>'\u001F'</code>.
* <br>
* Java whitespace = ([Zs] not Nb)|[Zl]|[Zp]|U+0009-U+000D|U+001C-U+001F
*
* @param ch character to test
* @return true if ch is Java whitespace, else false
* @see #isSpaceChar(char)
* @since 1.1
)

: lang.Character.isWhitespace§1944873216
   LOCAL ch
   
   \ new statement
   
   \ new statement
   ch
   lang.Character.isWhitespace§-441685760
   0 §break17859 BRANCH
   
   \ new statement
   0 §break17859 LABEL
   
   \ new statement
   
   PURGE 1
   
   \ new statement
   DROP
;
( *
* Reverse the bytes in val.
* @since 1.5
)

: lang.Character.reverseBytes§1501590016
   LOCAL val
   
   \ new statement
   
   \ new statement
   val  S>D
   8
   ASHIFT  NIP 0FFFFH AND
   255
   AND
   val  S>D
   8
   SHIFTL  NIP 0FFFFH AND
   65280
   AND
   OR
   0FFFFH AND
   0 §break17868 BRANCH
   
   \ new statement
   0 §break17868 LABEL
   
   \ new statement
   
   PURGE 1
   
   \ new statement
   DROP
;
( *
* Converts a unicode code point to a UTF-16 representation of that
* code point.
*
* @param codePoint the unicode code point
*
* @return the UTF-16 representation of that code point
*
* @throws IllegalArgumentException if the code point is not a valid
*         unicode code point
*
* @since 1.5
)

: lang.Character.toChars§-410618880
   1 VALLOCATE LOCAL §base0
   LOCAL codePoint
   
   \ new statement
   0 DUP
   LOCALS ignore result |
   
   \ new statement
   codePoint
   lang.Character.isValidCodePoint§-155031296
   0=
   
   \ new statement
   IF
      
      \ new statement
      
      2 VALLOCATE LOCAL §tempvar
      U" Illegal Unicode code point :  " COUNT lang.JavaArray.createUnicode§-675323136 DUP §tempvar V!
      codePoint  <# 0 ..R lang.JavaArray.createString§-105880832 DUP §tempvar 4 + V!
      OVER  -1461427456 TRUE ( java.lang.String.concat§-1461427456 ) EXECUTE-METHOD
      lang.IllegalArgumentException§-946714368.table 737495296 EXECUTE-NEW
      lang.JavaArray.handler§-1096259584
      
      
      §tempvar SETVTOP
      
      PURGE 1
      
      
      \ new statement
      
      \ new statement
   ENDIF
   
   \ new statement
   
   1 VALLOCATE LOCAL §tempvar
   codePoint
   lang.Character.charCount§-1036950784
   131073 lang.JavaArray§1352878592.table -227194368 EXECUTE-NEW
   DUP §base0 V! TO result
   
   §tempvar SETVTOP
   
   PURGE 1
   
   
   \ new statement
   codePoint
   result
   0
   lang.Character.toChars§-707038208
   TO ignore
   
   \ new statement
   result
   DUP 0 V!0 §break17869 BRANCH
   
   \ new statement
   0 §break17869 LABEL
   
   \ new statement
   
   
   
   §base0 SETVTOP
   PURGE 4
   
   \ new statement
   DROP
;
( *
* Converts a unicode code point to its UTF-16 representation.
*
* @param codePoint the unicode code point
* @param dst the target char array
* @param dstIndex the start index for the target
*
* @return number of characters written to <code>dst</code>
*
* @throws IllegalArgumentException if <code>codePoint</code> is not a
*         valid unicode code point
* @throws NullPointerException if <code>dst</code> is <code>null</code>
* @throws IndexOutOfBoundsException if <code>dstIndex</code> is not valid
*         in <code>dst</code> or if the UTF-16 representation does not
*         fit into <code>dst</code>
*
* @since 1.5
)

: lang.Character.toChars§-707038208
   1 VALLOCATE LOCAL §base0
   LOCAL dstIndex
   DUP 0 §base0 + V! LOCAL dst
   LOCAL codePoint
   
   \ new statement
   0
   LOCALS result |
   
   \ new statement
   codePoint
   lang.Character.isValidCodePoint§-155031296
   0=
   
   \ new statement
   IF
      
      \ new statement
      
      \ new statement
      
      2 VALLOCATE LOCAL §tempvar
      U" not a valid code point:  " COUNT lang.JavaArray.createUnicode§-675323136 DUP §tempvar V!
      codePoint  <# 0 ..R lang.JavaArray.createString§-105880832 DUP §tempvar 4 + V!
      OVER  -1461427456 TRUE ( java.lang.String.concat§-1461427456 ) EXECUTE-METHOD
      lang.IllegalArgumentException§-946714368.table 737495296 EXECUTE-NEW
      lang.JavaArray.handler§-1096259584
      
      
      §tempvar SETVTOP
      
      PURGE 1
      
      
      \ new statement
      
      \ new statement
      0 §break17871 LABEL
      
      \ new statement
      
      \ new statement
      0>!
      IF
         10001H - §break17870 BRANCH
      ENDIF
      DROP
      
      \ new statement
   ENDIF
   
   \ new statement
   codePoint
   lang.Character.isSupplementaryCodePoint§2137155840
   
   \ new statement
   IF
      
      \ new statement
      0
      LOCALS cp2 |
      
      \ new statement
      \  Write second char first to cause IndexOutOfBoundsException
      \  immediately.
      
      
      \ new statement
      codePoint
      65536
      -
      TO cp2
      
      \ new statement
      
      0
      LOCALS 0§ |
      
      dstIndex
      1
      + TO 0§
      cp2
      A:R@ ABS A:10 A:XOR 1023 AND R> 0<
      IF
         NEGATE
      ENDIF
      lang.Character.MIN_LOW_SURROGATE
      H@ A:R@
      A:DROP
      +
      0FFFFH AND
      dst A:R@
      0§
      R> -942971136 TRUE ( java.lang.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
      H!
      
      PURGE 1
      
      
      \ new statement
      
      0
      LOCALS 0§ |
      
      dstIndex TO 0§
      cp2
      S>D 10 ASHIFT NIP
      lang.Character.MIN_HIGH_SURROGATE
      H@ A:R@
      A:DROP
      +
      0FFFFH AND
      dst A:R@
      0§
      R> -942971136 TRUE ( java.lang.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
      H!
      
      PURGE 1
      
      
      \ new statement
      2
      TO result
      
      \ new statement
      0 §break17872 LABEL
      
      \ new statement
      
      PURGE 1
      
      \ new statement
      0>!
      IF
         10001H - §break17870 BRANCH
      ENDIF
      DROP
      
      \ new statement
   ELSE
      
      \ new statement
      
      \ new statement
      
      0
      LOCALS 0§ |
      
      dstIndex TO 0§
      codePoint  0FFFFH AND
      dst A:R@
      0§
      R> -942971136 TRUE ( java.lang.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
      H!
      
      PURGE 1
      
      
      \ new statement
      1
      TO result
      
      \ new statement
      0 §break17873 LABEL
      
      \ new statement
      
      \ new statement
      0>!
      IF
         10001H - §break17870 BRANCH
      ENDIF
      DROP
      
      \ new statement
   ENDIF
   
   \ new statement
   result
   0 §break17870 BRANCH
   
   \ new statement
   0 §break17870 LABEL
   
   \ new statement
   
   
   
   
   §base0 SETVTOP
   PURGE 5
   
   \ new statement
   DROP
;
( *
* Given a valid surrogate pair, this returns the corresponding
* code point.
* @param high the high character of the pair
* @param low the low character of the pair
* @return the corresponding code point
*
* @since 1.5
)

: lang.Character.toCodePoint§-2064157696
   LOCAL low
   LOCAL high
   
   \ new statement
   
   \ new statement
   high
   lang.Character.MIN_HIGH_SURROGATE
   H@
   -
   S>D 10 SHIFTL NIP
   low
   lang.Character.MIN_LOW_SURROGATE
   H@
   -
   +
   65536
   +
   0 §break17880 BRANCH
   
   \ new statement
   0 §break17880 LABEL
   
   \ new statement
   
   
   PURGE 2
   
   \ new statement
   DROP
;
( *
* Converts a Unicode character into its lowercase equivalent mapping.
* If a mapping does not exist, then the character passed is returned.
* Note that isLowerCase(toLowerCase(ch)) does not always return true.
*
* @param ch character to convert to lowercase
* @return lowercase mapping of ch, or ch if lowercase mapping does
*         not exist
* @see #isLowerCase(char)
* @see #isUpperCase(char)
* @see #toTitleCase(char)
* @see #toUpperCase(char)
)

: lang.Character.toLowerCase§-1264749568
   LOCAL ch
   
   \ new statement
   
   \ new statement
   \ return (char) (lower[0][readCodePoint((int)ch) >>> 7] + ch);
   
   
   \ new statement
   ch
   lang.Character.isUpperCase§203516160
   
   \ new statement
   IF
      
      \ new statement
      ch
      32
      +  0FFFFH AND
      TO ch
      
      \ new statement
   ENDIF
   
   \ new statement
   ch
   0 §break17852 BRANCH
   
   \ new statement
   0 §break17852 LABEL
   
   \ new statement
   
   PURGE 1
   
   \ new statement
   DROP
;
( *
* Converts the wrapped character into a String.
*
* @return a String containing one character -- the wrapped character
*         of this instance
)

:LOCAL lang.Character.toString§1621718016
   1 VALLOCATE LOCAL §base0
   DUP 0 §base0 + V! LOCAL §this
   
   \ new statement
   
   \ new statement
   \  Package constructor avoids an array copy.
   
   
   \ new statement
   
   1 VALLOCATE LOCAL §tempvar
   1
   131073 lang.JavaArray§1352878592.table -227194368 EXECUTE-NEW DUP §tempvar V!
   §this CELL+ @ 28 + ( java.lang.Character.value )    H@ OVER 0  OVER -942971136 TRUE ( java.lang.JavaArray.getElem§-942971136 ) EXECUTE-METHOD  H!
   lang.String§1651856128.table -99003648 EXECUTE-NEW
   DUP 0 V!0 §break17828 BRANCH
   
   
   §tempvar SETVTOP
   
   PURGE 1
   
   
   \ new statement
   0 §break17828 LABEL
   
   \ new statement
   §base0 SETVTOP
   PURGE 2
   
   \ new statement
   DROP
;
( *
* Returns a String of length 1 representing the specified character.
*
* @param ch the character to convert
* @return a String containing the character
* @since 1.4
)

: lang.Character.toString§1882158080
   LOCAL ch
   
   \ new statement
   
   \ new statement
   \  Package constructor avoids an array copy.
   
   
   \ new statement
   
   1 VALLOCATE LOCAL §tempvar
   1
   131073 lang.JavaArray§1352878592.table -227194368 EXECUTE-NEW DUP §tempvar V!
   ch OVER 0  OVER -942971136 TRUE ( java.lang.JavaArray.getElem§-942971136 ) EXECUTE-METHOD  H!
   lang.String§1651856128.table -99003648 EXECUTE-NEW
   DUP 0 V!0 §break17829 BRANCH
   
   
   §tempvar SETVTOP
   
   PURGE 1
   
   
   \ new statement
   0 §break17829 LABEL
   
   \ new statement
   
   PURGE 1
   
   \ new statement
   DROP
;
( *
* Converts a Unicode character into its uppercase equivalent mapping.
* If a mapping does not exist, then the character passed is returned.
* Note that isUpperCase(toUpperCase(ch)) does not always return true.
*
* @param ch character to convert to uppercase
* @return uppercase mapping of ch, or ch if uppercase mapping does
*         not exist
* @see #isLowerCase(char)
* @see #isUpperCase(char)
* @see #toLowerCase(char)
* @see #toTitleCase(char)
)

: lang.Character.toUpperCase§1730376704
   LOCAL ch
   
   \ new statement
   
   \ new statement
   \ return (char) (upper[0][readCodePoint((int)ch) >>> 7] + ch);
   
   
   \ new statement
   ch
   lang.Character.isLowerCase§-543201024
   
   \ new statement
   IF
      
      \ new statement
      ch
      32
      -  0FFFFH AND
      TO ch
      
      \ new statement
   ENDIF
   
   \ new statement
   ch
   0 §break17853 BRANCH
   
   \ new statement
   0 §break17853 LABEL
   
   \ new statement
   
   PURGE 1
   
   \ new statement
   DROP
;


: lang.Character.toUpperCaseCharArray§706376704
   LOCAL ch
   
   \ new statement
   
   \ new statement
   
   1 VALLOCATE LOCAL §tempvar
   1
   131073 lang.JavaArray§1352878592.table -227194368 EXECUTE-NEW DUP §tempvar V!
   ch
   lang.Character.toUpperCase§1730376704
   OVER 0  OVER -942971136 TRUE ( java.lang.JavaArray.getElem§-942971136 ) EXECUTE-METHOD  H!
   DUP 0 V!0 §break17855 BRANCH
   
   
   §tempvar SETVTOP
   
   PURGE 1
   
   
   \ new statement
   0 §break17855 LABEL
   
   \ new statement
   
   PURGE 1
   
   \ new statement
   DROP
;


: lang.Character.toUpperCaseEx§498889728
   LOCAL ch
   
   \ new statement
   
   \ new statement
   \ return (char) (upper[0][readCodePoint((int)ch) >>> 7] + ch);
   
   
   \ new statement
   0
   lang.Character.CHAR_ERROR
   H!
   
   \ new statement
   ch
   lang.Character.isLowerCase§-543201024
   
   \ new statement
   IF
      
      \ new statement
      ch
      32
      -  0FFFFH AND
      TO ch
      
      \ new statement
   ELSE
      
      \ new statement
      ch
      lang.Character.isUpperCase§203516160
      0=
      
      \ new statement
      IF
         
         \ new statement
         ch
         lang.Character.CHAR_ERROR
         H!
         
         \ new statement
      ENDIF
      
      \ new statement
   ENDIF
   
   \ new statement
   ch
   0 §break17854 BRANCH
   
   \ new statement
   0 §break17854 LABEL
   
   \ new statement
   
   PURGE 1
   
   \ new statement
   DROP
;
( *
* Returns an <code>Character</code> object wrapping the value.
* In contrast to the <code>Character</code> constructor, this method
* will cache some values.  It is used by boxing conversion.
*
* @param val the value to wrap
* @return the <code>Character</code>
*
* @since 1.5
)

: lang.Character.valueOf§-555452928
   LOCAL val
   
   \ new statement
   
   \ new statement
   val
   lang.Character.MAX_CACHE
   @
   >
   
   \ new statement
   IF
      
      \ new statement
      val
      lang.Character§1114260224.table 849625856 EXECUTE-NEW
      DUP 0 V!0 §break17866 BRANCH
      
      \ new statement
   ENDIF
   
   \ new statement
   
   \ new statement
   lang.Character.charCache
   @
   LOCAL §synchronized0
   DI
   §synchronized0 A:R@
   lang.Character._staticThread  @
   R> 1472163072 TRUE ( Object.monitorEnter ) EXECUTE-METHOD
   EI
   
   \ new statement
   lang.Character.charCache  @ A:R@
   val
   lang.Character.MIN_VALUE
   H@
   -
   R> -942971136 TRUE ( java.lang.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
   @  " Character " CASTTO
   0=
   
   \ new statement
   IF
      
      \ new statement
      
      0
      LOCALS 0§ |
      
      val
      lang.Character.MIN_VALUE
      H@
      - TO 0§
      val
      lang.Character§1114260224.table 849625856 EXECUTE-NEW
      lang.Character.charCache  @ A:R@
      0§
      R> -942971136 TRUE ( java.lang.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
      OVER 0 V! !
      
      PURGE 1
      
      
      \ new statement
   ENDIF
   
   \ new statement
   lang.Character.charCache  @ A:R@
   val
   lang.Character.MIN_VALUE
   H@
   -
   R> -942971136 TRUE ( java.lang.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
   @  " Character " CASTTO
   DUP 0 V!65537 §break17867 BRANCH
   
   \ new statement
   0 §break17867 LABEL
   
   \ new statement
   §synchronized0 A:R@
   R> 1974299904 TRUE ( Object.monitorLeave ) EXECUTE-METHOD
   PURGE 1
   
   \ new statement
   
   \ new statement
   0>!
   IF
      10001H - §break17866 BRANCH
   ENDIF
   DROP
   
   \ new statement
   0 §break17866 LABEL
   
   \ new statement
   
   PURGE 1
   
   \ new statement
   DROP
;
VARIABLE lang.Character.CHAR_ERROR
VARIABLE lang.Character.COMBINING_SPACING_MARK
VARIABLE lang.Character.CONNECTOR_PUNCTUATION
VARIABLE lang.Character.CONTROL
VARIABLE lang.Character.CURRENCY_SYMBOL
VARIABLE lang.Character.DASH_PUNCTUATION
VARIABLE lang.Character.DECIMAL_DIGIT_NUMBER
VARIABLE lang.Character.DIRECTIONALITY_ARABIC_NUMBER
VARIABLE lang.Character.DIRECTIONALITY_BOUNDARY_NEUTRAL
VARIABLE lang.Character.DIRECTIONALITY_COMMON_NUMBER_SEPARATOR
VARIABLE lang.Character.DIRECTIONALITY_EUROPEAN_NUMBER
VARIABLE lang.Character.DIRECTIONALITY_EUROPEAN_NUMBER_SEPARATOR
VARIABLE lang.Character.DIRECTIONALITY_EUROPEAN_NUMBER_TERMINATOR
VARIABLE lang.Character.DIRECTIONALITY_LEFT_TO_RIGHT
VARIABLE lang.Character.DIRECTIONALITY_LEFT_TO_RIGHT_EMBEDDING
VARIABLE lang.Character.DIRECTIONALITY_LEFT_TO_RIGHT_OVERRIDE
VARIABLE lang.Character.DIRECTIONALITY_NONSPACING_MARK
VARIABLE lang.Character.DIRECTIONALITY_OTHER_NEUTRALS
VARIABLE lang.Character.DIRECTIONALITY_PARAGRAPH_SEPARATOR
VARIABLE lang.Character.DIRECTIONALITY_POP_DIRECTIONAL_FORMAT
VARIABLE lang.Character.DIRECTIONALITY_RIGHT_TO_LEFT
VARIABLE lang.Character.DIRECTIONALITY_RIGHT_TO_LEFT_ARABIC
VARIABLE lang.Character.DIRECTIONALITY_RIGHT_TO_LEFT_EMBEDDING
VARIABLE lang.Character.DIRECTIONALITY_RIGHT_TO_LEFT_OVERRIDE
VARIABLE lang.Character.DIRECTIONALITY_SEGMENT_SEPARATOR
VARIABLE lang.Character.DIRECTIONALITY_UNDEFINED
VARIABLE lang.Character.DIRECTIONALITY_WHITESPACE
VARIABLE lang.Character.ENCLOSING_MARK
VARIABLE lang.Character.END_PUNCTUATION
VARIABLE lang.Character.FINAL_QUOTE_PUNCTUATION
VARIABLE lang.Character.FORMAT
VARIABLE lang.Character.INITIAL_QUOTE_PUNCTUATION
VARIABLE lang.Character.LETTER_NUMBER
VARIABLE lang.Character.LINE_SEPARATOR
VARIABLE lang.Character.LOWERCASE_LETTER
VARIABLE lang.Character.MATH_SYMBOL
VARIABLE lang.Character.MAX_CACHE
VARIABLE lang.Character.MAX_CODE_POINT
VARIABLE lang.Character.MAX_HIGH_SURROGATE
VARIABLE lang.Character.MAX_LOW_SURROGATE
VARIABLE lang.Character.MAX_RADIX
VARIABLE lang.Character.MAX_SURROGATE
VARIABLE lang.Character.MAX_VALUE
VARIABLE lang.Character.MIN_CODE_POINT
VARIABLE lang.Character.MIN_HIGH_SURROGATE
VARIABLE lang.Character.MIN_LOW_SURROGATE
VARIABLE lang.Character.MIN_RADIX
VARIABLE lang.Character.MIN_SUPPLEMENTARY_CODE_POINT
VARIABLE lang.Character.MIN_SURROGATE
VARIABLE lang.Character.MIN_VALUE
VARIABLE lang.Character.MIRROR_MASK
VARIABLE lang.Character.MODIFIER_LETTER
VARIABLE lang.Character.MODIFIER_SYMBOL
VARIABLE lang.Character.NON_SPACING_MARK
VARIABLE lang.Character.NO_BREAK_MASK
VARIABLE lang.Character.OTHER_LETTER
VARIABLE lang.Character.OTHER_NUMBER
VARIABLE lang.Character.OTHER_PUNCTUATION
VARIABLE lang.Character.OTHER_SYMBOL
VARIABLE lang.Character.PARAGRAPH_SEPARATOR
VARIABLE lang.Character.PRIVATE_USE
VARIABLE lang.Character.SIZE
VARIABLE lang.Character.SPACE_SEPARATOR
VARIABLE lang.Character.START_PUNCTUATION
VARIABLE lang.Character.SURROGATE
VARIABLE lang.Character.TITLECASE_LETTER
VARIABLE lang.Character.TYPE_MASK
VARIABLE lang.Character.UNASSIGNED
VARIABLE lang.Character.UPPERCASE_LETTER
VARIABLE lang.Character._staticBlocking
VARIABLE lang.Character._staticThread
-1 VALLOCATE CONSTANT lang.Character.charCache
2VARIABLE lang.Character.serialVersionUID

A:HERE VARIABLE lang.Character§1114260224.table 8 DUP 2* CELLS ALLOT R@ ! A:CELL+
-1412545792 R@ ! A:CELL+ lang.Character.Character§-1412545792 VAL R@ ! A:CELL+
849625856 R@ ! A:CELL+ lang.Character.Character§849625856 VAL R@ ! A:CELL+
-63216896 R@ ! A:CELL+ lang.Character.charValue§-63216896 VAL R@ ! A:CELL+
-1220123904 R@ ! A:CELL+ lang.Character.compareTo§-1220123904 VAL R@ ! A:CELL+
-879140096 R@ ! A:CELL+ lang.Character.compareTo§-879140096 VAL R@ ! A:CELL+
-240098048 R@ ! A:CELL+ lang.Character.equals§-240098048 VAL R@ ! A:CELL+
-1604556800 R@ ! A:CELL+ lang.Character.hashCode§-1604556800 VAL R@ ! A:CELL+
1621718016 R@ ! A:CELL+ lang.Character.toString§1621718016 VAL R@ ! A:CELL+
A:DROP
