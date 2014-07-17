( 
  BIOS for FORTH-System

  Copyright © 2004-2008 by Gerhard Hohner
)
  

DEFINE OPEN 0
DEFINE CREATEF 1
DEFINE READ 2
DEFINE LINE 3
DEFINE LISTDICT 4
DEFINE ATXY 5
DEFINE GETFILEPOS 6
DEFINE SETFILEPOS 7
DEFINE GETFILESIZE 8
DEFINE SETFILESIZE 9
DEFINE INCLUDEF 10
DEFINE CLEAR 11
DEFINE STATUS 12
DEFINE RENAME 13
DEFINE DELETE 14
DEFINE CLOSE 15

DEFINE ABSOLUTE 16
DEFINE RELATIVE 17
DEFINE SETDATE 18
DEFINE GETDATE 19

DEFINE READBUFFERED 22
DEFINE ASCII 23

( set directory flag
  parameter:
  return:
      fam
)

: DIR 8 ;

( change access method to binary mode
  parameter:
      fam
  return:
      modified fam
)

: BIN 1 OR ( mode bit 0 indicates binary ) ;

( file access methode for write only file
  parameter:
  return:
      fam
)

: W/O 4 ;

( file access methode for read only file
  parameter:
  return:
      fam
)

: R/O 2 ;

( file access methode for read/write file
  parameter:
  return:
      fam
)

: R/W 6 ;

( wait, till master quits
  parameter:
  return:
      0, if accepted, true otherwise
)

:LOCAL WAIT DI 0 DUP SENTALL C! RECERROR ! EI
            BEGIN SENTALL C@ 0<> UNTIL ( wait, till quit )
            DI SENTALL C@ 26 - NEGATE RECERROR @ + 0 DUP SENTALL C! RECERROR ! OVER EI NIP ; 

( allocate a file handle
  parameter:
  return:
      handle, if available, otherwise -1
)

:LOCAL ALLOCATEHANDLE 14 1 DO FILEHANDLE @ 1 I LSHIFT OVER OVER AND 0= IF OR FILEHANDLE ! I UNLOOP EXIT ENDIF 2DROP LOOP -1 ;


( close associated file
  parameter:
      fileID
  return:
      0, if successful
)

: CLOSE-FILE A:DI 1 EI 15 OUTQUEUEAPPENDRAWCHAR CALL ( API )
             CLOSE OUTQUEUEAPPENDMASKEDCHAR CALL ( close )
             A:R@ OUTQUEUEAPPENDMASKEDCHAR CALL ( fileID )
             26 OUTQUEUEAPPENDRAWCHAR CALL
             WAIT CALL 0=!
             IF
                A:2/ A:2/ A:2/ A:2/
                1 R@ LSHIFT INVERT 1 OR
                A:FILEHANDLE R@ @
                AND R@ !
                A:DROP
             ENDIF
             A:DROP A:EI ;

( open/create a distinct file
  parameter:
      open/createf
      fam
      length of name
      address of name
  return:
      0, if successful
      fileID
)

:LOCAL OPEN_ A:DI 1 EI A:R@ DROP	( move operation )
             ALLOCATEHANDLE CALL
             CELLS CELLS OR ( build fileID )
             0<! INVERT
             IF
                A:R@ ( save fileID )
                15 OUTQUEUEAPPENDRAWCHAR CALL ( API )
                R1@ OUTQUEUEAPPENDMASKEDCHAR CALL ( open or createf )
                OUTQUEUEAPPENDMASKEDCHAR CALL ( fileID )
                OUTQUEUEAPPENDARRAY CALL ( name )
                26 OUTQUEUEAPPENDRAWCHAR CALL
                R@ A:2DROP
                WAIT CALL
             ELSE
                A:DROP NIP NIP DUP
             ENDIF
             A:EI ;

( create or recreate a file for output
  parameter:
      fam
      length of name
      address of name
  return:
      0, if succesfull
      fileID
)

: CREATE-FILE W/O CALL OR ( set output )
              CREATEF
              OPEN_ CALL ;

( open a distinct file
  parameter:
      fam
      length of name
      address of name
  return:
      0, if successful
      fileID
)

: OPEN-FILE OPEN
            OPEN_ CALL ;

( delete a named file
  parameter:
      length of name
      address of name
  return:
      0, if successful
)

: DELETE-FILE A:DI 1 EI 15 OUTQUEUEAPPENDRAWCHAR CALL ( API )
              DELETE OUTQUEUEAPPENDMASKEDCHAR CALL ( delete )
              OUTQUEUEAPPENDARRAY CALL ( name )
              26 OUTQUEUEAPPENDRAWCHAR CALL
              WAIT CALL A:EI ; 

