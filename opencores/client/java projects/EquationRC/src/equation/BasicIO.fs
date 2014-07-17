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
   BasicIO§1351107072.table OVER 12 + !
   1351090176 OVER 20 + !
   " BasicIO " OVER 16 + !
   0 OVER 24 + ! DROP
   
   \ new statement
   §this ( return object )    
   \ new statement
   0 §break2053 LABEL
   
   \ new statement
   PURGE 1
   
   \ new statement
   DROP
;


: BasicIO.classMonitorEnter§-433167616
   
   \ new statement
   
   \ new statement
   0 §break2046 LABEL
   
   \ new statement
   
   \ new statement
   DROP
;


: BasicIO.classMonitorLeave§-1691393280
   
   \ new statement
   
   \ new statement
   0 §break2047 LABEL
   
   \ new statement
   
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
   0 §break2051 LABEL
   
   \ new statement
   
   \ new statement
   DROP
;


: BasicIO.print§-1271697408
   LOCAL a
   
   \ new statement
   
   \ new statement
   
   0 0
   LOCALS 1§ 0§ |
   
   a  <# 0 ..R JavaArray.createString§-105880832  DUP TO 1§
   U"  " COUNT JavaArray.createUnicode§-675323136 DUP TO 0§
   DUP INCREFERENCE  OVER  -1461427456 TRUE ( equation.String.concat§-1461427456 ) EXECUTE-METHOD
   BasicIO.print§562786304
   1§ TRUE JavaArray.kill§1620077312
   0§ TRUE JavaArray.kill§1620077312
   
   PURGE 2
   
   
   \ new statement
   0 §break2050 LABEL
   
   \ new statement
   
   PURGE 1
   
   \ new statement
   DROP
;


: BasicIO.print§532901888
   LOCAL y
   
   \ new statement
   
   \ new statement
   \ System.out.print(y);
   
   
   \ new statement
   
   0 0
   LOCALS 1§ 0§ |
   
   U"  " COUNT JavaArray.createUnicode§-675323136 DUP TO 0§
   y  <# DUP 8 RSHIFT HOLD HOLD DUP DUP #> JavaArray.createUnicode§-675323136  DUP TO 1§
   DUP INCREFERENCE  OVER  -1461427456 TRUE ( equation.String.concat§-1461427456 ) EXECUTE-METHOD
   JavaArray.print§562786304
   1§ TRUE JavaArray.kill§1620077312
   0§ TRUE JavaArray.kill§1620077312
   
   PURGE 2
   
   
   \ new statement
   0 §break2048 LABEL
   
   \ new statement
   
   PURGE 1
   
   \ new statement
   DROP
;


: BasicIO.print§562786304
   LOCAL s
   
   \ new statement
   
   \ new statement
   \ System.out.print(s);
   
   
   \ new statement
   s DUP INCREFERENCE
   JavaArray.print§562786304
   
   \ new statement
   0 §break2049 LABEL
   
   \ new statement
   s TRUE JavaArray.kill§1620077312
   PURGE 1
   
   \ new statement
   DROP
;


: BasicIO.readLine§-939691520
   
   \ new statement
   0 DUP DUP
   LOCALS buffer bytes length |
   
   \ new statement
   1024
   65537 JavaArray§1352878592.table -227194368 EXECUTE-NEW
   buffer TRUE JavaArray.kill§1620077312 TO buffer
   
   \ new statement
   buffer
   A:R@
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
   0 §break2052 BRANCH
   
   \ new statement
   0 §break2052 LABEL
   
   \ new statement
   buffer TRUE JavaArray.kill§1620077312
   
   
   PURGE 3
   
   \ new statement
   DROP
;


:LOCAL BasicIO.~destructor§-1086882304
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
   0 §break2054 LABEL
   
   \ new statement
   PURGE 2
   
   \ new statement
   DROP
;
VARIABLE BasicIO._staticBlocking
VARIABLE BasicIO._staticThread

A:HERE VARIABLE BasicIO§1351107072.table 2 DUP 2* CELLS ALLOT R@ ! A:CELL+
-1035320832 R@ ! A:CELL+ BasicIO.BasicIO§-1035320832 VAL R@ ! A:CELL+
-1086882304 R@ ! A:CELL+ BasicIO.~destructor§-1086882304 VAL R@ ! A:CELL+
A:DROP
