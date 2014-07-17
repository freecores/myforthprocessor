( demonstration of bubble sort )

PAGE
.( bubble sort ) CR
.( enter a text line: )
HERE A:R@
40 ACCEPT A:R@ ALLOT CR
R1@ R@ A:2DROP
DUP 0
?DO
   1- OVER OVER 0 ROT ROT 0
	?DO
	    DUP C@
		 OVER 1+ C@
		 >! IF 
		       A:R> A:R1@ A:C!
				 OVER 1+ C!
				 A:R> 1+ R>
			 ELSE
			    2DROP 
			 ENDIF
		 1+
	LOOP
	DROP
	IF
	   .( Iteration  ) I 1+ 3 U.R [CHAR] : EMIT SPACE
	   OVER OVER I 1+ + TYPE CR
	ELSE
	   UNLOOP LEAVE
	ENDIF
LOOP
DROP
TO HERE
.( sorted ) CR
