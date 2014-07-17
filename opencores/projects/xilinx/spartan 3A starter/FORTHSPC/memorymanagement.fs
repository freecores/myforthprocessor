( 
  BIOS for FORTH-System

  Copyright © 2004-2008 by Gerhard Hohner
)
  

DEFINE MEMORYUNUSED 21

( create a memory pool of defined length
  parameter:
      size
)

: CREATEMM A:DI ALIGNED ( make size a multiple of 4 )
           ALIGN CALL ( align heap )
           HERE @
           OVER ALLOT CALL ( block allocated )
           MMBASE 2! ( save base address of pool and length )
	        RESETDYNAMIC CALL
           A:EI ;

( service create pool
  parameter:
  return:
)

:LOCAL ALLOCATEDYNAMIC 4 READ-BUFFER CALL 2DROP @ ( size )
                       CREATEMM CALL ;
							  
( realease memory pool
  parameter:
  return:
)

: RELEASEMM A:DI MMBASE @ 0<>! IF DUP HERE ! ENDIF DUP - DUP MMBASE 2! A:EI ;

( service reset pool
  parameter:
  return:
)

:LOCAL RESETDYNAMIC MMBASE 2@ 0<>!
                    IF
			     A:R1@ A:3 A:CELLS A:- ( length of initial free block )
                       DUP CELL+ CELL+ CELL+ OVER ( address of first free block )
                       OVER OVER CELL+ ( address of last free block )
                       0 OVER CELL+ ( number of present handles )
                       OVER OVER CELL+ ( forward reference of free block )
                       OVER OVER CELL+ A:R@ CELL+ ( backward reference of free block )
                       ! ! ! ! ! A:!
                    ENDIF
			  2DROP ;

( move an aligned block of memory
  parameter:
      length
      source address
      target address
  return:
      next target address
)

:LOCAL MOVEBLOCKUP 0 ?DO CELL+ A:R@ @ OVER CELL+ ! CELL+ R@ A:DROP LOOP DROP ;

( move an aligned block of memory
  parameter:
      length
      source address
      target address
  return:
      next target address
)

:LOCAL MOVEBLOCKDOWN 0 ?DO 1 CELLS - A:R@ @ OVER 1 CELLS - A:R@ ! DROP R@ R1@ A:2DROP LOOP DROP ;

( get parameters of handle array
  parameter:
  return:
      base address of array
      address of length of array
)

:LOCAL GETHANDLEARRAY MMBASE @ CELL+ CELL+ DUP CELL+ ;

( compress occupied memory
  parameter:
  return:
)

:LOCAL COMPACTPOOL A:MMBASE A:@
                   BEGIN R@ @ R@ CELL+ @ <>! ( root forward != root backward )
                   WHILE
                      DI DROP
                      NIP A:R@ DUP CELL+ @ A:R@ + DUP ( destination of move )
                      R1@ ( source for move )
                      DUP
                      A:OVER A:CELL+ A:CELL+ A:@ ( backward reference )
                      A:DUP A:CELL+ A:@ ( size of backward reference )
                      R1@ R@ + - 2/ 2/ ( size of move )
                      MOVEBLOCKDOWN CALL
                      ?DO
                          I CELL+ DUP CELL+ ( new data pointer )
                          I @ CELL+ ! ( update handle )
                          @ ( size of block is increment )
                      +LOOP
                      R@ A:DROP R1@ A:NIP A:NIP + ( new size of new backward root )
                      R@ CELL+ ! ( store it )
                      0 R@ ! ( its forward reference is null )
                      A:OVER A:CELL+ A:! ( store new backward root )
                      1 EI
                   REPEAT
                   DI 2DROP 0<>!
                   IF ( pool not full )
                      R@ CELL+ CELL+ DUP @ CELLS 2* + CELL+ <>! ( desired root present ? )
                      IF
                         A:R1@ A:CELL+ A:@ ( size of free block )
                         A:R@ DROP ( move desired root )
                         R1@ A:R1@ ( copy size, copy source address )
                         + DUP ( destination )
                         R@ ( source )
                         R@ A:DROP R@ - 2/ 2/ ( size of move )
                         MOVEBLOCKDOWN CALL
                         ?DO
                             I CELL+ DUP CELL+ ( new data pointer )
                             I @ CELL+ ! ( update handle )
                             @ ( size of block is increment )
                         +LOOP
                         0 R@ ( forward reference )
                         OVER OVER CELL+ CELL+ ! ! ( backward reference of free block is null )
                         R1@ A:NIP ( size of free block )
                         R@ CELL+ ! ( store it )
                         A:OVER A:OVER A:OVER A:CELL+ A:! A:! ( store both roots )
                         DUP DUP
                      ENDIF
                      DROP
                   ENDIF
                   DROP A:DROP ;

