package java.lang;

//import gnu.java.lang.CharData;

//import java.io.Serializable;
//import java.text.Collator;

/**
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
 */
public final class Character implements /*Serializable,*/ Comparable
{

  /**
   * The immutable value of this Character.
   *
   * @serial the value of this Character
   */
  private final char value;

  /**
   * Compatible with JDK 1.0+.
   */
  private static final long serialVersionUID = 3786198910865385080L;

  /**
   * Smallest value allowed for radix arguments in Java. This value is 2.
   *
   * @see #digit(char, int)
   * @see #forDigit(int, int)
   * @see Integer#toString(int, int)
   * @see Integer#valueOf(String)
   */
  public static final int MIN_RADIX = 2;

  /**
   * Largest value allowed for radix arguments in Java. This value is 36.
   *
   * @see #digit(char, int)
   * @see #forDigit(int, int)
   * @see Integer#toString(int, int)
   * @see Integer#valueOf(String)
   */
  public static final int MAX_RADIX = 36;

  /**
   * The minimum value the char data type can hold.
   * This value is <code>'\\u0000'</code>.
   */
  public static final char MIN_VALUE = '\u0000';

  /**
   * The maximum value the char data type can hold.
   * This value is <code>'\\uFFFF'</code>.
   */
  public static final char MAX_VALUE = '\uFFFF';

  /**
   * The number of bits needed to represent a <code>char</code>.
   * @since 1.5
   */
  public static final int SIZE = 16;

  // This caches some Character values, and is used by boxing
  // conversions via valueOf().  We must cache at least 0..127;
  // this constant controls how much we actually cache.
  private static final int MAX_CACHE = 127;
  private static Character[] charCache = new Character[MAX_CACHE + 1];

  /**
   * Lu = Letter, Uppercase (Informative).
   *
   * @since 1.1
   */
  public static final byte UPPERCASE_LETTER = 1;

  /**
   * Ll = Letter, Lowercase (Informative).
   *
   * @since 1.1
   */
  public static final byte LOWERCASE_LETTER = 2;

  /**
   * Lt = Letter, Titlecase (Informative).
   *
   * @since 1.1
   */
  public static final byte TITLECASE_LETTER = 3;

  /**
   * Mn = Mark, Non-Spacing (Normative).
   *
   * @since 1.1
   */
  public static final byte NON_SPACING_MARK = 6;

  /**
   * Mc = Mark, Spacing Combining (Normative).
   *
   * @since 1.1
   */
  public static final byte COMBINING_SPACING_MARK = 8;

  /**
   * Me = Mark, Enclosing (Normative).
   *
   * @since 1.1
   */
  public static final byte ENCLOSING_MARK = 7;

  /**
   * Nd = Number, Decimal Digit (Normative).
   *
   * @since 1.1
   */
  public static final byte DECIMAL_DIGIT_NUMBER = 9;

  /**
   * Nl = Number, Letter (Normative).
   *
   * @since 1.1
   */
  public static final byte LETTER_NUMBER = 10;

  /**
   * No = Number, Other (Normative).
   *
   * @since 1.1
   */
  public static final byte OTHER_NUMBER = 11;

  /**
   * Zs = Separator, Space (Normative).
   *
   * @since 1.1
   */
  public static final byte SPACE_SEPARATOR = 12;

  /**
   * Zl = Separator, Line (Normative).
   *
   * @since 1.1
   */
  public static final byte LINE_SEPARATOR = 13;

  /**
   * Zp = Separator, Paragraph (Normative).
   *
   * @since 1.1
   */
  public static final byte PARAGRAPH_SEPARATOR = 14;

  /**
   * Cc = Other, Control (Normative).
   *
   * @since 1.1
   */
  public static final byte CONTROL = 15;

  /**
   * Cf = Other, Format (Normative).
   *
   * @since 1.1
   */
  public static final byte FORMAT = 16;

  /**
   * Cs = Other, Surrogate (Normative).
   *
   * @since 1.1
   */
  public static final byte SURROGATE = 19;

