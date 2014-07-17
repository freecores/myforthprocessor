/*
 * @(#)StackFrameStream.java	1.5 03/01/23 11:45:45
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.runtime;

import sun.jvm.hotspot.utilities.*;

/** <P> StackFrameStream iterates through the frames of a thread
    starting from top most frame. It automatically takes care of
    updating the location of all (callee-saved) registers. Notice: If
    a thread is stopped at a safepoint, all registers are saved, not
    only the callee-saved ones. </P>

    <P> Use: </P>
   
    <PRE>
    for(StackFrameStream fst = new StackFrameStream(thread); !fst.isDone(); fst.next()) {
      ...
    }
    </PRE>
*/

public class StackFrameStream {
  private Frame       fr;
  private RegisterMap regMap;
  private boolean     isDone;  

  /** Equivalent to StackFrameStream(thread, true) */
  public StackFrameStream(JavaThread thread) {
    this(thread, true);
  }

  public StackFrameStream(JavaThread thread, boolean update) {
    if (!VM.getVM().isDebugging()) {
      if (Assert.ASSERTS_ENABLED) {
        Assert.that(thread.hasLastJavaFrame(), "sanity check");  
      }
      fr = thread.getLastFrame(); 
      regMap = thread.newRegisterMap(update);
      isDone = false;
    } else {
      // Special case code to find the topmost Java frame
      // FIXME: should check to see whether we're at safepoint, and if
      // so, skip the "current frame guess" call and unnecessary
      // stackwalking work
      fr = thread.getCurrentFrameGuess();
      regMap = thread.newRegisterMap(update);
      while ((fr != null) && (!fr.isJavaFrame())) {
        if (fr.isFirstFrame()) {
          fr = null;
        } else {
          fr = fr.sender(regMap);
        }
      }
      if (fr == null) {
        isDone = true;
      }
    }
  }

  /** Iteration */
  public boolean isDone() {
    if (isDone) {
      return true;
    } else {
      if (fr == null) {
        isDone = true;
        return true;
      }
      isDone = fr.isFirstFrame();
      return false;
    }
  }

  public void next() {
    if (!isDone) {
      fr = fr.sender(regMap);
    }
  }

  /** Query */
  public Frame getCurrent()           { return fr;     }
  public RegisterMap getRegisterMap() { return regMap; }
  
  /** Is pc at current frame at a call */
  public boolean isPCAtCall() {
    return regMap.isPCAtCall(fr.getID());
  }
}
