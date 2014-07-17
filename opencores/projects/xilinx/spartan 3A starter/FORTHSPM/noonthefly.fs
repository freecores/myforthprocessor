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
					0=! IF DROP 1000 MALLOC CALL DROP DUP SSTACK ! ENDIF
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
				0=! IF DROP 1000 MALLOC CALL DROP DUP VSTACK ! ENDIF
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
     VSTACK
     @ CELL+ @ + !
     A:EI ;
