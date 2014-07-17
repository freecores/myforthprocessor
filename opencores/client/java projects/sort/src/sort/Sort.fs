MODULENAME sort.Sort
( *
* <p>Überschrift: </p>
*
* <p>Beschreibung: </p>
*
* <p>Copyright: Copyright (c) 2005</p>
*
* <p>Organisation: </p>
*
* @author unbekannt
* @version 1.0
)


:LOCAL sort.Sort.Sort§-1474145536
   
   \ new statement
   0
   LOCALS §this |
   
   \ new statement
   sort.Object§-1890496768.table 1827294976 EXECUTE-NEW
   
   \ new statement
   28 MALLOC DROP DUP TO §this OVER
   IF
      OVER CELL+ @ 4 + OVER OVER ! DROP
   ENDIF
   CELL+ @ OVER
   OVER ! NIP
   sort.Sort§1172329216.table OVER 12 + !
   1172307968 OVER 20 + !
   " Sort " OVER 16 + ! DROP
   
   \ new statement
   0
   §this CELL+ @ 24 + ( sort.Sort._dynamicBlocking )    !
   
   \ new statement
   §this ( return object )    
   \ new statement
   0 §break0 LABEL
   
   \ new statement
   PURGE 1
   
   \ new statement
   DROP
;


:LOCAL sort.Sort.bubble§-1514315264
   LOCAL s
   LOCAL §this
   
   \ new statement
   0 DUP DUP
   LOCALS ch goon start |
   
   \ new statement
   s A:R@
   R> 629044224 TRUE ( equation.String.toCharArray§629044224 ) EXECUTE-METHOD
   ch TRUE equation.JavaArray.kill§1620077312 TO ch
   
   \ new statement
   TRUE
   TO goon
   
   \ new statement
   0
   TO start
   
   \ new statement
   
   \ new statement
   BEGIN
      
      \ new statement
      FALSE
      TO goon
      
      \ new statement
      U" Iteration  " COUNT equation.JavaArray.createUnicode§-675323136
      sort.BasicIO.print§562786304
      
      \ new statement
      start 1+ DUP TO start
      sort.BasicIO.print§-1271697408
      
      \ new statement
      U" : " COUNT equation.JavaArray.createUnicode§-675323136
      sort.BasicIO.print§562786304
      
      \ new statement
      ch DUP INCREFERENCE
      equation.String§1651856128.table -99003648 EXECUTE-NEW
      sort.BasicIO.print§562786304
      
      \ new statement
      sort.BasicIO.newLine§-283087360
      
      \ new statement
      0
      LOCALS i |
      
      \ new statement
      0
      TO i
      
      \ new statement
      BEGIN
         
         \ new statement
         i
         ch CELL+ @ 28 + ( equation.JavaArray.length )          @
         start
         -
         <
         
         \ new statement
      WHILE
         
         \ new statement
         ch A:R@
         i
         R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
         H@
         ch A:R@
         i
         1
         +
         R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
         H@
         >
         
         \ new statement
         IF
            
            \ new statement
            0
            LOCALS x |
            
            \ new statement
            ch A:R@
            i
            R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
            H@
            TO x
            
            \ new statement
            
            0
            LOCALS 0§ |
            
            i TO 0§
            ch A:R@
            i
            1
            +
            R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
            H@
            ch A:R@
            0§
            R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
            H!
            
            PURGE 1
            
            
            \ new statement
            
            0
            LOCALS 0§ |
            
            i
            1
            + TO 0§
            x
            ch A:R@
            0§
            R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
            H!
            
            PURGE 1
            
            
            \ new statement
            TRUE
            TO goon
            
            \ new statement
            0 §break5 LABEL
            
            \ new statement
            
            PURGE 1
            
            \ new statement
            0>!
            IF
               10001H - 0<! §break4 0BRANCH §continue4 BRANCH
            ENDIF
            DROP
            
            \ new statement
         ENDIF
         
         \ new statement
         DUP §continue4 LABEL DROP
         
         \ new statement
         i 1+ TO i
         
         \ new statement
      REPEAT
      
      \ new statement
      0 §break4 LABEL
      
      \ new statement
      
      PURGE 1
      
      \ new statement
      0>!
      IF
         10001H - 0<! §break2 0BRANCH §continue2 BRANCH
      ENDIF
      DROP
      
      \ new statement
      DUP §continue2 LABEL DROP
      
      \ new statement
      goon
      
      \ new statement
      0=
   UNTIL
   
   \ new statement
   0 §break2 LABEL
   
   \ new statement
   
   \ new statement
   0>!
   IF
      10001H - §break1 BRANCH
   ENDIF
   DROP
   
   \ new statement
   0 §break1 LABEL
   
   \ new statement
   ch TRUE equation.JavaArray.kill§1620077312
   
   s TRUE equation.JavaArray.kill§1620077312
   
   PURGE 5
   
   \ new statement
   DROP
