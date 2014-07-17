MODULENAME java.lang.JavaArray
( *
* <p>Überschrift: </p>
* <p>Beschreibung: </p>
* <p>Copyright: Copyright (c) 2005</p>
* <p>Organisation: </p>
* @author unbekannt
* @version 1.0
)


:LOCAL lang.JavaArray.JavaArray§-2046277120
   1 VALLOCATE LOCAL §base0
   
   \ new statement
   0
   LOCALS §this |
   
   \ new statement
   lang.Object§-1890496768.table 1827294976 EXECUTE-NEW
   
   \ new statement
   40 MALLOC DROP DUP DUP §base0 V! TO §this OVER
   IF
      OVER CELL+ @ 4 + OVER OVER ! DROP
   ENDIF
   CELL+ @ OVER
   OVER ! NIP
   lang.JavaArray§1352878592.table OVER 12 + !
   1352859648 OVER 20 + !
   " JavaArray " OVER 16 + !
   0 OVER 24 + ! DROP
   
   \ new statement
   \  length of array
   
   
   \ new statement
   \  ld(wordlength)
   
   
   \ new statement
   §this DUP 0 V! ( return object )    
   \ new statement
   0 §break17530 LABEL
   
   \ new statement
   §base0 SETVTOP
   PURGE 2
   
   \ new statement
   DROP
;
\  reference to physical array
( *
* constructor
* @param length number of elements
* @param code lower 16 bit are the dimension, high part is the type
)

:LOCAL lang.JavaArray.JavaArray§-227194368
   1 VALLOCATE LOCAL §base0
   LOCAL code
   LOCAL length
   
   \ new statement
   0 DUP
   LOCALS x §this |
   
   \ new statement
   lang.Object§-1890496768.table 1827294976 EXECUTE-NEW
   
   \ new statement
   40 MALLOC DROP DUP DUP §base0 V! TO §this OVER
   IF
      OVER CELL+ @ 4 + OVER OVER ! DROP
   ENDIF
   CELL+ @ OVER
   OVER ! NIP
   lang.JavaArray§1352878592.table OVER 12 + !
   1352859648 OVER 20 + !
   " JavaArray " OVER 16 + !
   0 OVER 24 + ! DROP
   
   \ new statement
   \  length of array
   
   
   \ new statement
   \  ld(wordlength)
   
   
   \ new statement
   code  S>D
   16
   SHIFTR  D>S
   TO x
   
   \ new statement
   
   \ new statement
   x
   
   \ new statement
   1
   
   \ new statement
   OVER <> §§group_17520 0BRANCH
   
   \ new statement
   2
   
   \ new statement
   OVER <> §§group_17520 0BRANCH
   
   \ new statement
   5
   
   \ new statement
   OVER <> §§group_17521 0BRANCH
   
   \ new statement
   6
   
   \ new statement
   OVER <> §§group_17521 0BRANCH
   
   \ new statement
   §§group_17522 BRANCH
   
   \ new statement
   DUP §§group_17520 LABEL DROP
   
   \ new statement
   x
   1
   - DUP
   §this CELL+ @ 32 + ( java.lang.JavaArray.shift )    !
   TO x
   
   \ new statement
   0 §break17519 BRANCH
   
   \ new statement
   DUP §§group_17521 LABEL DROP
   
   \ new statement
   3
   DUP
   §this CELL+ @ 32 + ( java.lang.JavaArray.shift )    !
   TO x
   
   \ new statement
   0 §break17519 BRANCH
   
   \ new statement
   DUP §§group_17522 LABEL DROP
   
   \ new statement
   2
   DUP
   §this CELL+ @ 32 + ( java.lang.JavaArray.shift )    !
   TO x
   
   \ new statement
   0 §break17519 LABEL
   
   \ new statement
   
   \ new statement
   DROP
   
   \ new statement
   code
   65535
   AND
   1
   >
   
   \ new statement
   IF
      
      \ new statement
      2
      TO x
      
      \ new statement
   ENDIF
   
   \ new statement
   length
   §this CELL+ @ 28 + ( java.lang.JavaArray.length )    !
   
   \ new statement
   code §this CELL+ @ 20 + !
   
   \ new statement
   \  overwrite code field
   
   
   \ new statement
   length x LSHIFT MALLOC DROP TO x
   
   \ new statement
   x
   §this CELL+ @ 36 + ( java.lang.JavaArray.array )    !
   
   \ new statement
   §this DUP 0 V! ( return object )    
   \ new statement
   0 §break17518 LABEL
   
   \ new statement
   
   
   
   §base0 SETVTOP
   PURGE 5
   
   \ new statement
   DROP
