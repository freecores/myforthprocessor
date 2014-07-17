/*
 * @(#)JavaRunTime.java	1.47 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin;

/*
 * A loaded and running Java Virtual machine is initialized and
 * released in the JavaRunTime.
 * This is the opportunity for startup code
 *
 * @version 	1.2.1
 * @author	Jerome Dochez
 *
 *
 * Modified by:
 * Rita Fisher      Apr. 04, 2001       Bug fix 4421334 - made Java Console truly disable-able (there are now
 *                                      three possible states for Java Console - full size, hidden (icon/system
 *                                      tray) or no console).
 * 
 */

import java.io.IOException;
import java.io.BufferedOutputStream;
import java.io.OutputStream;
import java.io.File;
import java.io.FileOutputStream;    
import java.io.PrintStream;

import sun.plugin.util.Trace;
import sun.plugin.util.UserProfile;
import sun.plugin.util.PluginSysUtil;


public class JavaRunTime 
{
    /**
     * <p>
     * Method called once when the javaplugin is loaded. This is the opportunity
     * to add some initialization function. For convenience the java home is 
     * passed to be able to know where the java environement resides
     *
     * @param javaHome		java home directory
     * @param bridgeLibPath	Activator path
     * @param userHome		user home dir
     */
    protected static void initEnvironment(String javaHome, String bridgeLibPath, String userHome) 
    {
	java.util.Properties systemProps = System.getProperties();
	systemProps.put("java.home", javaHome );  
	if (userHome==null)
	    systemProps.put("user.home", javaHome);
	else
	    systemProps.put("user.home", userHome);

	// Load the properties
	AppletViewer.loadPropertiesFiles();
    }     

    /**
     * <p>
     * Return the console window for this running Java Virtual Machine.
     * If the console is not created yet, create a new Java Console.
     *
     * @return console window object
     * </p>
     */
    public synchronized static ConsoleWindow getJavaConsole()
    {
	// Initialize tracing environment
	initTraceEnvironment();

	// Obtain console from mainWriter
	return mainWriter.getJavaConsole();	    
    }

    /**
     * <p>
     * Initialize the trace file facilities.
     * </p>
     */
    public synchronized static void initTraceEnvironment()
    {
	// Return if environment has been initialized
	if (traceInit) 
	    return;

	traceInit = true;

	// Create a new Console writer to read data from queue and output to
	// console and trace file.
	mainWriter = new MainConsoleWriter(getTraceOutputStream());

        // Reassign stderr and stdout.
	//
	// Notice that two separate debug streams should be used
	// instead of one. They will put the data into a queue, then
	// the console writer thread will read from the queue periodically.
	//

	/* Set Err string to console and file (trace or my own) */
	DebugOutputStream myErrOS = new DebugOutputStream(mainWriter);
	PrintStream myErrPS = new PrintStream(myErrOS, true);
	System.setErr(myErrPS);

	/* Set out string to console and file (trace or my own)*/
	DebugOutputStream myOutOS = new DebugOutputStream(mainWriter);
	PrintStream myOutPS = new PrintStream(myOutOS, true);
	System.setOut(myOutPS);

	// Display version information
	String strDisplay = ConsoleWindow.displayVersion();
	System.out.print(strDisplay);
    }


    /**
     * <p>
     * Checks if the Java Console Window is visible.
     *
     * @return true if visible.
     * </p>
     */
    public static boolean isJavaConsoleVisible()  
    {
	ConsoleWindow f = getJavaConsole();   

	if (f != null)
	{
	    return f.isConsoleVisible();
	}
	else
	{
	    return false;
	}
    }


    /**
     * <p>
     * Show or hide Java Console Window.
     *
     * @param visible true if to show, else to hide.
     * </p>
     */
    public static void showJavaConsole(final boolean visible)  
    {      
	final ConsoleWindow f = getJavaConsole();

	if (f != null)
	{
	    // Notice that setVisible MUST be called from
	    // the event dispatch thread because the Swing
	    // component may have been realized. Otherwise,
	    // it may cause deadlock.
	    //
	    try
	    {
		PluginSysUtil.invokeAndWait(new Runnable()
		{
		    public void run()
		    {
			f.showConsole(visible);
		    }
		});
	    }
	    catch (Throwable e)
	    {
		e.printStackTrace();
	    }
	}	
    }


    /**
     * <p>
     * Show or hide Java Console Window later.
     *
     * @param visible true if to show, else to hide.
     * </p>
     */
    public static void showJavaConsoleLater(final boolean visible)  
    {
	final ConsoleWindow f = getJavaConsole();

	if (f != null)
	{
	    // Notice that setVisible MUST be called from
	    // the event dispatch thread because the Swing
	    // component may have been realized. Otherwise,
	    // it may cause deadlock.
	    //
	    try
	    {
		PluginSysUtil.invokeLater(new Runnable()
		{
		    public void run()
		    {
			f.showConsole(visible);
		    }
		});
	    }
	    catch (Throwable e)
	    {
		e.printStackTrace();
	    }
	}	
    }

    /**
     * <p>
     * Show Help in Java Console
     * </p>
     */
    public static void showJavaConsoleHelp()  
    {
	// Initialize environment only, delay starting the 
	// console if possible.
	//
	initTraceEnvironment();

	String helpString = ConsoleWindow.displayHelp();
	System.out.print(helpString);
    }

    /**
     * <p>
     * Show System Properties in Java Console
     * </p>
     */
    public static void showSystemProperties()
    {
	// Initialize environment only, delay starting the 
	// console if possible.
	//
        initTraceEnvironment();

	ConsoleWindow.displaySystemProperties();
    }

    /**
     * <p>
     * Print a message to Java Console Window.
     *
     * @param msg Message to be printed.
     * </p>
     */
    public static void printToJavaConsole(String msg)   
    {
	ConsoleWindow f = getJavaConsole();

	if (f != null)
	{
	    f.append(msg);
	}
    }

    /**
     * <p>
     * Return the trace output stream.
     *
     * @return trace output stream.
     * </p>
     */
    public static OutputStream getTraceOutputStream() 
    {  
	// Return stream if exists
	if (traceStream != null) 
	{
	    return traceStream;
	}

	try 
	{
	    File file = new File(UserProfile.getTraceFile());

	    if (file.exists()) 
		file.delete();

	    traceStream = new BufferedOutputStream(new FileOutputStream(file));
	} 
	catch (IOException e) 
	{
	    e.printStackTrace();
	}

	return traceStream;
    }

    /**
     * <p>
     *  @return plugin thread group
     * </p>
     */
     public synchronized static ThreadGroup getPluginThreadGroup() {
	return PluginSysUtil.getPluginThreadGroup();
     }

    /*
     * <p>
     * the Java Console to display useful error and trace messages
     * </p>
     *
     */
    private static OutputStream traceStream = null;    
    private static MainConsoleWriter mainWriter = null;
    private static boolean traceInit = false;
    public static native String dumpAllStacks();  

}