( reserve a free handle
  parameter:
  return:
     address of handle
)

:LOCAL GETFREEHANDLE A:MMBASE A:@
                     R@ CELL+ CELL+ ( address of handlecount )
                     DUP @ 0 ?DO CELL+ DUP CELL+ @ 0= IF UNLOOP A:DROP EXIT ENDIF CELL+ LOOP ( look for empty data pointer )
                     CELL+
                     R@ @ ( root of first free block )
                     A:R@ A:OVER A:CELL+ A:@ A:= ( forward root == backward root )
                     <>! NIP IF COMPACTPOOL CALL ENDIF ( compress allocated blocks, if first block not free )
                     R1@ @ ( root of first free block )
                     <>! NIP IF DUP - A:DROP EXIT ENDIF ( no free memory )
                     DUP CELL+ @ 4 CELLS - ( minor length of first free block )
                     17 <! NIP IF 2DROP 0 A:DROP EXIT ENDIF ( handle not allocated, no free block big enough )
                     A:R1@ OVER @ ( forward reference )
                     A:4 A:CELLS A:+ ( new address of first free block )
                     R@ A:DROP R1@ A:R1@ ! ( new forward root )
                     R1@ A:NIP IF R@ R1@ CELL+ ! ENDIF ( new backward root )
                     DUP R@ ! ( forward reference of first free block )
                     0<>! IF CELL+ CELL+ A:DUP A:R@ A:! ENDIF DROP ( backward reference of second block modified )
                     A:CELL+ R@ ! ( modified length of first free block )
                     A:CELL+ 0 R@ ! ( backward reference of first free block )
                     0 OVER OVER OVER CELL+ CELL+ ! ! ( reset counter of allocated handles )
                     A:DROP A:CELL+ A:CELL+
                     R@ @ HALF+ R@ ! ( count up handles )
                     A:DROP
                     4 0 DO 0 OVER I CELLS + ! LOOP ( clear handle )
                     ;
               
( search last best fitting free block  
  parameter:
      size
  return:
      0, if not found, address of block otherwise
)

:LOCAL SEARCHLAST MMBASE @
                  CELL+ @ 0=! IF NIP EXIT ENDIF ( no free block available )
                  A:0 A:-1 A:R@ DROP ( null, max. length, reference to free block )
                  BEGIN
                        R@ CELL+ @ U>! INVERT OVER R1@ U< AND ( size <= length of block && length of block < length of best fitting block )
                        IF A:NIP A:NIP A:R@ A:OVER ENDIF ( base of block, his length, base of block )
                        OVER = A:CELL+ A:CELL+ A:@ R@ 0= OR
                  UNTIL ( size == length of best fitting block || backward reference == null )
                  A:2DROP R@ A:DROP NIP ;

( occupy a distinct free block
  parameter:
      block address
      size
  return:
      address of occupied block
)

:LOCAL OCCUPYBLOCK A:R@ CELL+ @ ( copy block address, load his length )
                   OVER OVER - NEGATE 16 <! INVERT NIP 
                   IF
                      R@ A:R1@ CELL+ ! ( difference replaces length of remaining free block )
                      DROP
                      A:+ R@ CELL+ ! ( calculate address of occupied block and store length )
                   ELSE
                      2DROP DROP ( the 3 sizes not longer needed )
                      R@ @ R@ CELL+ CELL+ @ ( forward reference, backward reference )
                      0=! IF OVER MMBASE @ ! ( root forward changed ) ELSE OVER OVER ! ( update forward reference of predecessor ) ENDIF
                      OVER 0= IF DUP MMBASE @ CELL+ ! ( root backward changed ) ELSE OVER CELL+ CELL+ ! DUP ( update backward reference of successor ) ENDIF
                      2DROP
                   ENDIF
                   R@ A:DROP ;

