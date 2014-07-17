MODULENAME equation.Gauss
( *
* <p>Überschrift: Curve Fitting</p>
*
* <p>Beschreibung: solve a symmetric equation system using gauss's method</p>
*
* <p>Copyright: Copyright (c) 2008</p>
*
* <p>Organisation: </p>
*
* @author Gerhard Hohner
* @version 1.0
)


:LOCAL Gauss.Gauss§345327360
   
   \ new statement
   0
   LOCALS §this |
   
   \ new statement
   Object§-1890496768.table 1827294976 EXECUTE-NEW
   
   \ new statement
   48 MALLOC DROP DUP TO §this OVER
   IF
      OVER CELL+ @ 4 + OVER OVER ! DROP
   ENDIF
   CELL+ @ OVER
   OVER ! NIP
   Gauss§-1354217728.table OVER 12 + !
   -1354235904 OVER 20 + !
   " Gauss " OVER 16 + !
   2 OVER 24 + ! DROP
   
   \ new statement
   1.0E-18
   §this CELL+ @ 40 + ( equation.Gauss.MACH_EPS )    2!
   
   \ new statement
   §this ( return object )    
   \ new statement
   0 §break2370 LABEL
   
   \ new statement
   PURGE 1
   
   \ new statement
   DROP
;
( ====================================================================*
*                                                                    *
*  Gauss  berechnet die Zerlegung einer n x n Matrix in eine         *
*  untere und eine obere Dreieckmatrix. Diese Zerlegung wird zur     *
*  Loesung eines linearen Gleichungssystems benoetigt. Die Zerlegung *
*  befindet sich nach Aufruf von gaudec in der n x n Matrix lumat.   *
*                                                                    *
*====================================================================*
*                                                                    *
*   Eingabeparameter:                                                *
*   ================                                                 *
*      n        int n;  (n > 0)                                      *
*               Dimension von mat und lumat,                         *
*               Anzahl der Komponenten des b-Vektors, des Loe-       *
*               sungsvektors x, des Permutationsvektors perm.        *
*      mat      REAL   *mat[];                                       *
*               Matrix des Gleichungssystems. Diese wird als Vektor  *
*               von Zeigern uebergeben.                              *
*                                                                    *
*   Ausgabeparameter:                                                *
*   ================                                                 *
*      lumat    REAL   *lumat[];                                     *
*               LU-Dekompositionsmatrix, die die Zerlegung von       *
*               mat in eine untere und obere Dreieckmatrix ent-      *
*               haelt.                                               *
*      perm     int perm[];                                          *
*               Permutationsvektor, der die Zeilenvertauschungen     *
*               von lumat enthaelt.                                  *
*      signd    int signd;                                          *
*               Vorzeichen der Determinante von mat; die De-         *
*               terminante kann durch das Produkt der Diagonal-      *
*               elemente mal signd bestimmt werden.                  *
*                                                                    *
*====================================================================*
*                                                                    *
*   Benutzte Konstanten: MACH_EPS                              *
*==================================================================== )

