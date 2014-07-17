/*
 * @(#)Runtime1.java	1.4 03/01/23 11:23:24
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.c1;

import java.util.*;
import sun.jvm.hotspot.code.*;
import sun.jvm.hotspot.debugger.*;
import sun.jvm.hotspot.runtime.*;
import sun.jvm.hotspot.types.*;

/** Currently a minimal port to get C1 frame traversal working */

public class Runtime1 {
  private static Field         blobsField;
  private static int           osrFrameReturnID;

  static {
    VM.registerVMInitializedObserver(new Observer() {
        public void update(Observable o, Object data) {
          initialize(VM.getVM().getTypeDataBase());
        }
      });
  }

  private static synchronized void initialize(TypeDataBase db) {
    Type type = db.lookupType("Runtime1");
    
    blobsField = type.getField("_blobs");

    osrFrameReturnID = db.lookupIntConstant("Runtime1::osr_frame_return_id").intValue();
  }

  public Runtime1() {
  }

  public boolean returnsToOSRAdapter(Address pc) {
    return (entryFor(osrFrameReturnID).equals(pc));
  }

  /** FIXME: consider making argument "type-safe" in Java port */
  public Address entryFor(int id) {
    return blobFor(id).instructionsBegin();
  }

  /** FIXME: consider making argument "type-safe" in Java port */
  public CodeBlob blobFor(int id) {
    Address blobAddr = blobsField.getStaticFieldAddress().getAddressAt(id * VM.getVM().getAddressSize());
    return VM.getVM().getCodeCache().createCodeBlobWrapper(blobAddr);
  }
}