( get current file position
  parameter:
      fileID
  return:
      0, if successful
      double integer
)

: FILE-POSITION A:DI 1 EI 15 OUTQUEUEAPPENDRAWCHAR CALL ( API )
                GETFILEPOS OUTQUEUEAPPENDMASKEDCHAR CALL ( get current file position )
                OUTQUEUEAPPENDMASKEDCHAR CALL ( fileID )
                26 OUTQUEUEAPPENDRAWCHAR CALL
                HERE @ ALIGNED 8 0f0H SETBUFFER CALL DROP WAIT CALL ( receive position )
                A:R@ DROP HERE @ ALIGNED 2@ R@ A:DROP ( onto stack )
                A:EI ;

( get current file size
  parameter:
      fileID
  return:
      0, if successful
      double integer
)

: FILE-SIZE A:DI 1 EI 15 OUTQUEUEAPPENDRAWCHAR CALL ( API )
            GETFILESIZE OUTQUEUEAPPENDMASKEDCHAR CALL ( get current file size )
            OUTQUEUEAPPENDMASKEDCHAR CALL ( fileID )
            26 OUTQUEUEAPPENDRAWCHAR CALL
            HERE @ ALIGNED 8 0f0H SETBUFFER CALL DROP WAIT CALL ( receive size )
            A:R@ DROP HERE @ ALIGNED 2@ R@ A:DROP ( onto stack )
            A:EI ;


( include a file, that is already open
  parameter:
      fileID
      alternative fileID
  return:
      new fileID
)

: INCLUDE-FILE DROP ;

( standard input will come from this file
  parameter:
      length of name
      address of name
  return:
      0, if successful
)

: INCLUDED A:DI 1 EI 15 OUTQUEUEAPPENDRAWCHAR CALL ( API )
           INCLUDEF OUTQUEUEAPPENDMASKEDCHAR CALL ( include )
           OUTQUEUEAPPENDARRAY CALL ( name )
           26 OUTQUEUEAPPENDRAWCHAR CALL
           WAIT CALL A:EI ;

( set receiver buffer
  parameter:
      fileID
      bufferlength
      buffer address
  return:
      pointer to received length
)

:LOCAL SETBUFFER A:R1@ A:R@
                 0f0H AND INPUTFILEBASE + A:R@
                 CELL+ ! ( size )
                 R@ ! ( buffer address )
                 A:CELL+ A:CELL+
                 0 R@ ( offset )
                 OVER A:CELL+ R@ ! ! ( length )
                 R@ A:DROP A:2DROP ;
            
( get address of buffer
  parameter:
      number
  return:
      address
)

: BLOCK 15 AND CELLS INPUTFILEBASE + @ ;

( read from a file
  parameter:
      fileID
      bufferlength
      buffer address
  return:
      0, if successful
      number of characters read
)

: READ-FILE A:DI 1 EI A:R1@ A:R@
            SETBUFFER CALL A:R@ DROP
            15 OUTQUEUEAPPENDRAWCHAR CALL ( API )
            READ OUTQUEUEAPPENDMASKEDCHAR CALL ( read )
            R1@ A:NIP OUTQUEUEAPPENDMASKEDCHAR CALL ( fileID )
            R1@ A:NIP SENDWORD CALL ( length )
            26 OUTQUEUEAPPENDRAWCHAR CALL
            WAIT CALL
            A:R@ DROP R1@ @ R@ A:2DROP A:EI ;

( read from buffer
  parameter:
      bufferlength
  return:
      0, if successful
      number of characters read
      buffer address
)

:LOCAL READ-BUFFER A:DI >R HERE @ ALIGNED R@ 0F0H SETBUFFER CALL >R
                   15 OUTQUEUEAPPENDRAWCHAR CALL ( API )
                   READBUFFERED OUTQUEUEAPPENDMASKEDCHAR CALL ( read buffer )
                   240 OUTQUEUEAPPENDMASKEDCHAR CALL ( fileID )
                   R1@ A:NIP SENDWORD CALL ( length )
                   1 EI 
                   26 OUTQUEUEAPPENDRAWCHAR CALL
                   WAIT CALL
                   >R HERE @ ALIGNED R1@ @ R@ A:2DROP A:EI ;
                   

( read a line from a file
  parameter:
      fileID
      bufferlength
      buffer address
  return:
      0, if successful
      number of characters read
)

