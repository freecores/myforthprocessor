/*
 * @(#)LivenessAnalysis.java	1.5 03/01/23 11:51:32
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

/** Finds all paths from roots to the specified set of objects. NOTE:
    currently only a subset of the roots known to the VM is exposed to
    the SA: objects on the stack, static fields in classes, and JNI
    handles. These should be most of the user-level roots keeping
    objects alive. */

public class LivenessAnalysis {
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

  public LivenessAnalysis() {
  }

  /** Interface which defines whether a given object is in the desired
      set of target objects for which we want liveness information */
  public interface Target {
    public boolean isTarget(Oop obj);
  }

  /** Allows the user to specify the objects for which liveness is
      computed in the form of a piece of code which tests each object
      visited */
  public void setTarget(Target target) {
    this.target = target;
  }

  /** Alternatively, the user can specify the target objects in the
      form of a Set of Oops */
  public void setTargetObjects(final Set/*<Oop>*/ setOfOops) {
    target = new Target() {
        public boolean isTarget(Oop obj) {
          return setOfOops.contains(obj);
        }
      };
  }

  /** Sets an optional progress thunk */
  public void setProgressThunk(ProgressThunk thunk) {
    progressThunk = thunk;
  }

  /** Runs the analysis algorithm */
  public void run() {
    if (target == null) {
      throw new RuntimeException("Must specify target first");
    }

    VM vm = VM.getVM();
    Universe universe = vm.getUniverse();
    CollectedHeap collHeap = universe.heap();
    usedSize = collHeap.used();
    visitedSize = 0;

    // Allocate mark bits for heap
    markBits = new MarkBits(collHeap);

    // Get a hold of the object heap
    heap = vm.getObjectHeap();

    // Allocate working liveness path
    path = new LivenessPath();

    // Allocate result set
    result = new HashMap();

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
            ik.iterateFields(new DefaultOopVisitor() {
                public void doOop(OopField field, boolean isVMField) {
                  path.push
                    (new LivenessPathElement
                      (null,
                       new NamedFieldIdentifier("Static field \"" +
                                                field.getID().getName() +
                                                "\" in class \"" +
                                                ik.getName().asString() + "\"")));
                  try {
                    markAndTraverse(field.getValue(ik));
                  } catch (AddressException e) {
                    System.err.print("Liveness analysis: WARNING: AddressException at 0x" +
                                     Long.toHexString(e.getAddress()) +
                                     " while traversing static fields of InstanceKlass ");
                    ik.printValueOn(System.err);
                    System.err.println();
                  } catch (UnknownOopException e) {
                    System.err.println("Liveness analysis: WARNING: UnknownOopException while " +
                                       "traversing static fields of InstanceKlass ");
                    ik.printValueOn(System.err);
                    System.err.println();
                  }
                  path.pop();
                }
              }, false);
          }
        }
      });

    if (progressThunk != null) {
      progressThunk.done();
    }

    // Clear out markBits
    markBits = null;
  }

  /** Fetches the result of the analysis algorithm as a Map mapping
      Oops to LivenessPathLists */
  public Map result() {
    return result;
  }

  //---------------------------------------------------------------------------
  // Internals only below this point
  //
  private ProgressThunk       progressThunk;
  private long                usedSize;
  private long                visitedSize;
  private double              lastNotificationFraction;
  private static final double MINIMUM_NOTIFICATION_FRACTION = 0.01;
  private Target              target;
  private ObjectHeap          heap;
  private MarkBits            markBits;
  private LivenessPath        path;
  private Map                 result;
  private int                 depth; // Debugging only

  private void markAndTraverse(OopHandle handle) {
    try {
      markAndTraverse(heap.newOop(handle));
    } catch (AddressException e) {
      System.err.println("Liveness analysis: WARNING: AddressException at 0x" +
                         Long.toHexString(e.getAddress()) +
                         " while traversing oop at " + handle);
    } catch (UnknownOopException e) {
      System.err.println("Liveness analysis: WARNING: UnknownOopException for " +
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
      System.err.print("LivenessAnalysis.markAndTraverse(" + obj.getHandle() + ")");
    }
    boolean isTarget = target.isTarget(obj);
    if (isTarget) {
      if (DEBUG) {
        System.err.print(" ** target **");
      }
      path.push(new LivenessPathElement(obj, null));
      LivenessPathList list = (LivenessPathList) result.get(obj);
      if (list == null) {
        list = new LivenessPathList();
        result.put(obj, list);
      }
      list.add(path.copy());
      path.pop();
    }
    if (DEBUG) {
      System.err.println();
    }
    try {
      obj.iterate(new DefaultOopVisitor() {
          public void doOop(OopField field, boolean isVMField) {
            doFieldInObj(obj, field);
          }
        }, false);
    } catch (Exception e) {
      System.err.println("LivenessAnalysis: WARNING: " + e + " during traversal");
    }
    if (isTarget) {
      // Clear mark bit for this object so we get "all-paths-to"
      // behavior
      markBits.clear(obj);
    }
    if (DEBUG) {
      --depth;
    }
  }

  private void doFieldInObj(Oop obj, OopField field) {
    if (obj == null) return;
    path.push(new LivenessPathElement(obj, field.getID()));
    Oop next = field.getValue(obj);
    markAndTraverse(next);
    path.pop();
  }

  class RootVisitor implements AddressVisitor {
    RootVisitor(String baseRootDescription) {
      this.baseRootDescription = baseRootDescription;
    }
    
    public void visitAddress(Address addr) {
      path.push(
        new LivenessPathElement(null,
                                new NamedFieldIdentifier(baseRootDescription +
                                                         " @ " + addr)));
      markAndTraverse(addr.getOopHandleAt(0));
      path.pop();
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
