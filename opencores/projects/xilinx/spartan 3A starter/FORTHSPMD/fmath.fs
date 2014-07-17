( 
  BIOS for FORTH-System

  Copyright © 2004-2008 by Gerhard Hohner
)
  

( negate a double float value
  parameter:
      double float
  return:
      double float
)

: FNEGATE A:R@ DROP 80000000H XOR R@ A:DROP ;

( convert double integer to double float
  parameter
      double integer
  return
      double float value
)

: D>F OVER OVER D0= IF EXIT ENDIF ( zero )
      OVER 0< IF A:0C3FH DNEGATE ELSE A:43FH ENDIF
      BEGIN A:1- OVER OVER D+ B@ UNTIL ( normalize mantissa )
      12 RSHIFT OVER 20 LSHIFT OR A:R@ DROP ( adjust lower half )
      12 RSHIFT R1@ 20 LSHIFT OR ( build upper half )
      R@ A:2DROP ;

( calculate power of integer values
  parameter:
      power
      double base
  return:
      double float
)

:LOCAL INTPOWER A:R@ DROP	( move power )
                A:R1@ A:R@ 2DROP ( move base )
                1. ( start with 1. )
                A:2 A:PICK R@ A:DROP ABS
                BEGIN
                      DUP
                WHILE
                      2/ B@
                      IF R1@ A:R1@ NIP R1@ F* CALL R@ A:DROP ENDIF ( multiply with power of base )
                      R1@ A:NIP A:R1@ NIP R1@ A:NIP OVER OVER F* CALL	( square power of base )
                      A:R1@ NIP R1@ A:NIP A:R1@ NIP ( move square )
                REPEAT
                DROP A:2DROP
                A:0< A:IF A:R1@ A:R@ 2DROP 1. R1@ R@ A:2DROP F/ CALL ENDIF ( build negative power ) ;

( convert string to double float value
  parameter
      length of string
      address of string
  return
      valid
      double float value
)

: >FLOAT A:R@ A:R1@ 2DROP ( move length, address )
         0 DUP ( initial value )
         R@ R1@ A:2DROP ( restore address, length )
         >NUMBER CALL ( convert signed integer part )
         A:R@ ( copy length )
         OVER C@ ( load next character )
         [CHAR] . = OVER 0<> AND ( fraction follows? )
         IF 
            A:DROP 1- A:R@ DROP ( adjust length )
            1+ A:R@ DROP ( adjust address )
            A:R1@ DABS
            R1@ A:NIP R1@
            >NUMBER CALL ( convert fraction )
            A:R@ DROP R1@ A:NIP A:R1@ NIP
            0< IF DNEGATE ENDIF
            R@ R1@ A:2DROP
         ENDIF
         A:R@ A:- A:NEGATE ( calculate exponent )
         A:R@ A:R1@ 2DROP
	   D>F CALL ( convert mantissa to double )
         R@ C@ [CHAR] E =! NIP OVER [CHAR] e = OR NIP R1@ 0<> AND ( exponent follows? )
         IF
            0 DUP ( initial value )
            R@ 1+ R1@ 1- A:2DROP ( adjust address, length )
            >NUMBER CALL ( convert exponent )
            2DROP A:R@ A:+ ( modify exponent )
            A:R@ A:R1@ 2DROP
         ENDIF
         A:2DROP ( remove length, address )
         0 BASE @ D>F CALL R@ A:DROP INTPOWER CALL
         F* CALL ( join mantissa and exponent )
         TRUE ;

( multiply unsigned double words
  parameter
      double multiplicator
      double multiplikand
      double initial for product
  return
      rounded 64-bit product
)