:LOCAL Gauss.Gauss§926369536
   LOCAL §exception
   LOCAL mat
   
   \ new statement
   0 DUP DUP DUP DUP DUP
   LOCALS d i j j0 m §this |
   0 DUP 2DUP 2DUP
   2LOCALS piv tmp zmax |
   
   \ new statement
   Object§-1890496768.table 1827294976 EXECUTE-NEW
   
   \ new statement
   48 MALLOC DROP DUP TO §this OVER
   IF
      OVER CELL+ @ 4 + OVER OVER ! DROP
   ENDIF
   CELL+ @ OVER
   OVER ! NIP
   Gauss§-1354217728.table OVER 12 + !
   -1354235904 OVER 20 + !
   " Gauss " OVER 16 + !
   2 OVER 24 + ! DROP
   
   \ new statement
   1.0E-18
   §this CELL+ @ 40 + ( equation.Gauss.MACH_EPS )    2!
   
   \ new statement
   mat CELL+ @ 28 + ( equation.JavaArray.length )    @ 393217 JavaArray§1352878592.table -227194368 EXECUTE-NEW
   d TRUE JavaArray.kill§1620077312 TO d
   
   \ new statement
   mat CELL+ @ 28 + ( equation.JavaArray.length )    @ 262145 JavaArray§1352878592.table -227194368 EXECUTE-NEW
   §this CELL+ @ 32 + ( equation.Gauss.perm )    DUP @ TRUE JavaArray.kill§1620077312 !
   
   \ new statement
   mat CELL+ @ 28 + ( equation.JavaArray.length )    @ A:R@ 393218 JavaArray§1352878592.table -227194368 EXECUTE-NEW R> 0
   DO
      mat CELL+ @ 28 + ( equation.JavaArray.length )       @ 393217 JavaArray§1352878592.table -227194368 EXECUTE-NEW
      OVER I  OVER -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD  !
   LOOP
   §this CELL+ @ 28 + ( equation.Gauss.lumat )    DUP @ TRUE JavaArray.kill§1620077312 !
   
   \ new statement
   (  d = Skalierungsvektor       )  (  fuer Pivotsuche             )
   
   \ new statement
   
   \ new statement
   0
   TO i
   
   \ new statement
   BEGIN
      
      \ new statement
      i
      mat CELL+ @ 28 + ( equation.JavaArray.length )       @
      <
      
      \ new statement
   WHILE
      
      \ new statement
      
      \ new statement
      0
      TO j
      
      \ new statement
      BEGIN
         
         \ new statement
         j
         mat CELL+ @ 28 + ( equation.JavaArray.length )          @
         <
         
         \ new statement
      WHILE
         
         \ new statement
         
         0 0
         LOCALS 1§ 0§ |
         
         i TO 0§
         j TO 1§
         mat A:R@
         i
         R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD @ A:R@
         j
         R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
         2@
         §this CELL+ @ 28 + ( equation.Gauss.lumat ) @ A:R@
         0§
         R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD @ A:R@
         1§
         R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
         2!
         
         PURGE 2
         
         
         \ new statement
         
         \ new statement
         j 1+ TO j
         
         \ new statement
      REPEAT
      
      \ new statement
      0 §break2345 LABEL
      
      \ new statement
      
      \ new statement
      DROP
      
      \ new statement
      
      \ new statement
      i 1+ TO i
      
      \ new statement
   REPEAT
   
   \ new statement
   0 §break2344 LABEL
   
   \ new statement
   
   \ new statement
   DROP
   
   \ new statement
   
   \ new statement
   0
   TO i
   
   \ new statement
   BEGIN
      
      \ new statement
      i
      mat CELL+ @ 28 + ( equation.JavaArray.length )       @
      <
      
      \ new statement
   WHILE
      
      \ new statement
      
      0
      LOCALS 0§ |
      
      i TO 0§
      i
      §this CELL+ @ 32 + ( equation.Gauss.perm ) @ A:R@
      0§
      R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
      !
      
      PURGE 1
      
      
      \ new statement
      (  Initialisiere perm          )
      
      \ new statement
      0.0
      TO zmax
      
      \ new statement
      
      \ new statement
      0
      TO j
      
      \ new statement
      BEGIN
         
         \ new statement
         j
         mat CELL+ @ 28 + ( equation.JavaArray.length )          @
         <
         
         \ new statement
      WHILE
         
         \ new statement
         (  Zeilenmaximum bestimmen     )
         
         \ new statement
         §this CELL+ @ 28 + ( equation.Gauss.lumat ) @ A:R@
         i
         R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD @ A:R@
         j
         R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
         2@
         lang.Math.abs§-537370368
         TO tmp
         
         \ new statement
         tmp
         zmax
         F- FNEGATE F0<
         
         \ new statement
         IF
            
            \ new statement
            tmp
            TO zmax
            
            \ new statement
         ENDIF
         
         \ new statement
         
         \ new statement
         j 1+ TO j
         
         \ new statement
      REPEAT
      
      \ new statement
      0 §break2348 LABEL
      
      \ new statement
      
      \ new statement
      DROP
      
      \ new statement
      zmax
      F0=
      
      \ new statement
      IF
         
         \ new statement
         U" system singular " COUNT JavaArray.createUnicode§-675323136
         Exception§-1574681344.table 264127744 EXECUTE-NEW
         §exception TO §return
         
         \ new statement
         65537 §break2346 BRANCH
         
         \ new statement
      ENDIF
      
      \ new statement
      (  mat singulaer               )
      
      \ new statement
      
      0
      LOCALS 0§ |
      
      i TO 0§
      1.0
      zmax
      F/
      d A:R@
      0§
      R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
      2!
      
      PURGE 1
      
      
      \ new statement
      
      \ new statement
      i 1+ TO i
      
      \ new statement
   REPEAT
   
   \ new statement
   0 §break2346 LABEL
   
   \ new statement
   
   \ new statement
   0>!
   IF
      10001H - §break2343 BRANCH
   ENDIF
   DROP
   
   \ new statement
   1
   §this CELL+ @ 36 + ( equation.Gauss.signd )    !
   
   \ new statement
   (  Vorzeichen der Determinante   )
   
   \ new statement
   
   \ new statement
   0
   TO i
   
   \ new statement
   BEGIN
      
      \ new statement
      i
      mat CELL+ @ 28 + ( equation.JavaArray.length )       @
      <
      
      \ new statement
   WHILE
      
      \ new statement
      §this CELL+ @ 28 + ( equation.Gauss.lumat ) @ A:R@
      i
      R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD @ A:R@
      i
      R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
      2@
      lang.Math.abs§-537370368
      d A:R@
      i
      R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
      2@
      F*
      TO piv
      
      \ new statement
      i
      TO j0
      
      \ new statement
      (  Suche aktuelles Pivotelement  )
      
      \ new statement
      
      \ new statement
      i
      1
      +
      TO j
      
      \ new statement
      BEGIN
         
         \ new statement
         j
         mat CELL+ @ 28 + ( equation.JavaArray.length )          @
         <
         
         \ new statement
      WHILE
         
         \ new statement
         §this CELL+ @ 28 + ( equation.Gauss.lumat ) @ A:R@
         j
         R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD @ A:R@
         i
         R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
         2@
         lang.Math.abs§-537370368
         d A:R@
         j
         R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
         2@
         F*
         TO tmp
         
         \ new statement
         piv
         tmp
         F<
         
         \ new statement
         IF
            
            \ new statement
            
            \ new statement
            tmp
            TO piv
            
            \ new statement
            (  Merke Pivotelement u.         )
            
            \ new statement
            j
            TO j0
            
            \ new statement
            (  dessen Index                  )
            
            \ new statement
            0 §break2354 LABEL
            
            \ new statement
            
            \ new statement
            DROP
            
            \ new statement
         ENDIF
         
         \ new statement
         
         \ new statement
         j 1+ TO j
         
         \ new statement
      REPEAT
      
      \ new statement
      0 §break2352 LABEL
      
      \ new statement
      
      \ new statement
      DROP
      
      \ new statement
      piv
      §this CELL+ @ 40 + ( equation.Gauss.MACH_EPS )       2@
      F<
      
      \ new statement
      IF
         
         \ new statement
         
         0 0
         LOCALS 1§ 0§ |
         
         U" system almost singular  " COUNT JavaArray.createUnicode§-675323136 DUP TO 0§
         piv  F.. JavaArray.createString§-105880832  DUP TO 1§
         DUP INCREFERENCE  OVER  -1461427456 TRUE ( equation.String.concat§-1461427456 ) EXECUTE-METHOD
         Exception§-1574681344.table 264127744 EXECUTE-NEW
         §exception TO §return
         1§ TRUE JavaArray.kill§1620077312
         0§ TRUE JavaArray.kill§1620077312
         
         PURGE 2
         
         
         \ new statement
         65537 §break2350 BRANCH
         
         \ new statement
      ENDIF
      
      \ new statement
      (  Wenn piv zu klein, so ist     )
      
      \ new statement
      j0
      i
      <>
      
      \ new statement
      IF
         
         \ new statement
         0 DUP
         LOCALS x y |
         
         \ new statement
         §this CELL+ @ 36 + ( equation.Gauss.signd )          @ A:R@ NEGATE
         A:DROP
         §this CELL+ @ 36 + ( equation.Gauss.signd )          !
         
         \ new statement
         (  Vorzeichen Determinante *(-1) )
         
         \ new statement
         §this CELL+ @ 32 + ( equation.Gauss.perm ) @ A:R@
         j0
         R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
         @
         TO x
         
         \ new statement
         (  Tausche Pivoteintraege        )
         
         \ new statement
         
         0
         LOCALS 0§ |
         
         j0 TO 0§
         §this CELL+ @ 32 + ( equation.Gauss.perm ) @ A:R@
         i
         R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
         @
         §this CELL+ @ 32 + ( equation.Gauss.perm ) @ A:R@
         0§
         R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
         !
         
         PURGE 1
         
         
         \ new statement
         
         0
         LOCALS 0§ |
         
         i TO 0§
         x
         §this CELL+ @ 32 + ( equation.Gauss.perm ) @ A:R@
         0§
         R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
         !
         
         PURGE 1
         
         
         \ new statement
         (  Tausche Eintraege im          )
         
         \ new statement
         d A:R@
         j0
         R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
         2@
         TO tmp
         
         \ new statement
         (  Skalierungsvektor             )
         
         \ new statement
         
         0
         LOCALS 0§ |
         
         j0 TO 0§
         d A:R@
         i
         R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
         2@
         d A:R@
         0§
         R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
         2!
         
         PURGE 1
         
         
         \ new statement
         
         0
         LOCALS 0§ |
         
         i TO 0§
         tmp
         d A:R@
         0§
         R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
         2!
         
         PURGE 1
         
         
         \ new statement
         §this CELL+ @ 28 + ( equation.Gauss.lumat ) @ A:R@
         j0
         R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
         @ DUP INCREFERENCE
         y TRUE JavaArray.kill§1620077312 TO y
         
         \ new statement
         (  Tausch j0-te u. i-te   )
         
         \ new statement
         
         0
         LOCALS 0§ |
         
         j0 TO 0§
         §this CELL+ @ 28 + ( equation.Gauss.lumat ) @ A:R@
         i
         R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
         @ DUP INCREFERENCE
         §this CELL+ @ 28 + ( equation.Gauss.lumat ) @ A:R@
         0§
         R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
         DUP @ TRUE JavaArray.kill§1620077312 !
         
         PURGE 1
         
         
         \ new statement
         (  Zeile von lumat        )
         
         \ new statement
         
         0
         LOCALS 0§ |
         
         i TO 0§
         y DUP INCREFERENCE
         §this CELL+ @ 28 + ( equation.Gauss.lumat ) @ A:R@
         0§
         R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
         DUP @ TRUE JavaArray.kill§1620077312 !
         
         PURGE 1
         
         
         \ new statement
         0 §break2355 LABEL
         
         \ new statement
         
         y TRUE JavaArray.kill§1620077312
         PURGE 2
         
         \ new statement
         0>!
         IF
            10001H - 0<! §break2350 0BRANCH
         ENDIF
         DROP
         
         \ new statement
      ENDIF
      
      \ new statement
      
      \ new statement
      i
      1
      +
      TO j
      
      \ new statement
      BEGIN
         
         \ new statement
         j
         mat CELL+ @ 28 + ( equation.JavaArray.length )          @
         <
         
         \ new statement
      WHILE
         
         \ new statement
         (  Gauss Eliminationsschritt     )
         
         \ new statement
         §this CELL+ @ 28 + ( equation.Gauss.lumat ) @ A:R@
         j
         R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD @ A:R@
         i
         R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
         2@
         F0= 0=
         
         \ new statement
         IF
            
            \ new statement
            
            \ new statement
            
            0 0
            LOCALS 1§ 0§ |
            
            j TO 0§
            i TO 1§
            §this CELL+ @ 28 + ( equation.Gauss.lumat ) @ A:R@
            0§
            R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD @ A:R@
            1§
            R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
            2@
            §this CELL+ @ 28 + ( equation.Gauss.lumat ) @ A:R@
            i
            R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD @ A:R@
            i
            R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
            2@
            F/
            §this CELL+ @ 28 + ( equation.Gauss.lumat ) @ A:R@
            0§
            R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD @ A:R@
            1§
            R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
            2!
            
            PURGE 2
            
            
            \ new statement
            §this CELL+ @ 28 + ( equation.Gauss.lumat ) @ A:R@
            j
            R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD @ A:R@
            i
            R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
            2@
            TO tmp
            
            \ new statement
            
            \ new statement
            i
            1
            +
            TO m
            
            \ new statement
            BEGIN
               
               \ new statement
               m
               mat CELL+ @ 28 + ( equation.JavaArray.length )                @
               <
               
               \ new statement
            WHILE
               
               \ new statement
               
               0 0
               LOCALS 1§ 0§ |
               
               j TO 0§
               m TO 1§
               §this CELL+ @ 28 + ( equation.Gauss.lumat ) @ A:R@
               0§
               R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD @ A:R@
               1§
               R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
               2@
               tmp
               §this CELL+ @ 28 + ( equation.Gauss.lumat ) @ A:R@
               i
               R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD @ A:R@
               m
               R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
               2@
               F*
               F-
               §this CELL+ @ 28 + ( equation.Gauss.lumat ) @ A:R@
               0§
               R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD @ A:R@
               1§
               R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
               2!
               
               PURGE 2
               
               
               \ new statement
               
               \ new statement
               m 1+ TO m
               
               \ new statement
            REPEAT
            
            \ new statement
            0 §break2359 LABEL
            
            \ new statement
            
            \ new statement
            DROP
            
            \ new statement
            0 §break2358 LABEL
            
            \ new statement
            
            \ new statement
            DROP
            
            \ new statement
         ENDIF
         
         \ new statement
         
         \ new statement
         j 1+ TO j
         
         \ new statement
      REPEAT
      
      \ new statement
      0 §break2356 LABEL
      
      \ new statement
      
      \ new statement
      0>!
      IF
         10001H - 0<! §break2350 0BRANCH
      ENDIF
      DROP
      
      \ new statement
      
      \ new statement
      i 1+ TO i
      
      \ new statement
   REPEAT
   
   \ new statement
   0 §break2350 LABEL
   
   \ new statement
   
   \ new statement
   0>!
   IF
      10001H - §break2343 BRANCH
   ENDIF
   DROP
   
   \ new statement
   (  end i  )
   
   \ new statement
   §this ( return object )    
   \ new statement
   0 §break2343 LABEL
   
   \ new statement
   d TRUE JavaArray.kill§1620077312
   
   
   
   
   mat TRUE JavaArray.kill§1620077312
   
   
   
   
   PURGE 11
   
   \ new statement
   DROP
