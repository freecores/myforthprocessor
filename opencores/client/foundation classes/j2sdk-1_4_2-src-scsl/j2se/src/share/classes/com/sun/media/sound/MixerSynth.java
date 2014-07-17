/*
 * @(#)MixerSynth.java	1.22 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.media.sound;

import java.util.Vector;

import javax.sound.midi.*;
import javax.sound.sampled.LineUnavailableException;

			 


/******************************************************************************************
IMPLEMENTATION TODO:


******************************************************************************************/



/**
 * Synthesizer using the Headspace Mixer.
 *
 * @version 1.22, 03/01/23
 * @author David Rivas
 * @author Kara Kytle
 */
class MixerSynth extends AbstractPlayer implements Synthesizer {

    /**
     * All MixerSynths share this info object.
     */
    static final MixerSynthInfo info = new MixerSynthInfo();

	
    // CONSTRUCTOR
	
    MixerSynth() throws MidiUnavailableException {

	super(info);
	if(Printer.trace) Printer.trace(">> MixerSynth CONSTRUCTOR");
	if(Printer.trace) Printer.trace("<< MixerSynth CONSTRUCTOR completed");
    }


    // HELPER METHODS


    /**
     * MixerSynth always returns -1.
     */
    public long getTimeStamp() {
	return -1;
    }


    /**
     * MixerSynth ignores this.
     */
    public void setTimeStamp(long timeStamp) {
    }


    public void implOpen() throws MidiUnavailableException {
		
	id = nCreateSynthesizer();

	if (id == 0) {
	    throw new MidiUnavailableException("Failed to initialize synthesizer");
	}

	// open the default synth 
	openInternalSynth(); // can throw MidiUnavailableException

	// connect to the internal synth (with the new id value)
	connectToInternalSynth();


	// $$kk: 04.07.99: can this fail??
	if (!(nStartSynthesizer(id))) {

	    id = 0;
	    throw new MidiUnavailableException("Failed to start synthesizer");
	}

	// update the channels with the new id
	for (int i = 0; i < channels.length; i++) {
	    channels[i].setId(id);
	}
    }



    // INNER CLASSES

    private static class MixerSynthInfo extends MidiDevice.Info {
															    
	private static final String name = "Java Sound Synthesizer";
	private static final String vendor = "Sun Microsystems";
	private static final String description = "Software wavetable synthesizer and receiver";
	private static final String version = "Version 1.0";

	private MixerSynthInfo() {
	    super(name, vendor, description, version);
	}
    }


    // NATIVE METHODS

    // SYNTH CREATION AND DESTRUCTION

    // GM_CreateLiveSong
    private native long nCreateSynthesizer(); 

    // GM_StartLiveSong
    private native boolean nStartSynthesizer(long id);
}

										