( search best fitting free block in ascending order
  parameter:
      handle address
      size
  return:
      0, if not found, address of block otherwise
)

:LOCAL BESTFREEBLOCK A:R@ DROP DUP SEARCHLAST CALL 0=! ( search last best fitting free block )
                     ( do garbage collection and try again )
                     IF DROP COMPACTPOOL CALL DUP SEARCHLAST CALL ENDIF ( compress pool and try again )
                     0<>! IF OCCUPYBLOCK CALL DUP 0<>! IF R@ OVER ! ENDIF ENDIF A:DROP NIP ; ( allocate block, if available )
 
( allocate a block of memory
  parameter:
      size in byte
  return:
      ior, is 0 if allocated
      address of handle
)

: ALLOCATE MALLOC CALL 0=! IF OVER STATIC OVER ! DROP ENDIF ;

: MALLOC 0<>!
         IF
            A:DI 
            3 + -4 AND 2 CELLS + ( physical length = (desired length + 3) / 4 * 4 + 8 )
            DUP MACCU @ + MACCU ! ( accumulate requested size )
            GETFREEHANDLE CALL ( allocate handle )
            0<>! IF OVER OVER BESTFREEBLOCK CALL 0<>! IF CELL+ CELL+ OVER CELL+ ! WHITE OVER ! DUP ENDIF NIP ENDIF ( allocate block, if possible )
            SWAP 0<> OVER 0<> AND IF A:R@ A:CELL+ A:@ 0 R@ ! R@ A:1 A:CELLS A:- R@ R@ A:DROP @ 2/ 2/ 1- 1- 1- MOVEBLOCKUP CALL DROP ENDIF ( initialize allocated block with 0 )
            A:EI
         ENDIF
         0=! ;

( validate the address of a handle
  parameter:
      address of handle
  return:
      -1, if valid, 0 otherwise
)

: HANDLEVALID A:DI MMBASE @ 8 + A:R@ 4 + -
              A:R@ A:7 A:AND A:0=        ( true, if difference a multiple of 8 )
              2/ 2/ 2/ 0<! INVERT        ( true, if not negative )
              OVER R1@ @ < AND           ( true, if lower than handle count )
              R@ AND NIP A:2DROP A:EI ;

( free a block of memory
  parameter:
      address of handle
  return:
      -1, if deallocated, 0 otherwise
)

: FREE A:DI DUP HANDLEVALID CALL OVER CELL+ @ 0<> AND DUP
       IF
          OVER DUP
          0 OVER ! ( mark free handle )
          A:0
          CELL+ A:R@ @ -2 CELLS + ( address of allocated block )
          A:! ( clear data pointer )
          A:MMBASE A:@ ( base of pool )
          R@ CELL+ @ <! ( at least one free block after the allocated one present )
          IF ( allocated block has at least one following free block )
             DROP DUP BEGIN DUP CELL+ @ + DUP @ HANDLEVALID CALL 0= UNTIL ( look for next not free block )
             A:R@ ( copy address of follower )
             OVER ! ( forward reference of allocated becomes follower )
             A:CELL+ A:CELL+ A:DUP A:@ ( backward reference of follower )
             DUP R1@ A:NIP ! ( backward reference of follower becomes address of allocated block )
             DUP R@ IF R@ ( forward reference of predecessor becomes address of allocated block ) ELSE R1@ ( allocated becomes forward root ) ENDIF !
             A:R@ A:CELL+ A:CELL+ A:! ( backward reference of allocated block becomes old backward reference of follower )
          ELSE ( allocated block becomes new backward root )
             0=!
             IF
                OVER R@ ! ( new free block is new forward root too )
             ELSE
                OVER OVER ! ( forward reference of predecessor becomes address of allocated block )
             ENDIF
             OVER CELL+ CELL+ ! ( backward reference of allocated block becomes address of predecessor )
             0 OVER ! ( forward reference of allocated block becomes null )
             DUP R@ CELL+ ! ( becomes new backward root )
          ENDIF
          R@ A:DROP
          2 0
          DO
             A:R@ DROP DUP
             IF
                DUP DUP CELL+ @ + ( address points behind allocated block )
                OVER @ ( forward reference of allocated block )
                =! NIP IF ( coalescence detected )
                          A:R@ ( copy address of neighbour )
                          @ ( forward reference of neighbour )
                          0=! IF OVER R1@ CELL+ ! ENDIF ( allocated becomes new backward root )
                          OVER ! ( forward reference of neighbour replaces forward reference )
                          A:CELL+ A:@ ( size of neighbour )
                          DUP CELL+ A:R@ A:@ ( size of allocated )
                          A:+ A:R@ DROP A:! ( sum replaces size )
                          DUP DUP @ 0<>! IF CELL+ CELL+ ! DUP DUP ENDIF 2DROP DUP ( backward reference of new neighbour updated )
                       ENDIF
                DROP 
                CELL+ CELL+ @ ( backward reference of allocated block )
             ENDIF
             R@ A:DROP
          LOOP
          DROP
          2DROP
       ENDIF
       NIP A:EI ;

