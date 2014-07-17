MODULENAME equation.JavaArray
( *
* <p>Überschrift: </p>
* <p>Beschreibung: </p>
* <p>Copyright: Copyright (c) 2005</p>
* <p>Organisation: </p>
* @author unbekannt
* @version 1.0
)


:LOCAL JavaArray.JavaArray§-2046277120
   1 VALLOCATE LOCAL §base0
   
   \ new statement
   0
   LOCALS §this |
   
   \ new statement
   Object§-1890496768.table 1827294976 EXECUTE-NEW
   
   \ new statement
   40 MALLOC DROP DUP DUP §base0 V! TO §this OVER
   IF
      OVER CELL+ @ 4 + OVER OVER ! DROP
   ENDIF
   CELL+ @ OVER
   OVER ! NIP
   JavaArray§1352878592.table OVER 12 + !
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
   0 §break28178 LABEL
   
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

:LOCAL JavaArray.JavaArray§-227194368
   1 VALLOCATE LOCAL §base0
   LOCAL code
   LOCAL length
   
   \ new statement
   0 DUP
   LOCALS x §this |
   
   \ new statement
   Object§-1890496768.table 1827294976 EXECUTE-NEW
   
   \ new statement
   40 MALLOC DROP DUP DUP §base0 V! TO §this OVER
   IF
      OVER CELL+ @ 4 + OVER OVER ! DROP
   ENDIF
   CELL+ @ OVER
   OVER ! NIP
   JavaArray§1352878592.table OVER 12 + !
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
   OVER <> §§group_28165 0BRANCH
   
   \ new statement
   2
   
   \ new statement
   OVER <> §§group_28165 0BRANCH
   
   \ new statement
   5
   
   \ new statement
   OVER <> §§group_28166 0BRANCH
   
   \ new statement
   6
   
   \ new statement
   OVER <> §§group_28166 0BRANCH
   
   \ new statement
   §§group_28167 BRANCH
   
   \ new statement
   DUP §§group_28165 LABEL DROP
   
   \ new statement
   x
   1
   - DUP
   §this CELL+ @ 32 + ( equation.JavaArray.shift )    !
   TO x
   
   \ new statement
   0 §break28164 BRANCH
   
   \ new statement
   DUP §§group_28166 LABEL DROP
   
   \ new statement
   3
   DUP
   §this CELL+ @ 32 + ( equation.JavaArray.shift )    !
   TO x
   
   \ new statement
   0 §break28164 BRANCH
   
   \ new statement
   DUP §§group_28167 LABEL DROP
   
   \ new statement
   2
   DUP
   §this CELL+ @ 32 + ( equation.JavaArray.shift )    !
   TO x
   
   \ new statement
   0 §break28164 LABEL
   
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
   §this CELL+ @ 28 + ( equation.JavaArray.length )    !
   
   \ new statement
   code §this CELL+ @ 20 + !
   
   \ new statement
   \  overwrite code field
   
   
   \ new statement
   length x LSHIFT MALLOC DROP TO x
   
   \ new statement
   x
   §this CELL+ @ 36 + ( equation.JavaArray.array )    !
   
   \ new statement
   §this DUP 0 V! ( return object )    
   \ new statement
   0 §break28163 LABEL
   
   \ new statement
   
   
   
   §base0 SETVTOP
   PURGE 5
   
   \ new statement
   DROP
;
( *
* clone an array
* @return a new reference to the array
)

:LOCAL JavaArray.clone§1969054464
   2 VALLOCATE LOCAL §base0
   DUP 0 §base0 + V! LOCAL §this
   
   \ new statement
   0 DUP DUP
   LOCALS b code x |
   
   \ new statement
   §this CELL+ @ 20 + @ TO code
   
   \ new statement
   §this CELL+ @ 28 + ( equation.JavaArray.length )    @
   code
   JavaArray§1352878592.table -227194368 EXECUTE-NEW
   DUP 4 §base0 + V! TO b
   
   \ new statement
   b CELL+ @ 36 + ( equation.JavaArray.array )    @
   TO x
   
   \ new statement
   SWEEP @ 0=
   IF
      x DECREFERENCE
   ENDIF
   
   \ new statement
   §this CELL+ @ 36 + ( equation.JavaArray.array )    @
   TO x
   
   \ new statement
   SWEEP @ 0=
   IF
      x INCREFERENCE
   ENDIF
   
   \ new statement
   x
   b CELL+ @ 36 + ( equation.JavaArray.array )    !
   
   \ new statement
   b
   DUP 0 V!
   0 §break28168 BRANCH
   
   \ new statement
   0 §break28168 LABEL
   
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

