/*
 * @(#)AbstractLauncher.java	1.10 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.tools.jdi;

import com.sun.jdi.connect.*;
import com.sun.jdi.VirtualMachine;
import com.sun.jdi.InternalException;

import java.util.Map;
import java.util.StringTokenizer;
import java.util.List;
import java.util.ArrayList;
import java.io.IOException;
import java.io.InterruptedIOException;

abstract class AbstractLauncher extends ConnectorImpl implements LaunchingConnector {
    private TransportService transport;

    AbstractLauncher(VirtualMachineManagerService manager, 
                     TransportService transport) {
        super(manager);
        this.transport = transport;
    }

    abstract public VirtualMachine launch(Map arguments) 
                                 throws IOException, 
                                        IllegalConnectorArgumentsException,
                                        VMStartException;
    abstract public String name();
    abstract public String description();

     
    String[] tokenizeCommand(String command, char quote) {
        String quoteStr = String.valueOf(quote); // easier to deal with

        /*
         * Tokenize the command, respecting the given quote character.
         */
        StringTokenizer tokenizer = new StringTokenizer(command, 
                                                        quote + " \t\r\n\f",
                                                        true);
        String quoted = null;
        String pending = null;
        List tokenList = new ArrayList();
        while (tokenizer.hasMoreTokens()) {
            String token = tokenizer.nextToken();
            if (quoted != null) {
                if (token.equals(quoteStr)) {
                    tokenList.add(quoted);
                    quoted = null;
                } else {
                    quoted += token;
                }
            } else if (pending != null) {
                if (token.equals(quoteStr)) {
                    quoted = pending;
                } else if ((token.length() == 1) && 
                           Character.isWhitespace(token.charAt(0))) {
                    tokenList.add(pending);
                } else {
                    throw new InternalException("Unexpected token: " + token);
                }
                pending = null;
            } else {
                if (token.equals(quoteStr)) {
                    quoted = "";
                } else if ((token.length() == 1) && 
                           Character.isWhitespace(token.charAt(0))) {
                    // continue
                } else {
                    pending = token;
                }
            }
        }

        /*
         * Add final token.
         */
        if (pending != null) {
            tokenList.add(pending);
        }

        /*
         * An unclosed quote at the end of the command. Do an 
         * implicit end quote.
         */
        if (quoted != null) {
            tokenList.add(quoted);
        }

        String[] tokenArray = new String[tokenList.size()];
        for (int i = 0; i < tokenList.size(); i++) {
            tokenArray[i] = (String)tokenList.get(i);
        }
        return tokenArray;
    }

    protected VirtualMachine launch(String[] commandArray, String address) 
                                    throws IOException, VMStartException {
        Helper helper = new Helper(commandArray, address);
        helper.launchAndAccept();
        return manager().createVirtualMachine(helper.connection(),
                                              helper.process());
    }

    public Transport transport() {
        return transport;
    }

    protected TransportService transportService() {
        return transport;
    }

    /**
     * This class simply provides a context for a single launch and 
     * accept. It provides instance fields that can be used by 
     * all threads involved. This stuff can't be in the Connector proper
     * because the connector is is a singleton and not specific to any
     * one launch. 
     */
    private class Helper {
        private final String address;
        private final String[] commandArray;
        private Process process = null;
        private ConnectionService connection = null;
        private IOException acceptException = null;
        private boolean exited = false;

        Helper(String[] commandArray, String address) {
            this.commandArray = commandArray;
            this.address = address;
        }

        String commandString() {
            String str = "";
            for (int i = 0; i < commandArray.length; i++) {
                if (i > 0) {
                    str += " ";
                }
                str += commandArray[i];
            }
            return str;
        }

        synchronized void launchAndAccept() throws 
                                IOException, VMStartException {

            process = Runtime.getRuntime().exec(commandArray);

            Thread acceptingThread = acceptConnection();
            Thread monitoringThread = monitorTarget();
            try {
                while ((connection == null) && 
                       (acceptException == null) && 
                       !exited) {
                    wait();
                }

                if (exited) {
                    throw new VMStartException(
                        "VM initialization failed for: " + commandString(), process);
                } 
                if (acceptException != null) {
                    // Rethrow the exception in this thread
                    throw acceptException;
                }
            } catch (InterruptedException e) {
                throw new InterruptedIOException("Interrupted during accept");
            } finally {
                acceptingThread.interrupt();
                monitoringThread.interrupt();
            }
        }

        Process process() {
            return process;
        }

        ConnectionService connection() {
            return connection;
        }

        synchronized void notifyOfExit() {
            exited = true;
            notify();
        }

        synchronized void notifyOfConnection(ConnectionService connection) {
            this.connection = connection;
            notify();
        }

        synchronized void notifyOfAcceptException(IOException acceptException) {
            this.acceptException = acceptException;
            notify();
        }

        Thread monitorTarget() {
            Thread thread = new Thread(mainGroupForJDI(),
				       "launched target monitor") {
                public void run() {
                    try {
                        process.waitFor();
                        /*
                         * Notify waiting thread of VM error termination 
                         */
                        notifyOfExit();
                    } catch (InterruptedException e) {
                        // Connection has been established, stop monitoring 
                    }
                }
            };
            thread.setDaemon(true);
            thread.start();
            return thread;
        }

        Thread acceptConnection() {
            Thread thread = new Thread(mainGroupForJDI(),
				       "connection acceptor") {
                public void run() {
                    try {
                        ConnectionService connection = transport.accept(address);
                        /*
                         * Notify waiting thread of connection
                         */
                        notifyOfConnection(connection);
                    } catch (InterruptedIOException e) {
                        // VM terminated, stop accepting 
                    } catch (IOException e) {
                        // Report any other exception to waiting thread
                        notifyOfAcceptException(e);
                    }
                }
            };
            thread.setDaemon(true);
            thread.start();
            return thread;
        }
    }
}

