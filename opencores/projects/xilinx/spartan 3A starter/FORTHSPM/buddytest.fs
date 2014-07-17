.( heap address ) CR
HERE . CR
.( alocate pool and show heap address ) CR
2000 CREATEMM
HERE . CR
.( allocate 480 byte in pool ) CR
480 ALLOCATE DROP
DUP DUP @ . CR CELL+ @ . CR
.( allocate 128 byte in pool ) CR
128 ALLOCATE DROP
DUP DUP @ . CR CELL+ @ . CR
.( free first block ) CR
OVER FREE DROP NIP
.( allocate 240 byte in pool ) CR
240 ALLOCATE DROP
DUP DUP @ . CR CELL+ @ . CR
OVER DUP @ . CR CELL+ @ . CR
.( resize last block to 450 byte ) CR
450 RESIZE DROP
DUP DUP @ . CR CELL+ @ . CR
OVER DUP @ . CR CELL+ @ . CR
2DROP
RELEASEMM



 
