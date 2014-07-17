( 
  BIOS for FORTH-System

  Copyright © 2004-2008 by Gerhard Hohner
)

( allocate some variables
  parameter:
      number of entries
  return:
      0, if full, address of variable otherwise
)

: VALLOCATE A:DI CELLS 0<!
            IF
               DROP SLENGTH @
               DUP CELL+ BUILD> SSTACKSIZE 1- DOES> AND SLENGTH ! ( new length )
               SSTACK OVER + ( return address )
               0 OVER ! ( clear entry )
            ELSE
               VLENGTH @
               OVER OVER + VSTACKSIZE MIN VLENGTH ! ( new length )
               VSTACK OVER + ( return address )
               NIP OVER 0
               ?DO 0 OVER I CELLS + ! LOOP ( clear entries )
            ENDIF
            NIP
            A:EI ;

( set top of stack
  parameter:
      new address of stack top
  return:
)

: SETVTOP A:DI
          VSTACK - 8 MAX
          VSTACKSIZE MIN
          VLENGTH !
          A:EI ;

( set variable
  parameter:
      address of variable
      handle
  return:
)

: V! A:DI
     0=! IF DROP VSTACK ENDIF                ( store a return value )
     OVER 0<> VBARRIER @ 0<> AND 
     IF
        OVER DUP @ WHITE =
        IF GRAY OVER ! ENDIF DROP            ( strong invariant )
     ENDIF
     !
     A:EI ;

( mark and sweep garbage collector
  parameter:
  return:
)

:LOCAL MARK&SWEEP A:DI
                  0 MACCU ! ( clear accu of memory pool )
                  -1 VBARRIER ! ( mark and sweep active now! )
                  VLENGTH @
                  1 EI
                  ( mark all references in stack gray )
                  VSTACK + SSTACK
                  BEGIN
                     U>!
                  WHILE
                     DUP @ 0<>!
                     OVER @ WHITE = AND
                     IF A:DI GRAY OVER ! A:EI ENDIF
                     DROP CELL+
                  REPEAT
                  2DROP
                  ( process all grayed handles in use )
                  BEGIN
                     0
                     GETHANDLEARRAY CALL A:R@ A:R1@ A:@ 2DROP R1@ R@ A:2DROP 0
                     DO
                        DUP @ GRAY =
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
                                      OVER 0 ?DO DUP @ 0<>! OVER @ WHITE = AND IF A:DI GRAY OVER ! A:EI ENDIF DROP CELL+ LOOP ( gray all referenced objects )
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
                  DO DUP @ STATIC <>! NIP OVER 0<> AND NIP IF WHITE OVER ! ENDIF CELL+ CELL+ LOOP
                  2DROP
                  A:EI ;
