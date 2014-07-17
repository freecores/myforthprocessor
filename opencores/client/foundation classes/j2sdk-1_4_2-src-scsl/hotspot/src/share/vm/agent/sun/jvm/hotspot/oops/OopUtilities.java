/*
 * @(#)OopUtilities.java	1.9 03/01/23 11:43:53
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.oops;

import java.util.*;

import sun.jvm.hotspot.debugger.*;
import sun.jvm.hotspot.runtime.*;
import sun.jvm.hotspot.types.TypeDataBase;
import sun.jvm.hotspot.utilities.*;

/** A utility class encapsulating useful oop operations */

public class OopUtilities {

  // FIXME: access should be synchronized and cleared when VM is
  // resumed
  // String fields
  private static IntField offsetField;
  private static IntField countField;
  private static OopField valueField;
  // ThreadGroup fields
  private static OopField threadGroupParentField;
  private static OopField threadGroupNameField;
  private static IntField threadGroupNThreadsField;
  private static OopField threadGroupThreadsField;
  private static IntField threadGroupNGroupsField;
  private static OopField threadGroupGroupsField;
  // Thread fields
  private static OopField threadNameField;
  private static OopField threadGroupField;
  private static LongField threadEETopField;
  
  static {
    VM.registerVMInitializedObserver(new Observer() {
        public void update(Observable o, Object data) {
          initialize(VM.getVM().getTypeDataBase());
        }
      });
  }

  private static synchronized void initialize(TypeDataBase db) {
    // FIXME: don't need this observer; however, do need a VM resumed
    // and suspended observer to refetch fields
  }

  public static String charArrayToString(TypeArray charArray) {
    if (charArray == null) {
      return null;
    }
    return charArrayToString(charArray, 0, (int) charArray.getLength());
  }

  public static String charArrayToString(TypeArray charArray, int offset, int length) {
    if (charArray == null) {
      return null;
    }
    StringBuffer buf = new StringBuffer(length);
    for (int i = offset; i < length; i++) {
      buf.append(charArray.getCharAt(i));
    }
    return buf.toString();
  }
  
  public static String stringOopToString(Oop stringOop) {
    if (offsetField == null) {
      InstanceKlass k = (InstanceKlass) stringOop.getKlass();
      offsetField = (IntField) k.findField("offset", "I");
      countField  = (IntField) k.findField("count",  "I");
      valueField  = (OopField) k.findField("value",  "[C");
      if (Assert.ASSERTS_ENABLED) {
        Assert.that(offsetField != null &&
                    countField != null &&
                    valueField != null, "must find all java.lang.String fields");
      }
    }
    return charArrayToString((TypeArray) valueField.getValue(stringOop),
                             offsetField.getValue(stringOop),
                             countField.getValue(stringOop));
  }

  private static void initThreadGroupFields(Oop threadGroupOop) {
    if (threadGroupParentField == null) {
      InstanceKlass k = (InstanceKlass) threadGroupOop.getKlass();
      threadGroupParentField   = (OopField) k.findField("parent",   "Ljava/lang/ThreadGroup;");
      threadGroupNameField     = (OopField) k.findField("name",     "Ljava/lang/String;");
      threadGroupNThreadsField = (IntField) k.findField("nthreads", "I");
      threadGroupThreadsField  = (OopField) k.findField("threads",  "[Ljava/lang/Thread;");
      threadGroupNGroupsField  = (IntField) k.findField("ngroups",  "I");
      threadGroupGroupsField   = (OopField) k.findField("groups",   "[Ljava/lang/ThreadGroup;");
      if (Assert.ASSERTS_ENABLED) {
        Assert.that(threadGroupParentField   != null &&
                    threadGroupNameField     != null &&
                    threadGroupNThreadsField != null &&
                    threadGroupThreadsField  != null &&
                    threadGroupNGroupsField  != null &&
                    threadGroupGroupsField   != null, "must find all java.lang.ThreadGroup fields");
      }
    }
  }

  public static Oop threadGroupOopGetParent(Oop threadGroupOop) {
    initThreadGroupFields(threadGroupOop);
    return threadGroupParentField.getValue(threadGroupOop);
  }

  public static String threadGroupOopGetName(Oop threadGroupOop) {
    initThreadGroupFields(threadGroupOop);
    return stringOopToString(threadGroupNameField.getValue(threadGroupOop));
  }

  public static Oop[] threadGroupOopGetThreads(Oop threadGroupOop) {
    initThreadGroupFields(threadGroupOop);
    int nthreads = threadGroupNThreadsField.getValue(threadGroupOop);
    Oop[] result = new Oop[nthreads];
    ObjArray threads = (ObjArray) threadGroupThreadsField.getValue(threadGroupOop);
    for (int i = 0; i < nthreads; i++) {
      result[i] = threads.getObjAt(i);
    }
    return result;
  }

  public static Oop[] threadGroupOopGetGroups(Oop threadGroupOop) {
    initThreadGroupFields(threadGroupOop);
    int ngroups = threadGroupNGroupsField.getValue(threadGroupOop);
    Oop[] result = new Oop[ngroups];
    ObjArray groups = (ObjArray) threadGroupGroupsField.getValue(threadGroupOop);
    for (int i = 0; i < ngroups; i++) {
      result[i] = groups.getObjAt(i);
    }
    return result;
  }

  private static void initThreadFields(Oop threadOop) {
    if (threadNameField == null) {
      InstanceKlass k = (InstanceKlass) threadOop.getKlass();
      threadNameField  = (OopField) k.findField("name", "[C");
      threadGroupField = (OopField) k.findField("group", "Ljava/lang/ThreadGroup;");
      threadEETopField = (LongField) k.findField("eetop", "J");
      if (Assert.ASSERTS_ENABLED) {
        Assert.that(threadNameField   != null &&
                    threadGroupField  != null &&
                    threadEETopField  != null, "must find all java.lang.Thread fields");
      }
    }
  }

  public static Oop threadOopGetThreadGroup(Oop threadOop) {
    initThreadFields(threadOop);
    return threadGroupField.getValue(threadOop);
  }

  public static String threadOopGetName(Oop threadOop) {
    initThreadFields(threadOop);
    return charArrayToString((TypeArray) threadNameField.getValue(threadOop));
  }

  /** May return null if, e.g., thread was not started */
  public static JavaThread threadOopGetJavaThread(Oop threadOop) {
    initThreadFields(threadOop);
    Address addr = threadOop.getHandle().getAddressAt(threadEETopField.getOffset());
    if (addr == null) {
      return null;
    }
    return VM.getVM().getThreads().createJavaThreadWrapper(addr);
  }
}
