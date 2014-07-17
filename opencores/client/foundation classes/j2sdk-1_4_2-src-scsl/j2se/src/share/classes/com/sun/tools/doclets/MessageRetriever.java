/*
 * @(#)MessageRetriever.java	1.17 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.tools.doclets;

import com.sun.javadoc.*;
import java.io.*;
import java.util.*;
import java.lang.*;
import java.text.MessageFormat;


/**
 * Retrieve and format messages stored in a resource.
 *
 * @since JDK1.2
 * @author Atul M Dambalkar
 * @author Robert Field
 */
public class MessageRetriever {
    /**
     * The global configuration information for this run.
     */
    private final Configuration configuration;
    
    /**
     * The location from which to lazily fetch the resource..
     */
    private final String resourcelocation;
    
    /**
     * The lazily fetched resource..
     */
    private ResourceBundle messageRB;
    
    /**
     * True when running in quiet mode
     */
    private boolean isQuiet = false;
    
    /**
     * Initilize the ResourceBundle with the given resource.
     *
     * @param rm Resource.
     */
    public MessageRetriever(ResourceBundle rb) {
        this.configuration = null;
        this.messageRB = rb;
        this.resourcelocation = null;
    }
    
    /**
     * Initilize the ResourceBundle with the given resource.
     *
     * @param configuration the configuration
     * @param resourcelocation Resource.
     */
    public MessageRetriever(Configuration configuration,
                            String resourcelocation) {
        this.configuration = configuration;
        this.resourcelocation = resourcelocation;
    }
    
    /**
     * get and format message string from resource
     *
     * @param key selects message from resource
     */
    public String getText(String key) {
        return getText(key, (String)null);
    }
    
    /**
     * Get and format message string from resource
     *
     * @param key selects message from resource
     * @param a1 Argument, to be repalced in the message.
     */
    public String getText(String key, String a1) {
        return getText(key, a1, null);
    }
    
    /**
     * Get and format message string from resource
     *
     * @param key selects message from resource
     * @param a1 first argument to be replaced in the message.
     * @param a2 second argument to be replaced in the message.
     */
    public String getText(String key, String a1, String a2) {
        return getText(key, a1, a2, null);
    }
    
    /**
     * Get and format message string from resource
     *
     * @param key selects message from resource
     * @param a1 first argument to be replaced in the message.
     * @param a2 second argument to be replaced in the message.
     * @param a3 third argument to be replaced in the message.
     */
    public String getText(String key, String a1, String a2, String a3) {
        if (messageRB == null) {
            try {
                messageRB = ResourceBundle.getBundle(resourcelocation);
            } catch (MissingResourceException e) {
                throw new Error("Fatal: Resource (" + resourcelocation +
                                    ") for javadoc doclets is missing.");
            }
        }
        try {
            String message = messageRB.getString(key);
            String[] args = new String[3];
            args[0] = a1;
            args[1] = a2;
            args[2] = a3;
            return MessageFormat.format(message, args);
        } catch (MissingResourceException e) {
            throw new Error("Fatal: Resource (" + resourcelocation +
                                ") for javadoc is broken. There is no '" +
                                key + "' key in resource.");
        }
    }
    
    /**
     * Print error message, increment error count.
     *
     * @param pos the position of the source
     * @param msg message to print
     */
    void printError(SourcePosition pos, String msg) {
        if (configuration != null && configuration.root != null) {
            configuration.root.printError(pos, msg);
        } else {
            System.err.println(/* Main.program + ": " + */ msg);
        }
    }
    
    /**
     * Print error message, increment error count.
     *
     * @param msg message to print
     */
    void printError(String msg) {
        if (configuration != null && configuration.root != null) {
            configuration.root.printError(msg);
        } else {
            System.err.println(/* Main.program + ": " + */ msg);
        }
    }
    
    /**
     * Print warning message, increment warning count.
     *
     * @param pos the position of the source
     * @param msg message to print
     */
    void printWarning(SourcePosition pos, String msg) {
        if (configuration != null && configuration.root != null) {
            configuration.root.printWarning(pos, msg);
        } else {
            System.err.println(/* Main.program +  ": warning - " + */ "Warning: " + msg);
        }
    }
    