: JavaArray.createString§-105880832
   1 VALLOCATE LOCAL §base0
   LOCAL length
   LOCAL bytestring
   
   \ new statement
   0 DUP
   LOCALS body target |
   
   \ new statement
   
   1 VALLOCATE LOCAL §tempvar
   length 65537 JavaArray§1352878592.table -227194368 EXECUTE-NEW
   DUP §base0 V! TO body
   
   §tempvar SETVTOP
   
   PURGE 1
   
   
   \ new statement
   body
   >R
   0
   R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
   TO target
   
   \ new statement
   bytestring target length 0
   ?DO
      OVER I + C@ OVER I + C!
   LOOP
   2DROP
   
   \ new statement
   body
   String§1651856128.table -2016390400 EXECUTE-NEW
   DUP 0 V!
   0 §break28173 BRANCH
   
   \ new statement
   0 §break28173 LABEL
   
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

: JavaArray.createUnicode§-675323136
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
   TO length 131073 JavaArray§1352878592.table -227194368 EXECUTE-NEW
   DUP §base0 V! TO body
   
   §tempvar SETVTOP
   
   PURGE 1
   
   
   \ new statement
   body
   >R
   0
   R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
   TO target
   
   \ new statement
   forthstring target length 2* 0
   ?DO
      OVER I + C@ OVER I + C!
   LOOP
   2DROP
   
   \ new statement
   body
   String§1651856128.table -99003648 EXECUTE-NEW
   DUP 0 V!
   0 §break28174 BRANCH
   
   \ new statement
   0 §break28174 LABEL
   
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

:LOCAL JavaArray.getElem§-942971136
   1 VALLOCATE LOCAL §base0
   DUP 0 §base0 + V! LOCAL §this
   LOCAL pos
   
   \ new statement
   0 DUP DUP
   LOCALS code shift x |
   
   \ new statement
   §this CELL+ @ 36 + ( equation.JavaArray.array )    @
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
      §this CELL+ @ 32 + ( equation.JavaArray.shift )       @
      
   ENDIF
   TO shift
   
   \ new statement
   pos
   §this CELL+ @ 28 + ( equation.JavaArray.length )    @
   < and_32750 0BRANCH! DROP
   pos
   0< INVERT
   and_32750 LABEL
   
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
   0 §break28170 BRANCH
   
   \ new statement
   0 §break28170 LABEL
   
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

: JavaArray.handler§-1096259584
   1 VALLOCATE LOCAL §base0
   DUP 0 §base0 + V! LOCAL e
   
   \ new statement
   
   \ new statement
   
   1 VALLOCATE LOCAL §tempvar
   e >R  " Throwable " CASTTO
   R> 1695049472 TRUE ( equation.Throwable.getMessage§1695049472 ) EXECUTE-METHOD DUP §tempvar V!
   JavaArray.print§562786304
   
   
   §tempvar SETVTOP
   
   PURGE 1
   
   
   \ new statement
   ABORT" program aborted "
   
   \ new statement
   0 §break28177 LABEL
   
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

: JavaArray.kill§1620077312
   1 VALLOCATE LOCAL §base0
   LOCAL polymorph
   DUP 0 §base0 + V! LOCAL obj
   
   \ new statement
   
   \ new statement
   obj HANDLEVALID
   
   \ new statement
   IF
      obj -1086882304 polymorph EXECUTE-METHOD
   ENDIF
   
   \ new statement
   0 §break28175 LABEL
   
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

: JavaArray.print§562786304
   1 VALLOCATE LOCAL §base0
   DUP 0 §base0 + V! LOCAL s
   
   \ new statement
   0 DUP
   LOCALS a length |
   
   \ new statement
   s >R
   R> 188050432 TRUE ( equation.String.length§188050432 ) EXECUTE-METHOD
   TO length
   
   \ new statement
   s CELL+ @ 28 + ( equation.String.value )    A:R@ @
   A:DROP
   >R
   0
   R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
   TO a
   
   \ new statement
   a length 2* PRINT
   
   \ new statement
   0 §break28176 LABEL
   
   \ new statement
   
   
   
   §base0 SETVTOP
   PURGE 4
   
   \ new statement
   DROP
;
( *
* resize an array
* @param size int
)