\ :LOCAL *u53 53 0 DO 
\                    A:R1@ A:2/ A:R@ A:1 A:RSHIFTC 2DROP ( move multiplicator >> 1 )
\                    A:R@ A:R1@ 2DROP A:B@ ( move multiplicand, save bit shifted out )
\                    A:R@ DROP 2/ R@ A:DROP 1 RSHIFTC ( product >> 1 )
\                    A:IF R1@ + A:R@ DROP R1@ +B R@ A:DROP ENDIF ( product += multiplicand )
\                    R@ R1@ A:2DROP ( move multiplicand )
\                    R1@ R@ A:2DROP ( move multiplicator )
\                 LOOP
\            2DROP 2DROP ( remove multiplicator and multiplicand )
\            A:R@ DROP 2/ A:1 A:RSHIFTC A:0 A:+B 0 +B R@ A:DROP ( round product ) ;
:LOCAL *u53 64! 64! 1 FPU 32@ 32@ 32@ 32@ ( multiply )
            A:R1@ 2DROP
            A:R> 11 LSHIFT R@ 21 RSHIFT OR R@ 11 LSHIFT 0 R1@ A:2DROP 21 RSHIFT 0 +B D+
            ;

( multiply float double values
  parameter
      multiplicator
      multiplicand
  return
      product
)

: F* A:DI A:R@ DROP A:R@ A:000fffffH A:AND ( save mantissa of multiplicator )
     0fff00000H AND ( extract sign, exponent of multiplicator )
     DUP 7ff00000H AND 0<> IF A:00100000H A:OR ENDIF ( restore leading bit of mantissa )
     A:R1@ NIP A:R1@ NIP R@ A:000fffffH A:AND ( save mantissa of multiplicand ) 
     0fff00000H AND DUP 7ff00000H AND 0<> IF A:00100000H A:OR ENDIF ( restore leading bit of mantissa )
     A:R@ A:R1@
     A:XOR A:80000000H A:AND ( sign of product )
     7ff00000H AND 2/ SWAP 7ff00000H AND 2/ + 1ff80000H - ( sum of exponents )
     3ff00000H >! NIP IF DROP A:2* 0ffe00000H 1 RSHIFTC 0H A:DROP A:2DROP A:2DROP A:EI EXIT ENDIF ( handle overflow )
	  0 MAX
     2* A:R@ DROP A:OR ( join sign, exponent of product )
     \ 0 DUP ( initial value of product )
     R1@ A:NIP R1@ A:NIP ( multiplicand )
     OVER OVER D+ ( shift left for rounding )
     R1@ A:NIP R1@ A:NIP ( multiplicator )
     *u53 CALL
     OVER 100000H <! NIP IF A:80000000H A:AND ENDIF ( underflow )
     1fffffH > IF A:R@ DROP 2/ A:1 A:RSHIFTC R@ A:DROP A:100000H A:+ ENDIF ( adjust mantissa )
     A:R@ DROP
     0fffffH AND R1@ OR ( complete upper half of product )
     R@ A:2DROP A:EI ;

( split float into mantissa and exponent 
  parameter:
      double float
  return:
      exponent
      low word of mantissa
      high word of mantissa
)

:LOCAL SPLITFLOAT A:R1@ A:R@ DROP
                  0fffffH AND
                  R1@ 7ff00000H AND A:R@ IF 100000H OR ENDIF ( insert leading bit )
                  R1@ A:NIP ( move low word of mantissa )
                  2* A:R> DUP +B R> ( shift one bit left )
			R1@ 0< IF DNEGATE ENDIF ( mantissa signed )
                  R@ A:2DROP ( pure exponent )
                  ;

( difference lower than epsilon
  parameter:
      iterations
      exponent1
      exponent2
  return:
      -1, if difference lower than epsilon and exponent1 <> 0
)

:LOCAL GOON 50 < ROT 2* 21 RSHIFT ROT 2* 21 RSHIFT OVER OVER - ABS 54 < OVER 0<> AND NIP NIP AND ;

( add double float values
  parameter
      float value
      float value
  return
      sum
)

