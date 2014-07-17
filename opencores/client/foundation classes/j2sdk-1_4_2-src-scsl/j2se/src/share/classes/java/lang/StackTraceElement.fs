MODULENAME java.lang.StackTraceElement
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

:LOCAL lang.StackTraceElement.StackTraceElement§2108117760
   1 VALLOCATE LOCAL §base0
   
   \ new statement
   0
   LOCALS §this |
   
   \ new statement
   lang.Object§-1890496768.table 1827294976 EXECUTE-NEW
   
   \ new statement
   44 MALLOC DROP DUP DUP §base0 V! TO §this OVER
   IF
      OVER CELL+ @ 4 + OVER OVER ! DROP
   ENDIF
   CELL+ @ OVER
   OVER ! NIP
   lang.StackTraceElement§-1278127360.table OVER 12 + !
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
   0 §break21945 LABEL
   
   \ new statement
   §base0 SETVTOP
   PURGE 2
   
   \ new statement
   DROP
;


: lang.StackTraceElement.classMonitorEnter§-433167616
   
   \ new statement
   
   \ new statement
   0 §break21943 LABEL
   
   \ new statement
   
   \ new statement
   DROP
;


: lang.StackTraceElement.classMonitorLeave§-1691393280
   
   \ new statement
   
   \ new statement
   0 §break21944 LABEL
   
   \ new statement
   
   \ new statement
   DROP
;


: lang.StackTraceElement.eq§1633510656
   2 VALLOCATE LOCAL §base0
   DUP 0 §base0 + V! LOCAL b
   DUP 4 §base0 + V! LOCAL a
   
   \ new statement
   
   \ new statement
   a
   b
   =  0=! or_21974 0BRANCH DROP
   a
   0<> and_21975 0BRANCH! DROP
   a A:R@
   b
   R> -240098048 TRUE ( java.lang.Object.equals§-240098048 ) EXECUTE-METHOD
   and_21975 LABEL
   or_21974 LABEL
   0 §break21953 BRANCH
   
   \ new statement
   0 §break21953 LABEL
   
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

:LOCAL lang.StackTraceElement.equals§-240098048
   3 VALLOCATE LOCAL §base0
   DUP 0 §base0 + V! LOCAL obj
   DUP 4 §base0 + V! LOCAL §this
   
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
      0 §break21952 BRANCH
      
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
      0 §break21952 BRANCH
      
      \ new statement
   ENDIF
   
   \ new statement
   obj
   DUP 8 §base0 + V! TO e
   
   \ new statement
   e CELL+ @ 28 + ( java.lang.StackTraceElement.declaringClass ) @ A:R@
   §this CELL+ @ 28 + ( java.lang.StackTraceElement.declaringClass )    @
   R> -240098048 TRUE ( java.lang.String.equals§-240098048 ) EXECUTE-METHOD
   and_21990 0BRANCH! DROP
   e CELL+ @ 40 + ( java.lang.StackTraceElement.lineNumber )    @
   §this CELL+ @ 40 + ( java.lang.StackTraceElement.lineNumber )    @
   =
   and_21990 0BRANCH! DROP
   §this CELL+ @ 32 + ( java.lang.StackTraceElement.methodName )    @
   e CELL+ @ 32 + ( java.lang.StackTraceElement.methodName )    @
   lang.StackTraceElement.eq§1633510656
   and_21990 0BRANCH! DROP
   §this CELL+ @ 36 + ( java.lang.StackTraceElement.fileName )    @
   e CELL+ @ 36 + ( java.lang.StackTraceElement.fileName )    @
   lang.StackTraceElement.eq§1633510656
   and_21990 LABEL
   0 §break21952 BRANCH
   
   \ new statement
   0 §break21952 LABEL
   
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

:LOCAL lang.StackTraceElement.getClassName§1725720320
   1 VALLOCATE LOCAL §base0
   DUP 0 §base0 + V! LOCAL §this
   
   \ new statement
   
   \ new statement
   §this CELL+ @ 28 + ( java.lang.StackTraceElement.declaringClass )    @
   DUP 0 V!0 §break21948 BRANCH
   
   \ new statement
   0 §break21948 LABEL
   
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

:LOCAL lang.StackTraceElement.getFileName§-879466752
   1 VALLOCATE LOCAL §base0
   DUP 0 §base0 + V! LOCAL §this
   
   \ new statement
   
   \ new statement
   §this CELL+ @ 36 + ( java.lang.StackTraceElement.fileName )    @
   DUP 0 V!0 §break21946 BRANCH
   
   \ new statement
   0 §break21946 LABEL
   
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

