MODULENAME sort.BasicIO
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



:LOCAL sort.BasicIO.BasicIO§-1035320832
   
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
   sort.BasicIO§1351107072.table OVER 12 + !
   1351090176 OVER 20 + !
   " BasicIO " OVER 16 + ! DROP
   
   \ new statement
   0
   §this CELL+ @ 24 + ( sort.BasicIO._dynamicBlocking )    !
   
   \ new statement
   §this ( return object )    
   \ new statement
   0 §break2002 LABEL
   
   \ new statement
   PURGE 1
   
   \ new statement
   DROP
;


: sort.BasicIO.newLine§-283087360
   
   \ new statement
   
   \ new statement
   \ System.out.println();
   
   
   \ new statement
   CR
   
   \ new statement
   0 §break2001 LABEL
   
   \ new statement
   
   \ new statement
   DROP
;


: sort.BasicIO.print§-1271697408
   LOCAL a
   
   \ new statement
   
   \ new statement
   
   0 DUP
   LOCALS 1§ 0§ |
   
   a  <# 0 ..R equation.JavaArray.createString§-105880832  DUP TO 1§
   U"  " COUNT equation.JavaArray.createUnicode§-675323136 DUP TO 0§
   DUP INCREFERENCE  OVER  -1461427456 TRUE ( equation.String.concat§-1461427456 ) EXECUTE-METHOD
   sort.BasicIO.print§562786304
   1§ TRUE equation.JavaArray.kill§1620077312
   0§ TRUE equation.JavaArray.kill§1620077312
   
   PURGE 2
   
   
   \ new statement
   0 §break2000 LABEL
   
   \ new statement
   
   PURGE 1
   
   \ new statement
   DROP
;


: sort.BasicIO.print§532901888
   LOCAL y
   
   \ new statement
   
   \ new statement
   \ System.out.print(y);
   
   
   \ new statement
   
   0 DUP
   LOCALS 1§ 0§ |
   
   U"  " COUNT equation.JavaArray.createUnicode§-675323136 DUP TO 0§
   y  <# DUP 8 RSHIFT HOLD HOLD DUP DUP #> equation.JavaArray.createUnicode§-675323136  DUP TO 1§
   DUP INCREFERENCE  OVER  -1461427456 TRUE ( equation.String.concat§-1461427456 ) EXECUTE-METHOD
   equation.JavaArray.print§562786304
   1§ TRUE equation.JavaArray.kill§1620077312
   0§ TRUE equation.JavaArray.kill§1620077312
   
   PURGE 2
   
   
   \ new statement
   0 §break1998 LABEL
   
   \ new statement
   
   PURGE 1
   
   \ new statement
   DROP
;


: sort.BasicIO.print§562786304
   LOCAL s
   
   \ new statement
   
   \ new statement
   \ System.out.print(s);
   
   
   \ new statement
   s DUP INCREFERENCE
   equation.JavaArray.print§562786304
   
   \ new statement
   0 §break1999 LABEL
   
   \ new statement
   s TRUE equation.JavaArray.kill§1620077312
   PURGE 1
   
   \ new statement
   DROP
;


:LOCAL sort.BasicIO.~destructor§-1086882304
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
   0 §break2003 LABEL
   
   \ new statement
   PURGE 2
   
   \ new statement
   DROP
;
VARIABLE sort.BasicIO._staticBlocking

A:HERE VARIABLE sort.BasicIO§1351107072.table 2 DUP 2* CELLS ALLOT R@ ! A:CELL+
-1035320832 R@ ! A:CELL+ sort.BasicIO.BasicIO§-1035320832 VAL R@ ! A:CELL+
-1086882304 R@ ! A:CELL+ sort.BasicIO.~destructor§-1086882304 VAL R@ ! A:CELL+
A:DROP
