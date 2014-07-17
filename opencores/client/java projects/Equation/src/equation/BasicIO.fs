MODULENAME equation.BasicIO
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
)  \ import java.io.PrintStream;



:LOCAL BasicIO.BasicIO§-1035320832
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
   BasicIO§1351107072.table OVER 12 + !
   1351090176 OVER 20 + !
   " BasicIO " OVER 16 + !
   0 OVER 24 + ! DROP
   
   \ new statement
   §this DUP 0 V! ( return object )    
   \ new statement
   0 §break33020 LABEL
   
   \ new statement
   §base0 SETVTOP
   PURGE 2
   
   \ new statement
   DROP
;


: BasicIO.newLine§-283087360
   
   \ new statement
   
   \ new statement
   \ System.out.println();
   
   
   \ new statement
   CR
   
   \ new statement
   0 §break33018 LABEL
   
   \ new statement
   
   \ new statement
   DROP
;


: BasicIO.print§-1271697408
   LOCAL a
   
   \ new statement
   
   \ new statement
   
   2 VALLOCATE LOCAL §tempvar
   a  <# 0 ..R JavaArray.createString§-105880832 DUP §tempvar 4 + V!
   U"  " COUNT JavaArray.createUnicode§-675323136 DUP §tempvar V!
   SWAP  -1461427456 TRUE ( equation.String.concat§-1461427456 ) EXECUTE-METHOD
   BasicIO.print§562786304
   
   
   
   §tempvar SETVTOP
   
   PURGE 1
   
   
   \ new statement
   0 §break33017 LABEL
   
   \ new statement
   
   PURGE 1
   
   \ new statement
   DROP
;


: BasicIO.print§532901888
   LOCAL y
   
   \ new statement
   0 DUP
   LOCALS a b |
   
   \ new statement
   \ System.out.print(y);
   
   
   \ new statement
   
   2 VALLOCATE LOCAL §tempvar
   U"  " COUNT JavaArray.createUnicode§-675323136 DUP §tempvar V!
   y  <# DUP 8 RSHIFT HOLD HOLD DUP DUP #> JavaArray.createUnicode§-675323136 DUP §tempvar 4 + V!
   SWAP  -1461427456 TRUE ( equation.String.concat§-1461427456 ) EXECUTE-METHOD
   JavaArray.print§562786304
   
   
   
   §tempvar SETVTOP
   
   PURGE 1
   
   
   \ new statement
   0
   TO a
   
   \ new statement
   
   \ new statement
   a
   3
   *
   2/
   3
   MOD
   A:R@ ABS A:1 A:XOR 1 AND R> 0<
   IF
      NEGATE
   ENDIF
   TO b
   
   \ new statement
   0 §break33015 LABEL
   
   \ new statement
   
   
   
   PURGE 3
   
   \ new statement
   DROP
;


: BasicIO.print§562786304
   1 VALLOCATE LOCAL §base0
   DUP 0 §base0 + V! LOCAL s
   
   \ new statement
   
   \ new statement
   \ System.out.print(s);
   
   
   \ new statement
   s
   JavaArray.print§562786304
   
   \ new statement
   0 §break33016 LABEL
   
   \ new statement
   
   §base0 SETVTOP
   PURGE 2
   
   \ new statement
   DROP
;


: BasicIO.readLine§-939691520
   1 VALLOCATE LOCAL §base0
   
   \ new statement
   0 DUP DUP
   LOCALS buffer bytes length |
   
   \ new statement
   
   1 VALLOCATE LOCAL §tempvar
   1024
   65537 JavaArray§1352878592.table -227194368 EXECUTE-NEW
   DUP §base0 V! TO buffer
   
   §tempvar SETVTOP
   
   PURGE 1
   
   
   \ new statement
   buffer
   >R
   0
   R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
   TO bytes
   
   \ new statement
   buffer CELL+ @ 28 + ( equation.JavaArray.length )    @
   1
   -
   TO length
   
   \ new statement
   bytes length ACCEPT TO length
   
   \ new statement
   bytes
   length
   JavaArray.createString§-105880832
   DUP 0 V!
   0 §break33019 BRANCH
   
   \ new statement
   0 §break33019 LABEL
   
   \ new statement
   
   
   
   §base0 SETVTOP
   PURGE 4
   
   \ new statement
   DROP
;
VARIABLE BasicIO._staticBlocking
VARIABLE BasicIO._staticThread  HERE 4 - SALLOCATE

A:HERE VARIABLE BasicIO§1351107072.table 1 DUP 2* CELLS ALLOT R@ ! A:CELL+
-1035320832 R@ ! A:CELL+ BasicIO.BasicIO§-1035320832 VAL R@ ! A:CELL+
A:DROP
