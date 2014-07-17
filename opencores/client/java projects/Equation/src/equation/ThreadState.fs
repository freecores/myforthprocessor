MODULENAME equation.ThreadState
( *
)


:LOCAL ThreadState.ThreadState§-1772465152
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
   ThreadState§1676825600.table OVER 12 + !
   1676804096 OVER 20 + !
   " ThreadState " OVER 16 + !
   0 OVER 24 + ! DROP
   
   \ new statement
   §this DUP 0 V! ( return object )    
   \ new statement
   0 §break28533 LABEL
   
   \ new statement
   §base0 SETVTOP
   PURGE 2
   
   \ new statement
   DROP
;
( *
* append a thread to the scheduler table
* @param t Thread to append
)

: ThreadState.append§1533632768
   1 VALLOCATE LOCAL §base0
   DUP 0 §base0 + V! LOCAL t
   
   \ new statement
   
   \ new statement
   t
   0<>
   
   \ new statement
   IF
      
      \ new statement
      1 VALLOCATE LOCAL §base1
      0 DUP DUP
      LOCALS l n y |
      
      \ new statement
      ThreadState.queue
      @
      LOCAL §synchronized0
      ThreadState.Node._staticThread  @
      §synchronized0 801271040 TRUE ( Object.monitorEnter ) EXECUTE-METHOD
      
      \ new statement
      t CELL+ @ 64 + ( equation.Thread.stackSize )       @
      0=
      IF
         4000
         
      ELSE
         t CELL+ @ 64 + ( equation.Thread.stackSize )          @
         
      ENDIF
      TO y
      
      \ new statement
      0
      ThreadState.Node§-846770688.table -54178304 EXECUTE-NEW
      DUP §base1 V! TO n
      
      \ new statement
      t
      n CELL+ @ 28 + ( equation.ThreadState.Node.t )       !
      
      \ new statement
      ThreadState.RUNNING
      @
      n CELL+ @ 44 + ( equation.ThreadState.Node.state )       !
      
      \ new statement
      0
      n CELL+ @ 32 + ( equation.ThreadState.Node.obj )       !
      
      \ new statement
      0
      n CELL+ @ 48 + ( equation.ThreadState.Node.hash )       !
      
      \ new statement
      0 S>D
      n CELL+ @ 52 + ( equation.ThreadState.Node.time )       2!
      
      \ new statement
      
      1 VALLOCATE LOCAL §tempvar
      y 262145 JavaArray§1352878592.table -227194368 EXECUTE-NEW
      n CELL+ @ 36 + ( equation.ThreadState.Node.data )       !
      
      §tempvar SETVTOP
      
      PURGE 1
      
      
      \ new statement
      \  initialize data stack with parameters of run
      
      
      \ new statement
      n CELL+ @ 36 + ( equation.ThreadState.Node.data ) @ >R
      y
      1
      -
      R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
      TO l
      
      \ new statement
      t l -1 OVER 4 - DUP TO l ! !
      
      \ new statement
      l
      n CELL+ @ 64 + ( equation.ThreadState.Node.dataptr )       !
      
      \ new statement
      2
      n CELL+ @ 60 + ( equation.ThreadState.Node.datalength )       !
      
      \ new statement
      
      1 VALLOCATE LOCAL §tempvar
      y 262145 JavaArray§1352878592.table -227194368 EXECUTE-NEW
      n CELL+ @ 40 + ( equation.ThreadState.Node.ret )       !
      
      §tempvar SETVTOP
      
      PURGE 1
      
      
      \ new statement
      n CELL+ @ 40 + ( equation.ThreadState.Node.ret ) @ >R
      y
      1
      -
      R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
      DUP
      TO l
      n CELL+ @ 68 + ( equation.ThreadState.Node.retptr )       !
      
      \ new statement
      \  put address of run on stack
      
      
      \ new statement
      706638336 t
      BEGIN
         0<>! OVER CELL+ @ 4 + @ OVER OVER 0<> AND
      WHILE
         NIP NIP
      REPEAT
      2DROP
      
      \ new statement
      BEGIN
         0<>!
      WHILE
         OVER OVER CELL+ @ 12 + @ -4 SEARCH-METHOD
         
         \ new statement
         0<>!
         IF
            l ! DROP 0
         ELSE
            DROP CELL+ @ @
         ENDIF
      REPEAT
      2DROP
      
      \ new statement
      ADDCONSOLE
      
      \ new statement
      1
      n CELL+ @ 72 + ( equation.ThreadState.Node.retlength )       !
      
      \ new statement
      0
      n CELL+ @ 76 + ( equation.ThreadState.Node.vstack )       !
      
      \ new statement
      0
      n CELL+ @ 80 + ( equation.ThreadState.Node.vlength )       !
      
      \ new statement
      ThreadState.queuelength
      @
      ThreadState.queue  @ CELL+ @ 28 + ( equation.JavaArray.length )       @
      =
      
      \ new statement
      IF
         
         \ new statement
         ThreadState.queue  @ >R
         ThreadState.queuelength
         @
         10
         +
         R> 917664256 TRUE ( equation.JavaArray.resize§917664256 ) EXECUTE-METHOD
         
         \ new statement
      ENDIF
      
      \ new statement
      
      0
      LOCALS 0§ |
      
      ThreadState.queuelength
      A:R@ @ DUP 1+ R@ !
      A:DROP
      TO 0§
      n
      ThreadState.queue  @ >R
      0§
      R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
      !
      
      PURGE 1
      
      
      \ new statement
      0 §break28488 LABEL
      
      \ new statement
      §synchronized0 1974299904 TRUE ( Object.monitorLeave ) EXECUTE-METHOD
      PURGE 1
      
      \ new statement
      
      
      
      §base1 SETVTOP
      PURGE 4
      
      \ new statement
      DROP
      
      \ new statement
   ENDIF
   
   \ new statement
   0 §break28487 LABEL
   
   \ new statement
   
   §base0 SETVTOP
   PURGE 2
   
   \ new statement
   DROP