  /**
   * Co = Other, Private Use (Normative).
   *
   * @since 1.1
   */
  public static final byte PRIVATE_USE = 18;

  /**
   * Cn = Other, Not Assigned (Normative).
   *
   * @since 1.1
   */
  public static final byte UNASSIGNED = 0;

  /**
   * Lm = Letter, Modifier (Informative).
   *
   * @since 1.1
   */
  public static final byte MODIFIER_LETTER = 4;

  /**
   * Lo = Letter, Other (Informative).
   *
   * @since 1.1
   */
  public static final byte OTHER_LETTER = 5;

  /**
   * Pc = Punctuation, Connector (Informative).
   *
   * @since 1.1
   */
  public static final byte CONNECTOR_PUNCTUATION = 23;

  /**
   * Pd = Punctuation, Dash (Informative).
   *
   * @since 1.1
   */
  public static final byte DASH_PUNCTUATION = 20;

  /**
   * Ps = Punctuation, Open (Informative).
   *
   * @since 1.1
   */
  public static final byte START_PUNCTUATION = 21;

  /**
   * Pe = Punctuation, Close (Informative).
   *
   * @since 1.1
   */
  public static final byte END_PUNCTUATION = 22;

  /**
   * Pi = Punctuation, Initial Quote (Informative).
   *
   * @since 1.4
   */
  public static final byte INITIAL_QUOTE_PUNCTUATION = 29;

  /**
   * Pf = Punctuation, Final Quote (Informative).
   *
   * @since 1.4
   */
  public static final byte FINAL_QUOTE_PUNCTUATION = 30;

  /**
   * Po = Punctuation, Other (Informative).
   *
   * @since 1.1
   */
  public static final byte OTHER_PUNCTUATION = 24;

  /**
   * Sm = Symbol, Math (Informative).
   *
   * @since 1.1
   */
  public static final byte MATH_SYMBOL = 25;

  /**
   * Sc = Symbol, Currency (Informative).
   *
   * @since 1.1
   */
  public static final byte CURRENCY_SYMBOL = 26;

  /**
   * Sk = Symbol, Modifier (Informative).
   *
   * @since 1.1
   */
  public static final byte MODIFIER_SYMBOL = 27;

  /**
   * So = Symbol, Other (Informative).
   *
   * @since 1.1
   */
  public static final byte OTHER_SYMBOL = 28;

  /**
   * Undefined bidirectional character type. Undefined char values have
   * undefined directionality in the Unicode specification.
   *
   * @since 1.4
   */
  public static final byte DIRECTIONALITY_UNDEFINED = -1;

  /**
   * Strong bidirectional character type "L".
   *
   * @since 1.4
   */
  public static final byte DIRECTIONALITY_LEFT_TO_RIGHT = 0;

  /**
   * Strong bidirectional character type "R".
   *
   * @since 1.4
   */
  public static final byte DIRECTIONALITY_RIGHT_TO_LEFT = 1;

  /**
   * Strong bidirectional character type "AL".
   *
   * @since 1.4
   */
  public static final byte DIRECTIONALITY_RIGHT_TO_LEFT_ARABIC = 2;

  /**
   * Weak bidirectional character type "EN".
   *
   * @since 1.4
   */
  public static final byte DIRECTIONALITY_EUROPEAN_NUMBER = 3;

  /**
   * Weak bidirectional character type "ES".
   *
   * @since 1.4
   */
  public static final byte DIRECTIONALITY_EUROPEAN_NUMBER_SEPARATOR = 4;

  /**
   * Weak bidirectional character type "ET".
   *
   * @since 1.4
   */
  public static final byte DIRECTIONALITY_EUROPEAN_NUMBER_TERMINATOR = 5;

  /**
   * Weak bidirectional character type "AN".
   *
   * @since 1.4
   */
  public static final byte DIRECTIONALITY_ARABIC_NUMBER = 6;

