/*
 * @(#)ReversePtrsAnalysis.java	1.1 02/10/07 16:10:29
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.utilities;

import java.io.*;
import java.util.*;
import sun.jvm.hotspot.debugger.*;
import sun.jvm.hotspot.memory.*;
import sun.jvm.hotspot.oops.*;
import sun.jvm.hotspot.runtime.*;
import sun.jvm.hotspot.utilities.*;

/** Finds all paths from roots to the specified set of objects. NOTE:
    currently only a subset of the roots known to the VM is exposed to
    the SA: objects on the stack, static fields in classes, and JNI
    handles. These should be most of the user-level roots keeping
    objects alive. */

public class ReversePtrsAnalysis {
  // Used for debugging this code
  private static final boolean DEBUG = false;

  // Support for progress bar during analysis run.

  // NOTE that we don't know exactly how much of the heap we're going
  // to traverse, so it isn't possible to put up a progress bar that
  // goes from 0% to 100%. However, we can show how much of the heap
  // has been traversed.
  public interface ProgressThunk {
    // This will be called with a number between 0 and 1
    public void visitedFractionUpdate(double fractionOfHeapVisited);

    // This will be called after the iteration is complete
    public void done();
  }

  public ReversePtrsAnalysis() {
    rp = VM.getVM().getRevPtrs();
  }

  /** Sets an optional progress thunk */
  public void setProgressThunk(ProgressThunk thunk) {
    progressThunk = thunk;
  }

  /** Runs the analysis algorithm */
  public void run() {
    VM vm = VM.getVM();
    Universe universe = vm.getUniverse();
    CollectedHeap collHeap = universe.heap();
    usedSize = collHeap.used();
    visitedSize = 0;

    // Allocate mark bits for heap
    markBits = new MarkBits(collHeap);

    // Get a hold of the object heap
    heap = vm.getObjectHeap();

    // Do each thread's roots
    for (JavaThread thread = VM.getVM().getThreads().first();
         thread != null;
         thread = thread.next()) {
      ByteArrayOutputStream bos = new ByteArrayOutputStream();
      thread.printThreadIDOn(new PrintStream(bos));
      String threadDesc =
        " in thread \"" + thread.getThreadName() +
        "\" (id " + bos.toString() + ")";
      doStack(thread,
              new RootVisitor("Stack root" + threadDesc));
      doJNIHandleBlock(thread.activeHandles(),
                       new RootVisitor("JNI handle root" + threadDesc));
    }

    // Do global JNI handles
    JNIHandles handles = VM.getVM().getJNIHandles();
    doJNIHandleBlock(handles.globalHandles(),
                     new RootVisitor("Global JNI handle root"));
    doJNIHandleBlock(handles.weakGlobalHandles(),
                     new RootVisitor("Weak global JNI handle root"));
    
    // Do Java-level static fields in perm gen
    heap.iteratePerm(new DefaultHeapVisitor() {
        public void doObj(Oop obj) {
          if (obj instanceof InstanceKlass) {
            final InstanceKlass ik = (InstanceKlass) obj;
            ik.iterateFields(
               new DefaultOopVisitor() {
                   public void doOop(OopField field, boolean isVMField) {
                     Oop next = field.getValue(ik);
                     LivenessPathElement lp = new LivenessPathElement(null,
                             new NamedFieldIdentifier("Static field \"" +
                                                field.getID().getName() +
                                                "\" in class \"" +
                                                ik.getName().asString() + "\""));
                     rp.put(lp, next);
                     try {
                       markAndTraverse(next);
                     } catch (AddressException e) {
                       System.err.print("RevPtrs analysis: WARNING: AddressException at 0x" +
                                        Long.toHexString(e.getAddress()) +
                                        " while traversing static fields of InstanceKlass ");
                       ik.printValueOn(System.err);
                       System.err.println();
                     } catch (UnknownOopException e) {
                       System.err.println("RevPtrs analysis: WARNING: UnknownOopException while " +
                                          "traversing static fields of InstanceKlass ");
                       ik.printValueOn(System.err);
                       System.err.println();
                     }
                   }
                 },
               false);
          }
        }
      });

    if (progressThunk != null) {
      progressThunk.done();
    }

    // Clear out markBits
    markBits = null;
  }

  //---------------------------------------------------------------------------
  // Internals only below this point
  //
  private ProgressThunk       progressThunk;
  private long                usedSize;
  private long                visitedSize;
  private double              lastNotificationFraction;
  private static final double MINIMUM_NOTIFICATION_FRACTION = 0.01;
  private ObjectHeap          heap;
  private MarkBits            markBits;
  private int                 depth; // Debugging only
  private ReversePtrs         rp;

  private void markAndTraverse(OopHandle handle) {
    try {
      markAndTraverse(heap.newOop(handle));
    } catch (AddressException e) {
      System.err.println("RevPtrs analysis: WARNING: AddressException at 0x" +
                         Long.toHexString(e.getAddress()) +
                         " while traversing oop at " + handle);
    } catch (UnknownOopException e) {
      System.err.println("RevPtrs analysis: WARNING: UnknownOopException for " +
                         "oop at " + handle);
    }
  }

  private void printHeader() {
    for (int i = 0; i < depth; i++) {
      System.err.print(" ");
    }
  }

  private void markAndTraverse(final Oop obj) {
    if (obj == null) return;
    if (!markBits.mark(obj)) return;

    if (progressThunk != null) {
      visitedSize += obj.getObjectSize();
      double curFrac = (double) visitedSize / (double) usedSize;
      if (curFrac > lastNotificationFraction + MINIMUM_NOTIFICATION_FRACTION) {
        progressThunk.visitedFractionUpdate(curFrac);
        lastNotificationFraction = curFrac;
      }
    }

    if (DEBUG) {
      ++depth;
      printHeader();
      System.err.println("ReversePtrsAnalysis.markAndTraverse(" + obj.getHandle() + ")");
    }
    try {
      obj.iterate(new DefaultOopVisitor() {
          public void doOop(OopField field, boolean isVMField) {
            doFieldInObj(obj, field);
          }
        }, false);
    } catch (Exception e) {
      System.err.println("ReversePtrsAnalysis: WARNING: " + e + " during traversal");
    }
    if (DEBUG) {
      --depth;
    }
  }

  private void doFieldInObj(Oop obj, OopField field) {
    if (obj == null) return;
    Oop next = field.getValue(obj);
    rp.put(new LivenessPathElement(obj, field.getID()), next);
    markAndTraverse(next);
  }

  class RootVisitor implements AddressVisitor {
    RootVisitor(String baseRootDescription) {
      this.baseRootDescription = baseRootDescription;
    }
    
    public void visitAddress(Address addr) {
      Oop next = heap.newOop(addr.getOopHandleAt(0));
      LivenessPathElement lp = new LivenessPathElement(null,
                                        new NamedFieldIdentifier(baseRootDescription +
                                                                 " @ " + addr));
      rp.put(lp, next);
      markAndTraverse(next);
    }

    private String baseRootDescription;
  }

  // Traverse the roots on a given thread's stack
  private void doStack(JavaThread thread, AddressVisitor oopVisitor) {
    for (StackFrameStream fst = new StackFrameStream(thread); !fst.isDone(); fst.next()) {
      fst.getCurrent().oopsDo(oopVisitor, fst.getRegisterMap());
    }
  }

  // Traverse a JNIHandleBlock
  private void doJNIHandleBlock(JNIHandleBlock handles, AddressVisitor oopVisitor) {
    handles.oopsDo(oopVisitor);
  }
}
