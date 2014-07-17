( 
  BIOS for FORTH-System

  Copyright © 2004-2008 by Gerhard Hohner
)
  


( divide/modulo unsigned double integers
  parameter
      64-bit divisor
      64-bit dividend
  return
      64-bit unsigned quotient
      64-bit unsigned remainder
)

: UD/DMOD OVER OVER D0= IF DROP NIP NIP DUP DUP 1- DUP EXIT ENDIF
          A:0 BEGIN A:1+ A:R1@ A:R@ 2DROP OVER OVER R1@ R@ D- 2DROP R1@ R@ B@ INVERT R1@ A:2DROP 0< INVERT AND WHILE A:R> A:2* DUP +B R> REPEAT
          R> ( move count )
          A:0 A:DUP ( initialize quotient )
          BEGIN 0<>! WHILE 1- A:R@ DROP A:R1@ A:R@ D- B@ INVERT /2 0BRANCH ( dividend -= divisor )
                           /3 LABEL ( subtraction was possible, no carry produced )
                           R1@ 1 RSHIFT R@ 1 RSHIFTC A:2DROP ( shift divisor right )
                           R@ R1@ A:2DROP
                           2* A:DUP A:+B 1+ A:R@ DROP ( shift quotient left and set LSB )
          REPEAT
          DROP 2DROP ( drop count, divisor )
          /4 BRANCH
          BEGIN 0<>! WHILE 1- A:R@ A:R1@ 2DROP 1 RSHIFT R> 1 RSHIFTC ( shift divisor right )
                           A:R1@ A:R@ D+ B@ INVERT /3 0BRANCH ( dividend += divisor )
                           /2 LABEL ( additition produced a carry )
                           R1@ R@ A:2DROP ( move divisor )
                           R@ R1@ A:2DROP
                           2* A:DUP A:+B A:R@ DROP ( shift quotient left and clear LSB )
          REPEAT
          DROP D+ ( drop count, adjust remainder )
        
          /4 LABEL
          R1@ R@ A:2DROP ( move quotient )
         ;

( divide/modulo unsigned words
  parameter
      32-bit divisor
      64-bit dividend
  return
      64-bit unsigned quotient
      32-bit unsigned remainder
)

: UD/MOD A:R@ DUP - R> ( divisor now unsigned 64 bit )
         UD/DMOD CALL
         A:R1@ A:R@ 2DROP ( move quotient )
         NIP ( remainder now 32 bit )
         R1@ R@ A:2DROP ( move quotient )
         ;

( divide/modulo unsigned words
  parameter
      unsigned divisor
      unsigned double dividend
  return
      32-bit unsigned quotient
      32-bit unsigned remainder
)

: UM/MOD UD/MOD CALL NIP ;

( divide/modulo signed words
  parameter
      signed divisor
      signed double dividend
  return
      32-bit signed symetric quotient
      32-bit signed remainder
)

: SM/REM A:R> A:R1@ DABS R1@ ABS UM/MOD CALL
         A:XOR ( build sign of results )
         A:0< A:IF NEGATE A:R> NEGATE R> ENDIF ( make results signed ) ;

( floored signed division
  parameter
      divisor signed
      double dividend signed
  return
      signed quotient
      signed remainder
)

: FM/MOD A:R@ SM/REM CALL A:R> 0<! IF R1@ 0< IF NEGATE ENDIF R1@ + A:1- ENDIF R@ A:2DROP ;

( multiply signed integers
  parameter
      multiplikand
      multiplicator
  return
      64-bit product
)

: M* A:R1@ A:R@ 2DROP
     0 ( intial product )
     R@ ABS ( unsigned multiplicand )
     R1@ ABS ( unsigned multiplicator )
     A:XOR ( build sign )
     32 0 DO A:R1@ A:R@ 2/ DUP -B AND + 1 RSHIFTC R1@ R@ 1 RSHIFTC A:2DROP LOOP NIP 
     A:0< A:IF DNEGATE ENDIF ;

\ : M* A:R1@ A:R@ A:XOR ( build sign )
\     ABS SWAP
\     ABS
\     UM* CALL
\     A:0< A:IF DNEGATE ENDIF ;

