
\ new statement
0
ThreadState._staticBlocking
!

\ new statement
0
ThreadState._staticThread
!

\ new statement
0
ThreadState.TERMINATED
!

\ new statement
1
ThreadState.RUNNING
!

\ new statement
2
ThreadState.BLOCKED
!

\ new statement
3
ThreadState.WAITING
!

\ new statement
4
ThreadState.SUSPEND
!

\ new statement
0 DUP
2LOCALS x |

\ new statement
FALSE
ThreadState.skip
!

\ new statement
0
ThreadState.next
!

\ new statement

1 VALLOCATE LOCAL §tempvar
10
-846790655 JavaArray§1352878592.table -227194368 EXECUTE-NEW
ThreadState.queue
!

§tempvar SETVTOP

PURGE 1


\ new statement
0
ThreadState.queuelength
!

\ new statement
0
ThreadState.Node§-846770688.table -54178304 EXECUTE-NEW
ThreadState.running
!

\ new statement
0
ThreadState.running  @ CELL+ @ 32 + ( equation.ThreadState.Node.obj ) !

\ new statement
0
ThreadState.running  @ CELL+ @ 48 + ( equation.ThreadState.Node.hash ) !

\ new statement
0
ThreadState.running  @ CELL+ @ 36 + ( equation.ThreadState.Node.data ) !

\ new statement
0
ThreadState.running  @ CELL+ @ 60 + ( equation.ThreadState.Node.datalength ) !

\ new statement
0
ThreadState.running  @ CELL+ @ 64 + ( equation.ThreadState.Node.dataptr ) !

\ new statement
0
ThreadState.running  @ CELL+ @ 40 + ( equation.ThreadState.Node.ret ) !

\ new statement
0
ThreadState.running  @ CELL+ @ 72 + ( equation.ThreadState.Node.retlength ) !

\ new statement
0
ThreadState.running  @ CELL+ @ 68 + ( equation.ThreadState.Node.retptr ) !

\ new statement
0
ThreadState.running  @ CELL+ @ 76 + ( equation.ThreadState.Node.vstack ) !

\ new statement
0
ThreadState.running  @ CELL+ @ 80 + ( equation.ThreadState.Node.vlength ) !

\ new statement
0 S>D
ThreadState.running  @ CELL+ @ 52 + ( equation.ThreadState.Node.time ) 2!

\ new statement

2 VALLOCATE LOCAL §tempvar
ThreadGroup§-2665472.table -543337472 EXECUTE-NEW
DUP §tempvar V!
0
U" System " COUNT JavaArray.createUnicode§-675323136 DUP §tempvar 4 + V!
0 S>D
Thread§-1457236992.table 708269056 EXECUTE-NEW
ThreadState.running  @ CELL+ @ 28 + ( equation.ThreadState.Node.t ) !


§tempvar SETVTOP

PURGE 1


\ new statement
ThreadState.RUNNING
@
ThreadState.running  @ CELL+ @ 44 + ( equation.ThreadState.Node.state ) !

\ new statement
MILLISECONDS 2@ TO x

\ new statement
x
ThreadState.lastTime
2!

\ new statement
0

\ new statement

PURGE 1

\ new statement
DROP

\ new statement
0
ThreadState.Node._staticBlocking
!

\ new statement
0
ThreadState.Node._staticThread
!
