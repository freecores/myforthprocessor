/*
 * @(#)MixerSynthProvider.java	1.14 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.media.sound;

import javax.sound.midi.MidiDevice;
import javax.sound.midi.MidiUnavailableException;
import javax.sound.midi.spi.MidiDeviceProvider;

/**
 * Provider for MixerSynth objects.
 *
 * @version 1.14, 03/01/23
 * @author Kara Kytle
 */
public class MixerSynthProvider extends MidiDeviceProvider {


    public MidiDevice.Info[] getDeviceInfo() {

	MidiDevice.Info[] localArray = { MixerSynth.info };
	return localArray;
    }


    // $$kk: 04.14.99: should probably throw this exception!!
    public MidiDevice getDevice(MidiDevice.Info info) {

	if ( (info != null) && (!info.equals(MixerSynth.info)) ) {

	    // $$kk: 06.22.99: should throw exception?
	    return null;
	}

	try {
	    return new MixerSynth();
	} catch (MidiUnavailableException e) {
	    return null;
	}
    }
}
