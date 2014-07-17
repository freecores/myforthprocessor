/*
 * @(#)PluginLogger.java	1.16 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.util;

import java.io.File;
import java.security.PrivilegedAction;
import java.util.logging.FileHandler;
import java.util.logging.Handler;
import java.util.logging.Level;
import java.util.logging.Logger;
import sun.plugin.util.UserProfile;

/**
 * PluginLogger is a class that provides logging support through
 * the new Logging API in J2SE v1.4.
 *
 */
public class PluginLogger
{
    // Logger object for Java Plug-in
    private static Logger logger = null;
    
    // True if logging is on
    private static boolean logOn = false;

    // True if logger has been initialized
    private static boolean loggerInit = false;

    static
    {
	try
	{
            logOn = ((Boolean) java.security.AccessController.doPrivileged(
    			       new sun.security.action.GetBooleanAction("javaplugin.logging"))).booleanValue();

	    // Initialize the logger object
	    initLogger();
	}
	catch (Throwable e)
	{
	    Trace.printException(e);
	}
    }

    /**
     * Initialize the logger object
     */
    private static synchronized void initLogger()
    {
	if (loggerInit == false && logOn)
	{
	    loggerInit = true;

	    logger = (Logger) java.security.AccessController.doPrivileged(new PrivilegedAction()
		     {
			public Object run() 
			{
			    Logger logger = null;

			    try
			    {
    		 	        // Create logger object for sun.plugin package
			        logger = Logger.getLogger("sun.plugin");

    				Handler handler = new FileHandler(UserProfile.getLogFile());

				// Send logger output to our FileHandler
				logger.addHandler(handler);

				// Request every details gets logged
				logger.setLevel(Level.ALL);
			    }
			    catch (Throwable e)
			    {
				Trace.printException(e);
			    }

			    return logger;
			}
		    });
	}
    }
    
    /** 
     * Static method to trigger logging.
     */
    public static void setLogging(boolean on)
    {
	logOn = on;

	// Initialize the logger object
	initLogger();
    }

    /** 
     * Static method to trigger logging.
     */
    public static boolean getLogging()
    {
	return logOn;
    }

    /** 
     * Static method to obtain the logger.
     */
    private static Logger getLogger()
    {
	// Initialize the logger object
	initLogger();

	if (logOn == true)
	    return logger;
	else
	    return null;
    }

    /**
     * Static method to log a message.
     *
     * @param msg Message to be logged.
     */
    public static void log(String msg)
    {
	// Log exception using logger
	Logger logger = PluginLogger.getLogger();

	if (logger != null)
	    logger.log(Level.INFO, msg);
    }

    /**
     * Static method to log an exception.
     *
     * @param ex Throwable to be logged.
     */
    public static void logException(Throwable e)
    {
	logException("THROW", e);
    }

    /**
     * Static method to log an exception.
     *
     * @param msg Message related to the exception.
     * @param ex Throwable to be logged.
     */
    public static void logException(String msg, Throwable e)
    {
	// Log exception using logger
	Logger logger = PluginLogger.getLogger();

	if (logger != null)
	    logger.log(Level.FINER, msg, e);
    }
}