  /**
   * Weak bidirectional character type "CS".
   *
   * @since 1.4
   */
  public static final byte DIRECTIONALITY_COMMON_NUMBER_SEPARATOR = 7;

  /**
   * Weak bidirectional character type "NSM".
   *
   * @since 1.4
   */
  public static final byte DIRECTIONALITY_NONSPACING_MARK = 8;

  /**
   * Weak bidirectional character type "BN".
   *
   * @since 1.4
   */
  public static final byte DIRECTIONALITY_BOUNDARY_NEUTRAL = 9;

  /**
   * Neutral bidirectional character type "B".
   *
   * @since 1.4
   */
  public static final byte DIRECTIONALITY_PARAGRAPH_SEPARATOR = 10;

  /**
   * Neutral bidirectional character type "S".
   *
   * @since 1.4
   */
  public static final byte DIRECTIONALITY_SEGMENT_SEPARATOR = 11;

  /**
   * Strong bidirectional character type "WS".
   *
   * @since 1.4
   */
  public static final byte DIRECTIONALITY_WHITESPACE = 12;

  /**
   * Neutral bidirectional character type "ON".
   *
   * @since 1.4
   */
  public static final byte DIRECTIONALITY_OTHER_NEUTRALS = 13;

  /**
   * Strong bidirectional character type "LRE".
   *
   * @since 1.4
   */
  public static final byte DIRECTIONALITY_LEFT_TO_RIGHT_EMBEDDING = 14;

  /**
   * Strong bidirectional character type "LRO".
   *
   * @since 1.4
   */
  public static final byte DIRECTIONALITY_LEFT_TO_RIGHT_OVERRIDE = 15;

  /**
   * Strong bidirectional character type "RLE".
   *
   * @since 1.4
   */
  public static final byte DIRECTIONALITY_RIGHT_TO_LEFT_EMBEDDING = 16;

  /**
   * Strong bidirectional character type "RLO".
   *
   * @since 1.4
   */
  public static final byte DIRECTIONALITY_RIGHT_TO_LEFT_OVERRIDE = 17;

  /**
   * Weak bidirectional character type "PDF".
   *
   * @since 1.4
   */
  public static final byte DIRECTIONALITY_POP_DIRECTIONAL_FORMAT = 18;

  /**
   * Mask for grabbing the type out of the contents of data.
   * @see CharData#DATA
   */
  private static final int TYPE_MASK = 0x1F;

  /**
   * Mask for grabbing the non-breaking space flag out of the contents of
   * data.
   * @see CharData#DATA
   */
  private static final int NO_BREAK_MASK = 0x20;

  /**
   * Mask for grabbing the mirrored directionality flag out of the contents
   * of data.
   * @see CharData#DATA
   */
  private static final int MIRROR_MASK = 0x40;

  /**
   * Min value for supplementary code point.
   *
   * @since 1.5
   */
  public static final int MIN_SUPPLEMENTARY_CODE_POINT = 0x10000;

  /**
   * Min value for code point.
   *
   * @since 1.5
   */
  public static final int MIN_CODE_POINT = 0;


  /**
   * Max value for code point.
   *
   * @since 1.5
   */
  public static final int MAX_CODE_POINT = 0x010ffff;


  /**
   * Minimum high surrogate code in UTF-16 encoding.
   *
   * @since 1.5
   */
  public static final char MIN_HIGH_SURROGATE = '\ud800';

  /**
   * Maximum high surrogate code in UTF-16 encoding.
   *
   * @since 1.5
   */
  public static final char MAX_HIGH_SURROGATE = '\udbff';

  /**
   * Minimum low surrogate code in UTF-16 encoding.
   *
   * @since 1.5
   */
  public static final char MIN_LOW_SURROGATE = '\udc00';

  /**
   * Maximum low surrogate code in UTF-16 encoding.
   *
   * @since 1.5
   */
  public static final char MAX_LOW_SURROGATE = '\udfff';

  /**
   * Minimum surrogate code in UTF-16 encoding.
   *
   * @since 1.5
   */
  public static final char MIN_SURROGATE = MIN_HIGH_SURROGATE;