;


: Gauss.classMonitorEnter§-433167616
   
   \ new statement
   
   \ new statement
   0 §break2341 LABEL
   
   \ new statement
   
   \ new statement
   DROP
;


: Gauss.classMonitorLeave§-1691393280
   
   \ new statement
   
   \ new statement
   0 §break2342 LABEL
   
   \ new statement
   
   \ new statement
   DROP
;
( ====================================================================*
*                                                                    *
*  det berechnet die Determinante einer n x n Matrix.                *
*                                                                    *
*====================================================================*
*                                                                    *
*   Eingabeparameter:                                                *
*   ================                                                 *
*      n        int n;  (n > 0)                                      *
*               Dimension von mat.                                   *
*      mat      REAL   *mat[];                                       *
*               n x n Matrix, deren Determinante zu bestimmen ist.   *
*                                                                    *
*   Rueckgabewert:                                                   *
*   =============                                                    *
*      REAL     Determinante von mat.                                *
*               Ist der Rueckgabewert = 0, so ist die Matrix ent-    *
*               weder singulaer oder es ist nicht genuegend Speicher *
*               vorhanden.                                           *
*                                                                    *
*==================================================================== )

:LOCAL Gauss.det§-478714880
   LOCAL §exception
   LOCAL §this
   
   \ new statement
   0
   LOCALS i |
   0 DUP
   2LOCALS tmpdet |
   
   \ new statement
   §this CELL+ @ 28 + ( equation.Gauss.lumat ) @ CELL+ @ 28 + ( equation.JavaArray.length )    @
   1
   <  0=! or_2565 0BRANCH DROP
   §this CELL+ @ 36 + ( equation.Gauss.signd )    @
   0=
   or_2565 LABEL
   
   \ new statement
   IF
      
      \ new statement
      U" no valid matrix " COUNT JavaArray.createUnicode§-675323136
      Exception§-1574681344.table 264127744 EXECUTE-NEW
      §exception TO §return
      
      \ new statement
      0 §break2367 BRANCH
      
      \ new statement
   ENDIF
   
   \ new statement
   (  berechnen             )
   
   \ new statement
   §this CELL+ @ 36 + ( equation.Gauss.signd )    @  S>D D>F
   TO tmpdet
   
   \ new statement
   
   \ new statement
   0
   TO i
   
   \ new statement
   BEGIN
      
      \ new statement
      i
      §this CELL+ @ 28 + ( equation.Gauss.lumat ) @ CELL+ @ 28 + ( equation.JavaArray.length )       @
      <
      
      \ new statement
   WHILE
      
      \ new statement
      tmpdet
      lang.Math.abs§-537370368
      §this CELL+ @ 40 + ( equation.Gauss.MACH_EPS )       2@
      §this CELL+ @ 40 + ( equation.Gauss.MACH_EPS )       2@
      F*
      F<
      
      \ new statement
      IF
         
         \ new statement
         U" no valid matrix " COUNT JavaArray.createUnicode§-675323136
         Exception§-1574681344.table 264127744 EXECUTE-NEW
         §exception TO §return
         
         \ new statement
         65537 §break2368 BRANCH
         
         \ new statement
      ENDIF
      
      \ new statement
      tmpdet
      §this CELL+ @ 28 + ( equation.Gauss.lumat ) @ A:R@
      i
      R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD @ A:R@
      i
      R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
      2@
      F*
      TO tmpdet
      
      \ new statement
      (  Berechne det          )
      
      \ new statement
      
      \ new statement
      i 1+ TO i
      
      \ new statement
   REPEAT
   
   \ new statement
   0 §break2368 LABEL
   
   \ new statement
   
   \ new statement
   0>!
   IF
      10001H - §break2367 BRANCH
   ENDIF
   DROP
   
   \ new statement
   tmpdet
   0 §break2367 BRANCH
   
   \ new statement
   0 §break2367 LABEL
   
   \ new statement
   
   
   
   PURGE 4
   
   \ new statement
   DROP
