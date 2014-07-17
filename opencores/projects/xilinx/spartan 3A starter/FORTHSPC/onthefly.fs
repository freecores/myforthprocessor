( 
  BIOS for FORTH-System

  Copyright © 2004-2008 by Gerhard Hohner
)

( store a single variable
  parameter:
      address of variable
  return:
)

: SALLOCATE A:DI 0<>!
            IF
               SLENGTH @ A:R@ CELL+ ( new length )
					SSTACK @ 
					0=! IF DROP 1000 MALLOC CALL DROP GLOBAL OVER ! DUP SSTACK ! ENDIF
					A:R@ ALLOCATED-SIZE CALL >! NIP
					IF R> OVER 1000 + RESIZE CALL DROP A:R@ SSTACK ! ENDIF ( resize scope )
					SLENGTH !
					R1@ R@ A:2DROP CELL+ @ + !   ( store address )
					DUP
				ENDIF
				DROP A:EI ;
				
( allocate some variables
  parameter:
      number of entries
  return:
      index of first entry in scope
)

: VALLOCATE A:DI CELLS
            VLENGTH @ 4 MAX A:R@ + ( new length )
				VSTACK @
				0=! IF DROP 1000 MALLOC CALL DROP SCOPE OVER ! DUP VSTACK ! ENDIF
				A:R@ ALLOCATED-SIZE CALL >! NIP
				IF R> OVER 1000 + RESIZE CALL DROP A:R@ VSTACK ! ENDIF ( resize scope )
				VLENGTH !
				R1@ A:2DROP
            A:EI ;
		  
( set top of stack
  parameter:
      index
  return:
)

: SETVTOP A:DI
			 A:VSTACK A:@ A:CELL+ A:@
			 VLENGTH @ OVER
			 BEGIN >! WHILE CELL+ 0 OVER R@ + ! REPEAT
			 A:DROP 2DROP VLENGTH !
          A:EI ;

( set variable
  parameter:
      index of variable
		handle
  return:
)

: V! A:DI
	  OVER 0<> VBARRIER @ AND IF OVER DUP @ WHITE = IF GRAY OVER ! ENDIF DROP ENDIF ( strong invariant )
     VSTACK
     @ CELL+ @ + !
     A:EI ;

( mark and sweep garbage collector
  parameter:
  return:
)

:      MARK&SWEEP A:DI
                  0 MACCU ! ( clear accu of memory pool )
                  -1 VBARRIER ! ( mark and sweep active now! )
                  1 EI
                  ( process all grayed handles in use )
                  BEGIN
                     0
                     GETHANDLEARRAY CALL A:R@ A:R1@ A:@ 2DROP R1@ R@ A:2DROP 0
                     DO
							   DUP @ A:R@ 2/ 2* SCOPE =
								IF
								   R@ GRAY OR OVER ! ( gray scope )
									>R 1+
									R@ CELL+ @
									R1@
									R@ ALLOCATED-SIZE CALL 2/ 2/ 0
									?DO
									    OVER I CELLS + @ OVER 1 AND 
										 IF 0<>! OVER @ AND NIP ENDIF 
										 0<>! OVER @ WHITE = AND
										 IF A:DI GRAY OVER ! A:EI ENDIF
										 DROP
									LOOP
									2DROP R>
								ENDIF
                        R> GRAY =
                        IF
                           A:R> 1+ 
                           R@ CELL+ @
                           DUP FATHER + @ 0<>! OVER @ WHITE = AND IF  A:DI GRAY OVER ! A:EI ENDIF DROP ( mark father )
                           DUP SON + @ 0<>! OVER @ WHITE = AND IF  A:DI GRAY OVER ! A:EI ENDIF DROP ( mark son )
                           DUP OUTER + @ 0<>! OVER @ WHITE = AND IF  A:DI GRAY OVER ! A:EI ENDIF DROP ( mark outer object )
                           A:R@ A:CODED A:+ A:@ ( code of object )
                           A:DUP A:65535 A:AND
                           A:IF ( array )
                                DUP BUILD> AREA 4 + DOES> + @ ( length of array )
                                OVER BUILD> AREA 12 + DOES> + @ ( array address )
                                0<>! OVER @ BLACK <> AND
                                IF
                                   A:DI BLACK OVER ! A:EI ( mark array handle )
                                   R@ 65535 AND 1- 0>
                                   R@ 16 RSHIFT 6 > OR
                                   IF
                                      CELL+ @ ( data pointer )
                                      OVER 0
												  ?DO DUP @ 0<>! OVER @ WHITE = AND IF A:DI GRAY OVER ! A:EI ENDIF DROP CELL+ LOOP ( gray all referenced objects )
                                   ENDIF
                                ENDIF
                                2DROP
                           ELSE ( object )
                                REFCOUNT + ( count of referenced objects )
                                DUP @ 0
                                ?DO CELL+ DUP @ 0<>! OVER @ WHITE = AND IF A:DI GRAY OVER ! A:EI ENDIF DROP LOOP ( gray all referenced objects )
                           ENDIF
                           DROP
                           R1@ A:2DROP
                           A:DI BLACK OVER ! A:EI ( mark object black )
                        ENDIF
                        CELL+ CELL+
                     LOOP
                     DROP
                     0=
                  UNTIL
                  ( sweep white handles )
                  GETHANDLEARRAY CALL OVER @ 0
                  DO
                     DUP @ WHITE =
                     IF DUP FREE CALL DROP ENDIF
                     CELL+ CELL+
                  LOOP
                  2DROP
                  ( set all handles white )
                  DI DROP
                  0 VBARRIER ! ( mark and sweep inactive )
                  GETHANDLEARRAY CALL OVER @ 0
                  DO
						  DUP @
						  SCOPE OVER AND
						  IF
						     BUILD> GRAY INVERT DOES> AND OVER !
						  ELSE
						     BUILD> BLACK GRAY OR DOES> AND IF WHITE OVER ! ENDIF
						  ENDIF
						  CELL+ CELL+
						LOOP
                  2DROP
                  A:EI ;
