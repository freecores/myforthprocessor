/*
 * @(#)WorkerThread.java	1.4 03/01/23 11:52:20
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.utilities;

/** This class abstracts the notion of a worker thread which is fed
    tasks in the form of Runnables. */

public class WorkerThread {
  private volatile boolean done = false;
  private MessageQueueBackend mqb;
  private MessageQueue mq;

  public WorkerThread() {
    mqb = new MessageQueueBackend();
    mq = mqb.getFirstQueue();
    new Thread(new MainLoop()).start();
  }

  /** Runs the given Runnable in the thread represented by this
      WorkerThread object at an unspecified later time. */
  public void invokeLater(Runnable runnable) {
    mq.writeMessage(runnable);
  }

  /** Can be used to dispose of the internal worker thread. Note that
      this method may return before the internal worker thread
      terminates. */
  public void shutdown() {
    done = true;
    mq.writeMessage(new Runnable() { public void run() {} });
  }

  class MainLoop implements Runnable {
    private MessageQueue myMq;

    public MainLoop() {
      myMq = mqb.getSecondQueue();
    }

    public void run() {
      while (!done) {
        Runnable runnable = (Runnable) myMq.readMessage();
        try {
          runnable.run();
        } catch (Exception e) {
          e.printStackTrace();
        }
      }
    }
  }
}
