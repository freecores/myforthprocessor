/*
 * @(#)GenerationName.java	1.4 03/01/23 11:40:55
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.memory;

import java.util.*;

import sun.jvm.hotspot.runtime.*;
import sun.jvm.hotspot.types.*;
import sun.jvm.hotspot.utilities.*;

/** Mimics the enums in the VM under Generation::Name */

public class GenerationName {
  private String name;

  private static Map enumToNameMap;

  static {
    VM.registerVMInitializedObserver(new Observer() {
        public void update(Observable o, Object data) {
          initialize(VM.getVM().getTypeDataBase());
        }
      });
  }

  private static synchronized void initialize(TypeDataBase db) {
    enumToNameMap = new HashMap();
    enumToNameMap.put(db.lookupIntConstant("Generation::DefNew"),           DEF_NEW);
    enumToNameMap.put(db.lookupIntConstant("Generation::MarkSweepCompact"), MARK_SWEEP_COMPACT);
    enumToNameMap.put(db.lookupIntConstant("Generation::TrainGen"),         TRAIN_GEN);
    enumToNameMap.put(db.lookupIntConstant("Generation::Other"),            OTHER);
  }

  private GenerationName(String name) { this.name = name; }

  public static final GenerationName DEF_NEW            = new GenerationName("DEF_NEW");
  public static final GenerationName MARK_SWEEP_COMPACT = new GenerationName("MARK_SWEEP_COMPACT");
  public static final GenerationName TRAIN_GEN          = new GenerationName("TRAIN_GEN");
  public static final GenerationName OTHER              = new GenerationName("OTHER");

  public String toString() {
    return name;
  }

  /** Package-private routine for mapping from the enums in the VM to
      these objects */
  static GenerationName nameForEnum(int enum) {
    GenerationName name = (GenerationName) enumToNameMap.get(new Integer(enum));
    if (Assert.ASSERTS_ENABLED) {
      Assert.that(name != null, "illegal generation name " + enum);
    }
    return name;
  }
}