: READ-LINE A:DI 1 EI A:R1@ A:R@
            0f0H AND INPUTFILEBASE + A:R@
            CELL+ ! ( size )
            R@ ! ( buffer address )
            A:CELL+ A:CELL+
            0 R@ ( offset )
            OVER A:CELL+ R@ ! ! ( length )
            R@ A:DROP
            15 OUTQUEUEAPPENDRAWCHAR CALL ( API )
            LINE OUTQUEUEAPPENDMASKEDCHAR CALL ( read )
            R@ OUTQUEUEAPPENDMASKEDCHAR CALL ( fileID )
            R1@ SENDWORD CALL ( length )
            26 OUTQUEUEAPPENDRAWCHAR CALL
            A:2DROP
            WAIT CALL
            A:R@ DROP @ R@ A:DROP A:EI ;

:LOCAL SENDWORD DUP OUTQUEUEAPPENDMASKEDCHAR CALL ( low word )
                8 RSHIFT DUP OUTQUEUEAPPENDMASKEDCHAR CALL
                8 RSHIFT DUP OUTQUEUEAPPENDMASKEDCHAR CALL
                8 RSHIFT OUTQUEUEAPPENDMASKEDCHAR CALL ;

:LOCAL SENDLONG SENDWORD CALL SENDWORD CALL ;

( set file pointer
  parameter:
      fileID
      double integer position
  return:
      0, if successful
)

: REPOSITION-FILE A:DI 1 EI 15 OUTQUEUEAPPENDRAWCHAR CALL ( API )
                  SETFILEPOS OUTQUEUEAPPENDMASKEDCHAR CALL ( set current file position )
                  OUTQUEUEAPPENDMASKEDCHAR CALL ( fileID )
                  SENDLONG CALL
                  26 OUTQUEUEAPPENDRAWCHAR CALL
                  WAIT CALL A:EI ;


( set size of file
  parameter:
      fileID
      double integer size
  return:
      0, if successful
)

: RESIZE-FILE A:DI 1 EI 15 OUTQUEUEAPPENDRAWCHAR CALL ( API )
              SETFILESIZE OUTQUEUEAPPENDMASKEDCHAR CALL ( set current file size )
              OUTQUEUEAPPENDMASKEDCHAR CALL ( fileID )
              SENDLONG CALL
              26 OUTQUEUEAPPENDRAWCHAR CALL
              WAIT CALL A:EI ;

( set date of file
  parameter:
      length of name
      address of name
      double integer date
  return:
      0, if successful
)

: SET-FILEDATE A:DI 1 EI 15 OUTQUEUEAPPENDRAWCHAR CALL ( API )
               SETDATE OUTQUEUEAPPENDMASKEDCHAR CALL ( set current file date )
               OUTQUEUEAPPENDARRAY CALL ( name )
               SENDLONG CALL
               26 OUTQUEUEAPPENDRAWCHAR CALL
               WAIT CALL A:EI ;

( write to a file
  parameter:
      fileID
      bufferlength
      buffer address
  return:
      0, if successful
)

: WRITE-FILE A:DI 1 EI 2/ 2/ 2/ 2/ OUTQUEUEAPPENDRAWCHAR CALL ( fileID )
             OUTQUEUEAPPENDARRAY CALL
             26 OUTQUEUEAPPENDRAWCHAR CALL
             A:EI 0 ; 

( write a line to a file
  parameter:
      fileID
      bufferlength
      buffer address
  return:
      0, if successful
)

: WRITE-LINE A:DI 1 EI 2/ 2/ 2/ 2/ OUTQUEUEAPPENDRAWCHAR CALL ( fileID )
             OUTQUEUEAPPENDARRAY CALL
             13 OUTQUEUEAPPENDMASKEDCHAR CALL
             10 OUTQUEUEAPPENDMASKEDCHAR CALL
             26 OUTQUEUEAPPENDRAWCHAR CALL
             A:EI 0 ; 


( get file status
  parameter:
      length of name
      address of name
  return:
      0, if successful
      status
)

: FILE-STATUS A:DI 1 EI 15 OUTQUEUEAPPENDRAWCHAR CALL ( API )
              STATUS OUTQUEUEAPPENDMASKEDCHAR CALL ( get current file status )
              OUTQUEUEAPPENDARRAY CALL ( name )
              26 OUTQUEUEAPPENDRAWCHAR CALL
              HERE @ ALIGNED 4 0f0H SETBUFFER CALL DROP WAIT CALL ( receive status )
              A:R@ DROP HERE @ ALIGNED @ R@ A:DROP ( onto stack )
              A:EI ;

( get file date
  parameter:
      length of name
      address of name
  return:
      0, if successful
      long date
)

: GET-FILEDATE A:DI 1 EI 15 OUTQUEUEAPPENDRAWCHAR CALL ( API )
               GETDATE OUTQUEUEAPPENDMASKEDCHAR CALL ( get current file date )
               OUTQUEUEAPPENDARRAY CALL ( name )
               26 OUTQUEUEAPPENDRAWCHAR CALL
               HERE @ ALIGNED 8 0f0H SETBUFFER CALL DROP WAIT CALL ( receive date )
               A:R> A:HERE A:@ A:ALIGNED R@ CELL+ @ R@ @ R1@ A:2DROP ( onto stack )
               A:EI ;