: F+ A:R1@ A:R@ 2DROP
     SPLITFLOAT CALL

     R1@ A:NIP A:R1@ NIP R1@ A:NIP ( move operand2, exponent1 ) 
     SPLITFLOAT CALL A:R> ( copy exponent1 )

     R1@ 20 RSHIFT R@ 20 RSHIFT - 0<! ( exponent1 - exponent2 )
     IF ( adjust first mantissa )
        A:NIP ( new exponent is exponent2 )
        A:R1@ NIP A:R1@ NIP ( save mantissa2 )
        NEGATE ( shift factor must be positive )
        ASHIFT CALL ( adjust mantissa1 )
        R@ R1@ A:2DROP ( move mantissa2 )
     ELSE ( adjust second mantissa )
        A:DROP ( new exponent is exponent1 )
        ASHIFT CALL ( adjust mantissa2 )
     ENDIF

     D+
     A:2* OVER 2* A:1 A:RSHIFTC ( insert sign into exponent )
     DROP DABS ( unsigned mantissa )
     OVER OVER OR 0= IF A:80000000H A:AND ENDIF ( result is 0 )
     BEGIN
           OVER 200000H < R@ 2* AND ( mantissa < 2000000000000H and exponent != 0 )
     WHILE
           OVER OVER D+
           A:100000H A:-
     REPEAT
     R@ A:DROP ( move exponent )
     2 0 DO
            A:R@ DROP
            OVER 3fffffH > R@ 2* 0ffe00000H <> AND
            IF
               A:R@ DROP 2/ R@ A:DROP 1 RSHIFTC
               A:100000H A:+
            ENDIF
            0 DUP 1+ D+ -2 AND ( round sum )
            R@ A:DROP
         LOOP
     A:R@ DROP ( move exponent )
     A:R@ DROP 2/ R@ A:DROP 1 RSHIFTC ( shift sum right )
     OVER 0fffffH > R@ 2* 0= AND IF A:100000H A:+ ENDIF ( adjust exponent when leaving underrun )
     A:R@ DROP 0fffffH AND R1@ OR R@ A:2DROP ( combine exponent, mantissa )
     ;

( subtract double float values
  parameter
      minuend
      float value
  return
      difference
)

: F- FNEGATE CALL F+ CALL ;

( divide float values
  parameter
      divisor
      dividend
  return
      quotient
)

: F/ A:DI OVER 2* OVER OR 0=
     IF DROP NIP XOR 2* DROP 0ffe00000H 1 RSHIFTC 0 A:EI EXIT ENDIF ( zero divide )
     OVER DUP
     000fffffH AND ( pure mantissa of upper half )
     OVER 7ff00000H AND
     IF 100000H OR ENDIF ( insert leading 1 into upper half of mantissa )
     NIP OVER 64! DROP ( move mantissa to FPU )
     A:R@ DROP ( move exponent )

     A:R1@ ( save exponent of second operand )
     A:R@ ( save lower half of its mantissa )
     A:R1@ A:000fffffH A:AND ( pure mantissa of upper half )
     A:R1@ A:7ff00000H A:AND A:IF A:100000H A:OR ENDIF ( insert leading 1 into upper half of mantissa )
     \ 2DROP 0 DUP ( initial quotient )
     R@ R1@ A:2DROP 64! 2DROP ( move dividend to FPU )
     
     R@ 2* 21 RSHIFT
     R1@ 2* 21 RSHIFT
     - 3ffH + ( exponent1 - exponent2 )
     A:XOR A:80000000H A:AND ( sign of new exponent )
     >R ( raw new exponent )

     3 FPU 32@ 32@ 32@ A:32@ A:+ DROP
	  >R 2/ A:1 A:RSHIFTC A:0 A:+B 0 +B R> ( adjust and round )
	  
     BEGIN
	     OVER 1fffffH >
	  WHILE
	     >R 2/ R> 1 RSHIFTC
		  A:1+ ( adjust exponent )
	  REPEAT
	  
     R@ 7feH > IF 2DROP R@ A:2DROP 07ff00000H OR 0 A:EI EXIT ENDIF ( overflow )
     R@ -52 < IF 2DROP R@ A:2DROP 0 A:EI EXIT ENDIF ( underflow )
	  R@ 0< IF R> NEGATE SHIFTR CALL A:0 ENDIF
	  
	  A:20 A:LSHIFT A:OR ( sign, new exponent )
     
     >R
     0fffffH AND
     R1@ OR ( upper word of mantissa with exponent )
     R@ A:2DROP A:EI ;

