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
   1 VALLOCATE LOCAL §base0
   
   \ new statement
   0
   LOCALS §this |
   
   \ new statement
   Object§-1890496768.table 1827294976 EXECUTE-NEW
   
   \ new statement
   28 MALLOC DROP DUP DUP §base0 V! TO §this OVER
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
   §this DUP 0 V! ( return object )    
   \ new statement
   0 §break28145 LABEL
   
   \ new statement
   §base0 SETVTOP
   PURGE 2
   
   \ new statement
   DROP
;


: Equation.fillRow§1526621696
   2 VALLOCATE LOCAL §base0
   DUP 0 §base0 + V! LOCAL line
   DUP 4 §base0 + V! LOCAL row
   
   \ new statement
   0 DUP
   LOCALS end st |
   
   \ new statement
   0
   TO end
   
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
      row CELL+ @ 28 + ( equation.JavaArray.length )       @
      <
      
      \ new statement
   WHILE
      
      \ new statement
      
      \ new statement
      end
      TO st
      
      \ new statement
      BEGIN
         
         \ new statement
         st
         line >R
         R> 188050432 TRUE ( equation.String.length§188050432 ) EXECUTE-METHOD
         < and_33597 0BRANCH! DROP
         line >R
         st
         R> -1014275328 TRUE ( equation.String.charAt§-1014275328 ) EXECUTE-METHOD
         32
         > INVERT
         and_33597 LABEL
         
         \ new statement
      WHILE
         
         \ new statement
         
         \ new statement
         st 1+ TO st
         
         \ new statement
      REPEAT
      
      \ new statement
      0 §break28129 LABEL
      
      \ new statement
      
      \ new statement
      DROP
      
      \ new statement
      
      \ new statement
      st
      TO end
      
      \ new statement
      BEGIN
         
         \ new statement
         end
         line >R
         R> 188050432 TRUE ( equation.String.length§188050432 ) EXECUTE-METHOD
         < and_33609 0BRANCH! DROP
         line >R
         end
         R> -1014275328 TRUE ( equation.String.charAt§-1014275328 ) EXECUTE-METHOD
         32
         >
         and_33609 LABEL
         
         \ new statement
      WHILE
         
         \ new statement
         
         \ new statement
         end 1+ TO end
         
         \ new statement
      REPEAT
      
      \ new statement
      0 §break28130 LABEL
      
      \ new statement
      
      \ new statement
      DROP
      
      \ new statement
      
      1 VALLOCATE LOCAL §tempvar
      
      0
      LOCALS  0§ |
      
      i TO 0§
      line >R
      st
      end
      R> -424054016 TRUE ( equation.String.substring§-424054016 ) EXECUTE-METHOD DUP §tempvar V!
      std33617 VAL
      Double.parseDouble§1159491584
      row >R
      0§
      R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
      2! FALSE DUP
      IF
         std33617 LABEL TRUE
      ENDIF
      
      §tempvar SETVTOP
      
      PURGE 2
      
      IF
         JavaArray.handler§-1096259584
      ENDIF
      
      \ new statement
      
      \ new statement
      i 1+ TO i
      
      \ new statement
   REPEAT
   
   \ new statement
   0 §break28127 LABEL
   
   \ new statement
   
   PURGE 1
   
   \ new statement
   DROP
   
   \ new statement
   0 §break28126 LABEL
   
   \ new statement
   
   
   
   
   §base0 SETVTOP
   PURGE 5
   
   \ new statement
   DROP
;


