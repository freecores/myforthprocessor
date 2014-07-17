/*
 * @(#)SunFileWriter.java	1.15 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.media.sound;	  	 

import java.io.File;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.IOException;
import java.io.DataInputStream;
import java.io.RandomAccessFile;
import java.net.URL;

import java.lang.reflect.Method;
//import java.security.PrivilegedActionException;

import javax.sound.sampled.AudioFormat;
import javax.sound.sampled.AudioFileFormat;
import javax.sound.sampled.AudioInputStream;
import javax.sound.sampled.AudioSystem;
import javax.sound.sampled.UnsupportedAudioFileException;
import javax.sound.sampled.spi.AudioFileWriter;




/**
 * Abstract File Writer class.
 *
 * @version 1.15 03/01/23
 * @author Jan Borgersen
 */
abstract class SunFileWriter extends AudioFileWriter {

	
    private static SunFileWriter[] fileWriters;

    // buffer size for write
    protected static final int bufferSize = 16384;

    // buffer size for temporary input streams
    protected static final int bisBufferSize = 4096;


    // List of parsers we provide
    private static final String[] fileWriterClassNames = {
	"com.sun.media.sound.AuFileWriter",
	"com.sun.media.sound.WaveFileWriter",
	"com.sun.media.sound.AiffFileWriter"
    };
					
    static {

	fileWriters = new SunFileWriter[ fileWriterClassNames.length ];
    }

    final AudioFileFormat.Type types[];


    /**
     * Constructs a new SunParser object.
     */
    SunFileWriter(AudioFileFormat.Type types[]) {		
	this.types = types;
    }



    // METHODS TO IMPLEMENT AudioFileWriter

    // new, 10.27.99

    public AudioFileFormat.Type[] getAudioFileTypes(){
		
	AudioFileFormat.Type[] localArray = new AudioFileFormat.Type[types.length];
	System.arraycopy(types, 0, localArray, 0, types.length);
	return localArray;
    }


    // old
	
    public abstract AudioFileFormat.Type[] getAudioFileTypes(AudioInputStream stream);

    //abstract public AudioFileFormat[] getAudioFileFormats(AudioFileFormat.Type type, AudioInputStream stream);

    //abstract public boolean isFileFormatSupported(AudioFileFormat fileFormat, AudioInputStream stream);
    //abstract public boolean isFileTypeSupported(AudioFileFormat.Type fileType, AudioInputStream stream);
	
    //abstract public int write(InputStream in, AudioFileFormat format, OutputStream out) throws IOException;
    public abstract int write(AudioInputStream stream, AudioFileFormat.Type fileType, OutputStream out) throws IOException;

    //abstract public int write(InputStream in, AudioFileFormat format, File out) throws IOException;
    public abstract int write(AudioInputStream stream, AudioFileFormat.Type fileType, File out) throws IOException;

	
    // STATIC METHODS REQUIRED BY AUDIO INSTALLER

    /**
     * Gets the number of file parsers available in the system.
     */
    static int getNumFileWriters() {
		
	return fileWriters.length;
    }

    /**
     * Gets a particular file parser instance.
     */
    static SunFileWriter getFileWriter(int index) {

	synchronized (fileWriters) {
	    try {
				// initalize it if it hasn't been done yet
		if (fileWriters[index] == null) {
				
		    Class fileWriterClass = Class.forName(fileWriterClassNames[index]);
		    fileWriters[index] = (SunFileWriter) fileWriterClass.newInstance();
		}
	    } catch (ClassNotFoundException e) {			
		if(Printer.err) Printer.err("ClassNotFoundException: " + e);
	    } catch (InstantiationException e2) {
		if(Printer.err) Printer.err("InstantiationException: " + e2);
	    } catch (IllegalAccessException e3) {
		if(Printer.err) Printer.err("IllegalAccessException: " + e3);
	    }
	}
	return fileWriters[index];
    }

	
    // HELPER METHODS

    private static JSSecurity jsSecurity = null;
    private static boolean securityPrivilege = false;
    private static Method m[] = new Method[1];
    private static Class cl[] = new Class[1];
    private static Object args[][] = new Object[1][0];