( compute reciprocal using newton's method
  parameter:
      float value
  return:
      reciprocal
)

: RECIPROCAL A:R1@ A:R@ 2DROP 0
             R1@ 80000000H AND R1@
             2* 21 RSHIFT 3ffH - INVERT 3feH + 21 LSHIFT 1 RSHIFT OR A:R@ A:0
             0
             BEGIN
                   OVER R1@ 4 PICK GOON CALL R1@ 2* 21 RSHIFT 7ffH < AND
             WHILE
                   R1@ R@ A:2DROP F+ CALL ( sum += x(i) )
                   OVER OVER
                   OVER OVER OVER OVER F* CALL R1@ R@ F* CALL
                   F- CALL ( x(i + 1) = x(i) - x(i) * x(i) * value )
                   A:R1@ A:R@ 2DROP
                   2 PICK 1+ 2 PUT
             REPEAT
             ROT DROP
             A:2DROP A:2DROP ;

( convert float to double integer
  parameter
      float value
  return
      double integer
)

: F>D OVER 80000000H AND 3fe00000H OR 0 F+ CALL stripf ( round )
      OVER 7ff00000H AND 20 RSHIFT 3ffH - 0<! ( get exponent )
      IF
        DROP 2DROP 0 DUP ( exponent < 0, causes zero as result )
      ELSE
        62 >! NIP
        IF
          2DROP 0< IF 80000000H 0 ELSE 7fffffffH -1 ENDIF ( an extremum is the result )
        ELSE
          A:R@ A:R1@ 2DROP A:R@
          0fffffH AND 100000H OR R1@ A:NIP ( insert Msb )
          R1@ 52 - 0>!
          IF
             SHIFTL CALL ( adjust mantissa left )
          ELSE
             NEGATE SHIFTR CALL ( adjust mantissa right )
          ENDIF
          A:0< A:IF DNEGATE ENDIF ( make mantissa signed )
          A:DROP
        ENDIF
      ENDIF ;

( strip fraction of float value
  parameter
      float value
  return
      float value
)

: stripf OVER 7ff00000H AND 3ff00000H - ( get exponent )
         0<! IF 2DROP 80000000H AND 0 EXIT ENDIF ( zero )
         20 RSHIFT ( adjust exponent )
         A:52 A:R@ DROP A:- ( length of mantissa - exponent )
         A:0>! A:IF -1 A:32 A:- A:0<! A:IF 32 R@ + LSHIFT AND ELSE NIP R@ LSHIFT AND 0 ENDIF ENDIF ( strip fraction )
         A:DROP ;

( floor of double float value
  parameter
      double float value
  return
      double float value
)

: FLOOR OVER OVER stripf CALL A:R1@ A:R@ D= INVERT R1@ 0< AND R1@ A:NIP A:R1@ NIP R1@ A:NIP
        A:IF 1. F- CALL ENDIF ;

( maximum of two float values
  parameter
      second operand
      first operand
  return
      maximum
)

: FMAX OVER OVER 5 PICK 5 PICK F< ( second >= first ) INVERT IF A:R1@ A:R@ 2DROP 2DROP R1@ R@ A:2DROP OVER OVER ENDIF 2DROP ;

( minimum of two float values
  parameter
      second operand
      first operand
  return
      minimum
)

: FMIN OVER OVER 5 PICK 5 PICK F< ( second < first ) IF A:R1@ A:R@ 2DROP 2DROP R1@ R@ A:2DROP OVER OVER ENDIF 2DROP ;

( round fraction of float value
  parameter
      float value
  return
      float value
)

: FROUND OVER OVER stripf CALL A:R1@ A:R@ D= INVERT R1@ 0< INVERT AND R1@ A:NIP A:R1@ NIP R1@ A:NIP
         A:IF 1. F+ CALL ENDIF ;

( raise to the power
  parameter
      double float exponent
      double float base
  return
      exp[ ln[base] * exponent ]
)

: F** A:R1@ A:R@ 2DROP
      FLN CALL R1@ R@ A:2DROP F* CALL
      FEXP CALL ;

