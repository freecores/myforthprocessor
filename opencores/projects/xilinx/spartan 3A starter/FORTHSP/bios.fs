( 
  BIOS for FORTH-System

  Copyright © 2004-2008 by Gerhard Hohner
)
  

( prolog for romable code )

DEFINE RAMSIZE BUILD> 1 26 LSHIFT DOES>               \ size of RAM

DEFINE DUMP 2/ 2/ RAMSIZE 4 - !                       \ parameter is a starting address, where memory dump begins

DEFINE WHITE 1                                        \ block allocated, but garbage
DEFINE GRAY 2                                         \ block allocated, must be examined
DEFINE BLACK 4                                        \ block allocated and esential
DEFINE SCOPE 8                                        \ block allocated from Java and holds references from a specific scope
DEFINE GLOBAL 9                                       \ block allocated from Java and holds references from static variables
DEFINE STATIC 16                                      \ block allocated from FORTH-program

DEFINE SIMULATION 0                                   \ must be zero at last!!!!!!!!!!!!!

BUILD> RAMSIZE 0 + DOES> $ORG                         \ start address
                           
DEFINE SYSCLOCK 0fffffffcH @                          \ system clock

DEFINE SIODATA 0ffffffe0H                             \ dataport UART
DEFINE SIODEFINE 0ffffffe4H                           \ definition port of UART
DEFINE SIOSCALER 0ffffffe8H                           \ prescaler for UART
DEFINE SIOSTATUS 0ffffffecH                           \ status of UART
DEFINE SIOESCAPE 0fffffff0H                           \ escape definition
DEFINE SIOXON 0fffffff4H                              \ xon definition
DEFINE SIOXOFF 0fffffff8H                             \ xoff definition
DEFINE COUNTERPORT 0ffffffc0H 

2                                                     \ 1 stopbit
0                                                     \ no parity
8                                                     \ 8 data bit
                                                      \ baudrate
BUILD> 4500000 SIMULATION AND 9600 SIMULATION INVERT AND + DOES>

SETSIO CALL
27 SIOESCAPE !                                        \ set escape 27
23 SIOXON !                                           \ set xon 23
24 SIOXOFF !                                          \ set xoff 24

\ INCLUDE .\memtest.txt

DEFINE OUTQUEUEBASE 64                                \ serial output buffer
                                                      \ serial input buffer buffer
DEFINE PAD BUILD> OUTQUEUEBASE 2048 + DOES>           \ Scratch buffer
DEFINE INPUTFILEBASE BUILD> PAD 256 + DOES>           \ base address of file handles 0 - 15
DEFINE tib BUILD> INPUTFILEBASE DOES>                 \ buffer base pointer of handle 0
DEFINE >in BUILD> INPUTFILEBASE 8 + DOES>             \ buffer offset of handle 0
DEFINE #tib BUILD> INPUTFILEBASE 12 + DOES>           \ length of data in buffer of handle 0
DEFINE OUTQUEUEOFFSET BUILD> INPUTFILEBASE 256 + DOES> \ buffer offset of output queue
DEFINE OUTQUEUELENGTH BUILD> OUTQUEUEOFFSET 4 + DOES> \ length of data in output queue
DEFINE LASTRECEIVED BUILD> OUTQUEUELENGTH 4 + DOES>   \ last received command
DEFINE STUFFED BUILD> LASTRECEIVED 4 + DOES>          \ stuffing
DEFINE SENTALL BUILD> STUFFED 1+ DOES>                \ message sent
DEFINE CURRENTFILEID BUILD> SENTALL 1+ DOES>          \ currently used handle for serial input
DEFINE LINK BUILD> CURRENTFILEID 1+ DOES>             \ link in progress
DEFINE FILEHANDLE BUILD> STUFFED 4 + DOES>            \ handle marker
DEFINE SWEEP BUILD> FILEHANDLE 4 + DOES>              \ mark and sweep routine, if present
DEFINE JAVATIMER BUILD> SWEEP 4 + DOES>               \ timer routine of java
DEFINE RECERROR BUILD> JAVATIMER 4 + DOES>            \ erroneous received characters
DEFINE MMBASE BUILD> RECERROR 4 + DOES>               \ base of memory pool
DEFINE MACCU BUILD> MMBASE 8 + DOES>                  \ accu of memory pool
DEFINE MILLISECONDS BUILD> MACCU 4 + DOES>            \ system time in milliseconds
DEFINE HERE' BUILD> MILLISECONDS 8 + DOES>
DEFINE IOR' BUILD> HERE' 4 + DOES>
DEFINE LENGTH' BUILD> IOR' 4 + DOES>
DEFINE LATE BUILD> LENGTH' 4 + DOES>
DEFINE MODULE' BUILD> LATE 4 + DOES>
DEFINE SSTACK BUILD> MODULE' 4 + DOES>
DEFINE VSTACK BUILD> SSTACK 4 + DOES>
DEFINE VLENGTH BUILD> VSTACK 4 + DOES>
DEFINE SLENGTH BUILD> VLENGTH 4 + DOES>
DEFINE VBARRIER BUILD> SLENGTH 4 + DOES>
DEFINE LOCKEDI/O BUILD> VBARRIER 4 + DOES>
DEFINE LOCKEDO BUILD> LOCKEDI/O 4 + DOES>
DEFINE TIMERCT BUILD> LOCKEDO 4 + DOES>
DEFINE HLD BUILD> TIMERCT 4 + DOES>                  \ hold
                                                      \ base of dictionary
