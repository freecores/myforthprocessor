
\ new statement
( Serializable, )

\ new statement
0
lang.Character._staticBlocking
!

\ new statement
0
lang.Character._staticThread
!

\ new statement
( *
* Compatible with JDK 1.0+.
)

\ new statement
3786198910865385080
lang.Character.serialVersionUID
2!

\ new statement
( *
* Smallest value allowed for radix arguments in Java. This value is 2.
*
* @see #digit(char, int)
* @see #forDigit(int, int)
* @see Integer#toString(int, int)
* @see Integer#valueOf(String)
)

\ new statement
2
lang.Character.MIN_RADIX
!

\ new statement
( *
* Largest value allowed for radix arguments in Java. This value is 36.
*
* @see #digit(char, int)
* @see #forDigit(int, int)
* @see Integer#toString(int, int)
* @see Integer#valueOf(String)
)

\ new statement
36
lang.Character.MAX_RADIX
!

\ new statement
( *
* The minimum value the char data type can hold.
* This value is <code>'\\u0000'</code>.
)

\ new statement
0
lang.Character.MIN_VALUE
H!

\ new statement
( *
* The maximum value the char data type can hold.
* This value is <code>'\\uFFFF'</code>.
)

\ new statement
65535
lang.Character.MAX_VALUE
H!

\ new statement
( *
* The number of bits needed to represent a <code>char</code>.
* @since 1.5
)

\ new statement
16
lang.Character.SIZE
!

\ new statement
\  This caches some Character values, and is used by boxing
\  conversions via valueOf().  We must cache at least 0..127;
\  this constant controls how much we actually cache.


\ new statement
127
lang.Character.MAX_CACHE
!

\ new statement

1 VALLOCATE LOCAL §tempvar
lang.Character.MAX_CACHE
@
1
+ 1114243073 lang.JavaArray§1352878592.table -227194368 EXECUTE-NEW
lang.Character.charCache
OVER 0 V! !

§tempvar SETVTOP

PURGE 1


\ new statement
( *
* Lu = Letter, Uppercase (Informative).
*
* @since 1.1
)

\ new statement
1
lang.Character.UPPERCASE_LETTER
C!

\ new statement
( *
* Ll = Letter, Lowercase (Informative).
*
* @since 1.1
)

\ new statement
2
lang.Character.LOWERCASE_LETTER
C!

\ new statement
( *
* Lt = Letter, Titlecase (Informative).
*
* @since 1.1
)

\ new statement
3
lang.Character.TITLECASE_LETTER
C!

\ new statement
( *
* Mn = Mark, Non-Spacing (Normative).
*
* @since 1.1
)

\ new statement
6
lang.Character.NON_SPACING_MARK
C!

\ new statement
( *
* Mc = Mark, Spacing Combining (Normative).
*
* @since 1.1
)

\ new statement
8
lang.Character.COMBINING_SPACING_MARK
C!

\ new statement
( *
* Me = Mark, Enclosing (Normative).
*
* @since 1.1
)

\ new statement
7
lang.Character.ENCLOSING_MARK
C!

\ new statement
( *
* Nd = Number, Decimal Digit (Normative).
*
* @since 1.1
)

\ new statement
9
lang.Character.DECIMAL_DIGIT_NUMBER
C!

\ new statement
( *
* Nl = Number, Letter (Normative).
*
* @since 1.1
)

\ new statement
10
lang.Character.LETTER_NUMBER
C!

\ new statement
( *
* No = Number, Other (Normative).
*
* @since 1.1
)

\ new statement
11
lang.Character.OTHER_NUMBER
C!

\ new statement
( *
* Zs = Separator, Space (Normative).
*
* @since 1.1
)

\ new statement
12
lang.Character.SPACE_SEPARATOR
C!

\ new statement
( *
* Zl = Separator, Line (Normative).
*
* @since 1.1
)

\ new statement
13
lang.Character.LINE_SEPARATOR
C!

\ new statement
( *
* Zp = Separator, Paragraph (Normative).
*
* @since 1.1
)

\ new statement
14
lang.Character.PARAGRAPH_SEPARATOR
C!

\ new statement
( *
* Cc = Other, Control (Normative).
*
* @since 1.1
)

\ new statement
15
lang.Character.CONTROL
C!

\ new statement
( *
* Cf = Other, Format (Normative).
*
* @since 1.1
)

\ new statement
16
lang.Character.FORMAT
C!

\ new statement
( *
* Cs = Other, Surrogate (Normative).
*
* @since 1.1
)

\ new statement
19
lang.Character.SURROGATE
C!