;
( * get currently running thread
* @return Thread the running thread
)

: ThreadState.currentThread§-1671470336
   
   \ new statement
   
   \ new statement
   ThreadState.running  @ CELL+ @ 28 + ( equation.ThreadState.Node.t )    @
   DUP 0 V!
   0 §break28486 BRANCH
   
   \ new statement
   0 §break28486 LABEL
   
   \ new statement
   
   \ new statement
   DROP
;
( *
* exit and remove thread from scheduler
* @param t Thread the thread
)

: ThreadState.exit§-1797888768
   1 VALLOCATE LOCAL §base0
   DUP 0 §base0 + V! LOCAL t
   
   \ new statement
   
   \ new statement
   ThreadState.running  @ CELL+ @ 28 + ( equation.ThreadState.Node.t )    @
   t
   =
   
   \ new statement
   IF
      
      \ new statement
      
      \ new statement
      SUBCONSOLE
      
      \ new statement
      TRUE
      ThreadState.skip
      !
      
      \ new statement
      ThreadState.TERMINATED
      @
      0
      0
      0 S>D
      ThreadState.switchTask§321549056
      
      \ new statement
      0 §break28490 LABEL
      
      \ new statement
      
      \ new statement
      DROP
      
      \ new statement
   ELSE
      
      \ new statement
      0
      LOCALS i |
      
      \ new statement
      ThreadState.queue
      @
      LOCAL §synchronized0
      ThreadState.Node._staticThread  @
      §synchronized0 801271040 TRUE ( Object.monitorEnter ) EXECUTE-METHOD
      
      \ new statement
      
      \ new statement
      0
      TO i
      
      \ new statement
      BEGIN
         
         \ new statement
         i
         ThreadState.queuelength
         @
         < and_29702 0BRANCH! DROP
         ThreadState.queue  @ >R
         i
         R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
         CELL+ @ 28 + ( equation.ThreadState.Node.t )          @
         t
         <>
         and_29702 LABEL
         
         \ new statement
      WHILE
         
         \ new statement
         
         \ new statement
         i 1+ TO i
         
         \ new statement
      REPEAT
      
      \ new statement
      0 §break28492 LABEL
      
      \ new statement
      
      \ new statement
      DROP
      
      \ new statement
      i
      ThreadState.queuelength
      @
      <
      
      \ new statement
      IF
         
         \ new statement
         1 VALLOCATE LOCAL §base2
         0
         LOCALS x |
         
         \ new statement
         SUBCONSOLE
         
         \ new statement
         ThreadState.queue  @ >R
         i
         R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
         @  " Node " CASTTO
         DUP §base2 V! TO x
         
         \ new statement
         0
         LOCALS j |
         
         \ new statement
         i
         1
         +
         TO j
         
         \ new statement
         BEGIN
            
            \ new statement
            j
            ThreadState.queuelength
            @
            <
            
            \ new statement
         WHILE
            
            \ new statement
            
            0
            LOCALS 0§ |
            
            i DUP 1+ TO i
            TO 0§
            ThreadState.queue  @ >R
            j DUP 1+ TO j
            R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
            @  " Node " CASTTO
            ThreadState.queue  @ >R
            0§
            R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
            !
            
            PURGE 1
            
            
            \ new statement
            
            \ new statement
         REPEAT
         
         \ new statement
         0 §break28494 LABEL
         
         \ new statement
         
         PURGE 1
         
         \ new statement
         DROP
         
         \ new statement
         
         0
         LOCALS 0§ |
         
         i DUP
         ThreadState.queuelength
         ! TO 0§
         0
         ThreadState.queue  @ >R
         0§
         R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
         !
         
         PURGE 1
         
         
         \ new statement
         x CELL+ @ 76 + ( equation.ThreadState.Node.vstack )          @ DUP
         TO i
         0<>
         
         \ new statement
         IF
            
            \ new statement
            1 VALLOCATE LOCAL §base3
            0 DUP
            LOCALS o y |
            
            \ new statement
            SWEEP @ TO y
            
            \ new statement
            0
            LOCALS k |
            
            \ new statement
            0
            TO k
            
            \ new statement
            BEGIN
               
               \ new statement
               k
               x CELL+ @ 80 + ( equation.ThreadState.Node.vlength )                @
               < and_29735 0BRANCH! DROP
               y
               0=
               and_29735 LABEL
               
               \ new statement
            WHILE
               
               \ new statement
               i CELL+ @ k CELLS + @ TO o
               
               \ new statement
               o
               -1
               JavaArray.kill§1620077312
               
               \ new statement
               
               \ new statement
               k 1+ TO k
               
               \ new statement
            REPEAT
            
            \ new statement
            0 §break28496 LABEL
            
            \ new statement
            
            PURGE 1
            
            \ new statement
            DROP
            
            \ new statement
            i 1 OVER ! TO o
            
            \ new statement
            \  mark list reference as white for GC
            
            
            \ new statement
            0 §break28495 LABEL
            
            \ new statement
            
            
            §base3 SETVTOP
            PURGE 3
            
            \ new statement
            DROP
            
            \ new statement
         ENDIF
         
         \ new statement
         0 §break28493 LABEL
         
         \ new statement
         
         §base2 SETVTOP
         PURGE 2
         
         \ new statement
         DROP
         
         \ new statement
      ENDIF
      
      \ new statement
      0 §break28491 LABEL
      
      \ new statement
      §synchronized0 1974299904 TRUE ( Object.monitorLeave ) EXECUTE-METHOD
      PURGE 1
      
      \ new statement
      
      PURGE 1
      
      \ new statement
      DROP
      
      \ new statement
   ENDIF
   
   \ new statement
   0 §break28489 LABEL
   
   \ new statement
   
   §base0 SETVTOP
   PURGE 2
   
   \ new statement
   DROP