;


: sort.Sort.main§505244928
   LOCAL args
   
   \ new statement
   0
   LOCALS sort |
   
   \ new statement
   sort.Sort§1172329216.table -1474145536 EXECUTE-NEW
   sort TRUE equation.JavaArray.kill§1620077312 TO sort
   
   \ new statement
   args A:R@
   0
   R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
   @  " String " CASTTO A:R@
   0
   R> -1014275328 TRUE ( equation.String.charAt§-1014275328 ) EXECUTE-METHOD
   98
   =
   
   \ new statement
   IF
      
      \ new statement
      sort A:R@
      args A:R@
      1
      R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
      @  " String " CASTTO DUP INCREFERENCE
      R> -1514315264 TRUE ( sort.Sort.bubble§-1514315264 ) EXECUTE-METHOD
      
      \ new statement
   ELSE
      
      \ new statement
      0
      LOCALS ch |
      
      \ new statement
      args A:R@
      1
      R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
      @  " String " CASTTO A:R@
      R> 629044224 TRUE ( equation.String.toCharArray§629044224 ) EXECUTE-METHOD
      ch TRUE equation.JavaArray.kill§1620077312 TO ch
      
      \ new statement
      sort A:R@
      ch DUP INCREFERENCE
      0
      args A:R@
      1
      R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
      @  " String " CASTTO A:R@
      R> 188050432 TRUE ( equation.String.length§188050432 ) EXECUTE-METHOD
      0
      R> 1454207232 TRUE ( sort.Sort.quick§1454207232 ) EXECUTE-METHOD
      
      \ new statement
      U" Sorted:  " COUNT equation.JavaArray.createUnicode§-675323136
      sort.BasicIO.print§562786304
      
      \ new statement
      ch DUP INCREFERENCE
      equation.String§1651856128.table -99003648 EXECUTE-NEW
      sort.BasicIO.print§562786304
      
      \ new statement
      sort.BasicIO.newLine§-283087360
      
      \ new statement
      0 §break18 LABEL
      
      \ new statement
      ch TRUE equation.JavaArray.kill§1620077312
      PURGE 1
      
      \ new statement
      0>!
      IF
         10001H - §break17 BRANCH
      ENDIF
      DROP
      
      \ new statement
   ENDIF
   
   \ new statement
   0 §break17 LABEL
   
   \ new statement
   args TRUE equation.JavaArray.kill§1620077312
   sort TRUE equation.JavaArray.kill§1620077312
   PURGE 2
   
   \ new statement
   DROP
;


