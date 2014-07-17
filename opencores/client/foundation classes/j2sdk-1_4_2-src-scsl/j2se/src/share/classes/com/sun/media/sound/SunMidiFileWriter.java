/*
 * @(#)SunMidiFileWriter.java	1.13 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.media.sound;

import java.io.File;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.IOException;
import java.net.URL;
import java.net.MalformedURLException;

import javax.sound.midi.spi.MidiFileWriter;
import javax.sound.midi.MidiFileFormat;
import javax.sound.midi.InvalidMidiDataException;	
import javax.sound.midi.Sequence;


/**
 * Abstract MIDI Reader class.
 *
 * @version 1.13 03/01/23
 * @author Jan Borgersen
 */
abstract class SunMidiFileWriter extends MidiFileWriter {

    private static SunMidiFileWriter[] midiFileWriters;

    // List of writers we provide
    private static final String[] midiFileWriterClassNames = {
	"com.sun.media.sound.MidiFileWriter"
    };

    static {

	midiFileWriters = new SunMidiFileWriter[ midiFileWriterClassNames.length ];
    }

    /**
     * Constructs a new SunMidiFileReader object.
     */
    public SunMidiFileWriter() {
    }

    /**
     * Obtains the file types that this provider can write from the
     * sequence specified.
     * @param sequence the sequence for which midi file type support
     * is queried
     * @return array of file types.  If no file types are supported, 
     * returns an array of length 0.
     */
    //	public abstract int[] getMidiFileTypes(Sequence sequence);

    /**
     * Obtains the file formats of the indicated type that this provider 
     * can write from the sequence specified.
     * @param type desired midi file type
     * @param sequence the sequence for which midi file format support
     * is queried
     * @return array of midi file formats of the type indicated.  If no 
     * file formats of this type are supported, returns an array of length 0.
     */
    //	public abstract MidiFileFormat[] getMidiFileFormats(int type, Sequence sequence);

    /**
     * Indicates whether an midi file of the file format specified can be written
     * from the sequence indicated.
     * @param fileFormat description of the file for which write capabilities are queried
     * @param sequence  the sequence for which file writing support is queried
     * @return <code>true</code> if the file format is supported for this sequence, 
     * otherwise <code>false</code>
     */
    //	public abstract boolean isFileFormatSupported(MidiFileFormat fileFormat, Sequence sequence);

    /**
     * Writes a stream of bytes representing an midi file of the file format
     * indicated to the output stream provided. 
     * @param in sequence containing midi data to be written to the file
     * @param format format of the file to be written to the output stream
     * @param out stream to which the file data should be written
     * @return the number of bytes written to the output stream
     * @throws IOException if an I/O exception occurs
     * @throws IllegalArgumentException if the file format is not supported by
     * the system  
     * @see #isFileFormatSupported
     * @see	#getMidiFileFormats
     */
    //	public abstract int write(Sequence in, MidiFileFormat format, OutputStream out) throws IOException;

    /**
     * Writes a stream of bytes representing an midi file of the file format
     * indicated to the external file provided.
     * @param in sequence containing midi data to be written to the file
     * @param format format of the file to be written to the output stream
     * @param out external file to which the file data should be written
     * @return the number of bytes written to the file
     * @throws IOException if an I/O exception occurs
     * @throws IllegalArgumentException if the file format is not supported by
     * the system  
     * @see #isFileFormatSupported
     * @see	#getMidiFileFormats
     */
    //	public abstract int write(Sequence in, MidiFileFormat format, File out) throws IOException;

    // STATIC METHODS REQUIRED BY MIDI INSTALLER

    /**
     * Gets the number of midi file parsers available in the system.
     */
    static int getNumFileWriters() {

	return midiFileWriters.length;
    }
    /**
     * Gets a particular midi file parser instance.
     */
    static SunMidiFileWriter getFileWriter(int index) {

	synchronized( midiFileWriters ) {
	    try {
				// initialize it if it hasn't been done yet
		if (midiFileWriters[index] == null) {

		    Class midiFileWriterClass = Class.forName(midiFileWriterClassNames[index]);
		    midiFileWriters[index] = (SunMidiFileWriter) midiFileWriterClass.newInstance();
		}
	    } catch (ClassNotFoundException e) {
		if(Printer.err) Printer.err("ClassNotFoundException: " + e);
	    } catch (InstantiationException e2) {
		if(Printer.err) Printer.err("InstantiationException: " + e2);
	    } catch (IllegalAccessException e3) {
		if(Printer.err) Printer.err("IllegalAccessException: " + e3);
	    }
	}
	return midiFileWriters[index];
    }
}