( flush file
  parameter:
      fileID
  return:
      0, if successful
)

: FLUSH-FILE DROP 0 ;

( rename a file
  parameter:
      length of new name
      address of new name
      length of old name
      address of old name
  return:
      0, if successful
)

: RENAME-FILE A:DI 1 EI 15 OUTQUEUEAPPENDRAWCHAR CALL ( API )
              RENAME OUTQUEUEAPPENDMASKEDCHAR CALL ( rename )
              A:R1@ A:R@ 2DROP
              DUP OUTQUEUEAPPENDMASKEDCHAR CALL ( length )
              DUP 8 RSHIFT OUTQUEUEAPPENDMASKEDCHAR CALL
              OUTQUEUEAPPENDARRAY CALL ( old name )
              R1@ R@ A:2DROP
              DUP OUTQUEUEAPPENDMASKEDCHAR CALL ( length )
              DUP 8 RSHIFT OUTQUEUEAPPENDMASKEDCHAR CALL
              OUTQUEUEAPPENDARRAY CALL ( new name )
              26 OUTQUEUEAPPENDRAWCHAR CALL
              WAIT CALL A:EI ; 

:LOCAL ABSOLUTE-RELATIVE A:DI 1 EI 15 OUTQUEUEAPPENDRAWCHAR CALL ( API )
                         OUTQUEUEAPPENDMASKEDCHAR CALL ( absolute/relative )
                         DUP OUTQUEUEAPPENDMASKEDCHAR CALL ( length )
                         DUP 8 RSHIFT OUTQUEUEAPPENDMASKEDCHAR CALL
                         OUTQUEUEAPPENDARRAY CALL ( name )
                         A:R1@ 0f0H SETBUFFER CALL A:R>
                         26 OUTQUEUEAPPENDRAWCHAR CALL
                         WAIT CALL ( receive position )
                         R1@ A:NIP A:R1@ NIP R1@ @ R@
                         A:2DROP A:EI ;

( get absolute filename
  parameter:
      length of name
      address of name
      length of buffer
      buffer address
  return:
      0, if successful
      length of buffer
      buffer address
)

: ABSOLUTE-FILE ABSOLUTE ABSOLUTE-RELATIVE CALL ;

( get entries from directory
  parameter:
      length of name
      address of name
      length of buffer
      buffer address
  return:
      0, if successful
      length of buffer
      buffer address
)

: ENTRIES-FILE RELATIVE ABSOLUTE-RELATIVE CALL ;

( send dictionary as list to host
  parameter:
  return:
)

: READDICTIONARY A:DI 1 EI 15 OUTQUEUEAPPENDRAWCHAR CALL ( API )
                 LISTDICT OUTQUEUEAPPENDMASKEDCHAR CALL ( list dictionary )
                 DICTBASE DICTTOP @ 
                 ?DO I @ DUP COUNT HALF+ NIP OUTQUEUEAPPENDARRAY CALL I CELL+ DUP 4 OUTQUEUEAPPENDARRAY CALL CELL+ 4 OUTQUEUEAPPENDARRAY CALL 3 CELLS +LOOP
                 26 OUTQUEUEAPPENDRAWCHAR CALL A:EI ;

( clear screen
  parameter:
  return:
)

: PAGE A:DI 1 EI 15 OUTQUEUEAPPENDRAWCHAR CALL ( API )
       CLEAR OUTQUEUEAPPENDMASKEDCHAR CALL ( clear )
       26 OUTQUEUEAPPENDRAWCHAR CALL A:EI ;

( set cursor
  parameter:
      row
      column
  return:
)

: AT-XY A:DI 1 EI 15 OUTQUEUEAPPENDRAWCHAR CALL ( API )
        ATXY OUTQUEUEAPPENDMASKEDCHAR CALL ( set cursor )
        OUTQUEUEAPPENDMASKEDCHAR CALL ( row )
        OUTQUEUEAPPENDMASKEDCHAR CALL ( column )
        26 OUTQUEUEAPPENDRAWCHAR CALL A:EI ;

( toggle character set
  parameter:
      boolean   true for ASCII, false for Unicode
  return:
)

: CHANGECHARSET A:DI 1 EI 15 OUTQUEUEAPPENDRAWCHAR CALL ( API )
                ASCII OUTQUEUEAPPENDMASKEDCHAR CALL ( change character set )
		    OUTQUEUEAPPENDMASKEDCHAR CALL
                26 OUTQUEUEAPPENDRAWCHAR CALL A:EI ;
