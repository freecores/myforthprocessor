( 
  BIOS for FORTH-System

  Copyright © 2004-2008 by Gerhard Hohner
)
  

DEFINE MEMORYUNUSED 21

( create a memory pool of defined length
  parameter:
      size
)

: CREATEMM LDMSB CALL A:R@
           ALIGN CALL ( align heap )
           HERE @
           1 R@ LSHIFT ( pool size )
           R> 3 - 2* CELLS + ALLOT CALL ( block allocated )
           MMBASE 2! ( save base address of pool and length )
	     RESETDYNAMIC CALL
           ;

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

: RELEASEMM MMBASE @ 0<>! IF DUP HERE ! ENDIF DUP - DUP MMBASE 2! ;

( service reset pool
  parameter:
  return:
)

:LOCAL RESETDYNAMIC MMBASE 2@ 0<>!
                    IF
                       OVER OVER CELL+ CELL+ ! ( set size of free block )
                       A:R1@ DUP 1 R@ LSHIFT +
                       DUP R> 3 - 2* SETBLOCKZERO CALL
                       OVER OVER !       ( set forward reference of highest root )
                       OVER OVER CELL+ ! ( set backward reference of highest root )
                       DROP 0 OVER !     ( set forward reference of free block )
                       0 OVER CELL+ !    ( set backward reference of free block )
                       10 OVER OCCUPYBLOCK CALL
                       CELL+ CELL+ BUILD> 1 10 LSHIFT 8 - 2/ 2/ DOES> SETBLOCKZERO CALL
                       0 MACCU !         ( clear accu )
                    ENDIF
			  2DROP ;

( move an aligned block of memory
  parameter:
      length
      target address
      source address
  return:
)

:LOCAL MOVEBLOCKUP 0 ?DO OVER @ OVER ! CELL+ A:R> CELL+ R> LOOP 2DROP ;

( get parameters of handle array
  parameter:
  return:
      base address of array
      address of length of array
)

:LOCAL GETHANDLEARRAY MMBASE @ DUP CELL+ CELL+ ;

( set an aligned block of memory to zero
  parameter:
      length
      target address
  return:
)

:LOCAL SETBLOCKZERO 0 ?DO 0 OVER ! CELL+ LOOP DROP ;

( get power of Msb
  parameter:
      number
  return:
      fitting power of 2 for number
)

:LOCAL LDMSB A:31 BEGIN 0<! INVERT R@ 0> AND WHILE A:1- 2* REPEAT 2* 0= 1+ R> + ;

( compress occupied memory partially
  parameter:
      size needed
  return:
)

:LOCAL COMPACTPOOL 5 <! NIP IF DROP EXIT ENDIF ( no compression possible )
                   A:MMBASE A:2@
                   1- DUP R1@ - NEGATE CELLS 2*
                   R@ 1 R1@ LSHIFT + + ( root )
                   A:R1@ ( size )
                   DUP CELL+ @ ( last entry )
                   DUP R1@ - 1 R@ LSHIFT XOR 0= ( is buddy handle array? )
                   IF
                      DROP DUP @ ( first entry )
                      DUP R1@ - 1 R@ LSHIFT XOR 0=
                      IF DROP 0 ENDIF ( not valid )
                   ENDIF
                   OVER 2@ - 0<> OVER 0<> AND ( more than one valid free blocks in root? )
                   IF
                      A:R1@ DUP R> UNCHAINFREEBLOCK CALL ( remove block )
                      DUP R1@ - 1 R@ LSHIFT XOR R1@ + ( buddy )
                      1 R@ LSHIFT OVER MOVEBUDDY CALL
                      DROP R1@ - 1 R@ LSHIFT INVERT AND R1@ +
                      OVER 2 CELLS - ( next higher root )
                      R@ 1+
                      CHAINFREEBLOCK CALL ( into free list )
                      DUP
                   ENDIF
                   2DROP DROP A:DROP A:2DROP ;

( move buddy elsewhere
  parameter:
      buddy address
      size
  return:
)

