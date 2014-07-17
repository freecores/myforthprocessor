
\ new statement
0
Double._staticBlocking
!

\ new statement
0
Double._staticThread
!

\ new statement
1.0
Double.one
2!

\ new statement
0.0
Double.zero
2!

\ new statement
( *
* A constant holding the positive infinity of type
* <code>double</code>. It is equal to the value returned by
* <code>Double.longBitsToDouble(0x7ff0000000000000L)</code>.
)

\ new statement
Double.one
2@
Double.zero
2@
F/
Double.POSITIVE_INFINITY
2!

\ new statement
( *
* A constant holding the negative infinity of type
* <code>double</code>. It is equal to the value returned by
* <code>Double.longBitsToDouble(0xfff0000000000000L)</code>.
)

\ new statement
Double.one
A:R@ 2@ FNEGATE
A:DROP
Double.zero
2@
F/
Double.NEGATIVE_INFINITY
2!

\ new statement
( *
* A constant holding a Not-a-Number (NaN) value of type
* <code>double</code>. It is equivalent to the value returned by
* <code>Double.longBitsToDouble(0x7ff8000000000000L)</code>.
)

\ new statement
Double.zero
2@
Double.zero
2@
F/
Double.NaN
2!

\ new statement
( *
* A constant holding the largest positive finite value of type
* <code>double</code>, (2-2<sup>-52</sup>)&middot;2<sup>1023</sup>.
* It is equal to the value returned by:
* <code>Double.longBitsToDouble(0x7fefffffffffffffL)</code>.
)

\ new statement
1.7976931348623157E308
Double.MAX_VALUE
2!

\ new statement
( *
* A constant holding the smallest positive nonzero value of type
* <code>double</code>, 2<sup>-1074</sup>. It is equal to the
* value returned by <code>Double.longBitsToDouble(0x1L)</code>.
)

\ new statement
4.9E-324
Double.MIN_VALUE
2!

\ new statement
( * use serialVersionUID from JDK 1.0.2 for interoperability  )

\ new statement
-9172774392245257468
Double.serialVersionUID
2!