: Equation.main§505244928
   4 VALLOCATE LOCAL §base0
   DUP 0 §base0 + V! LOCAL args
   
   \ new statement
   0 DUP DUP DUP DUP
   LOCALS b blank count line mat |
   
   \ new statement
   
   1 VALLOCATE LOCAL §tempvar
   U" Enter coeffizients of first row of equation\n " COUNT JavaArray.createUnicode§-675323136 DUP §tempvar V!
   BasicIO.print§562786304
   
   
   §tempvar SETVTOP
   
   PURGE 1
   
   
   \ new statement
   BasicIO.readLine§-939691520
   DUP 4 §base0 + V! TO line
   
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
      line >R
      R> 188050432 TRUE ( equation.String.length§188050432 ) EXECUTE-METHOD
      <
      
      \ new statement
   WHILE
      
      \ new statement
      line >R
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
            0 §break28133 LABEL
            
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
   0 §break28132 LABEL
   
   \ new statement
   
   PURGE 1
   
   \ new statement
   DROP
   
   \ new statement
   
   1 VALLOCATE LOCAL §tempvar
   count A:DI A:R@ 393218 JavaArray§1352878592.table -227194368 EXECUTE-NEW DUP §tempvar V! R> 0
   DO
      count 393217 JavaArray§1352878592.table -227194368 EXECUTE-NEW
      OVER I  SWAP -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD  !
   LOOP
   A:EI
   DUP 8 §base0 + V! TO mat
   
   §tempvar SETVTOP
   
   PURGE 1
   
   
   \ new statement
   
   1 VALLOCATE LOCAL §tempvar
   count 393217 JavaArray§1352878592.table -227194368 EXECUTE-NEW
   DUP 12 §base0 + V! TO b
   
   §tempvar SETVTOP
   
   PURGE 1
   
   
   \ new statement
   mat >R
   0
   R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
   @
   line
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
      
      4 VALLOCATE LOCAL §tempvar
      U" Enter coeffizients of  " COUNT JavaArray.createUnicode§-675323136 DUP §tempvar V!
      count
      1
      +
      <# 0 ..R JavaArray.createString§-105880832 DUP §tempvar 4 + V!
      SWAP  -1461427456 TRUE ( equation.String.concat§-1461427456 ) EXECUTE-METHOD DUP §tempvar 8 + V!
      U" . row of equation\n " COUNT JavaArray.createUnicode§-675323136 DUP §tempvar 12 + V!
      SWAP  -1461427456 TRUE ( equation.String.concat§-1461427456 ) EXECUTE-METHOD
      BasicIO.print§562786304
      
      
      
      
      
      §tempvar SETVTOP
      
      PURGE 1
      
      
      \ new statement
      
      1 VALLOCATE LOCAL §tempvar
      mat >R
      count
      R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
      @
      BasicIO.readLine§-939691520 DUP §tempvar V!
      Equation.fillRow§1526621696
      
      
      §tempvar SETVTOP
      
      PURGE 1
      
      
      \ new statement
      
      \ new statement
      count 1+ TO count
      
      \ new statement
   REPEAT
   
   \ new statement
   0 §break28134 LABEL
   
   \ new statement
   
   \ new statement
   DROP
   
   \ new statement
   1 VALLOCATE LOCAL §base1
   0
   LOCALS g |
   
   \ new statement
   mat
   std33674 VAL
   Gauss§-1354217728.table 926369536 EXECUTE-NEW
   DUP §base1 V! TO g FALSE DUP
   IF
      std33674 LABEL TRUE
   ENDIF
   IF
      0 §break28136 BRANCH
   ENDIF
   
   \ new statement
   
   1 VALLOCATE LOCAL §tempvar
   U" Enter right vector or empty line\n " COUNT JavaArray.createUnicode§-675323136 DUP §tempvar V!
   BasicIO.print§562786304
   
   
   §tempvar SETVTOP
   
   PURGE 1
   
   
   \ new statement
   BasicIO.readLine§-939691520
   DUP 4 §base0 + V! TO line
   
   \ new statement
   
   \ new statement
   BEGIN
      
      \ new statement
      line >R
      R> 188050432 TRUE ( equation.String.length§188050432 ) EXECUTE-METHOD
      0>
      
      \ new statement
   WHILE
      
      \ new statement
      b
      line
      Equation.fillRow§1526621696
      
      \ new statement
      g >R
      b
      std33682 VAL
      R> -766545152 TRUE ( equation.Gauss.solve§-766545152 ) EXECUTE-METHOD
      DUP 12 §base0 + V! TO b FALSE DUP
      IF
         std33682 LABEL TRUE
      ENDIF
      IF
         65537 §break28137 BRANCH
      ENDIF
      
      \ new statement
      1 LOCKI/O
      
      \ new statement
      
      1 VALLOCATE LOCAL §tempvar
      U" Solution:  " COUNT JavaArray.createUnicode§-675323136 DUP §tempvar V!
      BasicIO.print§562786304
      
      
      §tempvar SETVTOP
      
      PURGE 1
      
      
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
         
         2 VALLOCATE LOCAL §tempvar
         b >R
         count
         R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
         2@  F.. JavaArray.createString§-105880832 DUP §tempvar 4 + V!
         U"   " COUNT JavaArray.createUnicode§-675323136 DUP §tempvar V!
         SWAP  -1461427456 TRUE ( equation.String.concat§-1461427456 ) EXECUTE-METHOD
         BasicIO.print§562786304
         
         
         
         §tempvar SETVTOP
         
         PURGE 1
         
         
         \ new statement
         
         \ new statement
         count 1+ TO count
         
         \ new statement
      REPEAT
      
      \ new statement
      0 §break28139 LABEL
      
      \ new statement
      
      \ new statement
      0>!
      IF
         10001H - 0<! §break28137 0BRANCH
      ENDIF
      DROP
      
      \ new statement
      BasicIO.newLine§-283087360
      
      \ new statement
      -1 LOCKI/O
      
      \ new statement
      
      1 VALLOCATE LOCAL §tempvar
      U" Enter right vector or empty line\n " COUNT JavaArray.createUnicode§-675323136 DUP §tempvar V!
      BasicIO.print§562786304
      
      
      §tempvar SETVTOP
      
      PURGE 1
      
      
      \ new statement
      BasicIO.readLine§-939691520
      DUP 4 §base0 + V! TO line
      
      \ new statement
      
      \ new statement
   REPEAT
   
   \ new statement
   0 §break28137 LABEL
   
   \ new statement
   
   \ new statement
   0>!
   IF
      10001H - §break28136 BRANCH
   ENDIF
   DROP
   
   \ new statement
   0 §except28140 LABEL
   
   \ new statement
   0 §break28136 LABEL
   
   \ new statement
   
   §base1 SETVTOP
   PURGE 2
   
   \ new statement
   DROP
   
   \ new statement
   DUP " Exception " INSTANCEOF
   IF
      
      \ new statement
      1 VALLOCATE LOCAL §base1
      0
      LOCALS e |
      
      \ new statement
      DUP 0 V! TO e
      
      \ new statement
      
      1 VALLOCATE LOCAL §tempvar
      e >R  " Throwable " CASTTO
      R> 1695049472 TRUE ( equation.Throwable.getMessage§1695049472 ) EXECUTE-METHOD DUP §tempvar V!
      BasicIO.print§562786304
      
      
      §tempvar SETVTOP
      
      PURGE 1
      
      
      \ new statement
      0 §break28142 LABEL
      
      \ new statement
      
      §base1 SETVTOP
      PURGE 2
      
      \ new statement
      DROP
      
      \ new statement
      DUP §out28141 BRANCH
   ENDIF
   
   \ new statement
   §out28141 LABEL DROP
   
   \ new statement
   0 §break28131 LABEL
   
   \ new statement
   
   
   
   
   
   
   §base0 SETVTOP
   PURGE 7
   
   \ new statement
   DROP