:LOCAL MOVEBUDDY A:R1@ A:R@
                 A:MMBASE A:2@
                 ( remove included free blocks )
                 BEGIN
                       OVER
                 WHILE
                       DI DROP
                       DUP @ 3 >! NIP OVER 32 < AND 0=
                       IF
                          DROP
                          DUP CELL+ CELL+ @ ( size of free block )
                          R@ 1 R1@ LSHIFT +
                          OVER R1@ - NEGATE CELLS 2* + ( concerned root )
                          A:R1@ NIP ( move size of free block )
                          A:R1@ ( copy buddy )
                          UNCHAINFREEBLOCK CALL ( remove from list )
                          R@ R1@ A:2DROP
                       ENDIF
                       1 EI
                       A:R1@
                       A:1 A:R@ 2DROP A:LSHIFT R@ A:+ ( next block )
                       -
                       R>
                 REPEAT
                 2DROP A:2DROP R1@ R@ A:2DROP
                 ( move occupied included blocks )
                 BEGIN
                       OVER
                 WHILE
                       DI DROP
                       DUP @ 3 >! NIP OVER 32 < AND
                       IF
                          A:R@ A:R1@   ( block size, block address )
                          OVER CELL+ @ ( handle )
                          OVER SEARCHLAST CALL 0<>!
                          IF R1@ OVER OCCUPYBLOCK CALL NIP 0<>! IF A:R@ CELL+ CELL+ OVER CELL+ ! R> ENDIF ENDIF NIP ( allocate block, if possible )
                          0<>!
                          IF R@ OVER 1 R1@ 2 - LSHIFT MOVEBLOCKUP CALL ENDIF ( copy block )
                          DROP A:2DROP
                       ELSE
                          DROP
                          DUP CELL+ CELL+ @ ( size of free block )
                       ENDIF
                       1 EI
                       A:R1@
                       A:1 A:R@ 2DROP A:LSHIFT R@ A:+ ( next block )
                       -
                       R>
                 REPEAT
                 2DROP DI DROP ;

( get a free handle
  parameter:
  return:
     address of handle
)

:LOCAL GETFREEHANDLE A:MMBASE A:@ A:DUP
                     R@
                     A:1 A:OVER A:@ A:LSHIFT A:+ ( end address )
                     BEGIN
                        DUP @ 0<> OVER R@ < AND
                     WHILE
                        2 CELLS +
                     REPEAT
                     DUP R@ = ( no one available )
                     IF
                        1 R1@ @ A:R@ LSHIFT OVER MOVEBUDDY CALL ( move conent of buddy )
                        DUP 1 R@ 2 - LSHIFT SETBLOCKZERO CALL ( clear buddy )
                        R> 1+ R1@ ! ( join buddys )
                     ENDIF
                     A:2DROP ;
               
( search last best fitting free block  
  parameter:
      size
  return:
      0, if not found, address of block otherwise
)

:LOCAL SEARCHLAST A:MMBASE A:2@
                  R@ 1 R1@ LSHIFT + ( root table )
                  OVER R1@ - NEGATE A:R@ CELLS 2* + ( concerned root )
                  BEGIN
                        R@ 0<> OVER @ 0= AND
                  WHILE
                        2 CELLS - ( next higher root )
                        A:1-
                  REPEAT
                  CELL+ @ NIP ( get last free block from list )
                  A:DROP A:2DROP
                  ;

( occupy a distinct free block
  parameter:
      block address
      size
  return:
      address of occupied block
)