    static protected RandomAccessFile openRandomAccessFile( File file ) throws IOException {

	RandomAccessFile raf = null;

	// only get the security classes the first time we try to write.
	if( !securityPrivilege ) {
	    try {
		jsSecurity = JSSecurityManager.getJSSecurity();
		securityPrivilege = true;
	    } catch (SecurityException e) {
		if(Printer.err) Printer.err("SunParser.java: Security Exception: " + e);
	    }
	}

	if ( securityPrivilege && (jsSecurity != null ) ) {

	    if( jsSecurity.getName().startsWith("JDK12") ) {
			  
		//			  // run privileged code with 1.2-style security
		//				try {
		//					Class.forName("java.security.AccessController");
		//
		//					raf = (RandomAccessFile) 
		//						java.security.AccessController.doPrivileged(
		//							new java.security.PrivilegedExceptionAction() {
		//								public Object run() throws IOException {
		//									return new RandomAccessFile( file, "rw" );
		//								}
		//							});
		//	
		//				} catch (ClassNotFoundException e) {
		//					// run privileged code without security
		raf = new RandomAccessFile( file, "rw" );
		//				} catch (PrivilegedActionException e) {
		//
		//					throw (IOException) e.getException();
		//				}

	    } else {
				// run privileged code with non-1.2-style security
		try {
		    jsSecurity.requestPermission(m, cl, args, JSSecurity.READ_FILE);
		    m[0].invoke(cl[0], args[0]);
		    jsSecurity.requestPermission(m, cl, args, JSSecurity.WRITE_FILE);
		    m[0].invoke(cl[0], args[0]);

		} catch (Exception e) {
		    if(Printer.err) Printer.err("Unable to get file privileges: " + e);
		}
		raf = new RandomAccessFile( file, "rw" );
	    }

	} else {
	    // run privileged code without security
	    raf = new RandomAccessFile( file, "rw" );
	}

	return raf;
    }


    /** 
     * rllong 
     * Protected helper method to read 64 bits and changing the order of 
     * each bytes. 
     * @param DataInputStream 
     * @return 32 bits swapped value. 
     * @exception IOException
     */
    protected int rllong(DataInputStream dis) throws IOException {

        int b1, b2, b3, b4 ;
        int i = 0;
 
       	i = dis.readInt();
 
       	b1 = ( i & 0xFF ) << 24 ;
       	b2 = ( i & 0xFF00 ) << 8;
       	b3 = ( i & 0xFF0000 ) >> 8;
       	b4 = ( i & 0xFF000000 ) >>> 24;
 
       	i = ( b1 | b2 | b3 | b4 );
 
        return i;
    }

    /**
     * big2little
     * Protected helper method to swap the order of bytes in a 32 bit int
     * @param int
     * @return 32 bits swapped value
     */
    protected int big2little(int i) {

        int b1, b2, b3, b4 ;
 
       	b1 = ( i & 0xFF ) << 24 ;
       	b2 = ( i & 0xFF00 ) << 8;
       	b3 = ( i & 0xFF0000 ) >> 8;
       	b4 = ( i & 0xFF000000 ) >>> 24;
 
       	i = ( b1 | b2 | b3 | b4 );
 
        return i;
    }

    /**
     * rlshort
     * Protected helper method to read 16 bits value. Swap high with low byte.
     * @param DataInputStream
     * @return the swapped value.
     * @exception IOException
     */ 
    protected short rlshort(DataInputStream dis)  throws IOException {

	short s=0; 
	short high, low;

	s = dis.readShort();
		
	high = (short)(( s & 0xFF ) << 8) ;
	low = (short)(( s & 0xFF00 ) >>> 8);
		
	s = (short)( high | low );
		
	return s;
    }

    /**
     * big2little
     * Protected helper method to swap the order of bytes in a 16 bit short
     * @param int
     * @return 16 bits swapped value
     */
    protected short big2littleShort(short i) {

	short high, low;

	high = (short)(( i & 0xFF ) << 8) ;
	low = (short)(( i & 0xFF00 ) >>> 8);

	i = (short)( high | low );

	return i;
    }

}
