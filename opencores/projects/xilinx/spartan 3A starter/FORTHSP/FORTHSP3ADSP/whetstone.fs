(
 * C Converted Whetstone Double Precision Benchmark
 *		Version 1.2	22 March 1998
 *
 *	(c) Copyright 1998 Painter Engineering, Inc.
 *		All Rights Reserved.
 *
 *		Permission is granted to use, duplicate, and
 *		publish this text and program as long as it
 *		includes this entire comment block and limited
 *		rights reference.
 *
 * Converted by Rich Painter, Painter Engineering, Inc. based on the
 * www.netlib.org benchmark/whetstoned version obtained 16 March 1998.
 *
 * A novel approach was used here to keep the look and feel of the
 * FORTRAN version.  Altering the FORTRAN-based array indices,
 * starting at element 1, to start at element 0 for C, would require
 * numerous changes, including decrementing the variable indices by 1.
 * Instead, the array E1[] was declared 1 element larger in C.  This
 * allows the FORTRAN index range to function without any literal or
 * variable indices changes.  The array element E1[0] is simply never
 * used and does not alter the benchmark results.
 *
 * The major FORTRAN comment blocks were retained to minimize
 * differences between versions.  Modules N5 and N12, like in the
 * FORTRAN version, have been eliminated here.
 *
 * An optional command-line argument has been provided [-c] to
 * offer continuous repetition of the entire benchmark.
 * An optional argument for setting an alternate LOOP count is also
 * provided.  Define PRINTOUT to cause the POUT() function to print
 * outputs at various stages.  Final timing measurements should be
 * made with the PRINTOUT undefined.
 *
 * Questions and comments may be directed to the author at
 *			r.painter@ieee.org
 * )

2VARIABLE T
2VARIABLE T1
2VARIABLE T2

HERE VALUE E1
5 CELLS 2* ALLOT

VARIABLE j
VARIABLE k
VARIABLE l

( whetstones
  parameter:
      iterations
  return:
)
: WHETSTONE
  LOCAL loopstart
  MILLISECONDS 2@ 2LOCAL startsec
  loopstart 12 *
  loopstart 14 *
  loopstart 345 *
  loopstart 210 *
  loopstart 32 *
  loopstart 899 *
  loopstart 616 *
  loopstart 93 *
  0 0
  LOCALS N1 N10 N11 N9 N8 N7 N6 N4 N3 N2 |
  0. 0. 0. -1. -1. -1. 1.
  2LOCALS X1 X2 X3 X4 X Y Z |

  .499975 T 2!
  0.50025 T1 2!
  2.0 T2 2!

(
C
C	Module 1: Simple identifiers
C
)
  N1 0
  DO
     X1 X2 F+ X3 F+ X4 F- T 2@ F* TO X1
     X1 X2 F+ X3 F- X4 F+ T 2@ F* TO X2
     X1 X2 F- X3 F+ X4 F+ T 2@ F* TO X3
     X1 FNEGATE X2 F+ X3 F+ X4 F+ T 2@ F* TO X2
  LOOP

  X4 X3 X2 X1 N1 N1 N1 POUT CALL