  /**
   * Maximum low surrogate code in UTF-16 encoding.
   *
   * @since 1.5
   */
  public static final char MAX_SURROGATE = MAX_LOW_SURROGATE;

  /**
   * Wraps up a character.
   *
   * @param value the character to wrap
   */
  public Character(char value)
  {
    this.value = value;
  }

  /**
   * Returns the character which has been wrapped by this class.
   *
   * @return the character wrapped
   */
  public char charValue()
  {
    return value;
  }

  /**
   * Returns the numerical value (unsigned) of the wrapped character.
   * Range of returned values: 0x0000-0xFFFF.
   *
   * @return the value of the wrapped character
   */
  public int hashCode()
  {
    return value;
  }

  /**
   * Determines if an object is equal to this object. This is only true for
   * another Character object wrapping the same value.
   *
   * @param o object to compare
   * @return true if o is a Character with the same value
   */
  public boolean equals(Object o)
  {
    return o instanceof Character && value == ((Character) o).value;
  }

  /**
   * Converts the wrapped character into a String.
   *
   * @return a String containing one character -- the wrapped character
   *         of this instance
   */
  public String toString()
  {
    // Package constructor avoids an array copy.
    return new String(new char[] { value });
  }

  /**
   * Returns a String of length 1 representing the specified character.
   *
   * @param ch the character to convert
   * @return a String containing the character
   * @since 1.4
   */
  public static String toString(char ch)
  {
    // Package constructor avoids an array copy.
    return new String(new char[] { ch });
  }

  /**
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
   */
  public static boolean isLowerCase(char ch)
  {
    return isLowerCase((int)ch);
  }

  /**
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
   */
  public static boolean isLowerCase(int codePoint)
  {
    //return getType(codePoint) == LOWERCASE_LETTER;
    return codePoint >= 'a' && codePoint <= 'z';
  }

  /**
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
   */
  public static boolean isUpperCase(char ch)
  {
    return isUpperCase((int)ch);
  }

  /**
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
   */
  public static boolean isUpperCase(int codePoint)
  {
    //return getType(codePoint) == UPPERCASE_LETTER;
    return codePoint >= 'A' && codePoint <= 'Z';
  }

  /**
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
   */
  public static boolean isDigit(char ch)
  {
    return isDigit((int)ch);
  }

  /**
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
   */

  public static boolean isDigit(int codePoint)
  {
    //return getType(codePoint) == DECIMAL_DIGIT_NUMBER;
    return codePoint >= '0' && codePoint <= '9';
  }

  /**
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
   */
  public static boolean isLetter(char ch)
  {
    return isLetter((int)ch);
  }

  /**
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
   */
  public static boolean isLetter(int codePoint)
  {
    //return ((1 << getType(codePoint))
    //    & ((1 << UPPERCASE_LETTER)
    //        | (1 << LOWERCASE_LETTER)
      //      | (1 << TITLECASE_LETTER)
        //    | (1 << MODIFIER_LETTER)
          //  | (1 << OTHER_LETTER))) != 0;
    return isUpperCase(codePoint) || isLowerCase(codePoint);
  }

  /**
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
   */
  public static boolean isLetterOrDigit(char ch)
  {
    return isLetterOrDigit((int)ch);
  }

  /**
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
   */
  public static boolean isLetterOrDigit(int codePoint)
  {
    //return ((1 << getType(codePoint))
    //    & ((1 << UPPERCASE_LETTER)
    //       | (1 << LOWERCASE_LETTER)
    //       | (1 << TITLECASE_LETTER)
    //       | (1 << MODIFIER_LETTER)
    //       | (1 << OTHER_LETTER)
    //       | (1 << DECIMAL_DIGIT_NUMBER))) != 0;
    return isLetter(codePoint) || isDigit(codePoint);
  }

  /**
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
   */
  public static boolean isJavaLetter(char ch)
  {
    return isJavaIdentifierStart(ch);
  }