\ new statement
( *
* Co = Other, Private Use (Normative).
*
* @since 1.1
)

\ new statement
18
lang.Character.PRIVATE_USE
C!

\ new statement
( *
* Cn = Other, Not Assigned (Normative).
*
* @since 1.1
)

\ new statement
0
lang.Character.UNASSIGNED
C!

\ new statement
( *
* Lm = Letter, Modifier (Informative).
*
* @since 1.1
)

\ new statement
4
lang.Character.MODIFIER_LETTER
C!

\ new statement
( *
* Lo = Letter, Other (Informative).
*
* @since 1.1
)

\ new statement
5
lang.Character.OTHER_LETTER
C!

\ new statement
( *
* Pc = Punctuation, Connector (Informative).
*
* @since 1.1
)

\ new statement
23
lang.Character.CONNECTOR_PUNCTUATION
C!

\ new statement
( *
* Pd = Punctuation, Dash (Informative).
*
* @since 1.1
)

\ new statement
20
lang.Character.DASH_PUNCTUATION
C!

\ new statement
( *
* Ps = Punctuation, Open (Informative).
*
* @since 1.1
)

\ new statement
21
lang.Character.START_PUNCTUATION
C!

\ new statement
( *
* Pe = Punctuation, Close (Informative).
*
* @since 1.1
)

\ new statement
22
lang.Character.END_PUNCTUATION
C!

\ new statement
( *
* Pi = Punctuation, Initial Quote (Informative).
*
* @since 1.4
)

\ new statement
29
lang.Character.INITIAL_QUOTE_PUNCTUATION
C!

\ new statement
( *
* Pf = Punctuation, Final Quote (Informative).
*
* @since 1.4
)

\ new statement
30
lang.Character.FINAL_QUOTE_PUNCTUATION
C!

\ new statement
( *
* Po = Punctuation, Other (Informative).
*
* @since 1.1
)

\ new statement
24
lang.Character.OTHER_PUNCTUATION
C!

\ new statement
( *
* Sm = Symbol, Math (Informative).
*
* @since 1.1
)

\ new statement
25
lang.Character.MATH_SYMBOL
C!

\ new statement
( *
* Sc = Symbol, Currency (Informative).
*
* @since 1.1
)

\ new statement
26
lang.Character.CURRENCY_SYMBOL
C!

\ new statement
( *
* Sk = Symbol, Modifier (Informative).
*
* @since 1.1
)

\ new statement
27
lang.Character.MODIFIER_SYMBOL
C!

\ new statement
( *
* So = Symbol, Other (Informative).
*
* @since 1.1
)

\ new statement
28
lang.Character.OTHER_SYMBOL
C!

\ new statement
( *
* Undefined bidirectional character type. Undefined char values have
* undefined directionality in the Unicode specification.
*
* @since 1.4
)

\ new statement
-1
lang.Character.DIRECTIONALITY_UNDEFINED
C!

\ new statement
( *
* Strong bidirectional character type "L".
*
* @since 1.4
)

\ new statement
0
lang.Character.DIRECTIONALITY_LEFT_TO_RIGHT
C!

\ new statement
( *
* Strong bidirectional character type "R".
*
* @since 1.4
)

\ new statement
1
lang.Character.DIRECTIONALITY_RIGHT_TO_LEFT
C!

\ new statement
( *
* Strong bidirectional character type "AL".
*
* @since 1.4
)

\ new statement
2
lang.Character.DIRECTIONALITY_RIGHT_TO_LEFT_ARABIC
C!

\ new statement
( *
* Weak bidirectional character type "EN".
*
* @since 1.4
)

\ new statement
3
lang.Character.DIRECTIONALITY_EUROPEAN_NUMBER
C!

\ new statement
( *
* Weak bidirectional character type "ES".
*
* @since 1.4
)

\ new statement
4
lang.Character.DIRECTIONALITY_EUROPEAN_NUMBER_SEPARATOR
C!

\ new statement
( *
* Weak bidirectional character type "ET".
*
* @since 1.4
)

\ new statement
5
lang.Character.DIRECTIONALITY_EUROPEAN_NUMBER_TERMINATOR
C!

\ new statement
( *
* Weak bidirectional character type "AN".
*
* @since 1.4
)

\ new statement
6
lang.Character.DIRECTIONALITY_ARABIC_NUMBER
C!

\ new statement
( *
* Weak bidirectional character type "CS".
*
* @since 1.4
)

\ new statement
7
lang.Character.DIRECTIONALITY_COMMON_NUMBER_SEPARATOR
C!

