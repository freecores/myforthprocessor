( 
  BIOS for FORTH-System

  Copyright © 2004-2008 by Gerhard Hohner
)
  

( allocate buffer for ASCII-String
  parameter:
  return:
)

: <# PAD 256 + HLD ! ;

( insert stacked character in ASCII-String
  parameter:
      character
  return:
)

: HOLD HLD @ 1- DUP HLD ! C! ;

( insert sign in ASCII-String
  parameter:
      integer
  return:
)

: SIGN 0< IF [CHAR] - HOLD CALL ENDIF ;

( split last digit from number, make it displayable and store it in ASCII-String
  parameter:
      double integer
  return
      double integer / BASE
)

: # BASE @ UD/MOD CALL ROT ( split last digit )
    10 <! IF DROP [CHAR] 0 ELSE - [CHAR] A ENDIF + ( make it displayable )
    HOLD CALL ;

( store the number displayable, completely in ASCII-String
  parameter:
      double integer
  return:
      double 0
)

: #S BEGIN # CALL OVER OVER D0= UNTIL ;

( fetch displayable ASCII-String
  parameter:
      double integer [dummy]
  return:
      count
      string address
)

: #> 2DROP HLD @ PAD 256 + OVER - ;

( make an unsigned number a right aligned ASCII-String
  parameter:
      integer number defining sign
      fieldwidth
      unsigned double integer number
  return:
)

:LOCAL S.R A:R@ A:R1@ 2DROP <# CALL #S CALL R1@ SIGN CALL #> CALL NIP R@ A:2DROP 
           - NEGATE 0 ?DO BL HOLD CALL LOOP ;

( make an unsigned number a right aligned ASCII-String
  parameter:
      fieldwidth
      unsigned number
  return:
)

: U..R A:R@ A:R1@ 2DROP 0 R@ R1@ A:2DROP UD..R CALL ;

: U.R U..R TYPE CALL ;

( make an unsigned number a displayable ASCII-String
  parameter:
      unsigned number
  return:
)

: U. 0 U.R CALL ;

( make an signed number a right aligned ASCII-String
  parameter:
      fieldwidth
      integer
  return:
)

: ..R A:R@ A:R1@ DROP DUP - R@ ABS R1@ R@ A:2DROP S.R CALL 0 DUP #> CALL ;

: .R ..R CALL TYPE CALL ;

( make a signed number to an ASCII-String
  parameter:
      integer
  return:
)

: . 0 .R CALL ;

( make a signed double to an ASCII-String
  parameter:
      signed double
  return:
)

: D. 0 D.R CALL ;

( make an signed double a right aligned ASCII-String
  parameter:
      fieldwidth
      double integer
  return:
)

: D..R A:R@ DROP A:R1@ DABS R1@ R@ A:2DROP S.R CALL 0 DUP #> CALL ;

: D.R D..R CALL TYPE CALL ;

( make an unsigned number a right aligned ASCII-String
  parameter:
      fieldwidth
      unsigned double
  return:
)

: UD..R 0 S.R CALL 0 DUP #> CALL ;