;


: lang.JavaArray.classMonitorEnter§-433167616
   
   \ new statement
   
   \ new statement
   0 §break17516 LABEL
   
   \ new statement
   
   \ new statement
   DROP
;


: lang.JavaArray.classMonitorLeave§-1691393280
   
   \ new statement
   
   \ new statement
   0 §break17517 LABEL
   
   \ new statement
   
   \ new statement
   DROP
;
( *
* clone an array
* @return a new reference to the array
)

:LOCAL lang.JavaArray.clone§1043948288
   2 VALLOCATE LOCAL §base0
   DUP 0 §base0 + V! LOCAL §this
   
   \ new statement
   0 DUP DUP
   LOCALS b code x |
   
   \ new statement
   §this CELL+ @ 20 + @ TO code
   
   \ new statement
   §this CELL+ @ 28 + ( java.lang.JavaArray.length )    @
   code
   lang.JavaArray§1352878592.table -227194368 EXECUTE-NEW
   DUP 4 §base0 + V! TO b
   
   \ new statement
   b CELL+ @ 36 + ( java.lang.JavaArray.array )    @
   TO x
   
   \ new statement
   §this CELL+ @ 36 + ( java.lang.JavaArray.array )    @
   TO x
   
   \ new statement
   x
   b CELL+ @ 36 + ( java.lang.JavaArray.array )    !
   
   \ new statement
   b
   DUP 0 V!0 §break17523 BRANCH
   
   \ new statement
   0 §break17523 LABEL
   
   \ new statement
   
   
   
   §base0 SETVTOP
   PURGE 5
   
   \ new statement
   DROP
;
( *
* create an standard string
* @param length stringlength
* @param bytestring initial content
* @return an initialized string object
)

