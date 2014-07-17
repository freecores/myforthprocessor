( 
  BIOS for FORTH-System

  Copyright © 2004-2008 by Gerhard Hohner
)
  


DEFINE MODULPROCESSED 20

( set a distinct interrupt vector
  parameter:
      vector number
      address of service routine
  return:
)

: SETVECTOR 15 AND CELLS ! ;

( get a distinct interrupt vector
  parameter:
      vector number
  return:
      address of service routine
)

: GETVECTOR 15 AND CELLS @ ;

( reset return stack
  parameter:
  return:
)

: QUIT CLEARRET CALL ;

( reset both stacks
  parameter:
  return:
)

: ABORT LINK C@
        IF
           15 OUTQUEUEAPPENDRAWCHAR CALL MODULPROCESSED OUTQUEUEAPPENDMASKEDCHAR CALL
           0 SENDWORD CALL
           25 OUTQUEUEAPPENDRAWCHAR CALL
           HERE' @ HERE !
        ENDIF
        CLEARDATA CALL ;

( force software interrupt
  parameter:
  return:
)

: SOFTINT 0fffffffcH DUP ! ;

( service routine for illegal instruction
  parameter:
  return:
)

:LOCAL ILLEGAL B@ DI S" undefined instruction located near  " TYPE CALL BASE @ 16 BASE ! R@ U. CALL BASE ! QI EI
               1000000 0 DO LOOP B! ;


