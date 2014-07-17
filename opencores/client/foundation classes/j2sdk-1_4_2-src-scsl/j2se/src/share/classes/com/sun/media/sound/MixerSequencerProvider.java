/*
 * @(#)MixerSequencerProvider.java	1.12 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.media.sound;

import javax.sound.midi.MidiDevice;
import javax.sound.midi.MidiUnavailableException;
import javax.sound.midi.spi.MidiDeviceProvider;

/**
 * Provider for MixerSequencer objects.
 *
 * @version 1.12, 03/01/23
 * @author Kara Kytle
 */
public class MixerSequencerProvider extends MidiDeviceProvider {


    public MidiDevice.Info[] getDeviceInfo() {

	MidiDevice.Info[] localArray = { MixerSequencer.info };
	return localArray;
    }


    // $$kk: 04.14.99: should probably throw this exception!!
    public MidiDevice getDevice(MidiDevice.Info info) {

	if ( (info != null) && (!info.equals(MixerSequencer.info)) ) {

	    // $$kk: 06.22.99: should throw exception?
	    return null;
	}

	try {
	    return new MixerSequencer();
	} catch (MidiUnavailableException e) {
	    return null;
	}
    }
}