  /**
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
   */
  public static boolean isJavaLetterOrDigit(char ch)
  {
    return isJavaIdentifierPart(ch);
  }

  /**
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
   */
  public static boolean isJavaIdentifierStart(char ch)
  {
    return isJavaIdentifierStart((int)ch);
  }

  /**
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
   */
  public static boolean isJavaIdentifierStart(int codePoint)
  {
    //return ((1 << getType(codePoint))
    //        & ((1 << UPPERCASE_LETTER)
    //           | (1 << LOWERCASE_LETTER)
    //           | (1 << TITLECASE_LETTER)
    //           | (1 << MODIFIER_LETTER)
    //           | (1 << OTHER_LETTER)
    //           | (1 << LETTER_NUMBER)
    //           | (1 << CURRENCY_SYMBOL)
    //           | (1 << CONNECTOR_PUNCTUATION))) != 0;
    return isLetter(codePoint) || codePoint == '$' || codePoint == '_';
  }

  /**
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
   */
  public static boolean isJavaIdentifierPart(char ch)
  {
    return isJavaIdentifierPart((int)ch);
  }

  /**
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
   */
  public static boolean isJavaIdentifierPart(int codePoint)
  {
    //int category = getType(codePoint);
    //return ((1 << category)
    //        & ((1 << UPPERCASE_LETTER)
    //           | (1 << LOWERCASE_LETTER)
    //           | (1 << TITLECASE_LETTER)
    //           | (1 << MODIFIER_LETTER)
    //           | (1 << OTHER_LETTER)
    //           | (1 << NON_SPACING_MARK)
    //           | (1 << COMBINING_SPACING_MARK)
    //           | (1 << DECIMAL_DIGIT_NUMBER)
    //           | (1 << LETTER_NUMBER)
    //           | (1 << CURRENCY_SYMBOL)
    //           | (1 << CONNECTOR_PUNCTUATION)
    //           | (1 << FORMAT))) != 0
    //  || (category == CONTROL && isIdentifierIgnorable(codePoint));
    return isJavaIdentifierStart(codePoint) || isDigit(codePoint) || codePoint == '.';
  }

  /**
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
   */
  public static boolean isUnicodeIdentifierStart(char ch)
  {
    return isUnicodeIdentifierStart((int)ch);
  }

  /**
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
   */
  public static boolean isUnicodeIdentifierStart(int codePoint)
  {
    //return ((1 << getType(codePoint))
    //        & ((1 << UPPERCASE_LETTER)
    //           | (1 << LOWERCASE_LETTER)
    //           | (1 << TITLECASE_LETTER)
    //           | (1 << MODIFIER_LETTER)
    //           | (1 << OTHER_LETTER)
    //           | (1 << LETTER_NUMBER))) != 0;
    return isLetter(codePoint);
  }

  /**
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
   */
  public static boolean isUnicodeIdentifierPart(char ch)
  {
    return isUnicodeIdentifierPart((int)ch);
  }

  /**
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
   */
  public static boolean isUnicodeIdentifierPart(int codePoint)
  {
    //int category = getType(codePoint);
    //return ((1 << category)
    //        & ((1 << UPPERCASE_LETTER)
    //           | (1 << LOWERCASE_LETTER)
    //           | (1 << TITLECASE_LETTER)
    //           | (1 << MODIFIER_LETTER)
    //           | (1 << OTHER_LETTER)
    //           | (1 << NON_SPACING_MARK)
    //           | (1 << COMBINING_SPACING_MARK)
    //           | (1 << DECIMAL_DIGIT_NUMBER)
    //           | (1 << LETTER_NUMBER)
    //           | (1 << CONNECTOR_PUNCTUATION)
    //           | (1 << FORMAT))) != 0
    //  || (category == CONTROL && isIdentifierIgnorable(codePoint));
    return isJavaIdentifierPart(codePoint);
  }