(
C
C	Module 2: Array elements
C
)
  1. E1 1 3 GETELEM CALL 2!
  -1. E1 2 3 GETELEM CALL 2!
  -1. E1 3 3 GETELEM CALL 2!
  -1. E1 4 3 GETELEM CALL 2!

  N2 0
  DO
     E1 1 3 GETELEM CALL 2@
     E1 2 3 GETELEM CALL 2@ F+
     E1 3 3 GETELEM CALL 2@ F+
     E1 4 3 GETELEM CALL 2@ F-
     T 2@ F*
     E1 1 3 GETELEM CALL 2!
     E1 1 3 GETELEM CALL 2@
     E1 2 3 GETELEM CALL 2@ F+
     E1 3 3 GETELEM CALL 2@ F-
     E1 4 3 GETELEM CALL 2@ F+
     T 2@ F*
     E1 2 3 GETELEM CALL 2!
     E1 1 3 GETELEM CALL 2@
     E1 2 3 GETELEM CALL 2@ F-
     E1 3 3 GETELEM CALL 2@ F+
     E1 4 3 GETELEM CALL 2@ F+
     T 2@ F*
     E1 3 3 GETELEM CALL 2!
     E1 1 3 GETELEM CALL 2@ FNEGATE
     E1 2 3 GETELEM CALL 2@ F+
     E1 3 3 GETELEM CALL 2@ F+
     E1 4 3 GETELEM CALL 2@ F+
     T 2@ F*
     E1 4 3 GETELEM CALL 2!
  LOOP

  E1 4 3 GETELEM CALL 2@ E1 3 3 GETELEM CALL 2@ E1 2 3 GETELEM CALL 2@ E1 1 3 GETELEM CALL 2@ N2 N3 N2 POUT CALL

(
C
C	Module 3: Array as parameter
C
)
  N3 0
  DO
     E1 PA CALL
  LOOP

  E1 4 3 GETELEM CALL 2@ E1 3 3 GETELEM CALL 2@ E1 2 3 GETELEM CALL 2@ E1 1 3 GETELEM CALL 2@ N2 N2 N3 POUT CALL

(
C
C	Module 4: Conditional jumps
C
)
  1 j !
  N4 0
  DO
     J @ 1 = IF 2 ELSE 3 ENDIF J !
     J @ 2 > IF 0 ELSE 1 ENDIF J !
     J @ 1 < IF 1 ELSE 0 ENDIF J !
  LOOP

  X4 X3 X2 X1 j @ DUP N4 POUT CALL

(
C
C	Module 5: Omitted
C 	Module 6: Integer arithmetic
C
)
  1 J !
  2 k !
  3 l !
  N6 0
  DO
     J @ k @ j @ - * l @ k @ - * j !
     l @ k @ * l @ j @ - - k @ * k !
     l @ k @ - k @ j @ + * l !
     j @ k @ + l @ + S>D D>F E1 l @ 1- 3 GETELEM CALL 2!
     j @ k @ * l @ * S>D D>F E1 k @ 1- 3 GETELEM CALL 2!
  LOOP

  E1 4 3 GETELEM CALL 2@ E1 3 3 GETELEM CALL 2@ E1 2 3 GETELEM CALL 2@ E1 1 3 GETELEM CALL 2@ k @ j @ N6 POUT CALL


(
C
C	Module 7: Trigonometric functions
C
)
  .5 TO X
  .5 TO Y
  N7 0
  DO
     T 2@ T2 2@ X FSIN F* X FCOS F* X Y F+ FCOS X Y F- FCOS F+ 1. F- F/ FATAN F* TO X
     T 2@ T2 2@ Y FSIN F* Y FCOS F* X Y F+ FCOS X Y F- FCOS F+ 1. F- F/ FATAN F* TO Y
I 100 MOD 0= IF [CHAR] . EMIT ENDIF
  LOOP
CR
  Y Y X X k @ j @ N7 POUT CALL

(
C
C	Module 8: Procedure calls
C
)
  1. TO X
  1. TO Y
  1. E1 1 3 GETELEM CALL 2!
  N8 0
  DO
     E1 1 3 GETELEM CALL Y X P3 CALL
  LOOP

  E1 1 3 GETELEM CALL 2@ 2DUP Y X k @ j @ N8 POUT CALL

(
C
C	Module 9: Array references
C
)
  1 j !
  2 k !
  3 l !
  1. E1 1 3 GETELEM CALL 2!
  2. E1 2 3 GETELEM CALL 2!
  3. E1 3 3 GETELEM CALL 2!
  N9 0
  DO
     P0 CALL
  LOOP

  E1 4 3 GETELEM CALL 2@ E1 3 3 GETELEM CALL 2@ E1 2 3 GETELEM CALL 2@ E1 1 3 GETELEM CALL 2@ k @ j @ N9 POUT CALL