( arcus cosinus
  parameter
      double float argument
  return
      PI / 2 - asin(x)
)

: FACOS FASIN CALL 1.5707963267948966 F- CALL FNEGATE CALL ;

( arcus cosinus hyperbolicus
  parameter
      double float argument
  return
      ln[x + [x * x - 1] ** .5]
)

: FACOSH OVER OVER OVER OVER F* CALL 1.0 F- CALL FSQRT CALL F+ CALL FLN CALL ;

( raise to a power of ten
  parameter
      double float argument
  return
      exp[x * ln[10]]
)

: FALOG 2.302585092994046 F* CALL FEXP CALL ;

( arcus sinus
  parameter
      double float argument
  return
      atan[ x / [1 - x * x] ** .5]
)

: FASIN A:R1@ A:R@ 1.0 R1@ R@ A:2DROP OVER OVER F* CALL F- CALL FSQRT CALL F/ CALL FATAN CALL ;

( arcus sinus hyperbolicus
  parameter
      double float argument
  return
      ln[x + [1 + x * x] ** .5]
)

: FASINH OVER OVER OVER OVER F* CALL 1.0 F+ CALL FSQRT CALL F+ CALL FLN CALL ;

( arcus tangens
  parameter
      double float argument
  return
      sum[ x ** [i + i + 1] / [i + i + 1] * [-1] ** i ]
)

: FATAN OVER OVER OVER OVER F* CALL FNEGATE CALL A:R1@ A:R@ 2DROP ( - x ** 2 )
        A:R1@ A:R@ 2DROP ( move x )
        0 R1@ R@ ( counter, initial sum )
        BEGIN
              OVER R1@ 4 PICK GOON CALL
        WHILE
              R1@ R@ A:2DROP R1@ R@ F* CALL A:R1@ A:R@ 4 PICK 1+ DUP 5 PUT 2* 1+ S>D D>F CALL F/ CALL F+ CALL
        REPEAT
        ROT DROP
        A:2DROP A:2DROP ;

( arcus tangens of r1/r2
  parameter
      double float r1
      double float r2
  return
      double radians
)

: FATAN2 3 PICK A:R@ DROP A:R1@
         F/ CALL FATAN CALL
         A:0< A:IF 3.141592653589793 ( r2 < 0 )
                   A:0<! A:IF F- CALL ( r1 < 0 )
                         ELSE F+ CALL ( r1 > 0 )
                         ENDIF
              ENDIF
         A:DROP ;

( arcus tangens hyperbolicus
  parameter
      double float argument
  return
      .5 * ln[ [x + 1] / [1 - x] ]
)

: FATANH 1.0 F+ CALL ( x + 1 )
         OVER OVER 2.0 F- CALL FNEGATE CALL ( 1 - x )
         F/ CALL FLN CALL .5 F* CALL ;

( cosinus
  parameter
      double float radians
  return
      double cosinus
)

: FCOS OVER OVER F* CALL FNEGATE CALL A:R1@ A:R@ 2DROP ( - x ** 2 )
       0 1.0 A:R1@ A:R@ ( count, initial sum )
       BEGIN
             OVER R1@ 4 PICK GOON CALL
       WHILE
             R1@ R@ A:2DROP R1@ R@ F* CALL 4 PICK 1+ DUP 5 PUT 2* DUP 1- M* CALL D>F CALL F/ CALL A:R1@ A:R@ F+ CALL
       REPEAT
       ROT DROP
       A:2DROP A:2DROP ;

( cosinus hyperbolicus
  parameter
      double float radians
  return
      double cosinus hyperbolicus
)

: FCOSH FEXP CALL A:R1@ A:R@ 1.0 R1@ R@ A:2DROP F/ CALL F+ CALL .5 F* CALL ;

( exp[argument]
  parameter
      double float argument
  return
      double exp[argument]
)