\ :LOCAL UM* A:R1@ A:R@ 2DROP
\           R1@ 16 RSHIFT R@ 16 RSHIFT H* 0 ( ahigh * bhigh )
\           R1@ 16 RSHIFT R@ H* ( ahigh * blow )
\           R1@ R@ 16 RSHIFT H* ( alow * bhigh )
\           + 0
\           17 LSHIFTC OVER 16 RSHIFT OR SWAP 16 LSHIFT ( adjust sum )
\           0 R1@ R@ A:2DROP H* ( alow * blow )
\           D+ D+ ( sum all ) ;

( multiply unsigned words
  parameter
      multiplikand
      multiplicator
  return
      32-bit product
)

: * M* CALL NIP ;

( divide/modulo signed words
  parameter
      divisor
      dividend
  return
      32-bit signed quotient
      32-bit signed remainder
)

: /MOD A:R> S>D R> SM/REM CALL ;

( divide signed words
  parameter
      divisor
      dividend
  return
      32-bit signed quotient
)

: / /MOD CALL NIP ;

( remainder of signed words
  parameter
      divisor
      dividend
  return
      32-bit signed remainder
)

: MOD /MOD CALL DROP ;

( multiply and divide signed words
  parameter
      divisor
      multiplikator
      multiplikand
  return
      32-bit signed quotient
      32-bit signed remainder
)

: */MOD A:R@ DROP M* CALL R@ A:DROP SM/REM CALL ;

( multiply and divide signed words
  parameter
      divisor
      multiplikator
      multiplikand
  return
      32-bit signed quotient
)

: */ */MOD CALL NIP ;

( convert string to double integer
  parameter
      length of string
      address of string
      initial double value for result
  return
      remaining length of string
      address of first unconverted character
      double integer containing result
)

: >NUMBER OVER C@ [CHAR] - =! NIP
          IF A:-1 A:DUP ( negative sign ) ELSE A:0 ( positive sign ) A:R@ A:[CHAR] + A:= ENDIF
          DROP A:IF 1- A:R> 1+ R> ENDIF ( skip sign )
          A:R> ( move length )
          A:BEGIN
                A:0>! ( length > 0 )
          WHILE
                A:R@ C@ ( save address, load next character )
                [CHAR] 0 <! NIP OVER [CHAR] 9 > OR INVERT [CHAR] 0 AND -
                [CHAR] A <! NIP OVER [CHAR] Z > OR INVERT BUILD> [CHAR] A 10 - DOES> AND -
                [CHAR] a <! NIP OVER [CHAR] z > OR INVERT BUILD> [CHAR] a 10 - DOES> AND -
                BASE @ <! INVERT IF 2DROP R> LEAVE ENDIF ( end of number reached )
                A:R1@ ( move extracted digit )
                A:R@ 2DROP ( move value of BASE )
                ( multiply with BASE )
                A:R1@ A:R@ 2DROP 0 DUP R1@ R@ A:2DROP
                A:BEGIN A:0<>! WHILE A:2/ DUP DUP -B IF A:R1@ A:R@ D+ R1@ R@ A:2DROP ENDIF OVER OVER D+ REPEAT
                2DROP R@ R1@
                A:2DROP D+ ( add digit )
                R@ A:DROP
                1+ A:1- 
          REPEAT
          R1@ A:NIP A:R1@ NIP ( move sign, move address )
          IF DNEGATE ENDIF ( make signed )
          R@ R1@ A:2DROP ;

( convert string to double integer
  parameter
      counted string
      initial double value for result
  return
      address of first unconverted character
      double integer containing result
)

: CONVERT COUNT >NUMBER CALL DROP ;

( arithmetic right shift of double integer
  parameter
      integer shift
      double integer operand
  return
      double integer
)

: ASHIFT 0<!
         IF
            NEGATE SHIFTL CALL
         ELSE
            64 MIN
            A:R> A:R1@ A:0< R1@ A:NIP
            BEGIN 31 >! WHILE - 1- A:R@ 2DROP R1@ SWAP R> REPEAT DROP
            A:R@ RSHIFT OVER 32 R@ - LSHIFT OR
            SWAP R@ RSHIFT R1@ 32 R@ A:2DROP - LSHIFT OR SWAP
         ENDIF ;

