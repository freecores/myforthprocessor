/*
 * @(#)DebugOutputStream.java	1.28 03/01/23
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
 * @date 12/27/97
 * @Author Jerome Dochez
 *
 * Modified by:
 * Date:	05/30/01
 * Author:	Dennis Gu
 * Reason:	Fix bug 4380275 - two debug outputstream will put the text in a queue. 
 */

import java.io.ByteArrayOutputStream;

public class DebugOutputStream extends ByteArrayOutputStream
{
    private MainConsoleWriter mainConsoleWriter;

    /**
     * <p>
     * Creates a new output stream for debugging tracing
     * All messages will be forwarded to the console window.
     * Another hook is used for subclasses to display into
     * another tracing facility.
     * </p>
     *
     * @param mainConsoleWriter The java ConsoleWriter to use for output
     */
    DebugOutputStream(MainConsoleWriter mainConsoleWriter) 
    {
	this.mainConsoleWriter = mainConsoleWriter;
    }

    /**
     * The Main Console Writer thread will get the data from the queue to the 
     * TextArea and trace file.
     */

    private void enQueueData()
    {
	byte[]	buf = toByteArray();
	reset();
	mainConsoleWriter.enQueue(buf);
    }

    /**
     * Writes the specified byte to this buffered output stream. 
     *
     * @param      b   the byte to be written.
     */
    public synchronized void write(int b) {
	super.write(b);

	// Add data to queue when end of line or EOF.
	if (b == '\n' || b == '\r')
	   enQueueData();
    }

    /**
     * Writes <code>len</code> bytes from the specified byte array 
     * starting at offset <code>off</code> to this buffered output stream.
     *
     * @param      b     the data.
     * @param      off   the start offset in the data.
     * @param      len   the number of bytes to write.
     */
    public synchronized void write(byte b[], int off, int len) {
	super.write(b,off,len);

	// Add data to queue when end of line or EOF.
	if (b[len-1] == '\n' || b[len-1] == '\r')
	   enQueueData();
    }

    /**
     * <p>
     * Method to provide an alternative tracing facility beside 
     * the console window. Messages could be traced for example 
     * in a debugger window.
     * </p>
     *
     * @param str trace message
     */
    protected void printDebug(String out) {}
}
