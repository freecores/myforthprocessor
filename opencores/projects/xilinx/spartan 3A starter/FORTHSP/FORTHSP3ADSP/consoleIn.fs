( 
  BIOS for FORTH-System

  Copyright © 2004-2008 by Gerhard Hohner
)
  

( input buffer check
  parameter:
  return:
      0, if empty
)

: KEY? 16 OUTQUEUEAPPENDRAWCHAR CALL 1 -4 ! #tib @ 1024 OVER - DUP OUTQUEUEAPPENDMASKEDCHAR CALL 2 -4 ! 8 RSHIFT OUTQUEUEAPPENDMASKEDCHAR CALL  4 -4 !
       0 OUTQUEUEAPPENDMASKEDCHAR CALL 8 -4 ! 26 OUTQUEUEAPPENDRAWCHAR CALL 16 -4 ! ;

( read a single character without echo
  parameter:
  return:
      character
)

: KEY #tib @ 0=
      IF ( request input )
         16 OUTQUEUEAPPENDRAWCHAR CALL 1024 DUP OUTQUEUEAPPENDMASKEDCHAR CALL 8 RSHIFT OUTQUEUEAPPENDMASKEDCHAR CALL 
         -1 OUTQUEUEAPPENDMASKEDCHAR CALL 26 OUTQUEUEAPPENDRAWCHAR CALL 
         DI 1 EI BEGIN #tib @ UNTIL EI ( wait, till character available )
      ENDIF
      DI
      tib @ >in @ + C@ ( get character )
      #tib @ 1 - 0 +B #tib ! ( update length )
      >in @ 1+ 1023 AND >in ! ( update offset )
      SWAP EI ;

( parse next word from the input
  parameter:
      delimiter
  return:
      counted string
)

: WORD DUP BEGIN =! WHILE DROP KEY CALL REPEAT ( skip delimiters )
       A:HERE A:@ A:ALIGNED A:DUP A:HALF+
       BEGIN <>! WHILE R@ C! A:CHAR+ KEY CALL REPEAT
       2DROP A:OVER A:- A:1- A:1- A:OVER A:H! ( save length )
       R> ;

( parse next token from the input
  parameter:
      delimiter
  return:
      length
      address
)

: PARSE WORD CALL COUNT ;

( restore input buffer into a previous state
  parameter:
      1
      saved >IN
      saved #TIB
  return:
      true
)

: RESTORE-INPUT DROP A:DI ( lock interrupt )
                >in ! #tib !
                A:EI ( enable interrupt ) ;


( save current state of input buffer
  parameter:
  return:
      1
      >IN
      #TIB
)

: SAVE-INPUT A:DI #tib @ >in @ 1 A:EI ;

( reset input buffer
  parameter:
  return:
)

: QUERY 0 DUP DUP 1+ RESTORE-INPUT CALL  ;

( refill input buffer
  parameter:
  return:
      true, if characters entered buffer, false otherwise
)

: REFILL QUERY CALL KEY? CALL ;

( read a line with echo from console
  parameter:
      maximum length
      buffer address
  return:
      length
)

: ACCEPT A:R> A:0 ( save maximum length )
         A:BEGIN
               KEY CALL ( get character )
               DUP A:R@ EMIT CALL ( echo it )
               OVER C! ( store it )
               1+ ( increment address )
               A:10 A:= A:IF A:NIP A:DUP ELSE A:1+ ENDIF
               A:>! A:INVERT
         UNTIL
         DROP R1@ A:2DROP ; ( maximum length )

( read a line with echo from console, and store length in SPAN
  parameter:
      maximum length
      buffer address
  return:
      length
)

: EXPECT ACCEPT CALL SPAN ! ;

