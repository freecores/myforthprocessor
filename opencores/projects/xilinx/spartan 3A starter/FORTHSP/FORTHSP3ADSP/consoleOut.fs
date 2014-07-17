( 
  BIOS for FORTH-System

  Copyright © 2004-2008 by Gerhard Hohner
)
  


( send next character, if available
  parameter:
  return:
)

:LOCAL SENDNEXTCHAR A:DI
                    A:OUTQUEUELENGTH A:DUP A:@
                    BEGIN
                       24 SIOSTATUS @ ( UART status )
                       OVER AND = ( transmiter ready, transmitter buffer not full )
                       R@ 0<> AND ( queue empty? )
                    WHILE
                       A:1- A:1- ( update length )
                       OUTQUEUEOFFSET @ DUP HALF+ 1023 AND OUTQUEUEOFFSET ! ( update offset )
                       OUTQUEUEBASE + ( address of next character )
                       DUP C@ OVER CHAR+ C@ 8 LSHIFT OR NIP ( load it ) 
                       SIODATA ! ( send it ) SIOSTATUS @ -4 !
                    REPEAT
                    A:OVER A:! ( store length )
                    A:DROP A:EI 128 -4 ! ;

( output buffer check
  parameter:
  return:
      0, if full
)

: EMIT? OUTQUEUELENGTH @ 1024 < ;

( append a raw character to output buffer
  parameter:
      half word
  return:
)

:LOCAL OUTQUEUEAPPENDRAWCHAR BEGIN EMIT? CALL UNTIL ( wait, till buffer not full )
                             A:DI
                             A:OUTQUEUELENGTH A:DUP A:@ ( lenght of queue )
                             OUTQUEUEOFFSET @ R@ + 1023 AND ( entry index )
                             OUTQUEUEBASE + ( address of position )
                             OVER 8 RSHIFT OVER CHAR+ C! C! ( store it )
                             A:HALF+ A:OVER A:! A:DROP ( update length )
                             A:EI
                             SENDNEXTCHAR CALL 64 -4 ! ;

( append a, possibly to stuff, character to output buffer
  parameter:
      character
  return:
)

:LOCAL OUTQUEUEAPPENDMASKEDCHAR 0ffH AND DUP 27 > INVERT ( <= 27 )
                                IF ( stuff control character )
                                   256 OR ( append ESC )
                                ENDIF
                                OUTQUEUEAPPENDRAWCHAR CALL ( append character ) ;

( append a string to output buffer
  parameter:
      count
      address of string body
  return:
)

:LOCAL OUTQUEUEAPPENDARRAY 0 ?DO DUP C@ OUTQUEUEAPPENDMASKEDCHAR CHAR+ LOOP DROP ! ;

( print string on console
  parameter:
      count
      address of string body
  return:
)

: TYPE 0 OUTQUEUEAPPENDRAWCHAR CALL OUTQUEUEAPPENDARRAY CALL 26 OUTQUEUEAPPENDRAWCHAR CALL ;

( print single character on console
  parameter:
      character
  return:
)

: EMIT 0 OUTQUEUEAPPENDRAWCHAR CALL OUTQUEUEAPPENDMASKEDCHAR CALL 26 OUTQUEUEAPPENDRAWCHAR CALL ;

( print a number of blank on console
  parameter:
      count
  return:
)

: SPACES 0 OUTQUEUEAPPENDRAWCHAR CALL 0 ?DO BL OUTQUEUEAPPENDMASKEDCHAR CALL LOOP 26 OUTQUEUEAPPENDRAWCHAR CALL ;

( print blank on console
  parameter:
  return:
)

: SPACE 1 SPACES CALL ;

( print new line on console
  parameter:
  return:
)

: CR 0 OUTQUEUEAPPENDRAWCHAR CALL 13 OUTQUEUEAPPENDMASKEDCHAR CALL 10 OUTQUEUEAPPENDMASKEDCHAR CALL 26 OUTQUEUEAPPENDRAWCHAR CALL ;

( print unicode string on console
  parameter:
      count
      address of string body
  return:
)

: PRINT 0 CHANGECHARSET CALL ( unicode )
        TYPE CALL
        -1 CHANGECHARSET CALL ( ASCII ) ;

                             