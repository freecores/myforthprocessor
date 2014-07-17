.( heap address ) CR
HERE . CR
.( alocate pool and show heap address ) CR
1000 CREATEMM
HERE . CR
.( allocate 512 byte in pool ) CR
512 ALLOCATE DROP
DUP DUP @ . CR CELL+ @ . CR
.( allocate 128 byte in pool ) CR
128 ALLOCATE DROP
DUP DUP @ . CR CELL+ @ . CR
.( free first block ) CR
OVER FREE DROP NIP
.( allocate 640 byte in pool ) CR
640 ALLOCATE DROP
DUP DUP @ . CR CELL+ @ . CR
OVER DUP @ . CR CELL+ @ . CR
.( resize last block to 240 byte ) CR
240 RESIZE DROP
DUP DUP @ . CR CELL+ @ . CR
OVER DUP @ . CR CELL+ @ . CR
2DROP
RELEASEMM



 
