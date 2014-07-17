MODULENAME equation.StackTraceElement
(
* @(#)StackTraceElement.java	1.7 03/01/23
*
* Copyright 2003 Sun Microsystems, Inc. All rights reserved.
* SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
)  ( *
* An element in a stack trace, as returned by {@link
* Throwable#getStackTrace()}.  Each element represents a single stack frame.
* All stack frames except for the one at the top of the stack represent
* a method invocation.  The frame at the top of the stack represents the
* the execution point at which the stack trace was generated.  Typically,
* this is the point at which the throwable corresponding to the stack trace
* was created.
*
* @since  1.4
* @author Josh Bloch
)
( *
* Prevent inappropriate instantiation.  Only the VM creates these.
* It creates them "magically" without invoking this constructor.
)

:LOCAL StackTraceElement.StackTraceElement§2108117760
   1 VALLOCATE LOCAL §base0
   
   \ new statement
   0
   LOCALS §this |
   
   \ new statement
   Object§-1890496768.table 1827294976 EXECUTE-NEW
   
   \ new statement
   44 MALLOC DROP DUP DUP §base0 V! TO §this OVER
   IF
      OVER CELL+ @ 4 + OVER OVER ! DROP
   ENDIF
   CELL+ @ OVER
   OVER ! NIP
   StackTraceElement§-1278127360.table OVER 12 + !
   -1278148608 OVER 20 + !
   " StackTraceElement " OVER 16 + !
   3 OVER 24 + ! DROP
   
   \ new statement
   \  Initialized by VM
   
   
   \ new statement
   (  assert false;  )
   
   \ new statement
   §this DUP 0 V! ( return object )    
   \ new statement
   0 §break32343 LABEL
   
   \ new statement
   §base0 SETVTOP
   PURGE 2
   
   \ new statement
   DROP
;


: StackTraceElement.eq§1633510656
   2 VALLOCATE LOCAL §base0
   DUP 0 §base0 + V! LOCAL b
   DUP 4 §base0 + V! LOCAL a
   
   \ new statement
   
   \ new statement
   a
   b
   =  0=! or_32364 0BRANCH DROP
   a
   0<> and_32365 0BRANCH! DROP
   a >R
   b
   R> -240098048 TRUE ( equation.Object.equals§-240098048 ) EXECUTE-METHOD
   and_32365 LABEL
   or_32364 LABEL
   0 §break32351 BRANCH
   
   \ new statement
   0 §break32351 LABEL
   
   \ new statement
   
   
   §base0 SETVTOP
   PURGE 3
   
   \ new statement
   DROP
;
( *
* Returns true if the specified object is another
* <tt>StackTraceElement</tt> instance representing the same execution
* point as this instance.  Two stack trace elements <tt>a</tt> and
* <tt>b</tt> are equal if and only if:
* <pre>
*     equals(a.getFileName(), b.getFileName()) &&
*     a.getLineNumber() == b.getLineNumber()) &&
*     equals(a.getClassName(), b.getClassName()) &&
*     equals(a.getMethodName(), b.getMethodName())
* </pre>
* where <tt>equals</tt> is defined as:
* <pre>
*     static boolean equals(Object a, Object b) {
*         return a==b || (a != null && a.equals(b));
*     }
* </pre>
*
* @param  obj the object to be compared with this stack trace element.
* @return true if the specified object is another
*         <tt>StackTraceElement</tt> instance representing the same
*         execution point as this instance.
)

:LOCAL StackTraceElement.equals§-240098048
   3 VALLOCATE LOCAL §base0
   DUP 4 §base0 + V! LOCAL §this
   DUP 0 §base0 + V! LOCAL obj
   
   \ new statement
   0
   LOCALS e |
   
   \ new statement
   obj
   §this
   =
   
   \ new statement
   IF
      
      \ new statement
      TRUE
      0 §break32350 BRANCH
      
      \ new statement
   ENDIF
   
   \ new statement
   obj
   " StackTraceElement " INSTANCEOF
   0=
   
   \ new statement
   IF
      
      \ new statement
      FALSE
      0 §break32350 BRANCH
      
      \ new statement
   ENDIF
   
   \ new statement
   obj
   DUP 8 §base0 + V! TO e
   
   \ new statement
   e CELL+ @ 28 + ( equation.StackTraceElement.declaringClass ) @ >R
   §this CELL+ @ 28 + ( equation.StackTraceElement.declaringClass )    @
   R> -240098048 TRUE ( equation.String.equals§-240098048 ) EXECUTE-METHOD
   and_32380 0BRANCH! DROP
   e CELL+ @ 40 + ( equation.StackTraceElement.lineNumber )    @
   §this CELL+ @ 40 + ( equation.StackTraceElement.lineNumber )    @
   =
   and_32380 0BRANCH! DROP
   §this CELL+ @ 32 + ( equation.StackTraceElement.methodName )    @
   e CELL+ @ 32 + ( equation.StackTraceElement.methodName )    @
   StackTraceElement.eq§1633510656
   and_32380 0BRANCH! DROP
   §this CELL+ @ 36 + ( equation.StackTraceElement.fileName )    @
   e CELL+ @ 36 + ( equation.StackTraceElement.fileName )    @
   StackTraceElement.eq§1633510656
   and_32380 LABEL
   0 §break32350 BRANCH
   
   \ new statement
   0 §break32350 LABEL
   
   \ new statement
   
   
   §base0 SETVTOP
   PURGE 4
   
   \ new statement
   DROP
;
( *
* Returns the fully qualified name of the class containing the
* execution point represented by this stack trace element.
*
* @return the fully qualified name of the <tt>Class</tt> containing
*         the execution point represented by this stack trace element.
)

:LOCAL StackTraceElement.getClassName§1725720320
   1 VALLOCATE LOCAL §base0
   DUP 0 §base0 + V! LOCAL §this
   
   \ new statement
   
   \ new statement
   §this CELL+ @ 28 + ( equation.StackTraceElement.declaringClass )    @
   DUP 0 V!
   0 §break32346 BRANCH
   
   \ new statement
   0 §break32346 LABEL
   
   \ new statement
   §base0 SETVTOP
   PURGE 2
   
   \ new statement
   DROP
;
( *
* Returns the name of the source file containing the execution point
* represented by this stack trace element.  Generally, this corresponds
* to the <tt>SourceFile</tt> attribute of the relevant <tt>class</tt>
* file (as per <i>The Java Virtual Machine Specification</i>, Section
* 4.7.7).  In some systems, the name may refer to some source code unit
* other than a file, such as an entry in source repository.
*
* @return the name of the file containing the execution point
*         represented by this stack trace element, or <tt>null</tt> if
*         this information is unavailable.
)

:LOCAL StackTraceElement.getFileName§-879466752
   1 VALLOCATE LOCAL §base0
   DUP 0 §base0 + V! LOCAL §this
   
   \ new statement
   
   \ new statement
   §this CELL+ @ 36 + ( equation.StackTraceElement.fileName )    @
   DUP 0 V!
   0 §break32344 BRANCH
   
   \ new statement
   0 §break32344 LABEL
   
   \ new statement
   §base0 SETVTOP
   PURGE 2
   
   \ new statement
   DROP
;
( *
* Returns the line number of the source line containing the execution
* point represented by this stack trace element.  Generally, this is
* derived from the <tt>LineNumberTable</tt> attribute of the relevant
* <tt>class</tt> file (as per <i>The Java Virtual Machine
* Specification</i>, Section 4.7.8).
*
* @return the line number of the source line containing the execution
*         point represented by this stack trace element, or a negative
*         number if this information is unavailable.
)

:LOCAL StackTraceElement.getLineNumber§-1065851136
   1 VALLOCATE LOCAL §base0
   DUP 0 §base0 + V! LOCAL §this
   
   \ new statement
   
   \ new statement
   §this CELL+ @ 40 + ( equation.StackTraceElement.lineNumber )    @
   0 §break32345 BRANCH
   
   \ new statement
   0 §break32345 LABEL
   
   \ new statement
   §base0 SETVTOP
   PURGE 2
   
   \ new statement
   DROP
;
( *
* Returns the name of the method containing the execution point
* represented by this stack trace element.  If the execution point is
* contained in an instance or class initializer, this method will return
* the appropriate <i>special method name</i>, <tt>&lt;init&gt;</tt> or
* <tt>&lt;clinit&gt;</tt>, as per Section 3.9 of <i>The Java Virtual
* Machine Specification</i>.
*
* @return the name of the method containing the execution point
*         represented by this stack trace element.
)

:LOCAL StackTraceElement.getMethodName§910059264
   1 VALLOCATE LOCAL §base0
   DUP 0 §base0 + V! LOCAL §this
   
   \ new statement
   
   \ new statement
   §this CELL+ @ 32 + ( equation.StackTraceElement.methodName )    @
   DUP 0 V!
   0 §break32347 BRANCH
   
   \ new statement
   0 §break32347 LABEL
   
   \ new statement
   §base0 SETVTOP
   PURGE 2
   
   \ new statement
   DROP
;
( *
* Returns a hash code value for this stack trace element.
)

:LOCAL StackTraceElement.hashCode§-1604556800
   1 VALLOCATE LOCAL §base0
   DUP 0 §base0 + V! LOCAL §this
   
   \ new statement
   0
   LOCALS result |
   
   \ new statement
   §this CELL+ @ 28 + ( equation.StackTraceElement.declaringClass ) @ >R
   R> -1604556800 TRUE ( equation.String.hashCode§-1604556800 ) EXECUTE-METHOD
   31
   *
   §this CELL+ @ 32 + ( equation.StackTraceElement.methodName ) @ >R
   R> -1604556800 TRUE ( equation.String.hashCode§-1604556800 ) EXECUTE-METHOD
   +
   TO result
   
   \ new statement
   result
   31
   *
   §this CELL+ @ 36 + ( equation.StackTraceElement.fileName )    @
   0=
   IF
      0
      
   ELSE
      §this CELL+ @ 36 + ( equation.StackTraceElement.fileName ) @ >R
      R> -1604556800 TRUE ( equation.String.hashCode§-1604556800 ) EXECUTE-METHOD
      
   ENDIF
   +
   TO result
   
   \ new statement
   result
   31
   *
   §this CELL+ @ 40 + ( equation.StackTraceElement.lineNumber )    @
   +
   TO result
   
   \ new statement
   result
   0 §break32352 BRANCH
   
   \ new statement
   0 §break32352 LABEL
   
   \ new statement
   
   §base0 SETVTOP
   PURGE 3
   
   \ new statement
   DROP
;
( *
* Returns true if the method containing the execution point
* represented by this stack trace element is a native method.
*
* @return <tt>true</tt> if the method containing the execution point
*         represented by this stack trace element is a native method.
)

:LOCAL StackTraceElement.isNativeMethod§-2084869888
   1 VALLOCATE LOCAL §base0
   DUP 0 §base0 + V! LOCAL §this
   
   \ new statement
   
   \ new statement
   §this CELL+ @ 40 + ( equation.StackTraceElement.lineNumber )    @
   -2
   =
   0 §break32348 BRANCH
   
   \ new statement
   0 §break32348 LABEL
   
   \ new statement
   §base0 SETVTOP
   PURGE 2
   
   \ new statement
   DROP
;
( *
* Returns a string representation of this stack trace element.  The
* format of this string depends on the implementation, but the following
* examples may be regarded as typical:
* <ul>
* <li>
*   <tt>"MyClass.mash(MyClass.java:9)"</tt> - Here, <tt>"MyClass"</tt>
*   is the <i>fully-qualified name</i> of the class containing the
*   execution point represented by this stack trace element,
*   <tt>"mash"</tt> is the name of the method containing the execution
*   point, <tt>"MyClass.java"</tt> is the source file containing the
*   execution point, and <tt>"9"</tt> is the line number of the source
*   line containing the execution point.
* <li>
*   <tt>"MyClass.mash(MyClass.java)"</tt> - As above, but the line
*   number is unavailable.
* <li>
*   <tt>"MyClass.mash(Unknown Source)"</tt> - As above, but neither
*   the file name nor the line  number are available.
* <li>
*   <tt>"MyClass.mash(Native Method)"</tt> - As above, but neither
*   the file name nor the line  number are available, and the method
*   containing the execution point is known to be a native method.
* </ul>
* @see    Throwable#printStackTrace()
)

:LOCAL StackTraceElement.toString§1621718016
   1 VALLOCATE LOCAL §base0
   DUP 0 §base0 + V! LOCAL §this
   
   \ new statement
   
   \ new statement
   
   18 VALLOCATE LOCAL §tempvar
   §this >R
   R> 1725720320 TRUE ( equation.StackTraceElement.getClassName§1725720320 ) EXECUTE-METHOD DUP §tempvar V!
   U" . " COUNT JavaArray.createUnicode§-675323136 DUP §tempvar 4 + V!
   SWAP  -1461427456 TRUE ( equation.String.concat§-1461427456 ) EXECUTE-METHOD DUP §tempvar 8 + V!
   §this CELL+ @ 32 + ( equation.StackTraceElement.methodName )    @
   SWAP  -1461427456 TRUE ( equation.String.concat§-1461427456 ) EXECUTE-METHOD DUP §tempvar 12 + V!
   §this >R
   R> -2084869888 TRUE ( equation.StackTraceElement.isNativeMethod§-2084869888 ) EXECUTE-METHOD
   
   IF
      U" (Native Method) " COUNT JavaArray.createUnicode§-675323136 DUP §tempvar 16 + V!
      
   ELSE
      §this CELL+ @ 36 + ( equation.StackTraceElement.fileName )       @
      0<> and_32419 0BRANCH! DROP
      §this CELL+ @ 40 + ( equation.StackTraceElement.lineNumber )       @
      0< INVERT
      and_32419 LABEL
      IF
         U" ( " COUNT JavaArray.createUnicode§-675323136 DUP §tempvar 20 + V!
         §this CELL+ @ 36 + ( equation.StackTraceElement.fileName )          @
         SWAP  -1461427456 TRUE ( equation.String.concat§-1461427456 ) EXECUTE-METHOD DUP §tempvar 24 + V!
         U" : " COUNT JavaArray.createUnicode§-675323136 DUP §tempvar 28 + V!
         SWAP  -1461427456 TRUE ( equation.String.concat§-1461427456 ) EXECUTE-METHOD DUP §tempvar 32 + V!
         §this CELL+ @ 40 + ( equation.StackTraceElement.lineNumber )          @  <# 0 ..R JavaArray.createString§-105880832 DUP §tempvar 36 + V!
         SWAP  -1461427456 TRUE ( equation.String.concat§-1461427456 ) EXECUTE-METHOD DUP §tempvar 40 + V!
         U" ) " COUNT JavaArray.createUnicode§-675323136 DUP §tempvar 44 + V!
         SWAP  -1461427456 TRUE ( equation.String.concat§-1461427456 ) EXECUTE-METHOD DUP §tempvar 48 + V!
      ELSE
         §this CELL+ @ 36 + ( equation.StackTraceElement.fileName )          @
         0<>
         IF
            U" ( " COUNT JavaArray.createUnicode§-675323136 DUP §tempvar 52 + V!
            §this CELL+ @ 36 + ( equation.StackTraceElement.fileName )             @
            SWAP  -1461427456 TRUE ( equation.String.concat§-1461427456 ) EXECUTE-METHOD DUP §tempvar 56 + V!
            U" ) " COUNT JavaArray.createUnicode§-675323136 DUP §tempvar 60 + V!
            SWAP  -1461427456 TRUE ( equation.String.concat§-1461427456 ) EXECUTE-METHOD DUP §tempvar 64 + V!
         ELSE
            U" (Unknown Source) " COUNT JavaArray.createUnicode§-675323136 DUP §tempvar 68 + V!
            
         ENDIF
         
      ENDIF
      
   ENDIF
   SWAP  -1461427456 TRUE ( equation.String.concat§-1461427456 ) EXECUTE-METHOD
   DUP 0 V!
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   §tempvar SETVTOP
   
   PURGE 1
   
   0 §break32349 BRANCH
   
   \ new statement
   0 §break32349 LABEL
   
   \ new statement
   §base0 SETVTOP
   PURGE 2
   
   \ new statement
   DROP
;
VARIABLE StackTraceElement._staticBlocking
VARIABLE StackTraceElement._staticThread  HERE 4 - SALLOCATE
VARIABLE StackTraceElement._this  HERE 4 - SALLOCATE
2VARIABLE StackTraceElement.serialVersionUID

A:HERE VARIABLE StackTraceElement§-1278127360.table 9 DUP 2* CELLS ALLOT R@ ! A:CELL+
2108117760 R@ ! A:CELL+ StackTraceElement.StackTraceElement§2108117760 VAL R@ ! A:CELL+
-240098048 R@ ! A:CELL+ StackTraceElement.equals§-240098048 VAL R@ ! A:CELL+
1725720320 R@ ! A:CELL+ StackTraceElement.getClassName§1725720320 VAL R@ ! A:CELL+
-879466752 R@ ! A:CELL+ StackTraceElement.getFileName§-879466752 VAL R@ ! A:CELL+
-1065851136 R@ ! A:CELL+ StackTraceElement.getLineNumber§-1065851136 VAL R@ ! A:CELL+
910059264 R@ ! A:CELL+ StackTraceElement.getMethodName§910059264 VAL R@ ! A:CELL+
-1604556800 R@ ! A:CELL+ StackTraceElement.hashCode§-1604556800 VAL R@ ! A:CELL+
-2084869888 R@ ! A:CELL+ StackTraceElement.isNativeMethod§-2084869888 VAL R@ ! A:CELL+
1621718016 R@ ! A:CELL+ StackTraceElement.toString§1621718016 VAL R@ ! A:CELL+
A:DROP
