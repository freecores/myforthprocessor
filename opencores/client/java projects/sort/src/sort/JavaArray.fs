MODULENAME equation.JavaArray
( *
* <p>Überschrift: </p>
* <p>Beschreibung: </p>
* <p>Copyright: Copyright (c) 2005</p>
* <p>Organisation: </p>
* @author unbekannt
* @version 1.0
)


:LOCAL equation.JavaArray.JavaArray§-2046277120
   
   \ new statement
   0
   LOCALS §this |
   
   \ new statement
   sort.Object§-1890496768.table 1827294976 EXECUTE-NEW
   
   \ new statement
   40 MALLOC DROP DUP TO §this OVER
   IF
      OVER CELL+ @ 4 + OVER OVER ! DROP
   ENDIF
   CELL+ @ OVER
   OVER ! NIP
   equation.JavaArray§1352878592.table OVER 12 + !
   1352859648 OVER 20 + !
   " JavaArray " OVER 16 + ! DROP
   
   \ new statement
   0
   §this CELL+ @ 24 + ( equation.JavaArray._dynamicBlocking )    !
   
   \ new statement
   \  length of array
   
   
   \ new statement
   \  ld(wordlength)
   
   
   \ new statement
   §this ( return object )    
   \ new statement
   0 §break217 LABEL
   
   \ new statement
   PURGE 1
   
   \ new statement
   DROP
;
\  reference to physical array
( *
* constructor
* @param length number of elements
* @param code lower 16 bit are the dimension, high part is the type
)

:LOCAL equation.JavaArray.JavaArray§-227194368
   LOCAL code
   LOCAL length
   
   \ new statement
   0 DUP
   LOCALS x §this |
   
   \ new statement
   sort.Object§-1890496768.table 1827294976 EXECUTE-NEW
   
   \ new statement
   40 MALLOC DROP DUP TO §this OVER
   IF
      OVER CELL+ @ 4 + OVER OVER ! DROP
   ENDIF
   CELL+ @ OVER
   OVER ! NIP
   equation.JavaArray§1352878592.table OVER 12 + !
   1352859648 OVER 20 + !
   " JavaArray " OVER 16 + ! DROP
   
   \ new statement
   0
   §this CELL+ @ 24 + ( equation.JavaArray._dynamicBlocking )    !
   
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
   OVER <> §§group_205 0BRANCH
   
   \ new statement
   2
   
   \ new statement
   OVER <> §§group_205 0BRANCH
   
   \ new statement
   5
   
   \ new statement
   OVER <> §§group_206 0BRANCH
   
   \ new statement
   6
   
   \ new statement
   OVER <> §§group_206 0BRANCH
   
   \ new statement
   §§group_207 BRANCH
   
   \ new statement
   DUP §§group_205 LABEL DROP
   
   \ new statement
   x
   1
   - DUP
   §this CELL+ @ 32 + ( equation.JavaArray.shift )    !
   TO x
   
   \ new statement
   0 §break204 BRANCH
   
   \ new statement
   DUP §§group_206 LABEL DROP
   
   \ new statement
   3
   DUP
   §this CELL+ @ 32 + ( equation.JavaArray.shift )    !
   TO x
   
   \ new statement
   0 §break204 BRANCH
   
   \ new statement
   DUP §§group_207 LABEL DROP
   
   \ new statement
   2
   DUP
   §this CELL+ @ 32 + ( equation.JavaArray.shift )    !
   TO x
   
   \ new statement
   0 §break204 LABEL
   
   \ new statement
   
   \ new statement
   0>!
   IF
      10001H - §break203 BRANCH
   ENDIF
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
   §this ( return object )    
   \ new statement
   0 §break203 LABEL
   
   \ new statement
   
   
   
   PURGE 4
   
   \ new statement
   DROP
;
( *
* clone an array
* @return a new reference to the array
)

:LOCAL equation.JavaArray.clone§1043948288
   LOCAL §this
   
   \ new statement
   0 DUP DUP
   LOCALS b code x |
   
   \ new statement
   §this CELL+ @ 20 + @ TO code
   
   \ new statement
   §this CELL+ @ 28 + ( equation.JavaArray.length )    @
   code
   equation.JavaArray§1352878592.table -227194368 EXECUTE-NEW
   b TRUE equation.JavaArray.kill§1620077312 TO b
   
   \ new statement
   b CELL+ @ 36 + ( equation.JavaArray.array )    @
   TO x
   
   \ new statement
   x DECREFERENCE
   
   \ new statement
   §this CELL+ @ 36 + ( equation.JavaArray.array )    @
   TO x
   
   \ new statement
   x INCREFERENCE
   
   \ new statement
   x
   b CELL+ @ 36 + ( equation.JavaArray.array )    !
   
   \ new statement
   b DUP INCREFERENCE
   0 §break208 BRANCH
   
   \ new statement
   0 §break208 LABEL
   
   \ new statement
   b TRUE equation.JavaArray.kill§1620077312
   
   
   PURGE 4
   
   \ new statement
   DROP
;
( *
* create an standard string
* @param length stringlength
* @param bytestring initial content
* @return an initialized string object
)

: equation.JavaArray.createString§-105880832
   LOCAL length
   LOCAL bytestring
   
   \ new statement
   0 DUP
   LOCALS body target |
   
   \ new statement
   length 65537 equation.JavaArray§1352878592.table -227194368 EXECUTE-NEW
   body TRUE equation.JavaArray.kill§1620077312 TO body
   
   \ new statement
   body
   A:R@
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
   body DUP INCREFERENCE
   equation.String§1651856128.table -2016390400 EXECUTE-NEW
   0 §break212 BRANCH
   
   \ new statement
   0 §break212 LABEL
   
   \ new statement
   body TRUE equation.JavaArray.kill§1620077312
   
   
   
   PURGE 4
   
   \ new statement
   DROP
