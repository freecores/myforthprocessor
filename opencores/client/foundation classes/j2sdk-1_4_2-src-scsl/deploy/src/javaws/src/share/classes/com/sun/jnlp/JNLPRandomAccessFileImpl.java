/*
 * @(#)JNLPRandomAccessFileImpl.java	1.8 03/01/23
 * 
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.jnlp;

import java.io.File;
import java.io.IOException;
import java.io.RandomAccessFile;
import javax.jnlp.JNLPRandomAccessFile;
import javax.jnlp.FileContents;
import com.sun.javaws.Resources;
import com.sun.javaws.debug.Debug;

public final class JNLPRandomAccessFileImpl implements JNLPRandomAccessFile {
    private RandomAccessFile _raf = null;
    private FileContents _contents = null;  // Cannot be NULL
    private long _length = 0;
    private String _message = null;
    
    JNLPRandomAccessFileImpl(File file, String mode, FileContents callback) throws IOException {
	_raf = new RandomAccessFile(file, mode);
	_length = _raf.length();
	_contents = callback;
	Debug.jawsAssert(_contents != null, "must be set");
	// Initialize message here. This will always be called by priviledge code (i.e., Java Web Start).
	// The other entry points might be called by sandboxed code and therefore cannot be initialized
	if (_message == null) {
	    _message = Resources.getString("APIImpl.persistence.filesizemessage");
	}
    }
    
    public void close() throws IOException {
	_raf.close();
    }
    
    public long length() throws IOException {
	return _raf.length();
    }
    
    public long getFilePointer() throws IOException {
	return _raf.getFilePointer();
    }
    
    public int read() throws IOException {
	return _raf.read();
    }
    
    public int read(byte [] b, int off, int len) throws IOException {
	return _raf.read(b, off, len);
    }
    
    public int read(byte [] b) throws IOException {
	return _raf.read(b);
    }
    
    public void readFully(byte [] b) throws IOException {
	_raf.readFully(b);
    }
    
    public void readFully(byte b[], int off, int len) throws IOException {
	_raf.readFully(b, off, len);
    }
    
    public int skipBytes(int n) throws IOException {
	return _raf.skipBytes(n);
    }
    
    public boolean readBoolean() throws IOException {
	return _raf.readBoolean();
    }
    
    public byte readByte() throws IOException {
	return _raf.readByte();
    }
    
    public int readUnsignedByte() throws IOException {
	return _raf.readUnsignedByte();
    }
    
    public short readShort() throws IOException {
	return _raf.readShort();
    }
    
    public int readUnsignedShort() throws IOException {
	return _raf.readUnsignedShort();
    }
    
    public char readChar() throws IOException {
	return _raf.readChar();
    }
    
    public int readInt() throws IOException {
	return _raf.readInt();
    }
    
    public long readLong() throws IOException {
	return _raf.readLong();
    }
    
    public float readFloat() throws IOException {
	return _raf.readFloat();
    }
    
    public double readDouble() throws IOException {
	return _raf.readDouble();
    }
    
    public String readLine() throws IOException {
	return _raf.readLine();
    }
    
    public String readUTF() throws IOException {
	return _raf.readUTF();
    }
    
    public void seek(long pos) throws IOException {
	_raf.seek(pos);
    }
    
    public void setLength(long newLength) throws IOException {
	if (newLength > _contents.getMaxLength()) {
	    throw new IOException(_message);
	}
	_raf.setLength(newLength);
    }
    
    public void write(int b) throws IOException {
	checkWrite(1);
	_raf.write(b);
    }
    
    
    public void write(byte b[]) throws IOException {
	if (b != null) checkWrite(b.length);
	_raf.write(b);
    }
    
    public void write(byte b[], int off, int len) throws IOException {
	checkWrite(len);
	_raf.write(b, off, len);
    }
    
    public void writeBoolean(boolean v) throws IOException {
	checkWrite(1);
	_raf.writeBoolean(v);
    }
    
    public void writeByte(int v) throws IOException {
	checkWrite(1);
	_raf.writeByte(v);
    }
    
    public void writeShort(int v) throws IOException {
	checkWrite(2);
	_raf.writeShort(v);
    }
    
    public void writeChar(int v) throws IOException {
	checkWrite(2);
	_raf.writeChar(v);
    }
    
    public void writeInt(int v) throws IOException {
	checkWrite(4);
	_raf.writeInt(v);
    }
    
    public void writeLong(long v) throws IOException {
	checkWrite(8);
	_raf.writeLong(v);
    }
    
    public void writeFloat(float v) throws IOException {
	checkWrite(4);
	_raf.writeFloat(v);
    }
    
    public void writeDouble(double v) throws IOException {
	checkWrite(8);
	_raf.writeDouble(v);
    }
    
    public void writeBytes(String s) throws IOException {
	if (s != null) checkWrite(s.length());
	_raf.writeBytes(s);
    }
    
    public void writeChars(String s) throws IOException {
	if (s != null) checkWrite(s.length() * 2);
	_raf.writeChars(s);
    }
    
    public void writeUTF(String s) throws IOException {
	if (s != null) checkWrite(getUTFLen(s));
	_raf.writeUTF(s);
    }
    
    private int getUTFLen(String str) {
	int strlen = str.length();
	char[] charr = new char[strlen];
	int c, count;
	
	str.getChars(0, strlen, charr, 0);
	
	count = 2;
	for (int i = 0; i < strlen; i++ ) {
	    c = charr[i];
	    if ((c >= 0x0001) && (c <=0x007F)) {
		count++;
	    } else if (c > 0x07FF) {
		count += 3;
	    } else {
		count += 2;
	    }
	}
	
	return count;
    }
    
    private void checkWrite(int len) throws IOException {
	if (_raf.getFilePointer() + len > _contents.getMaxLength()) {
	    throw new IOException(_message);
	}
    }
}