    /**
     * Print warning message, increment warning count.
     *
     * @param msg message to print
     */
    void printWarning(String msg) {
        if (configuration != null && configuration.root != null) {
            configuration.root.printWarning(msg);
        } else {
            System.err.println(/* Main.program +  ": warning - " + */ "Warning: " + msg);
        }
    }
    
    /**
     * Print a message.
     *
     * @param pos the position of the source
     * @param msg message to print
     */
    void printNotice(SourcePosition pos, String msg) {
        
        if(! isQuiet){
            if (configuration != null && configuration.root != null) {
                configuration.root.printNotice(pos, msg);
            } else {
                System.out.println(msg);
            }
        }
    }
    
    /**
     * Print a message.
     *
     * @param msg message to print
     */
    void printNotice(String msg) {
        
        if(! isQuiet){
            if (configuration != null && configuration.root != null) {
                configuration.root.printNotice(msg);
            } else {
                System.out.println(msg);
            }
        }
    }
    
    /**
     * Print error message, increment error count.
     *
     * @param pos the position of the source
     * @param key selects message from resource
     */
    public void error(SourcePosition pos, String key) {
        printError(pos, getText(key));
    }
    
    /**
     * Print error message, increment error count.
     *
     * @param key selects message from resource
     */
    public void error(String key) {
        printError(getText(key));
    }
    
    /**
     * Print error message, increment error count.
     *
     * @param pos the position of the source
     * @param key selects message from resource
     * @param a1 first argument to be replaced in the message.
     */
    public void error(SourcePosition pos, String key, String a1) {
        printError(pos, getText(key, a1));
    }
    
    /**
     * Print error message, increment error count.
     *
     * @param key selects message from resource
     * @param a1 first argument to be replaced in the message.
     */
    public void error(String key, String a1) {
        printError(getText(key, a1));
    }
    
    /**
     * Print error message, increment error count.
     *
     * @param pos the position of the source
     * @param key selects message from resource
     * @param a1 first argument to be replaced in the message.
     * @param a2 second argument to be replaced in the message.
     */
    public void error(SourcePosition pos, String key, String a1, String a2) {
        printError(pos, getText(key, a1, a2));
    }
    
    /**
     * Print error message, increment error count.
     *
     * @param key selects message from resource
     * @param a1 first argument to be replaced in the message.
     * @param a2 second argument to be replaced in the message.
     */
    public void error(String key, String a1, String a2) {
        printError(getText(key, a1, a2));
    }
    
    /**
     * Print error message, increment error count.
     *
     * @param pos the position of the source
     * @param key selects message from resource
     * @param a1 first argument to be replaced in the message.
     * @param a2 second argument to be replaced in the message.
     * @param a3 third argument to be replaced in the message.
     */
    public void error(SourcePosition pos, String key, String a1, String a2, String a3) {
        printError(pos, getText(key, a1, a2, a3));
    }
    
    /**
     * Print error message, increment error count.
     *
     * @param key selects message from resource
     * @param a1 first argument to be replaced in the message.
     * @param a2 second argument to be replaced in the message.
     * @param a3 third argument to be replaced in the message.
     */
    public void error(String key, String a1, String a2, String a3) {
        printError(getText(key, a1, a2, a3));
    }
    
    /**
     * Print warning message, increment warning count.
     *
     * @param pos the position of the source
     * @param key selects message from resource
     */
    public void warning(SourcePosition pos, String key) {
        printWarning(pos, getText(key));
    }
    
    /**
     * Print warning message, increment warning count.
     *
     * @param key selects message from resource
     */
    public void warning(String key) {
        printWarning(getText(key));
    }
    
    /**
     * Print warning message, increment warning count.
     *
     * @param pos the position of the source
     * @param key selects message from resource
     * @param a1 first argument to be replaced in the message.
     */
    public void warning(SourcePosition pos, String key, String a1) {
        printWarning(pos, getText(key, a1));
    }
    
