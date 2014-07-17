/*
 * @(#)SingleInstanceManager.java	1.6 03/01/23
 * 
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.javaws;
import com.sun.javaws.jnl.LaunchDesc;
import java.net.*;
import com.sun.jnlp.internal.SingleInstanceServiceImpl;
import java.io.*;
import java.util.StringTokenizer;
import com.sun.javaws.debug.*;

// This is the Java Implementation of the Single Instance Service.
// This code will only be used if someone tries to use Single Instance
// Service by invoking "javaws http://jnlp.url" - one example of it
// is invoking from the Java Web Start application manager.

public class SingleInstanceManager {

    static private String _currJnlpUrl;

    static private int _currPort;

    // returns true if single instance server is running for the jnlpUrl
    static boolean isServerRunning(URL jnlpUrl) {
	// read in the sis.cfg file and check if there is a url match
	String filePath = System.getProperty("user.home") + File.separator + ".javaws" + File.separator + SingleInstanceServiceImpl.FILENAME;
	InputStream is = null;
	boolean running = false;
	try {
	    is = new FileInputStream(filePath);
	    // read in the file
	    BufferedReader in = new BufferedReader(new InputStreamReader(is));
	    Debug.println("sis.cfg file contents:");
	    String line = null;	  
	    
	    while (true) {
		// Get next line
		try {
		    line = in.readLine();
			  
		    if (line == null)
			break;
		    Debug.println(line);
		    if (line.startsWith(jnlpUrl.toString())) {
			StringTokenizer st = new StringTokenizer(line);
			_currJnlpUrl = st.nextToken();
			_currPort = Integer.parseInt(st.nextToken());
			running = true;
		    }		    
		} catch (IOException ioe) {
		    ioe.printStackTrace();
		}
		
	    }		    
	} catch (FileNotFoundException fnfe) {
	}
	return running;
    }

    // returns true if we connect successfully to the server for the jnlpUrl
    static boolean connectToServer(LaunchDesc ld) {
	Debug.println("connect to: " + _currJnlpUrl + " " + _currPort);
	//Now we open the tcpSocket and the stream
	try {
	    Socket s_socket = new Socket("localhost",_currPort);
	  
	    PrintStream out = new PrintStream(s_socket.getOutputStream());
	    BufferedReader br = new BufferedReader(
		new InputStreamReader(s_socket.getInputStream()));

	    // send MAGICWORD
	    out.println(SingleInstanceServiceImpl.MAGICWORD);
	    
	    // send over the jnlp file	   
	    out.println(ld.toString());
	    // indicate end of file transmission
	    out.println(SingleInstanceServiceImpl.EOF);
	    out.flush();	 

	    // wait for ACK (OK) response
	    Debug.println("waiting for ack");
	    int tries = 5;
	    
	    // try to listen for ACK
	    for (int i=0; i<tries; i++) {		
		String str = br.readLine();
		if (str != null && str.equals(SingleInstanceServiceImpl.ACK)) {
		    Debug.println("GOT ACK");		  
		    s_socket.close();
		    return true;
		}
	    }
	    
	    s_socket.close();
	
	} catch (java.net.ConnectException ce) {
	    // for solaris
	    // no server is running - continue launch
	    Debug.println("no server is running - continue launch!");
	    return false;
	} catch (java.net.SocketException se) {
	    // for windows
	    // no server is running - continue launch
	    Debug.println("no server is running - continue launch!");
	    return false;
	} catch (Exception ioe) {
	    ioe.printStackTrace();
	}
	return false;
    }

}
