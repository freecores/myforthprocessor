MODULENAME equation.Equation
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


:LOCAL Equation.Equation§-1254013696
   
   \ new statement
   0
   LOCALS §this |
   
   \ new statement
   Object§-1890496768.table 1827294976 EXECUTE-NEW
   
   \ new statement
   28 MALLOC DROP DUP TO §this OVER
   IF
      OVER CELL+ @ 4 + OVER OVER ! DROP
   ENDIF
   CELL+ @ OVER
   OVER ! NIP
   Equation§-1560460032.table OVER 12 + !
   -1560477696 OVER 20 + !
   " Equation " OVER 16 + !
   0 OVER 24 + ! DROP
   
   \ new statement
   §this ( return object )    
   \ new statement
   0 §break19 LABEL
   
   \ new statement
   PURGE 1
   
   \ new statement
   DROP
;


: Equation.classMonitorEnter§-433167616
   
   \ new statement
   
   \ new statement
   0 §break0 LABEL
   
   \ new statement
   
   \ new statement
   DROP
;


: Equation.classMonitorLeave§-1691393280
   
   \ new statement
   
   \ new statement
   0 §break1 LABEL
   
   \ new statement
   
   \ new statement
   DROP
;


: Equation.fillRow§1526621696
   LOCAL line
   LOCAL row
   
   \ new statement
   
   \ new statement
   BasicIO.newLine§-283087360
   
   \ new statement
   0 DUP
   LOCALS count i |
   
   \ new statement
   0
   TO i
   
   \ new statement
   BEGIN
      
      \ new statement
      i
      row CELL+ @ 28 + ( equation.JavaArray.length )       @
      <
      
      \ new statement
   WHILE
      
      \ new statement
      line A:R@
      R> -1067748352 TRUE ( equation.String.trim§-1067748352 ) EXECUTE-METHOD
      line TRUE JavaArray.kill§1620077312 TO line
      
      \ new statement
      0
      TO count
      
      \ new statement
      
      \ new statement
      BEGIN
         
         \ new statement
         count
         line A:R@
         R> 188050432 TRUE ( equation.String.length§188050432 ) EXECUTE-METHOD
         < and_2698 0BRANCH! DROP
         line A:R@
         count
         R> -1014275328 TRUE ( equation.String.charAt§-1014275328 ) EXECUTE-METHOD
         32
         >
         and_2698 LABEL
         
         \ new statement
      WHILE
         
         \ new statement
         count 1+ TO count
         
         \ new statement
         
         \ new statement
      REPEAT
      
      \ new statement
      0 §break5 LABEL
      
      \ new statement
      
      \ new statement
      DROP
      
      \ new statement
      
      0
      LOCALS 0§ |
      
      i TO 0§
      line A:R@
      0
      count
      R> -424054016 TRUE ( equation.String.substring§-424054016 ) EXECUTE-METHOD
      std2706 VAL
      Double.parseDouble§1159491584
      row A:R@
      0§
      R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
      2! FALSE DUP
      IF
         std2706 LABEL TRUE
      ENDIF
      
      PURGE 1
      
      IF
         JavaArray.handler§-1096259584
      ENDIF
      
      \ new statement
      line A:R@
      count
      R> 18576128 TRUE ( equation.String.substring§18576128 ) EXECUTE-METHOD
      line TRUE JavaArray.kill§1620077312 TO line
      
      \ new statement
      
      \ new statement
      i 1+ TO i
      
      \ new statement
   REPEAT
   
   \ new statement
   0 §break3 LABEL
   
   \ new statement
   
   
   PURGE 2
   
   \ new statement
   DROP
   
   \ new statement
   0 §break2 LABEL
   
   \ new statement
   line TRUE JavaArray.kill§1620077312
   row TRUE JavaArray.kill§1620077312
   PURGE 2
   
   \ new statement
   DROP
;


