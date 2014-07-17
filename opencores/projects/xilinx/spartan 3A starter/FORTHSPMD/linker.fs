( 
  BIOS for FORTH-System

  Copyright © 2004-2008 by Gerhard Hohner
)
  

DEFINE ERRORREC 24

( resolve references in a segment
  parameter:
      base offset of linkable segment
      base address
      first address after module
  return:
)

:LOCAL PASS A:R@ DROP ( move segment )
            A:R@ ( save base )
            DUP @ + CELL+ ( base of initial code )
            DUP @ + CELL+ ( base of references )
            BEGIN
                  >!
            WHILE
                  A:R@ A:CELL+ 
                  @ DUP 1+ 
                  IF ( calculate absolute addresses )
                     R1@ + ( absolute address )
                     R1@ ( base of module )
                     R@ CELL+ ( list of references )
                     R> @ ( number of references )
                     RESOLVEADDRESS CALL
                  ELSE ( unresolved label )
                     DROP R@ FIND CALL ( find token )
                     A:COUNT A:+ A:ALIGNED ( base of references )
                     IF DUP R1@ R> R1@ A:R1@ RESOVLELABEL CALL ENDIF DROP ( resolve references, if found )
                     R@ R> @ 1+ CELLS + ( base of next label )
                  ENDIF
            REPEAT
            2DROP A:2DROP ;

( calculate absolute addresses
  parameter:
      number of addresses to process
      base of address list
      base of module
      absolute address to insert
  return:
      next address after address list
)

:LOCAL RESOLVEADDRESS 0 ?DO A:R@ A:CELL+ ( save and increment list pointer )
                            @ ( load relative position of replace )
                            A:R1@ ( save base of module ) 
                            + ( absolute address of replace )
                            A:R1@ A:R@ ( save replacement, address of replacement )
                            C! ( replace first byte )
                            R1@ 8 RSHIFT A:1+ R@ C! ( replace second byte )
                            R1@ 16 RSHIFT A:1+ R@ C! ( replace third byte )
                            R1@ 24 RSHIFT A:1+ R@ C! ( replace fourth byte )
                            R1@ A:2DROP ( move replacement )
                            R@ ( base of module )
                            R1@ A:2DROP ( list pointer )
                        LOOP                            
                      NIP NIP ;

( resolve references for a token
  parameter:
      base offset of segment
      base of references
      base of module
      address of token
  return:
)

:LOCAL RESOVLELABEL A:R> ( move segment offset )
                    A:R@ A:@ ( reference count )
                    BEGIN
                          R@	A:1-
                    WHILE
                          CELL+
                          DUP @ DUP 1+ 0<> OVER 7fffffffH AND R1@ < INVERT AND ( reference unresolved and in selected segment )
                          IF
                             A:R1@ NIP ( move reference list pointer )
                             OVER + ( link reference )
                             A:R@ ( move flag )
                             2* 1 RSHIFT 1- ( adjust reference )
                             A:R@ A:C@ A:80H A:AND A:SWAP ( extract address bit )
                             A:0<
                             A:IF 
                                  13 3 PICK CELL+ @ 0ffH AND ( load type )
                                  CASE
                                  3 OF DROP 63 ENDOF ( procedure )
                                  4 OF A:HALF+ ENDOF ( value )
                                  ELSE
						   A:1+
                                  ENDCASE
                                  R@ 80H AND OR OVER C! ( store Opcode )
                             ENDIF
                             1+ ( increment reference )
                             2 PICK CELL+ CELL+ ( address of data field )
                             A:2/ B@ IF @ ENDIF ( content of data field for variable )
                             A:R@ A:R1@ 2DROP
                             R1@ R@ C! ( store first byte )
                             R1@ 8 RSHIFT A:1+ R@ C! ( store second byte )
                             R1@ 16 RSHIFT A:1+ R@ C! ( store third byte )
                             R1@ 24 RSHIFT A:1+ R@ C! ( store fourth byte )
                             A:NIP R1@ 1 AND IF R1@ 2* R@ 1+ C! ENDIF ( set @-command for VALUE fetch )
                             A:2DROP
                             -1 R@ ! ( reference resolved )
                             R> ( move pointer )
                             DUP
                          ENDIF
                          DROP
                    REPEAT
                    DROP 2DROP A:2DROP ;

( check some references, if resolved
  parameter:
      base of references
      base offset of resolved code
  return:
      0, if resolved
)

:LOCAL REFCHECK DUP @ 0 ?DO CELL+ A:R@ @ 1+ 7fffffffH AND >! INVERT NIP DUP IF NIP A:DROP UNLOOP EXIT ENDIF DROP R@ A:DROP LOOP 2DROP 0 ;

( check, if all references are resolved in initial code
  parameter:
      initial error count
      base offset for resolved code
      base address
      first address after module
  return:
      count of unresolved references
)