:LOCAL sort.Sort.quick§1454207232
   LOCAL iteration
   LOCAL length
   LOCAL offset
   LOCAL ch
   LOCAL §this
   
   \ new statement
   
   \ new statement
   U" Depth  " COUNT equation.JavaArray.createUnicode§-675323136
   sort.BasicIO.print§562786304
   
   \ new statement
   iteration
   sort.BasicIO.print§-1271697408
   
   \ new statement
   U" : " COUNT equation.JavaArray.createUnicode§-675323136
   sort.BasicIO.print§562786304
   
   \ new statement
   ch DUP INCREFERENCE
   equation.String§1651856128.table -99003648 EXECUTE-NEW
   sort.BasicIO.print§562786304
   
   \ new statement
   sort.BasicIO.newLine§-283087360
   
   \ new statement
   ch A:R@
   offset
   R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
   H@
   ch A:R@
   offset
   length
   +
   1
   -
   R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
   H@
   >
   
   \ new statement
   IF
      
      \ new statement
      0
      LOCALS x |
      
      \ new statement
      ch A:R@
      offset
      R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
      H@
      TO x
      
      \ new statement
      
      0
      LOCALS 0§ |
      
      offset TO 0§
      ch A:R@
      offset
      length
      +
      1
      -
      R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
      H@
      ch A:R@
      0§
      R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
      H!
      
      PURGE 1
      
      
      \ new statement
      
      0
      LOCALS 0§ |
      
      offset
      length
      +
      1
      - TO 0§
      x
      ch A:R@
      0§
      R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
      H!
      
      PURGE 1
      
      
      \ new statement
      0 §break7 LABEL
      
      \ new statement
      
      PURGE 1
      
      \ new statement
      0>!
      IF
         10001H - §break6 BRANCH
      ENDIF
      DROP
      
      \ new statement
   ENDIF
   
   \ new statement
   length
   2
   >
   
   \ new statement
   IF
      
      \ new statement
      0
      LOCALS mid |
      
      \ new statement
      offset
      length
      2/
      +
      TO mid
      
      \ new statement
      ch A:R@
      offset
      R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
      H@
      ch A:R@
      mid
      R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
      H@
      >
      
      \ new statement
      IF
         
         \ new statement
         0
         LOCALS x |
         
         \ new statement
         ch A:R@
         offset
         R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
         H@
         TO x
         
         \ new statement
         
         0
         LOCALS 0§ |
         
         offset TO 0§
         ch A:R@
         mid
         R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
         H@
         ch A:R@
         0§
         R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
         H!
         
         PURGE 1
         
         
         \ new statement
         
         0
         LOCALS 0§ |
         
         mid TO 0§
         x
         ch A:R@
         0§
         R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
         H!
         
         PURGE 1
         
         
         \ new statement
         0 §break9 LABEL
         
         \ new statement
         
         PURGE 1
         
         \ new statement
         0>!
         IF
            10001H - §break8 BRANCH
         ENDIF
         DROP
         
         \ new statement
      ELSE
         
         \ new statement
         ch A:R@
         mid
         R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
         H@
         ch A:R@
         offset
         length
         +
         1
         -
         R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
         H@
         >
         
         \ new statement
         IF
            
            \ new statement
            0
            LOCALS x |
            
            \ new statement
            ch A:R@
            mid
            R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
            H@
            TO x
            
            \ new statement
            
            0
            LOCALS 0§ |
            
            mid TO 0§
            ch A:R@
            offset
            length
            +
            1
            -
            R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
            H@
            ch A:R@
            0§
            R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
            H!
            
            PURGE 1
            
            
            \ new statement
            
            0
            LOCALS 0§ |
            
            offset
            length
            +
            1
            - TO 0§
            x
            ch A:R@
            0§
            R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
            H!
            
            PURGE 1
            
            
            \ new statement
            0 §break10 LABEL
            
            \ new statement
            
            PURGE 1
            
            \ new statement
            0>!
            IF
               10001H - §break8 BRANCH
            ENDIF
            DROP
            
            \ new statement
         ENDIF
         
         \ new statement
      ENDIF
      
      \ new statement
      length
      3
      >
      
      \ new statement
      IF
         
         \ new statement
         0 DUP DUP DUP
         LOCALS end goon start x |
         
         \ new statement
         offset
         TO start
         
         \ new statement
         offset
         length
         +
         1
         -
         TO end
         
         \ new statement
         ch A:R@
         mid
         R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
         H@
         TO x
         
         \ new statement
         FALSE
         TO goon
         
         \ new statement
         
         \ new statement
         BEGIN
            
            \ new statement
            start
            end
            > INVERT
            
            \ new statement
         WHILE
            
            \ new statement
            
            \ new statement
            BEGIN
               
               \ new statement
               ch A:R@
               start
               R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
               H@
               x
               <
               
               \ new statement
            WHILE
               
               \ new statement
               start 1+ TO start
               
               \ new statement
               DUP §continue14 LABEL DROP
               
               \ new statement
            REPEAT
            
            \ new statement
            0 §break14 LABEL
            
            \ new statement
            
            \ new statement
            0>!
            IF
               10001H - 0<! §break12 0BRANCH §continue12 BRANCH
            ENDIF
            DROP
            
            \ new statement
            
            \ new statement
            BEGIN
               
               \ new statement
               ch A:R@
               end
               R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
               H@
               x
               >
               
               \ new statement
            WHILE
               
               \ new statement
               end 1- TO end
               
               \ new statement
               DUP §continue15 LABEL DROP
               
               \ new statement
            REPEAT
            
            \ new statement
            0 §break15 LABEL
            
            \ new statement
            
            \ new statement
            0>!
            IF
               10001H - 0<! §break12 0BRANCH §continue12 BRANCH
            ENDIF
            DROP
            
            \ new statement
            start
            end
            > INVERT
            
            \ new statement
            IF
               
               \ new statement
               0
               LOCALS y |
               
               \ new statement
               goon
               start
               end
               <
               OR
               TO goon
               
               \ new statement
               ch A:R@
               start
               R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
               H@
               TO y
               
               \ new statement
               
               0
               LOCALS 0§ |
               
               start DUP 1+ TO start
               TO 0§
               ch A:R@
               end
               R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
               H@
               ch A:R@
               0§
               R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
               H!
               
               PURGE 1
               
               
               \ new statement
               
               0
               LOCALS 0§ |
               
               end DUP 1- TO end
               TO 0§
               y
               ch A:R@
               0§
               R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
               H!
               
               PURGE 1
               
               
               \ new statement
               0 §break16 LABEL
               
               \ new statement
               
               PURGE 1
               
               \ new statement
               0>!
               IF
                  10001H - 0<! §break12 0BRANCH §continue12 BRANCH
               ENDIF
               DROP
               
               \ new statement
            ENDIF
            
            \ new statement
            DUP §continue12 LABEL DROP
            
            \ new statement
         REPEAT
         
         \ new statement
         0 §break12 LABEL
         
         \ new statement
         
         \ new statement
         0>!
         IF
            10001H - §break11 BRANCH
         ENDIF
         DROP
         
         \ new statement
         offset
         end
         < and_2204 0BRANCH! DROP
         goon
         and_2204 LABEL
         
         \ new statement
         IF
            
            \ new statement
            §this A:R@
            ch DUP INCREFERENCE
            offset
            end
            offset
            -
            1
            +
            iteration
            1
            +
            R> 1454207232 TRUE ( sort.Sort.quick§1454207232 ) EXECUTE-METHOD
            
            \ new statement
         ENDIF
         
         \ new statement
         end
         1
         +
         length
         offset
         +
         < and_2209 0BRANCH! DROP
         goon
         and_2209 LABEL
         
         \ new statement
         IF
            
            \ new statement
            §this A:R@
            ch DUP INCREFERENCE
            end
            1
            +
            length
            end
            1
            +
            offset
            -
            -
            iteration
            1
            +
            R> 1454207232 TRUE ( sort.Sort.quick§1454207232 ) EXECUTE-METHOD
            
            \ new statement
         ENDIF
         
         \ new statement
         0 §break11 LABEL
         
         \ new statement
         
         
         
         
         PURGE 4
         
         \ new statement
         0>!
         IF
            10001H - §break8 BRANCH
         ENDIF
         DROP
         
         \ new statement
      ENDIF
      
      \ new statement
      0 §break8 LABEL
      
      \ new statement
      
      PURGE 1
      
      \ new statement
      0>!
      IF
         10001H - §break6 BRANCH
      ENDIF
      DROP
      
      \ new statement
   ENDIF
   
   \ new statement
   0 §break6 LABEL
   
   \ new statement
   ch TRUE equation.JavaArray.kill§1620077312
   
   
   
   PURGE 5
   
   \ new statement
   DROP