:LOCAL JavaArray.resize§917664256
   1 VALLOCATE LOCAL §base0
   DUP 0 §base0 + V! LOCAL §this
   LOCAL size
   
   \ new statement
   0 DUP DUP
   LOCALS code x y |
   
   \ new statement
   §this CELL+ @ 36 + ( equation.JavaArray.array )    @
   TO x
   
   \ new statement
   §this CELL+ @ 20 + @ TO code
   
   \ new statement
   \  load code field
   
   
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
      TO y
      
      \ new statement
   ELSE
      
      \ new statement
      §this CELL+ @ 32 + ( equation.JavaArray.shift )       @
      TO y
      
      \ new statement
   ENDIF
   
   \ new statement
   x size y LSHIFT RESIZE DROP TO x
   
   \ new statement
   x
   §this CELL+ @ 36 + ( equation.JavaArray.array )    !
   
   \ new statement
   0 §break28169 LABEL
   
   \ new statement
   
   
   
   
   §base0 SETVTOP
   PURGE 6
   
   \ new statement
   DROP
;
( * destructor
)

:LOCAL JavaArray.~destructor§-1086882304
   2 VALLOCATE LOCAL §base0
   DUP 0 §base0 + V! LOCAL §this
   
   \ new statement
   0 DUP DUP DUP DUP DUP DUP DUP
   LOCALS c code depth hash l obj x y |
   
   \ new statement
   §this CELL+ @ 32 + ( equation.JavaArray.shift )    @
   TO y
   
   \ new statement
   §this CELL+ @ 36 + ( equation.JavaArray.array )    @
   TO x
   
   \ new statement
   §this CELL+ @ 28 + ( equation.JavaArray.length )    @
   TO l
   
   \ new statement
   §this CELL+ @ 20 + @ TO code
   
   \ new statement
   code
   65535
   AND
   TO depth
   
   \ new statement
   code  S>D
   16
   SHIFTR  D>S
   TO hash
   
   \ new statement
   §this @ 1- 0=
   IF
      §this CELL+ @ @ TO obj
      
      \ new statement
      obj
      0
      JavaArray.kill§1620077312
      
      \ new statement
      §this CELL+ @ 8 + @ TO obj
      
      \ new statement
      obj
      0
      JavaArray.kill§1620077312
      
      \ new statement
      x @ TO c
      
      \ new statement
      c
      1
      = and_32810 0BRANCH! DROP
      depth
      1
      >  0=! or_32811 0BRANCH DROP
      hash
      6
      >
      or_32811 LABEL
      and_32810 LABEL
      
      \ new statement
      IF
         
         \ new statement
         
         \ new statement
         §this >R
         0
         R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
         TO x
         
         \ new statement
         x l 0
         ?DO
            DUP @ TO obj
            
            \ new statement
            obj
            -1
            JavaArray.kill§1620077312
            
            \ new statement
            CELL+
         LOOP
         DROP
         
         \ new statement
         0 §break28172 LABEL
         
         \ new statement
         
         \ new statement
         DROP
         
         \ new statement
      ENDIF
      
      \ new statement
      §this CELL+ @ 36 + ( equation.JavaArray.array )       @
      TO x
      
      \ new statement
      x FREE DROP
      
      \ new statement
   ENDIF
   §this DECREFERENCE
   
   \ new statement
   0 §break28171 LABEL
   
   \ new statement
   §base0 SETVTOP
   PURGE 10
   
   \ new statement
   DROP
;
VARIABLE JavaArray._staticBlocking
VARIABLE JavaArray._staticThread  HERE 4 - SALLOCATE

A:HERE VARIABLE JavaArray§1352878592.table 6 DUP 2* CELLS ALLOT R@ ! A:CELL+
-2046277120 R@ ! A:CELL+ JavaArray.JavaArray§-2046277120 VAL R@ ! A:CELL+
-227194368 R@ ! A:CELL+ JavaArray.JavaArray§-227194368 VAL R@ ! A:CELL+
1969054464 R@ ! A:CELL+ JavaArray.clone§1969054464 VAL R@ ! A:CELL+
-942971136 R@ ! A:CELL+ JavaArray.getElem§-942971136 VAL R@ ! A:CELL+
917664256 R@ ! A:CELL+ JavaArray.resize§917664256 VAL R@ ! A:CELL+
-1086882304 R@ ! A:CELL+ JavaArray.~destructor§-1086882304 VAL R@ ! A:CELL+
A:DROP