\ new statement
( *
* Weak bidirectional character type "NSM".
*
* @since 1.4
)

\ new statement
8
lang.Character.DIRECTIONALITY_NONSPACING_MARK
C!

\ new statement
( *
* Weak bidirectional character type "BN".
*
* @since 1.4
)

\ new statement
9
lang.Character.DIRECTIONALITY_BOUNDARY_NEUTRAL
C!

\ new statement
( *
* Neutral bidirectional character type "B".
*
* @since 1.4
)

\ new statement
10
lang.Character.DIRECTIONALITY_PARAGRAPH_SEPARATOR
C!

\ new statement
( *
* Neutral bidirectional character type "S".
*
* @since 1.4
)

\ new statement
11
lang.Character.DIRECTIONALITY_SEGMENT_SEPARATOR
C!

\ new statement
( *
* Strong bidirectional character type "WS".
*
* @since 1.4
)

\ new statement
12
lang.Character.DIRECTIONALITY_WHITESPACE
C!

\ new statement
( *
* Neutral bidirectional character type "ON".
*
* @since 1.4
)

\ new statement
13
lang.Character.DIRECTIONALITY_OTHER_NEUTRALS
C!

\ new statement
( *
* Strong bidirectional character type "LRE".
*
* @since 1.4
)

\ new statement
14
lang.Character.DIRECTIONALITY_LEFT_TO_RIGHT_EMBEDDING
C!

\ new statement
( *
* Strong bidirectional character type "LRO".
*
* @since 1.4
)

\ new statement
15
lang.Character.DIRECTIONALITY_LEFT_TO_RIGHT_OVERRIDE
C!

\ new statement
( *
* Strong bidirectional character type "RLE".
*
* @since 1.4
)

\ new statement
16
lang.Character.DIRECTIONALITY_RIGHT_TO_LEFT_EMBEDDING
C!

\ new statement
( *
* Strong bidirectional character type "RLO".
*
* @since 1.4
)

\ new statement
17
lang.Character.DIRECTIONALITY_RIGHT_TO_LEFT_OVERRIDE
C!

\ new statement
( *
* Weak bidirectional character type "PDF".
*
* @since 1.4
)

\ new statement
18
lang.Character.DIRECTIONALITY_POP_DIRECTIONAL_FORMAT
C!

\ new statement
( *
* Mask for grabbing the type out of the contents of data.
* @see CharData#DATA
)

\ new statement
31
lang.Character.TYPE_MASK
!

\ new statement
( *
* Mask for grabbing the non-breaking space flag out of the contents of
* data.
* @see CharData#DATA
)

\ new statement
32
lang.Character.NO_BREAK_MASK
!

\ new statement
( *
* Mask for grabbing the mirrored directionality flag out of the contents
* of data.
* @see CharData#DATA
)

\ new statement
64
lang.Character.MIRROR_MASK
!

\ new statement
( *
* Min value for supplementary code point.
*
* @since 1.5
)

\ new statement
65536
lang.Character.MIN_SUPPLEMENTARY_CODE_POINT
!

\ new statement
( *
* Min value for code point.
*
* @since 1.5
)

\ new statement
0
lang.Character.MIN_CODE_POINT
!

\ new statement
( *
* Max value for code point.
*
* @since 1.5
)

\ new statement
1114111
lang.Character.MAX_CODE_POINT
!

\ new statement
( *
* Minimum high surrogate code in UTF-16 encoding.
*
* @since 1.5
)

\ new statement
55296
lang.Character.MIN_HIGH_SURROGATE
H!

\ new statement
( *
* Maximum high surrogate code in UTF-16 encoding.
*
* @since 1.5
)

\ new statement
56319
lang.Character.MAX_HIGH_SURROGATE
H!

\ new statement
( *
* Minimum low surrogate code in UTF-16 encoding.
*
* @since 1.5
)

\ new statement
56320
lang.Character.MIN_LOW_SURROGATE
H!

\ new statement
( *
* Maximum low surrogate code in UTF-16 encoding.
*
* @since 1.5
)

\ new statement
57343
lang.Character.MAX_LOW_SURROGATE
H!

\ new statement
( *
* Minimum surrogate code in UTF-16 encoding.
*
* @since 1.5
)

\ new statement
lang.Character.MIN_HIGH_SURROGATE
H@
lang.Character.MIN_SURROGATE
H!

\ new statement
( *
* Maximum low surrogate code in UTF-16 encoding.
*
* @since 1.5
)

\ new statement
lang.Character.MAX_LOW_SURROGATE
H@
lang.Character.MAX_SURROGATE
H!
