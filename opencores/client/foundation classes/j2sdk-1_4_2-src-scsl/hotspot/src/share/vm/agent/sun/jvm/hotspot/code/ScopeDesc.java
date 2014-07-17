/*
 * @(#)ScopeDesc.java	1.6 03/01/23 11:24:28
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.code;

import java.io.*;
import java.util.*;

import sun.jvm.hotspot.oops.*;
import sun.jvm.hotspot.runtime.*;

/** ScopeDescs contain the information that makes source-level
    debugging of nmethods possible; each scopeDesc describes a method
    activation */

public class ScopeDesc {
  /** NMethod information */
  private NMethod code;
  private Method  method;
  private int     bci;
  /** Decoding offsets */
  private int     decodeOffset;
  private int     senderDecodeOffset;
  private int     localsDecodeOffset;
  private int     expressionsDecodeOffset;
  private int     monitorsDecodeOffset;

  public ScopeDesc(NMethod code, int decodeOffset) {
    this.code = code;
    this.decodeOffset = decodeOffset;

    // Decode header
    DebugInfoReadStream stream  = streamAt(decodeOffset);

    senderDecodeOffset = stream.readInt();
    method = (Method) VM.getVM().getObjectHeap().newOop(stream.readOopHandle());
    bci    = stream.readInt();
    // Decode offsets for body and sender
    localsDecodeOffset      = stream.readInt();
    expressionsDecodeOffset = stream.readInt();
    monitorsDecodeOffset    = stream.readInt();
  }

  public NMethod getNMethod() { return code; }
  public Method getMethod() { return method; }
  public int    getBCI()    { return bci;    }

  /** Returns a List&lt;ScopeValue&gt; */
  public List getLocals() {
    return decodeScopeValues(localsDecodeOffset);
  }

  /** Returns a List&lt;ScopeValue&gt; */
  public List getExpressions() {
    return decodeScopeValues(expressionsDecodeOffset);
  }

  /** Returns a List&lt;MonitorValue&gt; */
  public List getMonitors() {
    return decodeMonitorValues(monitorsDecodeOffset);
  }

  /** Stack walking. Returns null if this is the outermost scope. */
  public ScopeDesc sender() {
    if (isTop()) {
      return null;
    }
    
    return new ScopeDesc(code, senderDecodeOffset);
  }

  /** Returns where the scope was decoded */
  public int getDecodeOffset() {
    return decodeOffset;
  }

  /** Tells whether sender() returns null */
  public boolean isTop() {
    return (senderDecodeOffset == DebugInformationRecorder.SERIALIZED_NULL);
  }

  public boolean equals(Object arg) {
    if (arg == null) {
      return false;
    }

    if (!(arg instanceof ScopeDesc)) {
      return false;
    }
    
    ScopeDesc sd = (ScopeDesc) arg;

    return (sd.method.equals(method) && (sd.bci == bci));
  }

  public void printValue() {
    printValueOn(System.out);
  }

  public void printValueOn(PrintStream tty) {
    tty.print("ScopeDesc for ");
    method.printValueOn(tty);
    tty.println(" @bci " + bci);
  }
  
  // FIXME: add more accessors

  //--------------------------------------------------------------------------------
  // Internals only below this point
  //

  private DebugInfoReadStream streamAt(int decodeOffset) {
    return new DebugInfoReadStream(code, decodeOffset);
  }

  /** Returns a List&lt;ScopeValue&gt; or null if no values were present */
  private List decodeScopeValues(int decodeOffset) {
    if (decodeOffset == DebugInformationRecorder.SERIALIZED_NULL) {
      return null;
    }
    DebugInfoReadStream stream = streamAt(decodeOffset);
    int length = stream.readInt();
    List res = new ArrayList(length);
    for (int i = 0; i < length; i++) {
      res.add(ScopeValue.readFrom(stream));
    }
    return res;
  }
  
  /** Returns a List&lt;MonitorValue&gt; or null if no values were present */
  private List decodeMonitorValues(int decodeOffset) {
    if (decodeOffset == DebugInformationRecorder.SERIALIZED_NULL) {
      return null;
    }
    DebugInfoReadStream stream = streamAt(decodeOffset);
    int length = stream.readInt();
    List res = new ArrayList(length);
    for (int i = 0; i < length; i++) {
      res.add(new MonitorValue(stream));
    }
    return res;
  }
}