;
( *
* get state of thread
* @param t Thread looking for its state
* @return int state
)

: ThreadState.getState§366110464
   1 VALLOCATE LOCAL §base0
   DUP 0 §base0 + V! LOCAL t
   
   \ new statement
   
   \ new statement
   
   \ new statement
   ThreadState.queue
   @
   LOCAL §synchronized0
   ThreadState.Node._staticThread  @
   §synchronized0 801271040 TRUE ( Object.monitorEnter ) EXECUTE-METHOD
   
   \ new statement
   1 VALLOCATE LOCAL §base2
   0 DUP
   LOCALS i x |
   
   \ new statement
   0
   TO i
   
   \ new statement
   BEGIN
      
      \ new statement
      i
      ThreadState.queuelength
      @
      <
      
      \ new statement
   WHILE
      
      \ new statement
      ThreadState.queue  @ >R
      i
      R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
      @  " Node " CASTTO
      DUP §base2 V! TO x
      
      \ new statement
      x CELL+ @ 28 + ( equation.ThreadState.Node.t )       @
      t
      =
      
      \ new statement
      IF
         
         \ new statement
         x CELL+ @ 44 + ( equation.ThreadState.Node.state )          @
         131074 §break28531 BRANCH
         
         \ new statement
      ENDIF
      
      \ new statement
      
      \ new statement
      i 1+ TO i
      
      \ new statement
   REPEAT
   
   \ new statement
   0 §break28531 LABEL
   
   \ new statement
   
   
   §base2 SETVTOP
   PURGE 3
   
   \ new statement
   0>!
   IF
      10001H - §break28530 BRANCH
   ENDIF
   DROP
   
   \ new statement
   ThreadState.running  @ CELL+ @ 28 + ( equation.ThreadState.Node.t )    @
   t
   =
   IF
      ThreadState.running  @ CELL+ @ 44 + ( equation.ThreadState.Node.state )       @
   ELSE
      ThreadState.TERMINATED
      @
      
   ENDIF
   65537 §break28530 BRANCH
   
   \ new statement
   0 §break28530 LABEL
   
   \ new statement
   §synchronized0 1974299904 TRUE ( Object.monitorLeave ) EXECUTE-METHOD
   PURGE 1
   
   \ new statement
   
   \ new statement
   0>!
   IF
      10001H - §break28529 BRANCH
   ENDIF
   DROP
   
   \ new statement
   0 §break28529 LABEL
   
   \ new statement
   
   §base0 SETVTOP
   PURGE 2
   
   \ new statement
   DROP
;
( *
* switch to next task
* @param next Node desired task
)

