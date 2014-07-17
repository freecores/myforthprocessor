( 
  BIOS for FORTH-System

  Copyright © 2004-2008 by Gerhard Hohner
)
  

(
 holds all routines for retrieving of the dictionary
)
 
( put a new entry on dictionary
  parameter:
      address of counted name
      type
      datafield
  return:
)

: create A:DICTTOP ( root top of dictinary )
         A:DUP A:@ ( pointer )
         A:3 A:CELLS A:- A:DUP ( update pointer and copy )
         R@ ! ( store counted name )
         A:CELL+ R@ ! ( store type )
         A:CELL+ R@ A:DROP ! ( store datafield )
         A:OVER A:! ( store new root )
         A:DROP ;

( compare two strings
  parameter:
      counted name1
      counted name2
  return:
      0, if equals
      < 0, if string1 < string2
      > 0, if string1 > string2
)

:LOCAL COMPARESTRING DUP COUNT NIP HALF+ 0 ( index )
                     DO OVER C@ OVER C@ - 0<>! IF NIP NIP UNLOOP EXIT ENDIF DROP CHAR+ A:R@ DROP CHAR+ R@ A:DROP LOOP
                     2DROP FALSE ; ( match )

( get address of a distinct entry
  parameter:
      type
      counted name
  return:
      address of entry, if found, 0 otherwise
)

:LOCAL FIND_ DICTBASE ( base of dictionary )
             DICTTOP @ ( top of dictinary )
             ?DO
                 A:R@ ( type )
                 R1@ ( copy dictionary pointer )
                 CELL+ @ 0ffH AND ( only, if type defined )
                 R@ =! NIP 
                 R> 0=
                 OR AND
                 IF
                    OVER ( duplicate name )
                    I @ ( load address of entries name string )
                    COMPARESTRING CALL 0=       
                    IF
                       2DROP ( drop name completely )
                       I ( address of matching entry )
                       UNLOOP
                       EXIT
                    ENDIF
                 ENDIF
                 3 CELLS ( move pointer to next entry )
             +LOOP
             DROP DUP - ; ( not found )

( find address of a distinct entry
  parameter:
      counted name
  return:
      0, if not found, TRUE otherwise
      counted name, if not found, token address otherwise
)

: FIND DUP 0 FIND_ CALL 0<>! IF NIP 0<>! ENDIF ;

( forget a distinct entry
  parameter:
      counted name
  return:
)

: forget FIND CALL IF 0 OVER CELL+ ! ENDIF DROP ( token undefined )
         BUILD> DICTBASE CELL+ DOES> DICTBASE DICTTOP @ CELL+ ( top of dictinary )
         ?DO I @ IF DROP I UNLOOP LEAVE ENDIF 3 CELLS +LOOP ( shrink to the bottom )
         1 CELLS - DICTTOP ! ;

( execute a token
  parameter:
      flag
      address of token
  return:
      data field, only if type is variable, constant or value, nothing is returned otherwise
)

: EXECUTE 0<> ( test flag )
          OVER CELL+ A:R@ @ 0ffH AND ( load type )
          AND ( mask type )
          A:CELL+ A:@ ( load data field )
          CASE
          0 OF DROP A:DROP EXIT ENDOF ( undefined )
          3 OF DROP EXIT ENDOF ( procedure long jump )
          5 OF A:MODULETOP A:! CELL+ CELL+ CELL+ DICTTOP ! EXIT ENDOF ( free module )
          6 OF A:HERE A:! DROP EXIT ENDOF ( marker )
          ENDCASE
          DROP R@ A:DROP ( all other )
          ;

( unload a module
  parameter:
  return:
)

:LOCAL UNLOAD -1 READ-BUFFER CALL NIP
              0= A:R@ IF DUP 5 FIND_ CALL 0<>! A:R@ A:AND EXECUTE CALL ENDIF DROP CR CALL
              A:IF " OK> " ELSE " FAILED> " ENDIF CR CALL COUNT TYPE CALL ;