  /**
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
   */
  public static boolean isIdentifierIgnorable(char ch)
  {
    return isIdentifierIgnorable((int)ch);
  }
  /**
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
   */
  public static boolean isIdentifierIgnorable(int codePoint)
  {
    if ((codePoint >= 0 && codePoint <= 0x0008)
        || (codePoint >= 0x000E && codePoint <= 0x001B)
        || (codePoint >= 0x007F && codePoint <= 0x009F)
        /*|| getType(codePoint) == FORMAT*/)
      return true;
    return false;
  }

  /**
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
   */
  public static char toLowerCase(char ch)
  {
    //return (char) (lower[0][readCodePoint((int)ch) >>> 7] + ch);
    if (isUpperCase(ch))
     ch = ch + 'a' - 'A';
    return ch;
  }

  /**
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
   */
  public static char toUpperCase(char ch)
  {
    //return (char) (upper[0][readCodePoint((int)ch) >>> 7] + ch);
    if (isLowerCase(ch))
     ch = ch - 'a' + 'A';
    return ch;
  }
  public static char CHAR_ERROR;
  public static char toUpperCaseEx(char ch)
  {
    //return (char) (upper[0][readCodePoint((int)ch) >>> 7] + ch);
    CHAR_ERROR = 0;
    if (isLowerCase(ch))
     ch = ch - 'a' + 'A';
    else if (!isUpperCase(ch))
     CHAR_ERROR = ch;
    return ch;
  }
  public static char [] toUpperCaseCharArray(char ch)
  {
   return new char[] { toUpperCase(ch) };
  }
  /**
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
   */
  public static int digit(char ch, int radix)
  {
    if (radix < MIN_RADIX || radix > MAX_RADIX)
      return -1;
    if (isDigit(ch))
     return ch - '0';
    if (isLowerCase(ch))
     return ch - 'a' + 10;
    if (isUpperCase(ch))
     return ch - 'A' + 10;
    return -1;
  }

  /**
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
   */
  public static boolean isSpace(char ch)
  {
    // Performing the subtraction up front alleviates need to compare longs.
    return ch-- <= ' ' && ((ch == (' ' - 1))
                              || (ch == ('\t' - 1))
                              || (ch == ('\n' - 1))
                              || (ch == ('\r' - 1))
                              || (ch == ('\f' - 1)));
  }

  /**
   * Determines if a character is a Unicode space character. This includes
   * SPACE_SEPARATOR, LINE_SEPARATOR, and PARAGRAPH_SEPARATOR.
   * <br>
   * Unicode space = [Zs]|[Zp]|[Zl]
   *
   * @param ch character to test
   * @return true if ch is a Unicode space, else false
   * @see #isWhitespace(char)
   * @since 1.1
   */
  public static boolean isSpaceChar(char ch)
  {
    return isSpace(ch);
  }

  /**
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
   */
  public static boolean isWhitespace(char ch)
  {
    return isWhitespace((int) ch);
  }

  /**
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
   */
  public static boolean isWhitespace(int codePoint)
  {

    return ((codePoint == '\t')
                                || (codePoint == '\n')
                                || (codePoint == '\u000B')
                                || (codePoint == '\u000C')
                                || (codePoint == '\r')
                                || (codePoint == '\u001C')
                                || (codePoint == '\u001D')
                                || (codePoint == '\u001E')
                                || (codePoint == '\u001F'));
  }

  /**
   * Determines if a character has the ISO Control property.
   * <br>
   * ISO Control = [Cc]
   *
   * @param ch character to test
   * @return true if ch is an ISO Control character, else false
   * @see #isSpaceChar(char)
   * @see #isWhitespace(char)
   * @since 1.1
   */
  public static boolean isISOControl(char ch)
  {
    return isISOControl((int)ch);
  }

  /**
   * Determines if the character is an ISO Control character.  This is true
   * if the code point is in the range [0, 0x001F] or if it is in the range
   * [0x007F, 0x009F].
   * @param codePoint the character to check
   * @return true if the character is in one of the above ranges
   *
   * @since 1.5
   */
  public static boolean isISOControl(int codePoint)
  {
    if ((codePoint >= 0 && codePoint <= 0x001F)
        || (codePoint >= 0x007F && codePoint <= 0x009F))
      return true;
    return false;
  }

