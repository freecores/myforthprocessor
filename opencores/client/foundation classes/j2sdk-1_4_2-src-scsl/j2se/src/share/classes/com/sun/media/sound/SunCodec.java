/*
 * @(#)SunCodec.java	1.20 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.media.sound;		  	 

import java.io.InputStream;

import javax.sound.sampled.AudioFormat;
import javax.sound.sampled.AudioInputStream;

import javax.sound.sampled.spi.FormatConversionProvider;


/**
 * A codec can encode and/or decode audio data.  It provides an
 * AudioInputStream from which processed data may be read.
 * <p>
 * Its input format represents the format of the incoming
 * audio data, or the format of the data in the underlying stream.
 * <p>  
 * Its output format represents the format of the processed, outgoing
 * audio data.  This is the format of the data which may be read from 
 * the filtered stream.
 *
 * @version 1.20 03/01/23
 * @author Kara Kytle
 */
abstract class SunCodec extends FormatConversionProvider {

    private static SunCodec[] codecs;

    AudioFormat.Encoding[] inputEncodings;
    AudioFormat.Encoding[] outputEncodings;

    private static final String[] codecClassNames = {
	"com.sun.media.sound.UlawCodec",
	"com.sun.media.sound.AlawCodec"
    };

    static {

	// $$kk: 03.25.99: initialize codecs!!!
	codecs = new SunCodec[ codecClassNames.length ];
    }


    /**
     * Constructs a new codec object.
     */
    protected SunCodec(AudioFormat.Encoding[] inputEncodings, AudioFormat.Encoding[] outputEncodings) {

	this.inputEncodings = inputEncodings;
	this.outputEncodings = outputEncodings;
    }

    // NEW FORMAT CONVERSION PROVIDER METHODS


    /**
     */
    public AudioFormat.Encoding[] getSourceEncodings() {

	AudioFormat.Encoding[] encodings = new AudioFormat.Encoding[inputEncodings.length];
	System.arraycopy(inputEncodings, 0, encodings, 0, inputEncodings.length);
	return encodings;
    }
    /**
     */
    public AudioFormat.Encoding[] getTargetEncodings() {

	AudioFormat.Encoding[] encodings = new AudioFormat.Encoding[outputEncodings.length];
	System.arraycopy(outputEncodings, 0, encodings, 0, outputEncodings.length);
	return encodings;
    }

    /**
     */
    public abstract AudioFormat.Encoding[] getTargetEncodings(AudioFormat sourceFormat);


    /**
     */
    public abstract AudioFormat[] getTargetFormats(AudioFormat.Encoding targetEncoding, AudioFormat sourceFormat);


    /**
     */
    public abstract AudioInputStream getAudioInputStream(AudioFormat.Encoding targetEncoding, AudioInputStream sourceStream);
    /**
     */
    public abstract AudioInputStream getAudioInputStream(AudioFormat targetFormat, AudioInputStream sourceStream);




    // OLD CODE


    /**
     * Opens the codec with the specifiec parameters.
     * @param stream stream from which data to be processed should be read
     * @param outputFormat desired data format of the stream after processing
     * @return stream from which processed data may be read
     * @throws IllegalArgumentException if the format combination supplied is
     * not supported.
     */
    /*	public abstract AudioInputStream getConvertedStream(AudioFormat outputFormat, AudioInputStream stream);
     */

    /**
     * Obtains the set of input encodings supported by the codec.
     * If none are supported, returns an array of length 0.
     * @return array of supported input encodings. 
     */
    /*	public AudioFormat.Encoding[] getInputEncodings() {

	AudioFormat.Encoding[] encodings = new AudioFormat.Encoding[inputEncodings.length];
	System.arraycopy(inputEncodings, 0, encodings, 0, inputEncodings.length);
	return encodings;
	}
    */

    /**
     * Obtains the set of input encodings supported by the codec,
     * given a particular output encoding.
     * If none are supported, returns an array of length 0.
     * @return array of supported input encodings. 
     */
    /*	public AudioFormat.Encoding[] getInputEncodings(AudioFormat.Encoding outputEncoding) {

	return getInputEncodings();
	}
    */


    /**
     * Obtains the set of input formats supported by the codec 
     * given a particular output format.  
     * If no input formats are supported for this output format, 
     * returns an array of length 0.
     * @return array of supported input formats. 
     */
    /*	public AudioFormat[] getInputFormats(AudioFormat outputFormat) {

	return new AudioFormat[0];
	}
    */


    /**
     * Obtains the set of output encodings supported by the codec.
     * If none are supported, returns an array of length 0.
     * @return array of supported output encodings. 
     */
    /*	public AudioFormat.Encoding[] getOutputEncodings() {

	AudioFormat.Encoding[] encodings = new AudioFormat.Encoding[outputEncodings.length];
	System.arraycopy(outputEncodings, 0, encodings, 0, outputEncodings.length);
	return encodings;
	}
    */


    /**
     * Obtains the set of output encodings supported by the codec,
     * given a particular input encoding.
     * If none are supported, returns an array of length 0.
     * @return array of supported output encodings. 
     */
    /*	public AudioFormat.Encoding[] getOutputEncodings(AudioFormat.Encoding inputEncoding) {

	return getOutputEncodings();
	}
    */


    /**
     * Obtains the set of output formats supported by the codec 
     * given a particular input format.  
     * If no output formats are supported for this input format, 
     * returns an array of length 0.
     * @return array of supported output formats. 
     */
    /*	public AudioFormat[] getOutputFormats(AudioFormat inputFormat) {

	return new AudioFormat[0];
	}
    */


    /**
     * Determines whether the codec supports conversion from one
     * particular format to another.
     * @param inputFormat format of the incoming data
     * @param desired format of outgoing data
     * @return true if the conversion is supported, otherwise false
     */
    /*	public boolean isConversionSupported(AudioFormat inputFormat, AudioFormat outputFormat) {

	return false;
	}
    */

    // STATIC METHODS REQUIRED BY AUDIO INSTALLER

    /**
     * Gets the number of codecs available system.
     */
    static int getNumCodecs() {

	return codecs.length;
    }


    /**
     * Gets a particular codec instance.
     */
    static SunCodec getCodec(int index) {

	// $$jb: 03.28.99: Does this need to be synchronized?  I think it does...
	//		 03.29.99: codecs is final, this probably doesn't matter...
	synchronized (codecs) {
	    try {
				// initalize it if it hasn't been done yet
		if (codecs[index] == null) {
			
		    Class codecClass = Class.forName(codecClassNames[index]);
		    codecs[index] = (SunCodec) codecClass.newInstance();
		}
	    } catch (ClassNotFoundException e) {			
		if(Printer.err) Printer.err("ClassNotFoundException: " + e);
	    } catch (InstantiationException e2) {
		if(Printer.err) Printer.err("InstantiationException: " + e2);
	    } catch (IllegalAccessException e3) {
		if(Printer.err) Printer.err("IllegalAccessException: " + e3);
	    }
	}
	return codecs[index];
    }			  
}