: ThreadState.switchTask§-1287097600
   
   \ new statement
   0 DUP
   LOCALS len ptr |
   
   \ new statement
   \  save data stack
   
   
   \ new statement
   ThreadState.next  @ CELL+ @ 64 + ( equation.ThreadState.Node.dataptr )    @
   TO ptr
   
   \ new statement
   ThreadState.next  @ CELL+ @ 60 + ( equation.ThreadState.Node.datalength )    @
   TO len
   
   \ new statement
   DI DEPTH >R SAVE SP@ A:R@
   
   \ new statement
   \  save data stack
   
   
   \ new statement
   len ptr SP!
   
   \ new statement
   \  assign new data stack
   
   
   \ new statement
   R@ R1@ A:2DROP TO len TO ptr
   
   \ new statement
   ptr
   ThreadState.running  @ CELL+ @ 64 + ( equation.ThreadState.Node.dataptr )    !
   
   \ new statement
   len
   ThreadState.running  @ CELL+ @ 60 + ( equation.ThreadState.Node.datalength )    !
   
   \ new statement
   ThreadState.next  @ CELL+ @ 68 + ( equation.ThreadState.Node.retptr )    @
   TO ptr
   
   \ new statement
   ThreadState.next  @ CELL+ @ 72 + ( equation.ThreadState.Node.retlength )    @
   TO len
   
   \ new statement
   len ptr A:2DROP
   
   \ new statement
   \  move local variables
   
   
   \ new statement
   A:DEPTH R> A:SAVE A:SP@ R@
   
   \ new statement
   \  save return stack
   
   
   \ new statement
   3 PICK A:R@ 3 PICK A:R@ A:SP! 2DROP
   
   \ new statement
   \  assign new return stack
   
   
   \ new statement
   A:0 A:0
   
   \ new statement
   \  dummies for local variables
   
   
   \ new statement
   TO ptr TO len 2DROP
   
   \ new statement
   \  restore local variables
   
   
   \ new statement
   ptr
   ThreadState.running  @ CELL+ @ 68 + ( equation.ThreadState.Node.retptr )    !
   
   \ new statement
   len
   ThreadState.running  @ CELL+ @ 72 + ( equation.ThreadState.Node.retlength )    !
   
   \ new statement
   VLENGTH @ TO len VSTACK @ TO ptr
   
   \ new statement
   \  save list of references
   
   
   \ new statement
   ptr
   ThreadState.running  @ CELL+ @ 76 + ( equation.ThreadState.Node.vstack )    !
   
   \ new statement
   len
   ThreadState.running  @ CELL+ @ 80 + ( equation.ThreadState.Node.vlength )    !
   
   \ new statement
   ThreadState.next  @ CELL+ @ 76 + ( equation.ThreadState.Node.vstack )    @
   TO ptr
   
   \ new statement
   ThreadState.next  @ CELL+ @ 80 + ( equation.ThreadState.Node.vlength )    @
   TO len
   
   \ new statement
   ptr VSTACK ! len VLENGTH !
   
   \ new statement
   \  new list of references
   
   
   \ new statement
   ThreadState.running  @ CELL+ @ 76 + ( equation.ThreadState.Node.vstack )    @
   TO ptr
   
   \ new statement
   ThreadState.next  @ CELL+ @ 28 + ( equation.ThreadState.Node.t )    @
   ThreadState.time§-1155435520
   S>D
   ThreadState.next  @ CELL+ @ 52 + ( equation.ThreadState.Node.time )    2!
   
   \ new statement
   ThreadState.skip
   @ and_29826 0BRANCH! DROP
   ptr
   0<>
   and_29826 LABEL
   
   \ new statement
   IF
      
      \ new statement
      1 VALLOCATE LOCAL §base1
      0
      LOCALS o |
      
      \ new statement
      SWEEP @ TO len
      
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
         ThreadState.running  @ CELL+ @ 80 + ( equation.ThreadState.Node.vlength )          @
         < and_29834 0BRANCH! DROP
         len
         0=
         and_29834 LABEL
         
         \ new statement
      WHILE
         
         \ new statement
         ptr CELL+ @ i CELLS + @ TO o
         
         \ new statement
         o
         -1
         JavaArray.kill§1620077312
         
         \ new statement
         
         \ new statement
         i 1+ TO i
         
         \ new statement
      REPEAT
      
      \ new statement
      0 §break28518 LABEL
      
      \ new statement
      
      PURGE 1
      
      \ new statement
      DROP
      
      \ new statement
      ptr 1 OVER ! TO o
      
      \ new statement
      0 §break28517 LABEL
      
      \ new statement
      
      §base1 SETVTOP
      PURGE 2
      
      \ new statement
      DROP
      
      \ new statement
   ENDIF
   
   \ new statement
   ThreadState.next
   @
   ThreadState.running
   !
   
   \ new statement
   0
   ThreadState.next
   !
   
   \ new statement
   0
   ThreadState.queue  @ CELL+ @ @ CELL+ @ 32 + ( equation.Object._dynamicBlocking )    !
   
   \ new statement
   0
   ThreadState.queue  @ CELL+ @ @ CELL+ @ 28 + ( equation.Object._dynamicThread )    !
   
   \ new statement
   ThreadState.WAITING
   @
   ThreadState.queue
   @
   TRUE
   ThreadState.unblockTask§1685746944
   
   \ new statement
   FALSE
   ThreadState.skip
   !
   
   \ new statement
   EI
   
   \ new statement
   0 §break28516 LABEL
   
   \ new statement
   
   
   PURGE 2
   
   \ new statement
   DROP
;


: ThreadState.switchTask§2070835968
   2LOCAL time
   LOCAL hash
   LOCAL state
   
   \ new statement
   
   \ new statement
   state
   0
   hash
   time
   ThreadState.switchTask§321549056
   
   \ new statement
   0 §break28510 LABEL
   
   \ new statement
   
   
   
   PURGE 3
   
   \ new statement
   DROP
;
( *
* change state of running task and switch to next task
* @param state int     new state
* @param obj Object    concerned object
* @param hash int      hashcode
* @param time long     time to remain in new state
)