    /**
     * Print warning message, increment warning count.
     *
     * @param key selects message from resource
     * @param a1 first argument to be replaced in the message.
     */
    public void warning(String key, String a1) {
        printWarning(getText(key, a1));
    }
    
    /**
     * Print warning message, increment warning count.
     *
     * @param pos the position of the source
     * @param key selects message from resource
     * @param a1 first argument to be replaced in the message.
     * @param a2 second argument to be replaced in the message.
     */
    public void warning(SourcePosition pos, String key, String a1, String a2) {
        printWarning(pos, getText(key, a1, a2));
    }
    
    /**
     * Print warning message, increment warning count.
     *
     * @param key selects message from resource
     * @param a1 first argument to be replaced in the message.
     * @param a2 second argument to be replaced in the message.
     */
    public void warning(String key, String a1, String a2) {
        printWarning(getText(key, a1, a2));
    }
    
    /**
     * Print warning message, increment warning count.
     *
     * @param pos the position of the source
     * @param key selects message from resource
     * @param a1 first argument to be replaced in the message.
     * @param a2 second argument to be replaced in the message.
     * @param a3 third argument to be replaced in the message.
     */
    public void warning(SourcePosition pos, String key, String a1, String a2, String a3) {
        printWarning(pos, getText(key, a1, a2, a3));
    }
    
    /**
     * Print warning message, increment warning count.
     *
     * @param key selects message from resource
     * @param a1 first argument to be replaced in the message.
     * @param a2 second argument to be replaced in the message.
     * @param a3 third argument to be replaced in the message.
     */
    public void warning(String key, String a1, String a2, String a3) {
        printWarning(getText(key, a1, a2, a3));
    }
    
    /**
     * Print a message.
     *
     * @param pos the position of the source
     * @param key selects message from resource
     */
    public void notice(SourcePosition pos, String key) {
        printNotice(pos, getText(key));
    }
    
    /**
     * Print a message.
     *
     * @param key selects message from resource
     */
    public void notice(String key) {
        printNotice(getText(key));
    }
    
    /**
     * Print a message.
     * @param pos the position of the source
     * @param key selects message from resource
     * @param a1 first argument to be replaced in the message.
     */
    public void notice(SourcePosition pos, String key, String a1) {
        printNotice(pos, getText(key, a1));
    }
    
    /**
     * Print a message.
     *
     * @param key selects message from resource
     * @param a1 first argument to be replaced in the message.
     */
    public void notice(String key, String a1) {
        printNotice(getText(key, a1));
    }
    
    /**
     * Print a message.
     *
     * @param pos the position of the source
     * @param key selects message from resource
     * @param a1 first argument to be replaced in the message.
     * @param a2 second argument to be replaced in the message.
     */
    public void notice(SourcePosition pos, String key, String a1, String a2) {
        printNotice(pos, getText(key, a1, a2));
    }
    
    /**
     * Print a message.
     *
     * @param pos the position of the source     * @param key selects message from resource
     * @param a1 first argument to be replaced in the message.
     * @param a2 second argument to be replaced in the message.
     */
    public void notice(String key, String a1, String a2) {
        printNotice(getText(key, a1, a2));
    }
    
    /**
     * Print a message.
     *
     * @param pos the position of the source
     * @param key selects message from resource
     * @param a1 first argument to be replaced in the message.
     * @param a2 second argument to be replaced in the message.
     * @param a3 third argument to be replaced in the message.
     */
    public void notice(SourcePosition pos, String key, String a1, String a2, String a3) {
        printNotice(pos, getText(key, a1, a2, a3));
    }
    
    /**
     * Print a message.
     *
     * @param key selects message from resource
     * @param a1 first argument to be replaced in the message.
     * @param a2 second argument to be replaced in the message.
     * @param a3 third argument to be replaced in the message.
     */
    public void notice(String key, String a1, String a2, String a3) {
        printNotice(getText(key, a1, a2, a3));
    }
    
    /**
     * Set to quiet mode.
     */
    public void setQuiet(){
        isQuiet = true;
    }
}