( resize an allocated block of memory
  parameter:
      size
      address of handle
  return:
      ior, is 0 if allocated
      address of handle
)

: RESIZE OVER HANDLEVALID CALL
         IF
            A:DI
            3 + -4 AND ( make size a multiple of 4 )
            OVER CELL+ @ 1 CELLS - @ 8 - ( current size )
            OVER MIN ( minimum of both sizes )
            UNUSED CALL -4 AND MIN 2/ 2/ >R A:R1@
            HERE @ A:R@ ALIGNED
            R1@ A:NIP CELL+ @
				R1@ 2* 2* 3 + ALLOT  ( allocate scratch on heap )
            R1@ MOVEBLOCKUP CALL DROP ( copy record on heap )
            A:R1@ A:@ ( copy reference counter )
            OVER 1 OVER ! FREE CALL DROP ( free record )
            2 CELLS + ( physical length = (desired length + 3) / 4 * 4 + 8 )
            OVER ( old handle )
            OVER OVER BESTFREEBLOCK CALL 0<>! IF CELL+ CELL+ OVER CELL+ ! R@ OVER ! DUP ENDIF NIP A:DROP ( allocate block, if possible )
            SWAP 0<> OVER 0<> AND IF DUP CELL+ @ R@ ALIGNED R1@ MOVEBLOCKUP CALL DROP ENDIF ( fill allocated block )
				A:HERE A:! A:DROP
            0=
            A:EI
         ELSE ( ordinary allocate )
            NIP
            ALLOCATE CALL
         ENDIF
         ;

( get size of allocated block
  parameter:
      address of handle
  return:
      size
)

: ALLOCATED-SIZE A:DI DUP HANDLEVALID CALL
                 IF CELL+ @ 1 CELLS - @ 8 - ELSE DUP - ENDIF
                 A:EI ;

( increment reference of a valid handle
  parameter:
      handle
  return:
)

: INCREFERENCE A:DI DUP HANDLEVALID CALL IF DUP @ 1+ OVER ! ENDIF DROP A:EI ;

( deccrement reference of a valid handle
  parameter:
      handle
  return:
)

: DECREFERENCE A:DI DUP HANDLEVALID CALL IF DUP @ 1 - A:R@ 0 +B OVER ! A:0= A:IF FREE CALL ENDIF ENDIF DROP A:EI ;

( free space on heap
  parameter:
  return:
      free space in byte
)

: UNUSED DICTTOP @ HERE @ - ;

( align heap pointer
  parameter:
  return:
)

: ALIGN HERE @ 3 + -4 AND HERE ! ;

( allocate space on heap
  parameter:
      space
  return:
)

: ALLOT HERE @ + HERE ! ;

( service get size of unused memory and size of dynamic memory
  parameter:
  return:
)

:LOCAL GETUNUSEDMEMORY 15 OUTQUEUEAPPENDRAWCHAR CALL ( API )
			     MEMORYUNUSED OUTQUEUEAPPENDMASKEDCHAR CALL ( unused memory )
                       UNUSED CALL SENDWORD CALL ( free heap )
                       BUILD> MMBASE CELL+ DOES> @ SENDWORD CALL ( dynamic memory )
                       RAMSIZE MODULETOP @ - SENDWORD CALL ( program memory )
			     26 OUTQUEUEAPPENDRAWCHAR CALL
			     ; 