  /**
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
   */
  public static char forDigit(int digit, int radix)
  {
    if (radix < MIN_RADIX || radix > MAX_RADIX
        || digit < 0 || digit >= radix)
      return '\0';
    if (digit < 10)
     return digit + '0';

    return digit - 10 + 'a';
  }

  /**
   * Compares another Character to this Character, numerically.
   *
   * @param anotherCharacter Character to compare with this Character
   * @return a negative integer if this Character is less than
   *         anotherCharacter, zero if this Character is equal, and
   *         a positive integer if this Character is greater
   * @throws NullPointerException if anotherCharacter is null
   * @since 1.2
   */
  public int compareTo(Character anotherCharacter)
  {
    return value - anotherCharacter.value;
  }

  /**
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
   */
  public int compareTo(Object o)
  {
    return compareTo((Character) o);
  }

  /**
   * Returns an <code>Character</code> object wrapping the value.
   * In contrast to the <code>Character</code> constructor, this method
   * will cache some values.  It is used by boxing conversion.
   *
   * @param val the value to wrap
   * @return the <code>Character</code>
   *
   * @since 1.5
   */
  public static Character valueOf(char val)
  {
    if (val > MAX_CACHE)
      return new Character(val);
    synchronized (charCache)
      {
    if (charCache[val - MIN_VALUE] == null)
      charCache[val - MIN_VALUE] = new Character(val);
    return charCache[val - MIN_VALUE];
      }
  }

  /**
   * Reverse the bytes in val.
   * @since 1.5
   */
  public static char reverseBytes(char val)
  {
    return (char) (((val >> 8) & 0xff) | ((val << 8) & 0xff00));
  }

  /**
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
   */
  public static char[] toChars(int codePoint)
  {
   if (!isValidCodePoint(codePoint))
     throw new IllegalArgumentException("Illegal Unicode code point : "
                                        + codePoint);
    char[] result = new char[charCount(codePoint)];
    int ignore = toChars(codePoint, result, 0);
    return result;
  }

  /**
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
   */
  public static int toChars(int codePoint, char[] dst, int dstIndex)
  {
    if (!isValidCodePoint(codePoint))
      {
        throw new IllegalArgumentException("not a valid code point: "
                                           + codePoint);
      }

    int result;
    if (isSupplementaryCodePoint(codePoint))
      {
        // Write second char first to cause IndexOutOfBoundsException
        // immediately.
        final int cp2 = codePoint - 0x10000;
        dst[dstIndex + 1] = (char) ((cp2 % 0x400) + (int) MIN_LOW_SURROGATE);
        dst[dstIndex] = (char) ((cp2 / 0x400) + (int) MIN_HIGH_SURROGATE);
        result = 2;
      }
    else
      {
        dst[dstIndex] = (char) codePoint;
        result = 1;
      }
    return result;
  }

  /**
   * Return number of 16-bit characters required to represent the given
   * code point.
   *
   * @param codePoint a unicode code point
   *
   * @return 2 if codePoint >= 0x10000, 1 otherwise.
   *
   * @since 1.5
   */
  public static int charCount(int codePoint)
  {
    return
      (codePoint >= MIN_SUPPLEMENTARY_CODE_POINT)
      ? 2
      : 1;
  }

  /**
   * Determines whether the specified code point is
   * in the range 0x10000 .. 0x10FFFF, i.e. the character is within the Unicode
   * supplementary character range.
   *
   * @param codePoint a Unicode code point
   *
   * @return <code>true</code> if code point is in supplementary range
   *
   * @since 1.5
   */
  public static boolean isSupplementaryCodePoint(int codePoint)
  {
    return codePoint >= MIN_SUPPLEMENTARY_CODE_POINT
      && codePoint <= MAX_CODE_POINT;
  }