( left shift of double integer
  parameter
      integer shift
      double integer operand
  return
      double integer
)

: SHIFTL 0<!
         IF
            NEGATE ASHIFT CALL
         ELSE
            64 MIN
            BEGIN 31 >! WHILE - 1- A:R> SWAP DUP - R> REPEAT DROP
            A:R@ A:R1@ LSHIFT SWAP R1@ LSHIFT
            R@ 32 R1@ - RSHIFT OR SWAP A:2DROP
         ENDIF ;

( right shift of double integer
  parameter
      integer shift
      double integer operand
  return
      double integer
)

: SHIFTR 0<!
         IF
            NEGATE SHIFTL CALL
         ELSE
            64 MIN
            BEGIN 31 >! WHILE - 1- A:R> DUP - SWAP R> REPEAT DROP
            A:R@ RSHIFT OVER 32 R@ - LSHIFT OR
            SWAP R> RSHIFT SWAP
         ENDIF ;

( multiplication of 2 double integers
  parameter
      double integer multiplicand
      double integer multiplicator
  return
      double integer product 
)

: D* A:R1@ DABS
     A:R1@ A:R@ 2DROP
     A:R1@ DABS
     R1@ A:NIP R1@ A:NIP A:R@ A:R1@ 2DROP
     A:R@ A:R1@ 2DROP
     0 DUP 64
     BEGIN 0<>!
     WHILE 1- A:R@ DROP ( decrement and move count )
           2* A:R@ DROP DUP +B R@ A:DROP ( shift product left )
           R1@ A:NIP R1@ R@ A:2DROP ( move multiplicand, count )
           A:2* R@ A:DROP A:DUP A:+B A:R@ DROP ( shift multiplicator left )
           A:R1@ A:R@ DROP A:R1@ A:SWAP ( copy multiplicand, move count )
           B@ IF D+ ELSE 2DROP ENDIF ( add multiplicand, or delete it )
           R@ A:DROP ( move count )
     REPEAT
     DROP A:2DROP A:2DROP ( delete count, multiplicand, multiplicator )
     A:XOR A:0< A:IF DNEGATE ENDIF ; ( make product signed )
\ : D* A:R1@ DABS
\     A:R@ A:R1@ 2DROP
\     A:R1@ DABS
\     R1@ A:NIP R1@ A:NIP
\     UD* CALL
\     A:XOR A:0< A:IF DNEGATE ENDIF ( make product signed )
\     A:R> NIP NIP R> ;

( unsigned 64-bit multiplication
  parameter:
      double multiplicand
      double multiplicator
  return:
      128-bit product
)

\ :LOCAL UD* A:R> A:R@ ( multiplicator reversed )
\           A:R1@ NIP A:R1@ ( multiplicand reversed )
\           UM* CALL ( ahigh * bhigh )
\           R> A:ROT R@ UM* CALL ( ahigh * blow )
\           R1@ A:ROT R> UM* CALL ( alow * bhigh )
\           D+ A:R> A:B@ A:NEGATE
\           + A:R>
\           R1@ A:NIP +B
\           R@ R1@ A:2DROP
\           R1@ R@ A:2DROP UM* CALL ( alow * blow )
\           A:R> + A:R>
\           0 +B A:R>
\           0 +B R>
\           R@ R1@ A:2DROP
\           ;

( divide double integer words
  parameter
      64-bit divisor
      64-bit dividend
  return
      64-bit signed quotient
)

: D/ A:R1@ DABS
     A:R@ A:R1@ 2DROP
     A:R1@ DABS
     R1@ A:NIP R1@ A:NIP A:XOR
     UD/DMOD CALL
     A:0< A:IF DNEGATE ENDIF
     A:R> NIP NIP R> ;

( modulo double integer words
  parameter
      64-bit divisor
      64-bit dividend
  return
      64-bit signed remainder
)

: DMOD A:R1@ DABS
       A:R@ A:R1@ 2DROP
       A:R1@ DABS
       R1@ A:NIP R1@ A:NIP A:XOR
       UD/DMOD CALL 2DROP
       A:0< A:IF DNEGATE ENDIF ;