: Equation.main§505244928
   LOCAL args
   
   \ new statement
   0 DUP DUP DUP DUP
   LOCALS b blank count line mat |
   
   \ new statement
   U" Enter coeffizients of first row of equation\n " COUNT JavaArray.createUnicode§-675323136
   BasicIO.print§562786304
   
   \ new statement
   BasicIO.readLine§-939691520
   line TRUE JavaArray.kill§1620077312 TO line
   
   \ new statement
   0
   TO count
   
   \ new statement
   TRUE
   TO blank
   
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
      line A:R@
      R> 188050432 TRUE ( equation.String.length§188050432 ) EXECUTE-METHOD
      <
      
      \ new statement
   WHILE
      
      \ new statement
      line A:R@
      i
      R> -1014275328 TRUE ( equation.String.charAt§-1014275328 ) EXECUTE-METHOD
      32
      > INVERT
      
      \ new statement
      IF
         
         \ new statement
         TRUE
         TO blank
         
         \ new statement
      ELSE
         
         \ new statement
         blank
         
         \ new statement
         IF
            
            \ new statement
            
            \ new statement
            FALSE
            TO blank
            
            \ new statement
            count 1+ TO count
            
            \ new statement
            0 §break8 LABEL
            
            \ new statement
            
            \ new statement
            DROP
            
            \ new statement
         ENDIF
         
         \ new statement
      ENDIF
      
      \ new statement
      
      \ new statement
      i 1+ TO i
      
      \ new statement
   REPEAT
   
   \ new statement
   0 §break7 LABEL
   
   \ new statement
   
   PURGE 1
   
   \ new statement
   DROP
   
   \ new statement
   count A:R@ 393218 JavaArray§1352878592.table -227194368 EXECUTE-NEW R> 0
   DO
      count 393217 JavaArray§1352878592.table -227194368 EXECUTE-NEW
      OVER I  OVER -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD  !
   LOOP
   mat TRUE JavaArray.kill§1620077312 TO mat
   
   \ new statement
   count 393217 JavaArray§1352878592.table -227194368 EXECUTE-NEW
   b TRUE JavaArray.kill§1620077312 TO b
   
   \ new statement
   mat A:R@
   0
   R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
   @ DUP INCREFERENCE
   line DUP INCREFERENCE
   Equation.fillRow§1526621696
   
   \ new statement
   
   \ new statement
   1
   TO count
   
   \ new statement
   BEGIN
      
      \ new statement
      count
      mat CELL+ @ 28 + ( equation.JavaArray.length )       @
      <
      
      \ new statement
   WHILE
      
      \ new statement
      
      0 0 0 0
      LOCALS 3§ 2§ 1§ 0§ |
      
      U" Enter coeffizients of  " COUNT JavaArray.createUnicode§-675323136 DUP TO 0§
      count
      1
      +
      <# 0 ..R JavaArray.createString§-105880832  DUP TO 1§
      DUP INCREFERENCE  OVER  -1461427456 TRUE ( equation.String.concat§-1461427456 ) EXECUTE-METHOD DUP TO 2§
      U" . row of equation\n " COUNT JavaArray.createUnicode§-675323136 DUP TO 3§
      DUP INCREFERENCE  OVER  -1461427456 TRUE ( equation.String.concat§-1461427456 ) EXECUTE-METHOD
      BasicIO.print§562786304
      3§ TRUE JavaArray.kill§1620077312
      2§ TRUE JavaArray.kill§1620077312
      1§ TRUE JavaArray.kill§1620077312
      0§ TRUE JavaArray.kill§1620077312
      
      PURGE 4
      
      
      \ new statement
      mat A:R@
      count
      R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
      @ DUP INCREFERENCE
      BasicIO.readLine§-939691520
      Equation.fillRow§1526621696
      
      \ new statement
      
      \ new statement
      count 1+ TO count
      
      \ new statement
   REPEAT
   
   \ new statement
   0 §break9 LABEL
   
   \ new statement
   
   \ new statement
   DROP
   
   \ new statement
   0
   LOCALS g |
   
   \ new statement
   mat DUP INCREFERENCE
   std2764 VAL
   Gauss§-1354217728.table 926369536 EXECUTE-NEW
   g TRUE JavaArray.kill§1620077312 TO g FALSE DUP
   IF
      std2764 LABEL TRUE
   ENDIF
   IF
      0 §break11 BRANCH
   ENDIF
   
   \ new statement
   U" Enter right vector or empty line\n " COUNT JavaArray.createUnicode§-675323136
   BasicIO.print§562786304
   
   \ new statement
   BasicIO.readLine§-939691520
   line TRUE JavaArray.kill§1620077312 TO line
   
   \ new statement
   
   \ new statement
   BEGIN
      
      \ new statement
      line A:R@
      R> 188050432 TRUE ( equation.String.length§188050432 ) EXECUTE-METHOD
      0>
      
      \ new statement
   WHILE
      
      \ new statement
      b DUP INCREFERENCE
      line DUP INCREFERENCE
      Equation.fillRow§1526621696
      
      \ new statement
      g A:R@
      b DUP INCREFERENCE
      std2772 VAL
      R> -766545152 TRUE ( equation.Gauss.solve§-766545152 ) EXECUTE-METHOD
      b TRUE JavaArray.kill§1620077312 TO b FALSE DUP
      IF
         std2772 LABEL TRUE
      ENDIF
      IF
         65537 §break12 BRANCH
      ENDIF
      
      \ new statement
      U" Solution:  " COUNT JavaArray.createUnicode§-675323136
      BasicIO.print§562786304
      
      \ new statement
      
      \ new statement
      0
      TO count
      
      \ new statement
      BEGIN
         
         \ new statement
         count
         b CELL+ @ 28 + ( equation.JavaArray.length )          @
         <
         
         \ new statement
      WHILE
         
         \ new statement
         
         0 0
         LOCALS 1§ 0§ |
         
         b A:R@
         count
         R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
         2@  F.. JavaArray.createString§-105880832  DUP TO 1§
         U"   " COUNT JavaArray.createUnicode§-675323136 DUP TO 0§
         DUP INCREFERENCE  OVER  -1461427456 TRUE ( equation.String.concat§-1461427456 ) EXECUTE-METHOD
         BasicIO.print§562786304
         1§ TRUE JavaArray.kill§1620077312
         0§ TRUE JavaArray.kill§1620077312
         
         PURGE 2
         
         
         \ new statement
         
         \ new statement
         count 1+ TO count
         
         \ new statement
      REPEAT
      
      \ new statement
      0 §break14 LABEL
      
      \ new statement
      
      \ new statement
      0>!
      IF
         10001H - 0<! §break12 0BRANCH
      ENDIF
      DROP
      
      \ new statement
      BasicIO.newLine§-283087360
      
      \ new statement
      U" Enter right vector or empty line\n " COUNT JavaArray.createUnicode§-675323136
      BasicIO.print§562786304
      
      \ new statement
      BasicIO.readLine§-939691520
      line TRUE JavaArray.kill§1620077312 TO line
      
      \ new statement
      
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
   0 §except15 LABEL
   
   \ new statement
   0 §break11 LABEL
   
   \ new statement
   g TRUE JavaArray.kill§1620077312
   PURGE 1
   
   \ new statement
   DROP
   
   \ new statement
   DUP " Exception " INSTANCEOF
   IF
      
      \ new statement
      0
      LOCALS e |
      
      \ new statement
      TO e
      
      \ new statement
      e A:R@
      R> 1695049472 TRUE ( equation.Exception.getMessage§1695049472 ) EXECUTE-METHOD
      BasicIO.print§562786304
      
      \ new statement
      0 §break17 LABEL
      
      \ new statement
      e TRUE JavaArray.kill§1620077312
      PURGE 1
      
      \ new statement
      DROP
      
      \ new statement
      DUP §out16 BRANCH
   ENDIF
   
   \ new statement
   §out16 LABEL DROP
   
   \ new statement
   0 §break6 LABEL
   
   \ new statement
   args TRUE JavaArray.kill§1620077312
   b TRUE JavaArray.kill§1620077312
   
   
   line TRUE JavaArray.kill§1620077312
   mat TRUE JavaArray.kill§1620077312
   PURGE 6
   
   \ new statement
   DROP
;


:LOCAL Equation.~destructor§-1086882304
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
      JavaArray.kill§1620077312
      
      \ new statement
      §this CELL+ @ 8 +  @ TO obj
      
      \ new statement
      obj
      0
      JavaArray.kill§1620077312
      
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
VARIABLE Equation._staticBlocking
VARIABLE Equation._staticThread

A:HERE VARIABLE Equation§-1560460032.table 2 DUP 2* CELLS ALLOT R@ ! A:CELL+
-1254013696 R@ ! A:CELL+ Equation.Equation§-1254013696 VAL R@ ! A:CELL+
-1086882304 R@ ! A:CELL+ Equation.~destructor§-1086882304 VAL R@ ! A:CELL+
A:DROP