: FEXP A:R1@ A:R@ A:R1@ A:R@ 2DROP 
       1
       1.0 R1@ R@ F+ CALL	( 1 + x )
       BEGIN
             OVER R1@ 4 PICK GOON CALL
       WHILE
             R1@ R@ A:2DROP R1@ R@ F* CALL 0 5 PICK 1+ DUP 6 PUT D>F CALL F/ CALL A:R1@ A:R@ F+ CALL
       REPEAT
       ROT DROP A:2DROP A:2DROP ;

( exp[argument] - 1
  parameter
      double float argument
  return
      double exp[argument] - 1
)

: FEXPM1 FEXP CALL 1.0 F- CALL ;

( ln[argument]
  parameter
      double float argument
  return
      double ln[argument]
)

: FLN OVER DUP 7ff00000H AND 0= OVER 0< OR NIP
      IF DROP 2* DROP 0ffe00000H 1 RSHIFTC 0 EXIT ENDIF ( illegal argument ) \ 2.7182818284590455
      A:R1@ A:R@ DROP
      2* 21 RSHIFT 3ffH -
      S>D D>F CALL 
      0.6931471805599453 F* CALL
      R1@ R@ R1@ A:2DROP 7ff00000H AND 0 F/ CALL ( normalize )
      A:R1@ A:R@ 1. F- CALL R1@ R@ A:2DROP F/ CALL ( (x - 1) / x )
      A:R1@ A:R@ 2DROP A:OVER A:OVER
      1 R1@ R@ ( counter, initial sum )
      BEGIN
            OVER R1@ 4 PICK GOON CALL
      WHILE
            R1@ R@ A:2DROP R1@ R@ F* CALL A:R1@ A:R@ 4 PICK 1+ DUP 5 PUT S>D D>F CALL F/ CALL F+ CALL
      REPEAT
      ROT DROP
      A:2DROP A:2DROP
	F+ CALL ;

( ln[argument + 1]
  parameter
      double float argument
  return
      double ln[argument + 1]
)

: FLNP1 1.0 F+ CALL FLN CALL ;

( log[argument]
  parameter
      double float argument
  return
      double log[argument]
)

: FLOG FLN CALL 2.302585092994046 F/ CALL ;

( sinus
  parameter
      double float radians
  return
      double sinus
)

: FSIN OVER OVER OVER OVER F* CALL FNEGATE CALL A:R1@ A:R@ 2DROP ( - x ** 2 )
       A:R1@ A:R@ 2DROP ( move x )
       0 R1@ R@ ( counter, initial sum )
       BEGIN
             OVER R1@ 4 PICK GOON CALL
       WHILE
             R1@ R@ A:2DROP R1@ R@ F* CALL 4 PICK 1+ DUP 5 PUT 2* DUP 1+ M* CALL D>F CALL F/ CALL A:R1@ A:R@ F+ CALL
       REPEAT
       ROT DROP
       A:2DROP A:2DROP ;

( sinus hyperbolicus
  parameter
      double float radians
  return
      double
)

: FSINH FEXP CALL A:R1@ A:R@ 1. R1@ R@ A:2DROP F/ CALL F- CALL 2. F/ CALL ;

( sinus, cosinus
  parameter
      double float radians
  return
      double cosinus
      double sinus
)

: FSINCOS A:R1@ A:R@ FSIN CALL R1@ R@ A:2DROP FCOS CALL ;

( square root
  parameter
      double float argument
  return
      double square root
)

