/*
 * @(#)SunMidiFileReader.java	1.13 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.media.sound;

import java.io.File;
import java.io.InputStream;
import java.io.IOException;
import java.net.URL;
import java.net.MalformedURLException;

import javax.sound.midi.spi.MidiFileReader;
import javax.sound.midi.MidiFileFormat;
import javax.sound.midi.InvalidMidiDataException;	
import javax.sound.midi.Sequence;


/**
 * Abstract MIDI Reader class.
 *
 * @version 1.13 03/01/23
 * @author Jan Borgersen
 */
abstract class SunMidiFileReader extends MidiFileReader {

    private static SunMidiFileReader[] midiFileReaders;

    // List of readers we provide
    private static final String[] midiFileReaderClassNames = {
	"com.sun.media.sound.MidiFileReader",
	"com.sun.media.sound.RmfFileReader"
    };

    static {

	midiFileReaders = new SunMidiFileReader[ midiFileReaderClassNames.length ];
    }

    /**
     * Constructs a new SunMidiFileReader object.
     */
    public SunMidiFileReader() {
    }


    /**
     * Obtains the MIDI file format of the input stream provided.  The stream must
     * point to valid MIDI file data.  In general, MIDI file providers may 
     * need to read some data from the stream before determining whether they
     * support it.  These parsers must
     * be able to mark the stream, read enough data to determine whether they 
     * support the stream, and, if not, reset the stream's read pointer to its original 
     * position.  If the input stream does not support this, this method may fail
     * with an IOException. 
     * @param stream the input stream from which file format information should be
     * extracted
     * @return an <code>MidiFileFormat</code> object describing the MIDI file format
     * @throws InvalidMidiDataException if the stream does not point to valid MIDI
     * file data recognized by the system
     * @throws IOException if an I/O exception occurs
     * @see InputStream#markSupported
     * @see InputStream#mark
     */
    public abstract MidiFileFormat getMidiFileFormat(InputStream stream) throws InvalidMidiDataException, IOException;


    /**
     * Obtains the MIDI file format of the URL provided.  The URL must
     * point to valid MIDI file data.
     * @param url the URL from which file format information should be
     * extracted
     * @return an <code>MidiFileFormat</code> object describing the MIDI file format
     * @throws InvalidMidiDataException if the URL does not point to valid MIDI
     * file data recognized by the system
     * @throws IOException if an I/O exception occurs
     */
    public abstract MidiFileFormat getMidiFileFormat(URL url) throws InvalidMidiDataException, IOException;


    /**
     * Obtains the MIDI file format of the File provided.  The File must
     * point to valid MIDI file data.
     * @param file the File from which file format information should be
     * extracted
     * @return an <code>MidiFileFormat</code> object describing the MIDI file format
     * @throws InvalidMidiDataException if the File does not point to valid MIDI
     * file data recognized by the system
     * @throws IOException if an I/O exception occurs
     */
    public abstract MidiFileFormat getMidiFileFormat(File file) throws InvalidMidiDataException, IOException;


    /**
     * Obtains a MIDI sequence from the input stream provided.  The stream must
     * point to valid MIDI file data.  In general, MIDI file providers may 
     * need to read some data from the stream before determining whether they
     * support it.  These parsers must
     * be able to mark the stream, read enough data to determine whether they 
     * support the stream, and, if not, reset the stream's read pointer to its original 
     * position.  If the input stream does not support this, this method may fail
     * with an IOException. 
     * @param stream the input stream from which the <code>Sequence</code> should be
     * constructed
     * @return an <code>Sequence</code> object based on the MIDI file data contained
     * in the input stream.
     * @throws InvalidMidiDataException if the stream does not point to valid MIDI
     * file data recognized by the system
     * @throws IOException if an I/O exception occurs
     * @see InputStream#markSupported
     * @see InputStream#mark
     */
    public abstract Sequence getSequence(InputStream stream) throws InvalidMidiDataException, IOException;


    /**
     * Obtains a MIDI sequence from the URL provided.  The URL must
     * point to valid MIDI file data.
     * @param url the URL for which the <code>Sequence</code> should be
     * constructed
     * @return an <code>Sequence</code> object based on the MIDI file data pointed
     * to by the URL
     * @throws InvalidMidiDataException if the URL does not point to valid MIDI
     * file data recognized by the system
     * @throws IOException if an I/O exception occurs
     */
    public abstract Sequence getSequence(URL url) throws InvalidMidiDataException, IOException;


    /**
     * Obtains a MIDI sequence from the File provided.  The File must
     * point to valid MIDI file data.
     * @param file the File for which the <code>Sequence</code> should be
     * constructed
     * @return an <code>Sequence</code> object based on the MIDI file data pointed
     * to by the File
     * @throws InvalidMidiDataException if the File does not point to valid MIDI
     * file data recognized by the system
     * @throws IOException if an I/O exception occurs
     */
    public abstract Sequence getSequence(File file) throws InvalidMidiDataException, IOException;



    // STATIC METHODS REQUIRED BY MIDI INSTALLER

    /**
     * Gets the number of midi file parsers available in the system.
     */
    static int getNumFileReaders() {

	return midiFileReaders.length;
    }
    /**
     * Gets a particular midi file parser instance.
     */
    static SunMidiFileReader getFileReader(int index) {

	synchronized( midiFileReaders ) {
	    try {
				// initialize it if it hasn't been done yet
		if (midiFileReaders[index] == null) {

		    Class midiFileReaderClass = Class.forName(midiFileReaderClassNames[index]);
		    midiFileReaders[index] = (SunMidiFileReader) midiFileReaderClass.newInstance();
		}
	    } catch (ClassNotFoundException e) {
		if(Printer.err) Printer.err("ClassNotFoundException: " + e);
	    } catch (InstantiationException e2) {
		if(Printer.err) Printer.err("InstantiationException: " + e2);
	    } catch (IllegalAccessException e3) {
		if(Printer.err) Printer.err("IllegalAccessException: " + e3);
	    }
	}
	return midiFileReaders[index];
    }
}