: lang.JavaArray.createString§-105880832
   1 VALLOCATE LOCAL §base0
   LOCAL length
   LOCAL bytestring
   
   \ new statement
   0 DUP
   LOCALS body target |
   
   \ new statement
   
   1 VALLOCATE LOCAL §tempvar
   length 65537 lang.JavaArray§1352878592.table -227194368 EXECUTE-NEW
   DUP §base0 V! TO body
   
   §tempvar SETVTOP
   
   PURGE 1
   
   
   \ new statement
   body
   A:R@
   0
   R> -942971136 TRUE ( java.lang.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
   TO target
   
   \ new statement
   bytestring target length 0
   ?DO
      OVER I + C@ OVER I + C!
   LOOP
   2DROP
   
   \ new statement
   body
   lang.String§1651856128.table -2016390400 EXECUTE-NEW
   DUP 0 V!0 §break17525 BRANCH
   
   \ new statement
   0 §break17525 LABEL
   
   \ new statement
   
   
   
   
   §base0 SETVTOP
   PURGE 5
   
   \ new statement
   DROP
;
( *
* create an unicode string
* @param length stringlength
* @param forthstring initial content
* @return an initialized string object
)

: lang.JavaArray.createUnicode§-675323136
   1 VALLOCATE LOCAL §base0
   LOCAL length
   LOCAL forthstring
   
   \ new statement
   0 DUP
   LOCALS body target |
   
   \ new statement
   
   1 VALLOCATE LOCAL §tempvar
   length
   2/ DUP
   TO length 131073 lang.JavaArray§1352878592.table -227194368 EXECUTE-NEW
   DUP §base0 V! TO body
   
   §tempvar SETVTOP
   
   PURGE 1
   
   
   \ new statement
   body
   A:R@
   0
   R> -942971136 TRUE ( java.lang.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
   TO target
   
   \ new statement
   forthstring target length 2* 0
   ?DO
      OVER I + C@ OVER I + C!
   LOOP
   2DROP
   
   \ new statement
   body
   lang.String§1651856128.table -99003648 EXECUTE-NEW
   DUP 0 V!0 §break17526 BRANCH
   
   \ new statement
   0 §break17526 LABEL
   
   \ new statement
   
   
   
   
   §base0 SETVTOP
   PURGE 5
   
   \ new statement
   DROP
;
( *
* fetch an element of the array
* @param pos index
* @return a reference to the element
)

:LOCAL lang.JavaArray.getElem§-942971136
   1 VALLOCATE LOCAL §base0
   LOCAL pos
   DUP 0 §base0 + V! LOCAL §this
   
   \ new statement
   0 DUP DUP
   LOCALS code shift x |
   
   \ new statement
   §this CELL+ @ 36 + ( java.lang.JavaArray.array )    @
   TO x
   
   \ new statement
   x CELL+ @ TO x
   
   \ new statement
   §this CELL+ @ 20 + @ TO code
   
   \ new statement
   code
   65535
   AND
   1
   >
   
   IF
      2
      
   ELSE
      §this CELL+ @ 32 + ( java.lang.JavaArray.shift )       @
      
   ENDIF
   TO shift
   
   \ new statement
   pos
   §this CELL+ @ 28 + ( java.lang.JavaArray.length )    @
   < and_17621 0BRANCH! DROP
   pos
   0< INVERT
   and_17621 LABEL
   
   \ new statement
   IF
      
      \ new statement
      x pos shift LSHIFT + TO x
      
      \ new statement
   ELSE
      
      \ new statement
      pos
      0<>
      
      \ new statement
      IF
         
         \ new statement
         S" index out of range:  " TYPE pos . CR
         
         \ new statement
      ENDIF
      
      \ new statement
   ENDIF
   
   \ new statement
   x
   0 §break17524 BRANCH
   
   \ new statement
   0 §break17524 LABEL
   
   \ new statement
   
   
   
   
   §base0 SETVTOP
   PURGE 6
   
   \ new statement
   DROP
;
( *
* default exception handler
* @param e Exception
)

: lang.JavaArray.handler§-1096259584
   1 VALLOCATE LOCAL §base0
   DUP 0 §base0 + V! LOCAL e
   
   \ new statement
   
   \ new statement
   
   1 VALLOCATE LOCAL §tempvar
   e A:R@  " Throwable " CASTTO
   R> 1695049472 TRUE ( java.lang.Throwable.getMessage§1695049472 ) EXECUTE-METHOD DUP §tempvar V!
   lang.JavaArray.print§562786304
   
   
   §tempvar SETVTOP
   
   PURGE 1
   
   
   \ new statement
   ABORT" program aborted "
   
   \ new statement
   0 §break17529 LABEL
   
   \ new statement
   
   §base0 SETVTOP
   PURGE 2
   
   \ new statement
   DROP
;
( *
* decrement (and remove) an object
* @param obj the object
* @param polymorph the complete object, if true, only this part otherwise
)

: lang.JavaArray.kill§1620077312
   1 VALLOCATE LOCAL §base0
   LOCAL polymorph
   DUP 0 §base0 + V! LOCAL obj
   
   \ new statement
   
   \ new statement
   obj HANDLEVALID
   
   \ new statement
   IF
      obj DUP -1086882304 polymorph EXECUTE-METHOD
   ENDIF
   
   \ new statement
   0 §break17527 LABEL
   
   \ new statement
   §base0 SETVTOP
   PURGE 3
   
   \ new statement
   DROP
;
( *
* print unicode string to console
* @param s String
)

: lang.JavaArray.print§562786304
   1 VALLOCATE LOCAL §base0
   DUP 0 §base0 + V! LOCAL s
   
   \ new statement
   0 DUP
   LOCALS a length |
   
   \ new statement
   s A:R@
   R> 188050432 TRUE ( java.lang.String.length§188050432 ) EXECUTE-METHOD
   TO length
   
   \ new statement
   s CELL+ @ 28 + ( java.lang.String.value )    @ A:R@
   A:DROP
   A:R@
   0
   R> -942971136 TRUE ( java.lang.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
   TO a
   
   \ new statement
   a length 2* PRINT
   
   \ new statement
   0 §break17528 LABEL
   
   \ new statement
   
   
   
   §base0 SETVTOP
   PURGE 4
   
   \ new statement
   DROP
;
VARIABLE lang.JavaArray._staticBlocking
VARIABLE lang.JavaArray._staticThread

A:HERE VARIABLE lang.JavaArray§1352878592.table 4 DUP 2* CELLS ALLOT R@ ! A:CELL+
-2046277120 R@ ! A:CELL+ lang.JavaArray.JavaArray§-2046277120 VAL R@ ! A:CELL+
-227194368 R@ ! A:CELL+ lang.JavaArray.JavaArray§-227194368 VAL R@ ! A:CELL+
1043948288 R@ ! A:CELL+ lang.JavaArray.clone§1043948288 VAL R@ ! A:CELL+
-942971136 R@ ! A:CELL+ lang.JavaArray.getElem§-942971136 VAL R@ ! A:CELL+
A:DROP
