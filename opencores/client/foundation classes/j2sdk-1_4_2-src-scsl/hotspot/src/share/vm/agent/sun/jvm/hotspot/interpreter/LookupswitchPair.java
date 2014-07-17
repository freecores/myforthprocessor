/*
 * @(#)LookupswitchPair.java	1.3 03/01/23 11:37:39
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.interpreter;

import sun.jvm.hotspot.oops.*;

public class LookupswitchPair extends Bytecode {
  LookupswitchPair(Method method, int bci) {
    super(method, bci);
  }
  
  public int match() {
    return javaSignedWordAt(0 * jintSize);
  }
  
  public int offset() {
    return javaSignedWordAt(1 * jintSize);
  }
}