:LOCAL LINKCHECK A:R1@ ( move base offset )
                 A:R@ 2DROP ( number of errors )
                 A:R@ ( save base address )
                 DUP @ + CELL+ ( base of initial code )
                 DUP @ + CELL+ ( base of first value )
                 BEGIN
                       >!
                 WHILE
                       A:R@ A:@ A:-1 A:= ( value == -1 )
                       CELL+ ( counted name )
                       A:IF
                          DUP COUNT + ALIGNED ( base of references )	
                          R> R1@ A:R1@ NIP
                          OVER REFCHECK CALL ( check all references of this name ) 
                          IF
                             OVER COUNT TYPE CALL .(  undefined ) CR CALL
                             R1@ 0<
                             IF ( return unresolved record )
                                15 OUTQUEUEAPPENDRAWCHAR CALL
                                ERRORREC OUTQUEUEAPPENDMASKEDCHAR CALL
                                R@ SENDWORD CALL ( base address )
                                A:R@ A:R1@ A:- ( length of aligned name )
                                OVER OVER
                                @ 1+ CELLS R> + OUTQUEUEAPPENDARRAY CALL ( record )
                                26 OUTQUEUEAPPENDRAWCHAR CALL
                             ENDIF
                             R> A:1+ A:R>
                          ENDIF ( error message )
                          NIP ( remove counted name )
                       ENDIF
                       DUP @ 1+ CELLS + ( base of next value )
                REPEAT
                2DROP A:DROP R@ A:2DROP ;

:LOCAL INITEXECUTE 1 EI A:;

( resolve references and execute initial code
  parameter:
  return:
      error count
)

:LOCAL LINKS MODULETOP @ DUP LENGTH' @ 1- + SWAP ( base address, first address after module )
             DUP DUP @ CELL+ + CELL+ A:R> ( initial code )
             OVER OVER R@ OVER - PASS CALL ( resolve all resolvable references )
             R@ OVER - 0 LINKCHECK CALL ( unresolved references in initial code? )
             0=!
             IF
               DROP R> A:DI INITEXECUTE CALL A:EI ( execute initial code )
               DEPTH 0=
               IF
                  MODULETOP @ DUP LENGTH' @ 1- + SWAP
                  OVER OVER 1 CELLS PASS CALL ( resolve remaining references )
                  1 CELLS LATE @ LINKCHECK CALL ( any unresolved references in code? )
               ELSE
                  CR CALL .( ******** data stack corrupted through initial code, forces a restart ! ******** ) CR CALL 
                  ABORT CALL
               ENDIF
             ELSE
               A:DROP
             ENDIF ;

( load and link a module
  parameter:
  return:
)

:LOCAL LOAD MODULETOP @ MODULE' @ 0= LATE @ 0= OR IF HERE @ HERE' ! DUP MODULE' ! ENDIF ( save heap, module top )
            0 RECERROR ! -1 LINK C!
            -1 ( max. length )
            0f0H ( fileID )
            READ-FILE CALL ( read module completely )
            RECERROR @ OR
            OVER LENGTH' !
            DUP IOR' ! NIP 0=
            IF ( read successfully )
               MODULETOP @ A:R@ 5
               R@ CELL+ ( address of counted module name )
               create CALL ( insert module into dictionary )
               LINKS CALL 0> A:R@ INVERT ( link module ) 
               R1@ LENGTH' @ 1- + C@ ( resident flag )
               AND ( join errors with flag )
               IF ( linked successfully )
                  R1@ DUP A:2DROP @ + CELL+ MODULETOP ! ( new top )
               ELSE ( stays not resident )
                  A:IOR' A:! ( note failure )
                  HERE' @ HERE ! ( reset heap )
                  A:DROP 0 MODULE' DUP @ CELL+ FIND CALL EXECUTE CALL ! ( reset moduletop )
               ENDIF
            ENDIF
            15 OUTQUEUEAPPENDRAWCHAR CALL MODULPROCESSED OUTQUEUEAPPENDMASKEDCHAR CALL
            RECERROR @ SENDWORD CALL
            IOR' @ 0<> 26 + OUTQUEUEAPPENDRAWCHAR CALL ( quit with 26, if succesful, with 25 otherwise )
            0 LINK C! ;

:LOCAL LATELINK -1 READ-BUFFER CALL ( read records )
                A:R@ 0=
                IF
                   A:R1@ + ( end )
                   R> ( start )
                   BEGIN
                         >!
                   WHILE
                         A:R@ A:@ ( base )
                         CELL+ DUP ( name )
                         COUNT + ALIGNED ( references )
                         OVER FIND CALL ( search in dictionary )
                         IF
                            A:R1@
                            R1@ ( base )
                            R@ A:2DROP ( references )
                            4 ( offset )
                            RESOVLELABEL CALL ( link )
                         ELSE
                            DROP A:DROP A:1+ ( update error count )
                            OVER COUNT TYPE CALL
                            "  unresolved "
                            COUNT TYPE CALL CR CALL
                         ENDIF
                         NIP DUP @ 1+ CELLS + ( next record )
                   REPEAT
                ENDIF
                2DROP 0 MODULE' A:0<>!
                A:IF
                     HERE' @ HERE !
                     DUP @ CELL+ FIND CALL EXECUTE CALL ( reset moduletop )
                ENDIF
                !
                15 OUTQUEUEAPPENDRAWCHAR CALL MODULPROCESSED OUTQUEUEAPPENDMASKEDCHAR CALL
                R> DUP SENDWORD CALL
                0<> 26 + 1 EI OUTQUEUEAPPENDRAWCHAR CALL ( quit with 26, if succesful, with 25 otherwise )
                ;