: FSQRT FABS A:R1@ A:R@ 2DROP ( save absolute argument )
        0 ( iteration )
        R1@ R@
        2. F/ CALL ( initial root value )
        
        BEGIN
              OVER OVER OVER OVER F* CALL R1@ R@ F- CALL ( x' ** 2 - x )
              3 PICK 3 PICK ( x' )
              2. F* CALL ( 2 * x' )
              F/ CALL
              A:R1@ F- CALL ( new aproximation )
              OVER R> 4 PICK 1+ DUP 5 PUT GOON CALL INVERT
        UNTIL
        ROT DROP
        A:2DROP ;

( tangens 
  parameter
      double float radians
  return
      double tangens
)

: FTAN FSINCOS CALL F/ CALL ;

( tangens hyperbolicus
  parameter
      double float radians
  return
      FSINH / FCOSH
)

: FTANH FEXP CALL A:R1@ A:R@
        1.0 R1@ R@ F/ CALL
        A:R1@ A:R@
        F- CALL
        R1@ R@ A:2DROP R1@ R@ A:2DROP F+ CALL
        F/ CALL ;

( r1 approximately r2
  parameter
      double float r3
      double float r2
      double float r1
  return
      true, if r3 > 0 and |r1 - r2| < r3
            if r3 == 0 and r1 == r2
      false otherwise
)

: F~ A:R1@ A:R@ 2DROP F- CALL FABS OVER OVER OR 0= R1@ A:NIP A:R1@ NIP R1@ F< R@ A:2DROP OR ;

( display double float in fixed point representation
  parameter:
      double float
  return:
      length of string
      address of string
)

: F.. STRINGIFY CALL
      DUP ABS PRECISION @ >
      IF
        [CHAR] . 1 INSERTCHAR CALL REMOVEZEROES CALL APPENDEXPONENT CALL ( use scientific representation instead )
      ELSE
        0<!
        IF ( insert leading zero )
            A:PAD A:DUP A:H@
            NEGATE 0 DO [CHAR] 0 0 INSERTCHAR CALL LOOP
            A:OVER A:H! A:DROP
            0
        ENDIF
        [CHAR] . OVER 1+ INSERTCHAR CALL DROP REMOVEZEROES CALL 
      ENDIF
      PAD COUNT ;

: F. F.. CALL TYPE CALL ;

( display double float in scientific representation
  parameter:
      double float
  return:
      length of string
      address of string
)

: FS.. STRINGIFY CALL REMOVEZEROES CALL REMOVELEADINGZEROES CALL [CHAR] . 1 INSERTCHAR CALL APPENDEXPONENT CALL PAD COUNT ;

: FS. FS.. CALL TYPE CALL ;

( display double float in engineers representation
  parameter:
      double float
  return:
      length of string
      address of string
)

: FE.. STRINGIFY CALL REMOVEZEROES CALL REMOVELEADINGZEROES CALL DUP S>D 3 FM/MOD CALL DROP
       A:R@ - [CHAR] . R> 1+ INSERTCHAR CALL APPENDEXPONENT CALL PAD COUNT ;

: FE. FE.. CALL TYPE CALL ;

( set number of significant digits
  parameter:
      number of digits
  return:
)

: SET-PRECISION PRECISION 64 MIN 3 MAX ! ;

:LOCAL APPENDDIGIT PAD H@ 1+ A:R@ 1+ PAD + C! R@ A:DROP PAD H! ;

( remove trailing zeroes
  parameter:
  return:
)

:LOCAL REMOVEZEROES A:PAD
                    R@ COUNT
                    BEGIN OVER OVER + 1- C@ [CHAR] 0 = OVER 1 > AND WHILE 1- REPEAT
                    R@ H! DROP
                    A:DROP ;
                 

( remove leading zeroes
  parameter:
      exponent
  return:
      adjusted exponent
)

:LOCAL REMOVELEADINGZEROES A:PAD A:R>
                           R1@ COUNT
                           BEGIN OVER C@ [CHAR] 0 = OVER 1 > AND WHILE A:1- 1- A:R@ OVER 1+ DUP 1- R> MOVE REPEAT
                           R1@ H! DROP
                           R@ A:2DROP ;

( insert character at distinct position
  parameter:
      position
      character
  return:
)

:LOCAL INSERTCHAR A:PAD A:COUNT
                  R1@ C@ [CHAR] - = - R1@ + ( start address with respect to sign )
                  A:OVER A:OVER A:+ ( end address )
 			A:R@ DROP ( starting index )
                  R1@ R@ A:DROP
                  ?DO
                      I ( copy address )
                      A:DUP A:C@ ( save content )
                      C! ( replace )
                      R@ A:DROP
                  LOOP
                  R@ C! A:DROP
                  A:1+ A:OVER A:1- A:1- A:H! A:DROP ;

( append a signed exponent as ASCII-string to scratch
  parameter:
      number
  return:
)

:LOCAL APPENDEXPONENT [CHAR] E APPENDDIGIT CALL
                      0<! IF NEGATE [CHAR] - ELSE [CHAR] + ENDIF APPENDDIGIT CALL
                      RECSPLIT CALL ;

( append unsigned integer as ASCII-string
  parameter:
      number:
  return:
)

:LOCAL RECSPLIT BASE @ /MOD 0<>! IF RECSPLIT CALL ELSE DROP ENDIF STOREDIGIT CALL ;

( store digit as ASCII in scratch
  parameter:
      number
  return:
)

:LOCAL STOREDIGIT 9 >! NIP IF BUILD> [CHAR] a 10 - DOES> ELSE [CHAR] 0 ENDIF + APPENDDIGIT CALL ;

( extract leading digit from double float
  parameter:
      power of leading digit
      double float
  return:
      leading digit
      power of leading digit
      double float - leading digit
)

:LOCAL LEADINGDIGIT A:R@ A:R1@ F/ CALL
                    OVER OVER stripf CALL
                    OVER OVER F>D CALL NIP >R
                    F- CALL
                    R1@ A:NIP R1@ A:R1@ F* CALL
                    R> R1@ R@ A:2DROP ;

(
  parameter:
      double float
  return:
      exponent
)

:LOCAL STRINGIFY 0 PAD H! ( initialize scratch string )
                 OVER 0< IF [CHAR] - APPENDDIGIT CALL FNEGATE CALL ENDIF ( store sign )
                 OVER 7ff00000H AND
                 IF OVER OVER FLN CALL ( ln(x) ) ELSE 0 DUP ENDIF
                 BASE @ S>D D>F CALL A:R1@ A:R@ ( double float of base )
                 FLN CALL F/ CALL ( ln(x) / ln(base) )
                 F>D CALL NIP ( convert exponent to integer )
                 R1@ A:R1@ NIP R1@ R@
                 PRECISION @ - INTPOWER CALL F+ CALL ( round )
                 R@ A:DROP R1@ A:R1@ NIP R1@ 
                 R@ INTPOWER CALL ( base ** floor(exponent) )
                 LEADINGDIGIT CALL
                 0=! IF A:1- ( adjust exponent ) ELSE DUP STOREDIGIT CALL ENDIF
                 R1@ A:NIP R1@ A:NIP A:R@ A:R1@ 2DROP ( swap base, exponent )
                 0<> PRECISION @ + ( remaining number of digits )
                 R1@ A:R1@ NIP R1@ R@ A:DROP A:2DROP 1+
                 0 ?DO
                       A:R1@ A:R@ F/ CALL ( minor power of leading digit )
                       LEADINGDIGIT CALL STOREDIGIT CALL
                       R1@ R@ A:2DROP
                   LOOP
                 2DROP 2DROP 2DROP
                 R@ A:DROP
                 A:BASE A:@
                 ( round mantissa )
                 PAD COUNT
					  1- A:R@ A:R1@
					  OVER OVER OVER 1- 1- H! DROP
					  +
					  A:C@ A:[CHAR] - A:= A:+
                 DUP C@ [CHAR] A <! NIP IF [CHAR] 0 ELSE BUILD> [CHAR] A 10 - DOES> ENDIF -
					  2* R1@ < INVERT ( last digit )
                 BEGIN
                       DUP R@ 0<> AND
                 WHILE
                       DROP 1- DUP C@
							  [CHAR] A <! NIP IF [CHAR] 0 ELSE BUILD> [CHAR] A 10 - DOES> ENDIF -
							  1+ ( round next digit )
							  
                       DUP R1@ - 0<!
                       IF DROP A:0 ELSE NIP A:-1 ENDIF
							  10 <! NIP IF [CHAR] 0 ELSE BUILD> [CHAR] A 10 - DOES> ENDIF +
                       OVER C!
							  
                       R>
							  A:1-
                 REPEAT
                 A:2DROP NIP
					  IF
                    [CHAR] 1 0 INSERTCHAR CALL
                    1+ ( adjust exponent )
                 ENDIF
                 ;
					  
					  
