MODULENAME equation.ThreadGroup
(
* @(#)ThreadGroup.java	1.55 03/01/23
*
* Copyright 2003 Sun Microsystems, Inc. All rights reserved.
* SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
)  \ import java.io.PrintStream;
( *
* A thread group represents a set of threads. In addition, a thread
* group can also include other thread groups. The thread groups form
* a tree in which every thread group except the initial thread group
* has a parent.
* <p>
* A thread is allowed to access information about its own thread
* group, but not to access information about its thread group's
* parent thread group or any other thread groups.
*
* @author  unascribed
* @version 1.55, 01/23/03
* @since   JDK1.0
)  (  The locking strategy for this code is to try to lock only one level of the
* tree wherever possible, but otherwise to lock from the bottom up.
* That is, from child thread groups to parents.
* This has the advantage of limiting the number of locks that need to be held
* and in particular avoids having to grab the lock for the root thread group,
* (or a global lock) which would be a source of contention on a
* multi-processor system with many thread groups.
* This policy often leads to taking a snapshot of the state of a thread group
* and working off of that snapshot, rather than holding the thread group locked
* while we work on the children.
)
( *
* Creates an empty Thread group that is not in any Thread group.
* This method is used to create the system Thread group.
)

:LOCAL ThreadGroup.ThreadGroup§-543337472
   1 VALLOCATE LOCAL §base0
   
   \ new statement
   0
   LOCALS §this |
   
   \ new statement
   Object§-1890496768.table 1827294976 EXECUTE-NEW
   
   \ new statement
   68 MALLOC DROP DUP DUP §base0 V! TO §this OVER
   IF
      OVER CELL+ @ 4 + OVER OVER ! DROP
   ENDIF
   CELL+ @ OVER
   OVER ! NIP
   ThreadGroup§-2665472.table OVER 12 + !
   -2686976 OVER 20 + !
   " ThreadGroup " OVER 16 + !
   4 OVER 24 + ! DROP
   
   \ new statement
   \  called from FORTH code
   
   
   \ new statement
   U" system " COUNT JavaArray.createUnicode§-675323136
   §this CELL+ @ 32 + ( equation.ThreadGroup.name )    !
   
   \ new statement
   Thread.MAX_PRIORITY
   @
   §this CELL+ @ 44 + ( equation.ThreadGroup.maxPriority )    !
   
   \ new statement
   §this DUP 0 V! ( return object )    
   \ new statement
   0 §break28534 LABEL
   
   \ new statement
   §base0 SETVTOP
   PURGE 2
   
   \ new statement
   DROP
;
( *
* Constructs a new thread group. The parent of this new group is
* the thread group of the currently running thread.
* <p>
* The <code>checkAccess</code> method of the parent thread group is
* called with no arguments; this may result in a security exception.
*
* @param   name   the name of the new thread group.
* @exception  SecurityException  if the current thread cannot create a
*               thread in the specified thread group.
* @see     java.lang.ThreadGroup#checkAccess()
* @since   JDK1.0
)

:LOCAL ThreadGroup.ThreadGroup§25908224
   2 VALLOCATE LOCAL §base0
   DUP 4 §base0 + V! LOCAL name
   
   \ new statement
   0
   LOCALS §this |
   
   \ new statement
   
   2 VALLOCATE LOCAL §tempvar
   Thread.currentThread§-1671470336 DUP §tempvar V!
   >R
   R> 409036544 TRUE ( equation.Thread.getThreadGroup§409036544 ) EXECUTE-METHOD DUP §tempvar 4 + V!
   name
   ThreadGroup§-2665472.table 606229504 EXECUTE-NEW
   DUP §base0 V! TO §this
   
   
   
   §tempvar SETVTOP
   
   PURGE 1
   
   
   \ new statement
   §this DUP 0 V! ( return object )    
   \ new statement
   0 §break28535 LABEL
   
   \ new statement
   
   §base0 SETVTOP
   PURGE 3
   
   \ new statement
   DROP
;
( *
* Creates a new thread group. The parent of this new group is the
* specified thread group.
* <p>
* The <code>checkAccess</code> method of the parent thread group is
* called with no arguments; this may result in a security exception.
*
* @param     parent   the parent thread group.
* @param     name     the name of the new thread group.
* @exception  NullPointerException  if the thread group argument is
*               <code>null</code>.
* @exception  SecurityException  if the current thread cannot create a
*               thread in the specified thread group.
* @see     java.lang.SecurityException
* @see     java.lang.ThreadGroup#checkAccess()
* @since   JDK1.0
)

:LOCAL ThreadGroup.ThreadGroup§606229504
   3 VALLOCATE LOCAL §base0
   DUP 4 §base0 + V! LOCAL name
   DUP 8 §base0 + V! LOCAL parent
   
   \ new statement
   0
   LOCALS §this |
   
   \ new statement
   Object§-1890496768.table 1827294976 EXECUTE-NEW
   
   \ new statement
   68 MALLOC DROP DUP DUP §base0 V! TO §this OVER
   IF
      OVER CELL+ @ 4 + OVER OVER ! DROP
   ENDIF
   CELL+ @ OVER
   OVER ! NIP
   ThreadGroup§-2665472.table OVER 12 + !
   -2686976 OVER 20 + !
   " ThreadGroup " OVER 16 + !
   4 OVER 24 + ! DROP
   
   \ new statement
   parent
   0=
   
   \ new statement
   IF
      
      \ new statement
      
      \ new statement
      NullPointerException§561335808.table 966675968 EXECUTE-NEW
      JavaArray.handler§-1096259584
      
      \ new statement
      
      \ new statement
      0 §break28537 LABEL
      
      \ new statement
      
      \ new statement
      0>!
      IF
         10001H - §break28536 BRANCH
      ENDIF
      DROP
      
      \ new statement
   ENDIF
   
   \ new statement
   name
   §this CELL+ @ 32 + ( equation.ThreadGroup.name )    !
   
   \ new statement
   parent CELL+ @ 44 + ( equation.ThreadGroup.maxPriority )    @
   §this CELL+ @ 44 + ( equation.ThreadGroup.maxPriority )    !
   
   \ new statement
   parent CELL+ @ 52 + ( equation.ThreadGroup.daemon )    @
   §this CELL+ @ 52 + ( equation.ThreadGroup.daemon )    !
   
   \ new statement
   parent CELL+ @ 56 + ( equation.ThreadGroup.vmAllowSuspension )    @
   §this CELL+ @ 56 + ( equation.ThreadGroup.vmAllowSuspension )    !
   
   \ new statement
   parent
   §this CELL+ @ 28 + ( equation.ThreadGroup.parent )    !
   
   \ new statement
   parent >R
   §this
   R> -1191681792 TRUE ( equation.ThreadGroup.add§-1191681792 ) EXECUTE-METHOD
   
   \ new statement
   §this DUP 0 V! ( return object )    
   \ new statement
   0 §break28536 LABEL
   
   \ new statement
   
   
   §base0 SETVTOP
   PURGE 4
   
   \ new statement
   DROP
;
( *
* Returns an estimate of the number of active threads in this
* thread group.
*
* @return  the number of active threads in this thread group and in any
*          other thread group that has this thread group as an ancestor.
* @since   JDK1.0
)

:LOCAL ThreadGroup.activeCount§-483565312
   2 VALLOCATE LOCAL §base0
   DUP 0 §base0 + V! LOCAL §this
   
   \ new statement
   0 DUP DUP
   LOCALS groupsSnapshot ngroupsSnapshot result |
   
   \ new statement
   \  Snapshot sub-group data so we don't hold this lock
   \  while our children are computing.
   
   
   \ new statement
   
   \ new statement
   §this
   LOCAL §synchronized0
   ThreadGroup._staticThread  @
   §synchronized0 801271040 TRUE ( Object.monitorEnter ) EXECUTE-METHOD
   
   \ new statement
   §this CELL+ @ 48 + ( equation.ThreadGroup.destroyed )    @
   
   \ new statement
   IF
      
      \ new statement
      
      \ new statement
      0
      131074 §break28563 BRANCH
      
      \ new statement
      0 §break28563 LABEL
      
      \ new statement
      
      \ new statement
      0>!
      IF
         10001H - §break28562 BRANCH
      ENDIF
      DROP
      
      \ new statement
   ENDIF
   
   \ new statement
   §this CELL+ @ 60 + ( equation.ThreadGroup.nthreads )    @
   TO result
   
   \ new statement
   §this CELL+ @ 64 + ( equation.ThreadGroup.ngroups )    @
   TO ngroupsSnapshot
   
   \ new statement
   §this CELL+ @ 40 + ( equation.ThreadGroup.groups )    @
   0<>
   
   \ new statement
   IF
      
      \ new statement
      
      \ new statement
      
      1 VALLOCATE LOCAL §tempvar
      ngroupsSnapshot -2686975 JavaArray§1352878592.table -227194368 EXECUTE-NEW
      DUP 4 §base0 + V! TO groupsSnapshot
      
      §tempvar SETVTOP
      
      PURGE 1
      
      
      \ new statement
      §this >R
      §this CELL+ @ 40 + ( equation.ThreadGroup.groups )       @
      0
      groupsSnapshot
      0
      ngroupsSnapshot
      R> -22060800 TRUE ( equation.ThreadGroup.arraycopy§-22060800 ) EXECUTE-METHOD
      
      \ new statement
      0 §break28564 LABEL
      
      \ new statement
      
      \ new statement
      0>!
      IF
         10001H - §break28562 BRANCH
      ENDIF
      DROP
      
      \ new statement
   ELSE
      
      \ new statement
      
      \ new statement
      0
      DUP 4 §base0 + V! TO groupsSnapshot
      
      \ new statement
      0 §break28565 LABEL
      
      \ new statement
      
      \ new statement
      0>!
      IF
         10001H - §break28562 BRANCH
      ENDIF
      DROP
      
      \ new statement
   ENDIF
   
   \ new statement
   0 §break28562 LABEL
   
   \ new statement
   §synchronized0 1974299904 TRUE ( Object.monitorLeave ) EXECUTE-METHOD
   PURGE 1
   
   \ new statement
   
   \ new statement
   0>!
   IF
      10001H - §break28561 BRANCH
   ENDIF
   DROP
   
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
      ngroupsSnapshot
      <
      
      \ new statement
   WHILE
      
      \ new statement
      result
      groupsSnapshot >R
      i
      R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
      @  " ThreadGroup " CASTTO >R
      R> -483565312 TRUE ( equation.ThreadGroup.activeCount§-483565312 ) EXECUTE-METHOD
      +
      TO result
      
      \ new statement
      
      \ new statement
      i 1+ TO i
      
      \ new statement
   REPEAT
   
   \ new statement
   0 §break28566 LABEL
   
   \ new statement
   
   PURGE 1
   
   \ new statement
   0>!
   IF
      10001H - §break28561 BRANCH
   ENDIF
   DROP
   
   \ new statement
   result
   0 §break28561 BRANCH
   
   \ new statement
   0 §break28561 LABEL
   
   \ new statement
   
   
   
   §base0 SETVTOP
   PURGE 5
   
   \ new statement
   DROP
;
( *
* Returns an estimate of the number of active groups in this
* thread group.
*
* @return  the number of active thread groups with this thread group as
*          an ancestor.
* @since   JDK1.0
)

:LOCAL ThreadGroup.activeGroupCount§-818650880
   2 VALLOCATE LOCAL §base0
   DUP 0 §base0 + V! LOCAL §this
   
   \ new statement
   0 DUP DUP
   LOCALS groupsSnapshot n ngroupsSnapshot |
   
   \ new statement
   
   \ new statement
   §this
   LOCAL §synchronized0
   ThreadGroup._staticThread  @
   §synchronized0 801271040 TRUE ( Object.monitorEnter ) EXECUTE-METHOD
   
   \ new statement
   §this CELL+ @ 48 + ( equation.ThreadGroup.destroyed )    @
   
   \ new statement
   IF
      
      \ new statement
      
      \ new statement
      0
      131074 §break28585 BRANCH
      
      \ new statement
      0 §break28585 LABEL
      
      \ new statement
      
      \ new statement
      0>!
      IF
         10001H - §break28584 BRANCH
      ENDIF
      DROP
      
      \ new statement
   ENDIF
   
   \ new statement
   §this CELL+ @ 64 + ( equation.ThreadGroup.ngroups )    @
   TO ngroupsSnapshot
   
   \ new statement
   §this CELL+ @ 40 + ( equation.ThreadGroup.groups )    @
   0<>
   
   \ new statement
   IF
      
      \ new statement
      
      \ new statement
      
      1 VALLOCATE LOCAL §tempvar
      ngroupsSnapshot -2686975 JavaArray§1352878592.table -227194368 EXECUTE-NEW
      DUP 4 §base0 + V! TO groupsSnapshot
      
      §tempvar SETVTOP
      
      PURGE 1
      
      
      \ new statement
      §this >R
      §this CELL+ @ 40 + ( equation.ThreadGroup.groups )       @
      0
      groupsSnapshot
      0
      ngroupsSnapshot
      R> -22060800 TRUE ( equation.ThreadGroup.arraycopy§-22060800 ) EXECUTE-METHOD
      
      \ new statement
      0 §break28586 LABEL
      
      \ new statement
      
      \ new statement
      0>!
      IF
         10001H - §break28584 BRANCH
      ENDIF
      DROP
      
      \ new statement
   ELSE
      
      \ new statement
      
      \ new statement
      0
      DUP 4 §base0 + V! TO groupsSnapshot
      
      \ new statement
      0 §break28587 LABEL
      
      \ new statement
      
      \ new statement
      0>!
      IF
         10001H - §break28584 BRANCH
      ENDIF
      DROP
      
      \ new statement
   ENDIF
   
   \ new statement
   0 §break28584 LABEL
   
   \ new statement
   §synchronized0 1974299904 TRUE ( Object.monitorLeave ) EXECUTE-METHOD
   PURGE 1
   
   \ new statement
   
   \ new statement
   0>!
   IF
      10001H - §break28583 BRANCH
   ENDIF
   DROP
   
   \ new statement
   ngroupsSnapshot
   TO n
   
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
      ngroupsSnapshot
      <
      
      \ new statement
   WHILE
      
      \ new statement
      n
      groupsSnapshot >R
      i
      R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
      @  " ThreadGroup " CASTTO >R
      R> -818650880 TRUE ( equation.ThreadGroup.activeGroupCount§-818650880 ) EXECUTE-METHOD
      +
      TO n
      
      \ new statement
      
      \ new statement
      i 1+ TO i
      
      \ new statement
   REPEAT
   
   \ new statement
   0 §break28588 LABEL
   
   \ new statement
   
   PURGE 1
   
   \ new statement
   0>!
   IF
      10001H - §break28583 BRANCH
   ENDIF
   DROP
   
   \ new statement
   n
   0 §break28583 BRANCH
   
   \ new statement
   0 §break28583 LABEL
   
   \ new statement
   
   
   
   §base0 SETVTOP
   PURGE 5
   
   \ new statement
   DROP
;
( *
* Adds the specified Thread group to this group.
* @param g the specified Thread group to be added
* @exception IllegalThreadStateException If the Thread group has been destroyed.
)

:LOCAL ThreadGroup.add§-1191681792
   2 VALLOCATE LOCAL §base0
   DUP 4 §base0 + V! LOCAL §this
   DUP 0 §base0 + V! LOCAL g
   
   \ new statement
   
   \ new statement
   
   \ new statement
   §this
   LOCAL §synchronized0
   ThreadGroup._staticThread  @
   §synchronized0 801271040 TRUE ( Object.monitorEnter ) EXECUTE-METHOD
   
   \ new statement
   §this CELL+ @ 48 + ( equation.ThreadGroup.destroyed )    @
   
   \ new statement
   IF
      
      \ new statement
      
      \ new statement
      IllegalThreadStateException§-782350080.table -1901442816 EXECUTE-NEW
      JavaArray.handler§-1096259584
      
      \ new statement
      
      \ new statement
      0 §break28638 LABEL
      
      \ new statement
      
      \ new statement
      0>!
      IF
         10001H - §break28637 BRANCH
      ENDIF
      DROP
      
      \ new statement
   ENDIF
   
   \ new statement
   §this CELL+ @ 40 + ( equation.ThreadGroup.groups )    @
   0=
   
   \ new statement
   IF
      
      \ new statement
      
      \ new statement
      
      1 VALLOCATE LOCAL §tempvar
      4
      -2686975 JavaArray§1352878592.table -227194368 EXECUTE-NEW
      §this CELL+ @ 40 + ( equation.ThreadGroup.groups )       !
      
      §tempvar SETVTOP
      
      PURGE 1
      
      
      \ new statement
      0 §break28639 LABEL
      
      \ new statement
      
      \ new statement
      0>!
      IF
         10001H - §break28637 BRANCH
      ENDIF
      DROP
      
      \ new statement
   ELSE
      
      \ new statement
      §this CELL+ @ 64 + ( equation.ThreadGroup.ngroups )       @
      §this CELL+ @ 40 + ( equation.ThreadGroup.groups ) @ CELL+ @ 28 + ( equation.JavaArray.length )       @
      =
      
      \ new statement
      IF
         
         \ new statement
         1 VALLOCATE LOCAL §base2
         0
         LOCALS newgroups |
         
         \ new statement
         
         1 VALLOCATE LOCAL §tempvar
         §this CELL+ @ 64 + ( equation.ThreadGroup.ngroups )          @
         2* -2686975 JavaArray§1352878592.table -227194368 EXECUTE-NEW
         DUP §base2 V! TO newgroups
         
         §tempvar SETVTOP
         
         PURGE 1
         
         
         \ new statement
         §this >R
         §this CELL+ @ 40 + ( equation.ThreadGroup.groups )          @
         0
         newgroups
         0
         §this CELL+ @ 64 + ( equation.ThreadGroup.ngroups )          @
         R> -22060800 TRUE ( equation.ThreadGroup.arraycopy§-22060800 ) EXECUTE-METHOD
         
         \ new statement
         newgroups
         §this CELL+ @ 40 + ( equation.ThreadGroup.groups )          !
         
         \ new statement
         0 §break28640 LABEL
         
         \ new statement
         
         §base2 SETVTOP
         PURGE 2
         
         \ new statement
         0>!
         IF
            10001H - §break28637 BRANCH
         ENDIF
         DROP
         
         \ new statement
      ENDIF
      
      \ new statement
   ENDIF
   
   \ new statement
   
   0
   LOCALS 0§ |
   
   §this CELL+ @ 64 + ( equation.ThreadGroup.ngroups )    @ TO 0§
   g
   §this CELL+ @ 40 + ( equation.ThreadGroup.groups ) @ >R
   0§
   R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
   !
   
   PURGE 1
   
   
   \ new statement
   \  This is done last so it doesn't matter in case the
   \  thread is killed
   
   
   \ new statement
   §this CELL+ @ 64 + ( equation.ThreadGroup.ngroups )    A:R@ @ 1+ R@ !
   A:DROP
   
   \ new statement
   0 §break28637 LABEL
   
   \ new statement
   §synchronized0 1974299904 TRUE ( Object.monitorLeave ) EXECUTE-METHOD
   PURGE 1
   
   \ new statement
   
   \ new statement
   0>!
   IF
      10001H - §break28636 BRANCH
   ENDIF
   DROP
   
   \ new statement
   0 §break28636 LABEL
   
   \ new statement
   
   §base0 SETVTOP
   PURGE 3
   
   \ new statement
   DROP
;
( *
* Adds the specified Thread to this group.
* @param t the Thread to be added
* @exception IllegalThreadStateException If the Thread group has been destroyed.
)

:LOCAL ThreadGroup.add§335503616
   2 VALLOCATE LOCAL §base0
   DUP 4 §base0 + V! LOCAL §this
   DUP 0 §base0 + V! LOCAL t
   
   \ new statement
   
   \ new statement
   
   \ new statement
   §this
   LOCAL §synchronized0
   ThreadGroup._staticThread  @
   §synchronized0 801271040 TRUE ( Object.monitorEnter ) EXECUTE-METHOD
   
   \ new statement
   §this CELL+ @ 48 + ( equation.ThreadGroup.destroyed )    @
   
   \ new statement
   IF
      
      \ new statement
      
      \ new statement
      IllegalThreadStateException§-782350080.table -1901442816 EXECUTE-NEW
      JavaArray.handler§-1096259584
      
      \ new statement
      
      \ new statement
      0 §break28651 LABEL
      
      \ new statement
      
      \ new statement
      0>!
      IF
         10001H - §break28650 BRANCH
      ENDIF
      DROP
      
      \ new statement
   ENDIF
   
   \ new statement
   §this CELL+ @ 36 + ( equation.ThreadGroup.threads )    @
   0=
   
   \ new statement
   IF
      
      \ new statement
      
      \ new statement
      
      1 VALLOCATE LOCAL §tempvar
      4
      -1457258495 JavaArray§1352878592.table -227194368 EXECUTE-NEW
      §this CELL+ @ 36 + ( equation.ThreadGroup.threads )       !
      
      §tempvar SETVTOP
      
      PURGE 1
      
      
      \ new statement
      0 §break28652 LABEL
      
      \ new statement
      
      \ new statement
      0>!
      IF
         10001H - §break28650 BRANCH
      ENDIF
      DROP
      
      \ new statement
   ELSE
      
      \ new statement
      §this CELL+ @ 60 + ( equation.ThreadGroup.nthreads )       @
      §this CELL+ @ 36 + ( equation.ThreadGroup.threads ) @ CELL+ @ 28 + ( equation.JavaArray.length )       @
      =
      
      \ new statement
      IF
         
         \ new statement
         1 VALLOCATE LOCAL §base2
         0
         LOCALS newthreads |
         
         \ new statement
         
         1 VALLOCATE LOCAL §tempvar
         §this CELL+ @ 60 + ( equation.ThreadGroup.nthreads )          @
         2* -1457258495 JavaArray§1352878592.table -227194368 EXECUTE-NEW
         DUP §base2 V! TO newthreads
         
         §tempvar SETVTOP
         
         PURGE 1
         
         
         \ new statement
         §this >R
         §this CELL+ @ 36 + ( equation.ThreadGroup.threads )          @
         0
         newthreads
         0
         §this CELL+ @ 60 + ( equation.ThreadGroup.nthreads )          @
         R> 794779904 TRUE ( equation.ThreadGroup.arraycopy§794779904 ) EXECUTE-METHOD
         
         \ new statement
         newthreads
         §this CELL+ @ 36 + ( equation.ThreadGroup.threads )          !
         
         \ new statement
         0 §break28653 LABEL
         
         \ new statement
         
         §base2 SETVTOP
         PURGE 2
         
         \ new statement
         0>!
         IF
            10001H - §break28650 BRANCH
         ENDIF
         DROP
         
         \ new statement
      ENDIF
      
      \ new statement
   ENDIF
   
   \ new statement
   
   0
   LOCALS 0§ |
   
   §this CELL+ @ 60 + ( equation.ThreadGroup.nthreads )    @ TO 0§
   t
   §this CELL+ @ 36 + ( equation.ThreadGroup.threads ) @ >R
   0§
   R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
   !
   
   PURGE 1
   
   
   \ new statement
   \  This is done last so it doesn't matter in case the
   \  thread is killed
   
   
   \ new statement
   §this CELL+ @ 60 + ( equation.ThreadGroup.nthreads )    A:R@ @ 1+ R@ !
   A:DROP
   
   \ new statement
   0 §break28650 LABEL
   
   \ new statement
   §synchronized0 1974299904 TRUE ( Object.monitorLeave ) EXECUTE-METHOD
   PURGE 1
   
   \ new statement
   
   \ new statement
   0>!
   IF
      10001H - §break28649 BRANCH
   ENDIF
   DROP
   
   \ new statement
   0 §break28649 LABEL
   
   \ new statement
   
   §base0 SETVTOP
   PURGE 3
   
   \ new statement
   DROP
;


:LOCAL ThreadGroup.arraycopy§-22060800
   3 VALLOCATE LOCAL §base0
   DUP 8 §base0 + V! LOCAL §this
   LOCAL len
   LOCAL offset
   DUP 0 §base0 + V! LOCAL groupsSnapshot
   LOCAL begin
   DUP 4 §base0 + V! LOCAL groups
   
   \ new statement
   
   \ new statement
   0
   LOCALS i |
   
   \ new statement
   offset
   TO i
   
   \ new statement
   BEGIN
      
      \ new statement
      i
      len
      <
      
      \ new statement
   WHILE
      
      \ new statement
      
      0
      LOCALS 0§ |
      
      i DUP 1+ TO i
      TO 0§
      groups >R
      begin DUP 1+ TO begin
      R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
      @  " ThreadGroup " CASTTO
      groupsSnapshot >R
      0§
      R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
      !
      
      PURGE 1
      
      
      \ new statement
      
      \ new statement
   REPEAT
   
   \ new statement
   0 §break28553 LABEL
   
   \ new statement
   
   PURGE 1
   
   \ new statement
   DROP
   
   \ new statement
   0 §break28552 LABEL
   
   \ new statement
   
   
   
   
   
   §base0 SETVTOP
   PURGE 7
   
   \ new statement
   DROP
;


:LOCAL ThreadGroup.arraycopy§794779904
   3 VALLOCATE LOCAL §base0
   DUP 8 §base0 + V! LOCAL §this
   LOCAL len
   LOCAL offset
   DUP 0 §base0 + V! LOCAL groupsSnapshot
   LOCAL begin
   DUP 4 §base0 + V! LOCAL groups
   
   \ new statement
   
   \ new statement
   0
   LOCALS i |
   
   \ new statement
   offset
   TO i
   
   \ new statement
   BEGIN
      
      \ new statement
      i
      len
      <
      
      \ new statement
   WHILE
      
      \ new statement
      
      0
      LOCALS 0§ |
      
      i DUP 1+ TO i
      TO 0§
      groups >R
      begin DUP 1+ TO begin
      R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
      @  " Thread " CASTTO
      groupsSnapshot >R
      0§
      R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
      !
      
      PURGE 1
      
      
      \ new statement
      
      \ new statement
   REPEAT
   
   \ new statement
   0 §break28555 LABEL
   
   \ new statement
   
   PURGE 1
   
   \ new statement
   DROP
   
   \ new statement
   0 §break28554 LABEL
   
   \ new statement
   
   
   
   
   
   §base0 SETVTOP
   PURGE 7
   
   \ new statement
   DROP
;
( *
* Determines if the currently running thread has permission to
* modify this thread group.
* <p>
* If there is a security manager, its <code>checkAccess</code> method
* is called with this thread group as its argument. This may result
* in throwing a <code>SecurityException</code>.
*
* @exception  SecurityException  if the current thread is not allowed to
*               access this thread group.
* @see        java.lang.SecurityManager#checkAccess(java.lang.ThreadGroup)
* @since      JDK1.0
)

:LOCAL ThreadGroup.checkAccess§354116352
   1 VALLOCATE LOCAL §base0
   DUP 0 §base0 + V! LOCAL §this
   
   \ new statement
   
   \ new statement
   \ SecurityManager security = System.getSecurityManager();
   \ if (security != null) {
   \     security.checkAccess(this);
   \ }
   
   
   \ new statement
   0 §break28560 LABEL
   
   \ new statement
   §base0 SETVTOP
   PURGE 2
   
   \ new statement
   DROP
;
( *
* Destroys this thread group and all of its subgroups. This thread
* group must be empty, indicating that all threads that had been in
* this thread group have since stopped.
* <p>
* First, the <code>checkAccess</code> method of this thread group is
* called with no arguments; this may result in a security exception.
*
* @exception  IllegalThreadStateException  if the thread group is not
*               empty or if the thread group has already been destroyed.
* @exception  SecurityException  if the current thread cannot modify this
*               thread group.
* @see        java.lang.ThreadGroup#checkAccess()
* @since      JDK1.0
)

:LOCAL ThreadGroup.destroy§855073792
   2 VALLOCATE LOCAL §base0
   DUP 0 §base0 + V! LOCAL §this
   
   \ new statement
   0 DUP
   LOCALS groupsSnapshot ngroupsSnapshot |
   
   \ new statement
   
   \ new statement
   §this
   LOCAL §synchronized0
   ThreadGroup._staticThread  @
   §synchronized0 801271040 TRUE ( Object.monitorEnter ) EXECUTE-METHOD
   
   \ new statement
   §this >R
   R> 354116352 TRUE ( equation.ThreadGroup.checkAccess§354116352 ) EXECUTE-METHOD
   
   \ new statement
   §this CELL+ @ 48 + ( equation.ThreadGroup.destroyed )    @  0=! or_28957 0BRANCH DROP
   §this CELL+ @ 60 + ( equation.ThreadGroup.nthreads )    @
   0>
   or_28957 LABEL
   
   \ new statement
   IF
      
      \ new statement
      
      \ new statement
      IllegalThreadStateException§-782350080.table -1901442816 EXECUTE-NEW
      JavaArray.handler§-1096259584
      
      \ new statement
      
      \ new statement
      0 §break28629 LABEL
      
      \ new statement
      
      \ new statement
      0>!
      IF
         10001H - §break28628 BRANCH
      ENDIF
      DROP
      
      \ new statement
   ENDIF
   
   \ new statement
   §this CELL+ @ 64 + ( equation.ThreadGroup.ngroups )    @
   TO ngroupsSnapshot
   
   \ new statement
   §this CELL+ @ 40 + ( equation.ThreadGroup.groups )    @
   0<>
   
   \ new statement
   IF
      
      \ new statement
      
      \ new statement
      
      1 VALLOCATE LOCAL §tempvar
      ngroupsSnapshot -2686975 JavaArray§1352878592.table -227194368 EXECUTE-NEW
      DUP 4 §base0 + V! TO groupsSnapshot
      
      §tempvar SETVTOP
      
      PURGE 1
      
      
      \ new statement
      §this >R
      §this CELL+ @ 40 + ( equation.ThreadGroup.groups )       @
      0
      groupsSnapshot
      0
      ngroupsSnapshot
      R> -22060800 TRUE ( equation.ThreadGroup.arraycopy§-22060800 ) EXECUTE-METHOD
      
      \ new statement
      0 §break28630 LABEL
      
      \ new statement
      
      \ new statement
      0>!
      IF
         10001H - §break28628 BRANCH
      ENDIF
      DROP
      
      \ new statement
   ELSE
      
      \ new statement
      
      \ new statement
      0
      DUP 4 §base0 + V! TO groupsSnapshot
      
      \ new statement
      0 §break28631 LABEL
      
      \ new statement
      
      \ new statement
      0>!
      IF
         10001H - §break28628 BRANCH
      ENDIF
      DROP
      
      \ new statement
   ENDIF
   
   \ new statement
   §this CELL+ @ 28 + ( equation.ThreadGroup.parent )    @
   0<>
   
   \ new statement
   IF
      
      \ new statement
      
      \ new statement
      TRUE
      §this CELL+ @ 48 + ( equation.ThreadGroup.destroyed )       !
      
      \ new statement
      0
      §this CELL+ @ 64 + ( equation.ThreadGroup.ngroups )       !
      
      \ new statement
      0
      §this CELL+ @ 40 + ( equation.ThreadGroup.groups )       !
      
      \ new statement
      0
      §this CELL+ @ 60 + ( equation.ThreadGroup.nthreads )       !
      
      \ new statement
      0
      §this CELL+ @ 36 + ( equation.ThreadGroup.threads )       !
      
      \ new statement
      0 §break28632 LABEL
      
      \ new statement
      
      \ new statement
      0>!
      IF
         10001H - §break28628 BRANCH
      ENDIF
      DROP
      
      \ new statement
   ENDIF
   
   \ new statement
   0 §break28628 LABEL
   
   \ new statement
   §synchronized0 1974299904 TRUE ( Object.monitorLeave ) EXECUTE-METHOD
   PURGE 1
   
   \ new statement
   
   \ new statement
   0>!
   IF
      10001H - §break28627 BRANCH
   ENDIF
   DROP
   
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
      ngroupsSnapshot
      <
      
      \ new statement
   WHILE
      
      \ new statement
      groupsSnapshot >R
      i
      R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
      @  " ThreadGroup " CASTTO >R
      R> 855073792 TRUE ( equation.ThreadGroup.destroy§855073792 ) EXECUTE-METHOD
      
      \ new statement
      
      \ new statement
      i
      1
      +
      TO i
      
      \ new statement
   REPEAT
   
   \ new statement
   0 §break28633 LABEL
   
   \ new statement
   
   PURGE 1
   
   \ new statement
   0>!
   IF
      10001H - §break28627 BRANCH
   ENDIF
   DROP
   
   \ new statement
   §this CELL+ @ 28 + ( equation.ThreadGroup.parent )    @
   0<>
   
   \ new statement
   IF
      
      \ new statement
      
      \ new statement
      §this CELL+ @ 28 + ( equation.ThreadGroup.parent ) @ >R
      §this
      R> -1031376384 TRUE ( equation.ThreadGroup.remove§-1031376384 ) EXECUTE-METHOD
      
      \ new statement
      0 §break28635 LABEL
      
      \ new statement
      
      \ new statement
      0>!
      IF
         10001H - §break28627 BRANCH
      ENDIF
      DROP
      
      \ new statement
   ENDIF
   
   \ new statement
   0 §break28627 LABEL
   
   \ new statement
   
   
   §base0 SETVTOP
   PURGE 4
   
   \ new statement
   DROP
;


:LOCAL ThreadGroup.enumerate§-1064803072
   3 VALLOCATE LOCAL §base0
   DUP 4 §base0 + V! LOCAL §this
   LOCAL recurse
   LOCAL n
   DUP 0 §base0 + V! LOCAL list
   
   \ new statement
   0 DUP
   LOCALS groupsSnapshot ngroupsSnapshot |
   
   \ new statement
   0
   TO ngroupsSnapshot
   
   \ new statement
   0
   DUP 8 §base0 + V! TO groupsSnapshot
   
   \ new statement
   0
   LOCALS ng |
   
   \ new statement
   §this
   LOCAL §synchronized0
   ThreadGroup._staticThread  @
   §synchronized0 801271040 TRUE ( Object.monitorEnter ) EXECUTE-METHOD
   
   \ new statement
   §this CELL+ @ 48 + ( equation.ThreadGroup.destroyed )    @
   
   \ new statement
   IF
      
      \ new statement
      
      \ new statement
      0
      131074 §break28594 BRANCH
      
      \ new statement
      0 §break28594 LABEL
      
      \ new statement
      
      \ new statement
      0>!
      IF
         10001H - §break28593 BRANCH
      ENDIF
      DROP
      
      \ new statement
   ENDIF
   
   \ new statement
   §this CELL+ @ 64 + ( equation.ThreadGroup.ngroups )    @
   TO ng
   
   \ new statement
   ng
   list CELL+ @ 28 + ( equation.JavaArray.length )    @
   n
   -
   >
   
   \ new statement
   IF
      
      \ new statement
      
      \ new statement
      list CELL+ @ 28 + ( equation.JavaArray.length )       @
      n
      -
      TO ng
      
      \ new statement
      0 §break28595 LABEL
      
      \ new statement
      
      \ new statement
      0>!
      IF
         10001H - §break28593 BRANCH
      ENDIF
      DROP
      
      \ new statement
   ENDIF
   
   \ new statement
   ng
   0>
   
   \ new statement
   IF
      
      \ new statement
      
      \ new statement
      §this >R
      §this CELL+ @ 40 + ( equation.ThreadGroup.groups )       @
      0
      list
      n
      ng
      R> -22060800 TRUE ( equation.ThreadGroup.arraycopy§-22060800 ) EXECUTE-METHOD
      
      \ new statement
      n
      ng
      +
      TO n
      
      \ new statement
      0 §break28596 LABEL
      
      \ new statement
      
      \ new statement
      0>!
      IF
         10001H - §break28593 BRANCH
      ENDIF
      DROP
      
      \ new statement
   ENDIF
   
   \ new statement
   recurse
   
   \ new statement
   IF
      
      \ new statement
      
      \ new statement
      §this CELL+ @ 64 + ( equation.ThreadGroup.ngroups )       @
      TO ngroupsSnapshot
      
      \ new statement
      §this CELL+ @ 40 + ( equation.ThreadGroup.groups )       @
      0<>
      
      \ new statement
      IF
         
         \ new statement
         
         \ new statement
         
         1 VALLOCATE LOCAL §tempvar
         ngroupsSnapshot -2686975 JavaArray§1352878592.table -227194368 EXECUTE-NEW
         DUP 8 §base0 + V! TO groupsSnapshot
         
         §tempvar SETVTOP
         
         PURGE 1
         
         
         \ new statement
         §this >R
         §this CELL+ @ 40 + ( equation.ThreadGroup.groups )          @
         0
         groupsSnapshot
         0
         ngroupsSnapshot
         R> -22060800 TRUE ( equation.ThreadGroup.arraycopy§-22060800 ) EXECUTE-METHOD
         
         \ new statement
         0 §break28598 LABEL
         
         \ new statement
         
         \ new statement
         DROP
         
         \ new statement
      ELSE
         
         \ new statement
         
         \ new statement
         0
         DUP 8 §base0 + V! TO groupsSnapshot
         
         \ new statement
         0 §break28599 LABEL
         
         \ new statement
         
         \ new statement
         DROP
         
         \ new statement
      ENDIF
      
      \ new statement
      0 §break28597 LABEL
      
      \ new statement
      
      \ new statement
      0>!
      IF
         10001H - §break28593 BRANCH
      ENDIF
      DROP
      
      \ new statement
   ENDIF
   
   \ new statement
   0 §break28593 LABEL
   
   \ new statement
   §synchronized0 1974299904 TRUE ( Object.monitorLeave ) EXECUTE-METHOD
   PURGE 1
   
   \ new statement
   
   PURGE 1
   
   \ new statement
   0>!
   IF
      10001H - §break28592 BRANCH
   ENDIF
   DROP
   
   \ new statement
   recurse
   
   \ new statement
   IF
      
      \ new statement
      
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
         ngroupsSnapshot
         <
         
         \ new statement
      WHILE
         
         \ new statement
         groupsSnapshot >R
         i
         R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
         @  " ThreadGroup " CASTTO >R
         list
         n
         TRUE
         R> -1064803072 TRUE ( equation.ThreadGroup.enumerate§-1064803072 ) EXECUTE-METHOD
         TO n
         
         \ new statement
         
         \ new statement
         i 1+ TO i
         
         \ new statement
      REPEAT
      
      \ new statement
      0 §break28601 LABEL
      
      \ new statement
      
      PURGE 1
      
      \ new statement
      DROP
      
      \ new statement
      0 §break28600 LABEL
      
      \ new statement
      
      \ new statement
      0>!
      IF
         10001H - §break28592 BRANCH
      ENDIF
      DROP
      
      \ new statement
   ENDIF
   
   \ new statement
   n
   0 §break28592 BRANCH
   
   \ new statement
   0 §break28592 LABEL
   
   \ new statement
   
   
   
   
   
   §base0 SETVTOP
   PURGE 7
   
   \ new statement
   DROP
;
( *
* Copies into the specified array every active thread in this
* thread group and its subgroups.
* <p>
* First, the <code>checkAccess</code> method of this thread group is
* called with no arguments; this may result in a security exception.
* <p>
* An application should use the <code>activeCount</code> method to
* get an estimate of how big the array should be. If the array is
* too short to hold all the threads, the extra threads are silently
* ignored.
*
* @param   list   an array into which to place the list of threads.
* @return  the number of threads put into the array.
* @exception  SecurityException  if the current thread does not
*               have permission to enumerate this thread group.
* @see     java.lang.ThreadGroup#activeCount()
* @see     java.lang.ThreadGroup#checkAccess()
* @since   JDK1.0
)

:LOCAL ThreadGroup.enumerate§-1458019072
   2 VALLOCATE LOCAL §base0
   DUP 4 §base0 + V! LOCAL §this
   DUP 0 §base0 + V! LOCAL list
   
   \ new statement
   
   \ new statement
   §this >R
   R> 354116352 TRUE ( equation.ThreadGroup.checkAccess§354116352 ) EXECUTE-METHOD
   
   \ new statement
   §this >R
   list
   0
   TRUE
   R> -596417280 TRUE ( equation.ThreadGroup.enumerate§-596417280 ) EXECUTE-METHOD
   0 §break28568 BRANCH
   
   \ new statement
   0 §break28568 LABEL
   
   \ new statement
   
   §base0 SETVTOP
   PURGE 3
   
   \ new statement
   DROP
;
( *
* Copies into the specified array references to every active
* subgroup in this thread group. If the <code>recurse</code> flag is
* <code>true</code>, references to all active subgroups of the
* subgroups and so forth are also included.
* <p>
* First, the <code>checkAccess</code> method of this thread group is
* called with no arguments; this may result in a security exception.
* <p>
* An application should use the <code>activeGroupCount</code>
* method to get an estimate of how big the array should be.
*
* @param   list      an array into which to place the list of threads.
* @param   recurse   a flag indicating whether to recursively enumerate
*                    all included thread groups.
* @return  the number of thread groups put into the array.
* @exception  SecurityException  if the current thread does not
*               have permission to enumerate this thread group.
* @see     java.lang.ThreadGroup#activeGroupCount()
* @see     java.lang.ThreadGroup#checkAccess()
* @since   JDK1.0
)

:LOCAL ThreadGroup.enumerate§-488282880
   2 VALLOCATE LOCAL §base0
   DUP 4 §base0 + V! LOCAL §this
   LOCAL recurse
   DUP 0 §base0 + V! LOCAL list
   
   \ new statement
   
   \ new statement
   §this >R
   R> 354116352 TRUE ( equation.ThreadGroup.checkAccess§354116352 ) EXECUTE-METHOD
   
   \ new statement
   §this >R
   list
   0
   recurse
   R> -1064803072 TRUE ( equation.ThreadGroup.enumerate§-1064803072 ) EXECUTE-METHOD
   0 §break28591 BRANCH
   
   \ new statement
   0 §break28591 LABEL
   
   \ new statement
   
   
   §base0 SETVTOP
   PURGE 4
   
   \ new statement
   DROP
;


:LOCAL ThreadGroup.enumerate§-596417280
   3 VALLOCATE LOCAL §base0
   DUP 4 §base0 + V! LOCAL §this
   LOCAL recurse
   LOCAL n
   DUP 0 §base0 + V! LOCAL list
   
   \ new statement
   0 DUP
   LOCALS groupsSnapshot ngroupsSnapshot |
   
   \ new statement
   0
   TO ngroupsSnapshot
   
   \ new statement
   0
   DUP 8 §base0 + V! TO groupsSnapshot
   
   \ new statement
   0
   LOCALS nt |
   
   \ new statement
   §this
   LOCAL §synchronized0
   ThreadGroup._staticThread  @
   §synchronized0 801271040 TRUE ( Object.monitorEnter ) EXECUTE-METHOD
   
   \ new statement
   §this CELL+ @ 48 + ( equation.ThreadGroup.destroyed )    @
   
   \ new statement
   IF
      
      \ new statement
      
      \ new statement
      0
      131074 §break28572 BRANCH
      
      \ new statement
      0 §break28572 LABEL
      
      \ new statement
      
      \ new statement
      0>!
      IF
         10001H - §break28571 BRANCH
      ENDIF
      DROP
      
      \ new statement
   ENDIF
   
   \ new statement
   §this CELL+ @ 60 + ( equation.ThreadGroup.nthreads )    @
   TO nt
   
   \ new statement
   nt
   list CELL+ @ 28 + ( equation.JavaArray.length )    @
   n
   -
   >
   
   \ new statement
   IF
      
      \ new statement
      
      \ new statement
      list CELL+ @ 28 + ( equation.JavaArray.length )       @
      n
      -
      TO nt
      
      \ new statement
      0 §break28573 LABEL
      
      \ new statement
      
      \ new statement
      0>!
      IF
         10001H - §break28571 BRANCH
      ENDIF
      DROP
      
      \ new statement
   ENDIF
   
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
      nt
      <
      
      \ new statement
   WHILE
      
      \ new statement
      §this CELL+ @ 36 + ( equation.ThreadGroup.threads ) @ >R
      i
      R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
      @  " Thread " CASTTO >R
      R> -128489216 TRUE ( equation.Thread.isAlive§-128489216 ) EXECUTE-METHOD
      
      \ new statement
      IF
         
         \ new statement
         
         \ new statement
         
         0
         LOCALS 0§ |
         
         n DUP 1+ TO n
         TO 0§
         §this CELL+ @ 36 + ( equation.ThreadGroup.threads ) @ >R
         i
         R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
         @  " Thread " CASTTO
         list >R
         0§
         R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
         !
         
         PURGE 1
         
         
         \ new statement
         0 §break28576 LABEL
         
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
   0 §break28574 LABEL
   
   \ new statement
   
   PURGE 1
   
   \ new statement
   0>!
   IF
      10001H - §break28571 BRANCH
   ENDIF
   DROP
   
   \ new statement
   recurse
   
   \ new statement
   IF
      
      \ new statement
      
      \ new statement
      §this CELL+ @ 64 + ( equation.ThreadGroup.ngroups )       @
      TO ngroupsSnapshot
      
      \ new statement
      §this CELL+ @ 40 + ( equation.ThreadGroup.groups )       @
      0<>
      
      \ new statement
      IF
         
         \ new statement
         
         \ new statement
         
         1 VALLOCATE LOCAL §tempvar
         ngroupsSnapshot -2686975 JavaArray§1352878592.table -227194368 EXECUTE-NEW
         DUP 8 §base0 + V! TO groupsSnapshot
         
         §tempvar SETVTOP
         
         PURGE 1
         
         
         \ new statement
         §this >R
         §this CELL+ @ 40 + ( equation.ThreadGroup.groups )          @
         0
         groupsSnapshot
         0
         ngroupsSnapshot
         R> -22060800 TRUE ( equation.ThreadGroup.arraycopy§-22060800 ) EXECUTE-METHOD
         
         \ new statement
         0 §break28578 LABEL
         
         \ new statement
         
         \ new statement
         DROP
         
         \ new statement
      ELSE
         
         \ new statement
         
         \ new statement
         0
         DUP 8 §base0 + V! TO groupsSnapshot
         
         \ new statement
         0 §break28579 LABEL
         
         \ new statement
         
         \ new statement
         DROP
         
         \ new statement
      ENDIF
      
      \ new statement
      0 §break28577 LABEL
      
      \ new statement
      
      \ new statement
      0>!
      IF
         10001H - §break28571 BRANCH
      ENDIF
      DROP
      
      \ new statement
   ENDIF
   
   \ new statement
   0 §break28571 LABEL
   
   \ new statement
   §synchronized0 1974299904 TRUE ( Object.monitorLeave ) EXECUTE-METHOD
   PURGE 1
   
   \ new statement
   
   PURGE 1
   
   \ new statement
   0>!
   IF
      10001H - §break28570 BRANCH
   ENDIF
   DROP
   
   \ new statement
   recurse
   
   \ new statement
   IF
      
      \ new statement
      
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
         ngroupsSnapshot
         <
         
         \ new statement
      WHILE
         
         \ new statement
         groupsSnapshot >R
         i
         R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
         @  " ThreadGroup " CASTTO >R
         list
         n
         TRUE
         R> -596417280 TRUE ( equation.ThreadGroup.enumerate§-596417280 ) EXECUTE-METHOD
         TO n
         
         \ new statement
         
         \ new statement
         i 1+ TO i
         
         \ new statement
      REPEAT
      
      \ new statement
      0 §break28581 LABEL
      
      \ new statement
      
      PURGE 1
      
      \ new statement
      DROP
      
      \ new statement
      0 §break28580 LABEL
      
      \ new statement
      
      \ new statement
      0>!
      IF
         10001H - §break28570 BRANCH
      ENDIF
      DROP
      
      \ new statement
   ENDIF
   
   \ new statement
   n
   0 §break28570 BRANCH
   
   \ new statement
   0 §break28570 LABEL
   
   \ new statement
   
   
   
   
   
   §base0 SETVTOP
   PURGE 7
   
   \ new statement
   DROP
;
( *
* Copies into the specified array references to every active
* subgroup in this thread group.
* <p>
* First, the <code>checkAccess</code> method of this thread group is
* called with no arguments; this may result in a security exception.
* <p>
* An application should use the <code>activeGroupCount</code>
* method to get an estimate of how big the array should be. If the
* array is too short to hold all the thread groups, the extra thread
* groups are silently ignored.
*
* @param   list   an array into which to place the list of thread groups.
* @return  the number of thread groups put into the array.
* @exception  SecurityException  if the current thread does not
*               have permission to enumerate this thread group.
* @see     java.lang.ThreadGroup#activeGroupCount()
* @see     java.lang.ThreadGroup#checkAccess()
* @since   JDK1.0
)

:LOCAL ThreadGroup.enumerate§189949184
   2 VALLOCATE LOCAL §base0
   DUP 4 §base0 + V! LOCAL §this
   DUP 0 §base0 + V! LOCAL list
   
   \ new statement
   
   \ new statement
   §this >R
   R> 354116352 TRUE ( equation.ThreadGroup.checkAccess§354116352 ) EXECUTE-METHOD
   
   \ new statement
   §this >R
   list
   0
   TRUE
   R> -1064803072 TRUE ( equation.ThreadGroup.enumerate§-1064803072 ) EXECUTE-METHOD
   0 §break28590 BRANCH
   
   \ new statement
   0 §break28590 LABEL
   
   \ new statement
   
   §base0 SETVTOP
   PURGE 3
   
   \ new statement
   DROP
;
( *
* Copies into the specified array every active thread in this
* thread group. If the <code>recurse</code> flag is
* <code>true</code>, references to every active thread in this
* thread's subgroups are also included. If the array is too short to
* hold all the threads, the extra threads are silently ignored.
* <p>
* First, the <code>checkAccess</code> method of this thread group is
* called with no arguments; this may result in a security exception.
* <p>
* An application should use the <code>activeCount</code> method to
* get an estimate of how big the array should be.
*
* @param   list      an array into which to place the list of threads.
* @param   recurse   a flag indicating whether also to include threads
*                    in thread groups that are subgroups of this
*                    thread group.
* @return  the number of threads placed into the array.
* @exception  SecurityException  if the current thread does not
*               have permission to enumerate this thread group.
* @see     java.lang.ThreadGroup#activeCount()
* @see     java.lang.ThreadGroup#checkAccess()
* @since   JDK1.0
)

:LOCAL ThreadGroup.enumerate§511730944
   2 VALLOCATE LOCAL §base0
   DUP 4 §base0 + V! LOCAL §this
   LOCAL recurse
   DUP 0 §base0 + V! LOCAL list
   
   \ new statement
   
   \ new statement
   §this >R
   R> 354116352 TRUE ( equation.ThreadGroup.checkAccess§354116352 ) EXECUTE-METHOD
   
   \ new statement
   §this >R
   list
   0
   recurse
   R> -596417280 TRUE ( equation.ThreadGroup.enumerate§-596417280 ) EXECUTE-METHOD
   0 §break28569 BRANCH
   
   \ new statement
   0 §break28569 LABEL
   
   \ new statement
   
   
   §base0 SETVTOP
   PURGE 4
   
   \ new statement
   DROP
;
( *
* Returns the maximum priority of this thread group. Threads that are
* part of this group cannot have a higher priority than the maximum
* priority.
*
* @return  the maximum priority that a thread in this thread group
*          can have.
* @see     #setMaxPriority
* @since   JDK1.0
)

:LOCAL ThreadGroup.getMaxPriority§836921088
   1 VALLOCATE LOCAL §base0
   DUP 0 §base0 + V! LOCAL §this
   
   \ new statement
   
   \ new statement
   §this CELL+ @ 44 + ( equation.ThreadGroup.maxPriority )    @
   0 §break28540 BRANCH
   
   \ new statement
   0 §break28540 LABEL
   
   \ new statement
   §base0 SETVTOP
   PURGE 2
   
   \ new statement
   DROP
;
( *
* Returns the name of this thread group.
*
* @return  the name of this thread group.
* @since   JDK1.0
)

:LOCAL ThreadGroup.getName§-1704827136
   1 VALLOCATE LOCAL §base0
   DUP 0 §base0 + V! LOCAL §this
   
   \ new statement
   
   \ new statement
   §this CELL+ @ 32 + ( equation.ThreadGroup.name )    @
   DUP 0 V!
   0 §break28538 BRANCH
   
   \ new statement
   0 §break28538 LABEL
   
   \ new statement
   §base0 SETVTOP
   PURGE 2
   
   \ new statement
   DROP
;
( *
* Returns the parent of this thread group.
* <p>
* First, if the parent is not <code>null</code>, the
* <code>checkAccess</code> method of the parent thread group is
* called with no arguments; this may result in a security exception.
*
* @return  the parent of this thread group. The top-level thread group
*          is the only thread group whose parent is <code>null</code>.
* @exception  SecurityException  if the current thread cannot modify
*               this thread group.
* @see        java.lang.ThreadGroup#checkAccess()
* @see        java.lang.SecurityException
* @see        java.lang.RuntimePermission
* @since   JDK1.0
)

:LOCAL ThreadGroup.getParent§-274962688
   1 VALLOCATE LOCAL §base0
   DUP 0 §base0 + V! LOCAL §this
   
   \ new statement
   
   \ new statement
   §this CELL+ @ 28 + ( equation.ThreadGroup.parent )    @
   DUP 0 V!
   0 §break28539 BRANCH
   
   \ new statement
   0 §break28539 LABEL
   
   \ new statement
   §base0 SETVTOP
   PURGE 2
   
   \ new statement
   DROP
;
( *
* Interrupts all threads in this thread group.
* <p>
* First, the <code>checkAccess</code> method of this thread group is
* called with no arguments; this may result in a security exception.
* <p>
* This method then calls the <code>interrupt</code> method on all the
* threads in this thread group and in all of its subgroups.
*
* @exception  SecurityException  if the current thread is not allowed
*               to access this thread group or any of the threads in
*               the thread group.
* @see        java.lang.Thread#interrupt()
* @see        java.lang.SecurityException
* @see        java.lang.ThreadGroup#checkAccess()
* @since      1.2
)

:LOCAL ThreadGroup.interrupt§384592128
   2 VALLOCATE LOCAL §base0
   DUP 0 §base0 + V! LOCAL §this
   
   \ new statement
   0 DUP
   LOCALS groupsSnapshot ngroupsSnapshot |
   
   \ new statement
   
   \ new statement
   §this
   LOCAL §synchronized0
   ThreadGroup._staticThread  @
   §synchronized0 801271040 TRUE ( Object.monitorEnter ) EXECUTE-METHOD
   
   \ new statement
   §this >R
   R> 354116352 TRUE ( equation.ThreadGroup.checkAccess§354116352 ) EXECUTE-METHOD
   
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
      §this CELL+ @ 60 + ( equation.ThreadGroup.nthreads )       @
      <
      
      \ new statement
   WHILE
      
      \ new statement
      §this CELL+ @ 36 + ( equation.ThreadGroup.threads ) @ >R
      i
      R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
      @  " Thread " CASTTO >R
      R> 384592128 TRUE ( equation.Thread.interrupt§384592128 ) EXECUTE-METHOD
      
      \ new statement
      
      \ new statement
      i 1+ TO i
      
      \ new statement
   REPEAT
   
   \ new statement
   0 §break28606 LABEL
   
   \ new statement
   
   PURGE 1
   
   \ new statement
   DROP
   
   \ new statement
   §this CELL+ @ 64 + ( equation.ThreadGroup.ngroups )    @
   TO ngroupsSnapshot
   
   \ new statement
   §this CELL+ @ 40 + ( equation.ThreadGroup.groups )    @
   0<>
   
   \ new statement
   IF
      
      \ new statement
      
      \ new statement
      
      1 VALLOCATE LOCAL §tempvar
      ngroupsSnapshot -2686975 JavaArray§1352878592.table -227194368 EXECUTE-NEW
      DUP 4 §base0 + V! TO groupsSnapshot
      
      §tempvar SETVTOP
      
      PURGE 1
      
      
      \ new statement
      §this >R
      §this CELL+ @ 40 + ( equation.ThreadGroup.groups )       @
      0
      groupsSnapshot
      0
      ngroupsSnapshot
      R> -22060800 TRUE ( equation.ThreadGroup.arraycopy§-22060800 ) EXECUTE-METHOD
      
      \ new statement
      0 §break28608 LABEL
      
      \ new statement
      
      \ new statement
      DROP
      
      \ new statement
   ELSE
      
      \ new statement
      
      \ new statement
      0
      DUP 4 §base0 + V! TO groupsSnapshot
      
      \ new statement
      0 §break28609 LABEL
      
      \ new statement
      
      \ new statement
      DROP
      
      \ new statement
   ENDIF
   
   \ new statement
   0 §break28605 LABEL
   
   \ new statement
   §synchronized0 1974299904 TRUE ( Object.monitorLeave ) EXECUTE-METHOD
   PURGE 1
   
   \ new statement
   
   \ new statement
   DROP
   
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
      ngroupsSnapshot
      <
      
      \ new statement
   WHILE
      
      \ new statement
      groupsSnapshot >R
      i
      R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
      @  " ThreadGroup " CASTTO >R
      R> 384592128 TRUE ( equation.ThreadGroup.interrupt§384592128 ) EXECUTE-METHOD
      
      \ new statement
      
      \ new statement
      i 1+ TO i
      
      \ new statement
   REPEAT
   
   \ new statement
   0 §break28610 LABEL
   
   \ new statement
   
   PURGE 1
   
   \ new statement
   DROP
   
   \ new statement
   0 §break28604 LABEL
   
   \ new statement
   
   
   §base0 SETVTOP
   PURGE 4
   
   \ new statement
   DROP
;
( *
* Tests if this thread group is a daemon thread group. A
* daemon thread group is automatically destroyed when its last
* thread is stopped or its last thread group is destroyed.
*
* @return  <code>true</code> if this thread group is a daemon thread group;
*          <code>false</code> otherwise.
* @since   JDK1.0
)

:LOCAL ThreadGroup.isDaemon§-1531156224
   1 VALLOCATE LOCAL §base0
   DUP 0 §base0 + V! LOCAL §this
   
   \ new statement
   
   \ new statement
   §this CELL+ @ 52 + ( equation.ThreadGroup.daemon )    @
   0 §break28541 BRANCH
   
   \ new statement
   0 §break28541 LABEL
   
   \ new statement
   §base0 SETVTOP
   PURGE 2
   
   \ new statement
   DROP
;
( *
* Tests if this thread group has been destroyed.
*
* @return  true if this object is destroyed
* @since   JDK1.1
)

:LOCAL ThreadGroup.isDestroyed§-1137940224
   1 VALLOCATE LOCAL §base0
   DUP 0 §base0 + V! LOCAL §this
   
   \ new statement
   
   \ new statement
   §this LOCAL §synchronized0
   ThreadGroup._staticThread  @
   §synchronized0 801271040 TRUE ( Object.monitorEnter ) EXECUTE-METHOD
   
   \ new statement
   §this CELL+ @ 48 + ( equation.ThreadGroup.destroyed )    @
   0 §break28542 BRANCH
   
   \ new statement
   0 §break28542 LABEL
   
   \ new statement
   §synchronized0 1974299904 TRUE ( Object.monitorLeave ) EXECUTE-METHOD
   PURGE 1
   
   \ new statement
   §base0 SETVTOP
   PURGE 2
   
   \ new statement
   DROP
;
( *
* Prints information about this thread group to the standard
* output. This method is useful only for debugging.
*
* @since   JDK1.0
)

:LOCAL ThreadGroup.list§709848064
   1 VALLOCATE LOCAL §base0
   DUP 0 §base0 + V! LOCAL §this
   
   \ new statement
   
   \ new statement
   \ list(System.out, 0);
   
   
   \ new statement
   0 §break28662 LABEL
   
   \ new statement
   §base0 SETVTOP
   PURGE 2
   
   \ new statement
   DROP
;
( *
* Tests if this thread group is either the thread group
* argument or one of its ancestor thread groups.
*
* @param   g   a thread group.
* @return  <code>true</code> if this thread group is the thread group
*          argument or one of its ancestor thread groups;
*          <code>false</code> otherwise.
* @since   JDK1.0
)

:LOCAL ThreadGroup.parentOf§666202112
   2 VALLOCATE LOCAL §base0
   DUP 4 §base0 + V! LOCAL §this
   DUP 0 §base0 + V! LOCAL g
   
   \ new statement
   
   \ new statement
   
   \ new statement
   BEGIN
      
      \ new statement
      g
      0<>
      
      \ new statement
   WHILE
      
      \ new statement
      g
      §this
      =
      
      \ new statement
      IF
         
         \ new statement
         
         \ new statement
         TRUE
         131074 §break28559 BRANCH
         
         \ new statement
         0 §break28559 LABEL
         
         \ new statement
         
         \ new statement
         0>!
         IF
            10001H - 0<! §break28557 0BRANCH
         ENDIF
         DROP
         
         \ new statement
      ENDIF
      
      \ new statement
      
      \ new statement
      g CELL+ @ 28 + ( equation.ThreadGroup.parent )       @
      DUP §base0 V! TO g
      
      \ new statement
   REPEAT
   
   \ new statement
   0 §break28557 LABEL
   
   \ new statement
   
   \ new statement
   0>!
   IF
      10001H - §break28556 BRANCH
   ENDIF
   DROP
   
   \ new statement
   FALSE
   0 §break28556 BRANCH
   
   \ new statement
   0 §break28556 LABEL
   
   \ new statement
   
   §base0 SETVTOP
   PURGE 3
   
   \ new statement
   DROP
;
( *
* Removes the specified Thread group from this group.
* @param g the Thread group to be removed
* @return if this Thread has already been destroyed.
)

:LOCAL ThreadGroup.remove§-1031376384
   2 VALLOCATE LOCAL §base0
   DUP 4 §base0 + V! LOCAL §this
   DUP 0 §base0 + V! LOCAL g
   
   \ new statement
   
   \ new statement
   
   \ new statement
   §this
   LOCAL §synchronized0
   ThreadGroup._staticThread  @
   §synchronized0 801271040 TRUE ( Object.monitorEnter ) EXECUTE-METHOD
   
   \ new statement
   §this CELL+ @ 48 + ( equation.ThreadGroup.destroyed )    @
   
   \ new statement
   IF
      
      \ new statement
      
      \ new statement
      131074 §break28643 BRANCH
      
      \ new statement
      0 §break28643 LABEL
      
      \ new statement
      
      \ new statement
      0>!
      IF
         10001H - §break28642 BRANCH
      ENDIF
      DROP
      
      \ new statement
   ENDIF
   
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
      §this CELL+ @ 64 + ( equation.ThreadGroup.ngroups )       @
      <
      
      \ new statement
   WHILE
      
      \ new statement
      §this CELL+ @ 40 + ( equation.ThreadGroup.groups ) @ >R
      i
      R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
      @  " ThreadGroup " CASTTO
      g
      =
      
      \ new statement
      IF
         
         \ new statement
         
         \ new statement
         §this CELL+ @ 64 + ( equation.ThreadGroup.ngroups )          @
         1
         -
         §this CELL+ @ 64 + ( equation.ThreadGroup.ngroups )          !
         
         \ new statement
         §this >R
         §this CELL+ @ 40 + ( equation.ThreadGroup.groups )          @
         i
         1
         +
         §this CELL+ @ 40 + ( equation.ThreadGroup.groups )          @
         i
         §this CELL+ @ 64 + ( equation.ThreadGroup.ngroups )          @
         i
         -
         R> -22060800 TRUE ( equation.ThreadGroup.arraycopy§-22060800 ) EXECUTE-METHOD
         
         \ new statement
         \  Zap dangling reference to the dead group so that
         \  the garbage collector will collect it.
         
         
         \ new statement
         
         0
         LOCALS 0§ |
         
         §this CELL+ @ 64 + ( equation.ThreadGroup.ngroups )          @ TO 0§
         0
         §this CELL+ @ 40 + ( equation.ThreadGroup.groups ) @ >R
         0§
         R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
         !
         
         PURGE 1
         
         
         \ new statement
         65537 §break28646 BRANCH
         
         \ new statement
         0 §break28646 LABEL
         
         \ new statement
         
         \ new statement
         0>!
         IF
            10001H - 0<! §break28644 0BRANCH
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
   0 §break28644 LABEL
   
   \ new statement
   
   PURGE 1
   
   \ new statement
   0>!
   IF
      10001H - §break28642 BRANCH
   ENDIF
   DROP
   
   \ new statement
   §this CELL+ @ 60 + ( equation.ThreadGroup.nthreads )    @
   0=
   
   \ new statement
   IF
      
      \ new statement
      
      \ new statement
      §this >R
      std29360 VAL
      R> 2130144768 TRUE ( equation.Object.notifyAll§2130144768 ) EXECUTE-METHOD
      FALSE DUP
      IF
         std29360 LABEL TRUE
      ENDIF
      IF
         JavaArray.handler§-1096259584
      ENDIF
      
      \ new statement
      0 §break28647 LABEL
      
      \ new statement
      
      \ new statement
      0>!
      IF
         10001H - §break28642 BRANCH
      ENDIF
      DROP
      
      \ new statement
   ENDIF
   
   \ new statement
   §this CELL+ @ 52 + ( equation.ThreadGroup.daemon )    @ and_29366 0BRANCH! DROP
   §this CELL+ @ 60 + ( equation.ThreadGroup.nthreads )    @
   0=
   and_29366 0BRANCH! DROP
   §this CELL+ @ 64 + ( equation.ThreadGroup.ngroups )    @
   0=
   and_29366 LABEL
   
   \ new statement
   IF
      
      \ new statement
      
      \ new statement
      §this >R
      R> 855073792 TRUE ( equation.ThreadGroup.destroy§855073792 ) EXECUTE-METHOD
      
      \ new statement
      0 §break28648 LABEL
      
      \ new statement
      
      \ new statement
      0>!
      IF
         10001H - §break28642 BRANCH
      ENDIF
      DROP
      
      \ new statement
   ENDIF
   
   \ new statement
   0 §break28642 LABEL
   
   \ new statement
   §synchronized0 1974299904 TRUE ( Object.monitorLeave ) EXECUTE-METHOD
   PURGE 1
   
   \ new statement
   
   \ new statement
   0>!
   IF
      10001H - §break28641 BRANCH
   ENDIF
   DROP
   
   \ new statement
   0 §break28641 LABEL
   
   \ new statement
   
   §base0 SETVTOP
   PURGE 3
   
   \ new statement
   DROP
;
( *
* Removes the specified Thread from this group.
* @param t the Thread to be removed
* @return if the Thread has already been destroyed.
)

:LOCAL ThreadGroup.remove§453997056
   2 VALLOCATE LOCAL §base0
   DUP 4 §base0 + V! LOCAL §this
   DUP 0 §base0 + V! LOCAL t
   
   \ new statement
   
   \ new statement
   
   \ new statement
   §this
   LOCAL §synchronized0
   ThreadGroup._staticThread  @
   §synchronized0 801271040 TRUE ( Object.monitorEnter ) EXECUTE-METHOD
   
   \ new statement
   §this CELL+ @ 48 + ( equation.ThreadGroup.destroyed )    @
   
   \ new statement
   IF
      
      \ new statement
      
      \ new statement
      131074 §break28656 BRANCH
      
      \ new statement
      0 §break28656 LABEL
      
      \ new statement
      
      \ new statement
      0>!
      IF
         10001H - §break28655 BRANCH
      ENDIF
      DROP
      
      \ new statement
   ENDIF
   
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
      §this CELL+ @ 60 + ( equation.ThreadGroup.nthreads )       @
      <
      
      \ new statement
   WHILE
      
      \ new statement
      §this CELL+ @ 36 + ( equation.ThreadGroup.threads ) @ >R
      i
      R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
      @  " Thread " CASTTO
      t
      =
      
      \ new statement
      IF
         
         \ new statement
         
         \ new statement
         §this >R
         §this CELL+ @ 36 + ( equation.ThreadGroup.threads )          @
         i
         1
         +
         §this CELL+ @ 36 + ( equation.ThreadGroup.threads )          @
         i
         §this CELL+ @ 60 + ( equation.ThreadGroup.nthreads )          A:R@ @ 1- DUP R@ !
         A:DROP
         i
         -
         R> 794779904 TRUE ( equation.ThreadGroup.arraycopy§794779904 ) EXECUTE-METHOD
         
         \ new statement
         \  Zap dangling reference to the dead thread so that
         \  the garbage collector will collect it.
         
         
         \ new statement
         
         0
         LOCALS 0§ |
         
         §this CELL+ @ 60 + ( equation.ThreadGroup.nthreads )          @ TO 0§
         0
         §this CELL+ @ 36 + ( equation.ThreadGroup.threads ) @ >R
         0§
         R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
         !
         
         PURGE 1
         
         
         \ new statement
         65537 §break28659 BRANCH
         
         \ new statement
         0 §break28659 LABEL
         
         \ new statement
         
         \ new statement
         0>!
         IF
            10001H - 0<! §break28657 0BRANCH
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
   0 §break28657 LABEL
   
   \ new statement
   
   PURGE 1
   
   \ new statement
   0>!
   IF
      10001H - §break28655 BRANCH
   ENDIF
   DROP
   
   \ new statement
   §this CELL+ @ 60 + ( equation.ThreadGroup.nthreads )    @
   0=
   
   \ new statement
   IF
      
      \ new statement
      
      \ new statement
      §this >R
      std29417 VAL
      R> 2130144768 TRUE ( equation.Object.notifyAll§2130144768 ) EXECUTE-METHOD
      FALSE DUP
      IF
         std29417 LABEL TRUE
      ENDIF
      IF
         JavaArray.handler§-1096259584
      ENDIF
      
      \ new statement
      0 §break28660 LABEL
      
      \ new statement
      
      \ new statement
      0>!
      IF
         10001H - §break28655 BRANCH
      ENDIF
      DROP
      
      \ new statement
   ENDIF
   
   \ new statement
   §this CELL+ @ 52 + ( equation.ThreadGroup.daemon )    @ and_29423 0BRANCH! DROP
   §this CELL+ @ 60 + ( equation.ThreadGroup.nthreads )    @
   0=
   and_29423 0BRANCH! DROP
   §this CELL+ @ 64 + ( equation.ThreadGroup.ngroups )    @
   0=
   and_29423 LABEL
   
   \ new statement
   IF
      
      \ new statement
      
      \ new statement
      §this >R
      R> 855073792 TRUE ( equation.ThreadGroup.destroy§855073792 ) EXECUTE-METHOD
      
      \ new statement
      0 §break28661 LABEL
      
      \ new statement
      
      \ new statement
      0>!
      IF
         10001H - §break28655 BRANCH
      ENDIF
      DROP
      
      \ new statement
   ENDIF
   
   \ new statement
   0 §break28655 LABEL
   
   \ new statement
   §synchronized0 1974299904 TRUE ( Object.monitorLeave ) EXECUTE-METHOD
   PURGE 1
   
   \ new statement
   
   \ new statement
   0>!
   IF
      10001H - §break28654 BRANCH
   ENDIF
   DROP
   
   \ new statement
   0 §break28654 LABEL
   
   \ new statement
   
   §base0 SETVTOP
   PURGE 3
   
   \ new statement
   DROP
;
( *
* Resumes all threads in this thread group.
* <p>
* First, the <code>checkAccess</code> method of this thread group is
* called with no arguments; this may result in a security exception.
* <p>
* This method then calls the <code>resume</code> method on all the
* threads in this thread group and in all of its sub groups.
*
* @exception  SecurityException  if the current thread is not allowed to
*               access this thread group or any of the threads in the
*               thread group.
* @see        java.lang.SecurityException
* @see        java.lang.Thread#resume()
* @see        java.lang.ThreadGroup#checkAccess()
* @since      JDK1.0
* @deprecated    This method is used solely in conjunction with
*      <tt>Thread.suspend</tt> and <tt>ThreadGroup.suspend</tt>,
*       both of which have been deprecated, as they are inherently
*       deadlock-prone.  See {@link Thread#suspend} for details.
)

:LOCAL ThreadGroup.resume§599945728
   2 VALLOCATE LOCAL §base0
   DUP 0 §base0 + V! LOCAL §this
   
   \ new statement
   0 DUP
   LOCALS groupsSnapshot ngroupsSnapshot |
   
   \ new statement
   
   \ new statement
   §this
   LOCAL §synchronized0
   ThreadGroup._staticThread  @
   §synchronized0 801271040 TRUE ( Object.monitorEnter ) EXECUTE-METHOD
   
   \ new statement
   §this >R
   R> 354116352 TRUE ( equation.ThreadGroup.checkAccess§354116352 ) EXECUTE-METHOD
   
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
      §this CELL+ @ 60 + ( equation.ThreadGroup.nthreads )       @
      <
      
      \ new statement
   WHILE
      
      \ new statement
      §this CELL+ @ 36 + ( equation.ThreadGroup.threads ) @ >R
      i
      R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
      @  " Thread " CASTTO >R
      R> 599945728 TRUE ( equation.Thread.resume§599945728 ) EXECUTE-METHOD
      
      \ new statement
      
      \ new statement
      i 1+ TO i
      
      \ new statement
   REPEAT
   
   \ new statement
   0 §break28621 LABEL
   
   \ new statement
   
   PURGE 1
   
   \ new statement
   DROP
   
   \ new statement
   §this CELL+ @ 64 + ( equation.ThreadGroup.ngroups )    @
   TO ngroupsSnapshot
   
   \ new statement
   §this CELL+ @ 40 + ( equation.ThreadGroup.groups )    @
   0<>
   
   \ new statement
   IF
      
      \ new statement
      
      \ new statement
      
      1 VALLOCATE LOCAL §tempvar
      ngroupsSnapshot -2686975 JavaArray§1352878592.table -227194368 EXECUTE-NEW
      DUP 4 §base0 + V! TO groupsSnapshot
      
      §tempvar SETVTOP
      
      PURGE 1
      
      
      \ new statement
      §this >R
      §this CELL+ @ 40 + ( equation.ThreadGroup.groups )       @
      0
      groupsSnapshot
      0
      ngroupsSnapshot
      R> -22060800 TRUE ( equation.ThreadGroup.arraycopy§-22060800 ) EXECUTE-METHOD
      
      \ new statement
      0 §break28623 LABEL
      
      \ new statement
      
      \ new statement
      DROP
      
      \ new statement
   ELSE
      
      \ new statement
      
      \ new statement
      0
      DUP 4 §base0 + V! TO groupsSnapshot
      
      \ new statement
      0 §break28624 LABEL
      
      \ new statement
      
      \ new statement
      DROP
      
      \ new statement
   ENDIF
   
   \ new statement
   0 §break28620 LABEL
   
   \ new statement
   §synchronized0 1974299904 TRUE ( Object.monitorLeave ) EXECUTE-METHOD
   PURGE 1
   
   \ new statement
   
   \ new statement
   DROP
   
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
      ngroupsSnapshot
      <
      
      \ new statement
   WHILE
      
      \ new statement
      groupsSnapshot >R
      i
      R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
      @  " ThreadGroup " CASTTO >R
      R> 599945728 TRUE ( equation.ThreadGroup.resume§599945728 ) EXECUTE-METHOD
      
      \ new statement
      
      \ new statement
      i 1+ TO i
      
      \ new statement
   REPEAT
   
   \ new statement
   0 §break28625 LABEL
   
   \ new statement
   
   PURGE 1
   
   \ new statement
   DROP
   
   \ new statement
   0 §break28619 LABEL
   
   \ new statement
   
   
   §base0 SETVTOP
   PURGE 4
   
   \ new statement
   DROP
;
( *
* Changes the daemon status of this thread group.
* <p>
* First, the <code>checkAccess</code> method of this thread group is
* called with no arguments; this may result in a security exception.
* <p>
* A daemon thread group is automatically destroyed when its last
* thread is stopped or its last thread group is destroyed.
*
* @param      daemon   if <code>true</code>, marks this thread group as
*                      a daemon thread group; otherwise, marks this
*                      thread group as normal.
* @exception  SecurityException  if the current thread cannot modify
*               this thread group.
* @see        java.lang.SecurityException
* @see        java.lang.ThreadGroup#checkAccess()
* @since      JDK1.0
)

:LOCAL ThreadGroup.setDaemon§-1235782912
   1 VALLOCATE LOCAL §base0
   DUP 0 §base0 + V! LOCAL §this
   LOCAL daemon
   
   \ new statement
   
   \ new statement
   daemon
   §this CELL+ @ 52 + ( equation.ThreadGroup.daemon )    !
   
   \ new statement
   0 §break28543 LABEL
   
   \ new statement
   
   §base0 SETVTOP
   PURGE 3
   
   \ new statement
   DROP
;
( *
* Sets the maximum priority of the group. Threads in the thread
* group that already have a higher priority are not affected.
* <p>
* First, the <code>checkAccess</code> method of this thread group is
* called with no arguments; this may result in a security exception.
* <p>
* If the <code>pri</code> argument is less than
* {@link Thread#MIN_PRIORITY} or greater than
* {@link Thread#MAX_PRIORITY}, the maximum priority of the group
* remains unchanged.
* <p>
* Otherwise, the priority of this ThreadGroup object is set to the
* smaller of the specified <code>pri</code> and the maximum permitted
* priority of the parent of this thread group. (If this thread group
* is the system thread group, which has no parent, then its maximum
* priority is simply set to <code>pri</code>.) Then this method is
* called recursively, with <code>pri</code> as its argument, for
* every thread group that belongs to this thread group.
*
* @param      pri   the new priority of the thread group.
* @exception  SecurityException  if the current thread cannot modify
*               this thread group.
* @see        #getMaxPriority
* @see        java.lang.SecurityException
* @see        java.lang.ThreadGroup#checkAccess()
* @since      JDK1.0
)

:LOCAL ThreadGroup.setMaxPriority§-1956941056
   2 VALLOCATE LOCAL §base0
   DUP 0 §base0 + V! LOCAL §this
   LOCAL pri
   
   \ new statement
   0 DUP
   LOCALS groupsSnapshot ngroupsSnapshot |
   
   \ new statement
   
   \ new statement
   §this
   LOCAL §synchronized0
   ThreadGroup._staticThread  @
   §synchronized0 801271040 TRUE ( Object.monitorEnter ) EXECUTE-METHOD
   
   \ new statement
   pri
   Thread.MIN_PRIORITY
   @
   <
   
   \ new statement
   IF
      
      \ new statement
      
      \ new statement
      Thread.MIN_PRIORITY
      @
      §this CELL+ @ 44 + ( equation.ThreadGroup.maxPriority )       !
      
      \ new statement
      0 §break28546 LABEL
      
      \ new statement
      
      \ new statement
      DROP
      
      \ new statement
   ELSE
      
      \ new statement
      pri
      §this CELL+ @ 44 + ( equation.ThreadGroup.maxPriority )       @
      <
      
      \ new statement
      IF
         
         \ new statement
         
         \ new statement
         pri
         §this CELL+ @ 44 + ( equation.ThreadGroup.maxPriority )          !
         
         \ new statement
         0 §break28547 LABEL
         
         \ new statement
         
         \ new statement
         DROP
         
         \ new statement
      ENDIF
      
      \ new statement
   ENDIF
   
   \ new statement
   §this CELL+ @ 64 + ( equation.ThreadGroup.ngroups )    @
   TO ngroupsSnapshot
   
   \ new statement
   §this CELL+ @ 40 + ( equation.ThreadGroup.groups )    @
   0<>
   
   \ new statement
   IF
      
      \ new statement
      
      \ new statement
      
      1 VALLOCATE LOCAL §tempvar
      ngroupsSnapshot -2686975 JavaArray§1352878592.table -227194368 EXECUTE-NEW
      DUP 4 §base0 + V! TO groupsSnapshot
      
      §tempvar SETVTOP
      
      PURGE 1
      
      
      \ new statement
      §this >R
      §this CELL+ @ 40 + ( equation.ThreadGroup.groups )       @
      0
      groupsSnapshot
      0
      ngroupsSnapshot
      R> -22060800 TRUE ( equation.ThreadGroup.arraycopy§-22060800 ) EXECUTE-METHOD
      
      \ new statement
      0 §break28548 LABEL
      
      \ new statement
      
      \ new statement
      DROP
      
      \ new statement
   ELSE
      
      \ new statement
      
      \ new statement
      0
      DUP 4 §base0 + V! TO groupsSnapshot
      
      \ new statement
      0 §break28549 LABEL
      
      \ new statement
      
      \ new statement
      DROP
      
      \ new statement
   ENDIF
   
   \ new statement
   0 §break28545 LABEL
   
   \ new statement
   §synchronized0 1974299904 TRUE ( Object.monitorLeave ) EXECUTE-METHOD
   PURGE 1
   
   \ new statement
   
   \ new statement
   DROP
   
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
      ngroupsSnapshot
      <
      
      \ new statement
   WHILE
      
      \ new statement
      groupsSnapshot >R
      i
      R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
      @  " ThreadGroup " CASTTO >R
      pri
      R> -1956941056 TRUE ( equation.ThreadGroup.setMaxPriority§-1956941056 ) EXECUTE-METHOD
      
      \ new statement
      
      \ new statement
      i 1+ TO i
      
      \ new statement
   REPEAT
   
   \ new statement
   0 §break28550 LABEL
   
   \ new statement
   
   PURGE 1
   
   \ new statement
   DROP
   
   \ new statement
   0 §break28544 LABEL
   
   \ new statement
   
   
   
   §base0 SETVTOP
   PURGE 5
   
   \ new statement
   DROP
;
( *
* Stops all threads in this thread group.
* <p>
* First, the <code>checkAccess</code> method of this thread group is
* called with no arguments; this may result in a security exception.
* <p>
* This method then calls the <code>stop</code> method on all the
* threads in this thread group and in all of its subgroups.
*
* @exception  SecurityException  if the current thread is not allowed
*               to access this thread group or any of the threads in
*               the thread group.
* @see        java.lang.SecurityException
* @see        java.lang.Thread#stop()
* @see        java.lang.ThreadGroup#checkAccess()
* @since      JDK1.0
* @deprecated    This method is inherently unsafe.  See
*     {@link Thread#stop} for details.
)

:LOCAL ThreadGroup.stop§-74747136
   1 VALLOCATE LOCAL §base0
   DUP 0 §base0 + V! LOCAL §this
   
   \ new statement
   
   \ new statement
   §this >R
   FALSE
   R> 546927360 TRUE ( equation.ThreadGroup.stopOrSuspend§546927360 ) EXECUTE-METHOD
   
   \ new statement
   IF
      
      \ new statement
      
      1 VALLOCATE LOCAL §tempvar
      Thread.currentThread§-1671470336 DUP §tempvar V!
      >R
      R> -74747136 TRUE ( equation.Thread.stop§-74747136 ) EXECUTE-METHOD
      
      
      §tempvar SETVTOP
      
      PURGE 1
      
      
      \ new statement
   ENDIF
   
   \ new statement
   0 §break28603 LABEL
   
   \ new statement
   §base0 SETVTOP
   PURGE 2
   
   \ new statement
   DROP
;
( *
* Helper method: recursively stops or suspends (as directed by the
* boolean argument) all of the threads in this thread group and its
* subgroups, except the current thread.  This method returns true
* if (and only if) the current thread is found to be in this thread
* group or one of its subgroups.
)

:LOCAL ThreadGroup.stopOrSuspend§546927360
   3 VALLOCATE LOCAL §base0
   DUP 0 §base0 + V! LOCAL §this
   LOCAL suspend
   
   \ new statement
   0 DUP DUP DUP
   LOCALS groupsSnapshot ngroupsSnapshot suicide us |
   
   \ new statement
   FALSE
   TO suicide
   
   \ new statement
   Thread.currentThread§-1671470336
   DUP 4 §base0 + V! TO us
   
   \ new statement
   0
   DUP 8 §base0 + V! TO groupsSnapshot
   
   \ new statement
   
   \ new statement
   §this
   LOCAL §synchronized0
   ThreadGroup._staticThread  @
   §synchronized0 801271040 TRUE ( Object.monitorEnter ) EXECUTE-METHOD
   
   \ new statement
   §this >R
   R> 354116352 TRUE ( equation.ThreadGroup.checkAccess§354116352 ) EXECUTE-METHOD
   
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
      §this CELL+ @ 60 + ( equation.ThreadGroup.nthreads )       @
      <
      
      \ new statement
   WHILE
      
      \ new statement
      §this CELL+ @ 36 + ( equation.ThreadGroup.threads ) @ >R
      i
      R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
      @  " Thread " CASTTO
      us
      =
      
      \ new statement
      IF
         
         \ new statement
         TRUE
         TO suicide
         
         \ new statement
      ELSE
         
         \ new statement
         suspend
         
         \ new statement
         IF
            
            \ new statement
            §this CELL+ @ 36 + ( equation.ThreadGroup.threads ) @ >R
            i
            R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
            @  " Thread " CASTTO >R
            R> -2031258880 TRUE ( equation.Thread.suspend§-2031258880 ) EXECUTE-METHOD
            
            \ new statement
         ELSE
            
            \ new statement
            §this CELL+ @ 36 + ( equation.ThreadGroup.threads ) @ >R
            i
            R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
            @  " Thread " CASTTO >R
            R> -74747136 TRUE ( equation.Thread.stop§-74747136 ) EXECUTE-METHOD
            
            \ new statement
         ENDIF
         
         \ new statement
      ENDIF
      
      \ new statement
      
      \ new statement
      i 1+ TO i
      
      \ new statement
   REPEAT
   
   \ new statement
   0 §break28615 LABEL
   
   \ new statement
   
   PURGE 1
   
   \ new statement
   DROP
   
   \ new statement
   §this CELL+ @ 64 + ( equation.ThreadGroup.ngroups )    @
   TO ngroupsSnapshot
   
   \ new statement
   §this CELL+ @ 40 + ( equation.ThreadGroup.groups )    @
   0<>
   
   \ new statement
   IF
      
      \ new statement
      
      \ new statement
      
      1 VALLOCATE LOCAL §tempvar
      ngroupsSnapshot -2686975 JavaArray§1352878592.table -227194368 EXECUTE-NEW
      DUP 8 §base0 + V! TO groupsSnapshot
      
      §tempvar SETVTOP
      
      PURGE 1
      
      
      \ new statement
      §this >R
      §this CELL+ @ 40 + ( equation.ThreadGroup.groups )       @
      0
      groupsSnapshot
      0
      ngroupsSnapshot
      R> -22060800 TRUE ( equation.ThreadGroup.arraycopy§-22060800 ) EXECUTE-METHOD
      
      \ new statement
      0 §break28617 LABEL
      
      \ new statement
      
      \ new statement
      DROP
      
      \ new statement
   ENDIF
   
   \ new statement
   0 §break28614 LABEL
   
   \ new statement
   §synchronized0 1974299904 TRUE ( Object.monitorLeave ) EXECUTE-METHOD
   PURGE 1
   
   \ new statement
   
   \ new statement
   DROP
   
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
      ngroupsSnapshot
      <
      
      \ new statement
   WHILE
      
      \ new statement
      groupsSnapshot >R
      i
      R> -942971136 TRUE ( equation.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
      @  " ThreadGroup " CASTTO >R
      suspend
      R> 546927360 TRUE ( equation.ThreadGroup.stopOrSuspend§546927360 ) EXECUTE-METHOD
      0=! or_29606 0BRANCH DROP
      suicide
      or_29606 LABEL
      TO suicide
      
      \ new statement
      
      \ new statement
      i 1+ TO i
      
      \ new statement
   REPEAT
   
   \ new statement
   0 §break28618 LABEL
   
   \ new statement
   
   PURGE 1
   
   \ new statement
   DROP
   
   \ new statement
   suicide
   0 §break28613 BRANCH
   
   \ new statement
   0 §break28613 LABEL
   
   \ new statement
   
   
   
   
   
   §base0 SETVTOP
   PURGE 7
   
   \ new statement
   DROP
;
( *
* Suspends all threads in this thread group.
* <p>
* First, the <code>checkAccess</code> method of this thread group is
* called with no arguments; this may result in a security exception.
* <p>
* This method then calls the <code>suspend</code> method on all the
* threads in this thread group and in all of its subgroups.
*
* @exception  SecurityException  if the current thread is not allowed
*               to access this thread group or any of the threads in
*               the thread group.
* @see        java.lang.Thread#suspend()
* @see        java.lang.SecurityException
* @see        java.lang.ThreadGroup#checkAccess()
* @since      JDK1.0
* @deprecated    This method is inherently deadlock-prone.  See
*     {@link Thread#suspend} for details.
)

:LOCAL ThreadGroup.suspend§-2031258880
   1 VALLOCATE LOCAL §base0
   DUP 0 §base0 + V! LOCAL §this
   
   \ new statement
   
   \ new statement
   §this >R
   TRUE
   R> 546927360 TRUE ( equation.ThreadGroup.stopOrSuspend§546927360 ) EXECUTE-METHOD
   
   \ new statement
   IF
      
      \ new statement
      
      1 VALLOCATE LOCAL §tempvar
      Thread.currentThread§-1671470336 DUP §tempvar V!
      >R
      R> -2031258880 TRUE ( equation.Thread.suspend§-2031258880 ) EXECUTE-METHOD
      
      
      §tempvar SETVTOP
      
      PURGE 1
      
      
      \ new statement
   ENDIF
   
   \ new statement
   0 §break28612 LABEL
   
   \ new statement
   §base0 SETVTOP
   PURGE 2
   
   \ new statement
   DROP
;
(     void list(PrintStream out, int indent) {
int ngroupsSnapshot;
ThreadGroup[] groupsSnapshot;
synchronized (this) {
for (int j = 0 ; j < indent ; j++) {
out.print(" ");
}
out.println(this);
indent += 4;
for (int i = 0 ; i < nthreads ; i++) {
for (int j = 0 ; j < indent ; j++) {
out.print(" ");
}
out.println(threads[i]);
}
ngroupsSnapshot = ngroups;
if (groups != null) {
groupsSnapshot = new ThreadGroup[ngroupsSnapshot];
arraycopy(groups, 0, groupsSnapshot, 0, ngroupsSnapshot);
} else {
groupsSnapshot = null;
}
}
for (int i = 0 ; i < ngroupsSnapshot ; i++) {
groupsSnapshot[i].list(out, indent);
}
}
)  ( *
* Called by the Java Virtual Machine when a thread in this
* thread group stops because of an uncaught exception.
* <p>
* The <code>uncaughtException</code> method of
* <code>ThreadGroup</code> does the following:
* <ul>
* <li>If this thread group has a parent thread group, the
*     <code>uncaughtException</code> method of that parent is called
*     with the same two arguments.
* <li>Otherwise, this method determines if the <code>Throwable</code>
*     argument is an instance of <code>ThreadDeath</code>. If so, nothing
*     special is done. Otherwise, the <code>Throwable</code>'s
*     <code>printStackTrace</code> method is called to print a stack
*     backtrace to the standard error stream.
* </ul>
* <p>
* Applications can override this method in subclasses of
* <code>ThreadGroup</code> to provide alternative handling of
* uncaught exceptions.
*
* @param   t   the thread that is about to exit.
* @param   e   the uncaught exception.
* @see     java.lang.System#err
* @see     java.lang.ThreadDeath
* @see     java.lang.Throwable#printStackTrace(java.io.PrintStream)
* @since   JDK1.0
public void uncaughtException(Thread t, Throwable e) {
if (parent != null) {
parent.uncaughtException(t, e);
} else if (!(e instanceof ThreadDeath)) {
e.printStackTrace(System.err);
}
}
)  ( *
* Used by VM to control lowmem implicit suspension.
*
* @param b boolean to allow or disallow suspension
* @return true on success
* @since   JDK1.1
* @deprecated The definition of this call depends on {@link #suspend},
*		   which is deprecated.  Further, the behavior of this call
*		   was never specified.
public boolean allowThreadSuspension(boolean b) {
this.vmAllowSuspension = b;
if (!b) {
VM.unsuspendSomeThreads();
}
return true;
}
)  ( *
* Returns a string representation of this Thread group.
*
* @return  a string representation of this thread group.
* @since   JDK1.0
)

:LOCAL ThreadGroup.toString§1621718016
   1 VALLOCATE LOCAL §base0
   DUP 0 §base0 + V! LOCAL §this
   
   \ new statement
   
   \ new statement
   
   8 VALLOCATE LOCAL §tempvar
   U" ThreadGroup[name= " COUNT JavaArray.createUnicode§-675323136 DUP §tempvar V!
   §this >R
   R> -1704827136 TRUE ( equation.ThreadGroup.getName§-1704827136 ) EXECUTE-METHOD DUP §tempvar 4 + V!
   SWAP  -1461427456 TRUE ( equation.String.concat§-1461427456 ) EXECUTE-METHOD DUP §tempvar 8 + V!
   U" ,maxpri= " COUNT JavaArray.createUnicode§-675323136 DUP §tempvar 12 + V!
   SWAP  -1461427456 TRUE ( equation.String.concat§-1461427456 ) EXECUTE-METHOD DUP §tempvar 16 + V!
   §this CELL+ @ 44 + ( equation.ThreadGroup.maxPriority )    @  <# 0 ..R JavaArray.createString§-105880832 DUP §tempvar 20 + V!
   SWAP  -1461427456 TRUE ( equation.String.concat§-1461427456 ) EXECUTE-METHOD DUP §tempvar 24 + V!
   U" ] " COUNT JavaArray.createUnicode§-675323136 DUP §tempvar 28 + V!
   SWAP  -1461427456 TRUE ( equation.String.concat§-1461427456 ) EXECUTE-METHOD
   DUP 0 V!
   
   
   
   
   
   
   
   
   §tempvar SETVTOP
   
   PURGE 1
   
   0 §break28663 BRANCH
   
   \ new statement
   0 §break28663 LABEL
   
   \ new statement
   §base0 SETVTOP
   PURGE 2
   
   \ new statement
   DROP
;
VARIABLE ThreadGroup._staticBlocking
VARIABLE ThreadGroup._staticThread  HERE 4 - SALLOCATE

A:HERE VARIABLE ThreadGroup§-2665472.table 34 DUP 2* CELLS ALLOT R@ ! A:CELL+
-543337472 R@ ! A:CELL+ ThreadGroup.ThreadGroup§-543337472 VAL R@ ! A:CELL+
25908224 R@ ! A:CELL+ ThreadGroup.ThreadGroup§25908224 VAL R@ ! A:CELL+
606229504 R@ ! A:CELL+ ThreadGroup.ThreadGroup§606229504 VAL R@ ! A:CELL+
-483565312 R@ ! A:CELL+ ThreadGroup.activeCount§-483565312 VAL R@ ! A:CELL+
-818650880 R@ ! A:CELL+ ThreadGroup.activeGroupCount§-818650880 VAL R@ ! A:CELL+
-1191681792 R@ ! A:CELL+ ThreadGroup.add§-1191681792 VAL R@ ! A:CELL+
335503616 R@ ! A:CELL+ ThreadGroup.add§335503616 VAL R@ ! A:CELL+
-22060800 R@ ! A:CELL+ ThreadGroup.arraycopy§-22060800 VAL R@ ! A:CELL+
794779904 R@ ! A:CELL+ ThreadGroup.arraycopy§794779904 VAL R@ ! A:CELL+
354116352 R@ ! A:CELL+ ThreadGroup.checkAccess§354116352 VAL R@ ! A:CELL+
855073792 R@ ! A:CELL+ ThreadGroup.destroy§855073792 VAL R@ ! A:CELL+
-1064803072 R@ ! A:CELL+ ThreadGroup.enumerate§-1064803072 VAL R@ ! A:CELL+
-1458019072 R@ ! A:CELL+ ThreadGroup.enumerate§-1458019072 VAL R@ ! A:CELL+
-488282880 R@ ! A:CELL+ ThreadGroup.enumerate§-488282880 VAL R@ ! A:CELL+
-596417280 R@ ! A:CELL+ ThreadGroup.enumerate§-596417280 VAL R@ ! A:CELL+
189949184 R@ ! A:CELL+ ThreadGroup.enumerate§189949184 VAL R@ ! A:CELL+
511730944 R@ ! A:CELL+ ThreadGroup.enumerate§511730944 VAL R@ ! A:CELL+
836921088 R@ ! A:CELL+ ThreadGroup.getMaxPriority§836921088 VAL R@ ! A:CELL+
-1704827136 R@ ! A:CELL+ ThreadGroup.getName§-1704827136 VAL R@ ! A:CELL+
-274962688 R@ ! A:CELL+ ThreadGroup.getParent§-274962688 VAL R@ ! A:CELL+
384592128 R@ ! A:CELL+ ThreadGroup.interrupt§384592128 VAL R@ ! A:CELL+
-1531156224 R@ ! A:CELL+ ThreadGroup.isDaemon§-1531156224 VAL R@ ! A:CELL+
-1137940224 R@ ! A:CELL+ ThreadGroup.isDestroyed§-1137940224 VAL R@ ! A:CELL+
709848064 R@ ! A:CELL+ ThreadGroup.list§709848064 VAL R@ ! A:CELL+
666202112 R@ ! A:CELL+ ThreadGroup.parentOf§666202112 VAL R@ ! A:CELL+
-1031376384 R@ ! A:CELL+ ThreadGroup.remove§-1031376384 VAL R@ ! A:CELL+
453997056 R@ ! A:CELL+ ThreadGroup.remove§453997056 VAL R@ ! A:CELL+
599945728 R@ ! A:CELL+ ThreadGroup.resume§599945728 VAL R@ ! A:CELL+
-1235782912 R@ ! A:CELL+ ThreadGroup.setDaemon§-1235782912 VAL R@ ! A:CELL+
-1956941056 R@ ! A:CELL+ ThreadGroup.setMaxPriority§-1956941056 VAL R@ ! A:CELL+
-74747136 R@ ! A:CELL+ ThreadGroup.stop§-74747136 VAL R@ ! A:CELL+
546927360 R@ ! A:CELL+ ThreadGroup.stopOrSuspend§546927360 VAL R@ ! A:CELL+
-2031258880 R@ ! A:CELL+ ThreadGroup.suspend§-2031258880 VAL R@ ! A:CELL+
1621718016 R@ ! A:CELL+ ThreadGroup.toString§1621718016 VAL R@ ! A:CELL+
A:DROP
