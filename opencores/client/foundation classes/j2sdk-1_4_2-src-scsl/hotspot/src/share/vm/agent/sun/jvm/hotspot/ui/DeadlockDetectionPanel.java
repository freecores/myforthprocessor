/*
 * @(#)DeadlockDetectionPanel.java	1.2 03/01/23 11:48:38
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.ui;

import java.awt.*;
import java.io.*;
import javax.swing.*;
import java.util.*;
import java.util.Map.Entry;

import sun.jvm.hotspot.memory.*;
import sun.jvm.hotspot.runtime.*;
import sun.jvm.hotspot.oops.*;
import sun.jvm.hotspot.debugger.*;

/** Provides information about Java-level deadlocks. */

public class DeadlockDetectionPanel extends JPanel {
  public DeadlockDetectionPanel() {
    super();

    setLayout(new BorderLayout());

    // Simple at first
    JScrollPane scroller = new JScrollPane();
    JTextArea textArea = new JTextArea();
    textArea = new JTextArea();
    textArea.setEditable(false);
    textArea.setLineWrap(true);
    textArea.setWrapStyleWord(true);
    scroller.getViewport().add(textArea);
    add(scroller, BorderLayout.CENTER);

    ByteArrayOutputStream bos = new ByteArrayOutputStream();
    PrintStream tty = new PrintStream(bos);
    tty.println("Deadlock Detection:");
    tty.println();
    findDeadlocks(tty);

    textArea.setText(bos.toString());
  }

  private HashMap threadTable;

  private static Threads threads = VM.getVM().getThreads();

  private static ObjectHeap heap = VM.getVM().getObjectHeap();

  private void createThreadTable() {
    threadTable = new HashMap();
    for (JavaThread cur = threads.first(); cur != null; cur = cur.next()) {
      // initialize dfn for each thread to -1
      threadTable.put(cur, new Integer(-1));
    }
  }

  private int dfn(JavaThread thread) {
    Object obj = threadTable.get(thread);
    if (obj != null) {
      return ((Integer)obj).intValue();
    }
    return -1;
  }

  private int dfn(Entry e) {
    return ((Integer)e.getValue()).intValue();
  }

  private static void printOneDeadlock(PrintStream tty, JavaThread thread) {
    tty.println("Found one Java-level deadlock:");
    tty.println("=============================");
    ObjectMonitor waitingToLock;
    JavaThread currentThread = thread;
    do {
      tty.println();
      tty.println("\"" + currentThread.getThreadName() + "\":");
      waitingToLock = currentThread.getCurrentPendingMonitor();
      tty.print("  waiting to lock Monitor@" + waitingToLock.getAddress());
      OopHandle obj = waitingToLock.object();
      Oop oop = heap.newOop(obj);
      if (obj != null) {
        tty.print(" (Object@");
        Oop.printOopAddressOn(oop, tty);
        tty.print(", a " + oop.getKlass().getName().asString() + ")" );
        tty.print(",\n  which is held by");
      } else {
        // No Java object associated - a JVMDI/JVMPI raw monitor
        tty.print(" (a JVMDI/JVMPI raw monitor),\n  which is held by");
      }
      currentThread = threads.owningThreadFromMonitor(waitingToLock);
      tty.print(" \"" + currentThread.getThreadName() + "\"");
    } while (!currentThread.equals(thread));
    tty.println();
    tty.println();
  }

  private void findDeadlocks(PrintStream tty) {
    int globalDfn = 0, thisDfn;
    int numberOfDeadlocks = 0;
    JavaThread currentThread, previousThread;
    ObjectMonitor waitingToLock;

    createThreadTable();

    Iterator i = threadTable.entrySet().iterator();
    while (i.hasNext()) {
      Entry e = (Entry)i.next();
      if (dfn(e) >= 0) {
        // this thread was already visited
        continue;
      }

      thisDfn = globalDfn;
      JavaThread thread = (JavaThread)e.getKey();
      previousThread = thread;

      // When there is a deadlock, all the monitors involved in the dependency
      // cycle must be contended and heavyweight. So we only care about the
      // heavyweight monitor a thread is waiting to lock.
      try {
        waitingToLock = thread.getCurrentPendingMonitor();
      } catch (RuntimeException re) {
        tty.println("This version of HotSpot VM doesn't support deadlock detection.");
        tty.println("You need 1.4.0_04, 1.4.1_01 or later versions");
        return;
      }
      while (waitingToLock != null) {
        currentThread = threads.owningThreadFromMonitor(waitingToLock);
        if (currentThread == null) {
          // No dependency on another thread
          break;
        }
        if (dfn(currentThread) < 0) {
          // First visit to this thread
          threadTable.put(currentThread, new Integer(globalDfn++));
        } else if (dfn(currentThread) < thisDfn) {
          // Thread already visited, and not on a (new) cycle
          break;
        } else if (currentThread == previousThread) {
          // Self-loop, ignore
          break;
        } else {
          // We have a (new) cycle
          numberOfDeadlocks ++;
          printOneDeadlock(tty, currentThread);
          break;
        }
        previousThread = currentThread;
        waitingToLock = (ObjectMonitor)currentThread.getCurrentPendingMonitor();
      }
    }

    if (numberOfDeadlocks == 1) {
      tty.println("Found a total of 1 deadlock.");
    } else {
      tty.println("Found a total of " + numberOfDeadlocks + " deadlocks.");
    }
  }
}