;
( ====================================================================*
*                                                                    *
*  solve  bestimmt die Loesung x des linearen Gleichungssystems     *
*  lumat * x = b mit der n x n Koeffizientenmatrix lumat, wobei      *
*  lumat in zerlegter Form (LU - Dekomposition) vorliegt, wie        *
*  sie von gaudec als Ausgabe geliefert wird.                        *
*                                                                    *
*====================================================================*
*                                                                    *
*   Eingabeparameter:                                                *
*   ================                                                 *
*      n        int n;  (n > 0)                                      *
*               Dimension von lumat,                                 *
*               Anzahl der Komponenten des b-Vektors, des Loe-       *
*               sungsvektors x, des Permutationsvektors perm.        *
*      lumat    REAL   *lumat[];                                     *
*               LU-Dekompositionsmatrix, wie sie von gaudec          *
*               geliefert wird.                                      *
*      perm     int perm[];                                          *
*               Permutationsvektor, der die Zeilenvertauschungen     *
*               von lumat enthaelt.                                  *
*      b        REAL   b[];                                          *
*               Rechte Seite des Gleichungssystems.                  *
*                                                                    *
*   Ausgabeparameter:                                                *
*   ================                                                 *
*      x        REAL   x[];                                          *
*               Loesungsvektor des Systems.                          *
*                                                                    *
*==================================================================== )