:LOCAL lang.StackTraceElement.getLineNumber§-1065851136
   1 VALLOCATE LOCAL §base0
   DUP 0 §base0 + V! LOCAL §this
   
   \ new statement
   
   \ new statement
   §this CELL+ @ 40 + ( java.lang.StackTraceElement.lineNumber )    @
   0 §break21947 BRANCH
   
   \ new statement
   0 §break21947 LABEL
   
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

:LOCAL lang.StackTraceElement.getMethodName§910059264
   1 VALLOCATE LOCAL §base0
   DUP 0 §base0 + V! LOCAL §this
   
   \ new statement
   
   \ new statement
   §this CELL+ @ 32 + ( java.lang.StackTraceElement.methodName )    @
   DUP 0 V!0 §break21949 BRANCH
   
   \ new statement
   0 §break21949 LABEL
   
   \ new statement
   §base0 SETVTOP
   PURGE 2
   
   \ new statement
   DROP
;
( *
* Returns a hash code value for this stack trace element.
)

:LOCAL lang.StackTraceElement.hashCode§-1604556800
   1 VALLOCATE LOCAL §base0
   DUP 0 §base0 + V! LOCAL §this
   
   \ new statement
   0
   LOCALS result |
   
   \ new statement
   §this CELL+ @ 28 + ( java.lang.StackTraceElement.declaringClass ) @ A:R@
   R> -1604556800 TRUE ( java.lang.String.hashCode§-1604556800 ) EXECUTE-METHOD
   31
   *
   §this CELL+ @ 32 + ( java.lang.StackTraceElement.methodName ) @ A:R@
   R> -1604556800 TRUE ( java.lang.String.hashCode§-1604556800 ) EXECUTE-METHOD
   +
   TO result
   
   \ new statement
   result
   31
   *
   §this CELL+ @ 36 + ( java.lang.StackTraceElement.fileName )    @
   0=
   IF
      0
      
   ELSE
      §this CELL+ @ 36 + ( java.lang.StackTraceElement.fileName ) @ A:R@
      R> -1604556800 TRUE ( java.lang.String.hashCode§-1604556800 ) EXECUTE-METHOD
      
   ENDIF
   +
   TO result
   
   \ new statement
   result
   31
   *
   §this CELL+ @ 40 + ( java.lang.StackTraceElement.lineNumber )    @
   +
   TO result
   
   \ new statement
   result
   0 §break21954 BRANCH
   
   \ new statement
   0 §break21954 LABEL
   
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

:LOCAL lang.StackTraceElement.isNativeMethod§-2084869888
   1 VALLOCATE LOCAL §base0
   DUP 0 §base0 + V! LOCAL §this
   
   \ new statement
   
   \ new statement
   §this CELL+ @ 40 + ( java.lang.StackTraceElement.lineNumber )    @
   -2
   =
   0 §break21950 BRANCH
   
   \ new statement
   0 §break21950 LABEL
   
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