DEFINE DICTBASE BUILD> RAMSIZE 4 / 3 * DOES>

DEFINE HERE BUILD> DICTBASE 4 - DOES>                 \ pointer to top of heap
DEFINE SPAN BUILD> HERE 12 - DOES>                    \ span
DEFINE BASE BUILD> SPAN 12 - DOES>                    \ base
DEFINE DICTTOP BUILD> BASE 12 - DOES>                 \ points to top of dictionary
DEFINE MODULETOP BUILD> DICTTOP 12 - DOES>            \ points to free module space
DEFINE PRECISION BUILD> MODULETOP 12 - DOES>          \ number of displayable digits in a float number

0 DUP MILLISECONDS 2!                                 \ initialize system time
3 TRIGGER-SYSCLOCK CALL SYSCLOCK 1000 / CALL SETCOUNTER CALL 

RESTART LABEL

DI DROP                                               \ disable interrupts

GETSIO CALL SETSIO CALL

DICTBASE DICTTOP !
0 OUTQUEUEOFFSET !                                    \ reset offset
0 OUTQUEUELENGTH !                                    \ reset length
0 DUP MMBASE 2!						      \ no dynamic memory
0 LATE !                                              \ no late binding
0 MODULE' !                                           \ 
0 VBARRIER !							\ write barrier for on-the-fly
0 VLENGTH !								\ current length of local references
0 SLENGTH !								\ current length of static references
0 SSTACK !                       \ no stack allocated
0 VSTACK !                       \ no stack allocated
0 MACCU !                                             \ clear accu

BUILD> HLD 4 + DOES> VALUE HERE                       \ create and assign heap pointer
0 VALUE SPAN							\ clear span
10 VALUE BASE							\ default is 10
DICTBASE VALUE DICTTOP						\ points before the dictionary space
DICTBASE VALUE MODULETOP					\ begin of module space
16 VALUE PRECISION                                    \ 16 displayable digits
MILLISECONDS VALUE MILLISECONDS                       \ system time

INCLUDE .\dictionary.fs
INCLUDE .\interruptservice.fs
INCLUDE .\consoleOut.fs
INCLUDE .\math.fs
INCLUDE .\fmath.fs
INCLUDE .\formatnumber.fs
INCLUDE .\SIO.fs
INCLUDE .\consoleIn.fs
INCLUDE .\linker.fs
INCLUDE .\fileIO.fs
INCLUDE .\java.txt
INCLUDE .\onthefly.fs 
INCLUDE .\buddy.fs 
 \ INCLUDE .\noonthefly.fs 
 \ INCLUDE .\memorymanagement.fs