  /**
   * Determines whether the specified code point is
   * in the range 0x0000 .. 0x10FFFF, i.e. it is a valid Unicode code point.
   *
   * @param codePoint a Unicode code point
   *
   * @return <code>true</code> if code point is valid
   *
   * @since 1.5
   */
  public static boolean isValidCodePoint(int codePoint)
  {
    return codePoint >= MIN_CODE_POINT && codePoint <= MAX_CODE_POINT;
  }

  /**
   * Return true if the given character is a high surrogate.
   * @param ch the character
   * @return true if the character is a high surrogate character
   *
   * @since 1.5
   */
  public static boolean isHighSurrogate(char ch)
  {
    return ch >= MIN_HIGH_SURROGATE && ch <= MAX_HIGH_SURROGATE;
  }

  /**
   * Return true if the given character is a low surrogate.
   * @param ch the character
   * @return true if the character is a low surrogate character
   *
   * @since 1.5
   */
  public static boolean isLowSurrogate(char ch)
  {
    return ch >= MIN_LOW_SURROGATE && ch <= MAX_LOW_SURROGATE;
  }

  /**
   * Return true if the given characters compose a surrogate pair.
   * This is true if the first character is a high surrogate and the
   * second character is a low surrogate.
   * @param ch1 the first character
   * @param ch2 the first character
   * @return true if the characters compose a surrogate pair
   *
   * @since 1.5
   */
  public static boolean isSurrogatePair(char ch1, char ch2)
  {
    return isHighSurrogate(ch1) && isLowSurrogate(ch2);
  }

  /**
   * Given a valid surrogate pair, this returns the corresponding
   * code point.
   * @param high the high character of the pair
   * @param low the low character of the pair
   * @return the corresponding code point
   *
   * @since 1.5
   */
  public static int toCodePoint(char high, char low)
  {
    return ((high - MIN_HIGH_SURROGATE) * 0x400) +
      (low - MIN_LOW_SURROGATE) + 0x10000;
  }

  /**
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
   */
  public static int codePointAt(CharSequence sequence, int index)
  {
    int len = sequence.length();
    if (index < 0 || index >= len)
      throw new IndexOutOfBoundsException();
    char high = sequence.charAt(index);
    if (! isHighSurrogate(high) || ++index >= len)
      return high;
    char low = sequence.charAt(index);
    if (! isLowSurrogate(low))
      return high;
    return toCodePoint(high, low);
  }

  /**
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
   */
  public static int codePointAt(char[] chars, int index)
  {
    return codePointAt(chars, index, chars.length);
  }

  /**
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
   */
  public static int codePointAt(char[] chars, int index, int limit)
  {
    if (index < 0 || index >= limit || limit < 0 || limit > chars.length)
      throw new IndexOutOfBoundsException();
    char high = chars[index];
    if (! isHighSurrogate(high) || ++index >= limit)
      return high;
    char low = chars[index];
    if (! isLowSurrogate(low))
      return high;
    return toCodePoint(high, low);
  }

  /**
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
   */
  public static int codePointBefore(char[] chars, int index)
  {
    return codePointBefore(chars, index, 1);
  }

  /**
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
   */
  public static int codePointBefore(char[] chars, int index, int start)
  {
    if (index < start || index > chars.length
	|| start < 0 || start >= chars.length)
      throw new IndexOutOfBoundsException();
    --index;
    char low = chars[index];
    if (! isLowSurrogate(low) || --index < start)
      return low;
    char high = chars[index];
    if (! isHighSurrogate(high))
      return low;
    return toCodePoint(high, low);
  }

  /**
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
   */
  public static int codePointBefore(CharSequence sequence, int index)
  {
    int len = sequence.length();
    if (index < 1 || index > len)
      throw new IndexOutOfBoundsException();
    --index;
    char low = sequence.charAt(index);
    if (! isLowSurrogate(low) || --index < 0)
      return low;
    char high = sequence.charAt(index);
    if (! isHighSurrogate(high))
      return low;
    return toCodePoint(high, low);
  }
} // class Character
