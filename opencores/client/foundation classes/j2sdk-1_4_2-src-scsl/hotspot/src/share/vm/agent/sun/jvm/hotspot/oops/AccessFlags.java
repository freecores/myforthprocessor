/*
 * @(#)AccessFlags.java	1.9 03/01/23 11:41:38
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.oops;

import sun.jvm.hotspot.runtime.ClassConstants;
import java.io.*;

public class AccessFlags implements /* imports */ ClassConstants {
  public AccessFlags(long flags) {
    this.flags = flags;
  }

  private long flags;

  // Java access flags
  public boolean isPublic      () { return (flags & JVM_ACC_PUBLIC      ) != 0; }
  public boolean isPrivate     () { return (flags & JVM_ACC_PRIVATE     ) != 0; }
  public boolean isProtected   () { return (flags & JVM_ACC_PROTECTED   ) != 0; }
  public boolean isStatic      () { return (flags & JVM_ACC_STATIC      ) != 0; }
  public boolean isFinal       () { return (flags & JVM_ACC_FINAL       ) != 0; }
  public boolean isSynchronized() { return (flags & JVM_ACC_SYNCHRONIZED) != 0; }
  public boolean isSuper       () { return (flags & JVM_ACC_SUPER       ) != 0; }
  public boolean isVolatile    () { return (flags & JVM_ACC_VOLATILE    ) != 0; }
  public boolean isTransient   () { return (flags & JVM_ACC_TRANSIENT   ) != 0; }
  public boolean isNative      () { return (flags & JVM_ACC_NATIVE      ) != 0; }
  public boolean isInterface   () { return (flags & JVM_ACC_INTERFACE   ) != 0; }
  public boolean isAbstract    () { return (flags & JVM_ACC_ABSTRACT    ) != 0; }
  public boolean isStrict      () { return (flags & JVM_ACC_STRICT      ) != 0; }
  public long getValue         () { return flags; }

  // Attribute flags
  public boolean isSynthetic   () { return (flags & JVM_ACC_SYNTHETIC   ) != 0; }

  // methodOop flags
  public boolean isMonitorMatching   () { return (flags & JVM_ACC_MONITOR_MATCH          ) != 0; }
  public boolean hasMonitorBytecodes () { return (flags & JVM_ACC_HAS_MONITOR_BYTECODES  ) != 0; }
  public boolean hasLoops            () { return (flags & JVM_ACC_HAS_LOOPS              ) != 0; }  
  public boolean loopsFlagInit       () { return (flags & JVM_ACC_LOOPS_FLAG_INIT        ) != 0; }  
  public boolean queuedForCompilation() { return (flags & JVM_ACC_QUEUED                 ) != 0; }  
  public boolean hasOtherVtableIndex1() { return (flags & JVM_ACC_HAS_OTHER_VTABLE_INDEX1) != 0; }
  public boolean hasOtherVtableIndex2() { return (flags & JVM_ACC_HAS_OTHER_VTABLE_INDEX2) != 0; }
  public boolean isNotOsrCompilable  () { return (flags & JVM_ACC_NOT_OSR_COMPILABLE     ) != 0; }
  public boolean hasLineNumberTable  () { return (flags & JVM_ACC_HAS_LINE_NUMBER_TABLE  ) != 0; }
  public boolean hasCheckedExceptions() { return (flags & JVM_ACC_HAS_CHECKED_EXCEPTIONS ) != 0; }

  // klassOop flags
  public boolean hasMirandaMethods    () { return (flags & JVM_ACC_HAS_MIRANDA_METHODS    ) != 0; }
  public boolean hasVanillaConstructor() { return (flags & JVM_ACC_HAS_VANILLA_CONSTRUCTOR) != 0; }
  public boolean hasFinalizer         () { return (flags & JVM_ACC_HAS_FINALIZER          ) != 0; }
  public boolean isCloneable          () { return (flags & JVM_ACC_IS_CLONEABLE           ) != 0; }

  // klassOop and methodOop flags
  public boolean hasLocalVariableTable() { return (flags & JVM_ACC_HAS_LOCAL_VARIABLE_TABLE ) != 0; }

  public void printOn(PrintStream tty) {
    if (isPublic      ()) tty.print("public "      );
    if (isPrivate     ()) tty.print("private "     );
    if (isProtected   ()) tty.print("protected "   );
    if (isStatic      ()) tty.print("static "      );
    if (isFinal       ()) tty.print("final "       );
    if (isSynchronized()) tty.print("synchronized ");
    if (isVolatile    ()) tty.print("volatile "    );
    if (isTransient   ()) tty.print("transient "   );
    if (isNative      ()) tty.print("native "      );	
    if (isInterface   ()) tty.print("interface "   );
    if (isAbstract    ()) tty.print("abstract "    );
    if (isStrict      ()) tty.print("strict "      );
    if (isSynthetic   ()) tty.print("synthetic "   );
  }
}