: ThreadState.switchTask§321549056
   1 VALLOCATE LOCAL §base0
   2LOCAL time
   LOCAL hash
   DUP 0 §base0 + V! LOCAL obj
   LOCAL state
   
   \ new statement
   0 DUP
   LOCALS f pos |
   0 DUP 2DUP
   2LOCALS subtract y |
   
   \ new statement
   MILLISECONDS 2@ TO y
   
   \ new statement
   y
   ThreadState.lastTime
   2@
   D-
   TO subtract
   
   \ new statement
   y
   ThreadState.lastTime
   2!
   
   \ new statement
   ThreadState.running  @ CELL+ @ 52 + ( equation.ThreadState.Node.time )    2@
   subtract
   D-
   ThreadState.running  @ CELL+ @ 52 + ( equation.ThreadState.Node.time )    2!
   
   \ new statement
   sw1 LABEL DI
   
   \ new statement
   ThreadState.queue  @ CELL+ @ @ CELL+ @ 32 + ( equation.Object._dynamicBlocking )    @
   0=
   
   \ new statement
   IF
      
      \ new statement
      
      \ new statement
      ThreadState.running  @ CELL+ @ 28 + ( equation.ThreadState.Node.t )       @
      ThreadState.queue  @ CELL+ @ @ CELL+ @ 28 + ( equation.Object._dynamicThread )       !
      
      \ new statement
      1
      ThreadState.queue  @ CELL+ @ @ CELL+ @ 32 + ( equation.Object._dynamicBlocking )       !
      
      \ new statement
      0 §break28499 LABEL
      
      \ new statement
      
      \ new statement
      DROP
      
      \ new statement
   ELSE
      
      \ new statement
      
      \ new statement
      EI NOP NOP NOP NOP NOP NOP NOP NOP sw1 BRANCH
      
      \ new statement
      0 §break28500 LABEL
      
      \ new statement
      
      \ new statement
      DROP
      
      \ new statement
   ENDIF
   
   \ new statement
   EI
   
   \ new statement
   0
   ThreadState.next
   !
   
   \ new statement
   ThreadState.queuelength
   @
   TO pos
   
   \ new statement
   TRUE
   TO f
   
   \ new statement
   1 VALLOCATE LOCAL §base1
   0 DUP
   LOCALS i x |
   
   \ new statement
   0
   TO i
   
   \ new statement
   BEGIN
      
      \ new statement
      i
      ThreadState.queuelength
      @
      <
      
      \ new statement
   WHILE
      
      \ new statement
      ThreadState.queue  @ >R
      i
      R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
      @  " Node " CASTTO
      DUP §base1 V! TO x
      
      \ new statement
      x CELL+ @ 52 + ( equation.ThreadState.Node.time )       2@
      DNEGATE D0< and_29896 0BRANCH! DROP
      subtract
      DNEGATE D0<
      and_29896 LABEL
      
      \ new statement
      IF
         
         \ new statement
         x CELL+ @ 52 + ( equation.ThreadState.Node.time )          2@
         subtract
         D- OVER OVER
         x CELL+ @ 52 + ( equation.ThreadState.Node.time )          2!
         DNEGATE DROP 0< INVERT
         
         \ new statement
         IF
            
            \ new statement
            
            \ new statement
            ThreadState.RUNNING
            @
            x CELL+ @ 44 + ( equation.ThreadState.Node.state )             !
            
            \ new statement
            0
            x CELL+ @ 32 + ( equation.ThreadState.Node.obj )             !
            
            \ new statement
            0
            x CELL+ @ 48 + ( equation.ThreadState.Node.hash )             !
            
            \ new statement
            0 S>D
            x CELL+ @ 52 + ( equation.ThreadState.Node.time )             2!
            
            \ new statement
            0 §break28503 LABEL
            
            \ new statement
            
            \ new statement
            DROP
            
            \ new statement
         ENDIF
         
         \ new statement
      ENDIF
      
      \ new statement
      f and_29911 0BRANCH! DROP
      x CELL+ @ 44 + ( equation.ThreadState.Node.state )       @
      ThreadState.RUNNING
      @
      =
      and_29911 0BRANCH! DROP
      x CELL+ @ 52 + ( equation.ThreadState.Node.time )       2@
      D0=
      and_29911 LABEL
      
      \ new statement
      IF
         
         \ new statement
         
         \ new statement
         i
         TO pos
         
         \ new statement
         x
         ThreadState.next
         !
         
         \ new statement
         FALSE
         TO f
         
         \ new statement
         0 §break28504 LABEL
         
         \ new statement
         
         \ new statement
         DROP
         
         \ new statement
      ENDIF
      
      \ new statement
      
      \ new statement
      i 1+ TO i
      
      \ new statement
   REPEAT
   
   \ new statement
   0 §break28501 LABEL
   
   \ new statement
   
   
   §base1 SETVTOP
   PURGE 3
   
   \ new statement
   DROP
   
   \ new statement
   f 0=
   
   \ new statement
   IF
      
      \ new statement
      
      \ new statement
      0
      LOCALS j |
      
      \ new statement
      pos
      1
      +
      TO j
      
      \ new statement
      BEGIN
         
         \ new statement
         j
         ThreadState.queuelength
         @
         <
         
         \ new statement
      WHILE
         
         \ new statement
         
         0
         LOCALS 0§ |
         
         pos DUP 1+ TO pos
         TO 0§
         ThreadState.queue  @ >R
         j DUP 1+ TO j
         R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
         @  " Node " CASTTO
         ThreadState.queue  @ >R
         0§
         R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
         !
         
         PURGE 1
         
         
         \ new statement
         
         \ new statement
      REPEAT
      
      \ new statement
      0 §break28506 LABEL
      
      \ new statement
      
      PURGE 1
      
      \ new statement
      DROP
      
      \ new statement
      state
      ThreadState.running  @ CELL+ @ 44 + ( equation.ThreadState.Node.state )       !
      
      \ new statement
      obj
      ThreadState.running  @ CELL+ @ 32 + ( equation.ThreadState.Node.obj )       !
      
      \ new statement
      hash
      ThreadState.running  @ CELL+ @ 48 + ( equation.ThreadState.Node.hash )       !
      
      \ new statement
      time
      ThreadState.running  @ CELL+ @ 52 + ( equation.ThreadState.Node.time )       2!
      
      \ new statement
      ThreadState.skip
      @
      
      \ new statement
      IF
         
         \ new statement
         
         \ new statement
         ThreadState.queuelength
         A:R@ @ 1- R@ !
         A:DROP
         
         \ new statement
         
         0
         LOCALS 0§ |
         
         pos TO 0§
         0
         ThreadState.queue  @ >R
         0§
         R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
         !
         
         PURGE 1
         
         
         \ new statement
         0 §break28507 LABEL
         
         \ new statement
         
         \ new statement
         DROP
         
         \ new statement
      ELSE
         
         \ new statement
         
         0
         LOCALS 0§ |
         
         pos TO 0§
         ThreadState.running
         @
         ThreadState.queue  @ >R
         0§
         R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
         !
         
         PURGE 1
         
         
         \ new statement
      ENDIF
      
      \ new statement
      ThreadState.switchTask§-1287097600
      
      \ new statement
      0 §break28505 LABEL
      
      \ new statement
      
      \ new statement
      DROP
      
      \ new statement
   ELSE
      
      \ new statement
      
      \ new statement
      ThreadState.running  @ CELL+ @ 52 + ( equation.ThreadState.Node.time )       2@
      DNEGATE DROP 0< INVERT
      
      \ new statement
      IF
         
         \ new statement
         ThreadState.running  @ CELL+ @ 28 + ( equation.ThreadState.Node.t )          @
         ThreadState.time§-1155435520
         S>D
         ThreadState.running  @ CELL+ @ 52 + ( equation.ThreadState.Node.time )          2!
         
         \ new statement
      ENDIF
      
      \ new statement
      DI
      
      \ new statement
      0
      ThreadState.queue  @ CELL+ @ @ CELL+ @ 32 + ( equation.Object._dynamicBlocking )       !
      
      \ new statement
      0
      ThreadState.queue  @ CELL+ @ @ CELL+ @ 28 + ( equation.Object._dynamicThread )       !
      
      \ new statement
      ThreadState.WAITING
      @
      ThreadState.queue
      @
      TRUE
      ThreadState.unblockTask§1685746944
      
      \ new statement
      EI
      
      \ new statement
      0 §break28508 LABEL
      
      \ new statement
      
      \ new statement
      DROP
      
      \ new statement
   ENDIF
   
   \ new statement
   0
   ThreadState.next
   !
   
   \ new statement
   0 §break28498 LABEL
   
   \ new statement
   
   
   
   
   
   
   
   
   §base0 SETVTOP
   PURGE 9
   
   \ new statement
   DROP