:LOCAL Gauss.solve§-766545152
   LOCAL §exception
   LOCAL b
   LOCAL §this
   
   \ new statement
   0 DUP DUP
   LOCALS j k x |
   0 DUP
   2LOCALS sum |
   
   \ new statement
   §this CELL+ @ 28 + ( equation.Gauss.lumat ) @ CELL+ @ 28 + ( equation.JavaArray.length )    @ 393217 JavaArray§1352878592.table -227194368 EXECUTE-NEW
   x TRUE JavaArray.kill§1620077312 TO x
   
   \ new statement
   
   \ new statement
   0
   TO k
   
   \ new statement
   BEGIN
      
      \ new statement
      k
      §this CELL+ @ 28 + ( equation.Gauss.lumat ) @ CELL+ @ 28 + ( equation.JavaArray.length )       @
      <
      
      \ new statement
   WHILE
      
      \ new statement
      (  Vorwaertselimination    )
      
      \ new statement
      
      0
      LOCALS 0§ |
      
      k TO 0§
      b A:R@
      §this CELL+ @ 32 + ( equation.Gauss.perm ) @ A:R@
      k
      R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
      @
      R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
      2@
      x A:R@
      0§
      R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
      2!
      
      PURGE 1
      
      
      \ new statement
      
      \ new statement
      0
      TO j
      
      \ new statement
      BEGIN
         
         \ new statement
         j
         k
         <
         
         \ new statement
      WHILE
         
         \ new statement
         
         0
         LOCALS 0§ |
         
         k TO 0§
         x A:R@
         0§
         R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
         2@
         §this CELL+ @ 28 + ( equation.Gauss.lumat ) @ A:R@
         k
         R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD @ A:R@
         j
         R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
         2@
         x A:R@
         j
         R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
         2@
         F*
         F-
         x A:R@
         0§
         R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
         2!
         
         PURGE 1
         
         
         \ new statement
         
         \ new statement
         j 1+ TO j
         
         \ new statement
      REPEAT
      
      \ new statement
      0 §break2363 LABEL
      
      \ new statement
      
      \ new statement
      DROP
      
      \ new statement
      
      \ new statement
      k 1+ TO k
      
      \ new statement
   REPEAT
   
   \ new statement
   0 §break2361 LABEL
   
   \ new statement
   
   \ new statement
   DROP
   
   \ new statement
   
   \ new statement
   §this CELL+ @ 28 + ( equation.Gauss.lumat ) @ CELL+ @ 28 + ( equation.JavaArray.length )    @
   1
   -
   TO k
   
   \ new statement
   BEGIN
      
      \ new statement
      k
      0< INVERT
      
      \ new statement
   WHILE
      
      \ new statement
      (  Rueckwaertselimination  )
      
      \ new statement
      0.0
      TO sum
      
      \ new statement
      
      \ new statement
      k
      1
      +
      TO j
      
      \ new statement
      BEGIN
         
         \ new statement
         j
         §this CELL+ @ 28 + ( equation.Gauss.lumat ) @ CELL+ @ 28 + ( equation.JavaArray.length )          @
         <
         
         \ new statement
      WHILE
         
         \ new statement
         sum
         §this CELL+ @ 28 + ( equation.Gauss.lumat ) @ A:R@
         k
         R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD @ A:R@
         j
         R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
         2@
         x A:R@
         j
         R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
         2@
         F*
         F+
         TO sum
         
         \ new statement
         
         \ new statement
         j 1+ TO j
         
         \ new statement
      REPEAT
      
      \ new statement
      0 §break2366 LABEL
      
      \ new statement
      
      \ new statement
      DROP
      
      \ new statement
      §this CELL+ @ 28 + ( equation.Gauss.lumat ) @ A:R@
      k
      R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD @ A:R@
      k
      R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
      2@
      F0=
      
      \ new statement
      IF
         
         \ new statement
         U" invalid decomposition " COUNT JavaArray.createUnicode§-675323136
         Exception§-1574681344.table 264127744 EXECUTE-NEW
         §exception TO §return
         
         \ new statement
         65537 §break2364 BRANCH
         
         \ new statement
      ENDIF
      
      \ new statement
      
      0
      LOCALS 0§ |
      
      k TO 0§
      x A:R@
      k
      R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
      2@
      sum
      F-
      §this CELL+ @ 28 + ( equation.Gauss.lumat ) @ A:R@
      k
      R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD @ A:R@
      k
      R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
      2@
      F/
      x A:R@
      0§
      R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
      2!
      
      PURGE 1
      
      
      \ new statement
      
      \ new statement
      k 1- TO k
      
      \ new statement
   REPEAT
   
   \ new statement
   0 §break2364 LABEL
   
   \ new statement
   
   \ new statement
   0>!
   IF
      10001H - §break2360 BRANCH
   ENDIF
   DROP
   
   \ new statement
   x DUP INCREFERENCE
   0 §break2360 BRANCH
   
   \ new statement
   0 §break2360 LABEL
   
   \ new statement
   b TRUE JavaArray.kill§1620077312
   
   
   
   x TRUE JavaArray.kill§1620077312
   
   PURGE 7
   
   \ new statement
   DROP