(
C
C	Module 10: Integer arithmetic
C
)
  2 j !
  3 k !
  N10 0
  DO
     j @ k @ + j !
     j @ k @ + k !
     k @ j @ - j !
     k @ j @ - j @ - k !
  LOOP

  X4 X3 X2 X1 k @ j @ N10 POUT CALL

(
C
C	Module 11: Standard functions
C
)
  .75 TO X
  N11 0
  DO
     X FLN T1 2@ F/ FEXP FSQRT TO X
I 100 MOD 0= IF [CHAR] . EMIT ENDIF
  LOOP
CR
  X X X X k @ j @ N11 POUT CALL

(
C
C      Stop benchmark timing at this point.
C
)
	MILLISECONDS 2@ startsec D- TO startsec

(
C----------------------------------------------------------------
C      Performance in Whetstone KIP's per second is given by
C
C	(100*LOOP*II)/TIME
C
C      where TIME is in seconds.
C--------------------------------------------------------------------
)
        S" Loops:  " TYPE
        loopstart .
        .(  Duration:  )
        startsec D. .( msec ) CR
        .( FORTH Converted Double Precision Whetstones:  )
        loopstart S>D D>F 100000. F* startsec D>F F/
        2DUP 1000. F< INVERT
        IF
           1000. F/ F. .(  MIPS )
        ELSE
           F. .(  KIPS )
        ENDIF
        CR
;

( indicate an array
  parameter:
      shift
      index
      base address
  return
      address of element
)

:LOCAL GETELEM
       LSHIFT +
;
		 
:LOCAL PA
       LOCAL E
		 
		 6 0 DO
		 E 1 3 GETELEM CALL 2@
		 E 2 3 GETELEM CALL F+
		 E 3 3 GETELEM CALL 2@ F+
		 E 4 3 GETELEM CALL 2@ F-
		 T 2@ F*
		 E 1 3 GETELEM CALL 2!
		 E 1 3 GETELEM CALL 2@
		 E 2 3 GETELEM CALL F+
		 E 3 3 GETELEM CALL 2@ F-
		 E 4 3 GETELEM CALL 2@ F+
		 T 2@ F*
		 E 2 3 GETELEM CALL 2!		 
		 E 1 3 GETELEM CALL 2@
		 E 2 3 GETELEM CALL F-
		 E 3 3 GETELEM CALL 2@ F+
		 E 4 3 GETELEM CALL 2@ F+
		 T 2@ F*
		 E 3 3 GETELEM CALL 2!
		 E 1 3 GETELEM CALL 2@ FNEGATE
		 E 2 3 GETELEM CALL F+
		 E 3 3 GETELEM CALL 2@ F+
		 E 4 3 GETELEM CALL 2@ F+
		 T2 2@ F*
		 E 4 3 GETELEM CALL 2!
		 LOOP
		 ;

:LOCAL P0
       E1 k @ 3 GETELEM CALL 2@ E1 j @ 3 GETELEM 2!
       E1 l @ 3 GETELEM CALL 2@ E1 k @ 3 GETELEM 2!
       E1 j @ 3 GETELEM CALL 2@ E1 l @ 3 GETELEM 2!
;

( 
  parameter:
      double x
      double y
      pointer to double z
  return:
)

:LOCAL P3
       2LOCALS X1 Y1 |
       T 2@ X1 Y1 F+ F* TO X1
       T 2@ X1 Y1 F+ F* TO Y1
       >R
       X1 Y1 F+ T2 2@ F/ R> 2!
;

:LOCAL POUT
       7 .R SPACE
       7 .R SPACE
       7 .R
       FS.. 27 OVER - SPACES TYPE
       FS.. 27 OVER - SPACES TYPE
       FS.. 27 OVER - SPACES TYPE
       FS.. 27 OVER - SPACES TYPE
       CR
;

