( 
  BIOS for FORTH-System

  Copyright © 2004-2008 by Gerhard Hohner
)
  

( interface for internal counters )

( counter triggered by system clock
  parameter:
  return:
      mode word
)

: TRIGGER-SYSCLOCK 6 ;

( counter triggered by prescaler
  parameter:
      number of prescaler
  return:
      mode word
)

: TRIGGER-PRESCALER 3 AND ;

( counter triggered by input pin
  parameter:
  return:
      mode word
)

: TRIGGER-INPUT 4 ;

( lock counter
  parameter:
  return:
      mode word
)

: LOCK-COUNTER 7 ;

( set service routine for distinct counter
  parameter:
      counter number
      service routine
  return:
)

: SET-COUNTER-SERVICE 3 AND 10 + OVER OVER SETVECTOR 1 OVER LSHIFT GETIMASK OR SETIMASK 2DROP ;

( initialize and start a counter
  parameter:
      reload value
      mode
      counter number
  return:
)

: SETCOUNTER 2* 2* 2* OR OVER CELLS COUNTERPORT + ! DROP ;

( read out a distinct counter
  parameter:
      counter number
  return:
      count
)

: READCOUNTER CELLS COUNTERPORT + @ ;

( system timer service, counts milliseconds
  parameter:
  return:
)

:LOCAL TIMER3 DI A:MILLISECONDS A:VAL R@ 2@ 0 1 D+ R> 2!
              QI EI
              BUILD> MMBASE CELL+ DOES> @
              32 U<! NIP IF 1 OVER LSHIFT NIP ENDIF 2 RSHIFT
              MACCU @ U< VBARRIER @ 0= AND ( a quarter or more of storage was allocated and garbage collector is not running )
              IF
                MARK&SWEEP CALL
              ENDIF ( start garbage collector )
              ;

