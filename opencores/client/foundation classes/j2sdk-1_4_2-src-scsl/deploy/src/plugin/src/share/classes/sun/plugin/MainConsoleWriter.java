/*
 * @(#)MainConsoleWriter.java	1.13 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
 
package sun.plugin;

/*
 * This class is used to redirect the srderr and stdout of the java run
 * time environement. The output is using the normal Win32 debug output
 * APIs that all debuggers should implement. 
 *
 * @version 1.1 
 * @date 05/30/01
 * @Author Dennis Gu 
 */

import java.io.ByteArrayOutputStream;
import java.io.OutputStream;
import java.util.LinkedList;
import sun.plugin.util.Trace;
import sun.plugin.util.DialogFactory;
import sun.plugin.util.PluginSysUtil;
import sun.plugin.util.PluginSysAction;

public class MainConsoleWriter implements Runnable 
{
    private LinkedList queue = new LinkedList(); 
    private ByteArrayOutputStream consoleBuffer = null;
    private OutputStream traceOutputStream;
    private ConsoleWindow console = null;
    private Object consoleSyncObject = new Object();

    /* Creates a new thread for reading the data from queue periodically */
    MainConsoleWriter(OutputStream traceStream) 
    {
	this.console = null;
	this.traceOutputStream = traceStream;

	// Create a new main thread
	Thread t = PluginSysUtil.createPluginSysThread(this, "Main Console Writer");
	t.setDaemon(false);
	t.setPriority(Thread.NORM_PRIORITY + 1);
	t.start();
    }

    /*
     * Return Java Console window
     */
    ConsoleWindow getJavaConsole()
    {
	synchronized(consoleSyncObject)
	{
	    if (console == null)
	    {
		try {
		    console = (ConsoleWindow)PluginSysUtil.execute(new PluginSysAction() {
			public Object execute() 
			    throws Exception {
				return new ConsoleWindow();	
			}
		    }); 
		}
		catch(Exception e) {
		    // should never happen
		    assert(false);
		    return null;
		}

		// Check if there is any buffered data to be
		// displayed in Java Console
		//
		if (consoleBuffer != null) 
		{
		    // Remove byte array from buffer
		    byte[] buffer = (byte[]) consoleBuffer.toByteArray();

		    // Notice that the console.append MUST be called directly 
		    // because append() will try to post the event in the
		    // correct AWT event dispatch thread accordingly to
		    // the caller thread group.
		    //
		    console.append(new String(buffer));

		    // Console has been started, no need to have 
		    // buffer anymore
		    consoleBuffer = null;
		}		
	    }

	    return console;
	}
    }

    

    /* The Main Console Writer thread will get the data from the queue
     * and output to TextArea and trace file.
     */
    public void run() 
    {
	// Infinite loop
	while (true) 
	{
	    byte[] queueBytes = null;

	    try
	    {
		synchronized(queue)
		{
		    if (queue.size() > 0) 
		    {
			// Remove byte array from queue
			queueBytes = (byte[]) queue.remove(0);
		    }		
		    else
		    {
			// queue is empty, remove elements from queue
    			queue.wait();
		    }
		}

		// Notice that this operation is OUTSIDE the
		// synchronization block. This is to reduce the
		// chance to block System.out and System.err
		if (queueBytes != null)
		    this.trace(queueBytes);
	    } 
	    catch (InterruptedException e)
	    {}
	}
    }	

    /* Add element to queue */
    public void enQueue(byte[] bytes) {
	try 
	{
  	    synchronized(queue) 
	    {
		// Add element to queue
		queue.addLast(bytes);

		// Notify MainConsoleWriter thread
		queue.notifyAll();
	    }
	}
	catch(Exception e) 
	{
	    writeToConsole(e.toString());
	}
    }

    /*
     * Write the string to the console
     */
    private void writeToConsole(String str)
    {
	writeToConsole(str.getBytes());
    }

    /*
     * Write the string to the console - store in buffer
     * if console has not been created yet
     */
    private void writeToConsole(byte[] strBytes)
    {
	synchronized(consoleSyncObject)
	{
	    // Notice that the console.append MUST be called directly 
	    // because append() will try to post the event in the
	    // correct AWT event dispatch thread accordingly to
	    // the caller thread group.
	    //
	    if (console != null)
		console.append(new String(strBytes));
	    else
	    {
		// Console hasn't been created yet, 
		// queue up the output in the buffer
		//
		if (consoleBuffer == null)
		    consoleBuffer = new ByteArrayOutputStream();

		consoleBuffer.write(strBytes, 0, strBytes.length);
	    }
	}
    }

    /* Write string in Console window and tracd file */
    private void trace(byte[] strBytes) 
    {
	try
	{
	    writeToConsole(strBytes);

	    // Output string to trace file
	    traceOutputStream.write(strBytes);
	    traceOutputStream.flush();
	} catch (Exception e) {
	    console.append(e.toString());
	}
    }

}