;


: ThreadState.switchTask§416117504
   1 VALLOCATE LOCAL §base0
   2LOCAL time
   DUP 0 §base0 + V! LOCAL obj
   LOCAL state
   
   \ new statement
   
   \ new statement
   state
   obj
   0
   time
   ThreadState.switchTask§321549056
   
   \ new statement
   0 §break28509 LABEL
   
   \ new statement
   
   
   
   §base0 SETVTOP
   PURGE 4
   
   \ new statement
   DROP
;
( *
* change state of distinct thread
* @param state int     new state
* @param t Thread      the thread
)

: ThreadState.switchTask§874672896
   1 VALLOCATE LOCAL §base0
   DUP 0 §base0 + V! LOCAL t
   LOCAL state
   
   \ new statement
   
   \ new statement
   ThreadState.running  @ CELL+ @ 28 + ( equation.ThreadState.Node.t )    @
   t
   =
   
   \ new statement
   IF
      
      \ new statement
      state
      0
      0
      0 S>D
      ThreadState.switchTask§321549056
      
      \ new statement
   ELSE
      
      \ new statement
      
      \ new statement
      ThreadState.queue
      @
      LOCAL §synchronized0
      ThreadState.Node._staticThread  @
      §synchronized0 801271040 TRUE ( Object.monitorEnter ) EXECUTE-METHOD
      
      \ new statement
      1 VALLOCATE LOCAL §base2
      0 DUP
      LOCALS i x |
      
      \ new statement
      0
      TO i
      
      \ new statement
      BEGIN
         
         \ new statement
         i
         ThreadState.queuelength
         @
         <
         
         \ new statement
      WHILE
         
         \ new statement
         ThreadState.queue  @ >R
         i
         R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
         @  " Node " CASTTO
         DUP §base2 V! TO x
         
         \ new statement
         x CELL+ @ 28 + ( equation.ThreadState.Node.t )          @
         t
         =
         
         \ new statement
         IF
            
            \ new statement
            
            \ new statement
            state
            x CELL+ @ 44 + ( equation.ThreadState.Node.state )             !
            
            \ new statement
            0
            x CELL+ @ 32 + ( equation.ThreadState.Node.obj )             !
            
            \ new statement
            0
            x CELL+ @ 48 + ( equation.ThreadState.Node.hash )             !
            
            \ new statement
            0 S>D
            x CELL+ @ 52 + ( equation.ThreadState.Node.time )             2!
            
            \ new statement
            65537 §break28515 BRANCH
            
            \ new statement
            0 §break28515 LABEL
            
            \ new statement
            
            \ new statement
            0>!
            IF
               10001H - 0<! §break28513 0BRANCH
            ENDIF
            DROP
            
            \ new statement
         ENDIF
         
         \ new statement
         
         \ new statement
         i 1+ TO i
         
         \ new statement
      REPEAT
      
      \ new statement
      0 §break28513 LABEL
      
      \ new statement
      
      
      §base2 SETVTOP
      PURGE 3
      
      \ new statement
      DROP
      
      \ new statement
      0 §break28512 LABEL
      
      \ new statement
      §synchronized0 1974299904 TRUE ( Object.monitorLeave ) EXECUTE-METHOD
      PURGE 1
      
      \ new statement
      
      \ new statement
      DROP
      
      \ new statement
   ENDIF
   
   \ new statement
   0 §break28511 LABEL
   
   \ new statement
   
   
   §base0 SETVTOP
   PURGE 3
   
   \ new statement
   DROP