:LOCAL OCCUPYBLOCK A:R@ A:CELL+ A:CELL+ A:@ ( size of free block )
                   A:MMBASE A:2@
                   OVER ( copy size )
                   1 R1@ LSHIFT R@ + ( root table )
                   R1@ A:NIP R1@ - CELLS 2* + ( address of block's root )
                   A:R1@ NIP ( move size )
                   OVER OVER UNCHAINFREEBLOCK CALL ( remove block from free list )
                   R1@ A:NIP R1@ A:NIP A:R1@ A:R@ 2DROP
                   BEGIN
                      R> R1@ A:R1@ <>
                   WHILE
                      2 CELLS + ( next lower root )
                      A:1-      ( next smaller size )
                      OVER R1@ - 1 R@ LSHIFT XOR R1@ + ( buddy )
                      OVER R@ CHAINFREEBLOCK CALL ( put buddy into free blocks list )
                   REPEAT
                   DROP A:R@
                   ! ( set size )
                   R@ A:2DROP A:2DROP ;

( search best fitting free block in ascending order
  parameter:
      handle address
      size
  return:
      0, if not found, address of block otherwise
)

: BESTFREEBLOCK A:R> DUP SEARCHLAST CALL 0=! ( search last best fitting free block )
                IF DROP MARK&SWEEP CALL DUP SEARCHLAST CALL ENDIF 0=! ( run garbage collection and try again )
                IF DROP DUP COMPACTPOOL CALL DUP SEARCHLAST CALL ENDIF ( compress pool and try again )
                0<>! IF OCCUPYBLOCK CALL DUP 0<>! IF R@ OVER CELL+ ! ENDIF ENDIF A:DROP NIP ; ( allocate block, if available )

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
            11 + -4 AND ( physical length = (desired length + 3) / 4 * 4 + 8 )
            DUP MACCU @ + MACCU ! ( accumulate requested size )
            LDMSB CALL ( adjust length to next fitting power of 2 ) 
            GETFREEHANDLE CALL ( allocate handle )
            0<>! IF OVER OVER BESTFREEBLOCK CALL 0<>! IF CELL+ CELL+ OVER CELL+ ! WHITE OVER ! DUP ENDIF NIP ENDIF ( allocate block, if possible )
            SWAP 0<> OVER 0<> AND IF DUP CELL+ @ 1 OVER 2 CELLS - @ 2 - LSHIFT 2 - SETBLOCKZERO CALL ENDIF ( initialize allocated block with 0 )
            A:EI
         ENDIF
         0=!
         ;

( validate the address of a handle
  parameter:
      address of handle
  return:
      -1, if valid, 0 otherwise
)

: HANDLEVALID MMBASE @ -
              A:R@ A:7 A:AND A:0=                                  ( true, if a multiple of 8 )
              2/ 2/ 2/
              A:R@ A:0> A:AND 1-                                   ( true, if address not below handle pool )
              1 BUILD> MMBASE CELL+ DOES> @ 3 - LSHIFT <           ( true, if position below end of handle pool )
              R> AND ;
              
( append free block
  parameter:
      ld(size)
      root address
      block address
  return:
)

:LOCAL CHAINFREEBLOCK A:R> A:R@ A:@ ( first free block in list )
                      A:R@ A:CELL+ A:@ ( last free block in list )
                      OVER OVER CELL+ ! ( new last free block in root )
                      R1@ 0= IF OVER OVER ! ENDIF ( new first free block in root )
                      DROP R@ OVER CELL+ ! ( predecessor of free block assigned )
                      0 OVER ! ( free block has no successor )
                      R@ IF DUP R@ ! ENDIF ( new block is successor of last block ) 
                      A:2DROP
                      A:R> A:CELL+ A:CELL+ A:! ( set size )
                      ;

( remove block from free list
  parameter:
      root address
      block address
  return:
)

:LOCAL UNCHAINFREEBLOCK A:R1@ A:@ ( next free block )
                        A:R1@ A:CELL+ A:@ ( previous free block )
                        R@
                        IF
                           R1@ R@ ! ( update forward reference of predecessor )
                        ELSE
                           R1@ OVER ! ( update forward reference of root )
                        ENDIF
                        R1@
                        IF
                           R@ R1@ CELL+ ! ( update backward reference of follower )
                        ELSE
                           R@ OVER CELL+ ! ( update backward reference of root )
                        ENDIF
                        A:2DROP 2DROP
                        ;

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
          A:MMBASE A:2@ A:0
          DUP CELL+ A:R@ @ 8 - ( address of allocated block )
          A:! ( clear data pointer )
          R@ 1 R1@ LSHIFT + ( address of root table )
          OVER @ R1@ A:R1@ - NEGATE CELLS 2* + ( root of concerned free blocks )
          BEGIN
                OVER R1@ - 1 R@ LSHIFT XOR R1@ + ( buddy )
                DUP @ 3 >! NIP OVER 32 < AND NIP 0= ( free buddy, if not 3 < buddy size < 32 )
                OVER CELL+ CELL+ @ R@ = AND ( buddy is not splitted! )
          WHILE
                OVER UNCHAINFREEBLOCK CALL ( remove buddy from free list )
                OVER R1@ - 1 R@ LSHIFT INVERT AND R1@ +
                A:R1@ NIP NIP R>
                8 - ( root of new block )
                A:1+ ( next size )
          REPEAT
          DROP
          R> CHAINFREEBLOCK CALL ( insert block into free list )
          A:2DROP
          R>
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
            3 + -4 AND ( size now a multiple of 4 )
            OVER CELL+ @ 2 CELLS - @
            1 OVER LSHIFT 8 - NIP ( current size )
            OVER MIN ( minimum of both sizes )
            UNUSED CALL -4 AND MIN 2/ 2/ A:R>
            OVER CELL+ @ ( data array )
            HERE @ ALIGNED
            R@ MOVEBLOCKUP CALL ( copy record on heap )
            A:R1@ A:@ ( copy reference counter )
            OVER 1 OVER ! FREE CALL DROP ( free record )
            8 + ( physical length = (desired length + 3) / 4 * 4 + 8 )
            LDMSB CALL ( adjust length to next fitting power of 2 )
            OVER ( old handle )
            OVER OVER BESTFREEBLOCK CALL 0<>! IF CELL+ CELL+ OVER CELL+ ! R@ OVER ! DUP ENDIF NIP A:DROP ( allocate block, if possible )
            SWAP 0<> OVER 0<> AND IF HERE @ ALIGNED OVER CELL+ @ R@ MOVEBLOCKUP CALL ENDIF A:DROP ( fill allocated block )
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
                 IF CELL+ @ 2 CELLS - @ 1 SWAP LSHIFT 2 CELLS - ELSE DUP - ENDIF
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
                       1 BUILD> MMBASE CELL+ DOES> @ 1- LSHIFT 2* SENDWORD CALL ( dynamic memory )
                       RAMSIZE MODULETOP @ - SENDWORD CALL ( program memory )
			     26 OUTQUEUEAPPENDRAWCHAR CALL
			     ;
