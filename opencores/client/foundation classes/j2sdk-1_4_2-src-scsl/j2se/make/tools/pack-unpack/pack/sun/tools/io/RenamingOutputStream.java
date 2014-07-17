/*
 * @(#)RenamingOutputStream.java	1.3 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.tools.io;

import java.io.*;

/**
 * Wrapper for FileOutputStream which
 * Directs all output to a temporary file, then renames to the
 * final target file when the stream is finally closed.
 * Avoids trashing the target file if the update fails to complete.
 * @author John Rose
 * @version 1.3, 01/23/03
 */
public
class RenamingOutputStream extends FilterOutputStream {
    private final File temp;
    private final File file;
    private boolean didClose;
    private final static boolean debug = false;

    public RenamingOutputStream(String file) throws IOException {
	this(new File(file));
    }
    public RenamingOutputStream(File file) throws IOException {
	this(file, chooseTempFile(file));
    }
    private RenamingOutputStream(File file, File temp) throws IOException {
	super(new FileOutputStream(temp));
	this.file = file;
	this.temp = temp;
	if (file.exists() && !file.canWrite())  failWrite();
    }
    private static File chooseTempFile(File file) throws IOException {
	File dir = file.getParentFile();
	if (debug)  System.out.println("chooseTempFile "+file);
	File temp = File.createTempFile("#"+file.getName(), "#", dir);
	// Workaround:  make the temp be in the destination directory.
	temp = new File(dir, temp.getName());
	if (debug)  System.out.println("chooseTempFile -> "+temp);
	temp.deleteOnExit();
	return temp;
    }
    private void failWrite() throws IOException {
	throw new IOException("cannot replace: "+file);
    }

    // Be sure to pass through block output requests:
    public void write(byte b[], int off, int len) throws IOException {
	super.out.write(b, off, len);
    }
    public synchronized void close() throws IOException {
	super.close();
	if (didClose)  return;
	didClose = true;
	//System.out.println(temp+" => "+file);
	if (!temp.renameTo(file)) {
	    // Trouble?  Try deleting the target file first.
	    file.delete();
	    if (!temp.renameTo(file)) {
		// Trouble.  Cannot overwrite this file.
		String saveName = "#"+file.getName()+"#";
		File saveFile = new File(file.getParent(), saveName);
		temp.renameTo(saveFile);
		failWrite();
	    }
	}
    }
}
