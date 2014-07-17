MODULENAME java.lang.Math
(
* @(#)Math.java	1.57 03/01/23
*
* Copyright 2003 Sun Microsystems, Inc. All rights reserved.
* SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
)  \ import java.util.Random;
( *
* The class <code>Math</code> contains methods for performing basic
* numeric operations such as the elementary exponential, logarithm,
* square root, and trigonometric functions.
* <p>
* Unlike some of the numeric methods of class
* <code>StrictMath</code>, all implementations of the equivalent
* functions of class <code>Math</code> are not defined to return the
* bit-for-bit same results.  This relaxation permits
* better-performing implementations where strict reproducibility is
* not required.
* <p>
* By default many of the <code>Math</code> methods simply call
* the equivalent method in <code>StrictMath</code> for their
* implementation.  Code generators are encouraged to use
* platform-specific native libraries or microprocessor instructions,
* where available, to provide higher-performance implementations of
* <code>Math</code> methods.  Such higher-performance
* implementations still must conform to the specification for
* <code>Math</code>.
* <p>
* The quality of implementation specifications concern two
* properties, accuracy of the returned result and monotonicity of the
* method.  Accuracy of the floating-point <code>Math</code> methods
* is measured in terms of <i>ulps</i>, units in the last place.  For
* a given floating-point format, an ulp of a specific real number
* value is the difference between the two floating-point values
* closest to that numerical value.  When discussing the accuracy of a
* method as a whole rather than at a specific argument, the number of
* ulps cited is for the worst-case error at any argument.  If a
* method always has an error less than 0.5 ulps, the method always
* returns the floating-point number nearest the exact result; such a
* method is <i>correctly rounded</i>.  A correctly rounded method is
* generally the best a floating-point approximation can be; however,
* it is impractical for many floating-point methods to be correctly
* rounded.  Instead, for the <code>Math</code> class, a larger error
* bound of 1 or 2 ulps is allowed for certain methods.  Informally,
* with a 1 ulp error bound, when the exact result is a representable
* number the exact result should be returned; otherwise, either of
* the two floating-point numbers closest to the exact result may be
* returned.  Besides accuracy at individual arguments, maintaining
* proper relations between the method at different arguments is also
* important.  Therefore, methods with more than 0.5 ulp errors are
* required to be <i>semi-monotonic</i>: whenever the mathematical
* function is non-decreasing, so is the floating-point approximation,
* likewise, whenever the mathematical function is non-increasing, so
* is the floating-point approximation.  Not all approximations that
* have 1 ulp accuracy will automatically meet the monotonicity
* requirements.
*
* @author  unascribed
* @version 1.57, 01/23/03
* @since   JDK1.0
)
( *
* Computes the remainder operation on two arguments as prescribed
* by the IEEE 754 standard.
* The remainder value is mathematically equal to
* <code>f1&nbsp;-&nbsp;f2</code>&nbsp;&times;&nbsp;<i>n</i>,
* where <i>n</i> is the mathematical integer closest to the exact
* mathematical value of the quotient <code>f1/f2</code>, and if two
* mathematical integers are equally close to <code>f1/f2</code>,
* then <i>n</i> is the integer that is even. If the remainder is
* zero, its sign is the same as the sign of the first argument.
* Special cases:
* <ul><li>If either argument is NaN, or the first argument is infinite,
* or the second argument is positive zero or negative zero, then the
* result is NaN.
* <li>If the first argument is finite and the second argument is
* infinite, then the result is the same as the first argument.</ul>
*
* @param   f1   the dividend.
* @param   f2   the divisor.
* @return  the remainder when <code>f1</code> is divided by
*          <code>f2</code>.
)

: java.lang.Math.IEEEremainder§1596082432
   2LOCAL f2
   2LOCAL f1
   
   \ new statement
   
   \ new statement
   f1 f2 F/ 2DUP stripf F- f2 F*
   
   \ new statement
   0 §break432 LABEL
   
   \ new statement
   
   
   PURGE 2
   
   \ new statement
   DROP
;
( *
* Don't let anyone instantiate this class.
)

:LOCAL java.lang.Math.Math§-590525184
   
   \ new statement
   0
   LOCALS §this |
   
   \ new statement
   sort.Object§-1890496768.table 1827294976 EXECUTE-NEW
   
   \ new statement
   28 MALLOC DROP DUP TO §this OVER
   IF
      OVER CELL+ @ 4 + OVER OVER ! DROP
   ENDIF
   CELL+ @ OVER
   OVER ! NIP
   java.lang.Math§-1239069440.table OVER 12 + !
   -1239089152 OVER 20 + !
   " Math " OVER 16 + ! DROP
   
   \ new statement
   0
   §this CELL+ @ 24 + ( java.lang.Math._dynamicBlocking )    !
   
   \ new statement
   §this ( return object )    
   \ new statement
   0 §break420 LABEL
   
   \ new statement
   PURGE 1
   
   \ new statement
   DROP
;
( *
* Returns the absolute value of a <code>double</code> value.
* If the argument is not negative, the argument is returned.
* If the argument is negative, the negation of the argument is returned.
* Special cases:
* <ul><li>If the argument is positive zero or negative zero, the result
* is positive zero.
* <li>If the argument is infinite, the result is positive infinity.
* <li>If the argument is NaN, the result is NaN.</ul>
* In other words, the result is the same as the value of the expression:
* <p><code>Double.longBitsToDouble((Double.doubleToLongBits(a)&lt;&lt;1)&gt;&gt;&gt;1)</code>
*
* @param   a   the argument whose absolute value is to be determined
* @return  the absolute value of the argument.
)

: java.lang.Math.abs§-537370368
   2LOCAL a
   
   \ new statement
   
   \ new statement
   a FABS
   
   \ new statement
   0 §break442 LABEL
   
   \ new statement
   
   PURGE 1
   
   \ new statement
   DROP
;
(
private static Random randomNumberGenerator;

private static synchronized void initRNG() {
if (randomNumberGenerator == null)
randomNumberGenerator = new Random();
}

/**
* Returns a <code>double</code> value with a positive sign, greater
* than or equal to <code>0.0</code> and less than <code>1.0</code>.
* Returned values are chosen pseudorandomly with (approximately)
* uniform distribution from that range.
* <p>
* When this method is first called, it creates a single new
* pseudorandom-number generator, exactly as if by the expression
* <blockquote><pre>new java.util.Random</pre></blockquote>
* This new pseudorandom-number generator is used thereafter for all
* calls to this method and is used nowhere else.
* <p>
* This method is properly synchronized to allow correct use by more
* than one thread. However, if many threads need to generate
* pseudorandom numbers at a great rate, it may reduce contention for
* each thread to have its own pseudorandom-number generator.
*
* @return  a pseudorandom <code>double</code> greater than or equal
* to <code>0.0</code> and less than <code>1.0</code>.
* @see     java.util.Random#nextDouble()
public static double random() {
if (randomNumberGenerator == null) initRNG();
return randomNumberGenerator.nextDouble();
}
)  ( *
* Returns the absolute value of an <code>int</code> value.
* If the argument is not negative, the argument is returned.
* If the argument is negative, the negation of the argument is returned.
* <p>
* Note that if the argument is equal to the value of
* <code>Integer.MIN_VALUE</code>, the most negative representable
* <code>int</code> value, the result is that same value, which is
* negative.
*
* @param   a   the argument whose absolute value is to be determined
* @return  the absolute value of the argument.
* @see     java.lang.Integer#MIN_VALUE
)

: java.lang.Math.abs§1087791360
   LOCAL a
   
   \ new statement
   
   \ new statement
   a ABS
   
   \ new statement
   0 §break440 LABEL
   
   \ new statement
   
   PURGE 1
   
   \ new statement
   DROP
;
( *
* Returns the absolute value of a <code>long</code> value.
* If the argument is not negative, the argument is returned.
* If the argument is negative, the negation of the argument is returned.
* <p>
* Note that if the argument is equal to the value of
* <code>Long.MIN_VALUE</code>, the most negative representable
* <code>long</code> value, the result is that same value, which is
* negative.
*
* @param   a   the argument whose absolute value is to be determined
* @return  the absolute value of the argument.
* @see     java.lang.Long#MIN_VALUE
)

: java.lang.Math.abs§55402752
   2LOCAL a
   
   \ new statement
   
   \ new statement
   a DABS
   
   \ new statement
   0 §break441 LABEL
   
   \ new statement
   
   PURGE 1
   
   \ new statement
   DROP
;
( *
* Returns the arc cosine of an angle, in the range of 0.0 through
* <i>pi</i>.  Special case:
* <ul><li>If the argument is NaN or its absolute value is greater
* than 1, then the result is NaN.</ul>
* <p>
* A result must be within 1 ulp of the correctly rounded result.  Results
* must be semi-monotonic.
*
* @param   a   the value whose arc cosine is to be returned.
* @return  the arc cosine of the argument.
)

: java.lang.Math.acos§1364418816
   2LOCAL a
   
   \ new statement
   
   \ new statement
   a FACOS
   
   \ new statement
   0 §break425 LABEL
   
   \ new statement
   
   PURGE 1
   
   \ new statement
   DROP
;


: java.lang.Math.acosh§-571776768
   2LOCAL a
   
   \ new statement
   
   \ new statement
   a FACOSH
   
   \ new statement
   0 §break453 LABEL
   
   \ new statement
   
   PURGE 1
   
   \ new statement
   DROP
;
( *
* Returns the arc sine of an angle, in the range of -<i>pi</i>/2 through
* <i>pi</i>/2. Special cases:
* <ul><li>If the argument is NaN or its absolute value is greater
* than 1, then the result is NaN.
* <li>If the argument is zero, then the result is a zero with the
* same sign as the argument.</ul>
* <p>
* A result must be within 1 ulp of the correctly rounded result.  Results
* must be semi-monotonic.
*
* @param   a   the value whose arc sine is to be returned.
* @return  the arc sine of the argument.
)

: java.lang.Math.asin§-904437504
   2LOCAL a
   
   \ new statement
   
   \ new statement
   a FASIN
   
   \ new statement
   0 §break424 LABEL
   
   \ new statement
   
   PURGE 1
   
   \ new statement
   DROP
;


: java.lang.Math.asinh§-649043712
   2LOCAL a
   
   \ new statement
   
   \ new statement
   a FASINH
   
   \ new statement
   0 §break452 LABEL
   
   \ new statement
   
   PURGE 1
   
   \ new statement
   DROP
;
( *
* Returns the arc tangent of an angle, in the range of -<i>pi</i>/2
* through <i>pi</i>/2.  Special cases:
* <ul><li>If the argument is NaN, then the result is NaN.
* <li>If the argument is zero, then the result is a zero with the
* same sign as the argument.</ul>
* <p>
* A result must be within 1 ulp of the correctly rounded result.  Results
* must be semi-monotonic.
*
* @param   a   the value whose arc tangent is to be returned.
* @return  the arc tangent of the argument.
)

: java.lang.Math.atan§1069965568
   2LOCAL a
   
   \ new statement
   
   \ new statement
   a FATAN
   
   \ new statement
   0 §break426 LABEL
   
   \ new statement
   
   PURGE 1
   
   \ new statement
   DROP
;
( *
* Converts rectangular coordinates (<code>x</code>,&nbsp;<code>y</code>)
* to polar (r,&nbsp;<i>theta</i>).
* This method computes the phase <i>theta</i> by computing an arc tangent
* of <code>y/x</code> in the range of -<i>pi</i> to <i>pi</i>. Special
* cases:
* <ul><li>If either argument is NaN, then the result is NaN.
* <li>If the first argument is positive zero and the second argument
* is positive, or the first argument is positive and finite and the
* second argument is positive infinity, then the result is positive
* zero.
* <li>If the first argument is negative zero and the second argument
* is positive, or the first argument is negative and finite and the
* second argument is positive infinity, then the result is negative zero.
* <li>If the first argument is positive zero and the second argument
* is negative, or the first argument is positive and finite and the
* second argument is negative infinity, then the result is the
* <code>double</code> value closest to <i>pi</i>.
* <li>If the first argument is negative zero and the second argument
* is negative, or the first argument is negative and finite and the
* second argument is negative infinity, then the result is the
* <code>double</code> value closest to -<i>pi</i>.
* <li>If the first argument is positive and the second argument is
* positive zero or negative zero, or the first argument is positive
* infinity and the second argument is finite, then the result is the
* <code>double</code> value closest to <i>pi</i>/2.
* <li>If the first argument is negative and the second argument is
* positive zero or negative zero, or the first argument is negative
* infinity and the second argument is finite, then the result is the
* <code>double</code> value closest to -<i>pi</i>/2.
* <li>If both arguments are positive infinity, then the result is the
* <code>double</code> value closest to <i>pi</i>/4.
* <li>If the first argument is positive infinity and the second argument
* is negative infinity, then the result is the <code>double</code>
* value closest to 3*<i>pi</i>/4.
* <li>If the first argument is negative infinity and the second argument
* is positive infinity, then the result is the <code>double</code> value
* closest to -<i>pi</i>/4.
* <li>If both arguments are negative infinity, then the result is the
* <code>double</code> value closest to -3*<i>pi</i>/4.</ul>
* <p>
* A result must be within 2 ulps of the correctly rounded result.  Results
* must be semi-monotonic.
*
* @param   y   the ordinate coordinate
* @param   x   the abscissa coordinate
* @return  the <i>theta</i> component of the point
*          (<i>r</i>,&nbsp;<i>theta</i>)
*          in polar coordinates that corresponds to the point
*          (<i>x</i>,&nbsp;<i>y</i>) in Cartesian coordinates.
)

: java.lang.Math.atan2§1799971072
   2LOCAL x
   2LOCAL y
   
   \ new statement
   
   \ new statement
   y x FATAN2
   
   \ new statement
   0 §break436 LABEL
   
   \ new statement
   
   
   PURGE 2
   
   \ new statement
   DROP
;


: java.lang.Math.atanh§513892608
   2LOCAL a
   
   \ new statement
   
   \ new statement
   a FATANH
   
   \ new statement
   0 §break454 LABEL
   
   \ new statement
   
   PURGE 1
   
   \ new statement
   DROP
;
( *
* Returns the smallest (closest to negative infinity)
* <code>double</code> value that is not less than the argument and is
* equal to a mathematical integer. Special cases:
* <ul><li>If the argument value is already equal to a mathematical
* integer, then the result is the same as the argument.
* <li>If the argument is NaN or an infinity or positive zero or negative
* zero, then the result is the same as the argument.
* <li>If the argument value is less than zero but greater than -1.0,
* then the result is negative zero.</ul>
* Note that the value of <code>Math.ceil(x)</code> is exactly the
* value of <code>-Math.floor(-x)</code>.
*
* @param   a   a value.
* <!--@return  the value &lceil;&nbsp;<code>a</code>&nbsp;&rceil;.-->
* @return  the smallest (closest to negative infinity)
*          floating-point value that is not less than the argument
*          and is equal to a mathematical integer.
)

: java.lang.Math.ceil§1303274240
   2LOCAL a
   
   \ new statement
   
   \ new statement
   a FROUND
   
   \ new statement
   0 §break433 LABEL
   
   \ new statement
   
   PURGE 1
   
   \ new statement
   DROP
;
( *
* Returns the trigonometric cosine of an angle. Special cases:
* <ul><li>If the argument is NaN or an infinity, then the
* result is NaN.</ul>
* <p>
* A result must be within 1 ulp of the correctly rounded result.  Results
* must be semi-monotonic.
*
* @param   a   an angle, in radians.
* @return  the cosine of the argument.
)

: java.lang.Math.cos§-587308288
   2LOCAL a
   
   \ new statement
   
   \ new statement
   a FCOS
   
   \ new statement
   0 §break422 LABEL
   
   \ new statement
   
   PURGE 1
   
   \ new statement
   DROP
;


: java.lang.Math.cosh§-1551277312
   2LOCAL a
   
   \ new statement
   
   \ new statement
   a FCOSH
   
   \ new statement
   0 §break450 LABEL
   
   \ new statement
   
   PURGE 1
   
   \ new statement
   DROP
;
( *
* Returns Euler's number <i>e</i> raised to the power of a
* <code>double</code> value.  Special cases:
* <ul><li>If the argument is NaN, the result is NaN.
* <li>If the argument is positive infinity, then the result is
* positive infinity.
* <li>If the argument is negative infinity, then the result is
* positive zero.</ul>
* <p>
* A result must be within 1 ulp of the correctly rounded result.  Results
* must be semi-monotonic.
*
* @param   a   the exponent to raise <i>e</i> to.
* @return  the value <i>e</i><sup><code>a</code></sup>,
*          where <i>e</i> is the base of the natural logarithms.
)

: java.lang.Math.exp§-1340119808
   2LOCAL a
   
   \ new statement
   
   \ new statement
   a FEXP
   
   \ new statement
   0 §break429 LABEL
   
   \ new statement
   
   PURGE 1
   
   \ new statement
   DROP
;
( *
* Returns the largest (closest to positive infinity)
* <code>double</code> value that is not greater than the argument and
* is equal to a mathematical integer. Special cases:
* <ul><li>If the argument value is already equal to a mathematical
* integer, then the result is the same as the argument.
* <li>If the argument is NaN or an infinity or positive zero or
* negative zero, then the result is the same as the argument.</ul>
*
* @param   a   a value.
* <!--@return  the value &lfloor;&nbsp;<code>a</code>&nbsp;&rfloor;.-->
* @return  the largest (closest to positive infinity)
*          floating-point value that is not greater than the argument
*          and is equal to a mathematical integer.
)

: java.lang.Math.floor§2139252224
   2LOCAL a
   
   \ new statement
   
   \ new statement
   a FLOOR
   
   \ new statement
   0 §break434 LABEL
   
   \ new statement
   
   PURGE 1
   
   \ new statement
   DROP
;
( *
* Returns the natural logarithm (base <i>e</i>) of a <code>double</code>
* value.  Special cases:
* <ul><li>If the argument is NaN or less than zero, then the result
* is NaN.
* <li>If the argument is positive infinity, then the result is
* positive infinity.
* <li>If the argument is positive zero or negative zero, then the
* result is negative infinity.</ul>
* <p>
* A result must be within 1 ulp of the correctly rounded result.  Results
* must be semi-monotonic.
*
* @param   a   a number greater than <code>0.0</code>.
* @return  the value ln&nbsp;<code>a</code>, the natural logarithm of
*          <code>a</code>.
)

: java.lang.Math.log§3501056
   2LOCAL a
   
   \ new statement
   
   \ new statement
   a FLN
   
   \ new statement
   0 §break430 LABEL
   
   \ new statement
   
   PURGE 1
   
   \ new statement
   DROP
;
( *
* Returns the greater of two <code>long</code> values. That is, the
* result is the argument closer to the value of
* <code>Long.MAX_VALUE</code>. If the arguments have the same value,
* the result is that same value.
*
* @param   a   an argument.
* @param   b   another argument.
* @return  the larger of <code>a</code> and <code>b</code>.
* @see     java.lang.Long#MAX_VALUE
)

: java.lang.Math.max§-67736320
   2LOCAL b
   2LOCAL a
   
   \ new statement
   
   \ new statement
   a
   b
   D< INVERT
   
   IF
      a
   ELSE
      b
      
   ENDIF
   0 §break444 BRANCH
   
   \ new statement
   0 §break444 LABEL
   
   \ new statement
   
   
   PURGE 2
   
   \ new statement
   DROP
;
( *
* Returns the greater of two <code>int</code> values. That is, the
* result is the argument closer to the value of
* <code>Integer.MAX_VALUE</code>. If the arguments have the same value,
* the result is that same value.
*
* @param   a   an argument.
* @param   b   another argument.
* @return  the larger of <code>a</code> and <code>b</code>.
* @see     java.lang.Long#MAX_VALUE
)

: java.lang.Math.max§1704291584
   LOCAL b
   LOCAL a
   
   \ new statement
   
   \ new statement
   a b MAX
   
   \ new statement
   0 §break443 LABEL
   
   \ new statement
   
   
   PURGE 2
   
   \ new statement
   DROP
;
( *
* Returns the greater of two <code>double</code> values.  That
* is, the result is the argument closer to positive infinity. If
* the arguments have the same value, the result is that same
* value. If either value is NaN, then the result is NaN.  Unlike
* the the numerical comparison operators, this method considers
* negative zero to be strictly smaller than positive zero. If one
* argument is positive zero and the other negative zero, the
* result is positive zero.
*
* @param   a   an argument.
* @param   b   another argument.
* @return  the larger of <code>a</code> and <code>b</code>.
)

: java.lang.Math.max§76639488
   2LOCAL b
   2LOCAL a
   
   \ new statement
   
   \ new statement
   a b FMAX
   
   \ new statement
   0 §break445 LABEL
   
   \ new statement
   
   
   PURGE 2
   
   \ new statement
   DROP
;
( *
* Returns the smaller of two <code>long</code> values. That is,
* the result is the argument closer to the value of
* <code>Long.MIN_VALUE</code>. If the arguments have the same
* value, the result is that same value.
*
* @param   a   an argument.
* @param   b   another argument.
* @return  the smaller of <code>a</code> and <code>b</code>.
* @see     java.lang.Long#MIN_VALUE
)

: java.lang.Math.min§1264872704
   2LOCAL b
   2LOCAL a
   
   \ new statement
   
   \ new statement
   a
   b
   D> INVERT
   
   IF
      a
   ELSE
      b
      
   ENDIF
   0 §break447 BRANCH
   
   \ new statement
   0 §break447 LABEL
   
   \ new statement
   
   
   PURGE 2
   
   \ new statement
   DROP
;
( *
* Returns the smaller of two <code>double</code> values.  That
* is, the result is the value closer to negative infinity. If the
* arguments have the same value, the result is that same
* value. If either value is NaN, then the result is NaN.  Unlike
* the the numerical comparison operators, this method considers
* negative zero to be strictly smaller than positive zero. If one
* argument is positive zero and the other is negative zero, the
* result is negative zero.
*
* @param   a   an argument.
* @param   b   another argument.
* @return  the smaller of <code>a</code> and <code>b</code>.
)

: java.lang.Math.min§1500146944
   2LOCAL b
   2LOCAL a
   
   \ new statement
   
   \ new statement
   a b FMIN
   
   \ new statement
   0 §break448 LABEL
   
   \ new statement
   
   
   PURGE 2
   
   \ new statement
   DROP
;
( *
* Returns the smaller of two <code>int</code> values. That is,
* the result the argument closer to the value of
* <code>Integer.MIN_VALUE</code>.  If the arguments have the same
* value, the result is that same value.
*
* @param   a   an argument.
* @param   b   another argument.
* @return  the smaller of <code>a</code> and <code>b</code>.
* @see     java.lang.Long#MIN_VALUE
)

: java.lang.Math.min§61172992
   LOCAL b
   LOCAL a
   
   \ new statement
   
   \ new statement
   a b MIN
   
   \ new statement
   0 §break446 LABEL
   
   \ new statement
   
   
   PURGE 2
   
   \ new statement
   DROP
;
( *
* Returns the value of the first argument raised to the power of the
* second argument. Special cases:
*
* <ul><li>If the second argument is positive or negative zero, then the
* result is 1.0.
* <li>If the second argument is 1.0, then the result is the same as the
* first argument.
* <li>If the second argument is NaN, then the result is NaN.
* <li>If the first argument is NaN and the second argument is nonzero,
* then the result is NaN.
*
* <li>If
* <ul>
* <li>the absolute value of the first argument is greater than 1
* and the second argument is positive infinity, or
* <li>the absolute value of the first argument is less than 1 and
* the second argument is negative infinity,
* </ul>
* then the result is positive infinity.
*
* <li>If
* <ul>
* <li>the absolute value of the first argument is greater than 1 and
* the second argument is negative infinity, or
* <li>the absolute value of the
* first argument is less than 1 and the second argument is positive
* infinity,
* </ul>
* then the result is positive zero.
*
* <li>If the absolute value of the first argument equals 1 and the
* second argument is infinite, then the result is NaN.
*
* <li>If
* <ul>
* <li>the first argument is positive zero and the second argument
* is greater than zero, or
* <li>the first argument is positive infinity and the second
* argument is less than zero,
* </ul>
* then the result is positive zero.
*
* <li>If
* <ul>
* <li>the first argument is positive zero and the second argument
* is less than zero, or
* <li>the first argument is positive infinity and the second
* argument is greater than zero,
* </ul>
* then the result is positive infinity.
*
* <li>If
* <ul>
* <li>the first argument is negative zero and the second argument
* is greater than zero but not a finite odd integer, or
* <li>the first argument is negative infinity and the second
* argument is less than zero but not a finite odd integer,
* </ul>
* then the result is positive zero.
*
* <li>If
* <ul>
* <li>the first argument is negative zero and the second argument
* is a positive finite odd integer, or
* <li>the first argument is negative infinity and the second
* argument is a negative finite odd integer,
* </ul>
* then the result is negative zero.
*
* <li>If
* <ul>
* <li>the first argument is negative zero and the second argument
* is less than zero but not a finite odd integer, or
* <li>the first argument is negative infinity and the second
* argument is greater than zero but not a finite odd integer,
* </ul>
* then the result is positive infinity.
*
* <li>If
* <ul>
* <li>the first argument is negative zero and the second argument
* is a negative finite odd integer, or
* <li>the first argument is negative infinity and the second
* argument is a positive finite odd integer,
* </ul>
* then the result is negative infinity.
*
* <li>If the first argument is finite and less than zero
* <ul>
* <li> if the second argument is a finite even integer, the
* result is equal to the result of raising the absolute value of
* the first argument to the power of the second argument
*
* <li>if the second argument is a finite odd integer, the result
* is equal to the negative of the result of raising the absolute
* value of the first argument to the power of the second
* argument
*
* <li>if the second argument is finite and not an integer, then
* the result is NaN.
* </ul>
*
* <li>If both arguments are integers, then the result is exactly equal
* to the mathematical result of raising the first argument to the power
* of the second argument if that result can in fact be represented
* exactly as a <code>double</code> value.</ul>
*
* <p>(In the foregoing descriptions, a floating-point value is
* considered to be an integer if and only if it is finite and a
* fixed point of the method {@link #ceil <tt>ceil</tt>} or,
* equivalently, a fixed point of the method {@link #floor
* <tt>floor</tt>}. A value is a fixed point of a one-argument
* method if and only if the result of applying the method to the
* value is equal to the value.)
*
* <p>A result must be within 1 ulp of the correctly rounded
* result.  Results must be semi-monotonic.
*
* @param   a   the base.
* @param   b   the exponent.
* @return  the value <code>a<sup>b</sup></code>.
)

: java.lang.Math.pow§1665822720
   2LOCAL b
   2LOCAL a
   
   \ new statement
   
   \ new statement
   a b F**
   
   \ new statement
   0 §break437 LABEL
   
   \ new statement
   
   
   PURGE 2
   
   \ new statement
   DROP
;
( *
* Returns the <code>double</code> value that is closest in value
* to the argument and is equal to a mathematical integer. If two
* <code>double</code> values that are mathematical integers are
* equally close, the result is the integer value that is
* even. Special cases:
* <ul><li>If the argument value is already equal to a mathematical
* integer, then the result is the same as the argument.
* <li>If the argument is NaN or an infinity or positive zero or negative
* zero, then the result is the same as the argument.</ul>
*
* @param   a   a <code>double</code> value.
* @return  the closest floating-point value to <code>a</code> that is
*          equal to a mathematical integer.
)

: java.lang.Math.rint§486240768
   2LOCAL a
   
   \ new statement
   
   \ new statement
   a D>F F>D
   
   \ new statement
   0 §break435 LABEL
   
   \ new statement
   
   PURGE 1
   
   \ new statement
   DROP
;
( *
* Returns the closest <code>int</code> to the argument. The
* result is rounded to an integer by adding 1/2, taking the
* floor of the result, and casting the result to type <code>int</code>.
* In other words, the result is equal to the value of the expression:
* <p><pre>(int)Math.floor(a + 0.5f)</pre>
* <p>
* Special cases:
* <ul><li>If the argument is NaN, the result is 0.
* <li>If the argument is negative infinity or any value less than or
* equal to the value of <code>Integer.MIN_VALUE</code>, the result is
* equal to the value of <code>Integer.MIN_VALUE</code>.
* <li>If the argument is positive infinity or any value greater than or
* equal to the value of <code>Integer.MAX_VALUE</code>, the result is
* equal to the value of <code>Integer.MAX_VALUE</code>.</ul>
*
* @param   a   a floating-point value to be rounded to an integer.
* @return  the value of the argument rounded to the nearest
*          <code>int</code> value.
* @see     java.lang.Integer#MAX_VALUE
* @see     java.lang.Integer#MIN_VALUE
)

: java.lang.Math.round§-1758170624
   2LOCAL a
   
   \ new statement
   
   \ new statement
   a
   0.5
   F+
   java.lang.Math.floor§2139252224
   F>D D>S
   0 §break438 BRANCH
   
   \ new statement
   0 §break438 LABEL
   
   \ new statement
   
   PURGE 1
   
   \ new statement
   DROP
;
( *
* Returns the closest <code>long</code> to the argument. The result
* is rounded to an integer by adding 1/2, taking the floor of the
* result, and casting the result to type <code>long</code>. In other
* words, the result is equal to the value of the expression:
* <p><pre>(long)Math.floor(a + 0.5d)</pre>
* <p>
* Special cases:
* <ul><li>If the argument is NaN, the result is 0.
* <li>If the argument is negative infinity or any value less than or
* equal to the value of <code>Long.MIN_VALUE</code>, the result is
* equal to the value of <code>Long.MIN_VALUE</code>.
* <li>If the argument is positive infinity or any value greater than or
* equal to the value of <code>Long.MAX_VALUE</code>, the result is
* equal to the value of <code>Long.MAX_VALUE</code>.</ul>
*
* @param   a   a floating-point value to be rounded to a
*		<code>long</code>.
* @return  the value of the argument rounded to the nearest
*          <code>long</code> value.
* @see     java.lang.Long#MAX_VALUE
* @see     java.lang.Long#MIN_VALUE
)

: java.lang.Math.round§-1814662656
   2LOCAL a
   
   \ new statement
   
   \ new statement
   a
   0.5
   F+
   java.lang.Math.floor§2139252224
   F>D
   0 §break439 BRANCH
   
   \ new statement
   0 §break439 LABEL
   
   \ new statement
   
   PURGE 1
   
   \ new statement
   DROP
;
( *
* Returns the trigonometric sine of an angle.  Special cases:
* <ul><li>If the argument is NaN or an infinity, then the
* result is NaN.
* <li>If the argument is zero, then the result is a zero with the
* same sign as the argument.</ul>
* <p>
* A result must be within 1 ulp of the correctly rounded result.  Results
* must be semi-monotonic.
*
* @param   a   an angle, in radians.
* @return  the sine of the argument.
)

: java.lang.Math.sin§1203401472
   2LOCAL a
   
   \ new statement
   
   \ new statement
   a FSIN
   
   \ new statement
   0 §break421 LABEL
   
   \ new statement
   
   PURGE 1
   
   \ new statement
   DROP
;


: java.lang.Math.sinh§-1489865984
   2LOCAL a
   
   \ new statement
   
   \ new statement
   a FSINH
   
   \ new statement
   0 §break449 LABEL
   
   \ new statement
   
   PURGE 1
   
   \ new statement
   DROP
;
( *
* Returns the correctly rounded positive square root of a
* <code>double</code> value.
* Special cases:
* <ul><li>If the argument is NaN or less than zero, then the result
* is NaN.
* <li>If the argument is positive infinity, then the result is positive
* infinity.
* <li>If the argument is positive zero or negative zero, then the
* result is the same as the argument.</ul>
* Otherwise, the result is the <code>double</code> value closest to
* the true mathematical square root of the argument value.
*
* @param   a   a value.
* <!--@return  the value of &radic;&nbsp;<code>a</code>.-->
* @return  the positive square root of <code>a</code>.
*          If the argument is NaN or less than zero, the result is NaN.
)

: java.lang.Math.sqrt§851931904
   2LOCAL a
   
   \ new statement
   
   \ new statement
   a FSQRT
   
   \ new statement
   0 §break431 LABEL
   
   \ new statement
   
   PURGE 1
   
   \ new statement
   DROP
;
( *
* Returns the trigonometric tangent of an angle.  Special cases:
* <ul><li>If the argument is NaN or an infinity, then the result
* is NaN.
* <li>If the argument is zero, then the result is a zero with the
* same sign as the argument.</ul>
* <p>
* A result must be within 1 ulp of the correctly rounded result.  Results
* must be semi-monotonic.
*
* @param   a   an angle, in radians.
* @return  the tangent of the argument.
)

: java.lang.Math.tan§-1301580800
   2LOCAL a
   
   \ new statement
   
   \ new statement
   a FTAN
   
   \ new statement
   0 §break423 LABEL
   
   \ new statement
   
   PURGE 1
   
   \ new statement
   DROP
;


: java.lang.Math.tanh§1623421952
   2LOCAL a
   
   \ new statement
   
   \ new statement
   a FTANH
   
   \ new statement
   0 §break451 LABEL
   
   \ new statement
   
   PURGE 1
   
   \ new statement
   DROP
;
( *
* Converts an angle measured in radians to an approximately
* equivalent angle measured in degrees.  The conversion from
* radians to degrees is generally inexact; users should
* <i>not</i> expect <code>cos(toRadians(90.0))</code> to exactly
* equal <code>0.0</code>.
*
* @param   angrad   an angle, in radians
* @return  the measurement of the angle <code>angrad</code>
*          in degrees.
* @since   1.2
)

: java.lang.Math.toDegrees§2084860928
   2LOCAL angrad
   
   \ new statement
   
   \ new statement
   angrad
   180.0
   F*
   java.lang.Math.PI
   2@
   F/
   0 §break428 BRANCH
   
   \ new statement
   0 §break428 LABEL
   
   \ new statement
   
   PURGE 1
   
   \ new statement
   DROP
;
( *
* Converts an angle measured in degrees to an approximately
* equivalent angle measured in radians.  The conversion from
* degrees to radians is generally inexact.
*
* @param   angdeg   an angle, in degrees
* @return  the measurement of the angle <code>angdeg</code>
*          in radians.
* @since   1.2
)

: java.lang.Math.toRadians§-173575168
   2LOCAL angdeg
   
   \ new statement
   
   \ new statement
   angdeg
   180.0
   F/
   java.lang.Math.PI
   2@
   F*
   0 §break427 BRANCH
   
   \ new statement
   0 §break427 LABEL
   
   \ new statement
   
   PURGE 1
   
   \ new statement
   DROP
;


:LOCAL java.lang.Math.~destructor§-1086882304
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
      equation.JavaArray.kill§1620077312
      
      \ new statement
      §this CELL+ @ 8 +  @ TO obj
      
      \ new statement
      obj
      0
      equation.JavaArray.kill§1620077312
      
      \ new statement
   ENDIF
   §this DECREFERENCE
   
   \ new statement
   0 §break455 LABEL
   
   \ new statement
   PURGE 2
   
   \ new statement
   DROP
;
2VARIABLE java.lang.Math.E
2VARIABLE java.lang.Math.PI
VARIABLE java.lang.Math._staticBlocking

A:HERE VARIABLE java.lang.Math§-1239069440.table 2 DUP 2* CELLS ALLOT R@ ! A:CELL+
-590525184 R@ ! A:CELL+ java.lang.Math.Math§-590525184 VAL R@ ! A:CELL+
-1086882304 R@ ! A:CELL+ java.lang.Math.~destructor§-1086882304 VAL R@ ! A:CELL+
A:DROP