;


:LOCAL sort.Sort.~destructor§-1086882304
   LOCAL §this
   
   \ new statement
   0
   LOCALS obj |
   
   \ new statement
   §this @ 1- 0=
   IF
      §this CELL+ @  @ TO obj
      
      \ new statement
      obj
      0
      equation.JavaArray.kill§1620077312
      
      \ new statement
      §this CELL+ @ 8 +  @ TO obj
      
      \ new statement
      obj
      0
      equation.JavaArray.kill§1620077312
      
      \ new statement
   ENDIF
   §this DECREFERENCE
   
   \ new statement
   0 §break20 LABEL
   
   \ new statement
   PURGE 2
   
   \ new statement
   DROP
;
VARIABLE sort.Sort._staticBlocking

A:HERE VARIABLE sort.Sort§1172329216.table 4 DUP 2* CELLS ALLOT R@ ! A:CELL+
-1474145536 R@ ! A:CELL+ sort.Sort.Sort§-1474145536 VAL R@ ! A:CELL+
-1514315264 R@ ! A:CELL+ sort.Sort.bubble§-1514315264 VAL R@ ! A:CELL+
1454207232 R@ ! A:CELL+ sort.Sort.quick§1454207232 VAL R@ ! A:CELL+
-1086882304 R@ ! A:CELL+ sort.Sort.~destructor§-1086882304 VAL R@ ! A:CELL+
A:DROP
