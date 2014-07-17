( 
  BIOS for FORTH-System

  Copyright © 2004-2008 by Gerhard Hohner
)
  


( interrupt routines for UART )

( serial transmiter service routine
  parameter:
  return:
)

:LOCAL SERIALOUT B@ ( save carry )
                 SENDNEXTCHAR CALL
                 B! QI ; ( try to send, restore carry, quit interrupt and exit )

( read next character and buffer it
  parameter:
  return:
)

:LOCAL SERIALIN B@
                BEGIN
                   SIOSTATUS @ 18 AND 18 = ( FiFo empty ? )
                WHILE
                   SIODATA @ ( read it )
                   DUP 256 <
                   DUP 1+ A:RECERROR R@ @ + R> ! ( update error count )
                   STUFFED C@ 0= AND 0 STUFFED C!
                   IF ( received valid and not stuffed )
                      DUP A:256
                      CASE
                      255 OF QI RESTART CALL ENDOF ( restart immediately )
                      254 OF LASTRECEIVED ! R@ ENDOF ( load and execute )
                      253 OF LASTRECEIVED ! R@ ENDOF ( read dictionary )
                      252 OF LASTRECEIVED ! R@ ENDOF ( available capacity of input buffer )
                      251 OF QI ABORT VAL A:EXIT ENDOF ( execute an abort )
			    250 OF LASTRECEIVED ! R@ ENDOF ( redefine SIO )
                      249 OF LASTRECEIVED ! R@ ENDOF ( unused memory )
                      248 OF LASTRECEIVED ! R@ ENDOF ( allocate dynamic memory )
                      247 OF LASTRECEIVED ! R@ ENDOF ( free dynamic memory )
                      246 OF LASTRECEIVED ! R@ ENDOF ( reset dynamic memory )
                      245 OF LASTRECEIVED ! R@ ENDOF ( unload module )
                      244 OF LASTRECEIVED ! R@ ENDOF ( late link )
                      243 OF LASTRECEIVED ! R@ ENDOF ( load and execute supporting late link )
                      27  OF STUFFED C! R@ ENDOF ( stuffing )
                      26  OF SENTALL C! 0 CURRENTFILEID C! R@ ENDOF ( block sent successfully )
                      25  OF SENTALL C! 0 CURRENTFILEID C! R@ ENDOF ( block sent erroneous )
                      ELSE
                             DUP 16 < IF CURRENTFILEID C! R@ ENDIF ( new buffer selected )
                      ENDCASE
                      A:DROP
                   ENDIF
                   DUP 256 <
                   IF ( buffer it )
                      CURRENTFILEID C@ ( actual input file )
                      CELLS CELLS INPUTFILEBASE + A:R@ ( copy base of actual buffers )
                      CELL+ @ ( size of buffer )
                      A:DUP A:CELL+ A:CELL+ A:DUP A:CELL+ ( pointer to length )
                      R@ @ ( current length )
                      U>!
                      IF ( buffer does not exceed size )
                         DUP CHAR+ R@ A:DROP ! ( upadte length )
                         R@ @ ( offset )
                         + ( join offset and old length )
                         U>! INVERT IF - NEGATE DUP ENDIF NIP ( adjust offset into buffer )
                         R1@ @ ( pointer into buffer )
                         + C!
                         A:DUP DUP DUP DUP
                      ENDIF
                      A:2DROP A:DROP
                      2DROP
                   ENDIF
                   DROP
                REPEAT
                QI ( quit interrupt )
                B! ( restore carry ) ;

( set parameters of SIO
  parameter:
      baudrate [ baud ]
      wordlength [ 8 or 7 ]
      parity [ 0 = none, 1 = odd, 2 = even ]
      twice stopbits [ 4 = 2 bit, 3 = 1.5 bit, 2 = 1 bit ]
  return:
)

: SETSIO A:DI
         BEGIN SIOSTATUS @ 5 AND 0= UNTIL ( wait, till SIO not busy )
         A:R@ A:R1@ 2DROP ( move baudrate, wordlength )
         A:8 A:= A:10H A:AND ( define wordlength )
         A:0 ( parity none )
         CASE ( parity )
         2 OF A:HALF+ ENDOF ( even )
         1 OF A:1+ ENDOF ( odd )
         ENDCASE
         A:OR
         A:4
         CASE ( stopbits )
         4 OF A:DUP A:- ENDOF
         3 OF A:2* ENDOF
         ENDCASE
         A:OR
         A:SIODEFINE A:! ( set wordlength, parity, stopbits )
         SYSCLOCK R@ + R> 2* / CALL ( calculate scaler )
         SIOSCALER ! ( set scaler )
         A:EI ;


( get parameters of SIO
  parameter:
  return:
      baudrate [ baud ]
      wordlength [ 8 or 7 ]
      parity [ 0 = none, 1 = odd, 2 = even ]
      twice stopbits [ 4 = 2 bit, 3 = 1.5 bit, 2 = 1 bit ]
)

: GETSIO A:SIODEFINE A:@ ( read wordlength, parity, stopbits )
         R@ 12 AND
         CASE
         0 OF 4 ENDOF ( 2 stopbits )
         8 OF 3 ENDOF ( 1.5 stopbits )
              2       ( 1 stopbit )
         ENDCASE
         R@ 3 AND
         CASE
         2 OF 2 ENDOF ( even )
         1 OF 1 ENDOF ( odd )
              0       ( none )
         ENDCASE
         7 A:10H A:AND A:IF 1+ ENDIF ( wordlength )
         SYSCLOCK A:SIOSCALER A:@ R@ + R> 2* / CALL ( baudrate )
         ;

:LOCAL REDEFINESIO 7 READ-BUFFER CALL 2DROP
                   A:R@ C@ ( twice stopbits [ 4 = 2 bit, 3 = 1.5 bit, 2 = 1 bit ] )
                   A:CHAR+ R@ C@ ( parity [ 0 = none, 1 = odd, 2 = even ] )
			 A:CHAR+ R@ C@ ( wordlength [ 8 or 7 ] )
			 A:CHAR+ R@ C@
			 A:CHAR+ R@ C@ 8 LSHIFT OR
			 A:CHAR+ R@ C@ 16 LSHIFT OR
			 A:CHAR+ R> C@ 24 LSHIFT OR ( baudrate [ baud ] )
			 SETSIO CALL
			 ;
			 