;
( *
* calculate duration of time slot
* @param t Thread the thread
* @return int time in msec
)

: ThreadState.time§-1155435520
   1 VALLOCATE LOCAL §base0
   DUP 0 §base0 + V! LOCAL t
   
   \ new statement
   
   \ new statement
   t >R
   R> -487889152 TRUE ( equation.Thread.getPriority§-487889152 ) EXECUTE-METHOD
   Thread.MIN_PRIORITY
   @
   -
   1
   +
   S>D
   5
   SHIFTL  D>S
   0 §break28520 BRANCH
   
   \ new statement
   0 §break28520 LABEL
   
   \ new statement
   
   §base0 SETVTOP
   PURGE 2
   
   \ new statement
   DROP
;
( *
* interrupt service routine from scheduler
)

: ThreadState.timer§391541760
   
   \ new statement
   0 DUP 2DUP
   2LOCALS subtract x |
   
   \ new statement
   MILLISECONDS 2@ TO x
   
   \ new statement
   x
   ThreadState.lastTime
   2@
   D-
   TO subtract
   
   \ new statement
   ThreadState.queue  @ CELL+ @ @ CELL+ @ 28 + ( equation.Object._dynamicThread )    @
   0= and_30032 0BRANCH! DROP
   subtract
   ThreadState.running  @ CELL+ @ 52 + ( equation.ThreadState.Node.time )    2@
   D< INVERT
   and_30032 LABEL
   
   \ new statement
   IF
      
      \ new statement
      ThreadState.RUNNING
      @
      0
      0
      0 S>D
      ThreadState.switchTask§321549056
      
      \ new statement
   ENDIF
   
   \ new statement
   0 §break28528 LABEL
   
   \ new statement
   
   
   PURGE 2
   
   \ new statement
   DROP
;


: ThreadState.unblockTask§-2137950976
   LOCAL all
   LOCAL hash
   LOCAL state
   
   \ new statement
   
   \ new statement
   state
   0
   hash
   all
   ThreadState.unblockTask§1948218624
   
   \ new statement
   0 §break28527 LABEL
   
   \ new statement
   
   
   
   PURGE 3
   
   \ new statement
   DROP
;


: ThreadState.unblockTask§1685746944
   1 VALLOCATE LOCAL §base0
   LOCAL all
   DUP 0 §base0 + V! LOCAL obj
   LOCAL state
   
   \ new statement
   
   \ new statement
   state
   obj
   0
   all
   ThreadState.unblockTask§1948218624
   
   \ new statement
   0 §break28526 LABEL
   
   \ new statement
   
   
   
   §base0 SETVTOP
   PURGE 4
   
   \ new statement
   DROP
;
( *
* unblock task(s) waiting for an object
* @param state int   current state
* @param obj Object  the object
* @param hash int    hash code
* @param all boolean if true, unblock all tasks waiting for obj, otherwise only nearest task
)