:LOCAL lang.StackTraceElement.toString§1621718016
   1 VALLOCATE LOCAL §base0
   DUP 0 §base0 + V! LOCAL §this
   
   \ new statement
   
   \ new statement
   
   18 VALLOCATE LOCAL §tempvar
   §this A:R@
   R> 1725720320 TRUE ( java.lang.StackTraceElement.getClassName§1725720320 ) EXECUTE-METHOD DUP §tempvar V!
   U" . " COUNT lang.JavaArray.createUnicode§-675323136 DUP §tempvar 4 + V!
   OVER  -1461427456 TRUE ( java.lang.String.concat§-1461427456 ) EXECUTE-METHOD DUP §tempvar 8 + V!
   §this CELL+ @ 32 + ( java.lang.StackTraceElement.methodName )    @
   OVER  -1461427456 TRUE ( java.lang.String.concat§-1461427456 ) EXECUTE-METHOD DUP §tempvar 12 + V!
   §this A:R@
   R> -2084869888 TRUE ( java.lang.StackTraceElement.isNativeMethod§-2084869888 ) EXECUTE-METHOD
   
   IF
      U" (Native Method) " COUNT lang.JavaArray.createUnicode§-675323136 DUP §tempvar 16 + V!
      
   ELSE
      §this CELL+ @ 36 + ( java.lang.StackTraceElement.fileName )       @
      0<> and_22029 0BRANCH! DROP
      §this CELL+ @ 40 + ( java.lang.StackTraceElement.lineNumber )       @
      0< INVERT
      and_22029 LABEL
      IF
         U" ( " COUNT lang.JavaArray.createUnicode§-675323136 DUP §tempvar 20 + V!
         §this CELL+ @ 36 + ( java.lang.StackTraceElement.fileName )          @
         OVER  -1461427456 TRUE ( java.lang.String.concat§-1461427456 ) EXECUTE-METHOD DUP §tempvar 24 + V!
         U" : " COUNT lang.JavaArray.createUnicode§-675323136 DUP §tempvar 28 + V!
         OVER  -1461427456 TRUE ( java.lang.String.concat§-1461427456 ) EXECUTE-METHOD DUP §tempvar 32 + V!
         §this CELL+ @ 40 + ( java.lang.StackTraceElement.lineNumber )          @  <# 0 ..R lang.JavaArray.createString§-105880832 DUP §tempvar 36 + V!
         OVER  -1461427456 TRUE ( java.lang.String.concat§-1461427456 ) EXECUTE-METHOD DUP §tempvar 40 + V!
         U" ) " COUNT lang.JavaArray.createUnicode§-675323136 DUP §tempvar 44 + V!
         OVER  -1461427456 TRUE ( java.lang.String.concat§-1461427456 ) EXECUTE-METHOD DUP §tempvar 48 + V!
      ELSE
         §this CELL+ @ 36 + ( java.lang.StackTraceElement.fileName )          @
         0<>
         IF
            U" ( " COUNT lang.JavaArray.createUnicode§-675323136 DUP §tempvar 52 + V!
            §this CELL+ @ 36 + ( java.lang.StackTraceElement.fileName )             @
            OVER  -1461427456 TRUE ( java.lang.String.concat§-1461427456 ) EXECUTE-METHOD DUP §tempvar 56 + V!
            U" ) " COUNT lang.JavaArray.createUnicode§-675323136 DUP §tempvar 60 + V!
            OVER  -1461427456 TRUE ( java.lang.String.concat§-1461427456 ) EXECUTE-METHOD DUP §tempvar 64 + V!
         ELSE
            U" (Unknown Source) " COUNT lang.JavaArray.createUnicode§-675323136 DUP §tempvar 68 + V!
            
         ENDIF
         
      ENDIF
      
   ENDIF
   OVER  -1461427456 TRUE ( java.lang.String.concat§-1461427456 ) EXECUTE-METHOD
   DUP 0 V!0 §break21951 BRANCH
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   §tempvar SETVTOP
   
   PURGE 1
   
   
   \ new statement
   0 §break21951 LABEL
   
   \ new statement
   §base0 SETVTOP
   PURGE 2
   
   \ new statement
   DROP
;
VARIABLE lang.StackTraceElement._staticBlocking
VARIABLE lang.StackTraceElement._staticThread
-1 VALLOCATE CONSTANT lang.StackTraceElement._this
2VARIABLE lang.StackTraceElement.serialVersionUID

A:HERE VARIABLE lang.StackTraceElement§-1278127360.table 9 DUP 2* CELLS ALLOT R@ ! A:CELL+
2108117760 R@ ! A:CELL+ lang.StackTraceElement.StackTraceElement§2108117760 VAL R@ ! A:CELL+
-240098048 R@ ! A:CELL+ lang.StackTraceElement.equals§-240098048 VAL R@ ! A:CELL+
1725720320 R@ ! A:CELL+ lang.StackTraceElement.getClassName§1725720320 VAL R@ ! A:CELL+
-879466752 R@ ! A:CELL+ lang.StackTraceElement.getFileName§-879466752 VAL R@ ! A:CELL+
-1065851136 R@ ! A:CELL+ lang.StackTraceElement.getLineNumber§-1065851136 VAL R@ ! A:CELL+
910059264 R@ ! A:CELL+ lang.StackTraceElement.getMethodName§910059264 VAL R@ ! A:CELL+
-1604556800 R@ ! A:CELL+ lang.StackTraceElement.hashCode§-1604556800 VAL R@ ! A:CELL+
-2084869888 R@ ! A:CELL+ lang.StackTraceElement.isNativeMethod§-2084869888 VAL R@ ! A:CELL+
1621718016 R@ ! A:CELL+ lang.StackTraceElement.toString§1621718016 VAL R@ ! A:CELL+
A:DROP
