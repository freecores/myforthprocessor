
package equation;

/**
 */

public final class ThreadState
 {
  static final int TERMINATED = 0,
                   RUNNING = 1,
                   BLOCKED = 2,
                   WAITING = 3,
                   SUSPEND = 4;

  class Node
   {
    Thread t;
    int state;
    Object obj;
    int hash;
    long time;
    int [] data;
    int datalength;
    int dataptr;
    int [] ret;
    int retptr;
    int retlength;
    int vstack;
    int vlength;
   }

  private static Node running;
  private static Node [] queue;
  private static int queuelength;
  private static long lastTime;
  private static boolean skip;
  private static Node next;

  static
   {
    skip = false;
    next = null;

    queue = new Node[10];
    queuelength = 0;

    running = new Node();
    running.obj = null;
    running.hash = 0;
    running.data = null;
    running.datalength = 0;
    running.dataptr = 0;
    running.ret= null;
    running.retlength = 0;
    running.retptr = 0;
    running.vstack = 0;
    running.vlength = 0;
    running.time = 0;
    running.t = new Thread( new ThreadGroup(), null, "System", 0);
    running.state = RUNNING;

    long x;
    #ass "MILLISECONDS 2@ TO x";
    lastTime = x;
   }

  /** get currently running thread
   * @return Thread the running thread
   */
  public static Thread currentThread()
   {
    return running.t;
   }

  /**
   * append a thread to the scheduler table
   * @param t Thread to append
   */
  public static void append(Thread t)
   {
    if (t != null)
     synchronized(queue)
     {
      int y = t.stackSize == 0? 4000: t.stackSize;
      Node n = new Node();

      n.t = t;
      n.state = RUNNING;
      n.obj = null;
      n.hash = 0;
      n.time = 0;
      n.data = new int[y];
      // initialize data stack with parameters of run
      int l = n.data.getElem(y - 1);
      #ass "t l -1 OVER 4 - DUP TO l ! !";
      n.dataptr = l;
      n.datalength = 2;
      n.ret = new int[y];
      n.retptr = l = n.ret.getElem(y - 1);
      // put address of run on stack
      #ass "706638336 t BEGIN 0<>! OVER CELL+ @ 4 + @ OVER OVER 0<> AND WHILE NIP NIP REPEAT 2DROP";
      #ass "BEGIN 0<>! WHILE OVER OVER CELL+ @ 12 + @ -4 SEARCH-METHOD";
      #ass "0<>! IF l ! DROP 0 ELSE DROP CELL+ @ @ ENDIF REPEAT 2DROP";
      #ass "ADDCONSOLE";
      n.retlength = 1;
      n.vstack = 0;
      n.vlength = 0;

      if (queuelength == queue.length)
       queue.resize(queuelength + 10);

      queue[queuelength++] = n;
     }
   }
   
  /**
   * exit and remove thread from scheduler
   * @param t Thread the thread
   */
  public static void exit(Thread t)
   {
    if (running.t == t)
     {
      #ass "SUBCONSOLE";
      skip = true;
      switchTask(TERMINATED, null, 0, 0);
     }
    else
     synchronized(queue)
     {
      int i;

      for(i = 0; i < queuelength && queue[i].t != t; i++);

      if (i < queuelength)
       {
        #ass "SUBCONSOLE";
        Node x = queue[i];

        for(int j = i + 1; j < queuelength; )
         queue[i++] = queue[j++];

        queue[queuelength = i] = null;

        if ((i = x.vstack) != 0)
         {
          int y;
          Object o;

          #ass "SWEEP @ TO y";
          for(int k = 0; k < x.vlength && y == 0; k++)
           {
            #ass "i CELL+ @ k CELLS + @ TO o";
            JavaArray.kill(o, -1);
           }

          #ass "i 1 OVER ! TO o"; // mark list reference as white for GC
         }
       }
     }
   }

  /**
   * change state of running task and switch to next task
   * @param state int     new state
   * @param obj Object    concerned object
   * @param hash int      hashcode
   * @param time long     time to remain in new state
   */
  private static void switchTask(int state, Object obj, int hash, long time)
   {
    int pos;
    long y;

    #ass "MILLISECONDS 2@ TO y";
    long subtract = y - lastTime;
    lastTime = y;
    running.time -= subtract;

    #ass "sw1 LABEL DI ";
    if (queue._dynamicBlocking == 0)
     {
      queue._dynamicThread = running.t;
      queue._dynamicBlocking = 1;
     }
    else
     {
      #ass "EI NOP NOP NOP NOP NOP NOP NOP NOP sw1 BRANCH";
     }
    
    #ass "EI ";
    
    next = null;
    pos = queuelength;

    boolean f = true;

    for(int i = 0; i < queuelength; i++)
     {
      Node x = queue[i];

      if (x.time > 0 && subtract > 0)
       if ((x.time -= subtract) <= 0)
        {
         x.state = RUNNING;
         x.obj = null;
         x.hash = 0;
         x.time = 0;
        }

      if (f && x.state == RUNNING && x.time == 0)
       {
        pos = i;
        next = x;
        f = false;
       }
     }

    if (!f)
     {
      for(int j = pos + 1; j < queuelength;)
       queue[pos++] = queue[j++];

      running.state = state;
      running.obj = obj;
      running.hash = hash;
      running.time = time;
      
      if (skip)
       {
        queuelength--;
        queue[pos] = null;
       }
      else
       queue[pos] = running;

      switchTask();
     }
    else
     {
      if (running.time <= 0)
       running.time = time(running.t);

      #ass "DI";
      queue._dynamicBlocking = 0;
      queue._dynamicThread = null;
      unblockTask(ThreadState.WAITING, queue, true);
      #ass "EI";
     }
    
    next = null;
   }

  public static void switchTask(int state, Object obj, long time)
   {
    switchTask(state, obj, 0, time);
   }

  public static void switchTask(int state, int hash, long time)
   {
    switchTask(state, null, hash, time);
   }

  /**
   * change state of distinct thread
   * @param state int     new state
   * @param t Thread      the thread
   */
  public static void switchTask(int state, Thread t)
   {
    if (running.t == t)
     switchTask(state, null, 0, 0);
    else
     synchronized(queue)
      {
       for(int i = 0; i < queuelength;i++)
        {
         Node x = queue[i];

         if (x.t == t)
          {
           x.state = state;
           x.obj = null;
           x.hash = 0;
           x.time = 0;

           break;
          }
        }
      }
   }

  /**
   * switch to next task
   * @param next Node desired task
   */
  private static void switchTask()
   {
    // save data stack
    int ptr = next.dataptr;
    int len = next.datalength;

    #ass "DI DEPTH >R SAVE SP@ A:R@";              // save data stack
    #ass "len ptr SP!";                            // assign new data stack
    #ass "R@ R1@ A:2DROP TO len TO ptr";
    running.dataptr = ptr;
    running.datalength = len;

    ptr = next.retptr;
    len = next.retlength;
    #ass "len ptr A:2DROP";                        // move local variables
    #ass "A:DEPTH R> A:SAVE A:SP@ R@";             // save return stack
    #ass "3 PICK A:R@ 3 PICK A:R@ A:SP! 2DROP";    // assign new return stack
    #ass "A:0 A:0";                                // dummies for local variables
    #ass "TO ptr TO len 2DROP";                    // restore local variables
    running.retptr = ptr;
    running.retlength = len;

    #ass "VLENGTH @ TO len VSTACK @ TO ptr";       // save list of references
    running.vstack = ptr;
    running.vlength = len;
    ptr = next.vstack;
    len = next.vlength;
    #ass "ptr VSTACK ! len VLENGTH !";             // new list of references

    ptr = running.vstack;

    next.time = time(next.t);

    if (skip && ptr != 0)
     {
      Object o;
      #ass "SWEEP @ TO len";

      for(int i = 0; i < running.vlength && len == 0; i++)
       {
        #ass "ptr CELL+ @ i CELLS + @ TO o";
        JavaArray.kill(o, -1);
       }

      #ass "ptr 1 OVER ! TO o";
     }

    running = next;
    next = null;

    queue._dynamicBlocking = 0;
    queue._dynamicThread = null;
    unblockTask(ThreadState.WAITING, queue, true);
    skip = false;

    #ass "EI";
   }

  /**
   * calculate duration of time slot
   * @param t Thread the thread
   * @return int time in msec
   */
  private static int time(Thread t)
   {
    return (t.getPriority() - Thread.MIN_PRIORITY + 1) << 5;
   }

  /**
   * unblock task(s) waiting for an object
   * @param state int   current state
   * @param obj Object  the object
   * @param hash int    hash code
   * @param all boolean if true, unblock all tasks waiting for obj, otherwise only nearest task
   */
  private static void unblockTask(int state, Object obj, int hash, boolean all)
   {
    synchronized(queue)
     {
      boolean adv = true;

      for(int i = 0; i < queuelength && adv; i++)
       {
        Node x = queue[i];
      
        if (x.state == state && (x.obj == obj || x.hash == hash && hash != 0))
         {
          x.state = RUNNING;
          x.obj = null;
          x.time = 0;
          adv = all;
         }
       }
     }
   }

  public static void unblockTask(int state, Object obj, boolean all)
   {
    unblockTask(state, obj, 0, all);
   }

  public static void unblockTask(int state, int hash, boolean all)
   {
    unblockTask(state, null, hash, all);
   }

  /**
   * interrupt service routine from scheduler
   */
  private static void timer()
   {
    long x;

    #ass "MILLISECONDS 2@ TO x";
    long subtract = x - lastTime;

    if (queue._dynamicThread == null && subtract >= running.time)
     switchTask(RUNNING, null, 0, 0);
   }

  /**
   * get state of thread
   * @param t Thread looking for its state
   * @return int state
   */
  public static int getState(Thread t)
   {
    synchronized(queue)
     {
      for(int i = 0; i < queuelength; i++)
       {
        Node x = queue[i];

        if (x.t == t)
         return x.state;
       }

      return running.t == t? running.state: TERMINATED;
     }
   }
 }