;


:LOCAL Gauss.~destructor§-1086882304
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
      §this CELL+ @ 28 + ( equation.Gauss.lumat )       @ A:R@
      A:DROP
      TO obj
      
      \ new statement
      obj
      -1
      JavaArray.kill§1620077312
      
      \ new statement
      §this CELL+ @ 32 + ( equation.Gauss.perm )       @ A:R@
      A:DROP
      TO obj
      
      \ new statement
      obj
      -1
      JavaArray.kill§1620077312
      
      \ new statement
   ENDIF
   §this DECREFERENCE
   
   \ new statement
   0 §break2371 LABEL
   
   \ new statement
   PURGE 2
   
   \ new statement
   DROP
;
VARIABLE Gauss._staticBlocking
VARIABLE Gauss._staticThread

A:HERE VARIABLE Gauss§-1354217728.table 5 DUP 2* CELLS ALLOT R@ ! A:CELL+
345327360 R@ ! A:CELL+ Gauss.Gauss§345327360 VAL R@ ! A:CELL+
926369536 R@ ! A:CELL+ Gauss.Gauss§926369536 VAL R@ ! A:CELL+
-478714880 R@ ! A:CELL+ Gauss.det§-478714880 VAL R@ ! A:CELL+
-766545152 R@ ! A:CELL+ Gauss.solve§-766545152 VAL R@ ! A:CELL+
-1086882304 R@ ! A:CELL+ Gauss.~destructor§-1086882304 VAL R@ ! A:CELL+
A:DROP