;
( *
* create an unicode string
* @param length stringlength
* @param forthstring initial content
* @return an initialized string object
)

: equation.JavaArray.createUnicode§-675323136
   LOCAL length
   LOCAL forthstring
   
   \ new statement
   0 DUP
   LOCALS body target |
   
   \ new statement
   length
   2/ DUP
   TO length 131073 equation.JavaArray§1352878592.table -227194368 EXECUTE-NEW
   body TRUE equation.JavaArray.kill§1620077312 TO body
   
   \ new statement
   body
   A:R@
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
   body DUP INCREFERENCE
   equation.String§1651856128.table -99003648 EXECUTE-NEW
   0 §break213 BRANCH
   
   \ new statement
   0 §break213 LABEL
   
   \ new statement
   body TRUE equation.JavaArray.kill§1620077312
   
   
   
   PURGE 4
   
   \ new statement
   DROP
;
( *
* fetch an element of the array
* @param pos index
* @return a reference to the element
)

:LOCAL equation.JavaArray.getElem§-942971136
   LOCAL pos
   LOCAL §this
   
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
   < and_356 0BRANCH! DROP
   pos
   0< INVERT
   and_356 LABEL
   
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
   0 §break209 BRANCH
   
   \ new statement
   0 §break209 LABEL
   
   \ new statement
   
   
   
   
   PURGE 5
   
   \ new statement
   DROP
;
( *
* default exception handler
* @param e Exception
)

: equation.JavaArray.handler§-1096259584
   LOCAL e
   
   \ new statement
   
   \ new statement
   e A:R@
   R> 1695049472 TRUE ( java.lang.Exception.getMessage§1695049472 ) EXECUTE-METHOD
   equation.JavaArray.print§562786304
   
   \ new statement
   ABORT" program aborted "
   
   \ new statement
   0 §break216 LABEL
   
   \ new statement
   e TRUE equation.JavaArray.kill§1620077312
   PURGE 1
   
   \ new statement
   DROP
;
( *
* decrement (and remove) an object
* @param obj the object
* @param polymorph the complete object, if true, only this part otherwise
)

: equation.JavaArray.kill§1620077312
   LOCAL polymorph
   LOCAL obj
   
   \ new statement
   
   \ new statement
   obj HANDLEVALID
   
   \ new statement
   IF
      obj DUP -1086882304 polymorph EXECUTE-METHOD
   ENDIF
   
   \ new statement
   0 §break214 LABEL
   
   \ new statement
   PURGE 2
   
   \ new statement
   DROP
;
( *
* print unicode string to console
* @param s String
)

: equation.JavaArray.print§562786304
   LOCAL s
   
   \ new statement
   0 DUP
   LOCALS a length |
   
   \ new statement
   s A:R@
   R> 188050432 TRUE ( equation.String.length§188050432 ) EXECUTE-METHOD
   TO length
   
   \ new statement
   s CELL+ @ 28 + ( equation.String.value )    @ A:R@
   A:DROP
   A:R@
   0
   R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
   TO a
   
   \ new statement
   a length 2* PRINT
   
   \ new statement
   0 §break215 LABEL
   
   \ new statement
   
   
   s TRUE equation.JavaArray.kill§1620077312
   PURGE 3
   
   \ new statement
   DROP
;
( * destructor
)

:LOCAL equation.JavaArray.~destructor§-1086882304
   LOCAL §this
   
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
      equation.JavaArray.kill§1620077312
      
      \ new statement
      §this CELL+ @ 8 + @ TO obj
      
      \ new statement
      obj
      0
      equation.JavaArray.kill§1620077312
      
      \ new statement
      x @ TO c
      
      \ new statement
      c
      1
      = and_401 0BRANCH! DROP
      depth
      1
      >  0=! or_402 0BRANCH DROP
      hash
      6
      >
      or_402 LABEL
      and_401 LABEL
      
      \ new statement
      IF
         
         \ new statement
         
         \ new statement
         §this A:R@
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
            equation.JavaArray.kill§1620077312
            
            \ new statement
            CELL+
         LOOP
         DROP
         
         \ new statement
         0 §break211 LABEL
         
         \ new statement
         
         \ new statement
         0>!
         IF
            10001H - §break210 BRANCH
         ENDIF
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
   0 §break210 LABEL
   
   \ new statement
   PURGE 9
   
   \ new statement
   DROP
;
VARIABLE equation.JavaArray._staticBlocking

A:HERE VARIABLE equation.JavaArray§1352878592.table 5 DUP 2* CELLS ALLOT R@ ! A:CELL+
-2046277120 R@ ! A:CELL+ equation.JavaArray.JavaArray§-2046277120 VAL R@ ! A:CELL+
-227194368 R@ ! A:CELL+ equation.JavaArray.JavaArray§-227194368 VAL R@ ! A:CELL+
1043948288 R@ ! A:CELL+ equation.JavaArray.clone§1043948288 VAL R@ ! A:CELL+
-942971136 R@ ! A:CELL+ equation.JavaArray.getElem§-942971136 VAL R@ ! A:CELL+
-1086882304 R@ ! A:CELL+ equation.JavaArray.~destructor§-1086882304 VAL R@ ! A:CELL+
A:DROP