;
VARIABLE Equation._staticBlocking
VARIABLE Equation._staticThread  HERE 4 - SALLOCATE
VARIABLE Equation.x  HERE 4 - SALLOCATE

A:HERE VARIABLE Equation§-1560460032.table 1 DUP 2* CELLS ALLOT R@ ! A:CELL+
-1254013696 R@ ! A:CELL+ Equation.Equation§-1254013696 VAL R@ ! A:CELL+
A:DROP


:LOCAL Equation.eq.eq§1684497664
   2 VALLOCATE LOCAL §base0
   
   \ new statement
   0
   LOCALS §this |
   
   \ new statement
   Thread§-1457236992.table -1360112640 EXECUTE-NEW
   
   \ new statement
   28 MALLOC DROP DUP DUP §base0 V! TO §this OVER
   IF
      OVER CELL+ @ 4 + OVER OVER ! DROP
   ENDIF
   CELL+ @ OVER
   OVER ! NIP
   Equation.eq§1990485248.table OVER 12 + !
   1990459392 OVER 20 + !
   " eq " OVER 16 + !
   0 OVER 24 + ! DROP
   
   \ new statement
   §this DUP 0 V! ( return object )    
   \ new statement
   0 §break28144 LABEL
   
   \ new statement
   §base0 SETVTOP
   PURGE 2
   
   \ new statement
   DROP
;


:LOCAL Equation.eq.run§706638336
   1 VALLOCATE LOCAL §base0
   DUP 0 §base0 + V! LOCAL §this
   
   \ new statement
   
   \ new statement
   0
   Equation.main§505244928
   
   \ new statement
   §this >R  " Thread " CASTTO
   R> 1390306560 TRUE ( equation.Thread.exit§1390306560 ) EXECUTE-METHOD
   
   \ new statement
   0 §break28143 LABEL
   
   \ new statement
   §base0 SETVTOP
   PURGE 2
   
   \ new statement
   DROP
;
VARIABLE Equation.eq._staticBlocking
VARIABLE Equation.eq._staticThread  HERE 4 - SALLOCATE

A:HERE VARIABLE Equation.eq§1990485248.table 2 DUP 2* CELLS ALLOT R@ ! A:CELL+
1684497664 R@ ! A:CELL+ Equation.eq.eq§1684497664 VAL R@ ! A:CELL+
706638336 R@ ! A:CELL+ Equation.eq.run§706638336 VAL R@ ! A:CELL+
A:DROP