: ThreadState.unblockTask§1948218624
   1 VALLOCATE LOCAL §base0
   LOCAL all
   LOCAL hash
   DUP 0 §base0 + V! LOCAL obj
   LOCAL state
   
   \ new statement
   
   \ new statement
   0
   LOCALS adv |
   
   \ new statement
   ThreadState.queue
   @
   LOCAL §synchronized0
   ThreadState.Node._staticThread  @
   §synchronized0 801271040 TRUE ( Object.monitorEnter ) EXECUTE-METHOD
   
   \ new statement
   TRUE
   TO adv
   
   \ new statement
   1 VALLOCATE LOCAL §base2
   0 DUP
   LOCALS i x |
   
   \ new statement
   0
   TO i
   
   \ new statement
   BEGIN
      
      \ new statement
      i
      ThreadState.queuelength
      @
      < and_30057 0BRANCH! DROP
      adv
      and_30057 LABEL
      
      \ new statement
   WHILE
      
      \ new statement
      ThreadState.queue  @ >R
      i
      R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
      @  " Node " CASTTO
      DUP §base2 V! TO x
      
      \ new statement
      x CELL+ @ 44 + ( equation.ThreadState.Node.state )       @
      state
      = and_30061 0BRANCH! DROP
      x CELL+ @ 32 + ( equation.ThreadState.Node.obj )       @
      obj
      =  0=! or_30062 0BRANCH DROP
      x CELL+ @ 48 + ( equation.ThreadState.Node.hash )       @
      hash
      = and_30063 0BRANCH! DROP
      hash
      0<>
      and_30063 LABEL
      or_30062 LABEL
      and_30061 LABEL
      
      \ new statement
      IF
         
         \ new statement
         
         \ new statement
         ThreadState.RUNNING
         @
         x CELL+ @ 44 + ( equation.ThreadState.Node.state )          !
         
         \ new statement
         0
         x CELL+ @ 32 + ( equation.ThreadState.Node.obj )          !
         
         \ new statement
         0 S>D
         x CELL+ @ 52 + ( equation.ThreadState.Node.time )          2!
         
         \ new statement
         all
         TO adv
         
         \ new statement
         0 §break28525 LABEL
         
         \ new statement
         
         \ new statement
         DROP
         
         \ new statement
      ENDIF
      
      \ new statement
      
      \ new statement
      i 1+ TO i
      
      \ new statement
   REPEAT
   
   \ new statement
   0 §break28523 LABEL
   
   \ new statement
   
   
   §base2 SETVTOP
   PURGE 3
   
   \ new statement
   DROP
   
   \ new statement
   0 §break28522 LABEL
   
   \ new statement
   §synchronized0 1974299904 TRUE ( Object.monitorLeave ) EXECUTE-METHOD
   PURGE 1
   
   \ new statement
   
   PURGE 1
   
   \ new statement
   DROP
   
   \ new statement
   0 §break28521 LABEL
   
   \ new statement
   
   
   
   
   §base0 SETVTOP
   PURGE 5
   
   \ new statement
   DROP
;
VARIABLE ThreadState.BLOCKED
VARIABLE ThreadState.RUNNING
VARIABLE ThreadState.SUSPEND
VARIABLE ThreadState.TERMINATED
VARIABLE ThreadState.WAITING
VARIABLE ThreadState._staticBlocking
VARIABLE ThreadState._staticThread  HERE 4 - SALLOCATE
2VARIABLE ThreadState.lastTime
VARIABLE ThreadState.next  HERE 4 - SALLOCATE
VARIABLE ThreadState.queue  HERE 4 - SALLOCATE
VARIABLE ThreadState.queuelength
VARIABLE ThreadState.running  HERE 4 - SALLOCATE
VARIABLE ThreadState.skip

A:HERE VARIABLE ThreadState§1676825600.table 1 DUP 2* CELLS ALLOT R@ ! A:CELL+
-1772465152 R@ ! A:CELL+ ThreadState.ThreadState§-1772465152 VAL R@ ! A:CELL+
A:DROP


:LOCAL ThreadState.Node.Node§-54178304
   2 VALLOCATE LOCAL §base0
   DUP 4 §base0 + V! LOCAL §outer
   
   \ new statement
   0
   LOCALS §this |
   
   \ new statement
   0
   
   \ new statement
   84 MALLOC DROP DUP DUP §base0 V! TO §this OVER
   IF
      OVER CELL+ @ 4 + OVER OVER ! DROP
   ENDIF
   CELL+ @ OVER
   OVER ! NIP
   §outer OVER 8 + !
   ThreadState.Node§-846770688.table OVER 12 + !
   -846790656 OVER 20 + !
   " Node " OVER 16 + !
   4 OVER 24 + ! DROP
   
   \ new statement
   §this DUP 0 V! ( return object )    
   \ new statement
   0 §break28485 LABEL
   
   \ new statement
   §base0 SETVTOP
   PURGE 3
   
   \ new statement
   DROP
;
VARIABLE ThreadState.Node._staticBlocking
VARIABLE ThreadState.Node._staticThread  HERE 4 - SALLOCATE

A:HERE VARIABLE ThreadState.Node§-846770688.table 1 DUP 2* CELLS ALLOT R@ ! A:CELL+
-54178304 R@ ! A:CELL+ ThreadState.Node.Node§-54178304 VAL R@ ! A:CELL+
A:DROP
