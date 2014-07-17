( 
  BIOS for FORTH-System

  Copyright © 2004-2008 by Gerhard Hohner
)
  


( send next character, if available
  parameter:
  return:
)

:LOCAL SENDNEXTCHAR DI
                    A:OUTQUEUEOFFSET A:OUTQUEUELENGTH
						  R1@ @ R@ @
                    BEGIN
                         24 SIOSTATUS @ ( UART status )
                         OVER AND = ( transmiter ready, transmitter buffer not full )
                         OVER 0> AND ( queue empty? )
                    WHILE
                         2 - ( update length )
                         OVER OUTQUEUEBASE + ( address of next character )
                         H@ ( load it ) 
                         SIODATA ! ( send it )
								 SWAP HALF+ 1023 AND SWAP
                    REPEAT
						  R@ ! ( new length )
						  R1@ ! ( new offset )
                    A:2DROP EI ;

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
                             24 SIOSTATUS @ OVER AND = OUTQUEUELENGTH @ 0= AND
									  IF
									     SIODATA !
									  ELSE
                                A:DI
                                A:OUTQUEUELENGTH A:DUP A:@ ( lenght of queue )
                                OUTQUEUEOFFSET @ R@ + 1023 AND ( entry index )
                                OUTQUEUEBASE + ( address of position )
                                H! ( store it )
                                A:HALF+ A:OVER A:! A:DROP ( update length )
                                A:EI
									  ENDIF ;

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

:LOCAL OUTQUEUEAPPENDARRAY 0 ?DO DUP C@ OUTQUEUEAPPENDMASKEDCHAR CHAR+ LOOP DROP ;

( print string on console
  parameter:
      count
      address of string body
  return:
)

: TYPE 1 LOCKI/O CALL 0 OUTQUEUEAPPENDRAWCHAR CALL OUTQUEUEAPPENDARRAY CALL 26 OUTQUEUEAPPENDRAWCHAR CALL -1 LOCKI/O CALL ;

( print single character on console
  parameter:
      character
  return:
)

: EMIT 1 LOCKI/O CALL 0 OUTQUEUEAPPENDRAWCHAR CALL OUTQUEUEAPPENDMASKEDCHAR CALL 26 OUTQUEUEAPPENDRAWCHAR CALL -1 LOCKI/O CALL ;

( print a number of blank on console
  parameter:
      count
  return:
)

: SPACES 1 LOCKI/O CALL 0 OUTQUEUEAPPENDRAWCHAR CALL 0 ?DO BL OUTQUEUEAPPENDMASKEDCHAR CALL LOOP 26 OUTQUEUEAPPENDRAWCHAR CALL -1 LOCKI/O CALL ;

( print blank on console
  parameter:
  return:
)

: SPACE 1 SPACES CALL ;

( print new line on console
  parameter:
  return:
)

: CR 1 LOCKI/O CALL 0 OUTQUEUEAPPENDRAWCHAR CALL 13 OUTQUEUEAPPENDMASKEDCHAR CALL 10 OUTQUEUEAPPENDMASKEDCHAR CALL 26 OUTQUEUEAPPENDRAWCHAR CALL -1 LOCKI/O CALL ;

( print unicode string on console
  parameter:
      count
      address of string body
  return:
)

: PRINT 1 LOCKI/O CALL
        0 CHANGECHARSET CALL ( unicode )
        TYPE CALL
        -1 CHANGECHARSET CALL ( ASCII )
        -1 LOCKI/O CALL ;

                             