INCLUDE .\counter.fs
\ INCLUDE .\lcd.fs
\ INCLUDE .\dac.fs
\ INCLUDE .\adc.fs

" MARK&SWEEP " 0 FIND_ CALL SWEEP !
" JAVATIMER " 0 FIND_ CALL JAVATIMER !

PAD CONSTANT PAD
HLD CONSTANT HLD
tib CONSTANT tib
>in CONSTANT >in
#tib CONSTANT #tib
0 CONSTANT SOURCE-ID
SYSCLOCK CONSTANT SYSCLOCK
JAVATIMER CONSTANT JAVATIMER                             \ java timer
SWEEP CONSTANT SWEEP                                     \ mark and sweep
VLENGTH VALUE VLENGTH
VSTACK VALUE VSTACK

CLEARDATA LABEL                                       \ entry for ABORT
DEFINE DATASTACK BUILD> RAMSIZE DOES> 
DI DROP
0 DATASTACK SP!                                       \ reset data stack
RESETDYNAMIC CALL													\ reset dynamic memory

CLEARRET LABEL                                        \ entry for QUIT
DEFINE RETURNSTACK BUILD> RAMSIZE 1 12 LSHIFT - DOES> 
DI DROP
A:0 A:RETURNSTACK A:SP!                               \ reset return stack

BUILD> OUTQUEUEBASE 1024 + DOES> tib !                \ assign input buffer
1024 BUILD> INPUTFILEBASE CELL+ DOES> !               \ size of input buffer

0 LOCKEDI/O !
0 LOCKEDO !
0 TIMERCT !
0 STUFFED !                                           \ no stuffing, not sent all, input file is console
1 FILEHANDLE !                                        \ no filehandle allocated
0 >in !                                               \ reset offset
0 #tib !                                              \ reset length
0 RECERROR !                                          \ reset error count
1024 BUILD> tib 4 + DOES> !                           \ set size of input buffer
BUILD> OUTQUEUEBASE 1024 + DOES> tib !                \ set address of input buffer

-1 SETIMASK
16 0 DO QI LOOP ( clear all pending interrupts )

SERIALIN VAL 13 SETVECTOR CALL
SERIALOUT VAL 14 SETVECTOR CALL
TIMER3 VAL 15 SETVECTOR CALL
ILLEGAL VAL 0 SETVECTOR CALL
GETIMASK 1ffeH AND SETIMASK ( enable interrupt routine )

QUERY CALL ( reset input buffer )

0 LASTRECEIVED ! ( no command present )
1 EI ( enable interrupts )

KEY? CALL DROP ( test input buffer )
.( OK> )

( main loop of BIOS )
BEGIN
      1 LOCKI/O CALL
      DI DROP LASTRECEIVED @ 0 LASTRECEIVED ! 1 EI ( read and clear buffer )
      -1 LOCKI/O CALL
      CASE
      255 OF RESTART CALL ENDOF ( restart )
      254 OF 0 LATE ! LOAD CALL ENDOF ( load and execute )
      253 OF READDICTIONARY CALL ENDOF ( read out dictionary )
      252 OF KEY? CALL DROP ENDOF ( available capacity of input buffer )
      251 OF ABORT CALL ENDOF ( abort )
	250 OF REDEFINESIO CALL ENDOF ( redefine serial port )
	249 OF GETUNUSEDMEMORY CALL ENDOF ( return unused memory )
	248 OF ALLOCATEDYNAMIC CALL ENDOF ( allocate dynamic memory )
	247 OF RELEASEMM CALL ENDOF ( free dynamic memory )
	246 OF RESETDYNAMIC CALL ENDOF ( reset dynamic memory )
	245 OF UNLOAD CALL ENDOF ( unload module )
      244 OF LATELINK CALL ENDOF ( try to link error records )
      243 OF 80000000H LATE ! LOAD CALL ENDOF ( load and execute )
		ELSE
      ENDCASE
AGAIN

